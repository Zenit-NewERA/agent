/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Formations.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.16 $
 *
 *    $Id: Formations.C,v 2.16 2004/08/29 14:07:21 anton Exp $
 *
 ************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "Formations.h"
#include "client.h"



#include <string>
#include <fstream>
using namespace std;
///////////////////////
Formations::bp *Formations::begin_pos;
int Formations::num_of_begin_pos;
string Formations::PlayerName;//name of player
FormationData Formations::formations[Formations::num_of_formations];
//для добавления новой формации надо:
// 1. Создать новое имя в перечеслении Ftype (файл Formations.h)
// 2. Увеличить значение переменной num_of_formations на 1 (файл Formations.h)
// 3. Добавить новую запись в структуру form_str (файл Formations.C)

Formations::Formation_Str Formations::form_str[Formations::num_of_formations]={
  {"[433",FT_433},
  {"[433_offense",FT_433_OFFENSE},
  {"[523_defense",FT_523_DEFENSE}
};
/////////////////////////////////////////////////////////////////////////
bool FormationLine::LoadData(istream& file){
  char c;
  file>>c>>from_x>>c>>base>>c>>attr>>c;
  return true;
}
//////////////////////////////////////////////////////////////////////////
FormationData::FormationData(){
  for(int i=0;i<11;i++)
    line_x[i]=line_y[i]=0;
  formation=FT_None;
}
//////////////////////////////////////////////////////////////////////////
FormationData::~FormationData(){
  for(int i=0;i<11;i++){
    if(line_x[i]) delete []line_x[i];
    if(line_y[i]) delete []line_y[i];
  }
}
//////////////////////////////////////////////////////////////////////////
bool FormationData::LoadData(ifstream& file,Ftype formation,int num_fracture_x,int num_fracture_y){
  this->formation=formation;
  num_x_lines=num_fracture_x+1;
  num_y_lines=num_fracture_y+1;
  string str;
  char val;
  int num;
  int i=0;
  while(i<11){
    getline(file,str);
    if(str[0]=='#'||str.size()<=1)
      continue;
    istringstream istr(str);
    istr>>num;
    if(num<1||num>11){
      cout<<"Error player number "<<num<<endl;

      return false;
    }
    int player=num-1;//set number as index
    line_x[player]=new FormationLine[num_x_lines];
    line_y[player]=new FormationLine[num_y_lines];
   
    istr>>val;
    types[player]=conv_char_to_ptype(val);
    istr>>val;
    sides[player]=conv_char_to_pside(val);
    for(int j=0;j<num_x_lines;j++)
      if(!line_x[player][j].LoadData(istr))
	return false;
    for(int j=0;j<num_y_lines;j++)
      if(!line_y[player][j].LoadData(istr))
	return false;
    istr>>min_x[player];
    istr>>max_x[player];
    i++;
  }
  for(i=0;i<10;i++)
    for(int j=0;j<10;j++)
      file>>activity_digraph[i][j];
  return true;
}
/////////////////////////////////////////////////////////////////////////
Ptype FormationData::conv_char_to_ptype(char c) const{
  Ptype p;
  switch(c){
  case 'G': p=PT_Goaltender;
    break;
  case 'D': p=PT_Defender;
    break;
  case 'M': p=PT_Midfielder;
    break;
  case 'F': p=PT_Forward;
    break;
  case 'S': p=PT_Sweeper;
    break;
  default:
    cerr<<"Not right format of player role: "<<c<<endl;
    exit(1);
  }
  return p;
}
///////////////////////////////////////////////////
Pside FormationData::conv_char_to_pside(char c) const{
  Pside s;
  switch(c){
  case 'C': s=PS_Center;
    break;
  case 'L': s=PS_Left;
    break;
  case 'R': s=PS_Right;
    break;
  default:
    cerr<<"Not right format of player side: "<<c<<endl;
    exit(1);
  }
  return s;
}
/////////////////////////////////////////////////////

Formations::~Formations(){

  if(begin_pos)
    delete []begin_pos;
}
/////////////////////////////////////////////////////////////////////////

