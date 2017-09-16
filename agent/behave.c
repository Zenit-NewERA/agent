/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : behave.C
 *
 *    AUTHOR     : Anton Ivanov, Sergei Serebyakov
 *
 *    $Revision: 2.26 $
 *
 *    $Id: behave.C,v 2.26 2004/08/29 14:07:21 anton Exp $
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
/* -*- Mode: C++ -*- */
  
/* behave.C
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
 *
 */


#include <fstream>
#include "behave.h"
#include "client.h"
#include "kick.h"
#include "dribble.h"
#include "goalie.h"
#include "Handleball.h"
#include "SetPlay.h"
#include "Playposition.h"
#include "utils.h"
#include "test.h"
#include "scenario.h"

#include <queue>
//****************************BEHAVE**********************************
void behave(){
  FuncCalcTime calc("behave");
  
  static bool first=true;

  if (first) {
    if (!actions.Initialize()) exit(-1);
    Formations::LoadConfigFile();
    interception.InitNetworks();
    send_support_clang();//чтобы получать freeform сообщения
    //		char fileName[256];
    //		sprintf(fileName,"./logs/wm/WorldModel_%.0f.bin",float(Mem->MyNumber));
    //		logGr.Initialize(Visualization::LM_Save, fileName);
    first=false;
  }
  //TEST CODE
  // logGr.BeginCycle();
  //   test_SetPlay();
  // logGr.EndCycle(Mem->CurrentTime.t);
  //   return;
  //END TEST CODE
	if( !Mem->MyConf() || !Mem->BallPositionValid() ) {
		scan_field_with_body();
		return;
	}
  statistica::stat.UpdateStatistic();

  Pos.UpdateFastestPlayers();//update fastest opp and tm from communication and world model
  Pos.SetFormation();
  
  my_update_offside_position();
  bool call_scan_field=true;
  if(setPlay.IsPenaltyGoing()){
    setPlay.Standart();
    call_scan_field=false;
  }
  else
    call_scan_field=!play_game();
  if(call_scan_field) 
    scan_field();
  
  //  logGr.EndCycle(Mem->CurrentTime.t);

}
///////////////////////////////////////////////////////////////////////////////////
bool play_game(){//return true if NOT need scan_field
  static Time last_set_play_time = -2;

  Pos.UpdateTmPredictPos();//predict positions of players in offense and scenario

  if(Mem->MyNumCyclesToEndTackling()>0){
    Mem->LogAction3(10,"NUM CYCLES TO STOP TACKLE: %d",Mem->MyNumCyclesToEndTackling());
    return false;
  }
  
  if( actions.KickInProgress() && last_set_play_time==Mem->CurrentTime-1 &&Mem->CurrentTime.s==0) {
    Mem->LogAction4(10,"Kick in progress:: continue kick, that was started last set play (%.0f %.0f)",
		    (float)last_set_play_time.t,(float)last_set_play_time.s);
    actions.smartkick();
    last_set_play_time = Mem->CurrentTime;
    return true;
  }

  if (Mem->MyNumber==Mem->FP_goalie_number){	
    goalie.Behave();
    return true;//we not need to turn the neck of goalie outside of this function
  }
  if( !Mem->MyConf() ) {
    Mem->LogAction2(10,"I don't know my position, so I scan field");
    scan_field_with_body();
    return true;
  }
  if(!Mem->FieldRectangle.IsWithin(Mem->MyPos())&&!Mem->BallKickable()&&Mem->PlayMode==PM_Play_On&&fabs(Mem->MyY())>34.0f){
    Mem->LogAction2(10,"I`m outside of field, so go to field");
    static int i=-1;
    static Time start=0;
    if(Mem->CurrentTime-start>10){
      start=Mem->CurrentTime;
      i*=-1;
    }
    Pos.MoveToPos(Vector(Mem->MyX(),i*64.0),5.0,0.0);
    return false;
  }

  if( Mem->BallPositionValid()<0.7f ) {
    Mem->LogAction2(10, "Ball position not valid.Scanning field");
    eye.SearchBall();
    return true;
  }

  if( setPlay.Standart() ){
    last_set_play_time = Mem->CurrentTime;
    return true;//in new vertion must be true
  }

  if(Pos.TmInter()&&!Mem->IsPointInBounds(Pos.TmPoint(),-2.0)&&(!Pos.OppInter()||!Mem->IsPointInBounds(Pos.OppPoint(),-3.0))){
    Mem->LogAction2(10,"Think that ball will be outside of field, so rest");
    float tackle_probability=Mem->GetTackleProb(Mem->BallAbsolutePosition(),Mem->MyPos(),Mem->MyBodyAng());
    if(Mem->MyX()<0&&Mem->BallPositionValid()==1.0f&&tackle_probability>0.75f&&!Mem->BallKickable()){
      Mem->LogAction3(10,"Make tackle with probability %.2f",tackle_probability);
      tackle(fabs(Mem->MyBodyAng())>90.0f?-100.0f:100.0f);
      return false;
    }
    Pos.GoToHomePosition(Vector(-200.0f,-200.0f),.3*Mem->SP_max_power);
    return false;
  }

  //temp
  if(Mem->MyX()>20.0f&&Mem->TheirGoalieNum!=Unum_Unknown){
    Vector pos=(Mem->OpponentPositionValid(Mem->TheirGoalieNum)?Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum):Vector(0,0));
    Mem->LogAction6(10,"GOALIE NUMBER=%.0f (pos val=%.2f); pos=(%.2f;%.2f)",
		    float(Mem->TheirGoalieNum),Mem->OpponentPositionValid(Mem->TheirGoalieNum),pos.x,pos.y);
  }

  //end temp
  if( Dribble::kick_to_myself_in_progress() ) return false;

  if( actions.Handle()    ) return false;

  if(Scenarios.ExecuteScenario())
    return false;

  if(Mem->MyStamina()<Mem->EffortDecThreshold){
    recover();
    return false;

  }

  if ( Mem->MyX()>((Mem->my_offside_opp==Unum_Unknown||Mem->MyX()<35.0f)?Mem->my_offside_line-1.0f:Mem->my_offside_line-0.5f)){
    if(Pos.FastestTm()==Mem->MyNumber){
      Mem->LogAction2(10,"I am in offside but I can get ball! So go to it");
      get_ball();
      return false;
    }
    Mem->LogAction2(10, "Mode: get on side");
    get_on_sides();
    return false;
  }
  Mem->kick_in_progress=FALSE;
  return Pos.PlayWithoutBall();
}
/////////////////////////////////////////////////////////////////////
//************************Through pass*****************************************************//
ThroughPass throughPass;

//Часть паса на ход с использованием специального протокола обмена сообщениями
ThroughPass::ThroughPass(){
  beginTime=-1;
}
///////////////////////////////////////////////////////////////
float ThroughPass::GetMaxEndPassVel()const{
  return 1.6f;
}
///////////////////////////////////////////////////////////////
float ThroughPass::GetMinEndPassVel()const{
  return 0.5f;
}
////////////////////////////////////////////////////////////////
//*************
struct TmInfo{
  TmInfo(Unum t,float c,Vector b,Vector a){tm=t;conf=c;base=b;addition=a;}
  Unum tm;
  float conf;
  Vector base;
  Vector addition;
  bool operator< (const TmInfo& x)const{return Pos.OurPositionValue(base)+conf<x.conf+Pos.OurPositionValue(x.base);}
};

