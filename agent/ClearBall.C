/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : ClearBall.C
 *
 *    AUTHOR     : Sergei Serebyakov, Anton Ivanov
 *
 *    $Revision: 2.16 $
 *
 *    $Id: ClearBall.C,v 2.16 2004/06/22 17:06:16 anton Exp $
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
#include "ClearBall.h"
#include "cross.h"
#include <iomanip>
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ClearBall::clear_ball(bool fast_clear)
{
  ClearInfo info=clear_info();
  float vel=Mem->SP_ball_speed_max*2.0f;
  SK_Res res;
  if(fast_clear){
    res=smartkickg(vel, info.angle, SK_Fast);
  }else{
    if(!Mem->OwnPenaltyArea.IsWithin(Mem->MyPos())&&microKicks.CanMicroClearBall(info.type==OffenseClear?false:true))
      return;
    if(info.type==OffenseClear){
      if( Pos.GetMyType()>=PT_Midfielder){
	Vector BallPos=Mem->BallAbsolutePosition();
	static Unum* team=new Unum[Mem->SP_team_size];
	Vector pos2=BallPos+Polar2Vector(5.0f,Mem->MyBodyAng());
	Line l;l.LineFromTwoPoints(BallPos,pos2);
	int num=Mem->SortPlayersByDistanceToLine('t', l,team,TRUE,BallPos, pos2);
		
	if((num==0||team[0]==0||!Mem->OpponentPositionValid(team[0])||(l.ProjectPoint(Mem->OpponentAbsolutePosition(team[0]))-
		     Mem->OpponentAbsolutePosition(team[0])).mod()>Mem->GetOpponentKickableArea(team[0]))&&
	   Mem->FieldRectangle.IsWithin(pos2)&&Mem->MyStamina()>Mem->SP_stamina_max*0.6f&&(Mem->OpponentBodyAngleValid(team[0])<0.98f||
	   (l.ProjectPoint(Mem->OpponentAbsolutePosition(team[0])+
			   Polar2Vector(Mem->SP_dash_power_rate*Mem->SP_max_power,Mem->OpponentAbsoluteBodyAngle(team[0])))-
	    Mem->OpponentAbsolutePosition(team[0])).mod()>Mem->GetOpponentKickableArea(team[0]))){
	  if(Mem->OpponentPositionValid(team[0]))
	    Mem->LogAction5(10,"ClearBall: can kick forward, becouse opp %d may be not intercept (%.2f>%.2f)",
			    team[0],(l.ProjectPoint(Mem->OpponentAbsolutePosition(team[0]))-
		     Mem->OpponentAbsolutePosition(team[0])).mod(),Mem->GetOpponentKickableArea(team[0]));
	  smartkick(1.0f/*MinMax(1.2f,Mem->MySpeed()+1.0f,2.0f)*/,pos2,SK_Safe);
	  return;
	}
      }
      if(microKicks.CanMicroClearBall(true))
	return;
    }
    
    if(info.type<GoalClear){
      if(Cross::ThroughPass())
	return;
    }
    
    if(info.type==OffenseClear){
      if( Pos.GetMyType()>=PT_Midfielder){

	Vector target=Vector(48.0f,signf(Mem->BallY())*(Mem->SP_penalty_area_width/2.0f+3.0f));
	vel=(target-Mem->BallAbsolutePosition()).mod()/Mem->GetBallMoveCoeff(50);
    
	res=smartkick(vel,target,SK_Fast);
	return;
      }
      
    }

    res=smartkickg(vel, info.angle,SK_Safe);
  }
  switch(info.type) {
  case DefenseClear:
    if( res==SK_KickDone ) clear_statistica.defense_clear();
    sprintf(clear_log_message,"ClearBall::defense clear at global angle %.2f",info.angle);
    break;
  case OffenseClear:
    if( res==SK_KickDone ) clear_statistica.offense_clear();
    sprintf(clear_log_message,"ClearBall::offense clear at global angle %.2f",info.angle);
    break;
  case GoalClear:
    if( res==SK_KickDone ) clear_statistica.goal_clear();
    sprintf(clear_log_message,"ClearBall::goal clear at global angle %.2f",info.angle);
    break;
  case MidfieldClear:
    if(res==SK_KickDone) clear_statistica.midfield_clear();
    sprintf(clear_log_message,"ClearBall::midfield clear at global angle %.2f",info.angle);
    break;
  default:
    sprintf(clear_log_message,"ClearBall::unknown clear type at global angle %.2f",info.angle);
  }
  Mem->LogAction2(10,clear_log_message);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ClearBall::ClearInfo ClearBall::clear_info() {
  ClearInfo info;
  Vector ball=Mem->BallAbsolutePosition();

  if( ball.x>30.0) {//AI: was 35.0
    info.type=GoalClear;
    info.angle=goal_clear();
  }
  else if(ball.x<=-35.0) {
    info.type=DefenseClear;
    info.angle=defense_clear();
  }
  else if(ball.x>-35.0&&ball.x<0.0 ) {
    info.type=MidfieldClear;
    info.angle=midfield_clear();
  }
  else {
    info.type=OffenseClear;
    info.angle=offense_clear();
  }

  return info;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg ClearBall::defense_clear() {

  AngleDeg lowangle, upangle, angle, clear_angle=0;
  float dist;
  if( Mem->MyY()>0 ) {
    lowangle=GetNormalizeAngleDeg((Vector(-20.0f,34)-Mem->MyPos()).dir());
    upangle=GetNormalizeAngleDeg((Vector(0,0)-Mem->MyPos()).dir());
    dist=25;//Max((Vector(0,34)-Mem->MyPos()).mod(),(Vector(0,0)-Mem->MyPos()).mod());
  }else{
    lowangle=GetNormalizeAngleDeg((Vector(-20.0f,-34)-Mem->MyPos()).dir());
    upangle=GetNormalizeAngleDeg((Vector(0,0)-Mem->MyPos()).dir());
    dist=25;//Max((Vector(0,-34)-Mem->MyPos()).mod(),(Vector(0,0)-Mem->MyPos()).mod());
  }

  if( lowangle>upangle ) {
    AngleDeg keeper=lowangle;
    lowangle=upangle;
    upangle=keeper;
  }

  // clear_angle=Cross::GetDirectionOfWidestAngle(Mem->MyPos(),lowangle,upangle,&angle,dist);
  clear_angle=(Pos.GetOptimalPointOnLine(Mem->MyPos(),Mem->MyPos()+Polar2Vector(dist,lowangle),
  				 Mem->MyPos()+Polar2Vector(dist,upangle),&angle)-Mem->MyPos()).dir();
  Mem->LogAction6(10,"defense clear:lowang %.2f upang %.2f clearang %.2f widang %.2f",
		  lowangle,
		  upangle,
		  clear_angle,
		  angle);
  return clear_angle;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg ClearBall::midfield_clear() {

  AngleDeg lowangle, upangle, angle, clear_angle;
  float low,up,ladd=15.0f,uadd=15.0f;

  if(fabs(Mem->MyY())<15.0){
    low=-34.0f;ladd=Mem->MyY()<0?40.0:10.0f;
    up=34.0f;uadd=Mem->MyY()>0?40.0:10.0f;
  }else
    if(Mem->MyY()<0){
      low=Mem->MyY();
      up=0.0;
    }else{
      low=0.0f;
      up=Mem->MyY();
    }

  if(Mem->MyX()<-20.0){
    ladd=uadd=30.0f;
  }
  lowangle=(Vector(Mem->MyX()+ladd,low)-Mem->MyPos()).dir();
  upangle=(Vector(Mem->MyX()+uadd,up)-Mem->MyPos()).dir();
  float dist=25;

  if( lowangle>upangle ) {
    AngleDeg keeper=lowangle;
    lowangle=upangle;
    upangle=keeper;
  }
  //  clear_angle=Cross::GetDirectionOfWidestAngle(Mem->MyPos(),lowangle,upangle,&angle,dist);
  clear_angle=(Pos.GetOptimalPointOnLine(Mem->MyPos(),Mem->MyPos()+Polar2Vector(dist,lowangle),
  					 Mem->MyPos()+Polar2Vector(dist,upangle),&angle)-Mem->MyPos()).dir();
  Mem->LogAction7(10,"midfield clear:lowang %.2f upang %.2f clearang %.2f widang %.2f dist %.2f",
		  lowangle,
		  upangle,
		  clear_angle,
		  angle,
		  dist);
  return clear_angle;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg ClearBall::goal_clear() {

  AngleDeg lowangle, upangle, angle, clear_angle;


  lowangle=(Vector(Mem->SP_pitch_length/2,0)-Mem->MyPos()).dir();
  upangle=(Vector(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length,0)-Mem->MyPos()).dir();
  float dist=Max((Vector(Mem->SP_pitch_length/2,0)-Mem->MyPos()).mod(),
                 (Vector(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length,0)-Mem->MyPos()).mod());
  if( lowangle>upangle ) {
    AngleDeg keeper=lowangle;
    lowangle=upangle;
    upangle=keeper;
  }

  //  clear_angle=Cross::GetDirectionOfWidestAngle(Mem->MyPos(),lowangle,upangle,&angle,dist);
  clear_angle=(Pos.GetOptimalPointOnLine(Mem->MyPos(),Mem->MyPos()+Polar2Vector(dist,lowangle),
  					 Mem->MyPos()+Polar2Vector(dist,upangle),&angle)-Mem->MyPos()).dir();
  Mem->LogAction6(10,"goal clear:lowang %.2f upang %.2f clearang %.2f widang %.2f",
		  lowangle,
		  upangle,
		  clear_angle,
		  angle);
  return clear_angle;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg ClearBall::offense_clear() {
  AngleDeg lowangle, upangle, angle, clear_angle;
  float dist;
  if( Mem->MyY()>0 ) {
    upangle = GetNormalizeAngleDeg((Vector(52.5-Mem->SP_penalty_area_length,34)-Mem->MyPos()).dir());
    lowangle=GetNormalizeAngleDeg((Vector(52.5,Mem->SP_penalty_area_width/2)-Mem->MyPos()).dir());
    dist=Max((Vector(52.5-Mem->SP_penalty_area_length,34)-Mem->MyPos()).mod(),
             (Vector(52.5,Mem->SP_penalty_area_width/2)-Mem->MyPos()).mod());
  }else{
    upangle = GetNormalizeAngleDeg((Vector(52.5-Mem->SP_penalty_area_length,-34)-Mem->MyPos()).dir());
    lowangle=GetNormalizeAngleDeg((Vector(52.5,-Mem->SP_penalty_area_width/2)-Mem->MyPos()).dir());
    dist=Max((Vector(52.5-Mem->SP_penalty_area_length,-34)-Mem->MyPos()).mod(),
             (Vector(52.5,-Mem->SP_penalty_area_width/2)-Mem->MyPos()).mod());
  }

  if( lowangle>upangle ) {
    AngleDeg keeper=lowangle;
    lowangle=upangle;
    upangle=keeper;
  }


  //clear_angle=Cross::GetDirectionOfWidestAngle(Mem->MyPos(),lowangle,upangle,&angle,dist);
  clear_angle=(Pos.GetOptimalPointOnLine(Mem->MyPos(),Mem->MyPos()+Polar2Vector(dist,lowangle),
  				 Mem->MyPos()+Polar2Vector(dist,upangle),&angle)-Mem->MyPos()).dir();
  Mem->LogAction6(10,"offense clear:lowang %.2f upang %.2f clearang %.2f widang %.2f",
		  lowangle,
		  upangle,
		  clear_angle,
		  angle);
  return clear_angle;

}
