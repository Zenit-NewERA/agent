/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : utils.C
 *
 *    AUTHOR     : Anton Ivanov, Sergey Serebyakov
 *
 *    $Revision: 2.10 $
 *
 *    $Id: utils.C,v 2.10 2004/08/29 14:07:21 anton Exp $
 *
 ************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your opfion) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* -*- Mode: C++ -*- */

/* utils.C
 * CMUnited99 (soccer client for Robocup99)
 * Peter Stone <pstone@cs.cmu.edu>
 * Computer Science Department
 * Carnegie Mellon University
 * Copyright (C) 1999 Peter Stone
 *
 * CMUnited-99 was created by Peter Stone, Patrick Riley, and Manuela Veloso
 *
 * You may copy and distribute this program freely as long as you retain this notice.
 * If you make any changes or have any comments we would appreciate a message.
 * For more information, please see http://www.cs.cmu.edu/~robosoccer/
 */


#include "utils.h"
#include "client.h"
#include "Playposition.h"
#include <iomanip>
namespace statistica {
  Statistic stat;
  TimeCounter tcounter;

  Statistic::Statistic():MAX_DEBUG_FUNCTIONS(1) {
    if(MAX_DEBUG_FUNCTIONS>0){
      calcTimes=new double[MAX_DEBUG_FUNCTIONS*6];
      for(int i=0;i<MAX_DEBUG_FUNCTIONS*6;++i)
	calcTimes[i]=0.0;
      names=new string[MAX_DEBUG_FUNCTIONS];
    }else{
      calcTimes=0;
      names=0;
    }
	
    for(int i=0;i<11;i++){
      numberTM[i]=numberOP[i]=0;
      TM[i]=OP[i]=0.0f;
    }
    numberBall=0;
    Ball=0.0f;
    effortNum=0;
    effortSum=0.0;
	
    Handleball = 0.0;
    numberHandleball=0;
  }

  void Statistic::SaveStatistic() {
    string str="./logs/stat";
    str+=Mem->MyNumber>9?Mem->MyNumber%10+'A':'0'+Mem->MyNumber;
    str+=Mem->MySide;
    str+=".log";
    ofstream file(str.c_str());
    file<<"Player: "<<Pos.GetPlayerName()<<"; Number: "<<Mem->MyNumber<<"; All cycles: "<<Mem->CurrentTime.t;
    file<<endl<<"Lost kicks:"<<Mem->lost_kicks<<"/"<<Mem->kicks<<"; Lost dash:"<<Mem->lost_dash<<"/"<<Mem->dashes<<
      "; Lost turns:"<<Mem->lost_turns<<"/"<<Mem->turns<<"; Lost turn necks:"<<Mem->lost_turn_necks<<"/"<<Mem->turn_necks;

    file<<endl<<"********************************************************"<<endl;
    //out all values
    file<<"Teammates: ";
    for(int i=0;i<11;i++)
      file<<i+1<<" - "<<setprecision(2)<<TM[i]/numberTM[i]<<"; ";
    file<<endl<<"Opponents: ";
    for(int i=0;i<11;i++)
      file<<i+1<<" - "<<setprecision(2)<<OP[i]/numberOP[i]<<"; ";
    file<<endl<<"Avarage: ";
    float avtm=0.0f,avop=0.0f;
    for(int i=0;i<11;i++){
      avtm+=TM[i]/numberTM[i];
      avop+=OP[i]/numberOP[i];
    }
    file<<"teammates - "<<setprecision(2)<<avtm/11<<";  ";
    file<<"opponents - "<<setprecision(2)<<avop/11<<endl;
    file<<"Ball confidance: "<<setprecision(2)<<Ball/numberBall<<endl;
    file<<"Effort average: "<<setprecision(4)<<effortSum/effortNum<<endl;
    file<<"Number of cycles in ball possesion mode: "<<numberHandleball<<" ("<<float(numberHandleball)/Mem->CurrentTime.t<<"%)"<<endl;
    file<<"Handleball avarage time thinking: "<<setprecision(5)<<Handleball<<"s"<<endl;
	
    for(int j=0;j<MAX_DEBUG_FUNCTIONS;j++){
      file<<"Base "<<names[j]<<" ("<<j<<"):"<<endl;
      for(int i=0;i<6;i++){
	file<<"From "<<i*1000<<" to "<<(i+1)*1000<<": "<<calcTimes[i+j*6]<<endl;
      }
      file<<endl;
    }
  }

