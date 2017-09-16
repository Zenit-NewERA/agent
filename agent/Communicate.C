/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Communicate.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.13 $
 *
 *    $Id: Communicate.C,v 2.13 2004/06/22 17:06:16 anton Exp $
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

#include "Communicate.h"
#include "behave.h"
#include "Handleball.h"
#include "SetPlay.h"
#include "Playposition.h"
#include "scenario.h"
#include <queue>
/////////////////////////////////////////////////////////////////////////////
char TransferXCoor::GetFirstByte()const{
  int	temp=int((val+Sign(val)*0.05)*10)+int(105.0/2)*10;
  return Communicate::code(temp/max_num);
}
char TransferXCoor::GetSecondByte()const{
  int	temp=int((val+Sign(val)*0.05)*10)+int(105.0/2)*10;
  return Communicate::code(temp%max_num);
}
void TransferXCoor::ConstructValue(char* msg)const{
  *p=(Communicate::decode(msg[0])*max_num+Communicate::decode(msg[1]))/10.0-int(105.0/2);
}
////////////////////////////////////////////////////////////////////////////////
char TransferYCoor::GetFirstByte()const{
  return Communicate::code(int(val+Sign(val)*0.5)+int(68.0/2));
}
void TransferYCoor::ConstructValue(char* msg)const{
  *p=Communicate::decode(msg[0])-68.0/2;
}
////////////////////////////////////////////////////////////////////////////////
char TransferConf::GetFirstByte()const{
  if(val<=0.5)
    return Communicate::code(0);
  else
    return Communicate::code(int((val-0.5)*100));
}
void TransferConf::ConstructValue(char* msg)const{
  *p=Communicate::decode(msg[0])/100.0+0.5;
  if(*p==0.5)
    *p=0.0;
}
////////////////////////////////////////////////////////////////////////////////
char TransferVel::GetFirstByte()const{
  return Communicate::code(int((val+3.0)*100)/max_num);
}
char TransferVel::GetSecondByte()const{
  return Communicate::code(int((val+3.0)*100)%max_num);
}
void TransferVel::ConstructValue(char* msg)const{
  *p=(Communicate::decode(msg[0])*max_num+Communicate::decode(msg[1]))/100.0-3.0;
}
////////////////////////////////////////////////////////////////////////////////////
Msg& Msg::operator<<(const TransferObject& out){
  if(out.GetByteNum()==1){
    msg[0]=out.GetFirstByte();
    msg++;
    return *this;
  }
  msg[0]=out.GetFirstByte();
  msg[1]=out.GetSecondByte();
  msg+=2;
  return *this;
}
////////////////////////////////////////////////////////////////////////////////////
Msg& Msg::operator>>(const TransferObject& out){
  out.ConstructValue(msg);
  msg+=out.GetByteNum();
  return *this;
}
/////////////////////////////////////////////////////////////////////////////////////////////
Communicate::Communicate(){
  LastSayingTime=LastHearTime=update_opp_map=-1;
  player=Unum_Unknown;
  askPlayer=Unum_Unknown;
  playerThatAskQuestion=have_to_say_now=Unum_Unknown;
  mustAnswer=mustAsk=false;
  HearBallPos=HearBallVel=0;
  HearBallPosConf=HearBallVelConf=0.0f;
}
///////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::InitSayBuffer(void){
  say_buffer="(say \"";//6
  int s=0;
  while(msg[s]!=0&&s<10){
    say_buffer+=msg[s];
    s++;
  }
  say_buffer+="\")";
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::Initialize(void){
  for(int i=0;i<SP_say_msg_size+9;i++)
    say_buffer+=char(0);
  for(int i=0;i<SP_say_msg_size;i++){
    msg+=char(0);
    say_now_buffer+=char(0);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SayNow(char* buf){
  have_to_say_now=true;
  for(int i=0;i<10;i++)
    say_now_buffer[i]=buf[i];
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SayNow(aType type, Unum p){
  have_to_say_now=true;
  say_now_buffer[0]=code(ST_ask);
  say_now_buffer[1]=code(type);
  say_now_buffer[2]=code(int(p)+11);
  for(int i=3;i<10;i++)
    say_now_buffer[i]=code(0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SayNow(sayType saytype, Unum teammate) {
  have_to_say_now=true;
  switch(saytype) {
  case ST_pass_decision:
    //actions.GetPassMessage(const_cast <char*> (say_now_buffer.c_str()), ST_pass_decision, teammate);
    Mem->LogAction3(10,"comminucate::have said to %.0f about pass decision", float(teammate));
    break;
  case ST_pass_intention:
    //actions.GetPassMessage(const_cast <char*> (say_now_buffer.c_str()), ST_pass_intention, teammate);
    Mem->LogAction3(10,"comminucate::have said to %.0f about pass intention", float(teammate));
    break;
  default:
    my_error("say now (say type) unknown say type %.0f", float(saytype) );
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
Unum Communicate::GetAttentionFromTmSet()
{
  float min_conf=100.0f;
  float min_dist=100.0f;
  Unum opt_tm=Unum_Unknown;
  for(set<int>::const_iterator i=tm_set.begin();i!=tm_set.end();i++){
    if(*i==Mem->MyNumber||(Mem->TeammatePositionValid(*i)&&Mem->DistanceTo(Mem->TeammateAbsolutePosition(*i))>Mem->SP_audio_cut_dist))
      continue;
    if(Mem->TeammatePositionValid(*i)<min_conf){
      min_conf=Mem->TeammatePositionValid(*i);
      min_dist=min_conf>0.0f?Mem->DistanceTo(Mem->TeammateAbsolutePosition(*i)):0.0f;
      opt_tm=*i;
      continue;
    }
    if(Mem->TeammatePositionValid(*i)==min_conf&&min_conf>0.0f&&min_dist>Mem->DistanceTo(Mem->TeammateAbsolutePosition(*i))){
      min_dist=Mem->DistanceTo(Mem->TeammateAbsolutePosition(*i));
      opt_tm=*i;
      continue;
    }
  }
  if(opt_tm!=Unum_Unknown)
    Mem->LogAction5(100,"GetAttentionFromTmSet: setlect attention to %.0f (conf=%.2f; dist=%.2f)",
		    float(opt_tm),min_conf,min_dist);
  return opt_tm;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::CorrectAttentionTo(void){
  if((!Mem->CanSeePointWithNeck(Mem->BallAbsolutePosition())||(Mem->BallPositionValid()<1.0f&&Mem->ViewWidth==VW_Narrow)||
      (Mem->BallPositionValid()<0.9f&&Mem->ViewWidth==VW_Normal))&&Mem->DistanceTo(Vector(-45.0f,0.0))<=Mem->SP_audio_cut_dist&&!Mem->CP_goalie){
    Mem->LogAction2(30,"CorrectAttentionTo: i not see ball, so try listen goalie");
    Formations::Iterator iter=Pos.begin();
    Unum goalie=Pos.GetPlayerNumber(iter,PT_Goaltender);
    if(goalie!=Unum_Unknown){
      attentionto(goalie);
      return;
    }	
  }
  if((Pos.GetMyType()==PT_Defender||Pos.GetMyType()==PT_Sweeper)&&
     Mem->PlayMode==PM_Play_On/*&&!(Pos.FastestTm()==Mem->MyNumber&&Pos.OppCycles()>=Pos.TmCycles())*/){
    Unum to=Pos.GetDefenseAttention();
    if(to!=Unum_Unknown&&to!=attention_to_player){
      Mem->LogAction3(100,"CorrectAttentionTo: set defense attention to %d",to);
      attentionto(to);
    }
    if(to!=Unum_Unknown)
      return;
  }
  if(throughPass.ChangeAttantionInFroughPassToTm()!=Unum_Unknown){
    Mem->LogAction3(100,"AttentionTo: change attention to tm %d in through pass",throughPass.ChangeAttantionInFroughPassToTm());
    attentionto(throughPass.ChangeAttantionInFroughPassToTm());
    return;
  }
  if(!MyConf()||!BallPositionValid()||
     (MyInterceptionAble()&&FastestTeammateToBall()==MyNumber)||(Pos.IsDefense()&&Mem->BallX()<0.0f)){
    //    if(attention_to_player!=Unum_Unknown)
      attentionto(GetAttentionFromTmSet());
    return;
  }
  for(int i=1;i<=SP_team_size;i++){
    if(i!=MyNumber&&TeammatePositionValid(i)
       &&((TeammateInterceptionAble(i)&&FastestTeammateToBall()==i)||Mem->BallKickableForTeammate(i))
       &&DistanceTo(TeammateAbsolutePosition(i))<SP_audio_cut_dist){
      if(attention_to_player!=Unum(i)) {
	attentionto(Unum(i));
      }
      return;
    }
  }
  //  if(attention_to_player!=Unum_Unknown)
  attentionto(GetAttentionFromTmSet());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
Communicate::GameType Communicate::GetGameType(int& mask){
  Vector ball=BallAbsolutePosition();
  if(ball.x>30.0){
    mask=PT_Forward;
    return GT_zone4;
  }
  if(ball.x>-10.0){
    mask=PT_Forward|PT_Midfielder;
    return GT_zone3;
  }
  if(ball.x>-35.0||(Mem->PlayMode!=PM_Play_On && ball.x<0.0)){//hack
    mask=PT_Sweeper|PT_Defender|PT_Midfielder;
    return GT_zone2;
  }
  //ball.x<=-35.0
  mask=PT_Sweeper|PT_Defender;
  return GT_zone1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SetCommonTmSet()
{
  int mask;
  GetGameType(mask);
  if(Pos.IsOffense()&&Pos.FastestTm()!=Unum_Unknown)
    tm_set.insert(Pos.FastestTm());
  Formations::Iterator iter=Pos.begin();
  while(iter!=Pos.end()){
    Unum tm=Pos.GetPlayerNumber(iter,mask);
    if(tm==Unum_Unknown)
      break;
    tm_set.insert(tm);
  }  
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SetScenarioTmSet()
{
  if(!Scenarios.IsScenarioGoing())
    return;
  for(int i=1;i<=Mem->SP_team_size;i++){
    if(Scenarios.GetTmNumberInScenario(i)!=0)
      tm_set.insert(i);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Communicate::IsOurTimeToSay(int t){
  t%=tm_set.size();//0 - tm_set.size()-1
  ostringstream str;
  str<<"Teammates in communicate set:";
  for(set<int>::const_iterator i=tm_set.begin();i!=tm_set.end();i++)
    str<<*i<<"; ";
  str<<"Scenario_going="<<Scenarios.IsScenarioGoing()<<". t="<<t;
  Mem->LogAction3(50,"%s",const_cast<char*>(str.str().c_str()));
//   if(Mem->BallX()<0&&Pos.IsDefense()){
//     int num=0;
//     for(set<int>::const_iterator i=tm_set.begin();i!=tm_set.end();i++,num++){
//       if(*i==MyNumber&&t==num)
// 	return true;
//     }
//     return false;
//   }
  
  if(tm_set.find(Mem->MyNumber)!=tm_set.end())
    return true;
  return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::SayThatOpponentKickBall(){
  static Vector LastBallVel;
  static Time UpdateTime=0;
  static bool BallKickableForOpponent;
  if((CurrentTime-UpdateTime)<=2){
    bool was_kick=(LastBallVel-Mem->BallAbsoluteVelocity()/int_pow(SP_ball_decay,CurrentTime-UpdateTime)).mod()>.1;
    bool opp_kick=Mem->OpponentWithBall(-0.3f)!=Unum_Unknown&&Mem->TeammateWithBall(-0.1)==Unum_Unknown;
		
    //~ Mem->LogAction6(10,"SayThatOpponentKickBall: was_kick =%.0f; opp_kick=%.0f; opp=%.0f; dist=%.3f",
    //~ float(was_kick),float(opp_kick),(float)Mem->OpponentWithBall(-0.3f),
    //~ Mem->OpponentWithBall(-0.3f)==Unum_Unknown?1000.0f:Mem->OpponentDistanceToBall(Mem->OpponentWithBall(-0.3f)));
    //~ Mem->LogAction6(10,"SayThatOpponentKickBall: LastBallVel=(%.2f;%.2f); time %.0f; was kickable %.0f",
    //~ LastBallVel.x,LastBallVel.y,(float)UpdateTime.t,(float)BallKickableForOpponent);
		
    if(Mem->BallVelocityValid()>=0.95f&&was_kick&&(opp_kick||BallKickableForOpponent)){
      char msg[10]={0,0,0,0,0,0,0,0,0,0};
      Msg m(msg);
      if(opp_kick){
	Mem->LogAction2(10,"Recognize, that opponent stop ball; say about it");
	m<<char(ST_opponent_have_ball);
      }else{
	Mem->LogAction2(10,"Recognize, that opponent kick ball; say about it");
	m<<char(ST_opponent_kick_ball);
      }
      AddBallPos(m);
      AddBallVel(m);
      SayNow(msg);
    }
  }
  if(Mem->BallVelocityValid()==1.0f){
    UpdateTime=CurrentTime;
    LastBallVel=Mem->BallAbsoluteVelocity();
    if(Mem->OpponentWithBall(-0.3f)!=Unum_Unknown)
      BallKickableForOpponent=true;
    else
      BallKickableForOpponent=false;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Communicate::HaveSomethingToSay(void){
  int st;//type of send massage
  if(!CP_communicate)
    return false;//communicate is disable
  int t=CurrentTime.t;
  if(t==0||ClockStopped||(LastSayingTime.t-t)>=0)
    return false;
  
  tm_set.clear();
  LastSayingTime=CurrentTime;

  SetCommonTmSet();
  SetScenarioTmSet();

  CorrectAttentionTo();
  UpdateOppMap();
  //not good tested now
  //	SayThatOpponentKickBall();//if say then set have_to_say_now
	
  if(have_to_say_now){
    have_to_say_now=false;
    msg=say_now_buffer;
    Mem->LogAction4(100,"SayNow:1 byte - %d, 2 byte - %d",int(msg[0]),int(msg[1]));
    InitSayBuffer();
    return true;
  }
  
  if(!IsOurTimeToSay(t))
    return false;

  if(Scenarios.IsScenarioGoing()){
    if(Scenarios.SayInScenario(msg)){
      InitSayBuffer();
      return true;
    }
  }
  st=SelectTypeOfMessage();
  msg=code(st);
  char* from=const_cast <char*> (msg.c_str())+1;
  FillMsgBody(from,(sayType)st);
  InitSayBuffer();
  msg.clear();
  return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int Communicate::MustAnswer(){
  int ret=-1;
  if(mustAnswer&&MyConf()){
    mustAnswer=false;
    if(answerT==AT_ball_pos_and_vel&&BallPositionValid()&&BallVelocityValid())
      ret=ST_ball_and_conf_ballvel_and_conf;
    else
      if(answerT==AT_ball_pos&&BallPositionValid()){
	ret=ST_mypos_and_conf_ball_and_conf;
      }
      else
	if(answerT==AT_my_pos||answerT==AT_my_pos_ball_pos){
	  if((BallVelocityValid()&&BallMoving())||!BallVelocityValid())
	    ret=ST_mypos_and_conf_ball_and_conf;
	  else
	    if(BallVelocityValid())
	      ret=ST_ball_stop_mypos_and_conf_ball_and_conf;
	}
	else
	  if(answerT==AT_player_pos&&PlayerPositionValid(player>0?MySide:TheirSide,abs(player)))
	    ret=ST_mypos_and_conf_player_and_conf;
	  else
	    if(answerT==AT_ball_pos_player_pos&&(PlayerPositionValid(player>0?MySide:TheirSide,abs(player))&&BallPositionValid()))
	      ret=ST_player_and_conf_ball_and_conf;
	    else
	      if(answerT==AT_most_danger_opponent_pos_to_pass&&TeammatePositionValid(player)&&TeammatePositionValid(playerThatAskQuestion)){
		Vector targetPoint=TeammateAbsolutePosition(playerThatAskQuestion);
		if(BallPositionValid()&&TeammateWithBall()==Unum_Unknown){
		  for(int i=SP_team_size;i>=1;i--){
		    if(TeammatePositionValid(i)&&i==playerThatAskQuestion&&TeammateInterceptionAble(i)&&FastestTeammateToBall()==i){
		      targetPoint=TeammateInterceptionPoint(i);
		      break;
		    }
		  }
		}
		Unum opp=Unum_Unknown;
		actions.PassFromPoint2Point(targetPoint,TeammateAbsolutePosition(player),opp);
		if(opp==Unum_Unknown){
		  opp=FastestOpponentToBall();
		  if(opp==Unum_Unknown)
		    return  ST_mypos_and_conf_ball_and_conf;
		}
		player=-opp;
		ret=ST_player_and_conf_ball_and_conf;
	      }
  }
  return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::UpdateOppMap(){
  int num=CurrentTime-update_opp_map;
  typedef map<Unum,float>::iterator I;
  update_opp_map=CurrentTime;
  for(int i=0;i<num;i++){
    for(I p=opp_map.begin();p!=opp_map.end();p++)
      p->second*=CP_player_conf_decay;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
struct OppConf{
  Unum opp;
  float conf;
  OppConf(Unum o,float c){conf=c;opp=o;}
  bool operator< (const OppConf& x)const{return conf<x.conf;}
};
//********
int Communicate::SelectOpponents(Unum* opp){
  const int maximum=2;
  int num=0;
  Unum danger=Unum_Unknown;
  actions.PassFromPoint2Point(BallAbsolutePosition(),MyPos(),danger);
  if(Pos.FastestTm()!=Mem->MyNumber&&Pos.IsOffense()&&danger!=Unum_Unknown&&
     (opp_map.find(danger)==opp_map.end()||(opp_map.find(danger)!=opp_map.end()&&
					    opp_map[danger]<OpponentPositionValid(danger)))){
    opp[num]=danger;
    opp_map[danger]=OpponentPositionValid(danger);
    num++;
  }
  if(Mem->my_offside_opp!=Unum_Unknown)
    danger=Mem->my_offside_opp;
  else
    danger=Pos.FastestOpp();
  if(danger!=Unum_Unknown&&(opp_map.find(danger)==opp_map.end()||
			    (opp_map.find(danger)!=opp_map.end()&&opp_map[danger]<OpponentPositionValid(danger)))){
    opp[num]=danger;
    opp_map[danger]=OpponentPositionValid(danger);
    num++;
  }
  priority_queue<OppConf> pq;
  for(int i=1;i<=11;i++){
    OppConf conf(i,OpponentPositionValid(i));
    pq.push(conf);
  }
  while(1){
    if(num==maximum||pq.empty())
      break;
    Unum i=pq.top().opp;
    if(OpponentPositionValid(i)>0.9f&&Mem->OpponentDistanceToBall(i)<40.0f&&
       (opp_map.find(i)==opp_map.end()||(opp_map.find(i)!=opp_map.end()&&opp_map[i]<OpponentPositionValid(i)))){
      opp[num]=i;
      opp_map[i]=OpponentPositionValid(i);
      num++;
    }
    pq.pop();
  }
  LogAction7(100,"SelectOpponents:: num = %.0f; opp1 = %.0f (conf %.2f); opp2 = %.0f (conf %.2f)",float(num),float(opp[0]),
	     opp[0]!=Unum_Unknown?OpponentPositionValid(opp[0]):0.0f,float(opp[1]),
	     opp[1]!=Unum_Unknown?OpponentPositionValid(opp[1]):0.0f);
  return num;
}
//////////////////////////////////////////////////////////////////////
int Communicate::SelectDefenseOppponentsToSay(Unum* opp)
{
  const int maximum=2;
  int num=0;
  if(Mem->BallX()>-30.0f)
    return num;
  Unum danger=Pos.FastestOpp();
  if(danger!=Unum_Unknown&&(opp_map.find(danger)==opp_map.end()||
			    (opp_map.find(danger)!=opp_map.end()&&opp_map[danger]<OpponentPositionValid(danger)))){
    opp[num]=danger;
    opp_map[danger]=OpponentPositionValid(danger);
    num++;
  }
  priority_queue<OppConf> pq;
  for(int i=1;i<=11;i++){
    OppConf conf(i,OpponentPositionValid(i));
    if(!Mem->OpponentPositionValid(i)||Mem->OpponentX(i)>-20.0f)
      continue;
    pq.push(conf);
  }
  while(1){
    if(num==maximum||pq.empty())
      break;
    Unum i=pq.top().opp;
    if(OpponentPositionValid(i)>0.9f&&Mem->OpponentDistanceToBall(i)<40.0f&&
       (opp_map.find(i)==opp_map.end()||(opp_map.find(i)!=opp_map.end()&&opp_map[i]<OpponentPositionValid(i)))){
      opp[num]=i;
      opp_map[i]=OpponentPositionValid(i);
      num++;
    }
    pq.pop();
  }
  LogAction7(100,"SelectDefenseOppponentsToSay:: num = %.0f; opp1 = %.0f (conf %.2f); opp2 = %.0f (conf %.2f)",float(num),float(opp[0]),
	     opp[0]!=Unum_Unknown?OpponentPositionValid(opp[0]):0.0f,float(opp[1]),
	     opp[1]!=Unum_Unknown?OpponentPositionValid(opp[1]):0.0f);
  return num;
  
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int Communicate::SelectTypeOfMessage(void){
  int ret=-1;
  if(mustAsk){
    ret=ST_ask;
    mustAsk=false;
    return ret;
  }
  if(!MyConf())
    return ST_none;
  ret=MustAnswer();
  if(ret!=-1)
    return ret;
	
  if(!BallPositionValid()){
    float conf=0.0;
    Unum send=Unum_Unknown;
    for(int i=SP_team_size;i>=1;i--){
      if(conf<OpponentPositionValid(i)){
	conf=OpponentPositionValid(i);
	send=i;
      }
    }
    if(conf>0.0){
      player=-send;
      return ST_mypos_and_conf_player_and_conf;
    }
    return ST_mypos_and_conf;
  }

  if(PlayMode==PM_My_PenaltyTaken&&TheirGoalieNum!=Unum_Unknown&&OpponentPositionValid(TheirGoalieNum)){
    player=-TheirGoalieNum;
    return ST_mypos_and_conf_player_and_conf;
  }
  if(PlayMode==PM_Their_PenaltyTaken){
    return ST_ball_and_conf_ballvel_and_conf;
  }
  Unum opp[2];
  int num=SelectDefenseOppponentsToSay(opp);
  if(num==0)
    num=SelectOpponents(opp);
  int mask;

  if(BallKickable()){
    if(num==0)
      player=Mem->MyNumber;
    else
      player=-opp[0];
    return ST_kickable_and_player_and_conf;
  }
     
      
  //send information about ball
  float new_pos_conf=HearBallPosConf*(Mem->CurrentTime-HearBallPos)>20?0.0f:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime-HearBallPos);
  float new_vel_conf=HearBallVelConf*(Mem->CurrentTime-HearBallVel)>20?0.0f:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime-HearBallVel);
  bool say_about_ball=new_pos_conf<Mem->BallPositionValid()||new_vel_conf<Mem->BallVelocityValid();
  Unum tm=Pos.FastestTm();
  if(tm!=Unum_Unknown&&say_about_ball&&Mem->BallPositionValid()>0.8f&&Mem->BallVelocityValid()>0.8f
     &&Mem->TeammateWithBall()==Unum_Unknown&&Mem->BallX()>-30.0f)
    return ST_fastest_tm1+tm-1; 

  if(num==0)
    return ST_mypos_and_conf_ball_and_conf;
  else{//if(num==1||GetGameType(mask)==GT_zone4||GetGameType(mask)==GT_zone3||Mem->PlayMode!=PM_Play_On){//GT_zone4 -  in opp penalty area
    player=-opp[0];
    return ST_mypos_and_conf_player_and_conf;
  }
  //num==2
  player=-opp[1];
  return ST_opp1_player_and_conf+opp[0]-1;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::ParsePlayerSound(char* msg,Time t){
  if(msg[1]=='s'){//msg from  myself
    return;
  }
  int ang;
  Unum  from;
  if(isdigit(msg[1])||msg[1]=='-'){
    ang=get_int(&msg);
    msg++;//space
  }
  if(!strncmp(msg,"opp",3)){//msg from opponent
    msg+=4;//opp+space
    ParseOppMessage(msg,t);
    return;
  }else
    if(!strncmp(msg,"our",3)){
      msg+=4;//our+space
      from=(Unum)get_int(&msg);
      msg++;//space
      if(CurrentTime==LastHearTime){
	LogAction2(100,"MyError: hear more then one message in this circle!! lost one or more msg");
	return;
      }
      LastHearTime=CurrentTime;
      ParseOurMessage(msg,from,t);
      return;
    }else
      my_error("don`t know format of message");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::ParseOurMessage(char* msg,Unum from,Time t){
  msg++;//"
  sayType st=(sayType)decode(msg[0]);
  msg++;//say type
  if( st==ST_pass_decision || st==ST_pass_intention ) {
    //actions.ParsePassMessage(msg,st,from,t);
    return;
  }
  ParseMsgBody(msg,st,from,t);	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::ParseOppMessage(char* msg,Time t){
  //to avoid warnings while compilation
  if(t.t==-100) Mem->LogAction3(10,"Got opp msg: %.s ",msg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::UpdateHearBallPosVel(float PosConf,float VelConf,const Time& t){
  //log(0.5)/log(Mem->CP_ball_conf_decay)=6.57 - so use round value 6

  float new_pos_conf=HearBallPosConf*(Mem->CurrentTime-HearBallPos>6?0:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime-HearBallPos));
  float new_vel_conf=HearBallVelConf*(Mem->CurrentTime-HearBallVel>6?0:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime-HearBallVel));
  if(PosConf>new_pos_conf){
    HearBallPosConf=PosConf;
    HearBallPos=t;
  }
  if(VelConf>new_vel_conf){
    HearBallVelConf=VelConf;
    HearBallVel=t;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector Communicate::GetFrom(Unum from){
  if(TeammatePositionValid(from))
    return TeammateAbsolutePosition(from);
  else
    return Vector(-100,-100);
}
////////////////////////////
void Communicate::GetMyPos(Msg& m,Unum from,const Time& t){
  float myx,myy,myconf;
  m>>TransferXCoor(&myx)>>TransferYCoor(&myy)>>TransferConf(&myconf);
  if(myconf>0.0){
    LogAction6(100,"Recieved (send tm): player %.0f, playerx %.2f, playery %.2f, playerconf  %.2f",(float)from,myx,myy,myconf);
    if(mustAnswer&&player==from&&(answerT==AT_player_pos||answerT==AT_my_pos)&&myconf>=TeammatePositionValid(from))
      mustAnswer=false;      		
    HearPlayer(MySide,from,myx,myy,CP_player_conf_decay*myconf,0.0,t);
  }
}
///////////////////////
Unum Communicate::GetPlayerPos(Msg& m,Unum from,const Time& t,Unum about){
  float playerx,playery,playerconf;
  char temp,side;
  Unum Player;
  m>>TransferXCoor(&playerx)>>TransferYCoor(&playery)>>TransferConf(&playerconf);
  if(about==Unum_Unknown){
    m>>temp;
    Player=temp-11;
  }else
    Player=about;
  if(Player<0){
    side=TheirSide;
    Player=-Player;
  }
  else
    side=MySide;
  if(playerconf>0){
    Vector from_pos=GetFrom(from);
    LogAction6(100,"Recieved: player %.0f, playerx %.2f, playery %.2f, playerconf  %.2f",
	       side==MySide?float(Player):float(-Player),playerx,playery,playerconf);
    if(side==MySide&&Player==MyNumber&&MyConf())
      return MyNumber;//i not need update myself
    if(side==TheirSide){
      if(opp_map.find(Player)!=opp_map.end()){
        if(opp_map[Player]<CP_player_conf_decay*playerconf)
          opp_map[Player]=playerconf;
      }else
        opp_map[Player]=playerconf;
    }
    if(mustAnswer&&player==(side==MySide?1:-1)*Player&&(answerT==AT_player_pos||answerT==AT_my_pos)&&playerconf>=PlayerPositionValid(side,Player))
      mustAnswer=false;
    if(from_pos.x!=-100)
      HearPlayer(side,Player,playerx,playery,CP_player_conf_decay*playerconf,from_pos.dist(Vector(playerx,playery)),t);
    else
      HearPlayer(side,Player,playerx,playery,CP_player_conf_decay*playerconf,10.0,t);//10.0 - random ;)
    return side==MySide?Player:-Player;
  }else
    return Unum_Unknown;
}
///////////////////////////
void Communicate::GetBallPos(Msg& m,Unum from,const Time& t){
  float ballx,bally,ballconf;
  m>>TransferXCoor(&ballx)>>TransferYCoor(&bally)>>TransferConf(&ballconf);
  if(ballconf>0){
    LogAction5(100,"Recieved:ballx %.2f,bally %.2f,ballconf %.2f",ballx,bally,ballconf);
    if(mustAnswer&&answerT==AT_ball_pos&&ballconf>=BallPositionValid())
      mustAnswer=false;
    Vector from_pos=GetFrom(from);
    UpdateHearBallPosVel(CP_ball_conf_decay*ballconf,0.0f,t);      		
    if(from_pos.x!=-100)
      HearBall(ballx,bally,CP_ball_conf_decay*ballconf,from_pos.dist(Vector(ballx,bally)),t);
    else
      HearBall(ballx,bally,CP_ball_conf_decay*ballconf,0.0,t);
  }
}
//////////////////////////
void Communicate::GetBallPosAndVel(Msg& m,Unum from,const Time& t,bool know_vel,const Vector& vel,float conf){
  float ballx,bally,ballconf,ballvelx,ballvely,ballvelconf;
  m>>TransferXCoor(&ballx)>>TransferYCoor(&bally)>>TransferConf(&ballconf);
  if(know_vel){
    ballvelx=vel.x;ballvely=vel.y;
    if(conf==-1.0f)
      m>>TransferConf(&ballvelconf);
    else
      ballvelconf=conf;
  }else{
    m>>TransferVel(&ballvelx)>>TransferVel(&ballvely)>>TransferConf(&ballvelconf);
  }
  if(ballconf>0){
    Vector from_pos=GetFrom(from);
    LogAction5(100,"Recieved:ballx %.2f,bally %.2f,ballconf %.2f",ballx,bally,ballconf);

    if( !(BallPositionValid()>=0.93f&&FP_goalie_number==MyNumber) )  {
      if(from_pos.x!=-100)
	HearBall(ballx,bally,CP_ball_conf_decay*ballconf,from_pos.dist(Vector(ballx,bally)),t);
      else
	HearBall(ballx,bally,CP_ball_conf_decay*ballconf,0.0,t);
    }

    Unum p=Mem->PlayerWithBall();
    Vector bpos(ballx,bally),bvel(ballvelx,ballvely);
    if(p==Unum_Unknown&&ballconf<ballvelconf&&bvel==Vector(0,0))//hack
      ballvelconf=0;
    if(mustAnswer&&answerT==AT_ball_pos&&ballconf>=BallPositionValid())
      mustAnswer=false;
    UpdateHearBallPosVel(CP_ball_conf_decay*ballconf,CP_ball_conf_decay*ballvelconf,t);      		
    if(ballvelconf>0){
      LogAction5(100,"Recieved:ballvelx %.2f,ballvely %.2f,ballvelconf %.2f",ballvelx,ballvely,ballvelconf);

      if( !(BallPositionValid()>=0.93&&FP_goalie_number==MyNumber) )  {
	if(from_pos.x!=-100)
	  HearBall(bpos.x,bpos.y,CP_ball_conf_decay*ballconf,bvel.x,bvel.y,CP_ball_conf_decay*ballvelconf,from_pos.dist(Vector(ballx,bally)),t);
	else
	  HearBall(bpos.x,bpos.y,CP_ball_conf_decay*ballconf,bvel.x,bvel.y,CP_ball_conf_decay*ballvelconf,0.0,t);
      }

    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::ParseMsgBody(char* msg,sayType st,Unum from,Time t){
  char temp;
  LogAction4(100,"Recieved msg from: %.0f(type: %.0f)",(float)from,(float)st);
  Msg m(msg);
  Unum recieved_player;
  
  if(Pos.RecievedDefenseCommunicate(st,m,from,t))
    return;
	
  if(st>=ST_fastest_tm1&&st<=ST_fastest_tm11){
    Msg work(msg);
    float x,y,bconf,vconf;
    work>>TransferXCoor(&x)>>TransferYCoor(&y)>>TransferConf(&bconf);
    work>>TransferVel(&vconf)>>TransferVel(&vconf)>>TransferConf(&vconf);
    if(!Mem->CP_goalie||(Mem->CP_goalie&&Mem->BallPositionValid()<0.95f)){
      Pos.UpdateFastestTmToBall(st-ST_fastest_tm1+1,(GetFrom(from)-Vector(x,y)).mod(),
				bconf*CP_ball_conf_decay,vconf*CP_ball_conf_decay);
      GetBallPosAndVel(m,from,t);
    }
    return;
  }
  if(st>=ST_opp1_player_and_conf&&st<=ST_opp11_player_and_conf){
    GetPlayerPos(m,from,t,-(st-int(ST_opp1_player_and_conf)+1));
    GetPlayerPos(m,from,t);
    return;
  }
  if(st>=ST_tm1_player_and_conf&&st<=ST_tm11_player_and_conf){
    GetPlayerPos(m,from,t,st-int(ST_tm1_player_and_conf)+1);
    GetPlayerPos(m,from,t);
    return;
  }
  switch(st){
  case ST_kickable_and_player_and_conf:
    Mem->LogAction3(50,"Tm %d say that ball is kickable for him",from);
    GetBallPosAndVel(m,from,t,true,Vector(.0f,.0f),1.0f/Mem->CP_ball_conf_decay);
    recieved_player=GetPlayerPos(m,from,t);
    if(recieved_player!=from){//hack: должно работать!
      LogAction6(100,"Recieved: player %.0f, playerx %.2f, playery %.2f, playerconf  %.2f",
		 float(from),BallX(),BallY(),BallPositionValid());
      HearPlayer(MySide,from,BallX(),BallY(),BallPositionValid(),DistanceTo(BallAbsolutePosition()),t); 
    }
    break;    
  case ST_opponent_have_ball://we must reset all creative precess
    Mem->LogAction3(50,"Tm %d say that opponent have ball",from);
    throughPass.StopThroughPass();
    Scenarios.HearStopScenario();//break not need here
  case ST_opponent_kick_ball:
    if(st==ST_opponent_kick_ball)
      Mem->LogAction3(50,"Tm %d say that opponent kick ball",from);
    GetBallPosAndVel(m,from,t);
    break;
  case ST_begin_through_pass:
    throughPass.RecievedBeginThroughPassMsg(msg,from,t);
    break;
  case ST_answer_through_pass:
    throughPass.RecievedAnswerThroughPass(msg,from,t);
    break;
  case ST_mypos_and_conf:
    GetMyPos(m,from,t);
    break;
  case ST_start_scenario:
    Mem->LogAction3(10,"Tm %d say, that must start scenario",from);
    if(Scenarios.IsScenarioInit())
      Scenarios.HearStartScenario(msg);
    return;
  case ST_stop_scenario:
    Mem->LogAction3(10,"Tm %d say, that must stop scenario",from);
    if(Scenarios.IsScenarioInit())
      Scenarios.HearStopScenario();
    GetBallPosAndVel(m,from,t);
    break;		
  case ST_none:
    break;
  case ST_ask:
    mustAnswer=true;
    m>>temp;
    answerT=aType(temp);
    if(answerT==AT_start_set_play_scenario1){
      mustAnswer = false;
      Mem->LogAction2(100,"Communicate: set SetPlay scenario 1");
      setPlay.BeginScenario(1);
      return;
    }
    if(answerT==AT_start_set_play_scenario2){
      mustAnswer = false;
      Mem->LogAction2(100,"Communicate: set SetPlay scenario 2");
      setPlay.BeginScenario(-1);
      return;
    }
    if(answerT!=AT_ball_pos&&answerT!=AT_ball_pos_and_vel&&answerT!=AT_none){
      m>>temp;
      player=temp-11;
      if(player==MyNumber){
	if(answerT==AT_player_pos)
	  answerT=AT_my_pos;
	if(answerT==AT_ball_pos_player_pos)
	  answerT=AT_my_pos_ball_pos;
      }
    }
    if(answerT==AT_most_danger_opponent_pos_to_pass)
      playerThatAskQuestion=from;
    GetMyPos(m,from,t);
    break;	
  case ST_ball_stop_mypos_and_conf_ball_and_conf:
    GetMyPos(m,from,t);
    GetBallPosAndVel(m,from,t,true,Vector(.0f,.0f));
    break;
  case ST_mypos_and_conf_ball_and_conf:
    GetMyPos(m,from,t);
    GetBallPos(m,from,t);
    break;
  case ST_mypos_and_conf_player_and_conf:
    GetMyPos(m,from,t);
    GetPlayerPos(m,from,t);
    break;
  case ST_player_and_conf_ball_and_conf:
    GetPlayerPos(m,from,t);
    GetBallPos(m,from,t);
    break;
  case ST_ball_and_conf_ballvel_and_conf:
    GetBallPosAndVel(m,from,t);
    break;
  default:
    my_error("sayType not valid!!!");
  }//end case
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::CorrectXY(float& x,float& y){
  if(x<-int(SP_pitch_length/2))
    x=-int(SP_pitch_length/2);
  if(x>int(SP_pitch_length/2))
    x=int(SP_pitch_length/2);
  if(y<-int(SP_pitch_width/2))
    y=-int(SP_pitch_width/2);
  if(y>int(SP_pitch_width/2))
    y=int(SP_pitch_width/2);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::AddBallPos(Msg& m){
  float ballx=.0f,bally=.0f;
  if(!Mem->BallPositionValid()){
    for(int i=1;i<=4;i++)
      m<<char(0);
    return;
  }
  if(Mem->BallVelocityValid()){
    ballx=Mem->BallPredictedPositionWithQueuedActions().x;
    bally=Mem->BallPredictedPositionWithQueuedActions().y;
  }else{
    ballx=Mem->BallAbsolutePosition().x;
    bally=Mem->BallAbsolutePosition().y;
  }
  CorrectXY(ballx,bally);
  m<<TransferXCoor(ballx)<<TransferYCoor(bally)<<TransferConf(BallPositionValid());
  LogAction5(100,"Say: ballx %.2f, bally %.2f, ballconf  %.2f",ballx,bally,BallPositionValid());
}
//////////////
void Communicate::AddBallVel(Msg& m){
  if(!Mem->BallVelocityValid()){
    for(int i=1;i<=5;i++)
      m<<char(0);
    return;
  }
  Vector vel=BallAbsoluteVelocity();
  vel*=SP_ball_decay;
  if(Mem->BallKickable())//hack
    vel=Vector(.0,.0);
  m<<TransferVel(vel.x)<<TransferVel(vel.y)<<TransferConf(BallVelocityValid());
  LogAction5(100,"Say:ballvelx %.2f,ballvely %.2f,ballvelconf %.2f",vel.x,vel.y,BallVelocityValid());
}
////////////////
void Communicate::AddMyPos(Msg& m){
  float myx=.0f,myy=.0f;
  if(!Mem->MyConf()){
    for(int i=1;i<=4;i++)
      m<<char(0);
    return;
  }
  myx=Mem->MyPredictedPositionWithQueuedActions().x;
  myy=Mem->MyPredictedPositionWithQueuedActions().y;
  CorrectXY(myx,myy);
  m<<TransferXCoor(myx)<<TransferYCoor(myy)<<TransferConf(MyConf());
  LogAction5(100,"Say:myx %.2f, myy %.2f,myconf %.2f",myx,myy,MyConf());
}
////////////////
void Communicate::AddPlayerPos(Unum player,Msg& m,bool send_number){
  char side=player<0?TheirSide:MySide;
  float playerx=.0f,playery=.0f;
  if(player!=Unum_Unknown&&PlayerPositionValid(side,abs(player))){
    playerx=PlayerAbsolutePosition(side,abs(player)).x;
    playery=PlayerAbsolutePosition(side,abs(player)).y;
    CorrectXY(playerx,playery);
  }
  if(player==Unum_Unknown||!PlayerPositionValid(side,abs(player))){
    int end=7;
    if(send_number)
      end=8;
    for(int i=4;i<=end;i++)
      m<<char(0);
  }else{
    m<<TransferXCoor(playerx)<<TransferYCoor(playery)<<TransferConf(PlayerPositionValid(side,abs(player)));
    if(send_number){
      char temp=player+11;
      m<<temp;
    }
    LogAction6(100,"Say:player %.0f playerx %.2f, playery %.2f, playerconf  %.2f",
	       float(player),playerx,playery,PlayerPositionValid(side,abs(player)));
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Communicate::FillMsgBody(char* msg,sayType st){
  Msg m(msg);

  if(st>=ST_fastest_tm1&&st<=ST_fastest_tm11){
    LogAction3(100,"Say that fastest to ball is %d tm",st-ST_fastest_tm1+1);
    AddBallPos(m);
    AddBallVel(m);
    return;
  }
  if(st>=ST_opp1_player_and_conf&&st<=ST_opp11_player_and_conf){
    AddPlayerPos(-(st-int(ST_opp1_player_and_conf)+1),m,false);
    AddPlayerPos(player,m);
    return;
  }
  if(st>=ST_tm1_player_and_conf&&st<=ST_tm11_player_and_conf){
    AddPlayerPos(st-int(ST_tm1_player_and_conf)+1,m,false);
    AddPlayerPos(player,m);
    return;
  }
  switch(st){
  case ST_kickable_and_player_and_conf:
    AddBallPos(m);
    AddPlayerPos(player,m);
    Mem->LogAction2(100,"Say that ball is kickable");
    break;
  case ST_mypos_and_conf:
    AddMyPos(m);
    break;
  case ST_none:
    for(int i=0;i<9;i++)
      m[i]=code(0);
    LogAction2(100,"Say empty msg");
    break;
  case ST_ask:
    m<<char(askT);
    if(askT!=AT_ball_pos&&askT!=AT_ball_pos_and_vel&&askT!=AT_none)
      m<<char(askPlayer+11);
    else
      m<<char(30);
    if(MyConf())
      AddMyPos(m);
    else{
      for(int i=2;i<=6;i++)
	m<<char(0);
    }
    LogAction4(100,"Say: question %.0f, player %.0f",(float)askT,(float)askPlayer);
    break;	
  case ST_ball_stop_mypos_and_conf_ball_and_conf:
    AddMyPos(m);
    AddBallPos(m);
    m<<TransferConf(BallVelocityValid());
    LogAction2(100,"Say:Ball not moving");
    break;
  case ST_mypos_and_conf_ball_and_conf:
    AddMyPos(m);
    AddBallPos(m);
    break;
  case ST_mypos_and_conf_player_and_conf:
    AddMyPos(m);
    AddPlayerPos(player,m);
    break;
  case ST_player_and_conf_ball_and_conf:
    AddPlayerPos(player,m);
    AddBallPos(m);
    break;
  case ST_ball_and_conf_ballvel_and_conf:
    AddBallPos(m);
    AddBallVel(m);
    break;
  default:
    my_error("sayType not valid!!!");
  }

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
char Communicate::code(int val){//0-74
  if(val==0) return ' ';
  if(val>=1&&val<=4)
    return char('('+val-1);
  if(val>=5&&val<=17)
    return char('-'+val-5);
  if(val==18) return '<';
  if(val==19||val==20)
    return char('>'+val-19);
  if(val>=21&&val<=46)
    return char('A'+val-21);
  if(val==47) return '_';
  if(val>=48&&val<=73)
    return char('a'+val-48);
  my_error("too big val in code function (or <0)");
  return '!';
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int Communicate::decode(char val){
  if(val==' ') return 0;
  if(val>='('&&val<='+')
    return int(val-'('+1);
  if(val>='-'&&val<='9')
    return int(val-'-'+5);
  if(val=='<') return 18;
  if(val=='>'||val=='?')
    return int(val-'>'+19);
  if(val>='A'&&val<='Z')
    return int(val-'A'+21);
  if(val=='_') return 47;
  if(val>='a'&&val<='z')
    return int(val-'a'+48);
  my_error("not right val in decode function");
  return 777;
}
