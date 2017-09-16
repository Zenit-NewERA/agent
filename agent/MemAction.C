/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemAction.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.22 $
 *
 *    $Id: MemAction.C,v 2.22 2004/06/26 08:26:08 anton Exp $
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
 
/* MemAction.C
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

#include "MemAction.h"
#include "client.h"
#include "behave.h"

#include "dribble.h"
#include "Handleball.h"

#define DEBUG_OUTPUT

#ifdef DEBUG_OUTPUT
#define DebugInter(x) x 
#else
#define DebugInter(x) 
#endif
/*****************************************************************************************/
void TurnKickCommand::kick(float kickpower, AngleDeg kickangle) {
  type = CMD_kick;
  angle = kickangle;
  power = kickpower;
  time = Mem->CurrentTime;
};
/*****************************************************************************************/
void TurnKickCommand::dash(float dashpower) {
  type = CMD_dash;
  power = dashpower;
  time = Mem->CurrentTime;
}
/*****************************************************************************************/
void TurnKickCommand::turn(AngleDeg turnangle) {
  type = CMD_turn;
  angle = turnangle;
  time = Mem->CurrentTime;
}
/*****************************************************************************************/
void TurnKickCommand::turnneck(AngleDeg turnneckangle) {
  turnneckneeded = true;
  this->turnneckangle=turnneckangle;
  time=Mem->CurrentTime;
}
/*****************************************************************************************/
void TurnKickCommand::Reset() {
  type = CMD_none;
  turnneckneeded = false;
  time = -1;
}
/*****************************************************************************************/
bool TurnKickCommand::Valid() {
  return type!=CMD_none && time==Mem->CurrentTime;
}
/*****************************************************************************************/
bool TurnKickCommand::Valid(Time checktime) {
  return type!=CMD_none && time==checktime;	
}
/*****************************************************************************************/
CMDType TurnKickCommand::CommandType() {
  return type;
}
/*****************************************************************************************/
bool TurnKickCommand::TurnNeckNeeded() {
  return turnneckneeded;
}
/*****************************************************************************************/
void ActionInfo::Initialize()
{

  Stored_Fastest_Teammate_Time = 0;
  Stored_Fastest_Opponent_Time = 0;


  ChangeViewForHandleBallTime = 0;


  for (int i = 1; i <= SP_team_size; i++) {
    TeamIntInfo[i] = new PlayerInterceptInfo;
    TeamIntInfo[i]->time = -1;
    OppIntInfo[i] = new PlayerInterceptInfo;
    OppIntInfo[i]->time = -1;
  }

  kick_in_progress = FALSE;

  ////////////////////////

  DribbleTargetAvailable = FALSE;
  DribbleTargetTime = -1;

  ///////////////////////

  ball_info_time = -40;
  ScenarioCaptain = Unum_Unknown;
  //  CurrentScenario = ScenarioUnknown;
	
  InterceptLookahead = LA_Default;
  IntMinCyc = -1;
  IntMinCycTime = -1;

  HKTime = -1;
  HKStep = -1;
  HKStepNext = -1;
  HKrot = TURN_CW;
  //init table of sin/cos
  for(int i=0;i<=180;i++){
    Sin_1DegStep_ar[i]=Sin(i);
    Cos_1DegStep_ar[i]=Cos(i);
  }
}




/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/





/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

#ifdef DEBUG_OUTPUT
#define DebugInt(x) 
#else
#define DebugInt(x)
#endif

