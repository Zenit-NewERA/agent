/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : test.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.10 $
 *
 *    $Id: test.C,v 2.10 2004/05/10 14:18:15 anton Exp $
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
#include "client.h"
#include "behave.h"
#include "Handleball.h"
#include "dribble.h"
#include "SetPlay.h"
#include "Formations.h"
#include "goalie.h"
#include "Playposition.h"
#include "test.h"
#include "scenario.h"
#include "pass.h"
#include "utils.h"
#include "Interception.h"
///////////////////////////////////////////////////////////
void test_through_pass(){
  my_update_offside_position();

  if (Mem->MyNumber==Mem->FP_goalie_number){	
    //		goalie.Behave();
    return;
  }
  Pos.UpdateTmPredictPos();

  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if(setPlay.Standart())
    return;
  if(Mem->MySide=='l'){
    if(!Mem->BallPositionValid()||Mem->TeammatePositionValid(3)<0.8f){
      scan_field_with_body();
      return;
    }
    if(Mem->BallKickable()){
      if(Mem->MyNumber==3){
        stop_ball();
        scan_field();
        return;
      }
      //for 2
      if(throughPass.KickInThroughPass())
        return;
      stop_ball();
      scan_field();
      return;
    }
    if(Pos.FastestTm()==Mem->MyNumber){
      if(Mem->MyNumber==3&&!Mem->BallMoving()){
        if(Mem->DistanceTo(Mem->BallAbsolutePosition())<5.0){//test
	  get_ball();
        }
        scan_field();
        return;
      }
      get_ball();
      if(Mem->MyNumber==2)
        throughPass.StartThroughPass();
      scan_field();
      return;
    }
    if(Mem->MyNumber==3){
      if(throughPass.GoInThroughPass()){
        scan_field();
        return;
      }
      scan_field_with_body();
      return;
    }
  }
}
///////////////////////////////////////////////////////////
void test_interception(){
  PlayerInterceptInfo myInfo;
  InterceptionInput data;
  int  num;
  PlayerInterceptInfo res[3];
  Vector ballVel(1.1,0.06),ballPos(35.4,23.59),playerPos(30.69,23.35),playerVel=Polar2Vector(0.27,10.74);
  float fPlayerAng=11.0;
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  move(playerPos.x,playerPos.y);
  if((Mem->MyPos()-playerPos).mod()>1.0f)
    return;
  myInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,  ballPos,ballVel,Mem->MySide,1,
				playerPos,playerVel,fPlayerAng,TRUE,TRUE);
  Mem->LogAction5(10,"ActiveCanGetThere: pos of intercept (%.6f,%.6f); cycles=%.0f",myInfo.pos.x,myInfo.pos.y,float(myInfo.numCyc));
  data.vBallPos=ballPos;data.vBallVel=ballVel;data.vPlayerPos=playerPos;data.vPlayerVel=playerVel;data.fPlayerAng=fPlayerAng;
  data.stamina=Mem->MyStamina();data.effort=Mem->MyEffort();data.recovery=Mem->MyRecovery();
   
  num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
  Mem->LogAction3(10,"Number of roots:%.0f",float(num));

  //      statistica::TimeCounter tc1,tc2;
  //      tc1.Start();
  //      for(int i=0;i<500;i++){
  //          myInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,  ballPos,ballVel,Mem->MySide,1,
  //            playerPos,playerVel,0.0f,TRUE,TRUE);
  //       }
  //      tc1.Finish();
  //      tc2.Start();
  //      for(int i=0;i<500;i++){
  //          num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
  //      }
  //      tc2.Finish();
  //      Mem->LogAction5(10,"Calc times: first=%f (cycles=%.0f), second=%f",
  //        tc1.GetTime(),float(myInfo.numCyc),tc2.GetTime());

   
//   data.vPlayerPos.x=0.0;
//   Mem->LogAction3(10,"Player x:%f",data.vPlayerPos.x);
//   num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
//   Mem->LogAction3(10,"Number of roots:%.0f",float(num));
  
//   data.vPlayerPos=playerPos;data.vBallVel.y=0;
//   Mem->LogAction3(10,"Ball vel y:%f",data.vBallVel.y);
//   num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
//   Mem->LogAction3(10,"Number of roots:%.0f",float(num));
  
//   data.vBallVel.y=2.225;
//   Mem->LogAction3(10,"Ball vel y:%f",data.vBallVel.y);
//   num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
//   Mem->LogAction3(10,"Number of roots:%.0f",float(num));
  