struct DotInfo{
  DotInfo(float c,Vector p){conf=c;pos=p;}
  float conf;
  Vector pos;
  bool operator< (const DotInfo& x)const{return conf<x.conf;}
};
//*************
Unum ThroughPass::SelectThroughPass(int begin_cyc,Vector from,Vector* addition){
  const float deltaDist=4.0f,beginDist=1.0f,deltaAng=30.0f,beginAng=0.0f;//have only 19 steps on angle
  const float maxDist=16.0f,maxAng=360-deltaAng;

  Formations::Iterator iter=Pos.begin();
  Rectangle rec=Mem->FieldRectangle.shrink(3.0);
  priority_queue<TmInfo> pq;


  while(iter!=Pos.end()){
    Unum tm=Pos.GetPlayerNumber(iter,PT_Forward|PT_Midfielder);
    if(tm==Unum_Unknown) break;
    if(tm==Mem->MyNumber||Mem->TeammatePositionValid(tm)<.8f) continue;
    Vector base=Pos.GetTmPos(tm);

    Vector currentAddOptPos;
    bool have_opt=false;
    float currentOptVal=0.0f;

    priority_queue<DotInfo> dots;

    for(float ang=beginAng;ang<=maxAng;ang+=deltaAng){
      for(float dist=beginDist;dist<=maxDist;dist+=deltaDist){
        Vector pos=base+Polar2Vector(dist,GetNormalizeAngleDeg(ang));
        if(!rec.IsWithin(pos)) break;
        if(pos.x<=from.x/*&&Pos.OurPositionValue(pos)<Pos.OurPositionValue(from)*/) continue;
        Vector offside_vec=base+Polar2Vector(Min(Max(0,(begin_cyc-1))*Mem->GetTeammatePlayerSpeedMax(tm),dist),GetNormalizeAngleDeg(ang));
        if(Mem->my_offside_line-1.0f<offside_vec.x) continue;
        if(from.dist(pos)<dist) continue;
	if(fabs(from.y-pos.y)<7.0f) continue;
        DotInfo di(180-fabs(GetNormalizeAngleDeg(ang))+Pos.OurPositionValue(pos),pos);
        dots.push(di);
      }
    }
    int maxnum=dots.size();
    Unum danger_opp=Unum_Unknown;
    while(!dots.empty()){
      Vector pos=dots.top().pos;
      Mem->LogAction4(50,"SelectThroughPass: calc for add vector (%.2f,%.2f)",(pos-base).x,(pos-base).y);

      float vel=Min(Mem->SP_ball_speed_max,SolveForFirstTermGeomSeries(Mem->SP_ball_decay,int((base-pos).mod()/Mem->GetTeammatePlayerSpeedMax(tm)),(from-pos).mod()));
      float end_vel=vel*int_pow(Mem->SP_ball_decay,int((base-pos).mod()/Mem->GetTeammatePlayerSpeedMax(tm)));
      end_vel=end_vel<GetMinEndPassVel()?GetMinEndPassVel():end_vel;
      end_vel=end_vel>GetMaxEndPassVel()?GetMaxEndPassVel():end_vel;
      float ball_steps =SolveForLengthGeomSeries(end_vel, 1/Mem->SP_ball_decay,(from-pos).mod());
      vel=end_vel * pow(1/Mem->SP_ball_decay, ball_steps);//set correct vel

      float res_conf;
      if(danger_opp==Unum_Unknown||
	 (danger_opp!=Unum_Unknown&&Pos.OppInetrceptionValue(danger_opp,from,pos,Polar2Vector(vel,(pos-from).dir()))>0.5f)){
	if((res_conf=Pos.DotPassConf(from,pos,Polar2Vector(vel,(pos-from).dir()),&danger_opp))>0.5f){
	  have_opt=true;
	  res_conf+=Pos.OurPositionValue(pos);

	  currentAddOptPos=pos-base;
	  currentOptVal=res_conf;
	  break;
	}
      }
      dots.pop();
    }
    Mem->LogAction8(30,"SelectThroughPass. For tm %.0f: all=%.0f;valid=%.0f; add_dot=(%.2f,%.2f); ocenka=%.4f",
		    float(tm),float(maxnum),float(have_opt),currentAddOptPos.x,currentAddOptPos.y,currentOptVal);
    if(have_opt){
      TmInfo ti(tm,currentOptVal,base,currentAddOptPos);
      pq.push(ti);
    }
  }
  if(pq.empty())
    return Unum_Unknown;
  
  iter=Pos.begin();
  float cl_dist=100.0f;
  Unum cl_tm=Unum_Unknown;
  Vector res=pq.top().base+pq.top().addition;
  while(iter!=Pos.end()){
    Unum tm=Pos.GetPlayerNumber(iter,PT_Forward|PT_Midfielder);
    if(tm==Unum_Unknown) break;
    if(tm==Mem->MyNumber||Mem->TeammatePositionValid(tm)<.8f) continue;
    if(Mem->TeammateDistanceTo(tm,res)<cl_dist){
      cl_dist=Mem->TeammateDistanceTo(tm,res);
      cl_tm=tm;
    }
  }
   
  Mem->LogAction3(50,"*****SelectThroughPass: selcet tm %.0f",float(cl_tm));
  *addition=pq.top().addition;
  return cl_tm;
}