Formations::Formations(){
  begin_pos=0;
  for(int i=0;i<11;i++)
    updateHomePos[i]=-1;
  InitBDPMatrix();
  //initialize current formation
  SetCurrentFormation(FT_433_OFFENSE);
  important_action=false;
}
////////////////////////////////////////////////////
int Formations::IsStrongOpponent(){//используется при выборе формации 523
  static const string strong_opp[]={
    "uva",
    "u_v_a",
    ""
  };
  if(Mem->TheirTeamName[0]=='\n')
    return 0;
  static int check=-1;
  if(check!=-1)
    return check;
  string name=Mem->TheirTeamName;
  for(unsigned int i=0;i<name.size();i++)
    name[i]=tolower(name[i]);
  for(int i=0;strong_opp[i]!="";i++){
    if(name.find(strong_opp[i])!=string::npos){
      check=1;
      return check;
    }
  }
  check=0;
  return check;
}
//////////////////////////////////////////////////////////////////////
bool Formations::IsOpponentGoalieGoOutOfPenaltyArea()
{
  static const string strong_opp[]={
    "tsingh",
    "oxsy",
    "mersad",
    ""
  };
  if(Mem->TheirTeamName[0]=='\n')
    return 0;
  static int check=-1;
  if(check!=-1)
    return check;
  string name=Mem->TheirTeamName;
  for(unsigned int i=0;i<name.size();i++)
    name[i]=tolower(name[i]);
  for(int i=0;strong_opp[i]!="";i++){
    if(name.find(strong_opp[i])!=string::npos){
      check=1;
      return check;
    }
  }
  check=0;
  return check;  
}
//////////////////////////////////////////////////////////////////////
bool Formations::IsOpponentUseTackle()
{
  return true;//change in Portugal
  
  static const string strong_opp[]={
    "tsingh",
    "brain",
    "uva",
    "u_v_a",
    "portugal",
    "we200",
    "helios",
    "oxsy",
    "humboldt",
    "mrb",
    "tokyotech",
    "mersad",
    ""
  };
  if(Mem->TheirTeamName[0]=='\n')
    return 0;
  static int check=-1;
  if(check!=-1)
    return check;
  string name=Mem->TheirTeamName;
  for(unsigned int i=0;i<name.size();i++)
    name[i]=tolower(name[i]);
  for(int i=0;strong_opp[i]!="";i++){
    if(name.find(strong_opp[i])!=string::npos){
      check=1;
      return check;
    }
  }
  check=0;
  return check;  
}
//////////////////////////////////////////////////////////////////////
bool Formations::IsOpponentActiveUseDribble()
{
  static const string strong_opp[]={
    "uva",
    "u_v_a",
    ""
  };
  if(Mem->TheirTeamName[0]=='\n')
    return 0;
  static int check=-1;
  if(check!=-1)
    return check;
  string name=Mem->TheirTeamName;
  for(unsigned int i=0;i<name.size();i++)
    name[i]=tolower(name[i]);
  for(int i=0;strong_opp[i]!="";i++){
    if(name.find(strong_opp[i])!=string::npos){
      check=1;
      return check;
    }
  }
  check=0;
  return check;  
}
//////////////////////////////////////////////////////////////////////
Ftype Formations::GetFormationByString(string str){
  for(int i=0;i<num_of_formations;i++){
    if(str==form_str[i].name){//find formation
      return form_str[i].type;
    }
  }
  return FT_None;
}
////////////////////////////////////////////////////
void Formations::LoadConfigFile(){
  string str;
  ifstream file(Mem->CP_formation_conf);
  if(!file){

    cerr<<"Not find file \""<<Mem->CP_formation_conf<<"\" !!!"<<endl;
    exit(1);
  }
  Ftype formation;
  int fracture_x,fracture_y;
  while(file>>str){
    if(str[0]=='#'){
      getline(file,str);
      continue;
    }
  
    if(str=="[Players]"){
      int i;
      int step=0;
      while(step<11){
	step++;
	file>>i;
	if(i==Mem->MyNumber){
	  file>>str>>PlayerName;//= name
	  break;
	}else
	  getline(file,str);//= name
      }
    }else
  	
      if(str=="[BeginPos]"){
	file>>num_of_begin_pos;
	begin_pos=new bp[num_of_begin_pos];
	for(int j=0;j<num_of_begin_pos;j++)
	  for(int i=0;i<11;i++){
	    file>>begin_pos[j][i][0];
	    file>>begin_pos[j][i][1];
	  }
      }else{
	formation=GetFormationByString(str);
	if(formation!=FT_None){
	  file.ignore(100,'=');
	  file>>fracture_x;
	  if(fracture_x<0||fracture_x>10){
	    cout<<"Error in loading formation "<<form_str[formation].name<<"]; wrong fracture_x:"<<fracture_x<<endl;
	    exit(1);
	  }
	  file.ignore(100,'=');
	  file>>fracture_y>>str;
	  if(fracture_y<0||fracture_y>10){
	    cout<<"Error in loading formation "<<form_str[formation].name<<"]; wrong fracture_y:"<<fracture_y<<endl;
	    exit(1);
	  }
	  if(!formations[formation].LoadData(file,formation,fracture_x,fracture_y)){
            cout<<"Error in loading formation "<<form_str[formation].name<<"]"<<endl;
            exit(1);
	  }
	}else
	  if(str=="[EndFile]")
	    break;
      }//and else
  }
  if(Mem->MyNumber==1)
    cout<<"Initialize file team.conf"<<endl;
}
///////////////////////////////////////////////////
void Formations::SetCurrentFormation(Ftype fm){
  if (fm<=FT_None||fm>num_of_formations)
    cerr<<"Wrong formation "<<fm;
  else
    current_formation=int(fm);
}
//////////////////////////////////////////////////////
Vector Formations::_GetHomePosition(int index,bool fixed_ball_pos,const Vector& ball_pos){
  Unum player=index+1;
  Vector ball=Vector(0,0);
  if(fixed_ball_pos){
    ball=ball_pos;
  }else{
    if(Mem->BallPositionValid()){
      ball=Mem->BallAbsolutePosition();//Mem->BallEndPosition(&num_cyc);
      if(!Mem->FieldRectangle.IsWithin(ball))
	ball=Mem->FieldRectangle.AdjustToWithin(ball);
//       if(Mem->TeammatePositionValid(player)&&fabs(ball.x-Mem->TeammateX(player))>20.0f)
// 	ball=GetBDP(ball);
    }
  }
 
  Vector pos;
  for(int i=formations[current_formation].num_x_lines-1;i>=0;i--){
    if(formations[current_formation].line_x[index][i].from_x<=ball.x){
      pos.x=formations[current_formation].line_x[index][i].base+
	formations[current_formation].line_x[index][i].attr*ball.x;
      break;
    }
  }
  for(int i=formations[current_formation].num_y_lines-1;i>=0;i--){
    if(formations[current_formation].line_y[index][i].from_x<=ball.x){
      pos.y=formations[current_formation].line_y[index][i].base+
	formations[current_formation].line_y[index][i].attr*ball.y;
      break;
    }
  }
  if(pos.x>formations[current_formation].max_x[index])
    pos.x=formations[current_formation].max_x[index];
  if(pos.x<formations[current_formation].min_x[index])
    pos.x=formations[current_formation].min_x[index];
  if(fabs(pos.y)>32.0f)
    pos.y=Sign(pos.y)*32.0f;
  if(GetPlayerType(player)==PT_Sweeper&&ball.x<pos.x)
    pos.x=ball.x;

  if(Mem->my_offside_line<=pos.x)
    pos.x=Mem->my_offside_line-1.0;
  if(Mem->TeammatePositionValid(player)&&Mem->TeammateDistanceTo(player,pos)<=1.5f+fabs(Mem->TeammateX(player)-ball.x)/10&&Mem->PlayMode==PM_Play_On){
    if(player==Mem->MyNumber) Mem->LogAction2(100,"_GetHomePosition: i`m close enarth, so return my pos");
    return Mem->TeammateAbsolutePosition(player);
  }
  return pos;
}
/////////////////////////////////////////////////////////////
Vector Formations::GetHomePosition(Unum player){
  if(player==Unum_Unknown){

    my_error("Not right number of player in GetHomePosition");
    return Vector(.0f,.0f);
  }
  if(updateHomePos[player-1]<Mem->CurrentTime){
    updateHomePos[player-1]=Mem->CurrentTime;
    HomePos[player-1]=_GetHomePosition(player-1);
  }
  return HomePos[player-1];
}
/////////////////////////////////////////////////////////////
Vector Formations::GetHomePosition(const Vector& ball_pos,Unum player){
  if(player==Unum_Unknown){
    my_error("Not right number of player in GetHomePosition");
    return Vector(.0f,.0f);
  }
  return _GetHomePosition(player-1,true,ball_pos);
}
/////////////////////////////////////////////////////////////
void Formations::InitBDPMatrix(){
  float width=68.0/NUM_WIDTH_BLOCKS;
  float height=105.0/NUM_HEIGHT_BLOCKS;
  for(int i=0;i<NUM_HEIGHT_BLOCKS;i++)
    for(int j=0;j<NUM_WIDTH_BLOCKS;j++)
      BDPMatrix[i][j]=Vector((i+1)*height-105.0/2,(j+1)*width-68.0/2);
}
/////////////////////////////////////////////////////////////
Vector Formations::GetBDP(const Vector& ball){
  static float width=Mem->SP_pitch_width/NUM_WIDTH_BLOCKS;
  static float height=Mem->SP_pitch_length/NUM_HEIGHT_BLOCKS;
  int j=(int)floor((ball.y+Mem->SP_pitch_width/2)/width);
  if(j==NUM_WIDTH_BLOCKS) j--;
  int i=(int)floor((ball.x+Mem->SP_pitch_length/2)/height);
  if(i==NUM_HEIGHT_BLOCKS) i--;
  //  Mem->LogAction6(10,"GetBDP:was (%.2f,%.2f), new (%.2f,%.2f)",ball.x,ball.y,BDPMatrix[i][j].x,BDPMatrix[i][j].y);
  return BDPMatrix[i][j];
}
//////////////////////////////////////////////////////////////////////
int Formations::IsActivityByFastest(Unum tm,Unum fastest) const
{
  if(tm<=0||tm>11||tm==Mem->OurGoalieNum||fastest<=0||fastest>11||fastest==Mem->OurGoalieNum)
    return 0;
  return formations[current_formation].activity_digraph[fastest-2][tm-2];
}
//////////////////////////////////////////////////////////////
Vector Formations::GetMyGoalieFreeKickHomePos(Unum player){
  if(GetPlayerType(player)==PT_Forward)
    return GetHomePosition(player);
  if(GetPlayerType(player)==PT_Midfielder)
    return Vector(-18.0f,GetHomePosition(Vector(-30.0f,.0f),player).y);
  //defense
  return GetHomePosition(Vector(-30.0f,.0f),player);
}
///////////////////////////////////////////////////////////////
Vector Formations::GetMyKickInHomePos(Unum player){
  Vector hp=GetHomePosition(player);
  if(GetPlayerType(player)==PT_Forward){
    if(hp.x>Mem->my_offside_line)
      hp.x=Mem->my_offside_line-1.0;
    return hp;
  }
  if(GetPlayerType(player)==PT_Midfielder){
    float y=hp.y+Sign(Mem->BallY())*10.0;
    if(fabs(y)>Mem->SP_pitch_width*0.93f)
      y*=Mem->SP_pitch_width*0.9f;
    return Vector(hp.x,y);
  }
  //defense
  return hp;
}
//////////////////////////////////////////////////////////////
Vector Formations::GetCornerKickHomePos(Unum player){
  Vector hp=GetHomePosition(player);
  Pside side;
  if(Mem->BallY()>0)
    side=PS_Right;
  else
    side=PS_Left;
  if(GetPlayerType(player)==PT_Forward){
    if(GetPlayerSide(player)==side)
      return Vector(hp.x,hp.y-Sign(Mem->BallY())*5.0);
    else
      return hp;
  }
  if(GetPlayerType(player)==PT_Midfielder){
    float y=hp.y+Sign(Mem->BallY())*10.0;
    if(fabs(y)>Mem->SP_pitch_width*0.93f)
      y*=Mem->SP_pitch_width*0.9f;
    return Vector(hp.x,y);
  }
  return Vector(hp.x+5.0,hp.y);
}
///////////////////////////////////////////////////////////////
Vector Formations::GetMyGoalieKickHomePos(Unum player){
  if(GetPlayerType(player)==PT_Forward)
    return GetHomePosition(player);
  if(GetPlayerType(player)==PT_Midfielder)
    return Vector(-25.0f,GetHomePosition(Vector(-40.0f,Mem->BallY()),player).y);
  //defense
  return Vector(-32.0f,GetHomePosition(Vector(-40.0f,Mem->BallY()>0.0f?10.0f:-10.0f),player).y);

}
/////////////////////////////////////////////////////////////
Vector Formations::GetMyFreeKickHomePos(Unum player){
  Vector home=GetHomePosition(player);
  if(home.x>=Mem->my_offside_line)
    home.x=Mem->my_offside_line-1.0;
  if(Mem->BallX()>30.0&&GetPlayerType(player)==PT_Forward&&fabs(home.y)+4.0f>=fabs(Mem->BallY()))
    home.y=Mem->BallY()-Sign(Mem->BallY())*8.0;
  return home;
}

