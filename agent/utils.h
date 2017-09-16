/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : utils.h
 *
 *    AUTHOR     : Anton Ivanov, Sergey Serebyakov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: utils.h,v 2.3 2004/03/05 07:03:43 anton Exp $
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

/* utils.h
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

#ifndef _UTILS_
#define _UTILS_

#include "types.h"
#include <string>
#include <fstream>
#include <vector>
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////////
typedef float Value;
typedef float AngleRad;
typedef float AngleDeg;
/////////////////////////////////////////////////////////////////////////////////////////
#define my_stamp printf("%d:%d.%d ",Mem->MyNumber,Mem->CurrentTime.t,Mem->CurrentTime.s);
#define FLOAT_EPS .001
#define Mod(a,b) (a - (b)*(int)((a)/(b)))
#define Sign(x) ((x) >= 0 ? 1 : -1)
#define signf(x) ( ((x) > 0.0) ? 1.0 : -1.0)
#define Min(x,y) ((x) < (y) ? (x) : (y))
#define Max(x,y) ((x) > (y) ? (x) : (y))
#define MinMax(min, x, max) Min(Max((min),(x)), (max))

/////////////////////////////////////////////////////////////////////////////////////////
namespace statistica {
  class Statistic{
  public:
    Statistic();
    ~Statistic() {delete []calcTimes;delete []names;};
    void SaveStatistic();
    void UpdateStatistic();
    //special shortcut
    void UpdateHandleballStatistic(double time);
    void UpdateCalcTimes(int base, double time);
    //values
    int numberTM[11];
    int numberOP[11];
    double TM[11];
    double OP[11];
    int numberBall;
    double Ball;
    int effortNum;
    double effortSum;
	
    const int MAX_DEBUG_FUNCTIONS;		
    double* calcTimes;
    string* names;
    //added by Serg, things about handleball computations...
    double Handleball;
    int    numberHandleball;
  };
  extern Statistic stat;

  class TimeCounter {
  public:
    TimeCounter() {};
    ~TimeCounter() {};	
  public:
    void   Start() 	{ start=clock(); }
    void   Finish() { time = float(clock()-start)/CLOCKS_PER_SEC; }
    float  GetTime(){ return time; };
  private:
    clock_t start;
    float time;
  };
  extern TimeCounter tcounter;
}

class FuncCalcTime{
public:
  FuncCalcTime(const string& n):name(n){time.Start();}
  ~FuncCalcTime();
private:
  string name;
  statistica::TimeCounter time;
};

//use this class to evaluate avarage time of calculation for every 1000 cycles
//result wrights to stat*.* functions
//u must set MAX_DEBUG_FUNCTIONS to number of functions wich u want to debug
//param b in constructor - number of debug function: [0;MAX_DEBUG_FUNCTION-1]
//param n in constructor - comment of debug function (maybe name of function)
class FuncEvarTime{
public:
  FuncEvarTime(int b,string n):base(b){time.Start();statistica::stat.names[base]=n;}
  ~FuncEvarTime(){	
    time.Finish();
    statistica::stat.UpdateCalcTimes(base,time.GetTime());
  }
private:
  int base;
  statistica::TimeCounter time;
};

/////////////////////////////////////////////////////////////////////////////////////////
int dump_core(char*);
class Error {
public:
  Error();
  ~Error();
private:
  enum{
    ERROR_STRING_LENGTH=100
      };
};
void my_error(const char*);
void my_error(const char*,int);
void my_error(const char*,int,int);
void my_error(const char*,int,int,int);
void my_error(const char*,int,int,int,int);
void my_error(const char*,int,int,int,int,int);
void my_error(const char*,int,int,int,int,int,int);
void my_error(const char*,int,int,int,int,int,char,int);
void my_error(const char*,float);
void my_error(const char*,float,float);
void my_error(const char*,float,float,float);
void my_error(const char*,float,float,float,float);
void my_error(const char*,float,int);
void my_error(const char*,char*);
void my_error(const char*,char,int,int);
void my_error(const char*,char,int,float,float);
/////////////////////////////////////////////////////////////////////////////////////////
int closest_int( float x );
/////////////////////////////////////////////////////////////////////////////////////////
inline AngleDeg Rad2Deg(AngleRad x) { return x * 180 / M_PI; }
inline AngleRad Deg2Rad(AngleDeg x) { return x * M_PI / 180; }

inline float Cos(AngleDeg x) { return cos(Deg2Rad(x)); }
inline float Sin(AngleDeg x) { return sin(Deg2Rad(x)); }
inline float Tan(AngleDeg x) { return tan(Deg2Rad(x)); }

inline AngleDeg ACos(float x)           { return ((x) >= 1 ? 0 : ((x) <= -1 ? 180 : (Rad2Deg(acos(x))))); }
inline AngleDeg ASin(float x)           { return ((x) >= 1 ? 90 : ((x) <= -1 ? -90 : (Rad2Deg(asin(x))))); }
inline AngleDeg ATan(float x)           { return (Rad2Deg(atan(x))); }
inline AngleDeg ATan2(float x, float y) { return ((x == 0 && y == 0) ? 0 : (Rad2Deg(atan2(x,y)))); } 
/////////////////////////////////////////////////////////////////////////////////////////
double normalize( double a, double min, double max );
void NormalizeAngleDeg(int*);
void NormalizeAngleDeg(AngleDeg*);
void NormalizeAngleRad(AngleRad*);
AngleDeg GetNormalizeAngleDeg(AngleDeg);
AngleDeg GetDiff(AngleDeg angle1, AngleDeg angle2);
/////////////////////////////////////////////////////////////////////////////////////////
float GetDistance(float *x, float *y, float *a, float *b);

float int_pow(float x, int p);
inline int   Sqr(int x){ return x*x; }
inline float Sqr(float x) { return x*x; }
inline float Exp(float x, int y) { float a = 1; for (int i=0; i<y; i++) a*=x; return a; }

inline float SumInfGeomSeries(float first_term, float r)
{ return first_term / (1 - r); }
float SumGeomSeries(float first_term, float r, int n);
/* returns -1 on error */
float SolveForLengthGeomSeries(float first_term, float r, float sum);
float SolveForFirstTermGeomSeries(float r, int n, float sum);
inline float SolveForFirstTermInfGeomSeries(float r, float sum)
{ return sum * (1 - r); }