/* only used for this player */
PlayerInterceptInfo
ActionInfo::CloseBallInterception(float max_pow, int max_lookahead,
				  Vector vBallPos, Vector vBallVel)
{
  Vector vNewPos;
  PlayerInterceptInfo info;
  float dash_dist = max_pow * GetMyDashPowerRate();
  info.dash_pow = max_pow;
  info.lookahead = max_lookahead;
  info.res = BI_None;  
  
  vBallPos += vBallVel;
  vBallVel *= SP_ball_decay;
  /* we need to figure out the right dash power so that the ball ends up right in
     front of us. Like many things, this ends up as a LineCircleIntersect problem */
  Vector vMyPred = MyPredictedPosition();
  Ray    rDash(vMyPred, MyBodyAng());
  Line   lDash   = LineFromRay(rDash);
  Vector sol1, sol2;
  info.ang_error=5.0;

  Mem->LogAction2(10,"Enter in CloseBallInterception");
  //AI: predict dribble
  Unum opp;
  if((PlayMode==PM_Play_On||PlayMode==PM_My_PenaltyTaken)&&(vBallPos-vMyPred).mod()<GetMyKickableArea()-0.1f&&
     ((opp=Mem->ClosestOpponentTo(vBallPos))==Unum_Unknown||Mem->OpponentDistanceTo(opp,vBallPos)>2.0f)){
    Mem->LogAction4(10,"CloseBallInterception: predict control ball in next cycle without dash (dist=%.3f; kickable_arae=%.3f)",
		    (vBallPos-vMyPred).mod(),GetMyKickableArea());
    Vector target;
    if(actions.TeammateShootConf(Mem->MyNumber,1.0f,vBallPos)>0.8||ClosestOpponentToBallDistance()<1.0f
       ||((vBallPos-vMyPred).mod()>0.2f&&(vBallPos-vMyPred).mod()<GetMyPlayerSize()+SP_ball_size+CP_collision_buffer))
      target=vBallPos;
    else
      target=Dribble::SelectDribbleTarget();
    info.numCyc=1;
    info.res=BI_OnlyTurn;
    info.pos=target;//hack
    info.dash_pow_to_use=0.0f;
    return info;
  }
  
    
  //AI: try avoid collision problem
  float add=CP_collision_buffer;//BallVelocityValid()<=0.95f?0.3f:CP_collision_buffer;
  int num_sol = LineCircleIntersect(lDash, GetMyPlayerSize() + SP_ball_size + add,
				    vBallPos, &sol1, &sol2);
  //AI: try improve this algorithm
//   if(num_sol==0){
//     num_sol = LineCircleIntersect(lDash, GetMyKickableArea()*0.9f, vBallPos, &sol1, &sol2);
//     if(num_sol>0)
//       Mem->LogAction2(10,"CloseBallInterception: try second step");
//   }
  
  if (num_sol >= 1) {
    /* we'll make sure that the right answer is in sol1 */
    if (num_sol == 2) {
      /* we have to pick which point is better */
      if (fabs(GetNormalizeAngleDeg((vBallPos - sol2).dir() - MyBodyAng())) < 90) {
	sol1 = sol2; //sol2 is the right solution, put it in sol1
      } else if (!(fabs(GetNormalizeAngleDeg((vBallPos-sol1).dir() - MyBodyAng())) < 90)) {
	my_error("CloseBallInterception: 1 ahead, neither solution looks good %.2f %.2f",
		 GetNormalizeAngleDeg((vBallPos - sol2).dir() - MyBodyAng()),
		 GetNormalizeAngleDeg((vBallPos - sol1).dir() - MyBodyAng()));
      }
    }

    /* now figure out the dash power based on that point */
    float dash_pow = vMyPred.dist(sol1) / GetMyDashPowerRate();
    dash_pow = MinMax(SP_min_power, dash_pow, SP_max_power);
    if (!rDash.InRightDir(sol1))
      dash_pow = -dash_pow;
    float dist_after=vBallPos.dist(MyPredictedPosition(1, dash_pow));
    if (dist_after < GetMyKickableArea()) {
      /* this works! */
      info.res = BI_CanChase;
      info.numCyc = 1;
      info.dash_pow_to_use = dash_pow;
      //this'll make go_to_point dash
      info.pos = vMyPred + Polar2Vector(signf(dash_pow)*dash_dist, MyBodyAng());
      LogAction7(70, "CloseBallInterception: One dash and we're there: %.2f to (%.2f, %.2f); dist_after=%.2f (kickable_area=%.2f)",
		 dash_pow, sol1.x, sol1.y,dist_after,GetMyKickableArea());
      return info;
    }
    
    
  }
  
  vBallPos += vBallVel;
  vBallVel *= SP_ball_decay;
  //now look two cycles ahead
  //try turn then dash
  float targ_ang = (vBallPos - MyPredictedPosition(2)).dir();
  if (fabs(targ_ang - MyBodyAng()) > CP_max_go_to_point_angle_err) {
    vNewPos = MyPredictedPositionWithTurn(targ_ang - MyBodyAng(), 2, max_pow);
    if (vNewPos.dist(vBallPos) < GetMyKickableArea()) {
      info.res = BI_CanChase;
      info.numCyc = 2;
      info.dash_pow_to_use = max_pow;
      //this'll make go_to_point turn
      //info.pos = MyPos() + Polar2Vector(dash_dist, targ_ang); 
      info.pos = MyPredictedPosition() + Polar2Vector(dash_dist, targ_ang); 
      LogAction2(70, "CloseBallInterception: Turn then dash and we're there");
      return info;
    }
  }
  //try two dashes
  vNewPos = MyPredictedPosition(2, max_pow);
  if (vNewPos.dist(vBallPos) < GetMyKickableArea()) {
    info.res = BI_CanChase;
    info.numCyc = 2;
    info.dash_pow_to_use = max_pow;
    //this'll make go_to_point dash
    //info.pos = MyPos() + Polar2Vector(2*dash_dist, MyBodyAng()); 
    info.pos = MyPredictedPosition() + Polar2Vector(dash_dist, MyBodyAng());
    LogAction2(70, "CloseBallInterception: Two dashes and we're there");
    return info;
  }
  return info;
}
//~///////////////////////////////////////////////////////////////////////////////////
bool ActionInfo::GoalieCloseBallInterception(Vector vBallPos, Vector vBallVel)
{
  Vector vMyPos=Mem->MyPredictedPosition();
	
  vBallPos+=vBallVel;
  vBallVel*=SP_ball_decay;
	
  Ray    rDash(vMyPos, MyBodyAng());
  Line gLine(rDash);
  //	gLine.LineFromTwoPoints(vMyPos,Vector(vMyPos.x,77.0f));
  float dDist=SP_player_speed_max+SP_ball_speed_max+SP_catch_area_l;
  if(DistanceTo(Mem->BallAbsolutePosition())<dDist){
    Vector closePos=gLine.ProjectPoint(vBallPos);
    float dash_pow = vMyPos.dist(closePos) / (MyEffort()*GetMyDashPowerRate());
    dash_pow = MinMax(SP_min_power, dash_pow, SP_max_power);
    if (!rDash.InRightDir(closePos))
      dash_pow = -dash_pow;
    Vector posPred=Mem->MyPredictedPosition(1,dash_pow);
    if(vBallPos.dist(posPred)<SP_catch_area_l){
      dash(dash_pow);
      LogAction5(70, "GoalieCloseBallInterception: One dash and we're there: %.2f to (%.2f, %.2f)",
		 dash_pow, closePos.x, closePos.y);
      return true;
    }
  }
  dDist= SP_ball_speed_max*(1.0+SP_ball_decay)+2*SP_player_speed_max+SP_catch_area_l;
  if(DistanceTo(Mem->BallAbsolutePosition())<dDist){
    vBallPos += vBallVel;
    vBallVel *= SP_ball_decay;
    //try turn then dash
    float targ_ang = (vBallPos - MyPredictedPosition(2)).dir();
    if (fabs(targ_ang - MyBodyAng()) > 7.0f) {
      Vector vNewPos = MyPredictedPositionWithTurn(targ_ang - MyBodyAng(), 2, SP_max_power);
      if (vNewPos.dist(vBallPos) < SP_catch_area_l*0.9f) {
	face_only_body_to_point(vBallPos);
	LogAction2(70, "GoalieCloseBallInterception: Turn then dash and we're there");
	return true;
      }
    }
    //try two dashes
    Vector vNewPos=MyPredictedPosition(2,SP_max_power);
    if (vNewPos.dist(vBallPos) < SP_catch_area_l) {
      dash(SP_max_power);//was sign*dash_power
      LogAction2(70, "GoalieCloseBallInterception: Two dashes and we're there");
      return true;
    }
		
  }

  return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//Add by AI: change of PlayerPredictedPosition
Vector EstimatePlayerNewPos(char side,Unum num,Vector pos,Vector vel,int steps,Vector dash){
  Vector position=pos;
  for (int i=0; i<steps; i++){
    vel += dash;
    if ( vel.mod() >Mem->GetPlayerPlayerSpeedMax(side,num) )
      vel *= ( Mem->GetPlayerPlayerSpeedMax(side,num)/vel.mod() );
    position += vel;
    vel *= Mem->GetPlayerPlayerDecay(side,num);
  }
  return position;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// does not set time field 
//cyc inc is initially CP_intercept_step, but when an answer is found, we
// then bring cyc back a step, and go up by ones 
PlayerInterceptInfo
ActionInfo::ActiveCanGetThere(float max_pow, int max_lookahead,
			      Vector vBallPos, Vector vBallVel,
			      char side, Unum num,
			      Vector vPlayerPos, Vector vPlayerVel,
			      float fPlayerAng, int PlayerAngValid,
			      bool IsThisMe)
{
  float at_point_buffer = (side==TheirSide)?GetOpponentKickableArea(num)*1.2f:GetTeammateKickableArea(num)*0.95f;
  //next two lines were added by Serg
  if( side==Mem->MySide&&num==Mem->OurGoalieNum)
    at_point_buffer=SP_catch_area_l;
  if(side==Mem->TheirSide&&num==Mem->TheirGoalieNum)
    at_point_buffer=SP_catch_area_l*1.1f;//1.5f;
  PlayerInterceptInfo info;
  //Vector vPredPlayer = vPlayerPos + vPlayerVel;
  Vector vPredPlayer = vPlayerPos +
    vPlayerVel * (SumInfGeomSeries(vPlayerVel.mod(),GetPlayerPlayerDecay(side,num)));
  Vector vOldBallPos, vOldBallVel;
  float turn_angle;
  int cyc;
  int cyc_inc = (IsThisMe ? CP_my_intercept_step : CP_intercept_step); 
  int max_cyc = (max_lookahead + cyc_inc - 1);  
  max_cyc -= (max_cyc % cyc_inc); 
  //   max_cyc is so that we don't miss an interception if CP_intercept_step is not
  //   1. For example, if CP_intercept_step is 5, max_look is 14, and we can
  //   intercept in 13, we would return no intercept if we just used max_lookahead
  
  DebugInt(printf(" ACGT: BallPos.vel.mod: %f\n", vBallVel.mod() ));

  info.dash_pow_to_use = max_pow;
  NormalizeAngleDeg(&fPlayerAng);

  // we want to aim a little ahead of the ball, so advance it a little
  //AI: comment this becouse this is true only if IsThisMe==true
//   for (int i=0; i < CP_intercept_aim_ahead; i++) { //DANGER
//     vBallPos += vBallVel;
//     vBallVel *= SP_ball_decay;
//   }

  //  if (IsThisMe) LogAction4(140, "ActiveBallIntercept: %d %d", (int)max_pow, max_lookahead);
  
  for (cyc=0; cyc<=max_cyc; cyc += cyc_inc) {

    if (!IsPointInBounds(vBallPos,-300)) {  // expand the field by 3 meters so we don't give up to soon
      DebugInt(printf("The ball will go out of bounds before we can get it\n"));
      break;
    }
    
    // decide if we need to turn to ball
    float ball_ang = (vBallPos - vPredPlayer).dir();
    Vector vEndSpot;
    // SMURF - we should probably aim for ball 1 cycle ahead or something
    //   like that
    //DebugInt(printf(" angle to exp ball pos: %f\n", AngleTo(vBallPos)));
    turn_angle = ball_ang - fPlayerAng;
    if (fabs(turn_angle) < CP_max_go_to_point_angle_err)
      turn_angle = 0.0;      
    if (IsThisMe) {
      vEndSpot = MyPredictedPositionWithTurn(turn_angle, cyc, max_pow,(turn_angle != 0.0));
    } else {
      int run_cyc = cyc;
      if (PlayerAngValid&&!(side==TheirSide&&!BallKickable()&&Pos.GetMyType()<=PT_Midfielder)) {//AI: last cond is hack
	if (turn_angle != 0.0)
	  run_cyc--;
	run_cyc = Max(0, run_cyc);
      }
      Vector PlayerDash =
	Polar2Vector(max_pow*SP_dash_power_rate, ball_ang);
      vEndSpot =
	PlayerPredictedPosition(side, num, run_cyc, PlayerDash);

    }
    

    float dist_to_ball_after = (vBallPos - vEndSpot).mod();
    // if we can make it there
    //SMURF- is this too lenient?
    if (dist_to_ball_after <= at_point_buffer ||
	(vEndSpot - vPredPlayer).mod() > (vBallPos - vPredPlayer).mod() + GetPlayerKickableArea(side,num)) {
      // we can get to the ball!
      // OR we travelled far enough, but somehow missed the ball,
      //  return sucess
      if (dist_to_ball_after <= at_point_buffer) {
	//	if (IsThisMe) LogAction4(100, "Found a ball interception by being close (%.2f, %.2f)",
	//				vBallPos.x, vBallPos.y);
	info.numCyc = cyc;
      } else {	
	//	if (IsThisMe) LogAction4(100, "Found a ball interception by going far (%.2f, %.2f)",
	//				vBallPos.x, vBallPos.y);
	info.numCyc = cyc;
	//vBallPos += vBallVel; // advance one spot for that turn
      }
      
      if (cyc_inc > 1 && cyc != 0) {
	// we want the best answer- go back and go up by ones
	//	if (IsThisMe)
	//	  LogAction2(100, "Found a ball interception, but goign back for accuracy");	
	DebugInt(printf("Found answer, but going back for accuracy: %d\n", cyc));
	cyc -= cyc_inc;
	vBallPos = vOldBallPos;
	vBallVel = vOldBallVel;
	cyc_inc = 1;
	max_cyc = max_lookahead; // don;t need to go above this anymore
      } else {
	// we want to try avoiding turning towards the ball for only a small savings
	//   in time to intercept
	if (IsThisMe && CP_no_turn_max_cyc_diff > -1 &&
	    turn_angle != 0.0 &&
	    (vBallVel.x >= FLOAT_EPS || vBallVel.y >= FLOAT_EPS)) {
	  Ray rBall(vBallPos, vBallVel);
	  Ray rPlayer(vPredPlayer, fPlayerAng);
	  Vector int_pt;
	  if (rBall.intersection(rPlayer, &int_pt)) {
	    float dist = vEndSpot.dist(int_pt);
	    float num_cyc; // the number of cycles extra it takes the ball to get to this pos
	    num_cyc = SolveForLengthGeomSeries(vBallVel.mod(), SP_ball_decay, dist);
	    LogAction3(90, "No turn interception: It takes %.2f extra cycles", num_cyc);
	    // if an answer less than 0 is given, the ball will never get there
	    if (num_cyc >= 0 &&
		num_cyc <= CP_no_turn_max_cyc_diff) {
	      // use this target instead!
	      LogAction4(70, "Using the new no turning interception point (%.2f, %.2f)",
			 int_pt.x, int_pt.y);
	      info.res = BI_CanChase;
	      info.pos = int_pt;
	      return info;
	    } // using no turn interseption
	  } // there is an intersection
	  
	} // no turn interseption
	

	if (info.numCyc > max_lookahead) {
	  info.res = BI_Failure;
	} else {
	  info.res = BI_CanChase;
	  info.pos = vBallPos;
	}	
	return info;
      }      
    }
    
    // update ball position estimate
    vOldBallPos = vBallPos;
    vOldBallVel = vBallVel;
    for (int i=0; i < cyc_inc; i++) {
      vBallPos += vBallVel;
      vBallVel *= SP_ball_decay;
    }
      
    
  } // cycle loop

  info.res = BI_Failure; // can't make it to ball before max_lookahead
  return info;
}
/*****************************************************************************************/
PlayerInterceptInfo ActionInfo::SmartCanGetThere(float max_pow,
						 Vector vBallPos, Vector vBallVel,
						 Vector vPlayerPos, Vector vPlayerVel,
						 float fPlayerAng){

  const float ANG_ERROR_MAX=18.0;
  PlayerInterceptInfo info;
  InterceptionInput data;
  PlayerInterceptInfo res[4];
  
  data.vBallPos=vBallPos;
  data.vBallVel=vBallVel;
  data.vPlayerPos=vPlayerPos;
  data.vPlayerVel=vPlayerVel;
  data.fPlayerAng=fPlayerAng;
  data.stamina=MyStamina();
  data.effort=MyEffort();
  data.recovery=MyRecovery();

  for(int i=0;i<4;i++){
    res[i].dash_pow_to_use=max_pow;
    res[i].res=BI_CanChase;
  }
  info.dash_pow_to_use=max_pow;
  info.res=BI_CanChase;
//   //TEMP
  for(int i=1;i<11;i++){
    if(i==Mem->MyNumber||!Mem->TeammatePositionValid(i)||!Mem->TeammateInterceptionAble(i)){
      continue;
    }
    Mem->LogAction5(10,"TEAMMATE %.0f (conf %.2f) has %.0f cycles",float(i),Mem->TeammatePositionValid(i),
		    (float)Mem->TeammateInterceptionNumberCycles(i));
  }
//   //END TEMP
  
  int num=GetInterceptionPoints( max_pow,res,data);//главные вычисления
  
  DebugInter(Mem->LogAction3(10,"SmartCanGetThere: number of roots=%d",num));
  if(num==0){
    info.res = BI_Failure; // can't make it to ball 
    return info;
  }else if(num==1)
    {
      res[0].ang_error=ANG_ERROR_MAX;
      return res[0];
    }

  Ray ball_ray(data.vBallPos,data.vBallVel.dir());
  Line ball_line(ball_ray);

  if(CP_goalie){//обработка вратаря
    res[0].ang_error=ANG_ERROR_MAX;
    if(OwnPenaltyArea.shrink(0.5f).IsWithin(res[0].pos)){
      return res[0];
    }
    Vector target_pos=AdjustPtToRectOnLine(res[0].pos, OwnPenaltyArea.shrink(0.5f), ball_line);
    info.numCyc=(int)ceil(fcb((target_pos-data.vBallPos).mod(),data.vBallVel.mod()));
    if((info.numCyc>=res[0].numCyc&&info.numCyc<=res[1].numCyc)||(num==4&&info.numCyc>=res[2].numCyc&&info.numCyc<=res[3].numCyc)){
      GetPredictedBallPosVel(info.numCyc,vBallPos,vBallVel,&info.pos);
      DebugInter(Mem->LogAction3(10,"SmartCanGetThere: new info.numCyc=%.0f",(float)info.numCyc));
      info.res=BI_CanChase;
    }else{
      info.res=BI_Failure;
    }
    return info;
  }
  
  float max_dist=res[1].pos.dist(res[0].pos);
  float ball_dist=data.vBallPos.dist(res[0].pos),ball_dist2=data.vBallPos.dist(res[1].pos);
  float player_dist=data.vPlayerPos.dist(res[0].pos);
  float buffer=1.f*SP_ball_rand*ball_dist+1.f*SP_player_rand*player_dist;

  if(num==2){
    buffer=min(max_dist,buffer);
  }else{//num==4
    if(buffer>max_dist){
      buffer=max(buffer-res[2].pos.dist(res[0].pos),0.0f);
      res[0]=res[2];
      res[1]=res[3];
    }
  }

  Vector project_dot=ball_line.ProjectPoint(data.vPlayerPos);
  float project_dist=project_dot.dist(data.vBallPos);
  Vector target_point,diff;
  float add;

  if(ball_line.dist(data.vPlayerPos)<=GetMyKickableArea()){
    DebugInter(Mem->LogAction3(10,"SmartCanGetThere: i`m on ball line, so go on it (dist %.2f)",
			       ball_line.dist(data.vPlayerPos)));
    buffer=0;//MAYBE TEST
    float priority=SelectPriorityDot(res[0].pos,res[1].pos);
    if(priority<=0.0f){//select res[0]
      DebugInter(Mem->LogAction3(10,"SmartCanGetThere: select res[0](priority %.2f)",priority));
      add=buffer;
      diff=res[0].pos-data.vBallPos;

    }else{//select res[1]
      DebugInter(Mem->LogAction3(10,"SmartCanGetThere: select res[1](priority %.2f)",priority));
      add=-buffer;
      diff=res[1].pos-data.vBallPos;
    
    }
    target_point= data.vBallPos+diff.SetLength(diff.mod()+add);
    info.ang_error=ANG_ERROR_MAX;
    info.numCyc=res[0].numCyc;
    GetPredictedBallPosVel((int)/*ceil*/floor(fcb((target_point-data.vBallPos).mod(),data.vBallVel.mod())),vBallPos,vBallVel,&info.pos);
    DebugInter(Mem->LogAction3(10,"SmartCanGetThere: new info.numCyc=%.0f",(float)info.numCyc));
    info.res=BI_CanChase;
    return info;
  }

  if(project_dist-buffer<ball_dist){//situation number 1
    DebugInter(Mem->LogAction4(10,"SmartCanGetThere: situation number 1; ball_dist=%.2f; project_dist=%.2f",
			       ball_dist,project_dist));
    add=buffer;
    diff=res[0].pos-data.vBallPos;
  }else if(ball_line.InBetween(project_dot,res[0].pos,res[1].pos)){//situation number 2
    DebugInter(Mem->LogAction5(10,"SmartCanGetThere: situation number 2; ball_dist=%.2f; project_dist=%.2f; ball_dist2=%.2f",
			       ball_dist,project_dist,ball_dist2));
    add=min(ball_dist2,max(buffer,project_dot.dist(res[0].pos)));
    diff=res[0].pos-data.vBallPos;
  }else{//situation number 3
    DebugInter(Mem->LogAction5(10,"SmartCanGetThere: situation number 3; ball_dist=%.2f; project_dist=%.2f; ball_dist2=%.2f",
			       ball_dist,project_dist,ball_dist2));
    add=-buffer;
    diff=res[1].pos-data.vBallPos;
  }
                 
  target_point= data.vBallPos+diff.SetLength(min(diff.mod()+add,vBallVel.mod()/(1.0f-SP_ball_decay)-0.1f));
  DebugInter(Mem->LogAction3(10,"SmartCanGetThere: addition=%.2f",add));
  info.ang_error=ANG_ERROR_MAX;
  info.numCyc=res[0].numCyc;
  GetPredictedBallPosVel((int)/*ceil*/floor(fcb((target_point-data.vBallPos).mod(),data.vBallVel.mod())),vBallPos,vBallVel,&info.pos);
  DebugInter(Mem->LogAction3(10,"SmartCanGetThere: new info.numCyc=%.0f",(float)info.numCyc));
  info.res=BI_CanChase;

  //check for go without turn
  Ray my_ray(data.vPlayerPos,data.fPlayerAng);
  Vector my_direct_pos;
  if(my_ray.intersection(ball_ray,&my_direct_pos)){
    float c1= fcb((my_direct_pos-data.vBallPos).mod(),data.vBallVel.mod());
    DebugInter(Mem->LogAction3(10,"SmartCanGetThere: num of cycles without turn=%.2f",c1));
    c1=ceil(c1);//may be floor() ???
    float target_ang  = GetNormalizeAngleDeg((info.pos - MyInterceptPredictedPositionWithTurn(0,1,max_pow,false,data)).dir()
                                             - data.fPlayerAng);
    DebugInter(Mem->LogAction3(10,"SmartCanGetThere: error angle=%.2f",target_ang));
    if(fabs(target_ang)>info.ang_error&&c1>=res[0].numCyc&&c1<=res[1].numCyc){
      info.numCyc=res[0].numCyc;//(int)c1;
      info.pos=my_direct_pos;
      DebugInter(Mem->LogAction2(10,"SmartCanGetThere: go without turn!!!"));
      info.res=BI_CanChase;
      return info;
    }
  }

  return info;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//return positiov if pos2 most priority
//return negative or zero if pos1 more priority
float ActionInfo::SelectPriorityDot(Vector pos1,Vector pos2){
  //first check special cases
  if(pos1.x<=-35.0)
    return -1.0;
  //introduce our coefficients
  float diff1=pos2.x-pos1.x;
  float diff2=fabs(pos1.y)-fabs(pos2.y);
  //know modify this coefficients
  diff1*=2.0f;
  //return result
  return diff1+diff2;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//return: 0 - can not get ball
//        1 - ball is stopping; just go to ball pos
//        2 - go to anyone ball position in interval; if top part of interval contain 777 number of cycles,
//              then ball will stop there
//        4 - go to anyone ball position in two intervals (0-1; 2-3); 
int  ActionInfo::GetInterceptionPoints( float max_pow,PlayerInterceptInfo* res,const InterceptionInput& data){
  int numRoots=0;
  float prev_root=0.0,mod=data.vBallVel.mod();

  if(mod<0.2f){
    DebugInter(Mem->LogAction3(10,"GetInterceptionPoints: ball is stopping mod=%.4f",mod));
    res[0].res=BI_CanChase;
    Vector vel=data.vBallVel;
    res[0].pos=data.vBallPos+vel * (SumInfGeomSeries(mod,Mem->SP_ball_decay));
    res[0].numCyc=PredictedCyclesToPoint(res[0].pos,max_pow==0?Mem->SP_max_power:max_pow);
    return 1;
  }

  float first=1.0f;        
  float ps=(data.vBallVel.x*(data.vPlayerPos.x-data.vBallPos.x)+data.vBallVel.y*(data.vPlayerPos.y-data.vBallPos.y))/mod;
  float max_dist=mod/(1-SP_ball_decay)-0.1;
  float orig_ps=ps;
  if(first>=max_dist)
    first=max_dist/2.0f;
  if(ps>=max_dist||ps<=first){
    DebugInter(Mem->LogAction5(10,"GetInterceptionPoints: original ps=%.4f; max_dist=%.4f; first=%.4f",orig_ps,max_dist,first));
    ps=(first+max_dist)/2.0;
  }
  float start[]={first,ps,max_dist};
  int calcNumCycles[3];
  DebugInter(Mem->LogAction3(10,"GetInterceptionPoints: ps=%.4f",ps));
  for(int index=0;index<3;index++){  
    float d=NewtonMethod(start[index],prev_root,data,(prev_root==0&&orig_ps>1.0f));
    if(d<0)
      break;//find all possible roots (if any)
    if(fabs(d-prev_root)<0.1)
      continue;//not use this root
    if((fcb(d-0.1,mod)-fcp(d-0.1,data))*(fcb(d+0.1,mod)-fcp(d+0.1,data))>0){
      if(numRoots==1){//second touch on top area
	DebugInter(Mem->LogAction2(10,"GetInterceptionPoints: second touch on top area"));
	break;//we already find all (1) roots
      }else{//first touch on bottom area
	DebugInter(Mem->LogAction2(10,"GetInterceptionPoints: first touch on bottom area"));
	index++;//maximum two roots
      }
    }
    prev_root=d;
    numRoots++;
    if (numRoots!=2&&!IsPointInBounds(data.vBallPos+Polar2Vector(d,data.vBallVel.dir()),-1)) {
      DebugInter(Mem->LogAction2(10,"GetInterceptionPoints: the ball will go out of bounds before we can get it"));
      numRoots--;
      break;
    }
    calcNumCycles[numRoots-1]=int((numRoots==2)?floor(fcb(d,mod)):ceil(fcb(d,mod))); //if numRoots%2==0 then floor() else ceil()
    DebugInter(Mem->LogAction4(10," GetInterceptionPoints: calc num of cycles = %.4f(%.2f)",
			       fcb(d,mod),(float)calcNumCycles[numRoots-1]);)
      }

  
  return CorrectAllCycles(numRoots,calcNumCycles,max_pow,res,data);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ActionInfo::CorrectAllCycles(int numRoots,int* calcNumCycles,float max_pow,
				 PlayerInterceptInfo* res,const InterceptionInput& data)
{
  PlayerInterceptInfo save;
  //correction of values
  DebugInter(Mem->LogAction3(10," Number of roots=%.0f",float(numRoots)));
  DebugInter(Mem->LogAction2(10,"~~~~~~~~~~ NEW VERSION~~~~~~~~~~~"));
  if(numRoots>0){
    res[0]=CorrectPos(calcNumCycles[0],1,numRoots>1?calcNumCycles[1]:0,max_pow,data);
    AngleDeg ang1=(data.vBallPos-Mem->MyPos()).dir(),ang2=(res[0].pos-Mem->MyPos()).dir();
    if(!Mem->CP_goalie&&calcNumCycles[0]<=10&&(Mem->FastestOpponentToBall()==Unum_Unknown||
			      Mem->OpponentInterceptionNumberCycles(Mem->FastestOpponentToBall())>calcNumCycles[0])&&
       (!IsSuccessRes(res[0].res)||(res[0].numCyc-calcNumCycles[0]>=1
				    &&(fabs(GetNormalizeAngleDeg(ang1-data.fPlayerAng))>90.0f||
				       fabs(GetNormalizeAngleDeg(ang2-data.fPlayerAng))>90.0f)))){
      DebugInter(PrintInfoStruct("TRY GO BACKWARD BECOUSE RESULT OF FIRST ROOT:",res[0]));
      save=res[0];
      max_pow*=-1.0f;
      res[0]=CorrectPos(calcNumCycles[0],1,numRoots>1?calcNumCycles[1]:0,max_pow,data);
      if(!IsSuccessRes(res[0].res)||res[0].numCyc>5||res[0].numCyc-save.numCyc>=-1){
	DebugInter(PrintInfoStruct("WRONG BACKWARD CHECK OF FIRST ROOT:",res[0]));
	res[0]=save;
	max_pow*=-1.0f;
      }else
	DebugInter(Mem->LogAction2(10,"SELECT BACKWARD MOVE!!!"));
    }
    if(numRoots>2){
      if(IsSuccessRes(res[0].res)){
	DebugInter(PrintInfoStruct("RESULT OF FIRST ROOT:",res[0]));
	res[1]=CorrectPos(calcNumCycles[1],-1,res[0].numCyc,max_pow,data);
	DebugInter(PrintInfoStruct("RESULT OF SECOND ROOT:",res[1]));
      }
      if(!IsSuccessRes(res[0].res)||!IsSuccessRes(res[1].res))
	numRoots-=2;
      res[numRoots-1]=CorrectPos(calcNumCycles[2],1,0,max_pow,data);
      if(IsSuccessRes(res[numRoots-1].res)){
	DebugInter(PrintInfoStruct("RESULT OF 3 ROOT:",res[numRoots-1]));
	numRoots++;
	AddFinalDot(data,&numRoots,res);
	if(numRoots>1){
	  DebugInter(PrintInfoStruct("RESULT OF LAST DOT:",res[numRoots-1]));
	}
      }
    }else{//add last dot
      if(IsSuccessRes(res[0].res)){
	DebugInter(PrintInfoStruct("RESULT OF FIRST ROOT:",res[0]));
	numRoots++;
	AddFinalDot(data,&numRoots,res);
	if(numRoots>1){
	  DebugInter(PrintInfoStruct("RESULT OF LAST DOT:",res[1]););
	}
      }else
	numRoots=0;
    }
  }
  return numRoots;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PlayerInterceptInfo ActionInfo::CorrectPos(int startStep,int dir,int stopStep,float max_pow,InterceptionInput data){
  PlayerInterceptInfo info;
  InterceptionInput save=data;
  float at_point_buffer=Mem->CP_goalie?Mem->SP_catch_area_l*0.95f:GetMyKickableArea()*0.95f;
  Vector vOldBallPos, vOldBallVel;
  float turn_angle;
  int cyc_inc =  CP_my_intercept_step ;
  
  int max_lookahead=stopStep==0?CP_max_int_lookahead:abs(stopStep-startStep);
  
  int max_num_cyc = (max_lookahead + cyc_inc - 1);
  max_num_cyc -= (max_num_cyc % cyc_inc);

  info.dash_pow_to_use = max_pow;
  NormalizeAngleDeg(&data.fPlayerAng);
  
  GetPredictedBallPosVel(startStep,data.vBallPos,data.vBallVel,&data.vBallPos,&data.vBallVel);

  Vector vPredPlayer;
  if(max_pow>0.0f&&Mem->IsBetterStopEndTurn((data.vBallPos-data.vPlayerPos).dir()))
    vPredPlayer+=data.vPlayerVel;
  else
    vPredPlayer=data.vPlayerPos +data.vPlayerVel * (SumInfGeomSeries(data.vPlayerVel.mod(),GetMyPlayerDecay()));

  int cyc=startStep;  
  do{
    if (dir!=-1&&!IsPointInBounds(data.vBallPos,-2)) {
      DebugInter(Mem->LogAction2(10,"CorrectPos: the ball will go out of bounds before we can get it");)
	break;
    }
    // decide if we need to turn to ball
    float ball_ang = (data.vBallPos - vPredPlayer).dir();
    Vector vEndSpot;
    turn_angle = ball_ang - data.fPlayerAng;
    if(max_pow<0)
      turn_angle=GetNormalizeAngleDeg(turn_angle+180);
    if (fabs(turn_angle) < CP_max_go_to_point_angle_err)
      turn_angle = 0.0;
    vEndSpot = MyInterceptPredictedPositionWithTurn(turn_angle, cyc, max_pow,(turn_angle != 0.0),save);
 
    float dist_to_ball_after = (data.vBallPos - vEndSpot).mod();
    // if we can make it there
    if (dist_to_ball_after <= at_point_buffer ||(vEndSpot - vPredPlayer).mod() > (data.vBallPos - vPredPlayer).mod() + GetMyKickableArea()) {
      // we can get to the ball!
      // OR we travelled far enough, but somehow missed the ball,
      //  return sucess
      info.numCyc = cyc;
      if (cyc_inc > 1 && cyc != 0) {
	// we want the best answer- go back and go up by ones
	cyc -= cyc_inc*dir;
	data.vBallPos = vOldBallPos;
	data.vBallVel = vOldBallVel;
	cyc_inc = 1;
	max_num_cyc = max_lookahead; // don;t need to go above this anymore
      } else {
	if (info.numCyc > startStep+max_num_cyc*dir) {
	  info.res = BI_Failure;
	} else {
	  info.res = BI_CanChase;
	  info.pos = data.vBallPos;
	}
	return info;
      }
    }
    
    // update ball position estimate
    vOldBallPos = data.vBallPos;
    vOldBallVel = data.vBallVel;
    if(dir>0){
      if(fabs(data.vBallVel.mod())<0.1f){
	if(data.vBallVel.mod()!=0.0f)
	  DebugInter(Mem->LogAction2(10,"CorrectPos: the ball has small vel, so set it to zero"););
	data.vBallVel=0.0f;
      }
      for (int i=0; i < cyc_inc; i++) {
        data.vBallPos += data.vBallVel;
        data.vBallVel *= SP_ball_decay;
      }
    }else{ //dir<=0
      for (int i=0; i < cyc_inc; i++) {
        data.vBallVel /= SP_ball_decay;
        data.vBallPos -= data.vBallVel;
      }
    } 
    if(dir>0){
      cyc+=cyc_inc;
      if(cyc>startStep+max_num_cyc)
        break;
    }else{
      cyc-=cyc_inc;
      if(cyc<startStep-max_num_cyc)
        break;
    }          
  }while(1); // cycle loop
  info.res = BI_Failure; // can't make it to ball before max_lookahead
  return info;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//add final dot*****************
void ActionInfo::AddFinalDot(const InterceptionInput& data,int* numRoots,PlayerInterceptInfo* res){
  //numRoots may have 2 or 4
  const int BUFFER=3;//buffer of cycles befor opponent take ball
  float max_dist=data.vBallVel.mod()/(1-SP_ball_decay)-0.1;
  //get our opponent
  Unum opp=FastestOpponentToBall();
  bool have_opp=(opp!=Unum_Unknown)&&/*OpponentPositionValid(opp)>0.8f&&*/OpponentInterceptionAble(opp);
  int opp_inter_cyc=have_opp?OpponentInterceptionNumberCycles(opp):0;
  //correct
  if(have_opp){
    //     if(res[0].numCyc>opp_inter_cyc){
    //       DebugInter(Mem->LogAction5(10,"AddFinalDot: opp %.0f (conf %.2f) has %.0f cycles; return his pos",
    //         float(opp),OpponentPositionValid(opp),float(opp_inter_cyc));)
    //       *numRoots=1;
    //       res[0].pos=Mem->OpponentInterceptionPoint(opp);
    //       //res[0].numCyc=Mem->PredictedCyclesToPoint(res[0].pos);
    //       return;
    //     }
    if(res[0].numCyc>opp_inter_cyc-BUFFER&&!Mem->TheirPenaltyArea.IsWithin(data.vBallPos)){
      DebugInter(Mem->LogAction5(10,"AddFinalDot: opp %.0f (conf %.2f) has %.0f cycles; return 1 answer",
				 float(opp),OpponentPositionValid(opp),float(opp_inter_cyc)));
      *numRoots=1;
      return;
    }
    if(*numRoots==4&&res[2].numCyc>=opp_inter_cyc){
      int new_cyc=max(res[0].numCyc,opp_inter_cyc-BUFFER);
      if(res[1].numCyc>new_cyc){
        GetPredictedBallPosVel(new_cyc,data.vBallPos,data.vBallVel,&res[1].pos);
        res[1].numCyc=new_cyc;
        DebugInter(Mem->LogAction5(10,"AddFinalDot: opp %.0f (conf %.2f) has %.0f cycles; modify last dot",
				   float(opp),OpponentPositionValid(opp),float(opp_inter_cyc)));
      }
      DebugInter(Mem->LogAction5(10,"AddFinalDot: opp %.0f (conf %.2f) has %.0f cycles; return 2 answers",
				 float(opp),OpponentPositionValid(opp),float(opp_inter_cyc)));
      *numRoots=2;
      return;
    }
  }
  Vector final_pos=data.vBallPos+Polar2Vector(max_dist,data.vBallVel.dir());
  if(!IsPointInBounds(final_pos,-1)){
    DebugInter(Mem->LogAction4(10,"AddFinalDot: ball will go outside of field at final_pos=(%.2f,%.2f)",
			       final_pos.x,final_pos.y));
    Vector out=Mem->FieldRectangle.RayIntersection(Ray(data.vBallPos,data.vBallVel));
    res[*numRoots-1].numCyc=(int)floor(fcb((out-data.vBallPos).mod(),data.vBallVel.mod()))-3;
    GetPredictedBallPosVel(res[*numRoots-1].numCyc,data.vBallPos,data.vBallVel,&res[*numRoots-1].pos);
  }else{//ball will stop
    DebugInter(Mem->LogAction3(10,"AddFinalDot: ball will stop after trevalling %.2f m",max_dist);)
      res[*numRoots-1].pos=data.vBallPos+Polar2Vector(max_dist-1.0,data.vBallVel.dir());
    res[*numRoots-1].numCyc=(int)floor(fcb((res[*numRoots-1].pos-data.vBallPos).mod(),data.vBallVel.mod()));
  }
  res[*numRoots-1].res=BI_CanChase;

  if(have_opp){
    int new_cyc=max(res[*numRoots-2].numCyc,opp_inter_cyc-BUFFER);
    if(res[*numRoots-1].numCyc>new_cyc){
      GetPredictedBallPosVel(new_cyc,data.vBallPos,data.vBallVel,&res[*numRoots-1].pos);
      res[*numRoots-1].numCyc=new_cyc;
      DebugInter(Mem->LogAction5(10,"AddFinalDot: opp %.0f (conf %.2f) has %.0f cycles; modify last dot",
				 float(opp),OpponentPositionValid(opp),float(opp_inter_cyc)));
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Vector ActionInfo::MyInterceptPredictedPositionWithTurn(float turn_ang,
							int steps, float dash_power,
							bool with_turn,const InterceptionInput& data,
							int idle_cycles)
{
  float curr_turn_ang = GetNormalizeAngleDeg(turn_ang);
  float corrected_dash_power = dash_power;
  float effective_power;
  float predicted_stamina = data.stamina;
  float predicted_effort = data.effort;
  float predicted_recovery = data.recovery;
  float myang = data.fPlayerAng;
  Vector position = data.vPlayerPos;
  Vector velocity=data.vPlayerVel;
  for (int i=0; i<steps; i++){
    corrected_dash_power = CorrectDashPowerForStamina(dash_power,predicted_stamina);
    if (i < idle_cycles) {
      /* do nothing, we're idling! */
      effective_power = 0;
    } else if (with_turn &&
	       (i==0 || curr_turn_ang != 0.0)) {
      float this_turn = MinMax(-EffectiveTurn(SP_max_moment, velocity.mod()),
			       curr_turn_ang,
			       EffectiveTurn(SP_max_moment, velocity.mod()));
      if(Mem->IsBetterStopEndTurn(GetNormalizeAngleDeg(turn_ang+myang+dash_power<0?180.0f:0.0f),velocity.mod(),myang)){
        position+=velocity;
        velocity=Vector(0.0f,0.0f);
      }else{
        myang += this_turn;
        curr_turn_ang -= this_turn;
      }
      effective_power = 0;
    } else if (fabs(corrected_dash_power) > predicted_stamina)
      effective_power = Sign(corrected_dash_power) * predicted_stamina ;
    else
      effective_power = corrected_dash_power;

    effective_power *= predicted_effort;
    effective_power *= GetMyDashPowerRate();
    velocity += Polar2Vector( effective_power, myang );

    if ( velocity.mod() > GetMyPlayerSpeedMax() )
      velocity *= ( GetMyPlayerSpeedMax()/velocity.mod() );

    position += velocity;
    velocity *= GetMyPlayerDecay();

    UpdatePredictedStaminaWithDash(&predicted_stamina, &predicted_effort,
				   &predicted_recovery, corrected_dash_power);

  }
  return position;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ActionInfo::PrintInfoStruct(const char* prefix,const PlayerInterceptInfo& info){
  Mem->LogAction3(10,"%s",(char*)prefix);
  Mem->LogAction3(10,"result: %.0f",float(IsSuccessRes(info.res)));
  Mem->LogAction5(10,"pos: (%.2f,%.2f); cycles=%.0f",info.pos.x,info.pos.y,float(info.numCyc));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float   ActionInfo::NewtonMethod(float start,float prev_root,const InterceptionInput& data,bool use_orig_newton){
  float x=start,add,mod=data.vBallVel.mod(),xmax=mod/(1-SP_ball_decay);
  float znamenatel;
  static float step=20;
  do{
    if(use_orig_newton)//call for first root
      znamenatel=fcpDiff(x,data)-fcbDiff(x,mod);
    else
      znamenatel=fcpDiffEx(x,prev_root,data)-fcbDiffEx(x,prev_root,mod);
      
    if(fabs(znamenatel)<0.0001){
      DebugInter(Mem->LogAction3(10,"NewtonMethod: znamenatel=%f",znamenatel);)
	x=-1.0f;
      break;
    }
    if(use_orig_newton)
      add=(fcp(x,data)-fcb(x,mod))/znamenatel;
    else
      add=(fcpEx(x,prev_root,data)-fcbEx(x,prev_root,mod))/znamenatel;
    //DebugInter(Mem->LogAction5(50,"NewtonMethod: x=%.4f; add=%.4f; (step=%.0f)",x,add,float(step)));
    x=x-add;
    if(x<0||x>xmax){
      x=-1.0;
      break;
    }
  }while(--step&&fabs(add)>=0.2);
  if(!step){
    my_error("Newton method error");
    x=-1.0;
  }
  step=20;
  return x; 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ActionInfo::GetPredictedBallPosVel(int steps,Vector origPos,Vector vel,Vector* newPos,Vector* newVel){
  *newPos=origPos+vel*GetBallMoveCoeff(steps);
  if(newVel!=0)
    *newVel=vel*pow(SP_ball_decay,steps);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//optimization only
float ActionInfo::GetBallMoveCoeff(int steps){
  static bool first=true;
  static float coeff[100];
  if(first){
    first=false;
    for(int i=0;i<100;i++)
      coeff[i]=(1.0f-pow(SP_ball_decay,i))/(1.0f-SP_ball_decay);
  }
  return steps<100?coeff[steps]:(1.0f-pow(SP_ball_decay,steps))/(1.0f-SP_ball_decay);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float ActionInfo::fcp(float x,const InterceptionInput& data){
  float mod=data.vBallVel.mod();
  return sqrt(Sqr(x*data.vBallVel.x/mod-data.vPlayerPos.x+data. vBallPos.x)+
	      Sqr(x*data.vBallVel.y/mod-data.vPlayerPos.y+data.vBallPos.y))/GetMyPlayerSpeedMax();
}
float ActionInfo::fcpDiff(float x,const InterceptionInput& data){
  float v0x=data.vBallVel.x,v0y=data.vBallVel.y,v0=data.vBallVel.mod();
  float x_mult=x*v0x/v0-data.vPlayerPos.x+data.vBallPos.x;
  float y_mult=x*v0y/v0-data.vPlayerPos.y+data.vBallPos.y;
  float chislitel=x_mult*v0x+y_mult*v0y;
  float znamenatel=v0*sqrt(Sqr(x_mult)+Sqr(y_mult))*GetMyPlayerSpeedMax();
  return chislitel/znamenatel;
}
float ActionInfo::fcb(float x,float velMod){
  static float conf1=1.0f/log(SP_ball_decay);
  static float conf2=1.0f-SP_ball_decay;
  return log(1.0f-x*conf2/velMod)*conf1;
}
float ActionInfo::fcbDiff(float x,float velMod){
  static float conf1=(SP_ball_decay-1)/log(SP_ball_decay);
  static float conf2=1.0f-SP_ball_decay;
  return conf1/(velMod*(1-x*conf2/velMod));
}
//~~~~~~~~~~~~~~
float ActionInfo::fcpEx(float x,float prev_root,const InterceptionInput& data){
  return fcp(x,data)/(x-prev_root);
}
float ActionInfo::fcpDiffEx(float x,float prev_root,const InterceptionInput& data){
  float v0x=data.vBallVel.x,v0y=data.vBallVel.y,v0=data.vBallVel.mod();
  float x_mult=x*v0x/v0-data.vPlayerPos.x+data.vBallPos.x;
  float y_mult=x*v0y/v0-data.vPlayerPos.y+data.vBallPos.y;
  float conf=sqrt(Sqr(x_mult)+Sqr(y_mult));
  float chislitel=x_mult*v0x+y_mult*v0y;
  float x_prev= x-prev_root;
  float znamenatel=v0*conf*GetMyPlayerSpeedMax()*x_prev;
  return chislitel/znamenatel-conf/(GetMyPlayerSpeedMax()*Sqr(x_prev));
}
float ActionInfo::fcbEx(float x,float prev_root,float velMod){
  return fcb(x,velMod)/(x-prev_root);
}
float ActionInfo::fcbDiffEx(float x,float prev_root,float velMod){
  static float conf=log(SP_ball_decay);
  static float conf2=1.0f-SP_ball_decay;

  float invX=(1-x*conf2/velMod);
  float x_prev=x-prev_root;
  return -conf2/(conf*velMod*invX*x_prev)-log(invX)/(conf*Sqr(x_prev));
}
//////////////////////////////////////////////////////////////////////////////////////////
float ActionInfo::fcp_1DegStep(float x,AngleDeg ang,const InterceptionInput& data)
{
  return sqrt(Sqr(x*Cos_1DegStep(ang)-data.vPlayerPos.x+data. vBallPos.x)+
	      Sqr(x*Sin_1DegStep(ang)-data.vPlayerPos.y+data.vBallPos.y))/SP_player_speed_max;  
}
//////////////////////////////////////////////////////////////////////
float ActionInfo::GetMaxSpeedWithIntercept(float p,AngleDeg ang,const InterceptionInput& data)
{
  static const float conf=1-SP_ball_decay;
  static const float conf2=log(SP_ball_decay);

  return p*conf/(1-exp(fcp_1DegStep(p,ang,data)*conf2));
}
//////////////////////////////////////////////////////////////////////
int ActionInfo::GetPlayerInterceptionPoint(char side,Unum num,PlayerInterceptInfo* res,const InterceptionInput& data)
{
  float mod=data.vBallVel.mod();
  //DebugInter(Mem->LogAction4(20,"GetPlayerInterceptionPoint: calc for player %.0f %.0f",float(side),float(num)));
  if(mod<0.05f){
    //DebugInter(Mem->LogAction3(20,"GetPlayerInterceptionPoint: ball is stopping mod=%.4f",mod));
    res[0].res=BI_CanChase;
    res[0].pos=data.vBallPos;
    res[0].numCyc=PlayerPredictedCyclesToPoint(side,num,data.vBallPos);
    return 1;
  }

  // Line l(Ray(data.vBallPos,data.vBallVel));
  float first=1.0f;//(l.ProjectPoint(data.vPlayerPos)-data.vBallPos).mod();        
  float orig_ps=(data.vBallVel.x*(data.vPlayerPos.x-data.vBallPos.x)+data.vBallVel.y*(data.vPlayerPos.y-data.vBallPos.y))/mod;
  float max_dist=mod/(1-SP_ball_decay)-0.1;
  if(first>=max_dist)
    first=max_dist/2.0f;
  //DebugInter(Mem->LogAction3(20,"GetPlayerInterceptionPoint: ps=%.4f",orig_ps));
  float d=NewtonMethod(first,0.0f,data,orig_ps>1.0f);
  if(d<0){
    res->res=BI_Failure;
    return 0;
  }
  if (!IsPointInBounds(data.vBallPos+Polar2Vector(d,data.vBallVel.dir()),-1)) {
    //DebugInter(Mem->LogAction2(20,"GetPlayerInterceptionPoint: the ball will go out of bounds before player can get it"));
    res->res=BI_Failure;
    return 0;
  }
  res->numCyc=(int)ceil(fcb(d,mod));
  res->res=BI_CanChase;
  GetPredictedBallPosVel(res->numCyc,data.vBallPos,data.vBallVel,&res->pos);
  res->dash_pow_to_use=SP_max_power;
  //DebugInter(Mem->LogAction3(20," GetPlayerInterceptionPoint: calc num of cycles = %.0f",
  //		     float(res->numCyc)));
  return 1;
}
/****************************************************************************************/
void ActionInfo::BallIntercept_active(float max_pow_to_use, int max_lookahead,
				      char PlayerSide, Unum PlayerNum,
				      PlayerInterceptInfo* pInfo)
{
  Vector PlayerPos;
  Vector PlayerVel;
  float PlayerAng;
  int AngValid = FALSE;
  Vector BallVel;

  pInfo->res = BI_None;

  if (!BallPositionValid()) {
    my_error("BallIntercept_active: Can't get to ball if I don;t know where it is");
    pInfo->res = BI_Invalid;
    return;
  }
  
  if (!PlayerPositionValid(PlayerSide, PlayerNum)) {
    my_error("BallIntercept_active: Can't give an answer if I don't know where player is");
    pInfo->res = BI_Invalid;
    return;
  }
  PlayerPos = PlayerAbsolutePosition(PlayerSide, PlayerNum);
  //DebugInt(cout << "PlayerPos: " << PlayerPos << endl);
  
  if (PlayerVelocityValid(PlayerSide, PlayerNum)) {
    PlayerVel = PlayerAbsoluteVelocity(PlayerSide, PlayerNum);
  } else {    
    PlayerVel = Vector(0,0);
  }
  
  if (PlayerBodyAngleValid(PlayerSide, PlayerNum)) {
    AngValid = TRUE;
    PlayerAng = PlayerAbsoluteBodyAngle(PlayerSide, PlayerNum);
  } else
    PlayerAng = 0;
    
  if ((PlayerPos - BallAbsolutePosition()).mod() <
      GetPlayerKickableArea(PlayerSide,PlayerNum)) {
    pInfo->res = BI_ReadyToKick;
    pInfo->numCyc = 0;
    pInfo->pos = PlayerPos;
    return;
  }

  if (BallVelocityValid())
    BallVel = BallAbsoluteVelocity();
  else
    BallVel = Vector(0,0);
  
  DebugInt(printf("At BallIntercept_active  max_pow: %f, max_look: %d\n",
		  max_pow_to_use, max_lookahead));

  if (PlayerSide == MySide && PlayerNum == MyNumber) {
    *pInfo = CloseBallInterception(max_pow_to_use, max_lookahead,
				   BallAbsolutePosition(), BallVel);
    Unum opp=Mem->OpponentWithBall();
    if(opp!=Unum_Unknown&&Mem->OpponentAbsolutePosition(opp)==Mem->BallPositionValid())//MUST BE TESTED
      BallVel=Vector(0,0);
    
    if (pInfo->res == BI_None)
      *pInfo=SmartCanGetThere(max_pow_to_use,
			      BallAbsolutePosition(), BallVel,
			      PlayerPos, PlayerVel, PlayerAng) ;
  }else  
    *pInfo = ActiveCanGetThere(max_pow_to_use, max_lookahead,
			       BallAbsolutePosition(), BallVel,
			       PlayerSide, PlayerNum,
			       PlayerPos, PlayerVel, PlayerAng, AngValid,
			       (PlayerSide == MySide && PlayerNum == MyNumber));
} 

/*****************************************************************************************/

PlayerInterceptInfo* ActionInfo::GetPlayerIntInfo(char side, Unum num)
{
  if (side == MySide)
    return TeamIntInfo[num];
  else if (side == TheirSide)
    return OppIntInfo[num];
  else
    my_error("bad side passed to GetPlayerIntInfo");
  return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////
PlayerInterceptInfo* ActionInfo::VerifyIntInfo(char side, Unum num, float dash_pow)
{
  PlayerInterceptInfo* pInfo = GetPlayerIntInfo(side, num);
  if (pInfo == NULL) {
    my_error("Bad side or number passed to VerifyIntInfo");
    return NULL;
  }

  int lookahead;
  switch (InterceptLookahead) {
  case LA_Default: lookahead = CP_max_int_lookahead; break;
  case LA_BestSoFar:
    lookahead =
      (IntMinCycTime == CurrentTime) ? (IntMinCyc) : CP_max_int_lookahead;
    break;
  default: lookahead = InterceptLookahead; break;
    break;
  }
  if ( pInfo->time != CurrentTime || fabs((pInfo->dash_pow-dash_pow))>FLOAT_EPS ||
       (pInfo->lookahead < lookahead && !IsSuccessRes(pInfo->res)) ) {
    /* set the info struct */
    DebugInt(printf("%d %d Data not current. Calling interception code\n", MyNumber, int(num)));
    /*
      if (pInfo->time == CurrentTime && (pInfo->dash_pow-dash_pow)<=FLOAT_EPS &&
      (side != MySide || num != MyNumber))
      my_error("Recomputing %c %d because lookahead got bigger; old: %d\tnew: %d",
      side,int(num),pInfo->lookahead, lookahead);
    */    
    /* let's do a real quick estimate to see if the player can make it there
       if player dist to ball > max ball dist will travel + max_dist we'll
       travel, then there's no way to get there */
    if (!PlayerPositionValid(side, num)) {      
      my_error("VerifyIntInfo: Can't give an answer if I don't know where player is");
      pInfo->res = BI_Invalid;
      return pInfo;
    }
    DebugInt(printf("Lookahead: %d\n", lookahead));
    float ball_travel = SumGeomSeries((BallVelocityValid() ? BallSpeed() : 0),
				      SP_ball_decay, lookahead);
    float player_travel = GetPlayerPlayerSpeedMax(side,num)* lookahead;
    float play_ball_dist = (PlayerAbsolutePosition(side, num) -
			    BallAbsolutePosition()).mod() ;
    if (play_ball_dist > player_travel + ball_travel) {
      pInfo->time = CurrentTime;
      pInfo->dash_pow = dash_pow;
      pInfo->dash_pow_to_use = dash_pow;
      pInfo->lookahead = lookahead;
      pInfo->res = BI_Failure;
      DebugInt(printf("Interception: %d, %d Took shortcut to decide failure\n", MyNumber, num));
    } else {
      DebugInt(printf("Interception: %d, %d About to do actual calculation\n", MyNumber, num));
      BallIntercept_active( dash_pow, lookahead, side, num, pInfo);
      if (IsSuccessRes(pInfo->res))
	SetIntMinCyc(pInfo->numCyc);
      pInfo->time = CurrentTime;
      pInfo->dash_pow = dash_pow;
      pInfo->lookahead = lookahead;
    }    
  }
  else if ( IsSuccessRes(pInfo->res) )
    SetIntMinCyc(pInfo->numCyc);

  return pInfo;
}

/*****************************************************************************************/


InterceptRes ActionInfo::PlayerInterceptionResult(char side, Unum num,
						  float dash_pow)
{
  return (VerifyIntInfo(side, num, dash_pow))->res;
}

/*****************************************************************************************/

Bool ActionInfo::PlayerInterceptionAble(char side, Unum num, float dash_pow)
{
  return IsSuccessRes((VerifyIntInfo(side, num, dash_pow))->res) ? TRUE : FALSE;
}

/*****************************************************************************************/

int ActionInfo::PlayerInterceptionNumberCycles(char side, Unum num,
					       float dash_pow)
{
  PlayerInterceptInfo* pInfo = VerifyIntInfo(side, num, dash_pow);
  if (!IsSuccessRes(pInfo->res))
    my_error("Trying to get number of cycles on invalid result: %c%d %d",
	     side, num, pInfo->res);
  return pInfo->numCyc;
}

/*****************************************************************************************/

Vector ActionInfo::PlayerInterceptionPoint(char side, Unum num,
					   float dash_pow)
{
  PlayerInterceptInfo* pInfo = VerifyIntInfo(side, num, dash_pow);
  if (!IsSuccessRes(pInfo->res))
    my_error("Trying to get interception point on invalid result: %c%d %d", 
	     side, num, pInfo->res);
  return pInfo->pos;  
}

/*****************************************************************************************/

float ActionInfo::PlayerInterceptionDashPower(char side, Unum num, float dash_pow)
{
  PlayerInterceptInfo* pInfo = VerifyIntInfo(side, num, dash_pow);
  if (!IsSuccessRes(pInfo->res))
    my_error("Trying to get interception dash power on invalid result: %c%d %d", 
	     side, num, pInfo->res);
  return pInfo->dash_pow_to_use;  
}


/*****************************************************************************************/

float ActionInfo::PlayerInterceptionAngleError(char side, Unum num, float dash_pow)
{
  PlayerInterceptInfo* pInfo = VerifyIntInfo(side, num, dash_pow);
  if (!IsSuccessRes(pInfo->res))
    my_error("Trying to get interception angle error on invalid result: %c%d %d",
	     side, num, pInfo->res);
  return pInfo->ang_error;
}


/*****************************************************************************************/
int ActionInfo::GetInterceptionMinCyc()
{
  if (IntMinCycTime != CurrentTime)
    return -1;
  else
    return IntMinCyc;
}

/*****************************************************************************************/

void ActionInfo::SetIntMinCyc(int newval)
{
  if (IntMinCycTime != CurrentTime) {
    IntMinCycTime = CurrentTime;
    IntMinCyc = newval;
  } else if (IntMinCyc > newval)
    IntMinCyc = newval;
}

/*****************************************************************************************/

void ActionInfo::SetInterceptionLookahead(int newval)
{
  if (newval > 0 || newval == LA_Default || newval == LA_BestSoFar) {
    if (IntMinCycTime == CurrentTime) 
      DebugInt(cout << "Changing lookahead mid way through computations. Could be bad" <<endl);
    InterceptLookahead = newval;
  } else {
    my_error("Trying to set InterceptLookahead to an invlaid value");
  }
  
}




/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/
/* Passive interception stuff */

int ActionInfo::GetClosestPointToBallPath(Vector* pPt, float* pNumCycles,
					  Vector PlayerPos, Vector BallPos,
					  Vector BallVel)
{
  if (fabs(BallVel.x) < FLOAT_EPS && fabs(BallVel.y) < FLOAT_EPS) {
    *pPt = BallPos;
    *pNumCycles = 0;
    return 1;
  }

  Ray rBallPath(BallPos, BallVel);;
  
  *pPt = rBallPath.GetClosestPoint(PlayerPos);
  
  /* adjust point for sidelines */
  Rectangle field(Vector(0,0), Vector(SP_pitch_length, SP_pitch_width));
  *pPt = AdjustPtToRectOnLine(*pPt, field, LineFromRay(rBallPath));

  /* Now let's reason about how far off we will be if we favor not turning */
  Vector no_turn_pt;
  if (rBallPath.intersection(Ray(MyPos(), MyBodyAng()), &no_turn_pt)) {
    if (no_turn_pt.dist(*pPt) < CP_no_turn_max_dist_diff) {
      LogAction6(110, "BPI: using no turn interception, old: (%.1f, %.1f) new: (%.1f, %.1f)",
		 pPt->x, pPt->y, no_turn_pt.x, no_turn_pt.y);
      *pPt = no_turn_pt;
    }
  }
  
  /* compute the number of cycles to get here */
  *pNumCycles = 0;

  /* now get the number of cycles */
  Vector traj = *pPt - BallPos;
  DebugInt(cout << "Pt: " << *pPt << "\tBallVel: " << BallVel
	   << "\tBallPos: " << BallPos << "\ttraj: " << traj << endl);
  /* first decide if the ball is actually coming towards us */
  if (signf(traj.x) != signf(BallVel.x) ||
      signf(traj.y) != signf(BallVel.y)) {
    DebugInt(printf("  GCPTBP: Ball is goign wrong way for closest intercept!\n"));
    return 0;
  }

  float trajDist = traj.mod();
  float velMod = BallVel.mod();
  float temp = trajDist / velMod * (SP_ball_decay - 1) + 1;
  if (temp < 0.0) {
    /* ball will never make it to closest point */
    /* SMURF - shoudl adjust for actual closest!!!! */
    DebugInt(printf("GCPTBP: Ball will never make it to closest point, adjusting\n"));
    *pPt = BallPos + traj * SumInfGeomSeries(velMod, SP_ball_decay) / traj.mod();
    *pNumCycles = SP_half_time; //just a big number
    return 1; 
  } else
    *pNumCycles = log(temp) / log(SP_ball_decay);

  return 1;
}

/*****************************************************************************************/

void ActionInfo::VerifyBPIInfo()
{
  if (BPItime == CurrentTime)
    return;

  BPItime = CurrentTime;
  
  Vector BallVel;

  if (!MyConf()) {
    my_error("Can't intercept if I don't know where I am");
    BPIvalid = FALSE;
    return;
  }

  if (!BallPositionValid()) {
    my_error("Can't get to ball path if I don't know where it is");
    BPIvalid = FALSE;
    return;
  }
  
  if (BallKickable()) {
    BPIvalid = TRUE;
    BPIable = TRUE;
    BPIdist = 0;
    BPIpoint = MyPos();
    BPIballcyc = 0;
    return;
  }

  if (BallVelocityValid())
    BallVel = BallAbsoluteVelocity();
  else {
    BPIvalid = TRUE;
    BPIable = TRUE;
    BPIdist = BallDistance();
    BPIpoint = BallAbsolutePosition();
    BPIballcyc = 0;
    return;
  }
      
  DebugInt(printf("\nTime: %d\n", CurrentTime.t));
  DebugInt(printf("At BallIntercept_passive\n"));

  int passRet;
  passRet = GetClosestPointToBallPath(&BPIpoint, &BPIballcyc, MyPos(),
				      BallAbsolutePosition(), BallVel);
  DebugInt(printf("Passive Method: ret: %d\tx: %f\ty:%f\tcyc: %f\n",
		  passRet, BPIpoint.x, BPIpoint.y, BPIballcyc));
  if (passRet) {
    BPIvalid = TRUE;
    BPIable = TRUE;
    BPIdist = (BPIpoint - MyPos()).mod();
  } else {
    BPIvalid = TRUE;
    BPIable = FALSE;
  }

  return;
  
}

/*****************************************************************************************/

Vector ActionInfo::BallPathInterceptPoint()
{
  VerifyBPIInfo();
  if (!BPIvalid)
    my_error("Calling BallPathInterceptionPoint when info not valid?");
  return BPIpoint;
}

/*****************************************************************************************/

Bool ActionInfo::BallPathInterceptAmIThere(float buffer)
{
  VerifyBPIInfo();
  if (!BPIvalid)
    my_error("Calling BallPathInterceptionAmIThere when info not valid");
  return (BPIable && (MyPos() - BPIpoint).mod() <= buffer) ? TRUE : FALSE;
}

/*****************************************************************************************/

float ActionInfo::BallPathInterceptDistance()
{
  VerifyBPIInfo();
  if (!BPIable)
    my_error("Calling BallPathInterceptionDistance when I can't get get there");
  return BPIdist;
}

/*****************************************************************************************/

int ActionInfo::BallPathInterceptCyclesForBall()
{
  VerifyBPIInfo();
  if (!BPIable)
    my_error("Calling BallPathInterceptionCyclesForBall when I can't get get there");
  return (int)ceil(BPIballcyc);
}

/*****************************************************************************************/

Bool ActionInfo::BallPathInterceptCanIGetThere(float max_pow)
{
  VerifyBPIInfo();
  if (!BPIable)
    return FALSE;

  AngleDeg targAng = AngleToFromBody(BPIpoint);
  Vector myEnd;
  if (fabs(GetNormalizeAngleDeg(MyBodyAng() - targAng)) >
      CP_max_go_to_point_angle_err) {
    myEnd = MyPredictedPosition((int)ceil(BPIballcyc), max_pow);
  } else {
    myEnd = MyPredictedPositionWithTurn(targAng - MyBodyAng(),
					(int)ceil(BPIballcyc), max_pow);    
  }

  return ( (myEnd - MyPos()).mod() >= (BPIpoint - MyPos()).mod() ) ? TRUE : FALSE;
}




/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

float ActionInfo::VelAtPt2VelAtFoot(Vector pt, float targ_vel_at_pt)
{
  if (targ_vel_at_pt < FLOAT_EPS) {
    return SolveForFirstTermInfGeomSeries(SP_ball_decay, (pt - MyPos()).mod() );
  } else {
    float ball_steps =
      SolveForLengthGeomSeries(targ_vel_at_pt, 1/SP_ball_decay,
			       (pt - MyPos()).mod() );
    return targ_vel_at_pt * pow(1/SP_ball_decay, ball_steps);
  }  
}

/*****************************************************************************************/

/* looks at closeest opponent or teamless player */
/* SMURF: the teamless part is a hack */
KickMode ActionInfo::BestKickModeAbs(AngleDeg abs_ang)
{
  Unum closest = ClosestOpponent();
  if (NumTeamlessPlayers() > 0) {
    Vector teamless_pos = ClosestTeamlessPlayerPosition();
    if (closest == Unum_Unknown ||
	DistanceTo(teamless_pos) < OpponentDistance(closest))
      closest = Unum_Teamless;
  }

  if (closest == Unum_Unknown)
    return KM_HardestKick;
  int cyc_to_steal = EstimatedCyclesToSteal(closest);
  float targ_ang = abs_ang + signf(GetNormalizeAngleDeg(BallAngleFromBody()-abs_ang)) *
    (90 + CP_hardest_kick_ball_ang);
  float ang_diff = GetNormalizeAngleDeg(BallAngleFromBody() - targ_ang);
  NormalizeAngleDeg(&ang_diff);  
  if (cyc_to_steal > fabs(ang_diff)/CP_time_for_full_rotation + CP_cycles_to_kick)
    return KM_HardestKick;
  //if (OpponentWithBall() != Unum_Unknown)
  if (cyc_to_steal <= 1)
    return KM_QuickestRelease;
  if (cyc_to_steal < CP_cycles_to_kick)
    return KM_Quickly;
  if (cyc_to_steal < CP_cycles_to_kick + 1) // time for a dash in KM_Hard
    return KM_Moderate;
  return KM_Hard;
}

/*****************************************************************************************/


/* returns estimated cycles for opponent to get the ball into his kickable
   area */
/* can handle Unum_Teamless SMURF: it's kind of a hack though */
int ActionInfo::EstimatedCyclesToSteal(Unum opp, Vector ball_pos)
{
  if (!BallKickable())
    my_error("EstimatedCyclesToSteal: shouldn't use this if the ball is not kickable");

  if (BallKickableForOpponent(opp)) {
    LogAction2(110, "EstimatedCyclesToSteal: already kickable for opponent");
    return 0;
  }
  
  Vector targ = ball_pos;
  Vector pos;
  int cyc;
  float opp_kickable=(opp==TheirGoalieNum&&TheirPenaltyArea.IsWithin(ball_pos))?Mem->SP_catch_area_l:GetOpponentKickableArea(opp);
  if (opp == Unum_Teamless) {
    if (NumTeamlessPlayers() < 1)
      my_error("EstimatedCyclesToSteal: can't estimate teamless if there aren't any");
    pos = ClosestTeamlessPlayerPosition();
    targ -= (ball_pos - pos).SetLength(GetWorstOpponentKickableArea());
    cyc = (int)ceil(targ.dist(pos) / GetWorstOpponentPlayerSpeedMax());
  } else {
    if (!OpponentPositionValid(opp))
      my_error("EstimateCyclesToSteal: can't estimate if I don;t know where opponent is");
    pos = OpponentAbsolutePosition(opp);
    targ -= (ball_pos - pos).SetLength(opp_kickable);
    cyc = OpponentPredictedCyclesToPoint(opp, targ);
  }  
  //  Mem->LogAction4(10,"EstimateCyclesToSteal:opp %.0f has %.0f befor dodge correction",float(opp),float(cyc));
  /* now decide if the player will have to dodge */

  if (!pos.ApproxEqual(targ)&&PlayMode!=PM_Play_On) {
    Line oppLine = LineFromTwoPoints(pos, targ);
    Vector dodge_pos = oppLine.ProjectPoint(MyPos());
    dodge_pos += (pos - dodge_pos).SetLength(GetMyPlayerSize()+ GetOpponentPlayerSize(opp));
    float dodge_dist = oppLine.dist(MyPos());
    if (dodge_dist < (GetMyPlayerSize()+ GetOpponentPlayerSize(opp)) &&
	oppLine.InBetween(dodge_pos, pos, targ)) {
      // need to take into account a dodge 
      //      Mem->LogAction3(10,"EstimateCyclesToSteal:opp %.0f need to make dodge twice",float(opp));
      cyc += 2; //have to turn twice
      if (dodge_dist > (GetMyPlayerSize()+ GetOpponentPlayerSize(opp)) - GetOpponentPlayerSpeedMax(opp)){
	//Mem->LogAction3(10,"EstimateCyclesToSteal:opp %.0f one dash will dodge us",float(opp));
	cyc += 1; // one dash will dodge us
      }else{
	//	Mem->LogAction3(10,"EstimateCyclesToSteal:opp %.0f it takes two dashes to dodge us",float(opp));
	cyc += 2; // it takes two dashes to dodge us
      }
    }
  }
  
  return cyc;  
}



/*****************************************************************************************/

/* this is not an exact function becuase we don't have a perfect mapping of
   ball speed/position to kick power.
   Basically, this function returns whether the ball will be further back but still
   kickable after a dash */
Bool ActionInfo::WillDashHelpKick(Vector pt, float dash_pow)
{
  if (!BallWillBeKickable(1, dash_pow, CP_kickable_buffer)) {
    LogAction2(130, "WillDashHelpKick: ball will not be kickable");
    return FALSE;
  }
  
  /* we're going to assume that a collision is bad.
     but depending on how the ball is actually moving that could be good */
  if (WillDashBeCollision(dash_pow, CP_collision_buffer)) {
    LogAction2(130, "WillDashHelpKick: collision");
    return FALSE;
  }

  /* if we're not facing genrally in the direction we want to kick it,
     dashing will probably not help */
  if (fabs(AngleToFromBody(pt)) > CP_max_dash_help_kick_angle) {
    LogAction2(130, "WillDashHelpKick: not facing");    
    return FALSE;
  }
  
  AngleDeg curr_ang = BallAngleFromBody() - AngleToFromBody(pt);
  NormalizeAngleDeg(&curr_ang);
  Vector my_pred_pos = MyPredictedPosition(1, dash_pow);
  AngleDeg pred_ang =
    (BallPredictedPosition() - my_pred_pos).dir() -
    (pt - my_pred_pos).dir();
  NormalizeAngleDeg(&pred_ang);

  LogAction4(130, "WillDashHelpKick: curr: %.1f  pred: %.1f", curr_ang, pred_ang);
  
  return (fabs(pred_ang) > fabs(curr_ang)) ? TRUE : FALSE;
}


/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

Bool ActionInfo::KickInProgress()
{
  /* need to have kicked last cycle.  Updates kick_in_progress_time */
  if ( kick_in_progress && kick_in_progress_time == LastActionOpTime ){
    kick_in_progress_time = CurrentTime;
    return TRUE;
  }
  return FALSE;
}

/*****************************************************************************************/

void ActionInfo::StartKick(AngleDeg target_angle, KickMode mode, float target_vel, TurnDir rot)
{
  kick_in_progress = TRUE;
  start_kick_time = kick_in_progress_time = CurrentTime;
  kick_in_progress_abs_angle = GetNormalizeAngleDeg(target_angle + MyBodyAng());
  kick_in_progress_mode = mode;
  kick_in_progress_target_vel = target_vel;
  kick_in_progress_rotation = rot;
}

/*****************************************************************************************/

void ActionInfo::StartShot(AngleDeg target_angle, KickMode mode, TurnDir rot)
{
  StartKick(target_angle,mode,2*SP_ball_speed_max, rot);
}

/*****************************************************************************************/

void ActionInfo::StartPass(Unum target, float target_vel_at_dest, TurnDir rot)
{
  if ( target == Unum_Unknown || !TeammatePositionValid(target) ) my_error("can't start this pass");

  team_passer = MyNumber;
  team_receiver = target;
  team_pass_time = CurrentTime;

  float target_vel = VelAtPt2VelAtFoot(TeammateAbsolutePosition(target),target_vel_at_dest);
  StartKick(TeammateAngleFromBody(target),KM_Moderate,target_vel,rot);
}




/*****************************************************************************************/
/*****************************************************************************************/
/*****************************************************************************************/

/* No reasoning about players being tired yet:  
   if so, need to add dash_pow to the interception calls */

/* These functions are very computationally intensive */

/* the stored value does not include the goalie */
Unum ActionInfo::FastestTeammateToBall()
{
  if ( !BallPositionValid() ) my_error("Need to know ball position to know fastest to it\n");

  Unum closest = ClosestTeammateToBall();
  if ( !BallMoving() && !TeammateTired(closest) ) return closest;

  if ( CurrentTime == Stored_Fastest_Teammate_Time ) return Stored_Fastest_Teammate;

  ResetInterceptionMinCyc();
  SetInterceptionLookahead(LA_BestSoFar);

  Unum FastestPlayer = Unum_Unknown;
  int cycles, min_cycles = CP_max_int_lookahead+1;

  for (int i=1; i<=SP_team_size; i++){
    if ( TeammatePositionValid(i) && TeammateInterceptionAble(i) == TRUE &&
	 (cycles=TeammateInterceptionNumberCycles(i)) < min_cycles && 
	 (i != FP_goalie_number || CP_goalie) ){
      min_cycles = cycles;
      FastestPlayer = i;
    }
  }

  Stored_Fastest_Teammate = FastestPlayer;
  Stored_Fastest_Teammate_Time = CurrentTime;

  return Stored_Fastest_Teammate;
}

/*****************************************************************************************/

Unum ActionInfo::FastestOpponentToBall()
{
  if ( !BallPositionValid() ) my_error("Need to know ball position to know fastest to it\n");

  if ( !BallMoving() ) return ClosestOpponentToBall();

  if ( CurrentTime == Stored_Fastest_Opponent_Time ) return Stored_Fastest_Opponent;

  ResetInterceptionMinCyc();
  SetInterceptionLookahead(LA_BestSoFar);

  Unum FastestPlayer = Unum_Unknown;
  int cycles, min_cycles = CP_max_int_lookahead+1;
  for (int i=1; i<=SP_team_size; i++){
    if ( OpponentPositionValid(i) && OpponentInterceptionAble(i) == TRUE &&
	 (cycles=OpponentInterceptionNumberCycles(i)) < min_cycles ){
      min_cycles = cycles;
      FastestPlayer = i;
    }
  }

  Stored_Fastest_Opponent = FastestPlayer;
  Stored_Fastest_Opponent_Time = CurrentTime;

  return FastestPlayer;
}

/*****************************************************************************************/
Unum ActionInfo::BallPossessor(){

  if (!BallPositionValid()) {
    //my_error("BallPossesor: ball position not valid");
    return Unum_Unknown;
  }

  Unum num_with_ball = PlayerWithBall();
  if (num_with_ball != Unum_Unknown)
    return num_with_ball;

  Unum fastestTeammate, fastestOpponent;
  
  if (BallMoving()) {
    int teamCycles, oppCycles;
    fastestOpponent = FastestOpponentToBall();
    fastestTeammate = FastestTeammateToBall(); 
  
    if (fastestTeammate == Unum_Unknown ||
	fastestOpponent == Unum_Unknown) 
      return (fastestTeammate == Unum_Unknown ? -fastestOpponent : fastestTeammate);

    teamCycles = TeammateInterceptionNumberCycles(fastestTeammate); 
    oppCycles = OpponentInterceptionNumberCycles(fastestOpponent); 

    if (teamCycles + CP_possessor_intercept_space < oppCycles)
      return fastestTeammate;
    else if (oppCycles + CP_possessor_intercept_space < teamCycles)
      return -fastestOpponent;
  } else {
    fastestTeammate = ClosestTeammateToBall();
    fastestOpponent = ClosestOpponentToBall();

    if (fastestTeammate == Unum_Unknown ||
	fastestOpponent == Unum_Unknown) 
      return (fastestTeammate == Unum_Unknown ? -fastestOpponent : fastestTeammate);

    /* we'll just ignore facing angles because they probably aren't right anwyay */;
    if (TeammateAbsolutePosition(fastestTeammate).dist(BallAbsolutePosition()) <
	OpponentAbsolutePosition(fastestOpponent).dist(BallAbsolutePosition()))
      return fastestTeammate;
    else
      return -fastestOpponent;
  }

  return Unum_Unknown;
}

 
/*****************************************************************************************/

char ActionInfo::TeamInPossession()
{
  switch ( PlayMode ){
  case PM_Play_On: break;
  case PM_My_Kick_In:
  case PM_My_Corner_Kick:
  case PM_My_Kick_Off:
  case PM_My_Free_Kick:
  case PM_My_Goalie_Free_Kick:
  case PM_My_Offside_Kick:
  case PM_My_Goal_Kick: return MySide;
  case PM_Their_Kick_In:
  case PM_Their_Corner_Kick:
  case PM_Their_Goal_Kick:
  case PM_Their_Kick_Off:
  case PM_Their_Offside_Kick:
  case PM_Their_Free_Kick: 
  case PM_Their_Goalie_Free_Kick: return TheirSide;
  default: break;
  }

  Unum player = BallPossessor();
  if ( player > 0 ) return MySide;
  else if (player < 0) return TheirSide;
  else return '?';
}
/************************************************************************************/
//returns the expected success
float ActionInfo::EvaluateClearAngle(Vector end)
{
  Line l = LineFromTwoPoints(BallAbsolutePosition(), end);
  float exp_success = 1.0;
  for (Unum opp = 1; opp <= SP_team_size; opp++) {
    if (!OpponentPositionValid(opp))
      continue;
    Vector pt = l.ProjectPoint(OpponentAbsolutePosition(opp));
    if (!l.InBetween(pt, BallAbsolutePosition(), end))
      continue;
    float wid = pt.dist(OpponentAbsolutePosition(opp));
    float dist = BallAbsolutePosition().dist(pt);
    if (wid > dist * CP_clear_ball_cone_ratio)
      continue;
    exp_success *= wid / (dist * CP_clear_ball_cone_ratio);
  }
  return exp_success;
}
 

//**********************************************************************************8
///////////////////////////////////////////////////////////////////////////////
Unum ActionInfo::OurBreakaway()
{
  if (!BallPositionValid())
    my_error("breakway: lost ball");

  Unum team_ball = TeammateWithBall(-CP_our_breakaway_kickable_buffer);

  if (team_ball == Unum_Unknown)
    return Unum_Unknown;

  Unum opp_ball = OpponentWithBall(-CP_our_breakaway_kickable_buffer);

  float wid_dist_ratio =
    (SP_goal_area_width/2) / Min(TeammateDistanceTo(team_ball, MarkerPosition(RM_Their_Goal)),
				 CP_our_breakaway_min_cone_dist_wid);
  int num_opp = NumOpponentsInCone( wid_dist_ratio,
				    MarkerPosition(RM_Their_Goal),
				    TeammateAbsolutePosition(team_ball));
  return (team_ball != Unum_Unknown &&
	  opp_ball == Unum_Unknown &&
	  num_opp <= 1)
    ? team_ball : Unum_Unknown;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Unum ActionInfo::TheirBreakaway(){
  Unum opp=Mem->FastestOpponentToBall();
  if(opp==Unum_Unknown||!OpponentPositionValid(opp))
    return Unum_Unknown;
  Vector goal=MarkerPosition(Mem->RM_My_Goal);
  Vector alian=OpponentAbsolutePosition(opp);
  if(OpponentDistanceToBall(opp)>CP_their_breakaway_back_kickable_buffer  &&
     abs((int)GetNormalizeAngleDeg((goal-alian).dir() -(BallAbsolutePosition()-alian).dir()))>=90.0) //not breakaway
    return Unum_Unknown;
  float coneRatio=(SP_goal_area_width/2)/Min(OpponentDistanceTo(opp,goal),12.0);
  int num=NumTeammatesInCone(coneRatio,goal,alian);
  Unum our=TeammateWithBall(-CP_their_breakaway_back_kickable_buffer);
  if(num<=1&&our==Unum_Unknown)
    return opp;
  else
    return Unum_Unknown;
}