  void Statistic::UpdateStatistic() {
    for(int i=0;i<11;i++){
      numberTM[i]++;
      numberOP[i]++;
      TM[i]+=Mem->TeammatePositionValid(i+1);
      OP[i]+=Mem->OpponentPositionValid(i+1);
    }
    numberBall++;
    Ball+=Mem->BallPositionValid();
    effortNum++;
    effortSum+=Mem->MyEffort();
  }
  void Statistic::UpdateHandleballStatistic(double time) {
    numberHandleball++;
    Handleball=Handleball+(time-Handleball)/numberHandleball;
  }
  void Statistic::UpdateCalcTimes(int base,double time){
    if(Mem->PlayMode!=PM_Play_On||Mem->CurrentTime.t>6000)
      return;
    int index=Mem->CurrentTime.t/1000;
    int cout=Mem->CurrentTime.t%1000+1;
    calcTimes[index+base*6]+=(time-calcTimes[index+base*6])/cout;
  }

}

FuncCalcTime::~FuncCalcTime(){
  time.Finish();
  Mem->LogAction4(30,"%s: calculation time is %f",const_cast<char*> (name.c_str()),time.GetTime());
}

#define Near(max,min) ((max-min)<0.001 ? true : false )

double normalize( double a, double min, double max ) {
  if( max < min ) return 0.0;
  if( a > max ) return 1.0;
  if( a < min ) return 0.0;
  if( Near( max, min ) ) return 0.5;
  else return( (a-min)/(max-min) );
}
//!!buffer must be initialize somewhere(client.C!!!)

int dump_core(char* msg) {
  my_stamp;
  fprintf(stderr,"dumping core");
  msg[1000000]=0;
  my_error("Core didn't dump");
  return 0;
}

#define MY_ERROR_LOG_LEVEL 5

void my_error(const char* msg) {
  //  fprintf(stderr,"AGENT'S ERROR (player %d, time %d.%d): %s\n",
  //  Mem->MyNumber,Mem->CurrentTime.t,Mem->CurrentTime.s,msg);
  Mem->LogAction3(MY_ERROR_LOG_LEVEL, "MyError: %s", const_cast<char*>(msg));
}

void my_error(const char* msg, int param) {
  char outstring[100];
  sprintf(outstring,msg,param);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2, int param3) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2, int param3, int param4) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2, int param3, int param4, int param5) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4,param5);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2, int param3, int param4, int param5, int param6) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4,param5,param6);
  my_error(outstring);
}

void my_error(const char* msg, int param1, int param2, int param3, int param4, int param5, char c1, int param6) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4,param5,c1,param6);
  my_error(outstring);
}

void my_error(const char* msg, float param) {
  char outstring[100];
  sprintf(outstring,msg,param);
  my_error(outstring);
}

void my_error(const char* msg, float param1, float param2) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2);
  my_error(outstring);
}

void my_error(const char* msg, float param1, float param2, float param3) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3);
  my_error(outstring);
}

void my_error(const char *msg, float param1, float param2, float param3, float param4) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4);
  my_error(outstring);
}

void my_error(const char *msg, float param1, int param2) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2);
  my_error(outstring);
}

void my_error(const char *msg, char* param) {
  char outstring[100];
  sprintf(outstring,msg,param);
  my_error(outstring);
}

void my_error(const char *msg, char param1, int param2, int param3) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3);
  my_error(outstring);
}

void my_error(const char *msg , char param1, int param2, float param3, float param4) {
  char outstring[100];
  sprintf(outstring,msg,param1,param2,param3,param4);
  my_error(outstring);
}

float int_pow(float x, int p) {
  if (p < 0) 
    return (1.0 / int_pow(x,-p));
  else {
    float ans = 1.0;
    for (int i=0; i<p; i++)
      ans *= x;
    return ans;
  }
}


int closest_int( float x ) {
  if ( x < 0 ) x -= 0.5;
  else         x += 0.5;
  return (int) x;
}

