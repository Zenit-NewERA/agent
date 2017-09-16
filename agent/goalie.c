/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : goalie.C
 *
 *    AUTHOR     :Anton Ivanov, Sergei Serebyakov
 *
 *    $Revision: 2.16 $
 *
 *    $Id: goalie.C,v 2.16 2004/08/29 14:07:21 anton Exp $
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

#include "goalie.h"
#include "behave.h"
#include "Playposition.h"
#include "client.h"
#include "Handleball.h"
#include "SetPlay.h"
//////////////////////////////////////////////////////////////////////

Goalie::Goalie():BASE(5.5f)//may be with /2
{
  last_observe_time=last_catch_time=-1; 
}

//////////////////////////////////////////////////////////////////////

void Goalie::kick_off()
{
  static int count_steps=0;
  static long wait=-1;
  static bool first=true;
	
  if(Mem->PlayMode!=PM_My_Goalie_Free_Kick){
    setPlay.ForGoalieMyKickOff(SetPlay::flash);
    count_steps=0;
    wait=-1;
    first=true;
    return;
  }
  Mem->LogAction2(10,"Goalie - in kick off mode");
  if(Mem->MyConf()){
    if(setPlay.Recovery())
      return;
    count_steps++;
    if(wait>0)
      wait--;
  }
  if( actions.KickInProgress() ) {
    Mem->LogAction2(30, "Goalie::kick in progress");
    actions.smartkick();
    return;
  }
  const int wait_pouse=29;
  if(wait==0){
    Mem->LogAction2(10,"Goalie: wait too long, so must make kick");
    setPlay.ForGoalieMyKickOff(SetPlay::estimate_now);
    count_steps=wait_pouse;
    wait=-1;
    return;
  }
  Mem->LogAction3(10,"Goalie - in kick off, count_steps %.0f", float(count_steps));
  switch(count_steps)
    {
    case 1: move(-(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length+4.0), 0); break;
    case 2:	face_neck_and_body_to_point(Vector(0,0));
      change_view(VW_Wide);
      break;
    case wait_pouse:
      if(Mem->CurrentTime-1==Mem->LastSightTime)
	{
          if(setPlay.ScenarioGoing()){
            setPlay.ForGoalieMyKickOff(SetPlay::scenario_estimate);
          }else
	    if(setPlay.ForGoalieMyKickOff(SetPlay::estimate)==false){
	      count_steps-=9;
	      if(first){
		wait=9*6;
		first=false;
	      }
	      setPlay.ForGoalieMyKickOff(SetPlay::flash);
	      break;
	    }
          change_view(VW_Normal);
	}else count_steps--;
      break;
    case wait_pouse+1:
      if(Mem->CurrentTime-1==Mem->LastSightTime)
	{
          setPlay.ForGoalieMyKickOff(SetPlay::execute);
    } else count_steps--;
      break;
      //		case wait_pouse+2:

    }
  return;
}
//~//////////////////////////////////////////////////////////////////
bool Goalie::MustDestroyOpponent(){
  if(Pos.IsOffense()||Mem->BallX()>-39.0f||fabs(Mem->BallY())>Mem->SP_goal_width/2-3.0f)
  return false;
  for(int i=1;i<=Mem->SP_team_size;i++){
    if(i==Mem->MyNumber||!Mem->TeammatePositionValid(i)) continue;
    if(IsPointInCone(Mem->TeammateAbsolutePosition(i),0.5f,Vector(Mem->BallX()-7.0f,Mem->BallY()),Mem->BallAbsolutePosition())||
       Mem->TeammateDistanceToBall(i)<=2.5f){
      Mem->LogAction3(10,"MustDestroyOpponent: teammate %d block opponent",i);
      return false;
    }
  }
  Vector defendPos=GetDefendPos(GetDefendDist(Mem->BallAbsolutePosition()));
  if(fabs(Mem->MyBodyAng())>45.0f&&fabs(Mem->MyY()-defendPos.y)>0.4f){
    Mem->LogAction3(10,"MustDestroyOpponent: my init pos is wrong %.2f",(float)fabs(Mem->MyY()-defendPos.y));
    return false;
  }
  if(Mem->MyStamina()<0.5f*Mem->SP_stamina_max){
    Mem->LogAction2(10,"I`m tired to got ot ball");
    return false;
  }

  Mem->LogAction2(10,"MustDestroyOpponent: i can go to destroy opponent");
  return true;
}
//////////////////////////////////////////////////////////////////////
void Goalie::GoalieScanField() {
  if( (Mem->PlayMode==PM_Play_On||Mem->PlayMode==PM_Their_PenaltyTaken)&&Mem->LastSightTime==Mem->CurrentTime
      && !Mem->TimeToTurnForScan() ) {
    goto SCANFIELD;
  }
  if( (Mem->PlayMode==PM_Play_On||Mem->PlayMode==PM_Their_PenaltyTaken)
      &&Mem->LastSightTime==Mem->CurrentTime-1&&last_observe_time!=Mem->CurrentTime-1&&!Mem->TimeToTurnForScan() ) {
    goto SCANFIELD;
  }
  if( !Mem->TimeToTurnForScan() ) {
    Mem->LogAction2(10,"It's not a time to turn for scan");
    AngleDeg turn_angle=0;
    if( Mem->Action->valid()&&Mem->Action->type==CMD_turn )
      turn_angle=Mem->Action->angle/(1 + Mem->GetMyInertiaMoment() * Mem->MySpeed());
    if( turn_angle==0 ) {
      Mem->LogAction2(10,"There is no body turn , keep the previous view width");
      return;
    }
    Mem->LogAction2(10,"It's not a time to scan turn , but gonna turn body");
    AngleDeg neck_ang=Mem->MyNeckRelAng();
    Mem->LogAction4(10,"My neck ang is %.2f turn ang is %.2f", neck_ang, turn_angle);
    neck_ang-=turn_angle;
    Mem->LogAction3(10,"My ang will be %.2f", neck_ang);
    NormalizeAngleDeg(&neck_ang);
    if( fabs(neck_ang)<=90 ) {
      turn_neck_to_relative_angle(neck_ang);
      return;
    }
    Mem->LogAction2(10,"Eye - it's not a time scan,  but due to the body turn correction is needed");
  }
 SCANFIELD:
  last_observe_time=Mem->CurrentTime;

  Vector target;
  Vector myPos=Mem->MyPredictedPositionWithQueuedActions();

  target=Mem->BallAbsolutePosition();
  if( Mem->BallVelocityValid() )
    target+=Mem->BallAbsoluteVelocity();
	
  Vwidth v_width;
  if( myPos.dist(target)<=40 ) v_width=VW_Narrow;
  else if( myPos.dist(target)<=50 ) v_width=VW_Normal;
  else v_width=VW_Wide;

  if( Mem->ViewWidth!=v_width ) change_view(v_width);

  AngleDeg turn_angle=0;
  if( Mem->Action->valid()&&Mem->Action->type==CMD_turn )
    turn_angle=Mem->Action->angle/(1 + Mem->GetMyInertiaMoment() * Mem->MySpeed());

  if(Mem->ViewWidth==VW_Narrow&&Mem->BallPositionValid()<=0.95f&&
     (Mem->PlayMode==PM_Play_On||Mem->PlayMode==PM_Their_PenaltyTaken)&&
     (Mem->OpponentWithBall()==Unum_Unknown||(Mem->OpponentX(Mem->OpponentWithBall())>-40.0f&&Mem->PlayMode==PM_Play_On))){
    Mem->LogAction2(10,"I have Narrow view, but i not see ball in last cycle, so set view to normal");
    target=target+(Vector(-Mem->SP_pitch_length/2+11.0f,0.0f)-target).SetLength(Mem->SP_ball_speed_max);
    change_view(VW_Normal);
  }
  AngleDeg ballAng = (target-myPos).dir()-Mem->MyBodyAng()-turn_angle;
  NormalizeAngleDeg(&ballAng);
  AngleDeg angToSee=Min(90, fabs(ballAng));
  angToSee=signf(ballAng)*angToSee;

  turn_neck_to_relative_angle(angToSee);
}
//~/////////////////////////////////////////////////////////////////////////////////
bool Goalie::CanCatch(){
  return Mem->BallCatchable() && Mem->CurrentTime - last_catch_time > Mem->SP_catch_ban_cycle;
}
//~//////////////////////////////////////////////////////////////////
void Goalie::Catch(){
  Mem->LogAction2(10,"Make catch!!!");
  goalie_catch(Mem->BallAngleFromBody());
  last_catch_time=Mem->CurrentTime;
}
//~//////////////////////////////////////////////////////////////////
Vector Goalie::GetShootPoint(float side){
  const float BUFFER=3.0f;
//   if(IsBlindShoot()){
//     return Vector(Mem->MyX(),-signf(Mem->MyY())*6.0f);
//   }
  Line my_line;my_line.LineFromTwoPoints(Mem->MyPos(),Vector(Mem->MyX(),777.0f));
  Ray ball_ray(Mem->BallAbsolutePosition(),Mem->BallAbsoluteVelocity().dir());
  Vector target;
  //if kick to close coner, then use vertical line to intercept
  if(fabs(Mem->BallY())>Mem->SP_goal_width/2&&Sign(Mem->BallY())==Sign(Mem->MyY())&&
     fabs(Mem->MyY()-Mem->SP_goal_width/2)<2.0f){
    Line vert;
    vert.LineFromTwoPoints(Mem->MyPos(),Vector(-70.0f,Mem->MyY()));
    if(ball_ray.intersection(vert,&target)&&target.x<Mem->MyX()){
      Mem->LogAction2(10,"Use vertical line to intercept");
      return Vector(Max((-Mem->SP_pitch_length/2+0.5f)*side,target.x),target.y);			
    }
  }
  bool intercept=ball_ray.intersection(my_line,&target);
  if(!intercept&&fabs(Mem->BallX())>=fabs(Mem->MyX())){
    Mem->LogAction2(10,"Ball is behind me");
    intercept=true;//hack
    target=Vector(Mem->MyX(),Mem->BallPredictedPosition().y);
  }
  Line ball_line(ball_ray);
  if(fabs(ball_line.get_y((-Mem->SP_pitch_length/2)*side))>Mem->SP_goal_width/2+BUFFER){
    Mem->LogAction2(10,"Ball not go to goal");
    target=Vector(Mem->MyX(),Sign(target.y)*(Mem->SP_goal_width/2.0 + 0.5));
  }
  return target;
}
//~///////////////////////////////////////////////////////////////////
bool Goalie::IsShoot(float side){
  const float BUFFER=3.0f;
  if(!Mem->BallVelocityValid()||!Mem->BallMoving()) return false;
//   if(IsBlindShoot()){
//     Mem->LogAction2(10,"Predict blind shoot");
//     return true;
//   }
  Vector BallVel=Mem->BallAbsoluteVelocity();
  Ray ball_ray(Mem->BallAbsolutePosition(),BallVel.dir());
  Line my_line;my_line.LineFromTwoPoints(Mem->MyPos(),Vector(Mem->MyX(),777.0f));
  Vector target;
  bool intercept=ball_ray.intersection(my_line,&target);
  if(fabs(Mem->BallX())>=fabs(Mem->MyX())){
    intercept=true;
    Line goal_line;
    goal_line.LineFromTwoPoints(Vector(-Mem->SP_pitch_length/2*side,0.0),Vector(-Mem->SP_pitch_length/2*side,7.0));
    if(!ball_ray.intersection(goal_line,&target)){
      Mem->LogAction2(10,"IsShoot: ball backward to me and go to field, so go to intercept");
      return true;
    }
  }
  bool opp_inter=Pos.OppInter()&&signf(Pos.OppPoint().x-Mem->MyX())==side&&Mem->OpponentPositionValid(Pos.FastestOpp())>0.95f;
  Vector end_pos=Mem->BallAbsolutePosition()+BallVel*SumInfGeomSeries(BallVel.mod(),Mem->SP_ball_decay)/BallVel.mod();
	
  Line ball_line(ball_ray);
  bool go_to_goal=signf(end_pos.x-(-Mem->SP_pitch_length/2)*side)!=side&&
    fabs(ball_line.get_y(-Mem->SP_pitch_length/2*side))<=Mem->SP_goal_width/2+BUFFER;
  Mem->LogAction7(50,"IsShoot: intercept=%.0f; opp_inter=%.0f; end_pos=(%.2f,%.2f); go_to_goal=%.0f",
		  float(intercept),float(opp_inter),end_pos.x,end_pos.y,float(go_to_goal));
  return intercept&&!opp_inter&&go_to_goal;
}
//////////////////////////////////////////////////////////////////////
bool Goalie::IsBlindShoot()
{
  if(Mem->BallPositionValid()<=0.95f){
    Vector ball=Mem->BallAbsolutePosition(); //берем старую позицию
    if(fabs(ball.y)>=5.0f&&Mem->InOwnPenaltyArea(ball)&&Mem->CanSeePointWithNeck(ball)&&Mem->DistanceTo(ball)<=10.0f){
      return true;
    }
  }
  return false;
}
//~//////////////////////////////////////////////////////////////////
Vector Goalie::GetDefendPos(float dDist,float side,Vector posBall){
  float x= (- Mem->SP_pitch_length/2.0 + dDist)*side;
  Vector posGoalLeft ( -Mem->SP_pitch_length/2.0*side, -Mem->SP_goal_width/2.0 );
  Vector posGoalRight( -Mem->SP_pitch_length/2.0*side,  Mem->SP_goal_width/2.0 );
  Line left; left.LineFromTwoPoints( posBall, posGoalLeft  );
  Line right; right.LineFromTwoPoints( posBall, posGoalRight );
  posGoalLeft  = Vector(x,left.get_y(x));
  posGoalRight = Vector(x,right.get_y(x));
  float dDistLeft  = posGoalLeft.dist( posBall );
  float dDistRight = posGoalRight.dist( posBall );
  float dDistLine  = posGoalLeft.dist( posGoalRight );
  Vector posDefend  = posGoalLeft+ Vector( 0, (dDistLeft/(dDistLeft+dDistRight))*dDistLine);
  // do not stand further to side than goalpost
  if( fabs( posDefend.y ) > Mem->SP_goal_width/2.0f &&Mem->PlayMode!=PM_Their_PenaltyTaken)
    posDefend.y=Sign(posDefend.y)*Mem->SP_goal_width/2.0f;
  return posDefend;
}
//~//////////////////////////////////////////////////////////////////
//some idea taken from UvA_Trealern 2002
void Goalie::DefendGoalLine( float dDist,float side) {
  // determine defending point as intersection keeper line and line ball-goal
  Vector posBall    = Mem->BallAbsolutePosition(); 
  Vector posAgent   = Mem->MyPos();
  Vector posDefend=GetDefendPos(dDist,side);
  Unum   opp=Mem->ClosestOpponentToBall();
  bool   can_predict=(opp==Unum_Unknown||!Mem->BallKickableForOpponent(opp))&&Mem->BallVelocityValid();
  Vector predictBall=can_predict?Mem->BallPredictedPosition(5):posBall;
  if(DangerForCloseCorner(&predictBall,dDist,side)){
    Line l;l.LineFromTwoPoints(Vector(-Mem->SP_pitch_length/2*side,signf(predictBall.y)*Mem->SP_goal_width/2.0f),predictBall);
    float y=Sign(predictBall.y)*Mem->SP_goal_width/2;
    float one=(-Mem->SP_pitch_length/2.0f+Mem->SP_goal_area_length/2.0f)*side,two=posDefend.x;
    Vector target(MinMax(min(one,two),l.get_x(y),max(one,two)),y);
    if(Mem->DistanceTo(target)<0.3f){
      face_only_body_to_point(Vector(Mem->MyX(),signf(Mem->BallY())*Mem->SP_pitch_width));
      return;
    }
    if(target.x<Mem->MyX()&&fabs(GetNormalizeAngleDeg(Mem->MyBodyAng()-(target-Mem->MyPos()).dir()))>60.0f){
      Mem->LogAction2(10,"move backward to close corner");
      Pos.MoveBackToPos(target,20.0,Mem->SP_goal_width);
    }else{
      Mem->LogAction2(10,"move forward to close corner");
      Pos.MoveToPos(target,20.0,Mem->SP_goal_width);
    }
    return;
  }

  bool bBallInPen = Mem->InOwnPenaltyArea( posBall )||signf(posBall.x+32.0*side)!=side;

  // if too far away from line, move directly towards it
  float dDiff = ( bBallInPen == true ) ? 1.5 : 0.5;
  if( posDefend.x + dDiff < posAgent.x  )
    {
      Mem->LogAction2( 50, "move backwards to guard point" );
      Pos.MoveBackToPos( posDefend, 30, -1.0);
      return;
    }
  else if( posDefend.x - dDiff > posAgent.x )
    {
      Mem->LogAction2( 50, "move forward to guard point" );
      Pos.MoveToPos( posDefend, 30, -1.0 );
      return;		
    }
  AngleDeg angDes;
  int num_cyc;
  Vector ballEnd=Mem->BallEndPosition(&num_cyc);
  if( fabs( posBall.y - posDefend.y ) > 0.5 &&signf(ballEnd.x+30.0f*side)==side){
    Mem->LogAction3(10,"DefendGoalLine: can start turn if need, becouse ball end pos x:%.2f",ballEnd.x);
    angDes = Sign( posBall.y - posDefend.y )*90.0;
  }else if(fabs(Mem->MyBodyAng())<10.0f||fabs(GetNormalizeAngleDeg(Mem->MyBodyAng()-180))<10.0f||
	   (Mem->MySpeed()<=0.03f&&Pos.OppCycles()>4&&
	    (eye.AngleConf((Pos.TmPoint()-Mem->MyPos()).dir())>=0.74||Mem->BallVelocityValid()<0.95f))){
    Mem->LogAction2(10,"DefendGoalLine: can turn to ball,becouse it`s seems ok");
    angDes = Sign( ballEnd.y - posDefend.y )*90.0;
  }else  
    angDes = Sign( Mem->MyBodyAng() )*90.0;
  
  int iSign     = Sign( Mem->BallY() );
  if( bBallInPen )
    {
      Mem->LogAction2( 10, "move along line, with ball in penalty area" );
      Pos.MoveToPosAlongLine( posDefend, angDes, 3.0, iSign, 7.0, 12.0 );
    }
  else{
    Mem->LogAction2( 10, "move along line, with ball outside penalty area "),
      Pos.MoveToPosAlongLine( posDefend, angDes, 0.5, iSign, 2.0, 12.0 );
  }
}
//~///////////////////////////////////////////////////////////////////
void Goalie::InterceptShoot(Vector target){
  if(Mem->GoalieCloseBallInterception(Mem->BallAbsolutePosition(),Mem->BallAbsoluteVelocity()))
    return;
  if(fabs(-Mem->SP_pitch_length/2-Mem->MyX())>8.0){
    Mem->LogAction2(10,"I`m far away from keeper line");
    get_ball();
    return;
  }
  if(Mem->BallX()<=Mem->MyX()&&Mem->BallX()>-Mem->SP_pitch_length/2+GetDefendDist(Mem->BallAbsolutePosition())&&
     fabs( Mem->BallY() ) < Mem->SP_pitch_length/2 + 2.0 ){
    Mem->LogAction2(10,"ball heading and ball behind me" );
    get_ball();
    return;
  }
  if(Mem->DistanceTo(target)<0.5){
    Mem->LogAction2(10,"Already at target pos, so turn to ball");
    face_only_body_to_ball();
    return;
  }
  Pos.SetSaveStamina(false);
  //SMURF: in next condition may be error!
  if(Sign((target-Mem->MyPos()).dir())==Sign(Mem->MyBodyAng())){
    Mem->LogAction2(10,"move forward to intersection point keeperline" );
    Pos.MoveToPos(target,20.0,Mem->SP_goal_width);
  }else{
    Mem->LogAction2(10,"move backward to intersection point keeperline");
    Pos.MoveBackToPos(target,20.0,Mem->SP_goal_width);
  }
  Pos.SetSaveStamina(true);
}
//~//////////////////////////////////////////////////////////////////
float Goalie::GetDefendDist(Vector ball){
  float base=BASE;
  if(ball.x<-30.0f&&Mem->MyX()<-Mem->SP_pitch_length/2.0f+base-1.0f){
    // Mem->LogAction3(10,"GetDefndDist:modify base value = %.2f",base);
    base=Mem->MyX()+Mem->SP_pitch_length/2.0f;
    if(Pos.OppInter()&&Mem->OpponentPositionValid(Pos.FastestOpp())>0.97f&&Pos.OppCycles()>=5&&
       (fabs(Mem->MyY()-Pos.OppPoint().y)<4.0f||fabs(Pos.OppPoint().y)>10.0f||Pos.OppPoint().x>-25.0f)){
      Mem->LogAction5(10,"GetDefndDist: think, that can go out of goal OppPoint=(%.2f;%.2f); pos_val=%.2f",
		      Pos.OppPoint().x,Pos.OppPoint().y,Mem->OpponentPositionValid(Pos.FastestOpp()));
      base=BASE;
    }
  }
  return base;
}
//~//////////////////////////////////////////////////////////////////
bool Goalie::DangerForCloseCorner(Vector* ball,float dDist,float side){
  Line l;
  l.LineFromTwoPoints(*ball,Vector(-Mem->SP_pitch_length/2.0f*side,signf(ball->y)*Mem->SP_goal_width/2.0f));
  Mem->LogAction4(10,"DangerForCloseCorner: ball=(%.2f;%.2f)",
		  ball->x,ball->y);
  bool var1=false,var2_1=false,var2_2=false,var2_3=false,var2_4=false,var3_1=false,var3_2=false,var4=false;
  if(signf(ball->x+(Mem->SP_pitch_length/2-dDist)*side)!=side)
    var1=true;
  if(fabs(l.get_y((-Mem->SP_pitch_length/2+dDist)*side)-signf(ball->y)*Mem->SP_goal_width/2.0f)>Mem->SP_catch_area_l*0.9f)
    var2_1=true;
  if(fabs(ball->y)>Mem->SP_goal_width/2+Mem->SP_catch_area_l*1.1f)
    var2_2=true;
  if(signf(ball->x-(-Mem->SP_pitch_length/2+Mem->SP_penalty_area_length-5.0f)*side)!=side)
    var2_3=true;
  if(Mem->MyX()>(-Mem->SP_pitch_length/2.0f+BASE-3.5f)*side)
    var2_4=true;
  if(Mem->BallVelocityValid()<1.0f)
    var3_1=true;
  if(((Pos.OppInter()&&Pos.OppCycles()<=3)||
     (Mem->ClosestOpponentToBall()!=Unum_Unknown&&Mem->OpponentDistanceToBall(Mem->ClosestOpponentToBall())<3.5f))
    &&Mem->PlayMode==PM_Play_On)
    var3_2=true;
  if(fabs(ball->y)<20.0f)
    var4=true;
  Mem->LogAction7(10,"DangerForCloseCorner:var1=%.0f; var2_1=%.0f; var2_2=%.0f; var2_3=%.0f; var2_4=%.0f",
		  float(var1),float(var2_1),float(var2_2),float(var2_3),float(var2_4));
  Mem->LogAction5(10,"DangerForCloseCorner:var3_1=%.0f; var3_2=%.0f; var4=%.0f;",
		  float(var3_1),float(var3_2),float(var4));
  
  if((var1||(var2_1&&var2_2&&var2_3&&var2_4))&&(var3_1||var3_2)&&var4){
    if(signf(ball->x-Mem->MyX())==side||fabs(ball->y)>Mem->SP_penalty_area_width/2.0f+3.0f||
       signf(Mem->MyX()-1.0f-(-Mem->SP_pitch_length/2.0f+Mem->SP_goal_area_length/2.0f)*side)==side){//hack
      Mem->LogAction2(10,"We have danger for close corner");
      return true;
    }else{
      Mem->LogAction2(10,"DangerForCloseCorner: hack say that not danger");
    }
  }/*else{
    Line forward_l(Ray(Mem->MyPos(),Mem->MyBodyAng()));
    if((var3_1||var3_2)&&var4&&fabs(ball->y)>6.0f&&forward_l.get_x(signf(ball->y)*Mem->SP_goal_width/2.0f)<(-Mem->SP_pitch_length/2.0f+BASE-2.0f)*side){
      Mem->LogAction3(10,"DangerForCloseCorner: go without turn to CLOSE CORNER (x=%.2f)",
		      forward_l.get_x(signf(ball->y)*Mem->SP_goal_width/2.0f));
      return true;
    }
    }*/
  
  if(Pos.IsDefense()&&Pos.FastestOpp()!=Unum_Unknown&&Mem->OpponentPositionValid(Pos.FastestOpp())==1.0f&&Mem->BallVelocityValid()==1.0f){
    //проверяем на то, успеем ли мы перекрыть угол перед тем как противник схватит мяч
    int cyc;
    *ball=Mem->BallEndPosition(&cyc);
    if(fabs(ball->y)>Mem->SP_goal_width/2.0f){
      Vector target=GetDefendPos(dDist,side,*ball);
      int my_cyc=Mem->PredictedCyclesToPoint(target);
      Mem->LogAction4(10,"DangerForCloseCorner: opp cyc=%.0f; my cyc to target=%.0f",float(cyc),float(my_cyc));
      if(cyc>=2&&((Mem->PlayMode==PM_Play_On&&my_cyc>cyc)||(Mem->PlayMode!=PM_Play_On&&my_cyc>=cyc+2)))
	return true;
    }
  }
  
  return false;
}
//~//////////////////////////////////////////////////////////////////
bool Goalie::CanIntercept(){
  if(!Mem->MyInterceptionAble())
    return false;
  Vector inter=Mem->MyInterceptionPoint();
  if(!Mem->OwnPenaltyArea.shrink(Mem->SP_catch_area_l).IsWithin(inter))
    return false;
  Unum tm=Pos.FastestTm();
  if(tm!=Mem->MyNumber||Pos.TmCycles()>Pos.OppCycles()||!Mem->CanSeePointWithNeck(Pos.TmPoint()))
    return false;
  if(eye.AngleConf((Pos.TmPoint()-Mem->MyPos()).dir())<0.74&&Mem->MyX()<(-Mem->SP_pitch_length/2.0f+BASE+2.0f)){
    Mem->LogAction2(10,"CanIntercept: can not go to ball becouse pos is not passable");
    return false;
  }
  return true;
}
//~//////////////////////////////////////////////////////////////////
Vector Goalie::GetSimpleInterceptionPoint()
{
  Line my_line;
  my_line.LineFromTwoPoints(Mem->MyPos(),Vector(Mem->MyX(),777.0f));
  Ray ball_ray(Mem->BallAbsolutePosition(),Mem->BallAbsoluteVelocity().dir());
  Vector target;
  bool intercept=ball_ray.intersection(my_line,&target);
  Line body_line;
  body_line.LineFromRay(Ray(Mem->MyPos(),Mem->MyBodyAng()));
  Vector temp;
  if(ball_ray.intersection(body_line,&temp)&&(!intercept||fabs(temp.x)>fabs(target.x))){
    Mem->LogAction2(10,"GetSimpleInterceptionPoint: select intersection without turn");
    target=temp;
  }else  
    if(!intercept&&fabs(Mem->BallX())>=fabs(Mem->MyX())){
      Mem->LogAction2(10,"GetSimpleInterceptionPoint: Ball is behind me");
      intercept=true;//hack
      target=Vector(Mem->MyX(),Mem->BallPredictedPosition().y);
    }else if(!intercept){
      Mem->LogAction2(10,"GetSimpleInterceptionPoint: ball kick to wrong way");
      target=Mem->MyPos();
    }
  return target;
}
//////////////////////////////////////////////////////////////////////
void Goalie::PenaltyBehavior(float dDist,float side)
{
  Unum close_opp=Mem->ClosestOpponentToBall();
  if(Mem->GetBall()->catchable()){
    if(Mem->BallVelocityValid()<0.9f||!IsShoot(side)||
       (close_opp!=Unum_Unknown&&Mem->OpponentDistanceTo(close_opp,Mem->BallPredictedPosition())<3.0f)){
      Catch();
    }else{
      Vector target=GetShootPoint();
      float target_dash=Mem->SP_max_power*(fabs(GetNormalizeAngleDeg((target-Mem->MyPos()).dir()-Mem->MyBodyAng()))>15.0f?-1.0f:1.0f);
      Vector pred_pos=Mem->MyPredictedPosition(1,target_dash);
      Vector BallPred=Mem->BallPredictedPosition();
      if(Mem->BallDistance()>Mem->SP_catch_area_l*0.8f&&
	 (BallPred-pred_pos).mod()<Mem->SP_catch_area_l*0.8f&&BallPred.x>(-Mem->SP_pitch_length/2+0.5f)*side){
	Mem->LogAction2(10,"Catch will be better in next cycle");
	dash(target_dash);
      }else{
	Mem->LogAction4(10,"Second call catch(dist befor=%.2f; after=%.2f)",
			Mem->BallDistance(),(BallPred-pred_pos).mod());
	Catch();
      }
    }
  }else if(Mem->BallKickable()){
    Mem->LogAction2(10,"Ball is kickable, so clear ball");
    actions.clear_ball(true);
  }else if(!Mem->BallVelocityValid()){
    Mem->LogAction2(10,"PenaltyBehavior:Ball velocity not valid, so just guard line");
    DefendGoalLine(dDist,side); 
  }else if(IsShoot(side)){
    Mem->LogAction2(10,"PenaltyBehavior:was shoot");
    if(!Mem->MyInterceptionAble()){
      Vector target=GetSimpleInterceptionPoint();
      Mem->LogAction2(10,"Can not go intercept, so just go simple intercept");
      Pos.MoveToPos(target,15.0,20.0);
    }else
      get_ball();
  }else if(CanIntercept()){
    Mem->LogAction2(10,"PenaltyBehavior:I`m fastest, so go to ball");
    get_ball();
  }
  else{
    Mem->LogAction2(10,"PenaltyBehavior:Need defend goal line");
    DefendGoalLine(dDist,side); 
  }  
  Mem->LogAction2(10,"Goalie - scanning");
  GoalieScanField();
}
//////////////////////////////////////////////////////////////////////
void Goalie::SayAboutBall()
{
  if(!Mem->BallPositionValid()||!Mem->BallVelocityValid())
    return;
  char msg[10];
  Msg m(msg);
  m<<char(ST_ball_and_conf_ballvel_and_conf);
  Mem->AddBallPos(m);
  Mem->AddBallVel(m);
  Mem->SayNow(msg);
}
//////////////////////////////////////////////////////////////////////
void Goalie::Behave() {
  if( !Mem->BallPositionValid() ) {
    Mem->LogAction2(10,"Goalie - searching ball...");
    if( Mem->ViewWidth!=VW_Normal ) change_view(VW_Normal);
    turn(Mem->MyViewAngle()*2);
    Mem->SayNow(AT_ball_pos_and_vel);//ask for ball pos
    return;
  }
  kick_off();//rudiment from prevouse goalie
  if(Mem->PlayMode!=PM_Play_On){
    if(Mem->PlayMode==PM_Before_Kick_Off){
      move(-48.0f,0.0);
      return;
    }else if(Mem->PlayMode==PM_My_Goalie_Free_Kick){
      return;			
    }else if(Mem->PlayMode==PM_My_Goal_Kick){
      if(setPlay.Recovery()){
	scan_field_with_body();
	return;
      }
    }
    DefendGoalLine(GetDefendDist(Mem->BallAbsolutePosition()));
    Mem->LogAction2(10,"Goalie - scanning");
    GoalieScanField();
    return;
  }
  Unum tm=Pos.FastestTm();
  Mem->LogAction8(10,"Fastest teammate to ball: %.0f (pos conf %.2f). our cyc %.0f (x=%.2f); their cyc %.0f (opp=%.0f)",
		  float(tm),tm==Unum_Unknown?0.0f:Mem->TeammatePositionValid(tm),
		  float(Pos.TmCycles()),Pos.TmPoint().x,float(Pos.OppCycles()),float(Pos.FastestOpp()));	

  Unum close_opp=Mem->ClosestOpponentToBall();
  if(CanCatch()){
    if(Mem->BallVelocityValid()<0.9f||!IsShoot()||
       (close_opp!=Unum_Unknown&&Mem->OpponentDistanceTo(close_opp,Mem->BallPredictedPosition())<3.0f)){
      Catch();
    }else{
      Vector target=GetShootPoint();
      float target_dash=Mem->SP_max_power*(fabs(GetNormalizeAngleDeg((target-Mem->MyPos()).dir()-Mem->MyBodyAng()))>15.0f?-1.0f:1.0f);
      Vector pred_pos=Mem->MyPredictedPosition(1,target_dash);
      Vector BallPred=Mem->BallPredictedPosition();
      if(Mem->BallDistance()>Mem->SP_catch_area_l*0.8f&&
	 (BallPred-pred_pos).mod()<Mem->SP_catch_area_l*0.8f&&BallPred.x>-Mem->SP_pitch_length/2+0.5f){
	Mem->LogAction2(10,"Catch will be better in next cycle");
	dash(target_dash);
      }else{
	Mem->LogAction2(10,"Second call catch");
	Catch();
      }
    }
  }else if(Mem->BallKickable()){
    Mem->LogAction2(10,"Ball is kickable, so clear ball");
    actions.clear_ball(true);
  }else if(!Mem->BallVelocityValid()){
    Mem->LogAction2(10,"Ball velocity not valid, so just guard line");
    DefendGoalLine(GetDefendDist(Mem->BallAbsolutePosition())); 
  }else if(CanIntercept()&&Pos.TmPoint().x>Mem->MyX()+3.0f&&
	   (Pos.FastestOpp()==Unum_Unknown||Mem->OpponentPositionValid(Pos.FastestOpp())>0.97f)){
    Mem->LogAction2(10,"I`m fastest in first check, so go to ball");
    get_ball();
  }else if(IsShoot()){
    Vector target=GetShootPoint();
    Mem->LogAction4(10,"Shoot to point (%.2f,%.2f)",target.x,target.y);
    InterceptShoot(target);
  }else if(CanIntercept()){
    Mem->LogAction2(10,"I`m fastest, so go to ball");
    get_ball();
  }/*else if(MustDestroyOpponent()){
     Mem->LogAction2(10,"Go to destroy opp");
     get_ball();
     }*/
  else{
    Mem->LogAction2(10,"Need defend goal line");
    DefendGoalLine(GetDefendDist(Mem->BallAbsolutePosition())); 
  }	

  Mem->LogAction2(10,"Goalie - scanning");
  GoalieScanField();
  SayAboutBall();
  if(Mem->BallVelocityValid()<1.0f){
    Formations::Iterator iter=Pos.begin();
    Unum tm=Pos.GetPlayerNumber(iter,PT_Midfielder,Mem->GetBallSide());
    Mem->LogAction3(10,"Try liset %.0f tm (wing midfielder)",float(tm));
    if(tm!=Unum_Unknown)
      attentionto(tm);
  }
}
//~/////////////////////////////////////////////////////////////////
Goalie goalie;
//~/////////////////////////////////////////////////////////////////