////////////////////////////////////////////////////////////////
bool ThroughPass::StartThroughPass(){
  const int MAX_CYC =7;// - только для тестирования
  static Time last_calc_time=0;
  
  int cycles=Pos.TmCycles();
  if(Pos.FastestTm()!=Mem->MyNumber/*||Mem->MyX()>30.0f*/||!Pos.IsOffense()||cycles>MAX_CYC||cycles<2||
     (Mem->CurrentTime-last_calc_time)<5)
    return false;
  if(beginTime!=-1&&Mem->CurrentTime-beginTime<MAX_CYC*2){//through pass надо пересчитать
    if(from==Mem->MyNumber){
      if(Mem->CurrentTime-beginTime>=2&&answerTime-beginTime!=2){
	
	Mem->LogAction4(50,"In through pass - must recalculate again becouse there was not answer. %.0f; %.0f",
			float(Mem->CurrentTime-beginTime),float(answerTime-beginTime));
      }else
	return false;
    }else
      return false;//somebody else start through pass
  }
  last_calc_time=Mem->CurrentTime;
  
  Vector addPos;//добавочный вектор к оригинальной позиции игрока
  statistica::TimeCounter tc;
  tc.Start();
  Vector target=Dribble::PredictDribbleStopPosition(Mem->MyNumber,Dribble::SelectDribbleTarget(),0.6f,true,&cycles);
  Unum tm=SelectThroughPass(cycles,target,&addPos);//какому игроку даем пас на ход
  tc.Finish();
  if(tm==Unum_Unknown) return false;
  Vector origPos=Pos.GetTmPos(tm);

  Mem->LogAction7(10,"Through pass to tm %.0f (pos valid is %.2f). addPos=(%.2f,%.2f); cycles to ball: %.0f",
		  float(tm),Mem->TeammatePositionValid(tm),addPos.x,addPos.y,float(cycles));
  char msg[10]={0,0,0,0,0,0,0,0,0,0};
  Msg m(msg);
  m<<char(ST_begin_through_pass);


  targetPos=origPos+addPos;//update state
  m<<char(tm);
  m<<TransferXCoor(targetPos.x)<<TransferYCoor(targetPos.y);
  m<<TransferXCoor(Pos.TmPoint().x)<<TransferYCoor(Pos.TmPoint().y)<<char(cycles-1);
  Mem->SayNow(msg);
  //update state
  from=Mem->MyNumber;
  to=tm;
  beginTime=Mem->CurrentTime;
  num_cyc=cycles;
  return true;
}
///////////////////////////////////////////////////////////////////
void ThroughPass::RecievedBeginThroughPassMsg(char* msg,Unum from,Time time){
  this->from=from;
  beginTime=time;
  Msg m(msg);
  char temp;
  m>>temp>>TransferXCoor(&targetPos.x)>>TransferYCoor(&targetPos.y);
  to=Unum(temp);
  m>>TransferXCoor(&fromPos.x)>>TransferYCoor(&fromPos.y)>>temp;
  num_cyc=int(temp);
  Mem->LogAction7(50,"RecievedBeginThroughPass:from %.0f to %.0f; Point=(%.2f,%.2f); num_cyc=%.0f",
		  float(from),float(to),targetPos.x,targetPos.y,float(num_cyc));
}
////////////////////////////////////////////////////////////////////////
Unum ThroughPass::ChangeAttantionInFroughPassToTm(){
  if((Mem->CurrentTime==beginTime||(from==Mem->MyNumber&&Pos.FastestTm()==Mem->MyNumber&&Pos.IsOffense()&&Mem->CurrentTime-beginTime<=Max(2,num_cyc*2)))
     &&to!=Mem->MyNumber)
    return to;
  return Unum_Unknown;
}
//////////////////////////////////////////////////////////////////////
bool ThroughPass::IsTmGoInThroughPass(Unum tm){
  return beginTime!=-1&&to==tm&&((Pos.FastestTm()==from||Mem->ClosestTeammateToBall()==from)
				 &&Mem->CurrentTime-beginTime<=Max(2,num_cyc*2));
}
//////////////////////////////////////////////////////////////////////
bool ThroughPass::GoInThroughPass(){
  if(IsTmGoInThroughPass(Mem->MyNumber)){
    if(beginTime==Mem->CurrentTime){//надо послать ответное сообщение
      if(Mem->MyStamina()<=Mem->SP_stamina_max*0.5f){
	Mem->LogAction2(10,"I`m in through pass, but i tired, so not send massege");
	beginTime=-1;
	return false;
      }
      int predict_cyc=Mem->PredictedCyclesToPoint(targetPos);
      Mem->LogAction3(50,"GoInThroughPass: i`m reciever of pass and now say about cycles to point : %d",predict_cyc);
      char msg[10]={0,0,0,0,0,0,0,0,0,0};
      Msg m(msg);
      m<<char(ST_answer_through_pass)<<char(Max(0,predict_cyc-1));
      Mem->AddMyPos(m);
      Mem->SayNow(msg);
    }
    Mem->LogAction6(10,"GoInThroughPass: i`m reciever, so go to target pos (%.2f,%.2f).Begin at %.0f time (wait from %.0f tm).",
		    targetPos.x,targetPos.y,float(beginTime.t),float(from));
    if(!Pos.MoveToPos(targetPos,Mem->CP_max_go_to_point_angle_err,0,false)){
      face_only_body_to_ball();
    }
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////


void ThroughPass::RecievedAnswerThroughPass(char* msg,Unum from,Time time){
  if(from!=to)
    my_error("recieved msg from wrong tm %d !!!",from);
  answerTime=time;
  Msg m(msg);
  char temp;
  m>>temp;
  Mem->GetMyPos(m,from,time);
  answer_cyc=int(temp);
  Mem->LogAction4(50,"RecievedAnswerThroughPass: get msg from %.0f with cyc number %.0f",float(from),float(answer_cyc));
}
////////////////////////////////////////////////////////////////////////
float ThroughPass::CalcThroughPassVel(){
  int cyc_diff=Mem->CurrentTime-answerTime;

  return (targetPos-Mem->BallAbsolutePosition()).mod()/Mem->GetBallMoveCoeff(answer_cyc-cyc_diff+1);
}
////////////////////////////////////////////////////////////////////////
bool ThroughPass::IsIStartThroughPass()const
{
  const int KICK_BUFFER=5;
  return beginTime.t!=-1&&from==Mem->MyNumber&&Mem->CurrentTime.t-beginTime.t<=num_cyc+KICK_BUFFER&&beginTime.t-answerTime.t<=2;
}
///////////////////////////////////////////////////////////////////////

bool ThroughPass::KickInThroughPass(){
  float vel;
  if(!IsIStartThroughPass())
    return false;

  vel=CalcThroughPassVel();
  if(Pos.DotPassConf(Mem->BallAbsolutePosition(),targetPos,Polar2Vector(vel,(targetPos-Mem->BallAbsolutePosition()).dir()))<=.5f){
    Mem->LogAction4(10,"KickInThroughPass:conf of through kick to pos (%.2f,%.2f) is too small",targetPos.x,targetPos.y);
    //    beginTime=-1;
    return  false;
  }
  Mem->LogAction5(10,"KickInThroughPass: make through kick to pos (%.2f,%.2f). vel=%.2f",
		  targetPos.x,targetPos.y,vel);
  actions.smartkick(vel,actions.GetKickAngle(targetPos),SK_Safe);
  return true;
}
//************************************End of through pass********************************************************//

//****************************** Start micro kicks **************************************************//
MicroKicks microKicks;

Vector MicroKicks::SelectAvoidPosition(Vector avoid_pos,Vector target)
{
  Vector center=Mem->MyPredictedPosition();
  Line l;
  l.LineFromTwoPoints(center,target);
  Line perp_l=l.perpendicular(center/*+(target-center)*Mem->GetMyKickableArea()*0.2f/(target-center).mod()*/);
  Vector pos1=center+Polar2Vector(Mem->GetMyKickableArea()*0.8f,perp_l.angle()),
    pos2=center+Polar2Vector(Mem->GetMyKickableArea()*0.8f,GetNormalizeAngleDeg(perp_l.angle()+180.0f));
  if(avoid_pos.dist(pos1)<avoid_pos.dist(pos2))
    return pos2;
  else
    return pos1;
}
//////////////////////////////////////////////////////////////////////
bool MicroKicks::CanMicroAvoidGoalie(float side)
{
  if(Mem->TheirGoalieNum==Unum_Unknown||Mem->OpponentPositionValid(Mem->TheirGoalieNum)<0.95f)
    return false;
  if(IsGoalieActive(side)&&fabs(Mem->MyY())>Mem->SP_goal_width/2.0f)//hack
    return false;
  Vector avoid_pos=Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum);
  Vector target=Vector(Mem->SP_pitch_length/2.0f*side,0.0f);
  Vector new_pos=SelectAvoidPosition(avoid_pos,target);
  if(!Pos.CheckWithTackle(new_pos,Positioning::CT_VeryAgressive,true,side)){
    Mem->LogAction4(10,"CanMicroAvoidGoalie:position (%.2f,%.2f) is not valid",
		    new_pos.x,new_pos.y);
    return false;
  }
  if(fabs(new_pos.y)>Mem->SP_goal_width/2.0f-2.0f&&fabs(new_pos.y)>fabs(Mem->OpponentY(Mem->TheirGoalieNum)))//hack
    return false;
  float conf=actions.explore_player(Mem->MyNumber,side,new_pos).GetConfidence();
  Mem->LogAction5(10,"CanMicroAvoidGoalie: shoot from position (%.2f,%.2f) has conf %.2f",
		  new_pos.x,new_pos.y,conf);
  if(conf>0.7f&&actions.smartkick((new_pos-Mem->BallAbsolutePosition()).mod(),new_pos,SK_Fast)==SK_KickDone){
    Mem->LogAction2(10,"CanMicroAvoidGoalie: so can execute micro kick");
    eye.AddOpponent(Mem->TheirGoalieNum,1);
    return true;
  }
  
  return false;
}
//////////////////////////////////////////////////////////////////////
bool MicroKicks::CanMicroClearBall(bool can_stop_ball)
{
  Unum opp=Mem->ClosestOpponentToBall();
  if(opp==Unum_Unknown||Mem->OpponentPositionValid(opp)<0.95f)
    return false;
  Vector avoid_pos=Mem->OpponentAbsolutePosition(opp);
  Vector target=Mem->MyPos()+Polar2Vector(1.0f,Mem->MyBodyAng());
  Vector new_pos=SelectAvoidPosition(avoid_pos,target);
  if(!Pos.CheckWithTackle(new_pos,Positioning::CT_Agressive,true)){
    Mem->LogAction4(10,"CanMicroClearBall:position (%.2f,%.2f) is not valid",
		    new_pos.x,new_pos.y);
    return false;
  }
  if(!can_stop_ball&&Mem->BallAbsolutePosition().dist(new_pos)<=0.1f){
    Mem->LogAction3(10,"CanMicroClearBall: ball is close totarget, but can not stop ball (%.2f)",
		    Mem->BallAbsolutePosition().dist(new_pos));
    return false;
  }
  
  if(actions.smartkick((new_pos-Mem->BallAbsolutePosition()).mod(),new_pos,SK_Fast)==SK_KickDone){
    Mem->LogAction2(10,"CanMicroClearBall: can execute micro kick");
    return true;
  }
  
  return false;
}
//////////////////////////////////////////////////////////////////////
bool MicroKicks::CanMicroCrossBall(Vector target)
{
  
  Vector ball=Mem->BallAbsolutePosition();
  Vector avoid_pos;
  Unum opp=Unum_Unknown;

  if(actions.SelectShootMode()==SK_Fast)
    return false;
  Line l;
  l.LineFromTwoPoints(ball,target);
  if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.95f&&
     Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<10.0f&&
     l.InBetween(l.ProjectPoint(avoid_pos=Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)),ball,target)){
    Mem->LogAction3(10,"CanMicroCrossBall: opponent goalie has dist %.2f to line",
		    l.dist(avoid_pos));
    if(l.dist(avoid_pos)<Mem->SP_catch_area_l){
      opp=Mem->TheirGoalieNum;
    }
  }
  if(opp==Unum_Unknown){
    float min_dist=100.0f;
    for(int i=1;i<=11;i++){
      if(Mem->OpponentPositionValid(i)<0.8f||!l.InBetween(l.ProjectPoint(Mem->OpponentAbsolutePosition(i)),ball,target))
	continue;
      if((l.dist(Mem->OpponentAbsolutePosition(i))+Mem->OpponentDistanceToBall(i))<min_dist){
	min_dist=l.dist(Mem->OpponentAbsolutePosition(i))+Mem->OpponentDistanceToBall(i);
	opp=i;
      }
    }
    
    if(opp==Unum_Unknown)
     return false;
    Mem->LogAction3(10,"CanMicroCrossBall: closest opp to line is %.0f",
		    float(opp));
    avoid_pos=Mem->OpponentAbsolutePosition(opp);
  }
  
  Mem->LogAction4(10,"CanMicroCrossBall: target=(%.2f;%.2f)",
		  target.x,target.y);
  Vector new_pos=SelectAvoidPosition(avoid_pos,target);
  if(!Pos.CheckWithTackle(new_pos,Positioning::CT_Agressive,true)){
    Mem->LogAction4(10,"CanMicroCrossBall:position (%.2f,%.2f) is not valid",
		    new_pos.x,new_pos.y);
    return false;
  }
  if(Mem->BallAbsolutePosition().dist(new_pos)>0.1f&&actions.smartkick((new_pos-Mem->BallAbsolutePosition()).mod(),new_pos,SK_Fast)==SK_KickDone){
    Mem->LogAction2(10,"CanMicroCrossBall: can execute micro kick");
    return true;
  }
  
  return false;
}
//////////////////////////////////////////////////////////////////////
bool MicroKicks::CanMicroHardestShoot(float side)
{
  if(IsGoalieActive(side)&&Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<5.0f)
    return false;
  Vector new_pos=Mem->MyPos()+Polar2Vector(Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.1f,Mem->MyBodyAng());
  Mem->LogAction3(10,"CanMicroHardestShoot: dist to target pos=%.2f",
		  Mem->BallAbsolutePosition().dist(new_pos));
  if(Mem->BallAbsolutePosition().dist(new_pos)>0.2f&&Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.94f){
    new_pos=Mem->MyPredictedPosition()+Polar2Vector(Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.1f,Mem->MyBodyAng());
    if(!Pos.CheckWithTackle(new_pos,Positioning::CT_VeryAgressive,true)){
      Mem->LogAction4(10,"CanMicroHardestShoot:position (%.2f,%.2f) is not valid",
		      new_pos.x,new_pos.y);
      return false;
    }
    if(actions.smartkick((new_pos-Mem->BallAbsolutePosition()).mod(),new_pos,SK_Fast)==SK_KickDone){
      Mem->LogAction2(10,"CanMicroHardestShoot: can execute micro kick");
      return true;
    }else{
      Mem->LogAction2(10,"CanMicroHardestShoot: can not kick with target pos in 1 kick");      
      return false;
    }
  }
  //мяч находится на требуемой позиции (или плохо знаем вратаря), пытаемся выполнить удар
  Vector shoot_pos=fabs(-7.0f-Mem->MyY())<fabs(7.0f-Mem->MyY())?Vector(52.5f*side,-6.5f):Vector(52.5f*side,6.5f);
  if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)){  
    AngleDeg left=GetDiff((Vector(52.5f*side,-7.0f)-Mem->BallAbsolutePosition()).dir(),
			  (Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)-Mem->BallAbsolutePosition()).dir()),
      right=GetDiff((Vector(52.5f*side,7.0f)-Mem->BallAbsolutePosition()).dir(),
		    (Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)-Mem->BallAbsolutePosition()).dir());
    if(left<right){
      shoot_pos=Vector(52.5f*side,6.5f);
    }else{
      shoot_pos=Vector(52.5f*side,-6.5f);
    }
    Mem->LogAction4(10,"CanMicroHardestShoot: shoot angels left=%.2f; right=%.2f",
		    left,right);      
    if(max(left,right)<30.0f){
      Mem->LogAction2(10,"CanMicroHardestShoot: can not shoot becouse ang diff is very little");      
      return false;
    } 
  }
  actions.smartkick(2*Mem->SP_ball_speed_max,shoot_pos,SK_Fast);
  return true;
}
//****************************** End micro kicks **************************************************//
//////////////////////////////////////////////////////////////////////
bool IsGoalieActive(float penalty_side){
  bool active_goalie=false;
  Unum goalie=Mem->TheirGoalieNum;
  if(goalie!=Unum_Unknown&&Mem->OpponentPositionValid(goalie)&&(Mem->OpponentX(goalie)*penalty_side<45.0f||fabs(Mem->OpponentY(goalie))>9.0f)){
    active_goalie=true;
  }
  return active_goalie;
}
//////////////////////////////////////////////////////////////////////
bool PenaltyPlay(){//return true if must call scan_field()
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    Mem->LogAction2(10,"PenaltyPlay:find ball or myself");
    return false;
  }
  if(Mem->PlayMode!=PM_Play_On&&Mem->PlayMode!=PM_My_Kick_Off){
    move(Vector(-10.0f,0.0f));
    return true;
  }
  Unum goalie=Mem->TheirGoalieNum;

  Mem->LogAction4(10,"PenaltyPlay: goalie %.0f; conf = %.2f",float(Mem->TheirGoalieNum),Mem->OpponentPositionValid(goalie));
  if(Mem->OpponentPositionValid(goalie))
    Mem->LogAction4(10,"PenaltyPlay: his pos (%.2f;%.2f)",Mem->OpponentX(goalie),Mem->OpponentY(goalie));
  if(!Mem->BallKickable()){
    Mem->LogAction2(10,"PenaltyPlay: ball not kickable -> get_ball");
    get_ball();
    return true;
  }
  if( Dribble::kick_to_myself_in_progress() ) return true;

  //ball kickable
  bool active_goalie=IsGoalieActive();

  actions.ShootDecision();//calc shoot

  DribbleType dt=no_avoid;//no_control_dribble;

  if( actions.MyShootConf()>actions.ShootThreshold() ) {
    Mem->LogAction2(10,"PenaltyPlay:shooting with great conf");
    actions.shoot();
    return true;
  }

  static Dribble dribble;
  dribble.SetDribblePos(Vector(52.0f,0.0f));
  float dribbleAngle=10.0f;
  if(Mem->MyX()>30.0f)
    dribbleAngle=30.0f;
  dribble.SetDribbleTurnAngleError(dribbleAngle);

  if(active_goalie)
    dribble.SetPriority(0.2f);
  if(dribble.GoBaby(dt)){
    Mem->LogAction2(10,"PenaltyPlay:with dribble go to pos (52.0,0.0)");
    return true;
  }
  dribble.SetDribbleTurnAngleError(30.0f);
  if(active_goalie)
    dribble.SetPriority(0.8f);
  if(active_goalie&&dribble.DribbleDance()){
    Mem->LogAction2(10,"PenaltyPlay:try dribble dance");
    return true;
  }
  Mem->LogAction2(10,"PenaltyPlay: nothing to do : shoot at end");
  float s1,s2;
  Unum o1,o2;
  float c1=actions.shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5,-6.9),s1,o1,Mem->MyNumber);
  float c2=actions.shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5,6.9),s2,o2,Mem->MyNumber);
  Mem->LogAction4(10,"PenaltyPlay: c1=%.2f; c2= %.2f",c1,c2);
  if(c2>c1){
    actions.smartkick(Mem->SP_ball_speed_max*.9,Vector(52.5,6),SK_Safe);
  }else{
    actions.smartkick(Mem->SP_ball_speed_max*.9,Vector(52.5,-6),SK_Safe);
  }
  return true;
}
///////////////////////////////////////////////////////////////////////
void PenaltyPlayScanField(){
  change_view(eye.GetBestViewWidth());
  Unum goalie=Mem->TheirGoalieNum;
  if(Mem->BallPositionValid()<0.95||goalie==Unum_Unknown||!Mem->OpponentPositionValid(goalie))

    face_only_neck_to_ball();
  else
    face_only_neck_to_opponent(goalie);
}
//////////////////////////////////////////////////////////////////////