inline float Round(float x, int p=0) {
  x *= int_pow(10.0, -p);
  if (fmod(x, 1.0f) >= .5)
    return ceil(x) / int_pow(10.0, -p);
  else
    return floor(x) / int_pow(10.0, -p);
} 

extern const char char_for_num_array[16];
inline char char_for_num(int num)
{ return char_for_num_array[num]; }

/* returns a pointer to a static buffer, so be careful! */
char* repeat_char(char c, int n);

class Time {
public:
  int t; /* time from the server */
  int s; /* stopped clock cycles */

  Time(int vt = 0, int vs = 0) { t = vt; s = vs; }
  Time operator - (const int &a);
  int  operator - (const Time &a);
  Time operator + (const int &a);
  int  operator % (const int &a) { return (t+s)%a; }
  void operator -=(const int &a) { *this = *this - a; }
  void operator -=(const Time &a){ *this = *this - a; }
  void operator +=(const int &a) { *this = *this + a; }
  void operator ++()             { *this += 1; }
  void operator --()             { *this -= 1; }
  Time operator = (const int &a) { t = a; s = 0; return *this; }
  bool operator ==(const Time &a) { return (s == a.s) && (t == a.t); }
  bool operator ==(const int &a)  { return t == a; }
  bool operator !=(const Time &a) { return (s != a.s) || (t != a.t); }
  bool operator !=(const int &a)  { return t != a; }
  bool operator < (const Time &a) { return ( t < a.t ) || ( t == a.t && s < a.s ); }
  bool operator < (const int &a)  { return t < a; }
  bool operator <=(const Time &a) { return ( t < a.t ) || ( t == a.t && s <= a.s ); }
  bool operator <=(const int &a)  { return t <= a; }
  bool operator > (const Time &a) { return ( t > a.t ) || ( t == a.t && s > a.s ); }
  bool operator > (const int &a)  { return t > a; }
  bool operator >=(const Time &a) { return ( t > a.t ) || ( t == a.t && s >= a.s ); }
  bool operator >=(const int &a)  { return t >= a; }
  bool operator !() { return (s == 0) && (t == 0); }

  Bool CanISubtract(const Time &a);
};

extern double get_double(char **str_ptr);
extern double get_double(char *str);
extern float  get_float (char **str_ptr);
extern float  get_float (char *str);
extern int    get_int   (char **str_ptr);
extern int    get_int   (char *str);
extern void   get_word  (char **str_ptr);
extern void   get_next_word (char **str_ptr);
extern void   get_token  (char **str_ptr);
extern void   advance_to(char c, char **str_ptr);
extern void   advance_past_space(char **str_ptr);

extern int put_int(char *str, int num);
extern int put_float(char *str, float fnum, int precision);

extern void BubbleSort  (int length, int *elements, float *keys);
extern int  BinarySearch(int length, float *elements, float key);
extern void StrReplace  (char *str, char oldchar, char newchar);

extern int   int_random(int n);
extern float range_random(float lo, float hi);
extern int   very_random_int(int n);

extern float weighted_avg(float val1, float val2, float w1, float w2);

extern void GetStampedName( char *name, char *outputName );

#endif