//   data.vBallVel.y=2.3;
//   Mem->LogAction3(10,"Ball vel y:%f",data.vBallVel.y);
//   num=Mem->GetInterceptionPoints( Mem->SP_max_power,res,data);
//   Mem->LogAction3(10,"Number of roots:%.0f",float(num));
  return;
  //~~~~~~~~~~~~~~~~~~  
  //Pos.UpdateTmPredictPos();
  change_view(VW_Narrow);
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if(setPlay.Standart())
    return;
  if(Mem->MyNumber==1){
    if(Mem->BallKickable()){
      actions.smartkick(3.0,Vector(0.0),SK_Safe);
      return;
    }
    if(!Mem->BallMoving()&&Mem->DistanceTo(Mem->BallAbsolutePosition())<10.0){
      get_ball();
    }
    
    face_only_neck_to_ball();
    return;
  }
  if(Mem->BallKickable()){
    stop_ball();
    return;
  }
  if(Mem->BallMoving())
    get_ball();
}
/////////////////////////////////////////////////////////////
void test_dribble(){

  static int dash=0;
  static int kick=0;
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if(Mem->MySide=='l'){
    if(!Mem->BallKickable()){
      Mem->LogAction2(10,"GET BALL");
      get_ball();
      scan_field();
      return;
    }
    Dribble dribble;
    //dribble.SetStrongForward();
    //        static int i=0;
    //        i+=5;
    //        kick.turnball(GetNormalizeAngleDeg(i),TURN_NONE,SK_Fast,0.5);
    //        if(dribble.BallOnTrace()){
    //            Mem->LogAction2(10,"Ball on trace!!!");
    //        }else
    //            Mem->LogAction2(10,"Ball NOT on trace!!!");

    //         if(dribble.hold_ball()){
    dribble.SetDribblePos(Vector(52.5,0.0));
    if(dribble.GoBaby(only_control_dribble)){
      if(Mem->Action->type==CMD_dash)
	dash++;
      else if(Mem->Action->type==CMD_kick)
	kick++;
      Mem->LogAction5(10,"TEST_DRIBBLE: dash/kick=%.2f (dash=%.0f; kick=%.0f)",kick==0?0.0f:float(dash)/float(kick),float(dash),float(kick));
      return;
    }else{
      stop_ball();
      scan_field();
    }

  }else{//right site
    //        if(Mem->MyNumber==1)
    //            return;
    //number 1
    if(!Mem->BallKickable()){
      //    		if(Mem->OpponentWithBall()!=Unum_Unknown){
      //    			go_to_point(Mem->MyPos(),Mem->CP_at_point_buffer);//hack: stop
      //    		}else
      get_ball();
    }else
      clear_ball();
    scan_field();
    return;

  }
}
///////////////////////////////////////////////////////////////////////
void test_formation(){
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if( setPlay.Standart()) return;
  if(Mem->MyNumber==1)
    return;
  static Formations formation;
  go_to_point(formation.GetHomePosition(),1.0);

}
//~///////////////////////////////////////////////////////////////////////////
void test_tackle(){
  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if( setPlay.Standart()) return;
  static int num=0;
  if(Mem->Tackling()){
    Mem->LogAction2(10,"I`m tackling, so wait");
    return;
  }
  if(!Pos.MoveToPos(Mem->BallAbsolutePosition()+Polar2Vector(num*0.1+0.5,num*18),5.0f,0.0)){
    if(fabs(GetNormalizeAngleDeg((Mem->BallAbsolutePosition()-Mem->MyPos()).dir()-Mem->MyBodyAng()))>5.0f){
      face_neck_and_body_to_ball();
      return;
    }
    float prob=Mem->GetTackleProb(Mem->BallAbsolutePosition(),Mem->MyPos(),Mem->MyBodyAng());
    Mem->LogAction4(10,"Tackel with probability %.4f (num=%.0f)",prob,float(num));
    tackle(50.0);
    num=(num+1)%20;
  }
  face_only_neck_to_ball();
}
//--test_filter-------------------------------------------------------------------------------------
struct holder{
  Vector oldPos;
  Vector newPos;
  Vector server;
  holder(Vector o,Vector n,Vector s):oldPos(o),newPos(n),server(s){}
  holder(){oldPos=newPos=server=Vector(777.0,777.0);}
};
  