/////////////////////////////////////////////////////////////
Vector Formations::GetTheirSetPlayHomePos(Unum player){
  if((GetPlayerType(player)==PT_Defender||GetPlayerType(player)==PT_Sweeper)&&Mem->BallX()<-30.0f){
    Vector hp(-37.0f,GetHomePosition(Vector(Mem->BallX(),0.0),player).y);
    if((hp-Mem->BallAbsolutePosition()).mod()<9.5f)
      hp.y-=Sign(hp.y)*(11-fabs(hp.y-Mem->BallY()));
    return hp;
  }
  return GetHomePosition(player);
}
/////////////////////////////////////////////////////////////
Vector Formations::GetTheirGoalieFreeKickHomePos(Unum player){
  return GetHomePosition(Vector(30.0f,0.0f),player);
}
//////////////////////////////////////////////////////////////
Vector Formations::GetTheirCornerKickHomePos(Unum player){
  if(GetPlayerType(player)==PT_Forward)
    return GetHomePosition(player);
  if(GetPlayerType(player)==PT_Midfielder)
    return Vector(-35.0f,GetHomePosition(Vector(0.0f,.0f),player).y);
  //defense
  return Vector(-43.0f,GetHomePosition(Mem->BallAbsolutePosition(),player).y);
}
//////////////////////////////////////////////////////////////
Vector Formations::GetTheirKickInHomePos(Unum player){
  Vector ball=Mem->BallAbsolutePosition();
  if(GetPlayerType(player)==PT_Forward)
    return GetHomePosition(player);
  if(ball.x<-40.0f){
    if(GetPlayerType(player)==PT_Midfielder)
      return Vector(-30.0,GetHomePosition(player).y);
    //defense
    return Vector(ball.x<-48.0?-48.0:ball.x,GetHomePosition(player).y);
  }
  if(GetPlayerType(player)==PT_Midfielder)
    return Vector(ball.x>0?GetHomePosition(player).x:ball.x,GetHomePosition(player).y);
  return Vector(ball.x>0?GetHomePosition(player).x:GetHomePosition(player).x-7.0f,GetHomePosition(player).y);


}
//////////////////////////////////////////////////////////////
float Formations::GetFormationDash(Unum tm)const{
  if(Mem->PlayMode!=PM_Play_On){
    if(GetPlayerType(tm)<=PT_Defender){
      if(tm==Mem->MyNumber&&Mem->MyStamina()<Mem->SP_stamina_max*.9f)
        return Mem->GetTeammateStaminaIncMax(tm)*0.8f;
      else
        return Mem->GetTeammateStaminaIncMax(tm);
    }
    return Mem->GetTeammateStaminaIncMax(tm);
  }
  return 100.0;
}

///////////////////////////////////////////////////////////////
Unum Formations::GetPlayerNumber(Iterator& iter,int PT_mask,int PS_mask) const{
  if(iter<0||iter>=11)
    return Unum_Unknown;
  for(Iterator index=iter;index<11;index++)
    if(PT_mask&formations[current_formation].types[index] && PS_mask&formations[current_formation].sides[index]){
      iter=index+1;
      return static_cast<Unum> (iter);
    }
  return Unum_Unknown;
}