void NormalizeAngleDeg(int *ang) {
  if (fabs(float(*ang)) > 5000){
    my_error("Huge angle passed to NormalizeAngleDeg");
  }
		
  while( *ang >  180 ) *ang-=360;
  while( *ang < -180 ) *ang+=360;
}

void NormalizeAngleDeg(AngleDeg *ang) {
  if (fabs(*ang) > 5000)
    my_error("Huge angle passed to NormalizeAngleDeg");

  while( *ang >  180 ) *ang-=360;
  while( *ang < -180 ) *ang+=360;
}

void NormalizeAngleRad(AngleRad *ang) {
  while( *ang >  M_PI ) *ang-=2*M_PI;
  while( *ang < -M_PI ) *ang+=2*M_PI;
}

AngleDeg GetNormalizeAngleDeg(AngleDeg ang) {
  if (fabs(ang) > 5000)
    my_error("Huge angle passed to GetNormalizeAngleDeg");

  while( ang >  180 ) ang-=360;
  while( ang < -180 ) ang+=360;
  return ang;
}

AngleDeg GetDiff(AngleDeg angle1, AngleDeg angle2) {
  NormalizeAngleDeg(&angle1);
  NormalizeAngleDeg(&angle2);
	
  return fabs(GetNormalizeAngleDeg( angle1-angle2 ));
}

float GetDistance(float *x, float *y, float *a, float *b) {
  return sqrt((*x-*a)*(*x-*a) + (*y-*b)*(*y-*b));
}

float weighted_avg(float val1, float val2, float w1, float w2) {
  return (val1*w1 + val2*w2)/(w1+w2);
}

float SumGeomSeries(float first_term, float r, int n) {
  return first_term * (Exp(r, n) - 1) / (r - 1);
}

float SolveForLengthGeomSeries(float first_term, float r, float sum) {
  if (r < 0)
    my_error("SolveForSumGeomSeries: can't take r < 0");
  float temp = sum * (r-1) / first_term + 1;
  if (temp <= 0)
    return -1.0;
  return log(temp) / log(r);
}

float SolveForFirstTermGeomSeries(float r, int n, float sum) {
  return sum * (r - 1) / (Exp(r, n) - 1);
}



/* returns a pointer to a static buffer, so be careful! */
char* repeat_char(char c, int n) {
  const int MAX_REP = 100;
  static char out[MAX_REP+1];
  if (n > MAX_REP)
    my_error("repeat_char: asking for too many characters");
  for (int i=0; i<n; i++)
    out[i] = c;
  out[n] = 0;
  return out;
}


