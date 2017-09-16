/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : SetPlay.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.19 $
 *
 *    $Id: SetPlay.C,v 2.19 2004/08/29 14:07:21 anton Exp $
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

#include "SetPlay.h"
#include "Handleball.h"
#include "dribble.h"
#include "goalie.h"

SetPlay setPlay;

SetPlay::SetPlay(){
  IsScenario=false;
  start_penalty_mode=false;

  SP_functions[PM_My_Goalie_Free_Kick]=&Formations::GetMyGoalieFreeKickHomePos;
  SP_functions[PM_My_Kick_In]=&Formations::GetMyKickInHomePos;
  SP_functions[PM_My_Corner_Kick]=&Formations::GetCornerKickHomePos;
  SP_functions[PM_My_Goal_Kick]=&Formations::GetMyGoalieKickHomePos;
  SP_functions[PM_My_Free_Kick]=&Formations::GetMyFreeKickHomePos;
  SP_functions[PM_My_Offside_Kick]=&Formations::GetMyFreeKickHomePos;
  SP_functions[PM_My_Back_Pass]=&Formations::GetTheirSetPlayHomePos;
  SP_functions[PM_My_Free_Kick_Fault]=&Formations::GetTheirSetPlayHomePos;
  SP_functions[PM_My_Catch_Fault]=&Formations::GetTheirSetPlayHomePos;

  SP_functions[PM_Their_Goalie_Free_Kick]=&Formations::GetTheirGoalieFreeKickHomePos;
  SP_functions[PM_Their_Free_Kick]=&Formations::GetTheirSetPlayHomePos;
  SP_functions[PM_Their_Offside_Kick]=&Formations::GetTheirSetPlayHomePos;
  SP_functions[PM_Their_Goal_Kick]=&Formations::GetTheirGoalieFreeKickHomePos;
  SP_functions[PM_Their_Kick_In]=&Formations::GetTheirKickInHomePos;
  SP_functions[PM_Their_Corner_Kick]=&Formations::GetTheirCornerKickHomePos;
  SP_functions[PM_Their_Back_Pass]=&Formations::GetMyFreeKickHomePos;
  SP_functions[PM_Their_Free_Kick_Fault]=&Formations::GetMyFreeKickHomePos;
  SP_functions[PM_Their_Catch_Fault]=&Formations::GetMyFreeKickHomePos;

  functions[PM_Before_Kick_Off]=&SetPlay::BeforKickOff;
  functions[PM_Half_Time]=&SetPlay::HalfTime;

  functions[PM_My_Kick_Off]=&SetPlay::MyKickOff;
  functions[PM_My_Goalie_Free_Kick]=&SetPlay::MyGoalieFreeKick;
  functions[PM_My_Kick_In]=&SetPlay::MyKickIn;
  functions[PM_My_Corner_Kick]=&SetPlay::MyCornerKick;
  functions[PM_My_Goal_Kick]=&SetPlay::MyGoalieKick;
  functions[PM_My_Free_Kick]=&SetPlay::MyFreeKick;
  functions[PM_My_Offside_Kick]=&SetPlay::MyFreeKick;
  functions[PM_My_Back_Pass]=&SetPlay::MyFault;
  functions[PM_My_Free_Kick_Fault]=&SetPlay::MyFault;
  functions[PM_My_Catch_Fault]=&SetPlay::MyFault;
  functions[PM_My_PenaltySetup]=&SetPlay::MyPenaltySetup;
  functions[PM_My_PenaltyReady]=&SetPlay::MyPenaltyReady;
  functions[PM_My_PenaltyTaken]=&SetPlay::MyPenaltyTaken;
  functions[PM_My_PenaltyMiss]=&SetPlay::MyPenaltyScoreMiss;
  functions[PM_My_PenaltyScore]=&SetPlay::MyPenaltyScoreMiss;

  functions[PM_Their_Kick_Off]=&SetPlay::TheirKickOff;
  functions[PM_Their_Goalie_Free_Kick]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Kick_In]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Corner_Kick]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Goal_Kick]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Free_Kick]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Offside_Kick]=&SetPlay::TheirSetPlay;
  functions[PM_Their_Back_Pass]=&SetPlay::TheirFault;
  functions[PM_Their_Free_Kick_Fault]=&SetPlay::TheirFault;
  functions[PM_Their_Catch_Fault]=&SetPlay::TheirFault;
  functions[PM_Their_PenaltySetup]=&SetPlay::TheirPenaltySetup;
  functions[PM_Their_PenaltyReady]=&SetPlay::TheirPenaltyReady;
  functions[PM_Their_PenaltyTaken]=&SetPlay::TheirPenaltyTaken;
  functions[PM_Their_PenaltyMiss]=&SetPlay::TheirPenaltyScoreMiss;
  functions[PM_Their_PenaltyScore]=&SetPlay::TheirPenaltyScoreMiss;
}
////////////////////////////////////////////////////////////////////////
bool SetPlay::Standart(){
  if(Mem->PlayMode==PM_Play_On){
    ResetScenario();
    return false;
  }
  //now play one of standart situation
  FP f=functions[Mem->PlayMode];
  CurrentHomePos=SP_functions[Mem->PlayMode];
  if(f==0){
    my_error("Not know type of play mode");
    return false;
  }
  return (this->*f)();
}
///////////////////////////////////////////////////////////////////////////
bool SetPlay::BeforKickOff(){
  Mem->LogAction2(10,"In SetPlay: berfor kick off");
  Vector target=GetInitialPos();
  if(Mem->DistanceTo(target)>=2.0f){
    MoveToInitialPos();
    return true;
  }
  turn(Mem->AngleToFromBody(Vector(.0f,.0f)));
  eye.Observe();
  return true;
}
//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyKickOff(){
  Mem->LogAction2(10,"In SetPlay: My kick off");
  if(Mem->BallKickable()){
    Mem->LogAction2(10,"In SetPlay (my kick off): make kick backward");
    if(Recovery())
      return true;
    actions.smartkick(Mem->SP_ball_speed_max, 120.0f-Mem->MyBodyAng(), SK_Safe);
    return true;
  }
  Unum team=Mem->FastestTeammateToBall();
  if(team==Mem->MyNumber||team==Unum_Unknown){
    go_to_static_ball(180);
    eye.Observe();
    return true;
  }else{
    GoToHomePosition();
    eye.Observe();
    return true;
  }
}
////////////////////////////////////////////////////////////////////
bool SetPlay::TheirKickOff(){
  Mem->LogAction2(10,"In SetPlay: their kick off");
  scan_field();
  return true;
}
////////////////////////////////////////////////////////////////////
bool SetPlay::HalfTime(){
  Mem->LogAction2(10,"In SetPlay: half time");
  MoveToInitialPos();
  return true;
}
////////////////////////////////////////////////////////////////////
bool SetPlay::MyGoalieFreeKick(){
  if ( Mem->InOffsidePosition()) {//becouse in bahave() offside check after this function
    Mem->LogAction2(10, "Mode: get on side (call from SetPlay)");
    go_to_point(Vector(-50.0,Mem->MyY()),Mem->CP_at_point_buffer,GetFormationDash());
    return true;
  }
  if(GetMyType()!=PT_Midfielder){
    Mem->LogAction2(10,"SetPlay(MyGoalieFreeKick):go to default position");
    if(go_to_point(GetMyGoalieFreeKickHomePos(),1.0f,Mem->SP_max_power,DT_none)==AQ_ActionNotQueued){
      Recovery();
    }
    return true;
  }
  
  Mem->LogAction2(10,"SetPlay(MyGoalieFreeKick):go to default position");
  if(!GoToHomePosition(GetMyGoalieFreeKickHomePos())){
    face_only_body_to_ball();
    scan_field();
  }else
    scan_field();
  if(ScenarioGoing()&&GetMyType()==PT_Midfielder){
    go_to_point(Vector(Mem->MyX(),num_scenario*32.0f),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
    scan_field();
  }
  return true;
}
/////////////////////////////////////////////////////////////////////
bool SetPlay::ForGoalieMyKickOff(GoalieActions action){//return false - goalie start scenario, so must wait a little
  if(CurrentHomePos==0)//becouse goalie no call function Standart()
    CurrentHomePos=SP_functions[PM_My_Goalie_Free_Kick];

  float x=-(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length+2.0);
  static float max_conf=-1.0,ang_clear=0.0f;
  Vector opt_dot;
  float opt_dist=1000.0,ang;
  Unum opt_tm=Unum_Unknown;
  static Vector opt_target,opt_clear,from_clear;
  if(action==estimate||action==scenario_estimate||action==estimate_now){
    Iterator i=begin();
    float midfield_x=GetMyGoalieFreeKickHomePos(GetPlayerNumber(i,PT_Midfielder,PS_All)).x;
    for(float dy=-12.0;dy<=12.0;dy+=2){
      Vector clear_pos=GetOptimalPointOnLine(Vector(x,dy),Vector(midfield_x,-12),Vector(midfield_x,12),&ang);
      if(ang>ang_clear){
        ang_clear=ang;
        opt_clear=clear_pos;
        from_clear=Vector(x,dy);
      }
      Iterator iter=begin();
      while(iter!=end()){
        Unum num=GetPlayerNumber(iter,PT_Midfielder);
        if(num==Unum_Unknown||!Mem->TeammatePositionValid(num))
          break;
        if(!AtTargetPos(num)){//if at last one of tm not in right position
          if(action==estimate)
            return false;
          if(action==estimate_now)
            continue;
        }
        float conf;
        Vector pos=GetMyGoalieFreeKickHomePos(num);
        if(action==scenario_estimate){
          pos=Vector(pos.x,num_scenario*7.0+pos.y);
        }
	Unum opp=Unum_Unknown;
        if(//conf=TmPassConf(num,Mem->BallAbsolutePosition(),Polar2Vector(Mem->VelAtPt2VelAtFoot(opt_target,1.4f),
	   //						       (pos-Mem->BallAbsolutePosition()).dir()))>max_conf||
	   //Pos.DotPassConf(Vector(x,dy),pos,Polar2Vector(Mem->SP_ball_speed_max,0.0f)))>max_conf
	   (conf=actions.PassFromPoint2Point(Vector(x,dy),pos,opp))>max_conf||
	   (conf==max_conf&&(Vector(x,dy)-pos).mod()<opt_dist)){
          max_conf=conf;
          opt_dot=Vector(x,dy);
          opt_dist=(Vector(x,dy)-pos).mod();
          opt_target=pos;
          opt_tm=num;
        }
      }
    }
    Mem->LogAction6(10,"GoalieMyKickOff: estimate best kick to (%.2f,%.2f) ( tm %.0f), with conf %.2f",opt_target.x,opt_target.y,float(opt_tm),max_conf);
    if(max_conf<=0.5f||!(action!=estimate&&max_conf>=0.4f)){
      if(action==estimate){
        StartScenario(&SetPlay::SelectScenario_MyGoalieKickOff);
        return false;
      }
      if(max_conf<.5f){
	move(from_clear.x,from_clear.y);//просто выбрасываем мяч в свободную зону
	opt_target=Vector(777.0,777.0);//opt_clear;//hack
	return true;
      }
    }
    move(opt_dot.x,opt_dot.y);
    return true;
  }
  if(action==execute){
    if(opt_target.x==777.0){
      actions.clear_ball();
    }else    
      actions.smartkick(2*Mem->SP_ball_speed_max, opt_target, SK_Safe);
  }
  if(action==flash){
    max_conf=-1.0f;
    ang_clear=0.0f;
  }
  return true;
}
/////////////////////////////////////////////////////////////////////
aType SetPlay::SelectScenario_MyKickFree(){
  if(Mem->BallAbsolutePosition().y>0)
    return AT_start_set_play_scenario1;
  else
    return AT_start_set_play_scenario2;
}
//////////////////////////////////////////////////////////////////////
bool SetPlay::MyFreeKick(){
  //select who kick ball
  Vector ball=Mem->BallAbsolutePosition();
  static int pouse=0;
  int open_type=PT_Midfielder;
  int type=PT_Midfielder;
  if(ball.x<-20.0){
    type|=PT_Defender|PT_Sweeper;
    open_type|=PT_Forward;
  }
  Unum num=SelectOptimalPlayer(type,PS_All);
  if((Mem->CurrentTime-Mem->PlayModeTime)>Mem->SP_drop_ball_time*.7)//then nobody wants to go to ball
    num=Mem->FastestTeammateToBall();
  if(num==Mem->MyNumber){//make kick
    if(pouse){
      pouse--;
      scan_field();
      return true;
    }
    if(go_to_static_ball(180)){
      if(fabs(Mem->MyBodyAng())>10.0){
	face_only_body_to_point(Vector(Mem->MyX()+10.0f,Mem->MyY()));
	return true;
      }
      if(Recovery())
	return true;
      float max_conf=-1.0f;
      Vector opt_target;
      Unum opt_tm=Unum_Unknown,pl=Unum_Unknown;
      float opt_dist_to_goal=100.0f;
      Iterator iter=begin();
      while(iter!=end()){
	Unum num=GetPlayerNumber(iter,Mem->BallX()>20.0f?PT_Forward:PT_Midfielder|PT_Forward);
	if(num==Unum_Unknown)
	  break;
	if(num==Mem->MyNumber)
	  continue;
	float conf;
	Vector pos=GetMyFreeKickHomePos(num);
	if(Mem->TeammatePositionValid(num)>=0.98f)
	  pos=Mem->TeammateAbsolutePosition(num);
	if(ScenarioGoing()&&GetPlayerType(num)&open_type){
	  pos=Vector(pos.x,pos.y+num_scenario*5.0);
	}
	if((pos-ball).mod()<5.0)
	  continue;
	if((conf=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),pos,pl))>max_conf
	   ||(conf>(max_conf-0.05)&&(pos-Mem->MarkerPosition(Mem->RM_Their_Goal)).mod()<opt_dist_to_goal)){
	  max_conf=conf;
	  opt_dist_to_goal=(pos-Mem->MarkerPosition(Mem->RM_Their_Goal)).mod();
	  opt_target=pos;
	  opt_tm=num;
	}
      }
      Mem->LogAction6(10,"MyKickIn: estimate best kick to (%.2f,%.2f) ( tm %.0f), with conf %.2f",opt_target.x,opt_target.y,float(opt_tm),max_conf);
      if(max_conf<.5f){
	if(!ScenarioGoing()){
	  StartScenario(&SetPlay::SelectScenario_MyKickFree);
	  pouse=8;
	  return true;
	}
	Mem->LogAction3(10,"MyKickIn: max conf is %.2f, so clear ball",max_conf);
	actions.clear_ball();
	return true;
      }
      change_view(VW_Narrow);
      actions.smartkick(Mem->SP_ball_speed_max, opt_target, SK_SetPlay);
    }
    return true;
  }else{//without ball
    if ( Mem->InOffsidePosition()) {//becouse in bahave() offside check after this function
      if(Mem->my_offside_conf<0.96f){
        Mem->LogAction4(10,"I`m in offside, but conf is %.2f, so not shure (opp %.0f)",Mem->my_offside_conf,float(Mem->my_offside_opp));
        face_neck_to_opponent(Mem->my_offside_opp);
        return true;
      }
      Mem->LogAction2(10, "Mode: get on side (call from SetPlay)");
      go_to_point(Vector(-50.0,Mem->MyY()),Mem->CP_at_point_buffer,Mem->SP_max_power);
      return true;
    }
    if(!GoToHomePosition(GetMyFreeKickHomePos())){
      face_only_body_to_ball();
    }
    if(ScenarioGoing()&&GetMyType()&open_type){
      go_to_point(Vector(Mem->MyX(),num_scenario*32.0f),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
    }
    scan_field();
    return true;
  }
}
//////////////////////////////////////////////////////////////////////
aType SetPlay::SelectScenario_MyGoalieKickOff(){
  int top=0,bottom=0;
  for(int i=1;i<=11;i++){
    if(Mem->OpponentPositionValid(i)&&Mem->OpponentX(i)<-10.0){
      if(Mem->OpponentY(i)>0.0f)
        bottom++;
      else
        top++;
    }
  }
  if(bottom>top)
    return AT_start_set_play_scenario2;
  else
    return AT_start_set_play_scenario1;
}
///////////////////////////////////////////////////////////////////////
aType SetPlay::SelectScenario_MyKickIn(){//scenario1 - dir to opponent goal
  Vector ball=Mem->BallAbsolutePosition();
  Iterator iter=begin();
  Unum tm=Unum_Unknown;
  do{
    tm=GetPlayerNumber(iter,PT_Midfielder,PS_Center);
  }while(tm==Mem->MyNumber&&iter!=end());

  if(tm==Unum_Unknown){
    my_error("MyKickIn: i have no midfielders (maybe except me)!!!");
    return AT_start_set_play_scenario1;
  }
  if(ball.x-GetMyKickInHomePos(tm).x>15.0f)
    return  AT_start_set_play_scenario1;
  if(GetMyKickInHomePos(tm).x-ball.x>15.0f)
    return  AT_start_set_play_scenario2;
  Unum opp=Unum_Unknown;
  temp_PassConf(ball,GetMyKickInHomePos(tm),&opp);
  if(opp==Unum_Unknown){
    my_error("MyKickIn: that  suxx!!");
    return AT_start_set_play_scenario1;
  }
  Vector opp_pos=Mem->OpponentAbsolutePosition(opp);
  Mem->LogAction3(10,"MyKickIn: most danger opponent is %d",int(opp));
  float dir=(ball-GetMyKickInHomePos(tm)).dir()-(ball-opp_pos).dir();
  if(Mem->BallY()<0)
    dir=-dir;
  if(dir>0)
    return AT_start_set_play_scenario1;
  else
    return AT_start_set_play_scenario2;
}
////////////////////////////////////////////////////////////////////////
aType SetPlay::SelectScenario_MyCornerKick(){//scenario1 - dir to opponent goal
  Vector ball=Mem->BallAbsolutePosition();
  Iterator iter=begin();
  Unum tm=Unum_Unknown;
  Pside side;
  if(Mem->BallY()>0)
    side=PS_Right;
  else
    side=PS_Left;
  do{
    tm=GetPlayerNumber(iter,PT_Forward,side);
  }while(tm==Mem->MyNumber&&iter!=end());

  if(tm==Unum_Unknown){
    my_error("MyCornerKick: i have no right forwards!!!");
    return AT_start_set_play_scenario1;
  }
  Unum opp=Unum_Unknown;
  temp_PassConf(ball,GetMyKickInHomePos(tm),&opp);
  if(opp==Unum_Unknown){
    my_error("MyCornerKick: that  suxx!!");
    return AT_start_set_play_scenario1;
  }
  Vector opp_pos=Mem->OpponentAbsolutePosition(opp);
  Mem->LogAction3(10,"MyCornerKick: most danger opponent is %d",int(opp));
  Line l;
  l.LineFromTwoPoints(ball,GetMyKickInHomePos(tm));
  if(l.HalfPlaneTest(opp_pos))
    return AT_start_set_play_scenario1;
  else
    return AT_start_set_play_scenario2;
}
////////////////////////////////////////////////////////////////////////
bool SetPlay::MyKickIn(){//аут
  int role=0;//role of player in SetPlay
  static bool at_ball=false;
  Iterator iter=begin();
  Pside side=Mem->GetBallSide();
  float ang=Mem->BallY()>0?-90.0f:90.0f;
  if(GetPlayerNumber(iter,PT_Midfielder,side)==Mem->MyNumber)
    role=1;
  else
    role=0;
  if(role==0){
    if ( Mem->InOffsidePosition()) {//becouse in bahave() offside check after this function
      if(Mem->my_offside_conf<0.96f){
	Mem->LogAction4(10,"I`m in offside, but conf is %.2f, so not shure (opp %.0f)",Mem->my_offside_conf,float(Mem->my_offside_opp));
	face_neck_to_opponent(Mem->my_offside_opp);
	return true;
      }
      Mem->LogAction2(10, "Mode: get on side (call from SetPlay)");
      go_to_point(Vector(-50.0,Mem->MyY()),Mem->CP_at_point_buffer,/*Mem->GetMyStaminaIncMax()*/Mem->SP_max_power);
      return true;
    }
    if(Recovery())
      return true;
    Mem->LogAction2(10,"SetPlay(kick in):we are going to home position");
    if(!GoToHomePosition(GetMyKickInHomePos())){
      face_only_body_to_ball();
    }
    if(ScenarioGoing()&&GetMyType()==PT_Midfielder){
      go_to_point(Vector(num_scenario*52.0f,Mem->MyY()),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
    }
    scan_field();
    return true;
  }
  //go to ball and make kick
  static int pouse=0;
  if(pouse>0){//wait while going scenario
    pouse--;
    return true;
  }
  if(at_ball){//ball is at right dir and kickable
    if(fabs(Mem->MyBodyAng()-ang)>10.0f){
      turn(ang-Mem->MyBodyAng());
      face_only_neck_to_point(Vector(Mem->MyX(),Sign(ang)*1.0f));
      return true;
    }
    if(Recovery())
      return true;
    if(!(Mem->InViewAngle(Mem->LastSightTime,ang-Mem->MyNeckGlobalAng())
	 ||Mem->InViewAngle(Mem->PreviousSightTime(),ang-Mem->MyNeckGlobalAng()))){
      Mem->LogAction2(10,"MyKickIn: must see target angle");
      face_only_neck_to_point(Vector(Mem->MyX(),Sign(ang)*1.0f));
      return true;
    }
    float max_conf=-1.0f;
    Vector opt_target;
    Unum opt_tm=Unum_Unknown;
    float pos_val=-1.0;
    Iterator iter=begin();
    while(iter!=end()){
      Unum num=GetPlayerNumber(iter,PT_Midfielder|PT_Forward/*|(Mem->BallX()<-35.0f?PT_Defender|PT_Sweeper:PT_None)*/);
      if(num==Unum_Unknown)
        break;
      if(num==Mem->MyNumber||!Mem->TeammatePositionValid(num))
        continue;
      float conf;
      Vector pos=Mem->TeammateAbsolutePosition(num);
      if(ScenarioGoing()&&GetPlayerType(num)==PT_Midfielder){
        pos=Vector(GetMyKickInHomePos(num).x+num_scenario*5.0,pos.y);
      }
      Unum  opp=Unum_Unknown;
      if(!Mem->TeammateInOffsidePosition(num)&&Mem->TeammateX(num)>-20.0f&&
	 ((conf=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),pos,opp))>max_conf||
	  //TmPassConf(num,Mem->BallAbsolutePosition(),Polar2Vector(Mem->VelAtPt2VelAtFoot(opt_target,1.4f),
	  //(pos-Mem->BallAbsolutePosition()).dir()))>max_conf||
	  (conf==max_conf&&OurPositionValue(pos)>pos_val))){
	Mem->LogAction4(10,"MyKickIn: tm %.0f has conf %.4f",float(num),conf);
        max_conf=conf;
        pos_val=OurPositionValue(pos);
        opt_target=pos;
        opt_tm=num;
      }
    }
    Mem->LogAction6(10,"MyKickIn: estimate best kick to (%.2f,%.2f) ( tm %.0f), with conf %.2f",
		    opt_target.x,opt_target.y,float(opt_tm),max_conf);
    if(max_conf<=.5f){
      if(!ScenarioGoing()){
        StartScenario(&SetPlay::SelectScenario_MyKickIn);
        pouse=8;
        return true;
      }
      Mem->LogAction3(10,"MyKickIn: try to kick, to they side of field (conf=%.2f)",max_conf);
      actions.smartkick(2*Mem->SP_ball_speed_max,Vector(Mem->BallX()+10.0f,Mem->BallY()-signf(Mem->BallY())*5.0f),SK_Fast);
    }
    change_view(VW_Narrow);
    actions.smartkick(Mem->VelAtPt2VelAtFoot(opt_target,1.4f), opt_target, SK_Safe);
    at_ball=false;
    pouse=0;
    return true;
  }
  if(go_to_static_ball(ang)){
    at_ball=true;
    if(fabs(Mem->MyBodyAng()-ang)>10.0f){
      turn(ang-Mem->MyBodyAng());
      face_only_neck_to_point(Vector(Mem->MyX(),Sign(ang)*1.0f));
      return true;
    }
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyCornerKick(){
  int role=0;//role of player in SetPlay
  static bool at_ball=false;
  Iterator iter=begin();
  Pside side=Mem->GetBallSide();
  float ang=Mem->BallY()>0?-100.0f:100.0f;
  if(GetPlayerNumber(iter,PT_Midfielder,side)==Mem->MyNumber)
    role=1;
  else
    role=0;
  if(role==0){
    Mem->LogAction2(10,"SetPlay(corner kick):we are going to home position");
    if(!GoToHomePosition(GetCornerKickHomePos())){
      face_only_body_to_ball();
    }
    if(ScenarioGoing()&&GetMyType()==PT_Forward&&(GetMySide()==side||GetMySide()==PS_Center)){
      go_to_point(Vector(num_scenario*52.0f,Mem->MyY()),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
    }
    scan_field();
    return true;
  }
  //go to ball and make kick
  static int pouse=0;
  if(pouse>0){//wait while going scenario
    pouse--;
    return true;
  }
  if(at_ball){//ball is at right dir and kickable
    if(fabs(Mem->MyBodyAng()-ang)>10.0f){
      turn(ang-Mem->MyBodyAng());
      face_only_neck_to_point(Vector(.0f,.0f));
      return true;
    }
    if(Recovery())
      return true;
    if(!(Mem->InViewAngle(Mem->LastSightTime,Mem->AngleToGlobal(Vector(.0f,.0f))-Mem->MyNeckGlobalAng())
	 ||Mem->InViewAngle(Mem->PreviousSightTime(),Mem->AngleToGlobal(Vector(.0f,.0f))-Mem->MyNeckGlobalAng()))){
      Mem->LogAction2(10,"MyCornerKick: must see target angle");
      face_only_neck_to_point(Vector(.0f,.0f));
      return true;
    }
    float max_conf=-1.0f;
    Vector opt_target;
    Unum opt_tm=Unum_Unknown;
    Iterator iter=begin();
    while(iter!=end()){
      Unum num=GetPlayerNumber(iter,PT_Forward);
      if(num==Unum_Unknown)
        break;
      if(num==Mem->MyNumber||!Mem->TeammatePositionValid(num))
        continue;
      float conf;
      Vector pos=GetCornerKickHomePos(num);
      if(Mem->TeammatePositionValid(num)>=0.98f)
        pos=Mem->TeammateAbsolutePosition(num);
      if(ScenarioGoing()){
        pos=Vector(GetCornerKickHomePos(num).x+num_scenario*3.0,pos.y);
      }
      if((conf=temp_PassConf(Mem->BallAbsolutePosition(),pos))>max_conf){
        max_conf=conf;
        opt_target=pos;
        opt_tm=num;
      }
    }
    Mem->LogAction6(10,"MyCornerKick: estimate best kick to (%.2f,%.2f) ( tm %.0f), with conf %.2f",opt_target.x,opt_target.y,float(opt_tm),max_conf);
    if(max_conf<.49f&&!ScenarioGoing()){
      StartScenario(&SetPlay::SelectScenario_MyCornerKick);
      pouse=5;
      return true;
    }
    change_view(VW_Narrow);
    actions.smartkick(Mem->SP_ball_speed_max, opt_target, SK_Safe);
    at_ball=false;
    pouse=0;
    return true;
  }
  if(go_to_static_ball(ang)){
    at_ball=true;
    if(fabs(Mem->MyBodyAng()-ang)>10.0f){
      turn(ang-Mem->MyBodyAng());
      face_only_neck_to_point(Vector(0.0f,0.0f));
      return true;
    }
  }
  return true;
}
////////////////////////////////////////////////////////////////////////
void SetPlay::StartScenario(aType (SetPlay::*f)()){
  aType sc=(this->*f)();
  Mem->SayNow(sc); //start scenario

  if(sc==AT_start_set_play_scenario1){
    Mem->LogAction2(30,"Start scenario of type 1");
    BeginScenario(1);
  }else{
    Mem->LogAction2(30,"Start scenario of type 2");
    BeginScenario(-1);
  }
}
/////////////////////////////////////////////////////////////////////////////
void SetPlay::StartScenario(aType sc){
  Mem->SayNow(sc); //start scenario

  if(sc==AT_start_set_play_scenario1){
    Mem->LogAction2(30,"Start scenario of type 1");
    BeginScenario(1);
  }else{
    Mem->LogAction2(30,"Start scenario of type 2");
    BeginScenario(-1);
  }
}
/////////////////////////////////////////////////////////////////////////////
bool SetPlay::MyGoalieKick(){
  static int pouse=0;
  Unum opt_tm=SelectOptimalPlayer(PT_Sweeper|PT_Defender,PS_Center);
  if(opt_tm==Mem->MyNumber){
    if(pouse>0){//wait while scenatio going
      pouse--;
      scan_field();
      return true;
    }
    if(!go_to_static_ball(180.0f)){//go to ball
      return true;
    }
    if(fabs(Mem->MyBodyAng())>10.0){
      face_only_body_to_point(Vector(Mem->MyX()+10.0f,Mem->MyY()));
      return true;
    }
    if(Recovery())
      return true;
    Iterator iter=begin();
    float max_conf=-1.0f;
    Vector opt_target;
    Unum opt_tm=Unum_Unknown;
    while(iter!=end()){
      Unum num=GetPlayerNumber(iter,PT_Defender|PT_Midfielder,Mem->GetBallSide());
      if(num==Unum_Unknown)
	break;
      float conf;
      if(!ScenarioGoing()&&!AtTargetPos(num)){
	scan_field();
	return true;
      }
      Vector pos=(this->*CurrentHomePos)(num);
      if(ScenarioGoing()){
	pos=Vector(pos.x,pos.y+num_scenario*7.0);
      }
      if((conf=temp_PassConf(Mem->BallAbsolutePosition(),pos))>max_conf){
	max_conf=conf;
	opt_target=pos;
	opt_tm=num;
      }
    }
    Mem->LogAction6(10,"MyGoalieKick: estimate best kick to (%.2f,%.2f) ( tm %.0f), with conf %.2f",opt_target.x,opt_target.y,float(opt_tm),max_conf);
    if(max_conf<=.5f){
      if(!ScenarioGoing()){
	StartScenario(&SetPlay::SelectScenario_MyGoalieKick);
	scan_field();
	pouse=5;
	return true;
      }
      Mem->LogAction2(10,"MyGoalieKick: after scenario i have no targets, so clear ball");
      clear_ball();
      pouse=0;
      return true;
    }
    change_view(VW_Narrow);
    pouse=0;
    actions.smartkick(Mem->VelAtPt2VelAtFoot(opt_target,1.0f), opt_target, SK_Safe);
    return true;
  }else{//play without ball
    if ( Mem->InOffsidePosition()) {//becouse in bahave() offside check after this function
      Mem->LogAction2(10, "Mode: get on side (call from SetPlay)");
      go_to_point(Vector(-50.0,Mem->MyY()),Mem->CP_at_point_buffer,GetFormationDash());
      return true;
    }
    if(Mem->CurrentTime-Mem->PlayModeTime>2&&Mem->BallVelocityValid()&&FastestTm()==Mem->MyNumber&&Mem->BallSpeed()>0.2f){
      Vector pos;
      float num;
      Mem->GetClosestPointToBallPath(&pos,&num,Mem->MyPos(),Mem->BallAbsolutePosition(),Mem->BallAbsoluteVelocity());
      Mem->LogAction5(10,"MyGoalieKick: go to passive pos (%.2,%.2f) in %.0f cycles to take ball",pos.x,pos.y,float(num));
      MoveToPos(pos,5.0f,0.0);
      scan_field();
      return true;
    }
    Mem->LogAction2(10,"SetPlay(MyGoalieKick):go to default position");
    if(!GoToHomePosition(GetMyGoalieKickHomePos())){
      face_only_body_to_ball();
    }
    if(ScenarioGoing()&&(GetMyType()==PT_Midfielder||GetMyType()==PT_Defender)&&GetMySide()==Mem->GetBallSide()){
      go_to_point(Vector(Mem->MyX(),num_scenario*32.0f),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
    }
    scan_field();
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////////////////////////////
Unum SetPlay::SelectOptimalPlayer(int PT_mask,int PS_mask){//select closest teammate to ball
  Unum opt_tm=Unum_Unknown;
  float opt_dist=1000.0f;
  Iterator iter=begin();
  while(iter!=end()){
    Unum tm=GetPlayerNumber(iter,PT_mask,PS_mask);
    if(tm==Unum_Unknown)
      break;
    if((Mem->BallAbsolutePosition()-(this->*CurrentHomePos)(tm)).mod()<=opt_dist){
      opt_dist=(Mem->BallAbsolutePosition()-(this->*CurrentHomePos)(tm)).mod();
      opt_tm=tm;
    }
  }
  return opt_tm;
}
///////////////////////////////////////////////////////////////////////////////////

aType SetPlay::SelectScenario_MyGoalieKick(){
  if(Mem->BallY()>0)
    return AT_start_set_play_scenario1;
  else
    return AT_start_set_play_scenario2;
}
////////////////////////////////////////////////////////////////////////////////////
Unum SetPlay::MarkInSetPlay(Unum tm){
  Unum closestOpp[11];
  Unum ourTeam[11];
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,(this->*CurrentHomePos)(tm),closestOpp);
  for(int i=0;i<numClosestOpp;i++){
    if(!Mem->OpponentPositionValid(closestOpp[i])||
       Mem->OpponentDistanceTo(closestOpp[i],Mem->BallAbsolutePosition())<=Mem->SP_free_kick_buffer||
       Mem->OpponentDistanceTo(closestOpp[i],Mem->BallAbsolutePosition())>20.0f||
       Mem->OpponentDistanceTo(closestOpp[i],(this->*CurrentHomePos)(tm))>10.0f)
      continue;
    int numOurClosest=Mem->SortPlayersByDistanceToPoint('m' ,Mem->OpponentAbsolutePosition(closestOpp[i]),ourTeam);
    for(int j=0;j<numOurClosest;j++){
      if(ourTeam[j]==tm) //we must mark THIS player
	return  closestOpp[i];
      if(Mem->ClosestOpponentTo(Mem->MyPos())==closestOpp[i]){ //other player is closest to this player
        Mem->LogAction6(50,"MarkInSetPlay:opp %.0f(conf %.2f) must mark by tm %.0f(conf %.2f)",float(closestOpp[i]),
			Mem->OpponentPositionValid(closestOpp[i]),float(ourTeam[j]),Mem->TeammatePositionValid(ourTeam[j]));
	break;
      }
    }
  }
  return Unum_Unknown;

}
////////////////////////////////////////////////////////////////////////////////////
bool SetPlay::TheirSetPlay(){
  if(Mem->MyStamina()<Mem->SP_stamina_max*.9f/*&&(Mem->PlayMode!=PM_Their_Corner_Kick||GetMyType()>PT_Defender
					       ||Mem->MyX()-2.0f<=(this->*CurrentHomePos)(Mem->MyNumber).x)*/&&
     Mem->BallX()>-30.0f){
    Mem->LogAction4(10,"Recovery: dif time - %.2f;stamina - %.2f",float(Mem->CurrentTime-Mem->PlayModeTime),float(Mem->MyStamina()));
    face_only_body_to_ball();
    scan_field_with_neck();
    return true;
  }
  if(Mem->CurrentTime-Mem->PlayModeTime>2&&Mem->BallVelocityValid()&&FastestTm()==Mem->MyNumber&&Mem->BallSpeed()>0.2f){
    Vector pos;
    float num;
    Mem->GetClosestPointToBallPath(&pos,&num,Mem->MyPos(),Mem->BallAbsolutePosition(),Mem->BallAbsoluteVelocity());
    Mem->LogAction5(10,"TheirSetPlay: go to passive pos (%.2,%.2f) in %.0f cycles to take ball",pos.x,pos.y,float(num));
    MoveToPos(pos,5.0f,0.0);
    scan_field();
    return true;
  }
  if ( Mem->InOffsidePosition()) {//because in bahave() offside is checked after this function
    if(Mem->my_offside_conf<0.96f){
      Mem->LogAction4(10,"I`m in offside, but conf is %.2f, so not shure (opp %.0f)",Mem->my_offside_conf,float(Mem->my_offside_opp));
      face_neck_to_opponent(Mem->my_offside_opp);
      return true;
    }
    Mem->LogAction2(10, "Mode: get on side (call from SetPlay)");
    go_to_point(Vector(0.0,Mem->MyY()),Mem->CP_at_point_buffer,GetFormationDash());
    return true;
  }
  Unum mark=Mem->PlayMode==PM_Their_Corner_Kick?check_for_free_opponents_in_own_penalty_area():MarkInSetPlay(Mem->MyNumber);
  if(mark!=Unum_Unknown&&((GetPlayerType()>=PT_Midfielder&&Mem->PlayMode!=PM_Their_Corner_Kick)||
			  (Mem->PlayMode==PM_Their_Corner_Kick==GetPlayerType()<=PT_Midfielder))){
    Mem->LogAction3(10, "TheirSetPlay:Marking opponent %.0f",float(mark));
    AngleDeg  ball_ang = (Mem->BallAbsolutePosition() - Mem->OpponentAbsolutePosition(mark)).dir();
    if(MoveToPos (Mem->OpponentAbsolutePosition(mark) + Polar2Vector(2.0f, ball_ang),30.0f,3.0f))
      return true;
    if(Mem->BallAngleFromBody()>30.0f){
      Mem->LogAction3(30,"TheirSetPlay: already at pos but must turn body (ang %.2f)",Mem->BallAngleFromBody());
      face_only_body_to_ball();
    }
    return true;
  }
  if(!GoToHomePosition((this->*CurrentHomePos)(Mem->MyNumber))){
    face_only_body_to_ball();
  }
  scan_field();
  return true;
}
//////////////////////////////////////////////////////////////////////////////////////
bool SetPlay::AtTargetPos(Unum tm){
  if(!Mem->TeammatePositionValid(tm)){
    return true;//hack
  }
  Vector pos=Mem->TeammateAbsolutePosition(tm);
  float dist=2.0f;
  if(Mem->TeammateVelocityValid(tm)&&Mem->TeammateSpeed(tm)<0.1)
    dist=5.0f;

  if(pos.dist((this->*CurrentHomePos)(tm))<=dist){
    return true;
  }
  Mem->LogAction6(50,"AtTargetPos: teammate %d (conf %.2f) not at his  home position. Current dist %.2f (target d %.2f)",
		  int(tm),Mem->TeammatePositionValid(tm),pos.dist((this->*CurrentHomePos)(tm)),dist);
  return false;
}
///////////////////////////////////////////////////////////////////////////////////
bool SetPlay::Recovery(){
  if(Mem->MySpeed()>0.3f){
    Mem->LogAction2(10,"Recovery: i have big vel so stop first");
    Pos.StopNow();
    return true;
  }
  if((Mem->CurrentTime-Mem->PlayModeTime)<Mem->SP_drop_ball_time-10&&Mem->MyStamina()<Mem->SP_stamina_max*.9f){
    Mem->LogAction4(10,"Recovery: dif time - %.2f;stamina - %.2f",float(Mem->CurrentTime-Mem->PlayModeTime),float(Mem->MyStamina()));
    //scan_field_with_neck();
    change_view(VW_Wide);
    face_only_neck_to_point(Mem->MyPos()+Polar2Vector(1.0f,Mem->MyBodyAng()));
    return true;
  }
  return false;
}
/////////////////////////////////////////////////////////////////////////////////////
bool SetPlay::GoToHomePosition(Vector pos){
  const float radius=10.0f;//with buffer
  static Vector ball=Vector(-200.0f,-200.0f);
  static float conf=-1.0f;
  if((ball-Mem->BallAbsolutePosition()).mod()>1.5||Mem->BallPositionValid()>conf){//hack
    conf=Mem->BallPositionValid();
    ball=Mem->BallAbsolutePosition();
  }
  struct Info{
    bool inField;
    bool canGoToPos;
    bool iCanGo;
  };
  if(pos==Vector(-200.0f,-200.0f)){
    pos=GetHomePosition();
  }
  if(Mem->PlayMode==PM_Their_Offside_Kick||Mem->PlayMode==PM_Their_Free_Kick||Mem->PlayMode==PM_Their_Kick_In){
    Rectangle rec(ball,Vector(2*radius,2*radius));
    Line l;
    Ray r(Mem->MyPos(),pos-Mem->MyPos());
    Vector res1,res2;
    if(RayCircleIntersect(r,9.5f,ball,&res1,&res2)>0){
      Info info[4];
      if(rec.IsWithin(pos)){
        pos=rec.expand(0.5f).LeftEdge().ProjectPoint(pos);
        Mem->LogAction4(10,"GoToHomePosition: correct home pos - new pos (%.2f,%.2f)",pos.x,pos.y);
      }
      for(int i=0;i<4;i++){//fill info
        if(!Mem->FieldRectangle.IsWithin(rec.GetPoint(i))){
          info[i].inField=false;
          continue;
        }else
          info[i].inField=true;
        l.LineFromTwoPoints(rec.GetPoint(i),pos);
        if(LineCircleIntersect(l,9.5f,ball,&res1,&res2)==2)
          info[i].canGoToPos=false;
        else
          info[i].canGoToPos=true;
        l.LineFromTwoPoints(rec.GetPoint(i),Mem->MyPos());
        if(LineCircleIntersect(l,9.5f,ball,&res1,&res2)==2)
          info[i].iCanGo=false;
        else
          info[i].iCanGo=true;
      }//end of fill info
      for(int i=0;i<4;i++)
        Mem->LogAction5(10,"%.0f, %.0f, %.0f",float(info[i].inField),float(info[i].canGoToPos),float(info[i].iCanGo));
      for(int i=0;i<4;i++){
        if(!info[i].inField)
          continue;
        if(info[i].canGoToPos&&info[i].iCanGo){//we find solution, so go
          Mem->LogAction3(10,"GoToHomePos: i can avoid cycle to dot number %d",i);
          go_to_point(rec.GetPoint(i),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
          return true;
        }
      }
      if((rec.nearestEdge(Mem->MyPos())-Mem->MyPos()).mod()<=0.5f||!rec.IsWithin(Mem->MyPos())){//i`m on rectangle
        for(int i=0;i<4;i++){
          if(!info[i].inField)
            continue;
          if(info[i].canGoToPos){
            int next=(i+1)%4,prev=(i==0?3:i-1);
            if(info[next].inField&&info[next].iCanGo){
              Mem->LogAction3(10,"GoToHomePosition:i go to  forward dot %d",next);
              go_to_point(rec.GetPoint(next),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
              return true;
            }
            if(info[prev].inField&&info[prev].iCanGo){
              Mem->LogAction3(10,"GoToHomePosition:i go to  backward dot %d",prev);
              go_to_point(rec.GetPoint(prev),Mem->CP_at_point_buffer,GetFormationDash(),DT_none);
              return true;
            }
	    my_error("Error in this facking mehanism!");
          }
        }
        my_error("Not solution in GoToHomePos!!");
      }
      //go to rectangle
      Mem->LogAction2(10,"GoToHomePosition: go to rectangle edge");
      go_to_point(rec.expand(0.5f).nearestEdge(Mem->MyPos()),0.5f,50.0f,DT_none);
      return true;
    }
  }
  return Positioning::GoToHomePosition(pos);
}
//~///////////////////////////////////////////////////////////////////////
void SetPlay::InitializePenaltyMode(char side){
  start_penalty_mode=true;
  if(side=='l')
    penalty_side=-1;
  else
    penalty_side=1;
  
  if(Mem->MySide=='r')
    penalty_side*=-1;
  if(penalty_side<0){
    Rectangle rec=Mem->TheirPenaltyArea;
    Mem->TheirPenaltyArea=Mem->OwnPenaltyArea;
    Mem->OwnPenaltyArea=rec;
  }
  
      
  our_kicks=their_kicks=0;
  Mem->LogAction3(10,"SetPlay: start penalty mode with sign: %d",penalty_side);
}
//~///////////////////////////////////////////////////////////////////////
bool SetPlay::MyPenaltySetup(){
  
  if(IsMyTimeToKickPenalty()){
    
    if(Mem->DistanceTo(Vector((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0))>3.0f){
      Mem->LogAction2(10,"MyPenaltySetup: go to fixed pos there must be ball");
      Pos.MoveToPos(Vector((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0),5.0f,1.0f);
      scan_field_with_neck();
      return true;			
    }
    AngleDeg ang=penalty_side==1?180:0;
    Mem->LogAction2(10,"MyPenaltySetup: go to static ball");
    if(go_to_static_ball(ang)){
	face_neck_and_body_to_ball();
      change_view(VW_Narrow);
      return true;
    }
    face_only_neck_to_ball();
  }else
    if(Mem->FP_goalie_number==Mem->MyNumber){
      Mem->LogAction2(10,"MyPenaltySetup: go behind the goal");
      Mem->HearBall((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0,1.0f,10.0f,Mem->CurrentTime);//hack
      if(!Pos.MoveToPos(Vector((Mem->SP_pitch_length/2+2.0)*penalty_side,17.0),5.0,0.0)){
	;//face_neck_and_body_to_ball();
      }
      change_view(VW_Narrow);
    }else{
      float y=(Mem->MyNumber-6)*1.5;
      Mem->LogAction2(10,"MyPenaltySetup: go to central cicle");
      if(!Pos.MoveToPos(Vector(sqrt(64.0-Sqr(y))*penalty_side,y),5.0,0.0)){
	face_neck_and_body_to_point(Vector((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0f));
      }
    }
  return true;
}
//~///////////////////////////////////////////////////////////////////////
bool SetPlay::TheirPenaltySetup(){
  if(Mem->FP_goalie_number==Mem->MyNumber){
    Mem->LogAction2(10,"TheirPenaltySetup: i`m goalie and go to take penalty");
    float x=(Mem->SP_pitch_length/2-Mem->SP_catch_area_l*0.6)*penalty_side;
    if(Mem->SP_pen_allow_mult_kicks)
      x=(Mem->SP_pitch_length/2-MinMax(x,Mem->SP_pen_max_goalie_dist_x,11.0f))*penalty_side;

    if(!Pos.MoveToPos(Vector(x,0),5.0,0.0)){
      face_only_body_to_point(Vector(Mem->MyX(),777.0));
    }
    if(Mem->BallPositionValid()<0.95f)
      face_only_neck_to_point(Vector(-Mem->BallX(),0));//hack
    else
      face_only_neck_to_ball();
    change_view(VW_Narrow);
  }else{
    float y=(Mem->MyNumber-6)*1.5;
    Mem->LogAction2(10,"TheirPenaltySetup: go to central cicle");
    if(!Pos.MoveToPos(Vector((sqrt(64.0-Sqr(y))-1.5f)*penalty_side,y),5.0,0.0)){
      face_neck_and_body_to_point(Vector((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0f));
    }
  }
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyPenaltyReady()
{
  float CORNER=MinMax(Mem->SP_goal_width/2*0.92,
		      Mem->SP_goal_width/2-Max(fabs(Mem->BallY())*15.0f,0.2f),
		      Mem->SP_goal_width/2-0.01f);
  if(!IsMyTimeToKickPenalty()){
    return true;
  }
  if(Mem->SP_pen_allow_mult_kicks){
    Mem->LogAction2(10,"IN PEN READY: stop ball");
    actions.stopball();
    return true;
  }
  Mem->LogAction3(10,"MyPenaltyReady: select buffer =%.2f",float(Mem->SP_goal_width/2-CORNER));
  timeval tp;
  gettimeofday( &tp, NULL );
  srandom((unsigned int) tp.tv_usec);
  y_to_kick=very_random_int(2)==0?-CORNER:CORNER;
  if(Mem->TheirGoalieNum!=Unum_Unknown){
    if(Mem->OpponentPositionValid(Mem->TheirGoalieNum)<0.9f&&fabs(Mem->MyNeckRelAng())>5.0f){
      face_only_neck_to_point(Vector(52.0*penalty_side,0.0));
      Mem->LogAction2(10,"MyPenaltyReady: try to see goalie");
      return true;
    }
    if(Mem->OpponentPositionValid(Mem->TheirGoalieNum)>=0.9f&&fabs(Mem->OpponentY(Mem->TheirGoalieNum))>0.1f){
      Mem->LogAction2(10,"MyPenaltyReady: goalie may be in wrong position, so not use random function");
      y_to_kick=-Sign(Mem->OpponentY(Mem->TheirGoalieNum))*CORNER;
    }
  }
  Mem->LogAction3(10,"MyPenaltyReady: make kick to y = %.2f",y_to_kick);
  if(Mem->SP_pen_allow_mult_kicks){
    Mem->LogAction2(10,"MyPenaltyReady: pen_allow_mult_kicks is true, so use first moveball");
    actions.moveball(Vector(Mem->MyPos()+Polar2Vector(Mem->GetMyOptCtrlDist(),Mem->MyBodyAng())));
    face_only_neck_to_ball();
    return true;
  }
  Mem->LogAction2(10,"MyPenaltyReady: pen_allow_mult_kicks is false, so use simple kick");
  actions.smartkick(2*Mem->SP_ball_speed_max,Vector(Mem->SP_pitch_length/2*penalty_side,y_to_kick),SK_SetPlay);
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::TheirPenaltyReady(){
  if(Mem->FP_goalie_number!=Mem->MyNumber){
    return true;
  }
  //opponent is not kick now!
  if(Mem->BallPositionValid())
    face_only_neck_to_ball();
  else
    face_only_neck_to_point(Vector(-52.5*penalty_side,0.0f));
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyPenaltyTaken(){
  if(!IsMyTimeToKickPenalty()){
    if(Mem->ViewWidth!=VW_Narrow)
      change_view(VW_Narrow);
    face_neck_and_body_to_ball();    
    return true;
  }
  if(Mem->SP_pen_allow_mult_kicks){
    

    Unum goalie=Mem->TheirGoalieNum;
    float conf=goalie==Unum_Unknown?0.0f:Mem->OpponentPositionValid(goalie);
    Mem->LogAction6(10,"MyPenaltyTaken: goalie_num=%.0f; conf=%.2f; pos=(%.2f,%.2f)",
		    float(goalie),conf,!conf?0.0f:Mem->OpponentX(goalie),!conf?0.0f:Mem->OpponentY(goalie));
    float CORNER=6.9f;
    if(!Mem->BallKickable()){
      if( Dribble::kick_to_myself_in_progress() )
	return true;
      if(Mem->BallPositionValid()>0.9f){
	Mem->LogAction2(10,"MyPenaltyTaken: get ball");
	get_ball();
	eye.AddOpponent(Mem->TheirGoalieNum,1);
	eye.AddBall(2);
	eye.Observe();
	return true;
      }else{
	Mem->LogAction2(10,"MyPenaltyTaken: go to fixed pos there must be ball");
	Pos.MoveToPos(Vector((Mem->SP_pitch_length/2-Mem->SP_pen_dist_x)*penalty_side,0.0),5.0f,1.0f);
	scan_field_with_neck();
	return true;			
      }
    }
      
    //ball kickable
    bool active_goalie=IsGoalieActive(penalty_side);

    DribbleType dt=only_control_dribble;

    eye.AddOpponent(Mem->TheirGoalieNum,1);
    eye.AddBall(2);
    eye.Observe();

    if( actions.MyShootConf(penalty_side)>0.8f ) {
      Mem->LogAction2(10,"MyPenaltyTaken:shooting with great conf");
      actions.shoot();
      return true;
    }
    if(microKicks.CanMicroAvoidGoalie(penalty_side))
      return true;
    static Dribble dribble;
    dribble.SetDribblePos(Dribble::SelectDribbleTarget(Mem->MyPos(),Mem->MyBodyAng(),true,penalty_side)
			  /*Vector(52.0f*penalty_side,0.0f)*/);
//     float dribbleAngle=10.0f;
//     if(fabs(Mem->MyX())>fabs(30.0f*penalty_side))
    float dribbleAngle=30.0f;
    dribble.SetDribbleTurnAngleError(dribbleAngle);

    if(active_goalie)
      dribble.SetPriority(0.5f);
    if(dribble.GoBaby(dt,penalty_side)){
      Mem->LogAction2(10,"MyPenaltyTaken:with dribble go to pos (52.0,0.0)");
      return true;
    }
    dribble.SetDribbleTurnAngleError(30.0f);
    if(active_goalie)
      dribble.SetPriority(0.8f);
    if(active_goalie&&dribble.DribbleDance(penalty_side)){
      Mem->LogAction2(10,"MyPenaltyTaken:try dribble dance");
      return true;
    }
    if(microKicks.CanMicroHardestShoot(penalty_side))
      return true;
    Mem->LogAction2(10,"MyPenaltyTaken: nothing to do : shoot at end");
    float s1,s2;
    Unum o1,o2;
    float c1=actions.shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5*penalty_side,-CORNER),s1,o1,Mem->MyNumber);
    float c2=actions.shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5*penalty_side,CORNER),s2,o2,Mem->MyNumber);
    Mem->LogAction4(10,"MyPenaltyTaken: c1=%.2f; c2= %.2f",c1,c2);
    if(c2>c1){
      actions.smartkick(Mem->SP_ball_speed_max*2.0,Vector(52.5*penalty_side,6.0),SK_Safe);
    }else{
      actions.smartkick(Mem->SP_ball_speed_max*2.0,Vector(52.5*penalty_side,-6.0),SK_Safe);
    }
    return true;
  }
  Mem->LogAction3(10,"MyPenaltyTaken: make smartkick to y = %.2f",y_to_kick);
  actions.smartkick(2*Mem->SP_ball_speed_max,Vector(Mem->SP_pitch_length/2*penalty_side,y_to_kick),SK_Safe);
  face_only_neck_to_ball();
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::TheirPenaltyTaken(){
  if(Mem->FP_goalie_number!=Mem->MyNumber){
    if(Mem->ViewWidth!=VW_Narrow)
      change_view(VW_Narrow);
    face_neck_and_body_to_ball();
    return true;
  }
  //opponent start kick!!!
  static Time last_catch_time=0;
  if(!Mem->BallPositionValid()){
    Mem->LogAction2(10,"TheirPenaltyTaken: ball position not valid!!!");
    eye.SearchBall();
    return true;
  }
  if(Mem->SP_pen_allow_mult_kicks){
    float x=Mem->SP_pen_max_goalie_dist_x;
    x-=(fabs(Mem->BallAbsolutePosition().y)<=(Mem->SP_goal_width)/2+3.0f?
	0.0f:min((fabs(Mem->BallAbsolutePosition().y)-Mem->SP_goal_width/2.0f-3.0f)*0.5f,4.0f));
    x=MinMax(Mem->SP_catch_area_l*0.6,x,11.0f);
    Mem->LogAction3(10,"TheirPenaltyTaken: select x=%.2f",x);
    goalie.PenaltyBehavior( x,-penalty_side);
    return true;
  }else{
    Vector target=goalie.GetSimpleInterceptionPoint();
	
    if(Mem->BallCatchable()&&Mem->CurrentTime - last_catch_time > Mem->SP_catch_ban_cycle){
      float sign=(fabs(GetNormalizeAngleDeg((target-Mem->MyPos()).dir()-Mem->MyBodyAng()))>15.0f?-1.0f:1.0f);
      Vector pred_pos=Mem->MyPredictedPosition(1,Mem->SP_max_power*sign);
      if(Mem->BallDistance()>Mem->SP_catch_area_l*0.8f&&
	 (Mem->BallPredictedPosition()-pred_pos).mod()<=Mem->SP_catch_area_l*0.8f){
	Mem->LogAction2(10,"Catch will be better in next cycle");
	dash(Mem->SP_max_power*sign);
      }else{
	Mem->LogAction2(10,"TheirPenaltyTaken: ball is catchable, so make catch");
	goalie_catch(Mem->BallAngleFromBody());
	last_catch_time=Mem->CurrentTime;
      }
      return true;
    }
    if(Mem->BallKickable()){
      Mem->LogAction2(10,"TheirPenaltyTaken: i can`t catch ball, but can kick");
      actions.smartkick(Mem->SP_ball_speed_max,Vector(0,Mem->MyY()),SK_Fast);
      return true;
    }
    if(Mem->GetTackleProb(Mem->BallAbsolutePosition(),Mem->MyPos(),Mem->MyBodyAng())>0.5f){
      Mem->LogAction3(10,"TheirPenaltyTaken: make tackle with prob %.2f",
		      Mem->GetTackleProb(Mem->BallAbsolutePosition(),Mem->MyPos(),Mem->MyBodyAng()));
      tackle(100.0f);
      return true;
    }
    Unum opp=Mem->ClosestOpponentToBall();
    if(opp!=Unum_Unknown&&Mem->BallKickableForOpponent(opp)){
      Mem->LogAction3(10,"TheirPenaltyTaken: ball kickable for opponent %d",opp);
      //~ MoveToPos(Vector(Mem->MyX(),Mem->BallX()),15.0,10.0);
      //~ face_only_neck_to_ball();
      return true;
    }
    if(!Mem->BallVelocityValid()&&Mem->BallDistance()<11.0f){
      Mem->LogAction2(10,"TheirPenaltyTaken: ball velocity not valid");
      MoveToPos(Vector(Mem->MyX(),Mem->BallX()),15.0,10.0);
      face_only_neck_to_ball();
      return true;
    }
    Mem->LogAction4(10,"TheirPenaltyTaken: go to intercept ball to point (%.2f,%.2f)",target.x,target.y);
    MoveToPos(target,15.0,10.0);
    face_only_neck_to_ball();
    return true;
  }
  
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyPenaltyScoreMiss(){
  static Time enter=0;
  if(Mem->CurrentTime-enter>Mem->SP_pen_setup_wait){
    Mem->LogAction2(10,"MyPenaltyMissScore: we make next kick");
    enter=Mem->CurrentTime;
    our_kicks++;
  }
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::TheirPenaltyScoreMiss(){
  static Time enter=0;
  if(Mem->CurrentTime-enter>Mem->SP_pen_setup_wait){
    Mem->LogAction2(10,"TheirPenaltyMissScore: ther make next kick");
    enter=Mem->CurrentTime;
    their_kicks++;
  }
  return true;
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::MyFault(){
  Mem->LogAction2(10,"We in my fault mode");
	
  return TheirSetPlay();
}
//~//////////////////////////////////////////////////////////////////////////
bool SetPlay::TheirFault(){
  Mem->LogAction2(10,"We in their fault mode");
	
  return MyFreeKick();
}
//~//////////////////////////////////////////////////////////////////////////