void scan_field(void){
  //  eye.CheckForChangingViewWidth();
  //	const double player_conf=0.97;
  //  if(Mem->player_that_must_see!=Unum_Unknown){
  //  	char side=Mem->MySide;
  //  	if(Mem->player_that_must_see<0)
  //  		side=Mem->TheirSide;
  //  	Mem->player_that_must_see=abs(Mem->player_that_must_see);
  //  	if(Mem->PlayerPositionValid(side,Mem->player_that_must_see)<=player_conf&&(Mem->PlayerPositionValid(side,Mem->player_that_must_see)&&Mem->CanSeePointWithNeck(Mem->PlayerAbsolutePosition(side,Mem->player_that_must_see)))){
  //  		Mem->LogAction3(10,"scan_field: turn neck to see the player: %f",float((side==Mem->MySide?1.0:-1.0)*Mem->player_that_must_see));
  //  		face_only_neck_to_player(side,Mem->player_that_must_see);
  //  		Mem->player_that_must_see=Unum_Unknown;
  //  		return;
  //  	}
  //  	Mem->player_that_must_see=Unum_Unknown;

  //  }
  //
  //  if(Mem->must_see_ang){
  //  	if(Mem->BallPositionValid()>=0.95){
  //  		Mem->LogAction3(10,"scan_field: turn neck to see angle: %f",Mem->ang_to_see);
  //  		turn_neck(Mem->ang_to_see-Mem->MyBodyAng());
  //  		Mem->must_see_ang=false;
  //
  //  		return;
  //  	}
  //  	Mem->must_see_ang=false;
  //  }
  eye.Observe();
  return;
}
////////////////////////////////////////////////////////////////////
void scan_field_to_ball(void){
  static int type=1;
  const double buffer=15.0;
  const double player_conf=0.95;
  const double goalie_conf=0.97;
  float dist=Pos.IsDefense()?4.0:2.0;
  if(Mem->DistanceTo(Mem->BallPredictedPositionWithQueuedActions())<=dist){
    Mem->LogAction2(10,"I too far for ball so change view width to narrow");
    change_view(VW_Narrow);
  }else
    if(Mem->ViewWidth!=VW_Wide&&Mem->DistanceTo(Mem->BallPredictedPositionWithQueuedActions())>Mem->CP_dist_to_wide_view){
      Mem->LogAction2(10,"I too far for ball so change view width to wide");
      change_view(VW_Wide);
    }else

      if(Mem->ViewWidth!=VW_Normal&&Mem->DistanceTo(Mem->BallPredictedPositionWithQueuedActions())<=Mem->CP_dist_to_wide_view){
	Mem->LogAction2(10,"I close for ball so change view width to normal");
	change_view(VW_Normal);
      }

  if(Mem->TimeToTurnForScan()){
	
    if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->player_that_must_see==-Mem->TheirGoalieNum){//try to see opponent gaolie
      Mem->player_that_must_see=Unum_Unknown;
      if(Mem->OpponentPositionValid(Mem->TheirGoalieNum)<goalie_conf&&(Mem->OpponentPositionValid(Mem->TheirGoalieNum)&&Mem->CanSeeOpponentWithNeck(Mem->TheirGoalieNum))){
	Mem->LogAction3(10,"scan_field: try to see opponent goalie (conf: %f)",Mem->OpponentPositionValid(Mem->TheirGoalieNum));
	face_only_neck_to_opponent(Mem->TheirGoalieNum);
	return;
      }
    }
		
    if(Mem->player_that_must_see!=Unum_Unknown){
      char side=Mem->MySide;


      if(Mem->player_that_must_see<0)
	side=Mem->TheirSide;
      Mem->player_that_must_see=abs(Mem->player_that_must_see);
      if(Mem->PlayerPositionValid(side,Mem->player_that_must_see)<player_conf&&(Mem->PlayerPositionValid(side,Mem->player_that_must_see)&&Mem->CanSeePointWithNeck(Mem->PlayerAbsolutePosition(side,Mem->player_that_must_see)))){
	Mem->LogAction3(10,"scan_field: turn neck to see the player: %f",float((side==Mem->MySide?1.0:-1.0)*Mem->player_that_must_see));
	face_only_neck_to_player(side,Mem->player_that_must_see);

	Mem->player_that_must_see=Unum_Unknown;
	return;
      }
      Mem->player_that_must_see=Unum_Unknown;
    }
       	
    if(Mem->must_see_ang){
      if(Mem->BallPositionValid()>=0.95){
	Mem->LogAction3(10,"scan_field: turn neck to see angle: %f",Mem->ang_to_see);
	turn_neck(Mem->ang_to_see-Mem->MyBodyAng());
	Mem->must_see_ang=false;
	return;
      }
      Mem->must_see_ang=false;
    }
       	
    AngleDeg target_rel_ball_ang = Mem->PredictedPointRelAngFromBodyWithQueuedActions(Mem->BallPredictedPositionWithQueuedActions());
    double temp=Mem->MyViewAngle()-buffer;
    AngleDeg ang=target_rel_ball_ang+type*temp;
    if(Mem->ViewWidth==VW_Wide){
      ang=target_rel_ball_ang+type*30.0;
    }
    NormalizeAngleDeg(&ang);
    type*=-1;

    if(Mem->CanFaceAngleFromBodyWithNeck(ang)){
      Mem->LogAction4(50,"scan_field:turn neck to angle:%f (%d)",(float)ang,type);
      turn_neck_to_relative_angle(ang);

    }else{//try just see ball
      if ( Mem->CanSeeAngleFromBodyWithNeck(Mem->LimitTurnNeckAngle(ang - Mem->MyNeckRelAng())) ){
	Mem->LogAction2(10,"scan_field: try just see ball");
        if(Mem->DistanceTo(Mem->BallAbsolutePosition())>10.0)
	  // change_view(VW_Wide);
	  turn_neck( Mem->LimitTurnNeckAngle(ang - Mem->MyNeckRelAng()) );
	//face_only_neck_to_ball();
      }else{
	Mem->LogAction2(10,"scan_field: we can not face ball, so scan field with neck");
	scan_field_with_neck();
      }
    }
  }else{
    Mem->TurnNeck.type = CMD_none; // no one can send turn_neck exsept us
    Mem->LogAction2(10,"scan_field: it is not a time to turn to scan");
  }
}
/************************************************************************/
int NumOfCyclesThenILastSeePlayer(int c){
  return NumOfCyclesThenILastSeePlayer(c<0?Mem->OpponentPositionValid(-c):Mem->TeammatePositionValid(c));
}


