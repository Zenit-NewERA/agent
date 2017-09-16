/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : positioning.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.21 $
 *
 *    $Id: positioning.C,v 2.21 2004/08/29 14:07:21 anton Exp $
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
 
#include "positioning.h"
#include "behave.h"
#include "Interception.h"

int Positioning::hear_time=0;
/////////////////////////////////////////////////////////
Positioning::Positioning(){
  save_stamina=true;
  buffer_at_point=0.5f;
  use_formation_dash=from_comm=false;
  pos_conf=vel_conf=dist=0.0f;
}
Positioning::~Positioning(){
}
////////////////////////////////////////////////////
//return true if doing something
bool Positioning::MoveToPos(Vector pos,float err_ang,float d,bool stop_now,float max_power){
  float target_ang  = GetNormalizeAngleDeg((pos - Mem->MyPredictedPosition()).dir() - Mem->MyBodyAng());
  float target_dist = Mem->DistanceTo(pos);
  Mem->LogAction5(40,"MoveToPos: Go to pos (%.2f,%.2f); distance: %.2f",pos.x,pos.y,target_dist);
  if(Mem->DistanceTo(pos)<buffer_at_point){
    Mem->LogAction2(50,"MoveToPos: already at target position");
    if(stop_now&&Mem->MyVelConf()&&Mem->MySpeed()>0.1f){
      Mem->LogAction3(50,"MoveToPos: my speed %.2f, try stop",Mem->MySpeed());
      StopNow();
      return true;
    }
    return false;
  }
  if(fabs(target_ang)<err_ang||(fabs(GetNormalizeAngleDeg(target_ang+180))<err_ang&&target_dist<d))
    return DashToPoint(pos,max_power);
  else{
    if(Mem->IsBetterStopEndTurn((pos - Mem->MyPredictedPosition()).dir())&&(FastestTm()==Mem->MyNumber||Mem->CP_goalie)
       &&Mem->DistanceTo(Mem->BallPredictedPosition())<Mem->MyPredictedPosition().dist(Mem->BallPredictedPosition())){
      Mem->LogAction2(10,"MoveToPos: better first stop, and then turn");
      StopNow();
    }else{
      Mem->LogAction3(50, "MoveToPos: turn to ang %f", target_ang);
      turn(target_ang);
    }
    return true;
  }
}
/////////////////////////////////////////////////////
bool Positioning::MoveBackToPos(Vector pos,float err_ang,float,float max_power){
  float target_ang  = GetNormalizeAngleDeg((pos - Mem->MyPredictedPosition()).dir() - Mem->MyBodyAng());
  float target_dist = Mem->DistanceTo(pos);
  Mem->LogAction5(40,"MoveBackToPos: Go to pos (%.2f,%.2f); distance: %.2f",pos.x,pos.y,target_dist);
  target_ang=GetNormalizeAngleDeg(target_ang+180);
  if(Mem->DistanceTo(pos)<buffer_at_point*0.5f){
    Mem->LogAction2(50,"MoveBackToPos: already at target position");
    if(Mem->MyVelConf()&&Mem->MySpeed()>0.1f){
      Mem->LogAction3(50,"MoveBackToPos: my speed %.2f, try stop",Mem->MySpeed());
      StopNow();
      return true;
    }
    return false;
  }
  if(fabs(target_ang)<err_ang)
    return DashToPoint(pos,max_power);
  else{
    Mem->LogAction3(50, "MoveBackToPos: turn to ang %f", target_ang);
    turn(target_ang);
    return true;
  }
}
/////////////////////////////////////////////////////
//The idea taken from UvA_Trealern 2002
void Positioning::MoveToPosAlongLine( Vector pos, AngleDeg ang,float dDistThr, int iSign, AngleDeg angThr, AngleDeg angCorr ){
  Line l ( Ray(pos, ang) );             
  Vector posAgent = Mem->MyPos();
  AngleDeg angBody  = Mem->MyBodyAng();
  Vector posProj  = l.ProjectPoint( posAgent );
  float dDist    = posAgent.dist( posProj );
  float dDiff    = pos.dist ( posProj );
  // if deviated too much from line, compensate
  if( dDist > dDistThr )
    {
      // check on which side of line agent is located
      Vector posOrg(0,0);
      Line m;
      m.LineFromTwoPoints( posOrg, posAgent );
      Vector posIntersect = l.intersection( m );
      int iSide;
      if( posAgent.dist(posOrg) < posIntersect.dist( posOrg ) )
	iSide = +1;
      else
	iSide = -1;
 
      // adjust desired turning angle to move back to line in coming cycles
      ang = ang + iSign * iSide * angCorr;
    }
 
  Mem->LogAction3( 50, "MoveToPosAlongLine: y difference to defend point %.3f", dDiff );
  // if current body angle differs much from desired turning angle, turn body
  if( fabs( GetNormalizeAngleDeg( ang - angBody ) ) > angThr ){
      Mem->LogAction4( 50, "MoveToPosAlongLine: angle differs too much body = %f, des = %f", angBody, ang );
      Vector target=posAgent + Polar2Vector( 1.0, ang);
      int num_cyc=Mem->NumCyclesToTurn((target-Mem->MyPos()).dir());
      Mem->LogAction3(10,"MoveToPosAlongLine: predict %.0f cycles to turn",
		      float(num_cyc));
      if(num_cyc>1&&GetDiff((Vector(0,0)-Mem->MyPos()).dir(),angBody)>20.0f)
	face_only_body_to_point(Vector(0.0f,0.0f));
      else
	face_only_body_to_point(target);
      return;
  }
  float buffer=Mem->PlayMode==PM_Play_On||Mem->PlayMode==PM_Their_PenaltyTaken||Mem->MyStamina()>=Mem->SP_stamina_max*0.95f?0.3f:1.0f;
  if(dDiff<buffer||(fabs(Mem->BallX()-Mem->MyX())>25.0f&&dDiff<2.5f)){
    Mem->LogAction3(50,"MoveToPosAlongLine: can rest (buffer=%.2f)",buffer);
    Vector target(Mem->MyX(),signf(Mem->BallY())*Mem->SP_pitch_width);
    int num_cyc=Mem->NumCyclesToTurn((target-Mem->MyPos()).dir());
    Mem->LogAction4(10,"Number cycles to turn to angle %.2f = %.0f",(target-Mem->MyPos()).dir(),float(num_cyc));
    if(fabs(Mem->BallY())>5.0f&&Mem->DistanceTo(Mem->BallAbsolutePosition())>8.0f*num_cyc)
      face_only_body_to_point(target);
    return;
  }
  DashToPoint( pos,777.0f );
}
/////////////////////////////////////////////////////
bool Positioning::DashToPoint(Vector pos,float max_power){//max_power>SP_max_power if want to use default power
  Vector vel=Mem->MyVelConf()?Mem->MyVel():Vector(0.0f,0.0f);
  float dAcc=(pos-Mem->MyPos()).rotate(-Mem->MyBodyAng()).x;
  if(dAcc>Mem->GetMyPlayerSpeedMax())
    dAcc=Mem->GetMyPlayerSpeedMax();
  dAcc-=vel.rotate(-Mem->MyBodyAng()).x;
  float power=dAcc/(Mem->MyEffort()*Mem->GetMyDashPowerRate());
  if(power>Mem->SP_max_power)
    power=Mem->SP_max_power;
  if(power<Mem->SP_min_power)
    power=Mem->SP_min_power;
  if(max_power!=777.0f)
    power=max_power;
  else
    if(use_formation_dash&&power>GetFormationDash())
      power=GetFormationDash();
  float correct_power=Mem->CorrectDashPowerForStamina(power);
  if(save_stamina&&fabs(power)>fabs(correct_power))
    power=correct_power;
  Mem->LogAction6(50,"DashToPoint: make dash with power %.0f (max_power= %.0f; CorrectDashPowerForStamina=%.2f; save_stamina=%.0f)",
		  power,max_power,correct_power,float(save_stamina));
  save_stamina=true;
  dash(power);
  return true;
}
/////////////////////////////////////////////////////
void Positioning::StopNow(){
  DashToPoint(Mem->MyPos(),777.0f);//hack - stop
}
/////////////////////////////////////////////////////
bool Positioning::GoToHomePosition(Vector pos,float dash){
  if(pos==Vector(-200.0f,-200.0f))
    pos=GetHomePosition();
  Mem->LogAction4(30, "GoToHomePosition:  going to home position (%.1f, %.1f)",
		  pos.x,pos.y);
  dash=dash>Mem->SP_max_power?GetFormationDash():dash;
  if(Mem->MyX()<=Mem->their_offside_line+0.5f&&GetMyType()>=PT_Midfielder)
    dash=Mem->SP_max_power;
  if(go_to_point(pos,Mem->CP_at_point_buffer,dash)==AQ_ActionNotQueued){
    Mem->LogAction2(30, "GoToHomePosition:  Already there");
    face_only_body_to_ball();
    return false;
  }
  return true;
}
//////////////////////////////////////////////////////
float Positioning::temp_PassConf(Vector from,Vector to,Unum* opp){
  Line l;
  l.LineFromTwoPoints(from,to);
  float res=1.0f;
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i)||
       (!l.InBetween(l.ProjectPoint(Mem->OpponentAbsolutePosition(i)),from,to)&&(to-Mem->OpponentAbsolutePosition(i)).mod()>2.0f))
      continue;
    Vector opp_pos=Mem->OpponentAbsolutePosition(i);
    Vector pp=l.ProjectPoint(opp_pos);
    float work=/*(to-opp_pos).mod()/(from-to).mod()+*/(pp-opp_pos).mod()/(pp-to).mod();
    work=(pp-opp_pos).mod()<3.0f?0.45f:work;
    work=work>1.0f?1.0f:work;
    if((to-opp_pos).mod()<=2.0f)
      work=work<0.5f?work:0.5f;
    if(res>work){
      res=work;
      if(opp!=0)
        *opp=i;
    }
  }
  return res;
}
/////////////////////////////////////////////////////////////
float Positioning::OurPositionValue(Vector pos){
  float val=(pos-Mem->MarkerPosition(Mem->RM_Their_Goal)).mod();
  return 1/val;
}
/////////////////////////////////////////////////////////////
float Positioning::TheirPositionValue(Vector pos){
  float val=(pos-Mem->MarkerPosition(Mem->RM_My_Goal)).mod();
  return 1/val;
}
//////////////////////////////////////////////////////////////
AngleDeg Positioning::GetMinOppAngleAbs(Vector from,Vector to,Unum* opp){
  Line l;
  l.LineFromTwoPoints(from,to);
  AngleDeg min_ang=360.0f,ang;
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i)||i==Mem->TheirGoalieNum) continue;
    ang=GetDiff((Mem->OpponentAbsolutePosition(i)-from ).dir(),(to-from).dir());
    if(l.InBetween(l.ProjectPoint(Mem->OpponentAbsolutePosition(i)),from,to)
       /*&&from.dist(l.ProjectPoint(Mem->OpponentAbsolutePosition(i)))>2.0f*/&&fabs(ang)<min_ang){
      min_ang=fabs(ang);
      if(opp!=0) *opp=i;
    }
  }
  return min_ang;
}
//////////////////////////////////////////////////////////////
Vector Positioning::GetOptimalPointOnLine(Vector from,Vector start,Vector end,AngleDeg* res_ang,AngleDeg StopAng,float step){
  Vector pos,OptPos=start;
  AngleDeg opt_ang=0.0f;
  float mod=(end-start).mod();
  mod=mod<0.01?0.01:mod;
  Vector direction=(end-start)/mod;
  for(float dist=0.0f;dist<=mod;dist+=step){
    pos=start+direction*dist;
    AngleDeg ang=GetMinOppAngleAbs(from,pos);
    if(ang>=StopAng){
      if(res_ang!=0) *res_ang=ang;
      return pos;
    }
    if(ang>opt_ang){
      opt_ang=ang;
      OptPos=pos;
    }
  }
  if(res_ang!=0) *res_ang=opt_ang;
  return OptPos;
}
//////////////////////////////////////////////////////////////
float Positioning::DotPassConf(Vector from,Vector to,Vector vel,Unum* opp){
  const float TRESHOLD=0.49f;
  Unum o;
  AngleDeg ang=GetMinOppAngleAbs(from,to,&o);
  if(ang<15.0f){
    Mem->LogAction4(50,"Pass dot conf hack: opp %.0f too close to pass line (ang %.2f) so his conf <= 0.49f",float(o),ang);
    if(opp!=0) *opp=o;
    return 0.49f;
  }
  for(int i=1;i<=11;i++){
    if(OppInetrceptionValue(i,from,to,vel)>TRESHOLD){
      if(opp!=0)
	*opp=i;
      return 0.0f;
    }
  }
  return 1.0f;//conf;
}
//////////////////////////////////////////////////////////////
float Positioning::TmPassConf(Unum tm,Vector from,Vector vel,Unum* opp){
  PlayerInterceptInfo tmInfo;
  static Vector PlayerPos[11];
  static Vector PlayerVel[11];
  static float PlayerAng[11];
  static bool AngValid[11];
  static Time update=-1;
  if(update<Mem->CurrentTime){
    for(int i=1;i<=11;i++){
      if(!Mem->OpponentPositionValid(i))
	continue;
      PlayerPos[i-1] = Mem->OpponentAbsolutePosition(i);
      PlayerVel[i-1]=Mem->OpponentVelocityValid(i)?Mem->OpponentAbsoluteVelocity(i):Vector(0,0);
      if (Mem->OpponentBodyAngleValid(i)) {
	AngValid[i-1] = TRUE;
	PlayerAng[i-1] = Mem->OpponentAbsoluteBodyAngle(i);
      }else{
	AngValid[i-1]=FALSE;
	PlayerAng[i-1] = 0;
      }
    }
    update=Mem->CurrentTime;
  }
  if(!Mem->TeammatePositionValid(tm)){
    if(opp!=0)
      *opp=Unum_Unknown;
    return 0.0f;
  }
  Mem->LogAction3(50,"Calculate for teammate %d",(int)tm);
  tmInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,from,vel,Mem->MySide,tm,GetTmPos(tm),
				Mem->TeammateVelocityValid(tm)?Mem->TeammateAbsoluteVelocity(tm):Vector(0,0),
				Mem->TeammateBodyAngleValid(tm)?Mem->TeammateAbsoluteBodyAngle(tm):0,(bool)Mem->TeammateBodyAngleValid(tm),FALSE);
  if(!Mem->IsSuccessRes(tmInfo.res)){
    if(opp!=0)
      *opp=Unum_Unknown;
    return 0.0f;
  }
  Unum o;
  AngleDeg ang=GetMinOppAngleAbs(from,tmInfo.pos,&o);
  if(fabs(ang)<25.0f){
    Mem->LogAction4(50,"Pass tm conf hack: opp %.0f too close to pass line (ang %.2f) so his conf <= 0.49f",float(o),ang);
    if(opp!=0) *opp=o;
    return fabs(ang)/250.0f;
  }
  return   DotPassConf(from,tmInfo.pos,vel,opp);//conf;
}
//////////////////////////////////////////////////////////////
float Positioning::OppInetrceptionValue(Unum opp,Vector from,Vector to,Vector vel)
{
  if(opp==Unum_Unknown||!Mem->OpponentPositionValid(opp)||opp>11)
    return 0.0f;
  
  static Time update_time[11]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  static Vector PlayerPos[11];
  static Vector PlayerVel[11];
  static float PlayerAng[11];
  static bool AngValid[11];
  static Time update=-1;
  PlayerInterceptInfo pInfo;
  
  if(update_time[opp-1]<Mem->CurrentTime){
    PlayerPos[opp-1] = Mem->OpponentAbsolutePosition(opp);
    PlayerVel[opp-1]=Mem->OpponentVelocityValid(opp)?Mem->OpponentAbsoluteVelocity(opp):Vector(0,0);
    if (Mem->OpponentBodyAngleValid(opp)) {
      AngValid[opp-1] = TRUE;
      PlayerAng[opp-1] = Mem->OpponentAbsoluteBodyAngle(opp);
    }else{
      AngValid[opp-1]=FALSE;
      PlayerAng[opp-1] = 0;
    }
    update_time[opp-1]=Mem->CurrentTime;
  }
  pInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,
			       from,vel,Mem->TheirSide,opp,
			       PlayerPos[opp-1],PlayerVel[opp-1],PlayerAng[opp-1],AngValid[opp-1],FALSE);
 		

  Line l(Ray(from,vel));
  if(Mem->IsSuccessRes(pInfo.res)){
    if(l.InBetween(l.ProjectPoint(pInfo.pos),from,to)){
      Mem->LogAction5(50,"opp %.0f (conf %.2f); num cyc:%.0f  is intercept ball",
		      float(opp),Mem->OpponentPositionValid(opp),float(pInfo.numCyc));
      return 1.0f;
    }
  }
  return 0.0f;
  
}
/////////////////////////////////////////////////////////////
void Positioning::UpdateFastestPlayers(){//call only one time at begin
  //log(0.5)/log(Mem->CP_ball_conf_decay)=6.57 - so use round value 6
  //log(0.7)/log(Mem->CP_conf_decay)=17.65 - so use round value 17

  float hear_p=pos_conf*(Mem->CurrentTime.t-hear_time>6?0:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime.t-hear_time));
  float hear_v=vel_conf*(Mem->CurrentTime.t-hear_time>17?0:int_pow(Mem->CP_conf_decay,Mem->CurrentTime.t-hear_time));

  //OLD CODE: vary slow at end of game : main problem in Italy ;)
  //float hear_p=pos_conf*int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime.t-hear_time);
  //float hear_v=vel_conf*int_pow(Mem->CP_conf_decay,Mem->CurrentTime.t-hear_time);
	
  if(!Mem->BallPositionValid()){
    FastestOpponentToBall=FastestTeammateToBall=Unum_Unknown;
    KnowOppInter=KnowTmInter=false;
    return;
  }
  float sum=Mem->BallPositionValid()+Mem->BallVelocityValid();
  if((sum<hear_p+hear_v||(sum==hear_p+hear_v&&from_comm))&&Mem->TeammatePositionValid(FastestTmFromCommunicate)){
    FastestTeammateToBall=FastestTmFromCommunicate;
  }else
    FastestTeammateToBall=Mem->FastestTeammateToBall();

  Mem->LogAction7(50,"Fastest:%.0f (comm:%.0f, my:%.0f) my sum:%.2f; comm sum: %.2f",float(FastestTeammateToBall),
		  float(FastestTmFromCommunicate),float(Mem->FastestTeammateToBall()),sum,hear_p+hear_v);

  FastestOpponentToBall=Mem->FastestOpponentToBall();
  KnowTmInter=!(FastestTeammateToBall==Unum_Unknown||!Mem->TeammateInterceptionAble(FastestTeammateToBall));
  KnowOppInter=!(FastestOpponentToBall==Unum_Unknown||!Mem->OpponentInterceptionAble(FastestOpponentToBall));

  tm_cyc=(!KnowTmInter?10000:Mem->TeammateInterceptionNumberCycles(FastestTeammateToBall));
  opp_cyc=(!KnowOppInter?10000:Mem->OpponentInterceptionNumberCycles(FastestOpponentToBall));
  if(KnowOppInter)
    opp_pos=Mem->OpponentInterceptionPoint(FastestOpponentToBall);
  if(KnowTmInter)
    tm_pos=Mem->TeammateInterceptionPoint(FastestTeammateToBall);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void Positioning::UpdateFastestTmToBall(Unum fastest,float d,float pconf,float vconf){
  //log(0.5)/log(Mem->CP_ball_conf_decay)=6.57 - so use round value 6
  //log(0.7)/log(Mem->CP_conf_decay)=17.65 - so use round value 17

  float hear_p=pos_conf*(Mem->CurrentTime.t-hear_time>6?0:int_pow(Mem->CP_ball_conf_decay,Mem->CurrentTime.t-hear_time));
  float hear_v=vel_conf*(Mem->CurrentTime.t-hear_time>17?0:int_pow(Mem->CP_conf_decay,Mem->CurrentTime.t-hear_time));

  if(hear_p+hear_v<pconf+vconf||(hear_p+hear_v==pconf+vconf&&d<dist)){
    FastestTmFromCommunicate=fastest;
    dist=d;
    pos_conf=pconf;
    vel_conf=vconf;
    hear_time=Mem->CurrentTime.t;
    if(Mem->BallPositionValid()+Mem->BallVelocityValid()>=pos_conf+vel_conf)
      from_comm=false;
    else
      from_comm=true;
  }

}
//////////////////////////////////////////////////////////////////////////////////////////////
Unum Positioning::SelectOptimalPlayer(int PT_mask,int PS_mask,Vector position){//select closest teammate to ball
  Unum opt_tm=Unum_Unknown;
  float opt_dist=1000.0f;
  Iterator iter=begin();
  while(iter!=end()){
    Unum tm=GetPlayerNumber(iter,PT_mask,PS_mask);
    if(tm==Unum_Unknown)
      break;
    if(!Mem->TeammatePositionValid(tm))
      continue;
    if((position-Mem->TeammateAbsolutePosition(tm)).mod()<=opt_dist){
      opt_dist=(position-Mem->TeammateAbsolutePosition(tm)).mod();
      opt_tm=tm;
    }
  }
  return opt_tm;
}
////////////////////////////////////////////////////////////////////////////////////
float Positioning::GetFormationDash(Unum tm){
  if(tm==Mem->MyNumber&&important_action){
    important_action=false;
    return 100.0;
  }
  if(IsOffense()&&GetPlayerType(tm)<=PT_Midfielder)
    return 50.0;
  if(IsDefense()&&GetPlayerType(tm)==PT_Midfielder)
    return 50.0;
  if((IsDefense()||Mem->BallX()<0)&&GetPlayerType(tm)==PT_Forward)
    return 50.0f;
  if(IsDefense()&&Mem->TeammateX(tm)>-20.0f&&GetPlayerType(tm)<PT_Midfielder)
    return 50.0;
  return Formations::GetFormationDash(tm);
}
///////////////////////////////////////////////////////////////////////////////////
Unum Positioning::check_for_free_opponents_in_own_penalty_area(void){
  Unum closestOpp[11];
  Unum ourTeam[11];
  if(Mem->PlayMode==PM_Play_On&&GetMyType()==PT_Forward)
    return Unum_Unknown;
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,Mem->MyPos(),closestOpp);
  for(int i=0;i<numClosestOpp;i++){
    if(!Mem->OpponentPositionValid(closestOpp[i])||
       Mem->OpponentDistanceTo(closestOpp[i],Mem->MarkerPosition(Mem->RM_My_Goal))>(GetMyType()<=PT_Defender?20.0f:35.0f)||
       FastestOpp()==closestOpp[i]||
       (Mem->OpponentX(closestOpp[i])>-30.0f&&Mem->MyX()>-20.0f))
      continue;
    if(GetMyType()<=PT_Defender&&((Mem->OpponentVelocityValid(closestOpp[i])&&Mem->OpponentSpeed(closestOpp[i])>0.1)||
	!Mem->OpponentVelocityValid(closestOpp[i]))&&Mem->OpponentX(closestOpp[i])>-30.0f)//hack
      continue;
    int numOurClosest=Mem->SortPlayersByDistanceToPoint('m' ,Mem->OpponentAbsolutePosition(closestOpp[i]),ourTeam);
    for(int j=0;j<numOurClosest;j++){
      if(ourTeam[j]==Mem->MyNumber){ //we must mark THIS player
	return  closestOpp[i];
      }
      if(ourTeam[j]==Mem->OurGoalieNum||Mem->TeammateTackling(ourTeam[j]))
	continue;
      if(Mem->ClosestOpponentTo(Mem->TeammateAbsolutePosition(ourTeam[j]))==closestOpp[i]){ //other player is closest to this player
	Mem->LogAction6(50,"CFFOIOPA:opp %.0f(conf %.2f) must mark by tm %.0f(conf %.2f)",float(closestOpp[i]),
			Mem->OpponentPositionValid(closestOpp[i]),float(ourTeam[j]),Mem->TeammatePositionValid(ourTeam[j]));
	break;
      }
    }
  }
  return Unum_Unknown;
}
//////////////////////////////////////////////////////////////////////////////////////
bool Positioning::close_goalie_intercept(Vector pred_pos)
{
  if(Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum)||NumOfCyclesThenILastSeePlayer(-Mem->TheirGoalieNum)>3)
    return false;
  float danger_dist=Mem->SP_catch_area_l*1.15f;
  if((pred_pos-Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)).mod()<=danger_dist)
    return true;
  if((pred_pos-Mem->OpponentPredictedPosition(Mem->TheirGoalieNum)).mod()<=danger_dist)
    return true;
  if(Mem->OpponentBodyAngleValid(Mem->TheirGoalieNum)<0.98f||
     fabs(GetNormalizeAngleDeg(Mem->OpponentAbsoluteBodyAngle(Mem->TheirGoalieNum)-Mem->BallAbsolutePosition().dir()))<90.0f)
    return false;
  Vector vel=Polar2Vector(Mem->SP_max_power*Mem->SP_dash_power_rate,Mem->OpponentAbsoluteBodyAngle(Mem->TheirGoalieNum));
  if((pred_pos-Mem->OpponentPredictedPosition(Mem->TheirGoalieNum,1,vel)).mod()<=danger_dist)
    return true;
  vel=Polar2Vector(Mem->SP_max_power*Mem->SP_dash_power_rate,
		   GetNormalizeAngleDeg(180+Mem->OpponentAbsoluteBodyAngle(Mem->TheirGoalieNum)));
  if((pred_pos-Mem->OpponentPredictedPosition(Mem->TheirGoalieNum,1,vel)).mod()<=danger_dist)
    return true;
  return false;
}
//////////////////////////////////////////////////////////////////////
bool Positioning::CheckWithoutTackle(Vector BallPredPos,CheckType type,bool print_log,float side,float dist_stop_dribble)
{
  Unum goalie=Mem->TheirGoalieNum;
  Unum Opp[11];
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,BallPredPos,Opp);
  int cyc=777;
  float goalie_dist=2.5f;
  
  if(type==CT_PenaltyShoot)
    goalie_dist=dist_stop_dribble;
  else if(type>=CT_VeryAgressive)
    goalie_dist=Mem->SP_catch_area_l*1.05f;
  else if(IsGoalieActive(side))
    goalie_dist=5.0f;
  else if(fabs(Mem->BallY())>4.5f&&goalie!=Unum_Unknown&&Mem->OpponentPositionValid(goalie)&&fabs(Mem->BallY())>fabs(Mem->OpponentY(goalie))+1.0f)
    goalie_dist=6.0f+(float)min(3,NumOfCyclesThenILastSeePlayer(-goalie));
  if(print_log&&goalie!=Unum_Unknown&&Mem->OpponentPositionValid(goalie))
    Mem->LogAction5(50,"CheckWithoutTackle: goalie pos_valid=%.2f with dist=%.2f; (dist_stop_dribble=%.2f)",
		    Mem->OpponentPositionValid(goalie),Mem->OpponentDistanceTo(goalie,Mem->MyPos()),dist_stop_dribble);
    
  for(int i=0;i<numClosestOpp;i++){
    cyc=777;
    if(Opp[i]==Unum_Unknown||!Mem->OpponentPositionValid(Opp[i]))
      continue;
    if((Opp[i]!=Mem->TheirGoalieNum&&(cyc=Mem->EstimatedCyclesToSteal(Opp[i],BallPredPos))<(type>=CT_Agressive?2:3)
	&&Mem->OpponentPositionValid(Opp[i])>=(Mem->ViewWidth==VW_Narrow?0.99f:0.98f))||
        (Opp[i]==goalie&&(Mem->OpponentDistanceTo(goalie,Mem->MyPos())<=goalie_dist||
	(type!=CT_PenaltyShoot&&((cyc=(Mem->EstimatedCyclesToSteal(Opp[i],BallPredPos)-min(3,NumOfCyclesThenILastSeePlayer(-Opp[i]))))
	 <=(IsGoalieActive(side)?2:1))&&
	  Mem->OpponentBodyAngleValid(Opp[i])>0.97f)))){
      if(!Mem->OpponentTackling(Opp[i])){
	if(print_log)
	  Mem->LogAction7(50,"CheckWithoutTackle:point (%.2f,%.2f) is not valid, becouse opp %.0f is too close(conf %.2f; num cyc=%.0f)"
			  ,BallPredPos.x,BallPredPos.y,float(Opp[i]),Mem->OpponentPositionValid(Opp[i]),float(cyc));
        return false;
      }
    }else{
      if(print_log)
        Mem->LogAction7(50,"CheckWithoutTackle:point (%.2f,%.2f) is valid,becouse opp %.0f is not close(conf %.2f; num cyc=%.0f)",
			BallPredPos.x,BallPredPos.y,float(Opp[i]),Mem->OpponentPositionValid(Opp[i]),float(cyc));
      
      if(!Mem->TheirPenaltyArea.IsWithin(BallPredPos)||Opp[i]==Mem->TheirGoalieNum)
	break;
    }      
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
bool Positioning::CheckTackleOpportunity(Vector BallPredPos,bool print_log)
{
  if(!IsOpponentUseTackle())
    return false;
  Unum goalie=Mem->TheirGoalieNum;
  Unum Opp[11];
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,BallPredPos,Opp);
  for(int i=0;i<numClosestOpp;i++){
    if(Opp[i]==Unum_Unknown||!Mem->OpponentPositionValid(Opp[i])||Mem->OpponentTackling(Opp[i])||
       Mem->OpponentDistanceTo(Opp[i],BallPredPos)>3.0f||Opp[i]==goalie)
      continue;
    if(Mem->OpponentPositionValid(Opp[i])<(Mem->ViewWidth==VW_Narrow?0.99f:0.98f))
      continue;
    AngleDeg body_ang;
    float body_conf=Mem->OpponentBodyAngleValid(Opp[i]),vel_conf=Mem->OpponentVelocityValid(Opp[i]);
    Vector opp_pos=(vel_conf?Mem->OpponentAbsolutePosition(Opp[i])+Mem->OpponentAbsoluteVelocity(Opp[i]):Mem->OpponentAbsolutePosition(Opp[i]));
    if(body_conf==1.0f)
      body_ang=Mem->OpponentAbsoluteBodyAngle(Opp[i]);
    else
      body_ang=(BallPredPos-opp_pos).dir();
    Vector player_2_ball=(BallPredPos-opp_pos).rotate(-body_ang);
    if(print_log){
      Mem->LogAction6(10,"CheckTackleOpportunity: opp=%.0f; pos_conf=%.2f; body_conf=%.2f; vel_conf=%.2f",
		 float(Opp[i]),Mem->OpponentPositionValid(Opp[i]),body_conf,vel_conf);
      Mem->LogAction4(50,"CheckTackleOpportunity: player_2_ball=(%.4f,%.4f)",player_2_ball.x,player_2_ball.y);
    }
    if(player_2_ball.x>=-0.4f&&player_2_ball.x<=/*1.65f*/1.7f){
      if(print_log)
	Mem->LogAction5(50,"CheckTackleOpportunity:opponent %.0f is danger with/without turn for point (%.2f,%.2f)",
			float(Opp[i]),BallPredPos.x,BallPredPos.y);
      return true;
    }
    Vector vel=Polar2Vector(Mem->SP_max_power*Mem->SP_dash_power_rate,body_ang);
    player_2_ball=(BallPredPos-(opp_pos+vel)).rotate(-body_ang);
    if(print_log)
      Mem->LogAction4(50,"CheckTackleOpportunity: after dash player_2_ball=(%.4f,%.4f)",player_2_ball.x,player_2_ball.y);
    if(player_2_ball.x>=-0.4f&&player_2_ball.x<=/*1.65f*/1.8f&&fabs(player_2_ball.y)<=/*0.7f*/0.85f){
      if(print_log)
	Mem->LogAction5(50,"CheckTackleOpportunity:opponent %.0f is danger after dash for point (%.2f,%.2f)",
			float(Opp[i]),BallPredPos.x,BallPredPos.y);
      return true;
    }
  }
  return false;
}
//////////////////////////////////////////////////////////////////////
bool Positioning::CheckWithTackle(Vector BallPredPos,CheckType type,bool print_log,float side,float dist_stop_dribble)
{
  if(!CheckWithoutTackle(BallPredPos,type,print_log,side,dist_stop_dribble))
    return false;
  if(CheckTackleOpportunity(BallPredPos,print_log))
    return false;
  return true;
}