const char char_for_num_array[16] =
  {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

Time Time::operator -(const int &a) {
  if ( s==0 && t-a > Mem->LastStartClockTime.t ) /* default case */
    return Time(t-a,0);

  if ( s>0 ){ 
    if ( a <= s ) 
      return Time(t,s-a); /* Just take off from stopped time */
    else{ /* a > s */
      Time new_time = Time( t-(a-s),0 );  /* take off from stopped time, then server time */
//       if ( new_time < Mem->LastStartClockTime ) 
// 	my_error ("can't be sure of this subtraction (1)");
      return new_time;
    }
  }
  else{ /* t-a <= Mem->LastStartClockTime.t */
    int stopped_time =  a - (t - Mem->LastStartClockTime.t);  
    if ( stopped_time > Mem->LastStartClockTime.s ) {
      if ( !Mem->LastStartClockTime.t ) /* if ==0, OK---account for players starting at different times */
	return Time(0,0);
      int tmp = Mem->LastStartClockTime.t - (stopped_time - Mem->LastStartClockTime.s);
//       if ( tmp <= Mem->SecondLastStartClockTime.t ) 
// 	my_error("can't be sure of this subtraction (2) %d %d %d  %d",t,s,a,Mem->LastStartClockTime.s);
      return Time( tmp, 0 );
    }
    return Time( Mem->LastStartClockTime.t, Mem->LastStartClockTime.s - stopped_time );
  }
}

int  Time::operator -(const Time &a) {
  if ( s==0 ){
    if ( a.t < Mem->LastStartClockTime.t ){
//       if ( a.t <= Mem->SecondLastStartClockTime.t )
// 	my_error("Can't be sure of this subtraction (3) %d %d  -  %d %d",t,s,a.t,a.s);
      return (t - a.t) + Mem->LastStartClockTime.s;
    }
    if ( a.t > Mem->LastStartClockTime.t )
      return (t - a.t);
    if ( a.t == Mem->LastStartClockTime.t )
      return (t - a.t) + (Mem->LastStartClockTime.s - a.s);
  }
  else if ( s > 0 ){
 //    if ( a.t <= Mem->SecondLastStartClockTime.t )   /* If they're equal, it's not OK */
//       my_error("Can't be sure of this subtraction (4) %d %d  %d %d",t,s,a.t,a.s);
    if ( a.t < Mem->LastStartClockTime.t )
      return Mem->LastStartClockTime.s + (t - a.t);
    else if ( a.t == Mem->LastStartClockTime.t && a.t != t ) /* a is during the last stopped interval */
      return ( s + ( Mem->LastStartClockTime.s - a.s ) ) + (t - a.t);
    return (s - a.s) + (t - a.t);
  }
  else /* s<0 */
    my_error("s shouldn't be <0");
  return 0;
}

Time Time::operator +(const int &a) {
  if ( s==0 && t > Mem->LastStartClockTime.t && t+a < Mem->CurrentTime.t ) /* default case */
    return Time(t+a,0);

  Time new_time;
  
  if ( s==0 ){
     if ( Mem->LastStartClockTime.t > t ){ /* Could've missed one already */
//       my_error("Can't be sure of this addition (1) %d %d",Mem->LastStartClockTime.t,t);
      new_time = Time(t+a,0);
    }
    if ( t+a < Mem->CurrentTime.t )
      new_time = Time( t+a,0 );
    else  /* t+a >= Mem->CurrentTime.t */
      new_time = Time( Mem->CurrentTime.t, a-(Mem->CurrentTime.t-t) );
  }
  else if ( s>0 ){
    if ( t == Mem->CurrentTime.t ) /* clock still stopped */
      new_time = Time( t,s+a );
    else{
//       if ( Mem->LastStartClockTime.t != t ) /* Could've missed one already */
// 	my_error("Can't be sure of this addition (2)");
      new_time = Time ( t+(a-(Mem->LastStartClockTime.s - s)),0 );
    }
  }
  else /* s<0 */
    my_error("s shouldn't be <0");

  if ( new_time > Mem->CurrentTime ) /* clock might stop */
    my_error("Can't be sure of this addition (3)");

  return new_time;
}

Bool Time::CanISubtract(const Time &a) {
  if ( s==0 ){
    if ( a.t < Mem->LastStartClockTime.t ){
      if ( a.t <= Mem->SecondLastStartClockTime.t )
	return FALSE;
      return TRUE;
    }
    return TRUE;
  }
  else if ( s > 0 ){
    if ( a.t <= Mem->SecondLastStartClockTime.t )   /* If they're equal, it's not OK */
      return FALSE;
    return TRUE;
  }
  else /* s<0 */
    my_error("s shouldn't be <0");
  return FALSE;
}


/****************************************************************************/
/* These routines are to save time instead of using sscanf or atof, etc.    */
/* When passing **str_ptr, the pointer is advanced past the number          */
/* When passing  *str    , the pointer remains where it was before          */
/****************************************************************************/

double get_double(char **str_ptr) {

  double d_frac, result;
  int  m_flag, d_flag;

  d_frac = 1.0;
  result = 0.0;
  m_flag = d_flag = 0;

  /* Advance to the beginning of the number */
  while( !isdigit(**str_ptr) && **str_ptr!='-' && **str_ptr!='.')
    (*str_ptr)++;

  /* Process the number bit by bit */
  while((**str_ptr!=')') && (**str_ptr!=' ') && (**str_ptr!='\n') && (**str_ptr!=',')){
    if (**str_ptr=='.')
      d_flag=1;
    else if (**str_ptr=='-')
      m_flag=1;
    else if (d_flag){
      d_frac *= 10.0;
      result+=(double)(**str_ptr-'0')/d_frac;
    }
    else
      result=result*10.0+(double)(**str_ptr-'0');
    (*str_ptr)++;
  }
  if (m_flag)
    result=-result;

  //printf("%.1f\n",result);

  return result;
}

/* Get the number, but don't advance pointer */

double get_double(char *str) {
  char **str_ptr = &str;
  return get_double(str_ptr);
}

/****************************************************************************/

float get_float(char **str_ptr) {

  float d_frac, result;
  int  m_flag, d_flag;

  d_frac = 1.0;
  result = 0.0;
  m_flag = d_flag = 0;

  /* Advance to the beginning of the number */
  while( !isdigit(**str_ptr) && **str_ptr!='-' && **str_ptr!='.')
    (*str_ptr)++;

  /* Process the number bit by bit */
  while((**str_ptr!=')') && (**str_ptr!=' ') && (**str_ptr!='\n') && (**str_ptr!=',')){
    if (**str_ptr=='e')
      my_error("There's an e in my float! %s",*str_ptr);
    if (**str_ptr=='.')
      d_flag=1;
    else if (**str_ptr=='-')
      m_flag=1;
    else if (d_flag){
      d_frac *= 10.0;
      result+=(float)(**str_ptr-'0')/d_frac;
    }
    else
      result=result*10.0+(float)(**str_ptr-'0');
    (*str_ptr)++;
  }
  if (m_flag)
    result=-result;

  //printf("%.1f\n",result);

  if(result>100000||result<-100000){
    my_error("Wrong float value in parser %f",result);
    return 0.0f;
  }
  return result;
}

/* Get the number, but don't advance pointer */

float get_float(char *str) {
  char **str_ptr = &str;
  return get_float(str_ptr);
}

/****************************************************************************/

int get_int(char **str_ptr) {

  int result;
  int m_flag;

  result = 0;
  m_flag = 0;

  /* Advance to the beginning of the number */
  while( !isdigit(**str_ptr) && **str_ptr!='-')
    (*str_ptr)++;

  /* Process the number bit by bit */
  while((**str_ptr!=')') && (**str_ptr!=' ') && (**str_ptr!='\n') && (**str_ptr!=',')){
    if (**str_ptr=='-')
      m_flag=1;
    else
      result=result*10+(int)(**str_ptr-'0');
    (*str_ptr)++;
  }
  if (m_flag)
    result=-result;

  return result;
}

int get_int(char *str) {
  char **str_ptr = &str;
  return get_int(str_ptr);
}

/****************************************************************************/

void get_word(char **str_ptr) {
  while ( !isalpha(**str_ptr) ) (*str_ptr)++;
}

/****************************************************************************/

void get_next_word(char **str_ptr) {
  while ( isalpha(**str_ptr) ) (*str_ptr)++;
  get_word(str_ptr);
}

/****************************************************************************/

void get_token (char **str_ptr) {
  advance_past_space(str_ptr);
  while ( (*str_ptr) && !isspace(**str_ptr)) (*str_ptr)++;
}

/****************************************************************************/

void advance_to(char c, char **str_ptr) {
  while ( **str_ptr != c ) (*str_ptr)++;
}

/****************************************************************************/

void   advance_past_space(char **str_ptr) {
  while ( (*str_ptr) && isspace(**str_ptr)) (*str_ptr)++;
}



/****************************************************************************/
/* These routines are to save time instead of using sprintf or atof, etc.   */
/* *str should point to the END of the string where the number is going     */
/* return the length of the number placed in                                */
/****************************************************************************/

int put_float(char *str, float fnum, int precision) {
  int m_flag = 0, length = 0;
  int num, old_num;

  for (int i=0; i<precision; i++)
    fnum *= 10;

  num = closest_int(fnum);  /* round off the rest */

  if ( precision == 0 ) 
    return put_int(str,num);

  if ( num < 0 ){
    m_flag = 1;
    num = -num;
  }

  old_num = num;
  while ( num > 0 || length < precision ){
    num /= 10;
    *str = '0' + old_num - num*10;
    old_num = num;
    str--;
    length++;
    if ( length == precision ){
      *str = '.';
      str--;
      length++;
      if ( num == 0 ){
	*str = '0';
	str--;
	length++;
	break;
      }
    }
  }

  if ( m_flag ){
    *str = '-';
    length++;
  }

  return length;
}

/****************************************************************************/

int put_int(char *str, int num) {

  int m_flag = 0, length = 0;
  int old_num;

  if ( num == 0 ){
    *str = '0';
    return 1;
  }

  if ( num < 0 ){
    m_flag = 1;
    num = -num;
  }

  old_num = num;
  while ( num > 0 ){
    num /= 10;
    *str = '0' + old_num - num*10;
    old_num = num;
    str--;
    length++;
  }

  if ( m_flag ){
    *str = '-';
    length++;
  }

  return length;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void BubbleSort(int length, int *elements, float *keys) {
  
  /* Sort the elements in increasing order according to the keys */
  float keytemp;
  int eltemp;
  for (int i=0; i<length; i++){
    for (int j=i+1; j<length; j++){
      if ( keys[j] < keys[i] ){
	keytemp = keys[i];
	keys[i] = keys[j];
	keys[j] = keytemp;
	eltemp = elements[i];
	elements[i] = elements[j];
	elements[j] = eltemp;
      }
    }
  }
}

/****************************************************************************/

int BinarySearch(int length, float *elements, float key) {
  
  /* Assume the list is already sorted in increasing order */
  int lbound = 0, ubound = length;
  
  for ( int index = length/2; ubound-lbound > 0; index = lbound+(ubound-lbound)/2 ) {
    if ( elements[index] == key ){
      lbound = ubound = index;
    }
    else if ( elements[index] < key ){
      lbound = index+1;
    }
    else {
      ubound = index-1;
    }
  }
 
  int toReturn = Max(ubound,lbound);
  if (elements[toReturn] < key) toReturn++;  /* Guarantees >= key */

  return toReturn;
}

/****************************************************************************/ 

/* replace all occurrences in a string */
void StrReplace(char *str, char oldchar, char newchar) {
  int i=0;
#if 0
  int numReplaced;
#endif
  int strLength = strlen(str);
  while ( i++ < strLength ){
    if ( str[i] == oldchar ){
      str[i] = newchar;
#if 0
      numReplaced++;
#endif
    }
    if ( i==1000 ) 
      my_error("String of length >1000?");
  }
#if 0
  printf("***Replaced %d %c's in string of length %d (%d): %s***\n",
	 numReplaced,oldchar,strlen(str),i,str);
#endif
}

/****************************************************************************/
/***************************   random stuff    ******************************/
/****************************************************************************/
/* From Andrew's C package                                                  */

int int_random(int n) {
  static int FirstTime = TRUE;
  
  if ( FirstTime ){
    /* initialize the random number seed. */
    timeval tp;
    gettimeofday( &tp, NULL );
    srandom( (unsigned int) tp.tv_usec);
    FirstTime = FALSE;
  }

  if ( n > 2 )
    return( random() % n );
  else if ( n == 2 )
    return( ( (random() % 112) >= 56 ) ? 0 : 1 );
  else if ( n == 1 )
    return(0);
  else
    {
      printf("int_random(%d) ?\n",n);
      my_error("You called int_random(<=0)");
      return(0);
    }
}

float range_random(float lo, float hi) {
  int x1 = int_random(10000);
  int x2 = int_random(10000);
  float r = (((float) x1) + 10000.0 * ((float) x2))/(10000.0 * 10000.0);
  return( lo + (hi - lo) * r );
}

int very_random_int(int n) {
  int result = (int) range_random(0.0,(float)n);  /* rounds down */
  if ( result == n ) result = n-1;
  return(result);
}

void GetStampedName( char *name, char *outputName ) {
  char date[100],weekday[10],month[10],temp[10];
  int  day,hour,min,sec,year;
  FILE *dateFile;
  
  //system("date > ../date.log");        /* Put the date in a file */ /* done by a player */

  dateFile = fopen("../date.log","r");
  fscanf(dateFile,"%[^\n]",date);         /* Scan it in             */
  fclose(dateFile);
  
  sscanf(date,"%s %s %d %d:%d:%d %s %d",
	 weekday,month,&day,&hour,&min,&sec,temp,&year);
  sprintf(name,"%s-%s%d-%d:%d:%d.dat",outputName,month,day,hour,min,sec);
}