struct destroyer{
  holder* h;
  destroyer(){
    for(int i=0;i<2;i++)
      mean[i]=div[i]=Vector(0,0);
  }
  ~destroyer(){
    ofstream r("sres.out",ios_base::app);
    for(int i=0;i<6000;i++){
      if(h[i].oldPos.x!=777.0&&h[i].oldPos.y!=777.0){
	Vector dif[2]={Vector(fabs(h[i].oldPos.x-h[i].server.x),fabs(h[i].oldPos.y-h[i].server.y)),
		       Vector(fabs(h[i].newPos.x-h[i].server.x),fabs(h[i].newPos.y-h[i].server.y))};
	//          if(dif[0].x>2.0f||dif[0].y>2.0f)
	//    	    continue;
	for(int j=0;j<2;j++){
	  mean[j]+=(dif[j]-mean[j])/(i+1);
	  div[j].x=max(div[j].x,dif[j].x);
	  div[j].y=max(div[j].y,dif[j].y);
	}
      }
    }
    r<<endl<<"---------------------------"<<endl;
    r<<"Mean: "<<mean[0]<<" "<<mean[1]<<endl;
    r<<"Div: "<<div[0]<<" "<<div[1]<<endl;
    r<<endl<<"---------------------------"<<endl;
      
  }
  Vector mean[2],div[2];

};
//~///////////////////////////////////////////////////////////////////////////
void test_filter(){
  static int i=0;
  static AngleDeg angSee;
  static bool firstMove=false;
  static Vector new_pos;
  static holder h[6000];
  static destroyer dest;
  ofstream file;
  if(firstMove)
    file.open("sres.out",ios_base::app);
  else
    file.open("sres.out",ios_base::trunc);
  dest.h=h;
  
  if(Mem->MyConf()<0.9f){
    scan_field_with_body();
    return;
  }
  change_view(VW_Wide);
  if(firstMove&&(Mem->MyPos()-new_pos).mod()<2.0f&&
     face_neck_and_body_to_point(Mem->MyPos()+Polar2Vector(10.0f,angSee))!=AQ_ActionNotQueued)
    return;
  if(!firstMove||(Mem->MyPos()-new_pos).mod()<2.0f){
    if(firstMove){
      h[i]=holder(Mem->MyPos(),Mem->MyPos()/*particlePos*/,new_pos);
      Vector dif[2]={Vector(fabs(h[i].oldPos.x-h[i].server.x),fabs(h[i].oldPos.y-h[i].server.y)),
		     Vector(fabs(h[i].newPos.x-h[i].server.x),fabs(h[i].newPos.y-h[i].server.y))};
      file<<i<<" "<<h[i].oldPos<<"\t"<<h[i].newPos<<"\t"<<h[i].server<<"\t"<<dif[0]<<"\t"<<dif[1]<<endl;
      Mem->LogAction3(10,"Write line at i=%d",i);
      i++;
    }
    do{
      new_pos=Vector(range_random(-52.5,0.0),range_random(-34.0,34.0));
    }while((Mem->MyPos()-new_pos).mod()<4.0f);
    move(new_pos.x,new_pos.y);
    firstMove=true;
    angSee=range_random(-180.0,180.0);
  }else{
    move(new_pos.x,new_pos.y);
  }
}      
//////////////////////////////////////////////////////////////////////
void test_SetPlay(){
  if (Mem->MyNumber==Mem->FP_goalie_number){	
    //		goalie.Behave();
    return;
  }

  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  //	if( actions.KickInProgress() ) {
  //		Mem->LogAction2(30, "____CONTINUE KICKING::");
  //		actions.smartkick();
  //    return;
  //  }
  if(setPlay.Standart())
    return;
  if(Mem->BallKickable())
    clear_ball();
}
//////////////////////////////////////////////////////////////////////////
void test_defense(){
  if (Mem->MyNumber==Mem->FP_goalie_number){	
    //		goalie.Behave();
    return;
  }

  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  //	if( actions.KickInProgress() ) {
  //		Mem->LogAction2(30, "____CONTINUE KICKING::");
  //		actions.smartkick();
  //    return;
  //  }
  if(setPlay.Standart())
    return;
  if(Mem->BallKickable())
    clear_ball();
  if(!Pos.MoveToPos(Vector((Mem->MyNumber-6)*5,-15.0),30.0f,3.0f)){
    Mem->LogAction2(50,"Already at target pos");
  }
}
//////////////////////////////////////////////////////////////////////////
void test_scenario(){
  if (Mem->MyNumber==Mem->FP_goalie_number){	
    //		goalie.Behave();
    return;
  }

  Pos.UpdateTmPredictPos();

  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if(setPlay.Standart())
    return;
  if(Mem->MySide=='l'){
    if( Dribble::kick_to_myself_in_progress() ){
      scan_field();
      return;
    }                 
    if(!Scenarios.ExecuteScenario()){
      if(Mem->BallKickable())
        actions.stopball();
    }
    if(Pos.FastestTm()==Mem->MyNumber)
      scan_field();
    else
      scan_field_to_ball();
    return;
  }
  Pos.defense();
}
//////////////////////////////////////////////////////////////////////////
void test_pass(){
  if (Mem->MyNumber==Mem->FP_goalie_number){	
    //		goalie.Behave();
    return;
  }
  Pos.UpdateTmPredictPos();
  PlayerInterceptInfo myInfo;

  if(!Mem->MyConf()||!Mem->BallPositionValid()){
    scan_field_with_body();
    return;
  }
  if(setPlay.Standart())
    return;
  if(Mem->MySide=='l'){
    if(!Mem->BallKickable()&&Mem->MyNumber==2)
      get_ball();
    else{
      if(!Mem->TeammatePositionValid(3)||Mem->MyNumber==3){
        scan_field_with_body();
        return;
      }
      Vector from=Mem->BallAbsolutePosition(),to=Mem->TeammateAbsolutePosition(3);
      PassSkills::PassInfo info;//=actions.GetPassInfo(3);
      Vector BallVelModif = Polar2Vector( info.GetSpeed(),(to-from).dir() );
      AngleDeg ang=-1000;
      if(info.GetOpponent()!=Unum_Unknown&&Mem->OpponentPositionValid(info.GetOpponent())){
        ang=GetDiff((to-Mem->MyPos()).dir(),
		    (Mem->OpponentAbsolutePosition(info.GetOpponent())-Mem->MyPos()).dir());
        Mem->LogAction7(10,"PassInf: conf=%.2f;opp=%.0f (opp conf %.2f;ang_dif %.2f);vel=%.2f",info.GetConfidence(),
			float(info.GetOpponent()),Mem->OpponentPositionValid(info.GetOpponent()),ang,info.GetSpeed());

	myInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,
				      from,BallVelModif,Mem->TheirSide,info.GetOpponent(),
				      Mem->OpponentAbsolutePosition(info.GetOpponent()),Vector(.0,.0),0,FALSE,FALSE);
        if(Mem->IsSuccessRes(myInfo.res) ){
          Mem->LogAction3(10,"Calc cycles: %.0f",float(myInfo.numCyc));
          Line l;l.LineFromTwoPoints(from,to);
          Mem->LogAction3(10,"Is intercept point between: %d",(int)l.InBetween(l.ProjectPoint(myInfo.pos),from,to));
        }
      }
      Mem->LogAction4(10,"Cycles: opponent %.0f; teammate %.0f",float(info.GetOpponentCycles()),float(info.GetReceiverCycles()));
      Unum opp;
      float conf=Pos.TmPassConf(3,from,BallVelModif,&opp);
      Mem->LogAction4(10,"My conf: %.2f;opp=%.0f",conf,float(opp));

      //      statistica::TimeCounter tc1,tc2;
      //      tc1.Start();
      //      for(int i=0;i<500;i++){
      //        myInfo=Mem->ActiveCanGetThere(Mem->SP_max_power,Mem->CP_max_int_lookahead,
      //    	      from,Vector(3.0f,0.0f),Mem->MySide,3,
      //            Mem->TeammateAbsolutePosition(3),Vector(.0,.0),0,FALSE,FALSE);
      //      }
      //      tc1.Finish();
      //      int cyc;
      //      tc2.Start();
      //      for(int i=0;i<500;i++){
      //        cyc=interception.cycles_to_intercept(	3,
      //   																				Mem->MySide, 		
      //   																				from,
      //   																				Vector(3.0f,0.0f));
      //      }
      //      tc2.Finish();
      //      Mem->LogAction6(10,"Calc times: first=%f (cycles=%.0f), second=%f(cycles=%.0f)",
      //        tc1.GetTime(),float(myInfo.numCyc),tc2.GetTime(),float(cyc));

    }
  }
  scan_field();
}
/////////////////////////////////////////////////////////////////////////
