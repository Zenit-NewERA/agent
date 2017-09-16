/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : dribble.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.22 $
 *
 *    $Id: dribble.C,v 2.22 2004/06/26 13:05:53 anton Exp $
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

#include <cmath>
#include "client.h"
#include "kick.h"
#include "dribble.h"
#include "behave.h"
#include "Handleball.h"
#include <vector>

//#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#define DebugDrib(x) x
#define DebugDrib2(x) x
#else
#define DebugDrib(x)
#define DebugDrib2(x)
#endif

#define BUFFER 4
/*****************************************************************************/
//Замечания по проиретету дриблинга. Чем он выше тем агрессивнее ведем мяч (сейчас в принципе не реализовано).
//Если приоритет >=0.7 то проверка на подкат у противника не производится

const float Dribble::too_close_to_side_line=30.0f;//if fabs(MyY())>too_close_to_side_line then attention
const bool Dribble::watch_way_to_go=false;
const int Dribble::max_num=5;//maximum of avoid opponents
const int Dribble::max_cycles_to_run=20;
/*****************************************************************************/
Dribble::Dribble(const Vector& pos,float p):priority(p),trace(none){
  orig_dribble_angle=dribble_angle=Mem->AngleToGlobal(pos);
  StrongForward=false;
  dist_stop_controlling_dribble=(1-priority)*10.0f;
  dribble_target=pos;
  dribble_turn_ang_err=30.0f;
  
  dist_line1=Mem->GetMyPlayerSize()+0.1f;
  dist_line2=(Mem->GetMyPlayerSize()+Mem->GetMyKickableMargin())*.77f;
  reliable_kick_area=Mem->GetMyKickableArea()*0.855f;//(Mem->GetMyPlayerSize()+Mem->GetMyKickableMargin())*.9f;
}
///////////
Dribble::Dribble(AngleDeg ang,float p){
  Dribble(Mem->FieldRectangle.RayIntersection(Ray(Mem->MyPos(),ang)),p);
}
///////////
AngleDeg Dribble::SetDribbleAngle(AngleDeg da){
  float temp=dribble_angle;
  dribble_angle=da;
  dribble_target=Mem->FieldRectangle.RayIntersection(Ray(Mem->MyPos(),da));
  return temp;
}//return old value
//////////
Vector Dribble::SetDribblePos(const Vector& pos){
  orig_dribble_angle=dribble_angle=Mem->AngleToGlobal(pos);
  Vector old=dribble_target;
  dribble_target=pos;
  return old;
}
///////////////////////////////////////////////////////////////////////////////////
bool Dribble::kick_to_myself_in_progress(){
  if(Mem->pass_to_myself){
    if((Mem->BallKickable()&&!Mem->kick_in_progress)||Mem->PlayMode!=PM_Play_On||!Mem->MyInterceptionAble()){
      Mem->pass_to_myself=false;
      return false;
    }
    Unum num=Pos.FastestTm();
    //other teammate must go to ball(may be say to him?)
    if(num!=Unum_Unknown&&num!=Mem->MyNumber&&Mem->TeammatePositionValid(num)>0.95&&
       (Mem->MyInterceptionNumberCycles()-Pos.TmCycles()>3||Pos.TmCycles()<=3)){
      Mem->pass_to_myself=false;
      return false;
    }
    for(int i=1;i<=Mem->SP_team_size;i++){
      if(Mem->TeammatePositionValid(i)&&Mem->BallKickableForTeammate(i)&&Mem->TeammatePositionValid(i)>0.95){
	Mem->pass_to_myself=false;
	return false;
      }
      if(Mem->OpponentPositionValid(i)&&Mem->BallKickableForOpponent(i)&&Mem->OpponentPositionValid(i)>0.95){
	Mem->pass_to_myself=false;
	return false;
      }
    }
    Mem->LogAction2(10,"Kick to myself in progress:go to ball with max vel");
    Mem->kick_in_progress=FALSE;
    if(Mem->MyX()>30.0f)
      eye.AddOpponent(Mem->TheirGoalieNum,2);
    get_ball();
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////////
//предполагаем что отношение dash/kick для нашего CD составляет ~2.7
Vector Dribble::PredictDribbleStopPosition(Unum tm,Vector target,float prior,bool print_log,int* cycles)
{
  const float DASH_TO_KICK_COEFF=2.7f/(2.7f+1.0f);//Формула: (dash+1-kick)/(dash+2-kick) для kick=1
  const int MAX_CYC=10;
  bool agressive=false;
  if(prior>0.5f||Pos.GetPlayerType(tm)>=PT_Midfielder)
    agressive=true;
  int cyc_begin;
  Vector start;
  if(Mem->TeammateInterceptionAble(tm)){
    start=Mem->TeammateInterceptionPoint(tm);
    cyc_begin=Mem->TeammateInterceptionNumberCycles(tm);
  }else{
    start=Mem->BallAbsolutePosition();
    cyc_begin=0;
  }
  AngleDeg ang=Mem->AngleToGlobal(target);
  
  if(!Mem->FieldRectangle.IsWithin(start+Polar2Vector(1.5f,ang))){
    if(print_log)
      Mem->LogAction2(10,"PredictDribbleStopPosition:may lost ball outside of field so can not go controlling dribble");
    if(cycles!=0)
      *cycles=cyc_begin;
    return start;
  }
  Unum closestOpp=Mem->ClosestOpponentTo(start);
  if(closestOpp!=Unum_Unknown){
    if(!agressive&&Mem->OpponentDistanceTo(closestOpp,start)<(1-prior)*10.0f){
      if(print_log)
	Mem->LogAction4(10,"PredictDribbleStopPosition: opp %.0f will stop not agressive dribble with dist %.2f",
			float(closestOpp),Mem->OpponentDistanceTo(closestOpp,start));
      return start;
    }else if(agressive&&((closestOpp!=Mem->TheirGoalieNum&&Mem->OpponentDistanceTo(closestOpp,start)<1.5f)||
			 (closestOpp==Mem->TheirGoalieNum&&Mem->OpponentDistanceTo(closestOpp,start)<=
			  (IsGoalieActive()?(1-prior)*10.0f:4.0f)))){
      if(print_log)
	Mem->LogAction4(10,"PredictDribbleStopPosition: opp %.0f will stop agressive dribble with dist %.2f",
			float(closestOpp),Mem->OpponentDistanceTo(closestOpp,start));
      if(cycles!=0)
	*cycles=cyc_begin;
      return start;
    }
  }
  Vector acc_vec=Polar2Vector(DASH_TO_KICK_COEFF*Mem->SP_max_power*Mem->GetTeammateDashPowerRate(tm),ang);
  Vector velocity=Mem->TeammateVelocityValid(tm)?Mem->TeammateAbsoluteVelocity(tm):Vector(0.0f,0.0f);
  Vector position=Mem->TeammateAbsolutePosition(tm);
  Vector last_position;
  
  for(int i=1;i<=MAX_CYC;i++){
    velocity += acc_vec;
    if ( velocity.mod() > Mem->GetTeammatePlayerSpeedMax(tm) )
      velocity *= ( Mem->GetTeammatePlayerSpeedMax(tm)/velocity.mod() );
    last_position=position;
    position += velocity;
    velocity *= Mem->GetTeammatePlayerDecay(tm);
    Unum opp=Mem->ClosestOpponentTo(position);
    if(opp!=Unum_Unknown){
      int opp_cyc=Mem->OpponentPredictedCyclesToPoint(opp,position,Mem->SP_max_power,1.0f);
      if(opp_cyc<=i){
	if(print_log)
	  Mem->LogAction4(10,"PredictDribbleStopPosition: opp %.0f stop dribble after %.0f cycles",
			  float(opp),float(opp_cyc));
	if(cycles!=0)
	  *cycles=i-1;
	return last_position;
      }
    }
  }
  if(print_log)
    Mem->LogAction2(10,"PredictDribbleStopPosition: no opp can stop dribble befor 10 cycles");
  if(cycles!=0)
    *cycles=MAX_CYC;
  return position;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Dribble::hold_ball(int max_close_opp){
  const int step=10;//degrees
  float angles[360/step];
  for(int angle=0;angle<360;angle+=step)
    angles[angle/step]=0.0f;
  float max=0.0f,opt_angle=0.0f;
  ClosePlayer close_players[5];
  int num_to_avoid=SelectAvoidOpponents(close_players,7.0f);
  if(!Mem->BallVelocityValid()){
    Mem->LogAction2(10,"HoldBall: ball vel not valid so may be very bad!!");
    face_neck_to_ball();
    return false;
  }
  if(num_to_avoid>max_close_opp){
    Mem->LogAction3(10,"HoldBall:too much close opponents: %d. Can not hold ball",num_to_avoid);
    return false;
  }
  if(num_to_avoid==0){
    Mem->LogAction2(10,"HoldBall: i have no opponent to avoid so turn ball in front of us");
    kick.turnball(0.0,TURN_NONE,SK_Fast,Mem->GetMyOptCtrlDist());
    scan_field();
    return true;
  }
  Mem->LogAction3(10,"HoldBall: must avoid %d opponents",num_to_avoid);
  float dist=.9f;
  for(int i=0;i<num_to_avoid;i++){
    Vector pos=close_players[i].pos;
    if(close_players[i].body_ang_valid){//try to predict opponent position
      pos=GetPredictedOpponentPosition(close_players[i].opp,pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,close_players[i].vel);
    }
    for(int angle=0;angle<360;angle+=step){
      Vector new_pos=Polar2Vector(dist,GetNormalizeAngleDeg(angle))+Mem->MyPredictedPosition(1);
      angles[angle/step]+=(pos-new_pos).mod()/dist_stop_controlling_dribble;
      if(!Mem->FieldRectangle.IsWithin(new_pos))
	angles[angle/step]=-200.0f;
      if(close_players[i].body_ang_valid&&Covered(close_players[i].opp,new_pos,pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,
						  close_players[i].vel,(close_players[i].opp==Mem->TheirGoalieNum?true:false))){
	angles[angle/step]=-100.0f;
      }
    }
  }
  for(int angle=0;angle<360;angle+=step){
    if(max<angles[angle/step]){
      opt_angle=angle;
      max=angles[angle/step];
    }
  }
  if(max==0.0f){
    Mem->LogAction2(10,"HoldBall: i can not hold ball");
    return false;
  }
  dist=.8f;
  for(int i=0;i<num_to_avoid;i++){
    bool ok=false;
    if(close_players[i].body_ang_valid>=0.95f){
      for(float step=.8f;step<=.9f;step+=0.025f){
        if(!Covered(close_players[i].opp,Polar2Vector(step,GetNormalizeAngleDeg(opt_angle))+Mem->MyPredictedPosition(1),
		    close_players[i].pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,close_players[i].vel,
		    (close_players[i].opp==Mem->TheirGoalieNum?true:false))&&dist<=step){
	  dist=step;
	  ok=true;
	  break;
        }
      }
      if(ok==false){
        dist=.9f;
        break;
      }
    }else{
      Mem->LogAction3(10,"HoldBall:body angle of opponent %d is not valid",int(close_players[i].opp));
      dist=.9f;
      break;
    }
  }
  Mem->LogAction4(10,"HoldBall: can avoid all opp to angle %.2f and dist %.2f",opt_angle,dist);
  kick.turnball(GetNormalizeAngleDeg(opt_angle-Mem->MyBodyAng()),TURN_NONE,SK_Fast,dist);
  return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool Dribble::OpponentBehindUs(Vector pos,float buffer,Unum opp) const{
  Line l;
  Ray r(Mem->MyPos(),dribble_angle);
  l.LineFromRay(r);
  Vector point=l.ProjectPoint(pos);
  Vector target=r.RectangleIntersection(Mem->FieldRectangle.expand(5.0f));
  bool res=(!l.InBetween(point, Mem->MyPos(), target))&&((Mem->MyPos()-pos).mod()>buffer&&point.dist(Mem->MyPos())>3.0f);
  if(res){
    if(opp!=Unum_Unknown)
      Mem->LogAction6(100,"Opponent %.2f (pos :(%.2f,%.2f), valid: %.2f) is behind us",float(opp),
		      pos.x,pos.y,Mem->OpponentPositionValid(opp));
    else
      Mem->LogAction4(100,"Teamless player pos: (%.2f,%.2f) is behind us",
		      pos.x,pos.y);
  }
  return res;
}
///////////////////////////////////////////////////////////////////
bool Dribble::CanControllingDribble() const{
  
  if(Mem->NumTeamlessPlayers()>0){
    if(Mem->DistanceTo(Mem->ClosestTeamlessPlayerPosition())<6.0f*(1-priority)){
      Mem->LogAction2(10,"Dribble:Teamless player is close to me, so can not go controlling dribble");
      return false;
    }
  }
  if(!Mem->FieldRectangle.IsWithin(Mem->BallAbsolutePosition()+Polar2Vector(1.5f,dribble_angle))){
    Mem->LogAction2(10,"Dribble:may lost ball outside of field so can not go controlling dribble");
    return false;
  }
  Unum Opp[11];
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,Mem->MyPos(),Opp);
  for(int i=0;i<numClosestOpp;i++){
    if(Mem->OpponentDistanceTo(Opp[i],Mem->MyPos())<=dist_stop_controlling_dribble){
      if(!Mem->OpponentTackling(Opp[i])){
        Mem->LogAction3(50,"Dribble: can not go simple controlling dribble becouse opp %d is too close",Opp[i]);
        return false;
      }
    }else
      return true;
  }
   
  return true;
}
///////////////////////////////////////////////////////////////////
bool Dribble::CanAgressiveControllingDribble(float side,bool print_log) {
//   if(Mem->MyStamina()<=Mem->SP_stamina_max*0.45f&&Mem->MyX()*side<35.0f){
//     Mem->LogAction2(10,"CanAgressiveControllingDribble: i`m tired so can not go agressive contrilling dribble");
//     return false;
//   }
//   
//   if(Mem->NumTeamlessPlayers()>0){
//     if(Mem->EstimatedCyclesToSteal(Unum_Teamless)<2){
//       Mem->LogAction2(10,"CanAgressiveControllingDribble:Teamless player is close to me, so can not go controlling dribble");
//       return false;
//     }
//   }
  
  if(Mem->NumTeamlessPlayers()>0){
    if(Mem->DistanceTo(Mem->ClosestTeamlessPlayerPosition())<6.0f*(1-priority)){
      if(print_log)
	Mem->LogAction2(10,"Dribble:Teamless player is close to me, so can not go controlling dribble");
      return false;
    }else
      if(print_log)
	Mem->LogAction3(10,"CanAgressiveControllingDribble: dist to closest teamless player is %.2f",
			float(Mem->DistanceTo(Mem->ClosestTeamlessPlayerPosition())));
    
  }
  if(!Mem->FieldRectangle.IsWithin(Mem->BallAbsolutePosition()+Polar2Vector(1.5f,dribble_angle))){
    if(print_log)
      Mem->LogAction2(10,"CanAgressiveControllingDribble:may lost ball outside of field so can not go controlling dribble");
    return false;
  }
  Vector BallPredPos;
  if(controlling_dribbling(&BallPredPos)==AQ_ActionNotQueued)
    return false;

  if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>=0.99f&&!IsGoalieActive(side)&&
     Mem->OpponentX(Mem->TheirGoalieNum)*side<BallPredPos.x*side+2.5f&&fabs(dribble_angle)<45.0f&&fabs(BallPredPos.y)>7.0f){
    if(print_log)
      Mem->LogAction2(10,"CanAgressiveControllingDribble: opp goalie will block us, so can not go dribble");
    return false;
  }
  if(print_log)
    Mem->LogAction2(10,"CanAgressiveControllingDribble: check position to dribble");
  Positioning::CheckType type=(Pos.GetMyType()<=PT_Defender&&Mem->BallX()<30.0f&&priority<0.5f)?
    Positioning::CT_Normal:Positioning::CT_Agressive;
  if(Mem->PlayMode!=PM_Play_On)
    type=Positioning::CT_PenaltyShoot;
  bool ret;
  if(priority>=0.7f)
    ret=Pos.CheckWithoutTackle(BallPredPos,type,print_log,side,dist_stop_controlling_dribble);
  else
    ret=Pos.CheckWithTackle(BallPredPos,type,print_log,side,dist_stop_controlling_dribble);
  return ret;
}

///////////////////////////////////////////////////////////////////
//then return (max_num+1) - we have more then max_num close players
int Dribble::SelectAvoidOpponents(ClosePlayer* opp,float max_dist){
  int num=0;
  for (int i=0; i<Mem->NumTeamlessPlayers(); i++){
    Vector pos(Mem->TeamlessPlayers()[i]->get_x(),Mem->TeamlessPlayers()[i]->get_y());
    if(Mem->DistanceTo(pos)<=max_dist){
      if(num==max_num)
	return num+1;
      opp[num].opp=Unum_Unknown;
      opp[num].pos=pos;
      if(Mem->TeamlessPlayers()[i]->body_ang_valid()){
	opp[num].body_ang_valid=Mem->TeamlessPlayers()[i]->body_ang_valid();
	opp[num].body_angle=Mem->TeamlessPlayers()[i]->get_abs_body_ang();
      }else
	opp[num].body_ang_valid=0.0f;
      if(Mem->TeamlessPlayers()[i]->vel_valid())
	opp[num].vel=Mem->TeamlessPlayers()[i]->get_abs_vel();
      else
	opp[num].vel=Vector(.0f,.0f);
      num++;
    }
  }
  for(int i=1;i<=Mem->SP_team_size;i++){
    if(Mem->OpponentPositionValid(i)&&Mem->DistanceTo(Mem->OpponentAbsolutePosition(i))<=max_dist){
      if(num==max_num)
        return num+1;
      opp[num].opp=i;
      opp[num].pos=Mem->OpponentAbsolutePosition(i);
      if(Mem->OpponentBodyAngleValid(i)){
	opp[num].body_ang_valid=Mem->OpponentBodyAngleValid(i);
	opp[num].body_angle=Mem->OpponentAbsoluteBodyAngle(i);
      }else
	opp[num].body_ang_valid=0.0f;
      if(Mem->OpponentVelocityValid(i))
	opp[num].vel=Mem->OpponentAbsoluteVelocity(i);
      else
	opp[num].vel=Vector(.0f,.0f);
      num++;
    }
  }
  return num;
}
///////////////////////////////////////////////////////////////////
Vector Dribble::SelectDribbleTargetForTeammate(Unum tm,bool print_log)
{
  if(tm==Unum_Unknown||!Mem->TeammatePositionValid(tm)){
    my_error("SelectDribbleTargetForTeammate::Wrong %d teammate",tm);
    return Vector(0,0);
  }
  if(tm==Mem->MyNumber)
    return SelectDribbleTarget(Mem->MyPos(),Mem->MyBodyAng(),print_log);
  else
    return SelectDribbleTarget(Pos.GetTmPos(tm),
			       Mem->TeammateBodyAngleValid(tm)>0.9f?Mem->TeammateAbsoluteBodyAngle(tm):777.0f,
			       print_log);
}
//////////////////////////////////////////////////////////////////////
Vector Dribble::SelectDribbleTarget(Vector tm_pos,AngleDeg body_ang,bool print_log,float side_sign){
  float FORWARD_ADDITION=10.0f*side_sign;
  const float SIDE_ADDITION=7.0f;
  const AngleDeg STOP_ANGLE=45.0f;
  const AngleDeg STOP_ANGLE_IN_PENALTY_AREA=30.0f;
  const float MAX_X=48.0;

  static Time last_calc=0;
  static AngleDeg last_ang=0.0;
	
  if(fabs(tm_pos.x)<30.0f&&Mem->NumOpponentsInCone(0.6f,Vector(52.5*side_sign,tm_pos.y))>1){
    AngleDeg res_ang=0;
    Vector res=Pos.GetOptimalPointOnLine(tm_pos,Vector(tm_pos.x+FORWARD_ADDITION,tm_pos.y),
					 Vector(tm_pos.x+FORWARD_ADDITION,Min(30.0f,tm_pos.y+SIDE_ADDITION)),
					 &res_ang,STOP_ANGLE);
    AngleDeg optimal_dir=0.0f;
    if(res_ang>=STOP_ANGLE){
      optimal_dir=(res-tm_pos).dir();
    }else{
      AngleDeg res_ang2=0.0f;
      Vector res2=Pos.GetOptimalPointOnLine(tm_pos,Vector(tm_pos.x+FORWARD_ADDITION,tm_pos.y),
					    Vector(tm_pos.x+FORWARD_ADDITION,Max(-30.0f,tm_pos.y-SIDE_ADDITION)),
					    &res_ang2,STOP_ANGLE);
      if(res_ang2>res_ang){
        optimal_dir=(res2-tm_pos).dir();
      }else{
        optimal_dir=(res-tm_pos).dir();
      }
    }
    if(Mem->CurrentTime-last_calc<10&&GetNormalizeAngleDeg(fabs(last_ang-optimal_dir))<45.0f&&
       fabs(body_ang-last_ang)<=30.0f&&fabs(tm_pos.y)<30.0f){
      if(print_log)
	Mem->LogAction3(10,"Can use last optimal angle %.2f",last_ang);
      optimal_dir=last_ang;
    }else{
      last_ang=optimal_dir;
    }
    last_calc=Mem->CurrentTime;
    if(print_log)
      Mem->LogAction4(10,"SelectDribbleTarget: selct optimal angle %.2f that have wide angle %.2f",optimal_dir,res_ang);
    return Mem->FieldRectangle.RayIntersection(Ray(tm_pos,optimal_dir));
  }
  //MyX()>=30.0
  Vector target_position;
  if(body_ang!=777.0f&&Mem->FieldRectangle.shrink(4.0f).IsWithin(tm_pos)&&Mem->BallKickable()){
    Vector my_ray_pos=Mem->FieldRectangle.shrink(4.0f).RayIntersection(Ray(tm_pos,body_ang));
    if(fabs(my_ray_pos.y)<=10.0f&&signf(my_ray_pos.x)==side_sign){
      if(print_log)
	Mem->LogAction3(10,"SelectDribbleTarget: go forward without turn becouse my_ray_pos.y=%.2f",
			fabs(my_ray_pos.y));
      return my_ray_pos;
    }
  }
  
  if(fabs(tm_pos.y)<=Mem->SP_goal_width/2){
    if(print_log)
      Mem->LogAction2(10,"SelectDribbleTarget: i`m in central zone, so go direct to central of goal");
    target_position= Vector(MAX_X*side_sign,0.0);
  }else{
    float sign=Sign(tm_pos.y);
    AngleDeg res_ang=0.0f;
    float x=(Mem->SP_pitch_length/2-Mem->SP_goal_area_length)*side_sign;
    if(side_sign*(tm_pos.x)>side_sign*(x-2.0f)){//hack
      target_position= Vector(x,0.0f);
    }else{
      Vector res=Pos.GetOptimalPointOnLine(tm_pos,Vector(x,sign*Mem->SP_goal_width/2),
					   Vector(x,sign*(Mem->SP_penalty_area_width/2-3.0f)),
					   &res_ang,STOP_ANGLE_IN_PENALTY_AREA);
      AngleDeg optimal_dir=(res-tm_pos).dir();
      if(print_log)
	Mem->LogAction4(10,"SelectDribbleTarget: in their penalty area select angle %.2f that have wide angle %.2f",
			optimal_dir,res_ang);
      res= Mem->FieldRectangle.RayIntersection(Ray(tm_pos,optimal_dir));
      target_position=Vector(min(res.x*side_sign,MAX_X)*side_sign,res.y);
    }
  }
  Unum opp=Mem->ClosestOpponentToBall();
  if(body_ang!=777.0f&&opp!=Unum_Unknown&&Mem->OpponentDistanceToBall(opp)<=4.0f&&
     Mem->FieldRectangle.shrink(4.0f).IsWithin(tm_pos)&&Mem->BallKickable()){
    AngleDeg opp_ang=(Mem->OpponentAbsolutePosition(opp)-tm_pos).dir();
    AngleDeg ang1=fabs(GetNormalizeAngleDeg(body_ang-opp_ang));
    AngleDeg ang2=(fabs(GetNormalizeAngleDeg((target_position-tm_pos).dir()-opp_ang)))+5.0f;
    if(ang1>=ang2){
      if(print_log)
	Mem->LogAction6(10,"SelectDribbleTarget:opp %.0f is close (dist %.2f), so avoid turn (%.2f>=%.2f)",
			float(opp),Mem->OpponentDistanceToBall(opp),ang1,ang2);
      target_position=Mem->FieldRectangle.shrink(4.0f).RayIntersection(Ray(tm_pos,body_ang));
      if(signf(target_position.x-tm_pos.x)!=side_sign){
	target_position=Mem->BallAbsolutePosition()+Polar2Vector(4.0f,body_ang);
	target_position.y=min(30.0f,target_position.y)*signf(target_position.y);
      }
    }else
      if(print_log)
	Mem->LogAction6(10,"SelectDribbleTarget:opp %.0f is close (dist %.2f), but (%.2f<%.2f), so not avoid turn",
			float(opp),Mem->OpponentDistanceToBall(opp),ang1,ang2);
  }
  
  return target_position;
}
///////////////////////////////////////////////////////////////////
bool Dribble::GoBaby(DribbleType type,float side){
  ClosePlayer close_players[5];

  for(int i=1;i<=Mem->SP_team_size;i++){
    if(!Mem->OpponentPositionValid(i))
      OppIgnore[i-1]=true;
    else
      if(OpponentBehindUs(Unum(i)))
	OppIgnore[i-1]=true;
      else
	OppIgnore[i-1]=false;
  }
  float dist=Mem->DistanceTo(Mem->BallAbsolutePosition());
  if(dist>Mem->GetMyKickableArea()*0.95f){
    float pred_dist=Mem->MyPredictedPosition().dist(Mem->BallPredictedPosition(1));
    if(pred_dist<0.8&&pred_dist>Mem->SP_ball_size+Mem->GetMyPlayerSize()){
      Mem->LogAction3(10,"GoBaby: maybe ball is not kickable, so wait for ball (dist %.4f)",dist);
      eye.AddBall(1);
      return true;
    }
  }
  if(type!=only_control_dribble){
    if(PNHDribble()){
      return true;
    }
  }
  if(type==no_control_dribble)
    return false;
//   if(CanControllingDribble()||((GetPriority()>0.5f||(Pos.GetMyType()>=PT_Midfielder&&GetPriority()>0.2f))
// 				&&CanAgressiveControllingDribble(side))){
  if(CanAgressiveControllingDribble(side)){
    if(controlling_dribbling()==AQ_ActionQueued){
      return true;
    }
  }
  if(type==only_control_dribble)
    return false;
  int num_to_avoid=SelectAvoidOpponents(close_players);
  if((type==no_avoid&&num_to_avoid>0)||(type==avoid_1&&num_to_avoid>1)||(type==avoid_2&&num_to_avoid>2)||num_to_avoid>max_num){
    Mem->LogAction4(10,"GoBaby: %.0f opponents or teamless players is close to us - can not avoid (type - %.0f)",float(num_to_avoid),float(type));
    return false;
  }else
    Mem->LogAction3(10,"GoBaby: %d opponents or teamless players is close to us",num_to_avoid);
  if(type==no_avoid||num_to_avoid==0){
    Mem->LogAction2(10,"GoBaby: type of dribble is no_avoid and we can not go dribble");
    return false;
  }

  if(AvoidEnemy(close_players,num_to_avoid))
    return true;
  return false;
}
////////////////////////////////////////////////////////////////////
bool Dribble::DribbleDance(float side){//предпологаем, что по умолчанию идем к воротам
  Mem->LogAction2(10,"In dribble dance");
  bool can_close_dribble=CanAgressiveControllingDribble(side);
  Vector pos=Mem->MyPos();
  vector<Vector> dirs;
  //  dirs.push_back(Vector(52.0,pos.y));
  if(fabs(pos.y)>Mem->SP_goal_width/2+3.0f){
    dirs.push_back(Vector(pos.x,-signf(pos.y)*Mem->SP_goal_width/2.0f));
  }else{
    dirs.push_back(Vector(pos.x+2*side,pos.y+5.0f));
    dirs.push_back(Vector(pos.x+2*side,pos.y-5.0f));
  }
//   if(fabs(Mem->MyY())>Mem->SP_goal_width/2+1||fabs(Mem->MyBodyAng())>60.0f&&fabs(Mem->MyBodyAng())<120.0f){
//     dirs.clear();
//     dirs.push_back(Vector(Mem->MyX(),-signf(Mem->MyY())));
//   }
    
  AngleDeg old=GetDribbleAngle();
  for(unsigned int i=0;i<dirs.size();i++){
    if(PNHDribble(dirs[i])){
      SetDribbleAngle(old);
      return true;
    }
    if(!can_close_dribble)
      continue;
    SetDribblePos(dirs[i]);
    if(GoBaby(only_control_dribble)){
      SetDribbleAngle(old);
      return true;
    }
  }
  SetDribbleAngle(old);
  return false;
}
////////////////////////////////////////////////////////////////////
//we think that origpos is NOT in collision
Vector Dribble::CorrectForCollision(Vector pos,Vector mypos,Vector origpos) const{
  const float buffer=0.1f;
  if((pos-mypos).mod()<=(2*Mem->GetMyPlayerSize()-buffer)){
    Vector res=pos-origpos;
    float i=res.mod();
    for(;i>0.0f;i-=0.05f){
      res=(res/res.mod())*i;
      if((origpos+res-mypos).mod()>(2*Mem->GetMyPlayerSize()-buffer))
        return origpos+res;
    }
  }else
    return pos;
  return origpos;
}
////////////////////////////////////////////////////////////////////
Vector Dribble::GetPredictedOpponentPosition(Unum opp,Vector opppos,Vector mypos,AngleDeg oppangle, Vector oppvel) const{
  Vector new_vel=oppvel+Polar2Vector(1.0*Mem->GetOpponentDashPowerRate(opp)*Mem->SP_max_power,oppangle);
  if(new_vel.mod()>Mem->GetOpponentPlayerSpeedMax(opp))
    new_vel=(new_vel/new_vel.mod())*Mem->GetOpponentPlayerSpeedMax(opp);
  Vector pos=opppos+new_vel;
  pos=CorrectForCollision(pos,mypos,opppos);
  return pos;
}
////////////////////////////////////////////////////////////////////
//mypos, ballpos - in next cycle
//other parametrs in this cycle
bool Dribble::Covered(Unum opp,Vector ballpos,Vector opppos,Vector mypos,AngleDeg oppangle,Vector oppvel,bool isGoalie) const{
  float const buffer=0.05f;
  //  if(oppvel!=Vector(.0f,.0f)&&fabs(oppvel.dir()-oppangle)>15.0f)
  //    my_error("Covered:Vel of player is not paralel to the body facing");
  Vector new_vel=oppvel+Polar2Vector(1.0*Mem->GetOpponentDashPowerRate(opp)*Mem->SP_max_power,oppangle);
  if(new_vel.mod()>Mem->GetOpponentPlayerSpeedMax(opp))
    new_vel=(new_vel/new_vel.mod())*Mem->GetOpponentPlayerSpeedMax(opp);
  Vector pos1=opppos+new_vel;
  Line orig;
  orig.LineFromRay(opppos,oppangle);
  pos1=CorrectForCollision(pos1,mypos,opppos);
  new_vel=oppvel+Polar2Vector(1.0*Mem->GetOpponentDashPowerRate(opp)*Mem->SP_max_power,GetNormalizeAngleDeg(oppangle+180.0f));
  if(new_vel.mod()>Mem->GetOpponentPlayerSpeedMax(opp))
    new_vel=(new_vel/new_vel.mod())*Mem->GetOpponentPlayerSpeedMax(opp);
  Vector pos2=opppos+new_vel;
  pos2=CorrectForCollision(pos2,mypos,opppos);
  float target_area;
  if(!isGoalie){
    target_area=Mem->GetOpponentKickableArea(opp);
  }else{
    target_area=Mem->SP_catch_area_l;//for goalie
  }
  bool cond1=(orig.dist(ballpos)<=(target_area+buffer));
  bool cond2=orig.InBetween(orig.ProjectPoint(ballpos),pos1,pos2);
  bool cond3=((ballpos-pos1).mod()<=(target_area+buffer)||(ballpos-pos2).mod()
	      <=(target_area+buffer));
  return cond1&&(cond2||cond3);
}
////////////////////////////////////////////////////////////////////
//from CMUnited - not used now
float Dribble::WeightByDistance(float distance)const{
  float weight=-0.03f*Sqr(distance)+0.01f*distance+1.0f;
  weight=weight>1?1:weight;
  weight=weight<0?0:weight;
  return weight;
}
///////////////////////////////////////////////////////////////////////
bool Dribble::AvoidEnemy(const ClosePlayer* close_players,int num){
  const int step=10;//degrees
  float angles[360/step];
  for(int angle=0;angle<360;angle+=step)
    angles[angle/step]=0.0f;
  float max=0.0f,opt_angle=0.0f;

  static Time last[3]={-1,-1,-1};
  static AngleDeg last_angles[3];
  static int index=-1;

  bool can_avoid=true;
  AngleDeg turn_ang=GetNormalizeAngleDeg(Mem->MyBodyAng()-dribble_angle);
  for(int i=0;i<num;i++){
    Vector pos=close_players[i].pos;
    Vector vel=close_players[i].vel;
    if(fabs(turn_ang)<5.0&&OpponentBehindUs(close_players[i].pos,.0f)&&
       (!close_players[i].body_ang_valid||(pos-Mem->MyPos()).mod()>2.0f||fabs(dribble_angle-close_players[i].body_angle)>45.0f)&&
       (vel.mod()<0.5f||(pos-Mem->MyPos()).mod()>=3.0f||GetNormalizeAngleDeg(fabs(vel.dir()-dribble_angle))>15.0f)){
      Mem->LogAction3(10,"AE: think that opp %d is avoid",int(close_players[i].opp));
      if(close_players[i].opp!=Unum_Unknown)
	OppIgnore[close_players[i].opp-1]=true;//hack
    }else
      can_avoid=false;
  }
  if(can_avoid){
    Mem->LogAction2(10,"AE:all opponents are avoid so may kick to myself");
    if(PNHDribble())
      return true;
  }

  float dist=.8f;
  for(int i=0;i<num;i++){
    Vector pos=close_players[i].pos;
    if(close_players[i].body_ang_valid){//try to predict opponent position
      pos=GetPredictedOpponentPosition(close_players[i].opp,pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,close_players[i].vel);
    }
    for(int angle=0;angle<360;angle+=step){
      Vector new_pos=Polar2Vector(dist,GetNormalizeAngleDeg(angle))+Mem->MyPredictedPosition(1);
      angles[angle/step]+=(pos-new_pos).mod()/dist_stop_controlling_dribble;
      if(!Mem->FieldRectangle.shrink(0.5f).IsWithin(new_pos))
	angles[angle/step]=-200.0f;
      if(close_players[i].body_ang_valid&&Covered(close_players[i].opp,new_pos,pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,close_players[i].vel,
						  (close_players[i].opp==Mem->TheirGoalieNum?true:false))){
	angles[angle/step]=-100.0f;
      }
    }
  }
  for(int angle=0;angle<360;angle+=step){
    if(max<angles[angle/step]){
      opt_angle=angle;
      max=angles[angle/step];
    }
  }
  if(max==0.0f){
    Mem->LogAction2(10,"AE: i can not avoid opponents");
    return false;
  }
  dist=.8f;
  for(int i=0;i<num;i++){
    bool ok=false;
    if(close_players[i].body_ang_valid>=0.95f){
      for(float step=.85f;step<=.9f;step+=0.02f){
        if(!Covered(close_players[i].opp,Polar2Vector(step,GetNormalizeAngleDeg(opt_angle))+Mem->MyPredictedPosition(1),
		    close_players[i].pos,Mem->MyPredictedPosition(1),close_players[i].body_angle,close_players[i].vel,
		    (close_players[i].opp==Mem->TheirGoalieNum?true:false))&&dist<=step){
	  dist=step;
	  ok=true;
	  break;
        }
      }
      if(ok==false){
        dist=.9f;
        break;
      }
    }else{
      Mem->LogAction3(10,"AE:body angle of opponent %d is not valid",int(close_players[i].opp));
      dist=.9f;
      break;
    }
  }

  index=(index+1)%3;
  /*  float mindist=100.0f;
      for(int i=0;i<num;i++){
      Vector pos=close_players[i].pos;
      if(mindist>(pos-Mem->MyPos()).mod())
      mindist=(pos-Mem->MyPos()).mod();
      }
      if(mindist>2.0)
      last[index]=Mem->CurrentTime-5;//not count this cycle
      else*/
  if(last[(index+2)%3]==(Mem->CurrentTime-1)&&last[(index+1)%3]==(Mem->CurrentTime-2)&&last[index]==(Mem->CurrentTime-3)&&//del -3
     (opt_angle==last_angles[0]&&opt_angle==last_angles[1]&&opt_angle==last_angles[2])){
    /*  if(StrongForward==false){
        Mem->LogAction2(10,"AE: we 3 cycles try to go to once angle, so think that can not avoid, try kick_avoid");
        return TryKickAvoid(close_players,num);
	}else{  */
    Mem->LogAction2(10,"AE: we 3 cycles try to go to once angle, so think that can not avoid");
    return false;
    //      }
  }
  //turn forward
  last[index]=Mem->CurrentTime;
  last_angles[index]=opt_angle;

  //check if anyone may take ball if we turn now
  bool can_turn=true;
  for(int i=0;i<num;i++)
    if(close_players[i].body_ang_valid&&Covered(close_players[i].opp,Mem->BallPredictedPosition(1),close_players[i].pos,Mem->MyPredictedPosition(1),
						close_players[i].body_angle,close_players[i].vel,(close_players[i].opp==Mem->TheirGoalieNum?true:false))||
       (Mem->BallAbsolutePosition()-close_players[i].pos).mod()<(Mem->GetOpponentPlayerSpeedMax(close_players[i].opp)+Mem->GetOpponentPlayerSize(close_players[i].opp)+Mem->SP_ball_size)){
      can_turn=false;
    }

  if(can_turn&&fabs(turn_ang)>=5.0&&(Mem->BallPredictedPosition(1)-Mem->MyPredictedPosition(1)).mod()<reliable_kick_area){
    Mem->LogAction2(10,"AE: we may try turn to dribble angle");
    turn(dribble_angle-Mem->MyBodyAng());
    return true;
  }

  Mem->LogAction4(10,"AE: can avoid all opp to angle %.2f and dist %.2f",opt_angle,dist);
  kick.turnball(GetNormalizeAngleDeg(opt_angle-Mem->MyBodyAng()),TURN_NONE,SK_Fast,dist);
  return true;

}
///////////////////////////////////////////////////////////////////////
Vector Dribble::GetTracePoint(Dribble::Trace trace,Vector my_pos){
  AngleDeg turn_ang=GetNormalizeAngleDeg(Mem->MyBodyAng()-dribble_angle);
  AngleDeg ang=fabs(turn_ang)>10.0?dribble_angle:Mem->MyBodyAng();
  int i=-1;
  if(trace==right)
    i*=-1;
  Line perp_l;
  AngleDeg new_ang=GetNormalizeAngleDeg(ang+i*90);
  perp_l.LineFromRay(my_pos,GetNormalizeAngleDeg(new_ang-i*5));
  Vector pos=Polar2Vector((dist_line1+dist_line2)/2,new_ang)+my_pos;
  return perp_l.ProjectPoint(pos);
}
////////////////////////////////////////////////////////////////////
Dribble::Trace Dribble::SelectTrace(Vector my_pos){
  Vector pos1=GetTracePoint(left,my_pos);
  Vector pos2=GetTracePoint(right,my_pos);
  if(!Mem->FieldRectangle.shrink(2*Mem->GetMyPlayerSize()).IsWithin(my_pos)){
    Vector work_point=Mem->FieldRectangle.nearestEdge(my_pos);
    if((work_point-pos1).mod()<(work_point-pos2).mod())
      return right;
    else
      return left;
  }
  //  Unum opp=Mem->ClosestOpponentToBall();
  //  Vector pos;
  //  if(opp!=Unum_Unknown&&Mem->DistanceTo((pos=Mem->OpponentAbsolutePosition(opp)))<dist_stop_controlling_dribble){
  //    Mem->LogAction3(10,"SelectTrace: correct trace for opponent %d",opp);
  //    if((pos-pos1).mod()<(pos-pos2).mod())
  //      return right;
  //    else
  //      return left;
  //  }
  if((pos1-Mem->BallAbsolutePosition()).mod()<(pos2-Mem->BallAbsolutePosition()).mod())
    return left;
  else
    return right;
}
////////////////////////////////////////////////////////////////////
bool Dribble::BallOnTrace(Vector ballPos,Vector MyPos){
  Line perp_l;
  for(int i=1;i>=-1;i-=2){
    AngleDeg new_ang=GetNormalizeAngleDeg(Mem->MyBodyAng()+i*90);
    perp_l.LineFromRay(MyPos,new_ang);
    Vector pos1=Polar2Vector(dist_line1,new_ang)+MyPos;
    Vector pos2=Polar2Vector(dist_line2,new_ang)+MyPos;
    Vector res=perp_l.ProjectPoint(ballPos);
    if(perp_l.InBetween(res,pos1,pos2)){
      trace=(i==1?right:left);
      return true;
    }
  }
  trace=none;
  return false;
}
////////////////////////////////////////////////////////////////////////
bool Dribble::MustChangeTrace()const{
  Unum opp=Mem->ClosestOpponentToBall();
  if(opp==Unum_Unknown||Mem->OpponentDistanceToBall(opp)>=Mem->SP_tackle_dist*1.5)
    return false;
  Mem->LogAction3(10,"MustChangeTrace: opp %d is danger",opp);
  return true;
}
////////////////////////////////////////////////////////////////////////
int Dribble::GetBallAreaNum(Vector ballPos,Vector MyPos){
  AngleDeg turn_ang=GetNormalizeAngleDeg(Mem->MyBodyAng()-dribble_angle);
  AngleDeg ang=fabs(turn_ang)>10.0?dribble_angle:Mem->MyBodyAng();
  Vector rel_pos=(ballPos-MyPos).rotate(-ang);
  if(rel_pos.x>Mem->GetMyPlayerSize()){
    if(fabs(rel_pos.y)<Mem->GetMyPlayerSize())
      return 2;
    else if(rel_pos.y>0)
      return 3;
    else
      return 1;
  }else if(fabs(rel_pos.x)<=Mem->GetMyPlayerSize()){
    if(rel_pos.y>0)
      return 4;
    else
      return 8;
  }else{//rel_pos.x<-Mem->GetMyPlayerSize()
    if(fabs(rel_pos.y)<Mem->GetMyPlayerSize())
      return 6;
    else if(rel_pos.y>0)
      return 5;
    else
      return 7;
  }
    
}
//////////////////////////////////////////////////////////////////////
void Dribble::SetControlDribbleView(float dist_to_me,Vector ballVel)
{
  float buf=Mem->CP_collision_buffer;
  if(ballVel.mod()<1.5f)
    buf/=2.0f;
  if(dist_to_me>=reliable_kick_area*0.9f||
     (dist_to_me>0.2f&&dist_to_me<Mem->GetMyPlayerSize()+Mem->SP_ball_size+buf)){
    eye.AddBall(0);
    return;
  }
  
  Vector base=Mem->MyPos()+Polar2Vector(5.0f,Mem->MyBodyAng());
  Unum cl_opp=Mem->ClosestOpponentTo(base);
  if(cl_opp!=Unum_Unknown)
    eye.AddOpponent(cl_opp,3);
  else if(Mem->MyX()<33.0f)
    eye.AddPosition(base,10,3,RT_WithoutTurn,VW_Narrow);//смотрим раз в 1 с вперед себя
    
  if(Mem->MyX()>25.0f){
    eye.AddPosition(Vector(Mem->MyX(),-signf(Mem->MyY())*30.0f),5,15,RT_WithoutTurn,VW_Narrow);
    eye.AddOpponent(Mem->TheirGoalieNum,2);
  }
}
////////////////////////////////////////////////////////////////////////
//если BallPredPos==0 то непоседственно выполнить дриблинг, иначе только вернуть
//предсказанную позицию мяча
ActionQueueRes Dribble::controlling_dribbling(Vector* BallPredPos){
  
  AngleDeg turn_ang=GetNormalizeAngleDeg(Mem->MyBodyAng()-dribble_angle);
  Vector ballvel=Mem->BallVelocityValid()?Mem->BallAbsoluteVelocity():Vector(0,0);
  Vector ballpos=Mem->BallAbsolutePosition()+ballvel;

  if(!Mem->FieldRectangle.IsWithin(Mem->MyPos())){
    Mem->LogAction2(10,"CD: i outside of field so can not go dribble");
    return AQ_ActionNotQueued;
  }
  Vector work_point=Mem->FieldRectangle.nearestEdge(Mem->MyPos());
  if(fabs(turn_ang)>10||(Mem->DistanceTo(work_point)<2.0f&&fabs(turn_ang)>1.0f)){//must turn to trace
    TurnWithBall(dribble_angle,BallPredPos);
    return AQ_ActionQueued;
  }
  Vector myvel=(Mem->MyVelConf()?Mem->MyVel():Vector(0.0f,0.0f))+
    Polar2Vector(Mem->MyEffort()*Mem->GetMyDashPowerRate()*Mem->CorrectDashPowerForStamina(Mem->SP_max_power),Mem->MyBodyAng());
  Vector mypos=Mem->MyPos()+myvel;
  float dist_to_me=(mypos-ballpos).mod();

  if((dist_to_me<reliable_kick_area*0.95f||(dist_to_me<reliable_kick_area&&Mem->BallVelocityValid()>=0.95f))
     &&!Mem->WillDashBeCollision(Mem->CorrectDashPowerForStamina(Mem->SP_max_power))){
    if(BallPredPos==0){
      Mem->LogAction4(10,"CD:after dash ball will be on trace and kickable, so make dash(dist: %.2f;reliable_kick_area %.2f)",
		      dist_to_me,reliable_kick_area);
      SetControlDribbleView(dist_to_me,ballvel);
      dash(Mem->CorrectDashPowerForStamina(Mem->SP_max_power));
    }else
      *BallPredPos=ballpos;
    return AQ_ActionQueued;
  }else{
    int area=GetBallAreaNum();
    Trace trace=SelectTrace();
    Vector my_end_pos,ball_end_pos,ball_vel,rel_pos;
    if(BallPredPos==0)
      Mem->LogAction3(10,"CD: GetBallAreaNum return %d",area);
    float coef=0.65f*reliable_kick_area;
    switch(area){
    case 8:
    case 4:
      my_end_pos=Mem->MyPredictedPosition(3,Mem->SP_max_power,1);
      rel_pos=(Mem->BallAbsolutePosition()-Mem->MyPos()).rotate(-Mem->MyBodyAng());
      ball_end_pos=my_end_pos+Polar2Vector((Mem->GetMyKickableArea())/(1.6f-rel_pos.x*0.5f),GetNormalizeAngleDeg(Mem->MyBodyAng()));
      if(BallPredPos==0)
	Mem->LogAction4(10,"rel_pos=(%.2f;%.2f)",rel_pos.x,rel_pos.y);
      if(fabs(rel_pos.y)<coef){
	if(BallPredPos==0)
	  Mem->LogAction2(10,"Selecet alternative solution");
	ball_end_pos=GetTracePoint(trace,my_end_pos);
      }
      ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(3);
      break;
    case 7:
    case 5:
      my_end_pos=Mem->MyPredictedPosition(4,Mem->SP_max_power,1);
      ball_end_pos=GetTracePoint(trace,my_end_pos);
      ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(4);
      break;
    case 3:
    case 1:
      my_end_pos=Mem->MyPredictedPosition(3,Mem->SP_max_power,1);
      ball_end_pos=GetTracePoint(trace/*==left?right:left*/,my_end_pos);
      ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(3);
      break;
    case 2:
      my_end_pos=Mem->MyPredictedPosition(3,Mem->SP_max_power,1);
      ball_end_pos=GetTracePoint(trace,my_end_pos);
      ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(3);
      break;
    case 6:
      my_end_pos=Mem->MyPredictedPosition(3,Mem->SP_max_power,1);
      ball_end_pos=GetTracePoint(trace,my_end_pos);
      ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(3);
      break;
    default:
      if(BallPredPos==0)
	my_error("Wrong param %d in GetBallAreaNum",area);
      return AQ_ActionNotQueued;
    
    }
    
    Vector dir=Mem->BallAbsolutePosition()+ball_vel-(Mem->MyPos()+Mem->MyVel());
    
    if(dir.mod()<=Mem->GetMyPlayerSize() + Mem->SP_ball_size+0.02f){
      if(BallPredPos==0)
	Mem->LogAction5(10,"CD: predict collision with new vel in next cycle, so correct vel vel(%.2f,%.2f) old dist %f",
			ball_vel.x,ball_vel.y,dir.mod());
      dir=dir.SetLength(Mem->GetMyPlayerSize() + Mem->SP_ball_size+0.2f);
      ball_vel=Mem->MyPos()+Mem->MyVel()+dir-Mem->BallAbsolutePosition();
    }
    
    if(dir.mod()>=reliable_kick_area){
      if(BallPredPos==0)
	Mem->LogAction3(10,"CD: predict lose ball in next cycle after kick, with dist %.2f",
			dir.mod());
      Ray r(Mem->BallAbsolutePosition(),ball_vel);
      Vector res1,res2;
      int num=r.CircleIntersect(reliable_kick_area*0.95f,Mem->MyPos()+Mem->MyVel(), &res1, &res2);
      if(num==0){
	if(BallPredPos==0)
	  Mem->LogAction2(20,"CD:found no answer, move to base");
	my_end_pos=Mem->MyPredictedPosition(2,0,1);
	ball_end_pos=Mem->MyPredictedPosition()+Polar2Vector(Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.11f,Mem->MyBodyAng());
	ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(1);
	dir.SetLength(Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.11f);
      }else{
	if(num==2)
	  res1=res2;
	ball_vel=res1-Mem->BallAbsolutePosition();
      }
    }   
      
    if(BallPredPos==0){
      SetControlDribbleView(dir.mod(),ball_vel);
      if(ball_vel.mod()==0){
	actions.stopball();
	return AQ_ActionQueued;
      }
      Mem->LogAction7(10,"CD: make kick with vel (%.2f,%.2f); dist after kick %.3f (collision val: %.3f;kickable area %.2f)",
		      ball_vel.x,ball_vel.y,dir.mod(),Mem->GetMyPlayerSize() + Mem->SP_ball_size,reliable_kick_area);
      kick.smartkickg(ball_vel.mod(),ball_vel.dir(),SK_Fast);
    }else{
      *BallPredPos=Mem->BallAbsolutePosition()+ball_vel;
    }    
    return AQ_ActionQueued;
    
  }
  if(BallPredPos==0)
    my_error("That is going on?");
  return AQ_ActionNotQueued;
}
////////////////////////////////////////////////////////////////////////////
int Dribble::CorrectCyclesForStamina(int cycles){
  double stamina=(double)Mem->MyStamina();
  double recovery=(double)Mem->MyRecovery();
  if(cycles==0)
    return 0;
  for(int i=cycles;i>0;i--){
    if((stamina-(Mem->EffortDecThreshold+Mem->CP_tired_buffer))<100.0)
      return cycles+int(i*0.2);//+20%
    if(stamina<=Mem->SP_recover_dec_thr*Mem->SP_stamina_max&&recovery>Mem->SP_recover_min)
      recovery-=Mem->SP_recover_dec;
    stamina=stamina-100.0+recovery*Mem->GetMyStaminaIncMax();

  }
  return cycles;
}
////////////////////////////////////////////////////////////////////
Vector Dribble::CorrectPosForStamina(Vector pos, int cycles){
  double stamina=(double)Mem->MyStamina();
  double recovery=(double)Mem->MyRecovery();
  if(cycles==0)
    return pos;
  for(int i=cycles;i>0;i--){
    if((stamina-(Mem->EffortDecThreshold+Mem->CP_tired_buffer))<100.0)
      return pos+pos*((i*0.2)/pos.mod());//+0.2 m for each circle
    if(stamina<=Mem->SP_recover_dec_thr*Mem->SP_stamina_max&&recovery>Mem->SP_recover_min)
      recovery-=Mem->SP_recover_dec;
    stamina=stamina-100.0+recovery*Mem->GetMyStaminaIncMax();
  }
  return pos;
}
////////////////////////////////////////////////////////////////////////
//если BallPredPos==0, то не выполняем ничего, а возвращаем следующее положения мяча
void Dribble::TurnWithBall(AngleDeg ang,Vector* BallPredPos){
  
  if(BallPredPos==0)
    eye.AddPosition(dribble_target,5,3,RT_WithoutTurn,VW_Narrow);//смотрим раз в 0.5 с вперед себя
  
  int num_cyc=Mem->NumCyclesToTurn(ang);
  if(BallPredPos==0)
    Mem->LogAction4(10,"TurnWithBall: we need to make turn to ang %.2f with %.0f cycles",ang,(float)num_cyc);
  num_cyc++;//for kick
  Vector my_end_pos=Mem->MyPredictedPosition(num_cyc);
  
  float dist_to_me=(my_end_pos-Mem->BallPredictedPosition(num_cyc)).mod();
  if(dist_to_me<reliable_kick_area*0.95f||(dist_to_me<reliable_kick_area&&Mem->BallVelocityValid()>=0.95f)){
    if(BallPredPos==0){
      Mem->LogAction2(10,"TurnWithBall:ball will be kickable so turn");
      turn(ang-Mem->MyBodyAng());
    }else
      *BallPredPos=Mem->BallPredictedPosition();    
    return;
  }
  Trace trace=SelectTrace(my_end_pos);
  Vector trace_pos=GetTracePoint(trace,Mem->MyPredictedPosition(1));
  Vector ball_end_pos=GetTracePoint(trace,my_end_pos);

  int ball_area=GetBallAreaNum();
  if((GetBallAreaNum(trace_pos)==4&&(ball_area==1||ball_area==8||ball_area==7))||
     (GetBallAreaNum(trace_pos)==8&&(ball_area==3||ball_area==4||ball_area==5))){
    if(BallPredPos==0){
      Mem->LogAction4(10,"TurnWithBall: ball on wring trace so just move it (%.0f %.0f)",
		      float(ball_area),(float)GetBallAreaNum(trace_pos));
      if(!kick.moveball(trace_pos)){
	Mem->LogAction2(10,"TurnWithBall: can not make  move ball, so stop ball");
	stop_ball();
      }else
	return;
    }else{
      *BallPredPos=trace_pos;
      return;
    }    
  }
    
  Vector ball_vel=(ball_end_pos-Mem->BallAbsolutePosition())/Mem->GetBallMoveCoeff(max(3,num_cyc));
  if(BallPredPos==0)
    kick.smartkick(ball_vel.mod(),ball_end_pos,SK_Safe);
  else
    *BallPredPos=Mem->BallAbsolutePosition()+ball_vel;
}
////////////////////////////////////////////////////////////////////////
bool Dribble::PNHDribble(Vector to,int max_cyc){
  return PNHDribble(-777.0,to,max_cyc);
}
bool Dribble::PNHDribble(AngleDeg ang,int max_cyc){
  return PNHDribble(ang,Vector(-100.0f,-100.0f),max_cyc);
}
////////////////////////////////////////////////////////////////////////
bool Dribble::PNHDribble(AngleDeg ang,Vector target,int max_cyc){
  Vector vel;
  if(ang==-777.0&&target.x==-100.0f){
    ang=dribble_angle;
    target=dribble_target;
  }else
    if(ang==-777.0)
      ang=(target-Mem->MyPredictedPosition()).dir();

  if(!CanPNHDribble(ang,max_cyc,&vel,target))
    return false;
//   float max_dist=/*Mem->MyY()<=30.0f?4.0f:*/Mem->SP_player_speed_max;
//   bool is_opp_close=false;
//   Unum Opp[11];
//   int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,Mem->BallAbsolutePosition(),Opp),i=0;
//   for(;i<numClosestOpp;i++){
//     if(Mem->OpponentDistanceTo(Opp[i],Mem->BallAbsolutePosition())<=max_dist){
//       if(!Mem->OpponentTackling(Opp[i])){
//         is_opp_close=true;
//         break;
//       }
//     }else
//       break;
//   }


  if(fabs(GetNormalizeAngleDeg(Mem->MyBodyAng()-ang))>dribble_turn_ang_err||
     (fabs(Mem->MyY())>=too_close_to_side_line&&fabs(GetNormalizeAngleDeg(Mem->MyBodyAng()-ang))>5.0f)){
    if(!CanAgressiveControllingDribble(1.0f,false)){
      Mem->LogAction2(10,"PNHDribble: CanAgressiveControllingDribble say that we can not turn now");
      return false;
    }
    
    //(is_opp_close?dribble_turn_ang_err:10.0f)||
//      (fabs(Mem->MyY())>=too_close_to_side_line&&fabs(GetNormalizeAngleDeg(Mem->MyBodyAng()-ang))>5.0f)){
//     if(is_opp_close){
//       Mem->LogAction3(10,"PNHDribble: opp %d is too close for me to go forward",int(Opp[i]));
//       return false;
//     }
//     if(Mem->NumTeamlessPlayers()>0){
//       if(Mem->DistanceTo(Mem->ClosestTeamlessPlayerPosition())<max_dist&&!OpponentBehindUs(Mem->ClosestTeamlessPlayerPosition(),0.0f)){
//         Mem->LogAction2(10,"Teamless player is close to me, so can not go slow dribble");
//         return false;
//       }
//     }
    TurnWithBall(ang);
    return true;
  }
  PNHDribbleExecute(Mem->MyBodyAng(),vel);
  return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_OUTPUT
int num_times=0;
float evar_time=0.0f;
int num_select_cycles[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
//////////////////////////////////////////////////////////////////////////////////////////
bool Dribble::CanPNHDribble(AngleDeg ang,int max_cyc,Vector* res_vel,Vector to){//after this function must call TurnWithBall()
  FuncCalcTime calc("CanPNHDribble");
#ifdef DEBUG_OUTPUT
  statistica::TimeCounter tc;
  tc.Start();
#endif  
  if(to.x!=-100.0f){
    ang=(to-Mem->MyPos()).dir();
    Mem->LogAction5(20,"IN CanPNHDribble: convert pos (%.2f,%.2f) to ang %.2f",to.x,to.y,ang);
  }else
    Mem->LogAction4(10,"IN CanPNHDribble: try go to angle %.2f (max_cyc = %.0f)",ang,float(max_cyc));

  const int BASE_MIN_CYC=6;
  
  static Vector PlayerPos[11];
  static Vector PlayerVel[11];
  PlayerInterceptInfo pInfo;
  static float PlayerAng[11];
  static bool AngValid[11];
  static Vector BallVel;
  Unum danger_opp=Unum_Unknown;
  register int orig_danger_opp_cyc=0,min_opp_cyc=1000;
  float dang_opp_pos_val=0.0f;
  static Time time=-1;

  static Rectangle field=Mem->FieldRectangle.shrink(1.0f);
  if(time<Mem->CurrentTime){
    time=Mem->CurrentTime;
    for(register int i=1;i<=Mem->SP_team_size;i++){
      if(!Mem->OpponentPositionValid(i))
	continue;
      PlayerPos[i-1] = Mem->OpponentAbsolutePosition(i);
      if (Mem->OpponentVelocityValid(i))
	PlayerVel[i-1] = Mem->OpponentAbsoluteVelocity(i);
      else
	PlayerVel[i-1] = Vector(0,0);

      if (Mem->OpponentBodyAngleValid(i)) {
	AngValid[i-1] = TRUE;
	PlayerAng[i-1] = Mem->OpponentAbsoluteBodyAngle(i);
      } else{
	AngValid[i-1]=FALSE;
	PlayerAng[i-1] = 0;
      }
    }
    if (Mem->BallVelocityValid())
      BallVel = Mem->BallAbsoluteVelocity();
    else
      BallVel = Vector(0,0);
  }
  AngleDeg bodyAng,turnAng=GetNormalizeAngleDeg(Mem->MyBodyAng()-ang);
  Vector BallPos;
  if(fabs(turnAng)>dribble_turn_ang_err){
    bodyAng=ang;
    Vector myPredPos=Mem->MyPredictedPosition(Mem->NumCyclesToTurn(turnAng));
    BallPos=GetTracePoint(SelectTrace(myPredPos),myPredPos);
  }else{
    bodyAng=Mem->MyBodyAng();
    BallPos=Mem->BallAbsolutePosition();
  }
  
  Vector myPredPos,ballPredPos,ballNewVel;
  int add_cyc;
  
  int min_cyc=BASE_MIN_CYC;
  if(!CanAgressiveControllingDribble(1.0f,false)){
    CanAgressiveControllingDribble(1.0f);//print log
    min_cyc=BASE_MIN_CYC-2;
  }
  
  for(int num_cyc=max_cyc;num_cyc>=min_cyc;num_cyc-=2){
    myPredPos=Mem->MyPredictedPosition(num_cyc,Mem->SP_max_power,1);
    ballPredPos=myPredPos+Polar2Vector(Mem->GetMyOptCtrlDist(),GetNormalizeAngleDeg(bodyAng));
    if(Mem->DistanceTo(ballPredPos)>17.0f&&Mem->TheirPenaltyArea.expand(1.0f+num_cyc*2.0f/max_cyc).IsWithin(ballPredPos)&&
       (Mem->MyX()<25.0f||Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum))){
      Mem->LogAction4(10,"Ball maybe in their penalty area, and goalie take it (cycle %.0f,x=%.2f)",
		      float(num_cyc),ballPredPos.x);
      continue;
    }
    if(to.x!=-100.0f&&(to-Mem->MyPos()).mod()<(ballPredPos-Mem->MyPos()).mod())
      continue;
    if(!field.IsWithin(ballPredPos))
      continue;//ball maybe outside of the field
    ballNewVel=(ballPredPos-BallPos)/Mem->GetBallMoveCoeff(num_cyc);
    if(ballNewVel.mod()>=Mem->SP_ball_speed_max)
      continue;
    min_opp_cyc=1000;
    for(int i=1;i<=Mem->SP_team_size;i++){
      if(!Mem->OpponentPositionValid(i)||OppIgnore[i-1]||
	 (i==Mem->TheirGoalieNum&&!Mem->TheirPenaltyArea.expand(1.0f).IsWithin(ballPredPos)&&!Pos.IsOpponentGoalieGoOutOfPenaltyArea()))
	continue;
      add_cyc=(Mem->MyX()>25.0f||Mem->OpponentX(i)-1.5f<Mem->MyX())?1:3;
      if(i==Mem->TheirGoalieNum&&!Mem->TheirPenaltyArea.expand(1.0f+num_cyc*2.0f/max_cyc).IsWithin(ballPredPos))
	add_cyc=1;
      pInfo =Mem->ActiveCanGetThere(Mem->SP_max_power, Mem->CP_max_int_lookahead,
				    BallPos, ballNewVel,
				    Mem->TheirSide,i,
				    PlayerPos[i-1], PlayerVel[i-1], PlayerAng[i-1], AngValid[i-1],
				    FALSE);
      if(Mem->IsSuccessRes(pInfo.res)){
	int opp_cyc=pInfo.numCyc;
	if(Mem->TheirGoalieNum==i&&Mem->TheirPenaltyArea.IsWithin(ballPredPos)){//correct for goalie
	  opp_cyc-=(min_cyc<BASE_MIN_CYC)?1:3;
	}
	if(opp_cyc<0)
	  opp_cyc=0;
	if(min_opp_cyc>opp_cyc){
	  min_opp_cyc=opp_cyc;
	  danger_opp=Unum(i);
	  orig_danger_opp_cyc=pInfo.numCyc;
	  dang_opp_pos_val=Mem->OpponentPositionValid(i);
	  if(num_cyc+add_cyc>=min_opp_cyc)
	    break;
	}
      }
    }//opponent cycle
    if(num_cyc+add_cyc<min_opp_cyc){
      Mem->LogAction3(20,"I can make pass with fixed power with cycles: %.2f",float(num_cyc));
      Mem->LogAction6(20,"Most danger opponent: %.2f with cycles: %.2f; correct for pos valid: %.2f (pos valid:%.2f)",
		      (float)danger_opp,(float)orig_danger_opp_cyc,float(min_opp_cyc),dang_opp_pos_val);
#ifdef DEBUG_OUTPUT
      num_select_cycles[num_cyc]++;
      ostringstream ost;
      for(int i=6;i<=20;i+=2)
	ost<<i<<":"<<num_select_cycles[i]<<"; ";
      Mem->LogAction3(10,"CanPNHDribble: num_select_cycles= %s",const_cast<char*>(ost.str().c_str()));
      tc.Finish();
      num_times++;
      evar_time+=tc.GetTime();
      Mem->LogAction3(10,"CanPNHDribble: evar_time=%f",evar_time/(float)num_times);
#endif       
      *res_vel=ballNewVel;
      return true;
    }
    Mem->LogAction8(20,"Most danger opponent: %.0f with cycles: %.0f (my cycles=%.0f; add_cyc=%.0f); correct for pos valid: %.0f (pos valid:%.2f);",
		    (float)danger_opp,(float)orig_danger_opp_cyc,(float)num_cyc,(float)add_cyc,float(min_opp_cyc),dang_opp_pos_val);
    Mem->LogAction4(30,"ballPredPos=(%.2f;%.2f)",ballPredPos.x,ballPredPos.y);
  }//cycle for cycles
  Mem->LogAction2(20,"CanPNHDribble: nothing todo");
#ifdef DEBUG_OUTPUT
  ostringstream ost;
  for(int i=6;i<=20;i+=2)
    ost<<i<<":"<<num_select_cycles[i]<<"; ";
  Mem->LogAction3(10,"CanPNHDribble: num_select_cycles= %s",const_cast<char*>(ost.str().c_str()));
  tc.Finish();
  num_times++;
  evar_time+=tc.GetTime();
  Mem->LogAction3(10,"CanPNHDribble: evar_time=%f",evar_time/(float)num_times);
#endif       
  
  return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
ActionQueueRes Dribble::PNHDribbleExecute(AngleDeg ang,Vector BallVelmodif){
  Mem->LogAction4(10,"Make pass to myself:to angle %f and with target vel %f",ang,BallVelmodif.mod());
  float ang2=-Mem->MyBodyAng()+ang; //ang-global; ang2- rel to body
  NormalizeAngleDeg(&ang2);
  TurnKickCommand command;
  kick.kickball(BallVelmodif.mod(),ang2,command);
  kick.ExecuteKickCommand(command);
  Mem->pass_to_myself=true;
  Mem->time_set_pass_to_myself=Mem->CurrentTime;
  return AQ_ActionQueued;
}