int NumOfCyclesThenILastSeePlayer(float c){

  if(c>1.0f||c<=0.0f){
    my_error("wrong argument in NumOfCyclesThenILastSeeOfPlayer %.2f",c);
    return 0;
  }
  static float optimizing=log(Mem->CP_player_conf_decay);
  return int(log(c)/optimizing);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void my_update_offside_position(void){
  float first = 100.0, second = 100.0,first_tm_conf=0.0f;
  Unum first_tm=Unum_Unknown;
  const double max_dist_correct=20.0;
  for(int i=1;i<=Mem->num_my_players;i++){
    if(Mem->TeammatePositionValid(i)&&Mem->TeammateAbsolutePosition(i).x<second){
      second=Mem->TeammateAbsolutePosition(i).x;

      Mem->their_offside_conf=Mem->TeammatePositionValid(i);
      Mem->their_offside_tm=i;
      if(i==Mem->MyNumber)
	Mem->their_offside_without_me=Min(0,first);
      if (second < first&&Mem->OurGoalieNum!=Unum_Unknown&&Mem->TeammatePositionValid(Mem->OurGoalieNum)) {
	if(first_tm==Mem->MyNumber)
	  Mem->their_offside_without_me=second;
       	second = first ;
	Mem->their_offside_conf=first_tm_conf;
	Mem->their_offside_tm=first_tm;
       	first = Mem->TeammateAbsolutePosition(i).x;

	first_tm_conf=Mem->TeammatePositionValid(i);
	first_tm=i;
      }

    }else
      if(i==Mem->MyNumber)
	Mem->their_offside_without_me=second;	
  }
  if(Mem->BallPositionValid()&&Mem->BallAbsolutePosition().x<second){
    Mem->their_offside_conf=1.0f;
    Mem->their_offside_tm=Unum_Unknown;
    Mem->their_offside_line=Mem->BallAbsolutePosition().x;
    Mem->their_offside_without_me=Mem->their_offside_line;
  }else{
    Mem->their_offside_line=second;
  }
  //for our offside line

  first=second=-100.0;
  float first_conf=0.0f;
  Unum first_opp=Unum_Unknown;
  float modif_coef=(Mem->MyX()>30.0f&&Pos.IsOffense())?1.0f:Mem->CP_offside_value;
  
  for(int i=1;i<=Mem->SP_team_size;i++){
    if(Mem->OpponentPositionValid(i)){
      double num=NumOfCyclesThenILastSeePlayer(Mem->OpponentPositionValid(i));
      num*=modif_coef;
      if(num>max_dist_correct)
	num=max_dist_correct;
      double new_x=Mem->OpponentAbsolutePosition(i).x;
      if(i!=Mem->TheirGoalieNum)
        new_x-=num;		
      if(second<new_x){
	second=new_x;
	Mem->my_offside_conf=Mem->OpponentPositionValid(i);
	Mem->my_offside_opp=i;
	if (second > first/*&&Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)*/) {
	  second = first ;
	  first = new_x ;
	  Mem->my_offside_conf=first_conf;
	  Mem->my_offside_opp=first_opp;

	  first_conf= Mem->OpponentPositionValid(i);
	  first_opp=i;
	}

	//	Mem->LogAction6(180,"update_offside: player number: %f ;playerX %f, update_player_x: %f, his confidance %f",
	//	(float)i,Mem->OpponentAbsolutePosition(i).x,new_x,Mem->OpponentPositionValid(i));
      }
    }
  }
  if(Mem->BallPositionValid()&&Mem->BallAbsolutePosition().x>second){
    Mem->my_offside_line=Mem->BallAbsolutePosition().x;
    Mem->my_offside_conf=1.0f;//maybe ok

    Mem->my_offside_opp=Unum_Unknown;
  }
  else{

    Mem->my_offside_line=second;
  }
  Mem->my_offside_line=Max(0,Mem->my_offside_line);
  Mem->their_offside_line=Min(0,Mem->their_offside_line);
  Mem->LogAction6(150,"Offside: %.2f with confidence: %.2f (opp %.0f; orig opp x = %.2f)",
		  Mem->my_offside_line,Mem->my_offside_conf,float(Mem->my_offside_opp),
		  Mem->my_offside_opp==Unum_Unknown?0.0:Mem->OpponentX(Mem->my_offside_opp));
}
///////////////////////////////////////////////////////////////////////////////////////////////////

bool InDangerSituation() {
  if( Mem->NumOpponentsWithin(8)==0 ) return false;
  if( Mem->InOwnPenaltyArea() ) return true;
  if( Mem->MarkerDistance(Mem->RM_My_Goal) < 25 ) return true;
  return false;
}
/************************************************************************/
void clear_ball(){
  if(!Mem->BallKickable())
    my_error("clear_ball: ball not kickable");
  AngleDeg ang;
  AngleDeg ang_max=0.0f;
  float exp_utility_max=-1.0;

  for(ang=-90;ang<=90;ang+=Mem->CP_clear_ball_ang_step){

    Vector end=Mem->BallAbsolutePosition()+
      Vector(Mem->CP_clear_ball_max_dist,ang*signf(Mem->BallY()));
    //max utility at 30 degrees, and 0 at -90
    float utility=.5*(Sin(1.5*ang+45)+1);
    float exp_success=Mem->EvaluateClearAngle(end);
    float exp_utility=utility*exp_success;
    if(exp_utility>exp_utility_max){
      ang_max=ang;
      exp_utility_max=exp_utility;
    }
  }
  ang=GetNormalizeAngleDeg(signf(Mem->BallY())*ang_max-Mem->MyBodyAng());
  Mem->LogAction3(30,"clear_ball: target ang == %.1f",ang);

  actions.smartkick(2*Mem->SP_ball_speed_max, ang, SK_Safe);

}
/****************************************************************************/
void recover(void){
  Mem->LogAction3(10,"Recovering at stamina: %d",Mem->MyStamina());
  face_neck_and_body_to_ball();
}
//*******************************************************************
//********************************************************************
//********************************************************************
//****************************************************

/*****************************************************************************************/
/*****************************************************************************************/

ActionQueueRes scan_field_with_body()
{
  Mem->LogAction3(40,"scan_field_with_body (%d)",Mem->TimeToTurnForScan());
  if ( Mem->TimeToTurnForScan() ){
    turn(Mem->MyViewAngle() * 2 - Mem->CP_scan_overlap_angle);
    return AQ_ActionQueued;
  }
  else 
    return AQ_ActionNotQueued;
}

/*****************************************************************************************/
void get_on_sides()
{
  Mem->LogAction3(30, "get_on_sides (%f)",Mem->my_offside_line);

  eye.AddBall(1,RT_WithoutTurn,VW_Narrow);
  Vector ball=!Pos.TmInter()?Mem->BallAbsolutePosition():
    Dribble::PredictDribbleStopPosition(Pos.FastestTm(),Dribble::SelectDribbleTargetForTeammate(Pos.FastestTm()),0.6f);
  
  if(Pos.IsOffense()&&fabs(Mem->MyY()-Mem->BallY())<=Mem->SP_offside_area&&Mem->MyX()-ball.x<Mem->SP_offside_area+2.0f&&
     2.0f*Mem->my_offside_line-Mem->MyX()-Mem->BallX()>(Mem->SP_offside_area+2.0f)){
    Mem->LogAction2(50,"Try to avoid offside");
    Pos.MoveToPos(Vector(Mem->MyX(),Sign(Mem->MyY()-Mem->BallY())*32.0f),Mem->BallDistance()<20.0f?5.0f:0.0f,0.0);
    return;
  }
  if(Pos.IsDefense()&&Mem->MyStamina()<0.5f*Mem->SP_stamina_max/*||Mem->MyX()-Mem->my_offside_line>5.0f*/){

    Mem->LogAction2(50,"We in defense or i too far from offside line, so move not fast");
    Pos.MoveToPos(Vector(Mem->my_offside_line-1.0f,Mem->MyY()),7.0,0.0,Mem->GetMyStaminaIncMax()*0.9f);
    return;
  }
  Pos.MoveToPos(Vector(Mem->my_offside_line-1.0f,Mem->MyY()),30.0,Mem->BallDistance()<20.0f?3.0f:0.0f,false);
}
/******************************************************************************************/

void turn_neck_to_relative_angle(AngleDeg ang)
{
  turn_neck(GetNormalizeAngleDeg(ang - Mem->MyNeckRelAng()));
}

/*****************************************************************************************/

void scan_field_with_neck()
{
  Mem->LogAction3(40,"scan_field_with_neck (%d)",Mem->TimeToTurnForScan());
  if ( Mem->TimeToTurnForScan() ){
    if ( Mem->MyNeckRelAng() >= Mem->SP_max_neck_angle-1 ) /* take into account reporting error */
      turn_neck_to_relative_angle(Mem->SP_min_neck_angle);
    else 
      turn_neck(Mem->LimitTurnNeckAngle(Mem->MyViewAngle()*2 - Mem->CP_scan_overlap_angle));
  }
}

/*****************************************************************************************/

ActionQueueRes face_only_body_to_point(Vector point)
{
  /* don't turn neck */
  Mem->LogAction4(30,"facing only body to point (%.1f %.1f)",point.x,point.y);

  /* shouldn't actually have queued actions at this point */
  AngleDeg target_rel_ang = Mem->PredictedPointRelAngFromBodyWithQueuedActions(point);


  if ( fabs(target_rel_ang) < 1 ){
    Mem->LogAction2(40,"Already close enough");
    return AQ_ActionNotQueued;
  }
  turn( target_rel_ang );
  return AQ_ActionQueued;
}

/*****************************************************************************************/

void face_only_neck_to_point(Vector point)
{
  /* don't turn body */
  Mem->LogAction4(30,"facing only neck to point (%.1f %.1f)",point.x,point.y);

  AngleDeg target_rel_ang = Mem->PredictedPointRelAngFromBodyWithQueuedActions(point);

  if ( fabs(GetNormalizeAngleDeg(Mem->MyNeckRelAng() - target_rel_ang)) < 1 ){


    Mem->LogAction2(40,"Already close enough");

    return;
  }

  if ( Mem->CanSeeAngleFromBodyWithNeck(target_rel_ang) ){
    turn_neck( Mem->LimitTurnNeckAngle(target_rel_ang - Mem->MyNeckRelAng()) );
  }

  else
    Mem->LogAction5(30,"can't face point (%.1f %.1f) with only neck (%.1f)",
		    point.x,point.y, target_rel_ang);
}

/*****************************************************************************************/

ActionQueueRes face_neck_to_point(Vector point)
{
  /* face_neck can turn body if needed */
  Mem->LogAction4(30,"facing neck to point (%.1f %.1f)",point.x,point.y);

  AngleDeg target_rel_ang = Mem->PredictedPointRelAngFromBodyWithQueuedActions(point);

  if ( fabs(GetNormalizeAngleDeg(Mem->MyNeckRelAng() - target_rel_ang)) < 1 ){

    Mem->LogAction2(40,"Already close enough");
    return AQ_ActionNotQueued;
  }

  if ( Mem->CanFaceAngleFromBodyWithNeck(target_rel_ang) ){
    Mem->LogAction2(35,"can face with neck");
    turn_neck_to_relative_angle(target_rel_ang);
    return AQ_ActionNotQueued;
  }


  /* If can't do it with just neck, turn body as much as needed to face directly */
  AngleDeg max_turn = Mem->MaxEffectiveTurn();


  if ( fabs(target_rel_ang) < max_turn ) {
    Mem->LogAction2(35,"can't face with neck, can with body");
    turn(target_rel_ang);
    turn_neck_to_relative_angle(0);
    return AQ_ActionQueued;
  }

  Mem->LogAction2(35,"can't face with neck or body alone, turning both");  
  turn(target_rel_ang);
  target_rel_ang -= max_turn;  /* The neck target_ang */


  if ( target_rel_ang < Mem->SP_min_neck_angle ){
    Mem->LogAction2(40,"couldn't face all the way");  
    turn_neck_to_relative_angle(Mem->SP_min_neck_angle);
  }

  else if ( target_rel_ang > Mem->SP_max_neck_angle ){
    Mem->LogAction2(40,"couldn't face all the way");  
    turn_neck_to_relative_angle(Mem->SP_max_neck_angle);
  }
  else 

    turn_neck_to_relative_angle(target_rel_ang);

  return AQ_ActionQueued;
}

/*****************************************************************************************/

ActionQueueRes face_neck_and_body_to_point(Vector point)
{
  /* face_neck_and_body will turn both as much as possible to the point */
  Mem->LogAction4(30,"facing neck and body to point (%.1f %.1f)",point.x,point.y);


  AngleDeg max_turn = Mem->MaxEffectiveTurn();
  AngleDeg target_rel_ang = Mem->PredictedPointRelAngFromBodyWithQueuedActions(point);


  if ( fabs(GetNormalizeAngleDeg(Mem->MyNeckRelAng() - target_rel_ang)) < 1 && fabs(target_rel_ang) < 1 ){
    Mem->LogAction2(40,"Already close enough");
    return AQ_ActionNotQueued;
  }

  if ( fabs(target_rel_ang) < max_turn ) {
    Mem->LogAction2(35,"Can get both neck and body there");
    /* Can get both neck and body there */
    face_only_body_to_point(point);
    turn_neck_to_relative_angle(0);
    return AQ_ActionQueued;
  }

  /* Turn body as much as possible and try to get neck there */
  return face_neck_to_point(point);
}

/*****************************************************************************************/

ActionQueueRes face_only_body_to_player(char side, Unum num)
{
  if ( Mem->PlayerPositionValid(side, num) ){
    return face_only_body_to_point(Mem->PlayerAbsolutePosition(side,num));
  }
  else
    return scan_field_with_body();
}

/*****************************************************************************************/


void face_only_neck_to_player(char side, Unum num)
{
  if ( Mem->PlayerPositionValid(side,num) ){
    face_only_neck_to_point(Mem->PlayerAbsolutePosition(side,num));
  }
  else
    scan_field_with_neck();
}

/*****************************************************************************************/

ActionQueueRes face_neck_to_player(char side, Unum num)
{
  if ( Mem->PlayerPositionValid(side, num) ){
    return face_neck_to_point(Mem->PlayerAbsolutePosition(side,num));
  }
  else
    return scan_field_with_body();
}

/*****************************************************************************************/

ActionQueueRes face_neck_and_body_to_player(char side, Unum num)
{
  if ( Mem->PlayerPositionValid(side, num) ){
    return face_neck_and_body_to_point(Mem->PlayerAbsolutePosition(side,num));

  }
  else
    return scan_field_with_body();
}

/*****************************************************************************************/

ActionQueueRes face_only_body_to_opponent(Unum opponent){ 
  Mem->LogAction3(30,"facing only body to opponent %d",opponent); 
  return face_only_body_to_player(Mem->TheirSide, opponent);
}

/*****************************************************************************************/

void           face_only_neck_to_opponent(Unum opponent){
  Mem->LogAction3(30,"facing only neck to opponent %d",opponent);
  face_only_neck_to_player(Mem->TheirSide, opponent);
}

/*****************************************************************************************/

ActionQueueRes face_neck_to_opponent(Unum opponent){
  Mem->LogAction3(30,"facing neck to opponent %d",opponent);
  return face_neck_to_player(Mem->TheirSide, opponent);
}

/*****************************************************************************************/

ActionQueueRes face_neck_and_body_to_opponent(Unum opponent){
  Mem->LogAction3(30,"facing neck and body to opponent %d",opponent);
  return face_neck_and_body_to_player(Mem->TheirSide, opponent);
}

/*****************************************************************************************/

ActionQueueRes face_only_body_to_teammate(Unum teammate){ 
  Mem->LogAction3(30,"facing only body to teammate %d",teammate); 
  return face_only_body_to_player(Mem->MySide, teammate);
}


/*****************************************************************************************/

void           face_only_neck_to_teammate(Unum teammate){
  Mem->LogAction3(30,"facing only neck to teammate %d",teammate);
  face_only_neck_to_player(Mem->MySide, teammate);
}

/*****************************************************************************************/

ActionQueueRes face_neck_to_teammate(Unum teammate){
  Mem->LogAction3(30,"facing neck to teammate %d",teammate);
  return face_neck_to_player(Mem->MySide, teammate);

}

/*****************************************************************************************/

ActionQueueRes face_neck_and_body_to_teammate(Unum teammate){
  Mem->LogAction3(30,"facing neck and body to teammate %d",teammate);
  return face_neck_and_body_to_player(Mem->MySide, teammate);
}

/*****************************************************************************************/

ActionQueueRes face_only_body_to_ball()
{
  Mem->LogAction2(30,"facing body to ball");
  if ( Mem->BallPositionValid() ) {
    return face_only_body_to_point(Mem->BallPredictedPositionWithQueuedActions());
  }
  else 
    return scan_field_with_body();
}

/*****************************************************************************************/

void face_only_neck_to_ball()
{
  Mem->LogAction2(30,"facing only neck to ball");
  if ( Mem->BallPositionValid() ) {
    face_only_neck_to_point(Mem->BallPredictedPositionWithQueuedActions());
  }
  else 
    scan_field_with_neck();
}

/*****************************************************************************************/

ActionQueueRes face_neck_to_ball()
{
  Mem->LogAction2(30,"facing neck to ball");
  if ( Mem->BallPositionValid() ) {
    return face_neck_to_point(Mem->BallPredictedPositionWithQueuedActions());
  }
  else 
    return scan_field_with_body();

}

/*****************************************************************************************/

ActionQueueRes face_neck_and_body_to_ball()
{

  Mem->LogAction2(30,"facing neck and body to ball");
  if ( Mem->BallPositionValid() ) {
    return face_neck_and_body_to_point(Mem->BallPredictedPositionWithQueuedActions());
  }
  else 
    return scan_field_with_body();
}


/*****************************************************************************************/
/* if the arg is DT_all, we always dodge, otherwise we only dodge if they don't have ball */
void get_ball()
{
  if ( !Mem->MyConf() || !Mem->BallPositionValid() ) my_error("not enough info to get ball");
  //temp!!!
  DodgeType dodge = DT_none;//DT_only_with_ball;

  if ( !Mem->BallMoving() ){
    Mem->LogAction2(30, "get_ball: ball not moving, going to it's pos");
    if ( go_to_point(Mem->BallAbsolutePosition(),0,100,dodge) == AQ_ActionNotQueued ){
      my_error("already there???");
      face_neck_and_body_to_ball();
    }
    face_only_neck_to_ball();
  }
  else {
    if ( !Mem->MyInterceptionAble() ){
      Mem->LogAction2(30, "get_ball: going to the moving ball, but can't?");
      my_error("Can't get to the ball");

      face_neck_and_body_to_ball();
    } /*else if (Mem->MyInterceptionNumberCycles() == 1) {
      // we're just one dash away, so just do it 
      float signf=1.0f;//Mem->MyPredictedPosition(1,Mem->CorrectDashPowerForStamina(-Mem->MyInterceptionDashPower())).
        //dist(Mem->BallPredictedPosition(1))<Mem->GetMyKickableArea()*0.8f?-1.0f:1.0f;
      Mem->LogAction2(30, "get_ball: going to the moving ball, just dashing 1 cycle");
      dash(Mem->CorrectDashPowerForStamina(signf*Mem->MyInterceptionDashPower()));
      face_only_neck_to_ball();
    } */else{
      bool res;
      if(Mem->MyInterceptionResult()==BI_OnlyTurn){
	Mem->LogAction4(30,"get_ball: i need make only turn to point (%.2f,%.2f)",
			Mem->MyInterceptionPoint().x,Mem->MyInterceptionPoint().y);
	face_only_body_to_point(Mem->MyInterceptionPoint());
	face_only_neck_to_point(Mem->BallPredictedPosition());
	return;
      }
      if(Mem->MyInterceptionDashPower()<0)
	res=Pos.MoveBackToPos(Mem->MyInterceptionPoint(),Mem->MyInterceptionAngleError(),5.0f,Mem->MyInterceptionDashPower());
      else
	res=Pos.MoveToPos(Mem->MyInterceptionPoint(),Mem->MyInterceptionAngleError(),5.0,true,Mem->MyInterceptionDashPower());
      if(!res){
	Mem->LogAction2(30, "get_ball: going to the moving ball, and already there!");
	//my_error("already there (moving) ???");
	face_neck_and_body_to_point(Mem->BallPredictedPosition());
      } else {
	Mem->LogAction4(30, "get_ball: going to the moving ball (%d) pow %.2f",
			Mem->MyInterceptionNumberCycles(), Mem->MyInterceptionDashPower());
	face_only_neck_to_ball();

      }
    }
    
  }
  //add by AI:что бы не вылетать за границы поля
  Vector pred=Mem->MyPredictedPositionWithQueuedActions();
  if(fabs(pred.y)>=Mem->SP_pitch_width/2)
    Pos.StopNow();
}

/*****************************************************************************************/

void stop_ball()
{
  actions.stopball();
}

/*****************************************************************************************/

/*****************************************************************************************/










/*****************************************************************************************/

ActionQueueRes go_to_point( Vector p, float buffer, float dash_power, DodgeType dodge,bool to_ball )

{
  if( to_ball==true ) Mem->LogAction2(300,"to_ball");
  Mem->LogAction5(30, "go_to_point %d (%.1f %.1f)",dodge, p.x,p.y);
  if ( !Mem->MyConf() ) my_error("Can't go to a point if not localized");

  if ( Mem->DistanceTo(p) < buffer ){

    if ( Mem->SP_use_offside && fabs(Mem->MyX() - Mem->my_offside_line) < 5 ){ /* hack */
      Unum opp = Mem->FurthestForwardOpponent();
      if ( opp != Unum_Unknown && Mem->OpponentPositionValid(opp) < .9 ){ /* hack */
	Mem->LogAction2(40, "go_to_point: looking for offsides line");
	return face_neck_to_opponent(opp);
      }
    }
    Mem->LogAction2(40, "go_to_point: already at the point");
    return AQ_ActionNotQueued;
  }

  if ( Mem->PlayMode == PM_Their_Goal_Kick && Mem->MyPos() != p ){
    /* if ( Mem->TheirPenaltyArea.IsWithin(p) ){
       my_error("Can't go into their penalty area on a goal kick!"); */
    Line l = LineFromTwoPoints(Mem->MyPos(),p);
    Vector intersection = AdjustPtToRectOnLine(Mem->MyPos(),Mem->TheirPenaltyArea,l);
    if ( intersection != Mem->MyPos() && l.InBetween(intersection,Mem->MyPos(),p)){
      /* Need to go around the rectangle */
      Mem->LogAction2(40, "go_to_point: moving around penalty area");
      Vector target;
      if ( Mem->MyX() < Mem->TheirPenaltyArea.LeftX() )
	target = Vector(Mem->TheirPenaltyArea.LeftX()-3,p.y);
      else if ( Mem->MyY() > 0 )
	target = Vector(Mem->TheirPenaltyArea.LeftX()-3,Mem->TheirPenaltyArea.BottomY()+3 );

      else
	target = Vector(Mem->TheirPenaltyArea.LeftX()-3,Mem->TheirPenaltyArea.TopY()-3 );
      if(p==target){
        Mem->LogAction2(10,"go_to_point: AI- may be infinity cycle, so go out");
        return AQ_ActionNotQueued;
      }
      go_to_point( target, 0, dash_power, dodge );
      return AQ_ActionQueued;
    }

  }


  float target_ang  = GetNormalizeAngleDeg((p - Mem->MyPredictedPosition()).dir() - Mem->MyBodyAng());
  float target_dist = Mem->DistanceTo(p);

  if ( dodge != DT_none ){ /* dodge players */
    PlayerObject *player;
    float    dodge_dist = Min(Mem->CP_dodge_distance_buffer,target_dist);
    AngleDeg dodge_ang  = Mem->CP_dodge_angle_buffer;
    if ( (player = Mem->GetPlayerWithin( dodge_dist, dodge_ang, 0, target_ang - dodge_ang)) != NULL &&
	 (dodge!=DT_unless_with_ball || 
	  (Mem->BallPositionValid() && 
	   player->get_abs_pos().dist(Mem->BallAbsolutePosition()) > Mem->GetPlayerKickableArea(player->side,player->unum))) &&

	 (dodge!=DT_only_with_ball ||
	  (Mem->BallPositionValid() && 
	   player->get_abs_pos().dist(Mem->BallAbsolutePosition()) <= Mem->GetPlayerKickableArea(player->side,player->unum))) ){
      Mem->LogAction2(40, "go_to_point: dodging right");
      /*if ( Mem->NumPlayersWithin( dodge_dist, 2*dodge_ang) ){*/

      /* Target at dist player_size, so no players will be within in the next iteration ==> dash */
      Vector new_target = Mem->BodyPolar2Gpos(Mem->GetMyPlayerSize(),player->get_ang_from_body() + Mem->CP_dodge_angle);
      if ( new_target == p )
	my_error("Dodging isn't changing the point!");
      go_to_point(new_target,0,Mem->CP_dodge_power,DT_none); 
      /*}
	else{
	dash(Mem->CorrectDashPowerForStamina(Min(dash_power,Mem->CP_dodge_power)));
	}*/
      return AQ_ActionQueued;
    }
    if ( (player = Mem->GetPlayerWithin( dodge_dist, dodge_ang, 0, target_ang + dodge_ang)) != NULL &&
	 (dodge!=DT_unless_with_ball || 
	  (Mem->BallPositionValid() && 
	   player->get_abs_pos().dist(Mem->BallAbsolutePosition()) > Mem->GetPlayerKickableArea(player->side,player->unum))) &&
	 (dodge!=DT_only_with_ball ||
	  (Mem->BallPositionValid() && 

	   player->get_abs_pos().dist(Mem->BallAbsolutePosition()) <= Mem->GetPlayerKickableArea(player->side,player->unum))) ){
      Mem->LogAction2(40, "go_to_point: dodging left");
      /*if ( Mem->NumPlayersWithin( dodge_dist, 2*dodge_ang) ){*/
      /* Target at dist player_size, so no players will be within in the next iteration ==> dash */
      Vector new_target = Mem->BodyPolar2Gpos(Mem->GetMyPlayerSize(),player->get_ang_from_body() - Mem->CP_dodge_angle);
      if ( new_target == p )
	my_error("Dodging isn't changing the point!");

      go_to_point(new_target,0,Mem->CP_dodge_power,DT_none);
      /*}
	else{
	dash(Mem->CorrectDashPowerForStamina(Min(dash_power,Mem->CP_dodge_power)));
	}*/
      return AQ_ActionQueued;
    }
  }
  if ( fabs(target_ang) > (to_ball?Mem->MyInterceptionAngleError():Mem->CP_max_go_to_point_angle_err) ||
       (Mem->PlayMode == PM_Their_Goal_Kick && 
	Mem->TheirPenaltyArea.IsWithin(Mem->MyPredictedPosition(1,dash_power))) ){
    Mem->LogAction3(50, "go_to_point: turning %f", target_ang);
    turn(target_ang);
    return AQ_ActionQueued;
  }

  dash_power = Mem->CorrectDashPowerForStamina(dash_power);
  if ( dash_power > 0 ){
    //    if(to_ball){
    //      if(Mem->MyPredictedPosition(1).dist(p)<=buffer||Mem->MyPredictedPosition(2).dist(p)<=buffer){
    //        Mem->LogAction2(10,"I will be at target point without dash");
    //        return AQ_ActionQueued;
    //      }
    //    }
    Mem->LogAction3(50, "go_to_point: dashing %f", dash_power);
    dash( dash_power );
    return AQ_ActionQueued;
  }
  else
    {my_stamp; printf("recovering\n");} 

  Mem->LogAction2(50, "go_to_point: doing nothing?");
  return AQ_ActionNotQueued;
}
