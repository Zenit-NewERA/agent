/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Playposition.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.12 $
 *
 *    $Id: Playposition.C,v 2.12 2004/06/22 17:06:16 anton Exp $
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


#include "Playposition.h"
#include "behave.h"
#include "scenario.h"
#include "Handleball.h"

PlayPosition Pos;
//////////////////////////////////////////////////////////////////
PlayPosition::PlayPosition(){
  for(int i=0;i<11;i++){
    posValid[i]=bodyAngValid[i]=velValid[i]=lastPosValid[i]=lastBodyAngValid[i]=lastVelValid[i]=-1.0f;
    bodyAng[i]=0.0f;
    vel[i]=Vector(.0f,.0f);
    infoUpdate[i]=-1;
  }
}
//////////////////////////////////////////////////////////////////
bool PlayPosition::PlayWithoutBall(){ //return true if this func must control turn neck
  
  Mem->LogAction6(10,"Fastest teammate: %.0f (cycles %.0f), fastest opponent: %.0f (cycles %.0f)",float(FastestTm()),
		  float(TmCycles()),float(FastestOpp()),float(OppCycles()));

  if(throughPass.GoInThroughPass())
    return false;
  int temp=without_ball();
  if(temp==1)
    offense();
  else if(temp==0)
    defense();
    
  return false;
}
////////////////////////////////////////////////
int PlayPosition::without_ball(Unum tm){//return 0 - defense; 1- offense; 2- ball will be outside of field
//   if(Mem->BallX()>=Mem->SP_pitch_length/4&&GetPlayerType(tm)>PT_Defender){
//     if(Mem->TeamInPossession()==Mem->TheirSide&&Pos.FastestTm()==tm)//otbiraem miach u protivnika v uglu
//       return 0;
//     return 1;
//   }
  Unum fastest=FastestTm();
  if(GetPlayerType(tm)>=PT_Midfielder&&fastest==Unum_Unknown||(GetPlayerType(fastest)<=PT_Defender&&Mem->BallX()<0.0f&&
							       ((Mem->TeammateInterceptionAble(tm)&&(Mem->TeammateInterceptionNumberCycles(tm)-TmCycles())<2)||(OppCycles()-TmCycles())<4)))//hack
    return 0;

  if(GetPlayerType(tm)<=PT_Defender)
    return 0;
  if(IsOffense()||(GetPlayerType(tm)==PT_Forward&&(OppCycles()+2>=TmCycles()||FastestOpp()==Mem->TheirGoalieNum)))//if other teammate controls the ball
    return 1;
  else
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////
bool PlayPosition::CheckTopBehavior(Unum tm){//the content is exectly such  in play_game()
  if ( Mem->TeammateInOffsidePosition(tm) ) {
    if(Mem->TeammateInterceptionAble(tm)&&Mem->FastestTeammateToBall()==tm){
      holder=Holder(t_get_ball,"get ball then player in offside");
      return true;
    }
    holder=Holder(t_go_to_pos,Vector(-50.0f,GetTmPos(tm).y),5.0f,Mem->SP_max_power,"get on side"); //get_on_side()
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////////
void PlayPosition::UpdateTmState(Unum tm){//info in holder
  int i=tm-1;
  if(holder.type==t_turn_body_to_ball){
    bodyAng[i]=Mem->AngleToGlobal(Mem->BallAbsolutePosition());//hack
    pos[i]+=vel[i];
    vel[i]*=Mem->GetTeammatePlayerDecay(tm);
    return;
  }
  if(holder.type==t_get_ball){
    holder.ang_error=5.0f;
    if(Mem->TeammateInterceptionAble(tm))
      holder.pos=Mem->TeammateInterceptionPoint(tm);
    else
      holder.pos=Mem->BallAbsolutePosition();
  }
  //check if already in target pos
  if((pos[i]-holder.pos).mod()<=Mem->CP_at_point_buffer){
    pos[i]+=vel[i];
    vel[i]*=Mem->GetTeammatePlayerDecay(tm);
    return;
  }
  AngleDeg turn_ang=GetNormalizeAngleDeg(holder.pos.dir()-bodyAng[i]);
  if(fabs(turn_ang)>holder.ang_error){//need to make turn
    float this_turn = MinMax(-Mem->EffectiveTurn(Mem->SP_max_moment, vel[i].mod()),
			     turn_ang,
			     Mem->EffectiveTurn(Mem->SP_max_moment, vel[i].mod()));
    bodyAng[i] += this_turn;
    pos[i]+=vel[i];
    vel[i]*=Mem->GetTeammatePlayerDecay(tm);
    return;
  }
  //make dash
  vel[i]+=Polar2Vector(Mem->GetTeammateEffortMax(tm)*Mem->GetTeammateDashPowerRate(tm)*holder.dash,bodyAng[i]);
  if(vel[i].mod()>Mem->GetTeammatePlayerSpeedMax(tm))
    vel[i]=(vel[i]*Mem->GetTeammatePlayerSpeedMax(tm))/vel[i].mod();
  pos[i]+=vel[i];

  vel[i]*=Mem->GetTeammatePlayerDecay(tm);
}
/////////////////////////////////////////////////////////////////////////////////
void PlayPosition::UpdateTmPredictPos(){ //hard calculations
  Vector target;
  float ang_err=0.0f;
  if(Mem->PlayMode!=PM_Play_On)
    return;
  
  Scenarios.StartScenario();//update scenario info
  
  for(Unum tm=1;tm<=11;tm++){
    bool update_state=true;
    if(!Mem->TeammatePositionValid(tm)||Mem->OurGoalieNum==tm||tm==Mem->MyNumber)
      continue;
    if(Mem->TeammateVelocityValid(tm)&&(Mem->TeammateVelocityValid(tm)>=velValid[tm-1]||lastVelValid[tm-1]<=Mem->TeammateVelocityValid(tm))){
      velValid[tm-1]=Mem->TeammateVelocityValid(tm);
      vel[tm-1]=Mem->TeammateAbsoluteVelocity(tm);
    }
    lastVelValid[tm-1]=Mem->TeammateVelocityValid(tm);
    if(Mem->TeammateBodyAngleValid(tm)&&(Mem->TeammateBodyAngleValid(tm)>=bodyAngValid[tm-1]||lastBodyAngValid[tm-1]<=Mem->TeammateBodyAngleValid(tm))){
      bodyAngValid[tm-1]=Mem->TeammateBodyAngleValid(tm);
      bodyAng[tm-1]=Mem->TeammateAbsoluteBodyAngle(tm);
    }
    lastBodyAngValid[tm-1]=Mem->TeammateBodyAngleValid(tm);
    if(Mem->TeammatePositionValid(tm)>=posValid[tm-1]||lastPosValid[tm-1]<=Mem->TeammatePositionValid(tm)){
      posValid[tm-1]=Mem->TeammatePositionValid(tm);
      pos[tm-1]=Mem->TeammateAbsolutePosition(tm);
      update_state=false;
    }
    lastPosValid[tm-1]=Mem->TeammatePositionValid(tm);
		
    //update positions
    if(Mem->BallPositionValid()&&Pos.FastestTm()==Mem->MyNumber){
      if(without_ball(tm)==0||Mem->PlayMode!=PM_Play_On)//not in defense and set play
        continue;
      infoUpdate[tm-1]=Mem->CurrentTime;
      if(Scenarios.IsScenarioGoing()&&Scenarios.GetTmTargetPos(tm,target,ang_err)){
        holder=Holder(t_go_to_pos,target,ang_err,Mem->SP_max_power,"go to pos in scenario");
        info[tm-1]=holder;
        if(update_state)
          UpdateTmState(tm);
      }else
	if(CheckTopBehavior(tm)){
	  if(update_state)
	    UpdateTmState(tm);
	  info[tm-1]=holder;
	}else
	  if(throughPass.IsTmGoInThroughPass(tm)){
	    holder=Holder(t_go_to_pos,throughPass.GetThroughPassTarget(),Mem->CP_max_go_to_point_angle_err,Mem->SP_max_power,"go to pos in through pass");
	    info[tm-1]=holder;
	    if(update_state)
	      UpdateTmState(tm);
	  }else{
	    offense(tm);//fill holder
	    info[tm-1]=holder;
	    if(update_state)
	      UpdateTmState(tm);
	  }
			
    }
  }//end cycle of tm
}
/////////////////////////////////////////////////////////////////////////////////
Vector PlayPosition::GetTmPos(Unum tm){
  if(!Mem->TeammatePositionValid(tm))
    return GetHomePosition(tm);
  if(Mem->OurGoalieNum==tm||tm==Mem->MyNumber||without_ball(tm)==0||Mem->PlayMode!=PM_Play_On)
    return Mem->TeammateAbsolutePosition(tm);
  return pos[tm-1];
}
///////////////////////////////////////////////////////////////////////////////////
Vector PlayPosition::GetTmTargetPoint(Unum tm){
  if(infoUpdate[tm-1]==Mem->CurrentTime){
    ostringstream ost("Teammate ");
    ost<<tm<<" "<<info[tm-1].name<<" ("<<info[tm-1].pos.x<<","<<info[tm-1].pos.y<<")";
    Mem->LogAction3(50,"%s",const_cast<char*>(ost.str().c_str()));
    return info[tm-1].pos;
  }
  if(tm==Mem->MyNumber)
    return Mem->MyPos();
  if(!Mem->TeammatePositionValid(tm)||Mem->OurGoalieNum==tm||without_ball(tm)==0||Mem->PlayMode!=PM_Play_On||FastestTm()!=Mem->MyNumber){
    if(Mem->TeammatePositionValid(tm))
      return Mem->TeammateAbsolutePosition(tm);
    else
      return Pos.GetHomePosition(tm);
  }
  my_error("Function UpdateTmPredictPos() was not call first!!!");
  return Vector(.0f,.0f);
}
///////////////////////////////////////////////////////////////////////////////////
float PlayPosition::GetEndVelInSmartPassKick()const{
  return 1.0f;
}
//////////////////////////////////////////////////////////////////////////////////
Vector PlayPosition::GetSmartPassPoint(Unum tm,float* ret_vel){//ret_vel is not used now
  Vector target=GetTmTargetPoint(tm);
  Mem->LogAction5(100,"GetSmartPassPoint: for tm %.0f target point is (%.2f,%.2f)",float(tm),target.x,target.y);
  Vector begin=GetTmPos(tm);
  if((target-begin).mod()<Mem->CP_at_point_buffer*2){
    if(ret_vel!=0)
      *ret_vel=1.4f;
    return begin;
  }
  Vector ball=Mem->BallAbsolutePosition();
  float vel=Mem->VelAtPt2VelAtFoot(begin,2.0f);
  float cyc_travel=SolveForLengthGeomSeries(vel,Mem->SP_ball_decay,(ball-begin).mod());
  int kick_cyc=actions.get_kick_cycles(vel,actions.GetKickAngle(begin));
  Mem->LogAction5(150,"GetSmartPassPoint: vel: %.2f; cyc_travel: %.0f; kick_cyc: %.0f",vel,float(cyc_travel),float(kick_cyc));
  float add_dist=(cyc_travel+kick_cyc)*1.0f;
  if(add_dist>3.0f)
    add_dist=3.0f;
  //  add_dist/=2;//temp
  Vector add_vector=((target-begin)*add_dist)/(target-begin).mod();
  if(add_vector.mod()>(target-begin).mod())
    add_vector*=(target-begin).mod()/add_vector.mod();
  if(ret_vel!=0)
    *ret_vel=vel;
  Mem->LogAction5(150,"GetSmartPassPoint: add dist is %.2f (dist to target: %.2f);correct vel: %.2f",
		  add_vector.mod(),(target-begin).mod(),ret_vel==0?0:*ret_vel);
  return begin+add_vector;
}
///////////////////////////////////////////////////////////////////////////////////
Vector PlayPosition::GetSmartPassPointInScenario(Unum tm,float* ret_vel){
  Vector target=GetTmTargetPoint(tm);
  Mem->LogAction5(100,"GetSmartPassPoint: for tm %.0f target point is (%.2f,%.2f)",float(tm),target.x,target.y);
  Vector begin=GetTmPos(tm);
  if((target-begin).mod()<Mem->CP_at_point_buffer){
    *ret_vel=1.4f;
    return begin;
  }
  Vector ball=Mem->BallAbsolutePosition();
  float vel=Mem->VelAtPt2VelAtFoot(begin,GetEndVelInSmartPassKick());
  float cyc_travel=SolveForLengthGeomSeries(vel,Mem->SP_ball_decay,(ball-begin).mod());
  int kick_cyc=actions.get_kick_cycles(vel,actions.GetKickAngle(begin));
  Mem->LogAction5(150,"GetSmartPassPoint: vel: %.2f; cyc_travel: %.0f; kick_cyc: %.0f",vel,float(cyc_travel),float(kick_cyc));
  float add_dist=(cyc_travel+kick_cyc)*1.0f;
  if(add_dist>7.0f)
    add_dist=7.0f;
  Vector add_vector=((target-begin)*add_dist)/(target-begin).mod();
  if(add_vector.mod()>(target-begin).mod())
    add_vector*=(target-begin).mod()/add_vector.mod();
  *ret_vel=SolveForFirstTermGeomSeries(Mem->SP_ball_decay,int(cyc_travel)+kick_cyc,(begin+add_vector-ball).mod());//=vel;
  Mem->LogAction5(150,"GetSmartPassPoint: add dist is %.2f (dist to target: %.2f);correct vel: %.2f",
		  add_vector.mod(),(target-begin).mod(),*ret_vel);
  return begin+add_vector;
}
///////////////////////////////////////////////////////////////////////////////////
void PlayPosition::SetFormation()
{
  if(IsStrongOpponent()==1&&Mem->BallX()<-15.0f&&!Mem->BallKickable())
    SetCurrentFormation(FT_523_DEFENSE);
  else
    SetCurrentFormation(FT_433_OFFENSE);
}
//////////////////////////////////////////////////////////////////////
