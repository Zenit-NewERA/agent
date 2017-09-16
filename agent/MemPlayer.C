/* -*- Mode: C++ -*- */
/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemPlayer.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: MemPlayer.C,v 2.7 2004/08/29 14:07:21 anton Exp $
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

/* MemPlayer.C
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

#include "types.h"
#include "MemPlayer.h"
#include "client.h"
#include "SetPlay.h"

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
const int PlayerInfo::DEFAULT_HETRO_PLAYER_TYPE_NUMBER=0;
//--------------------------------------------------------
PlayerInfo::PlayerInfo()
{
  Initialized = FALSE;
  ServerAlive = FALSE;
  CoachActive = FALSE;
  ViewQuality = VQ_High;
  ViewWidth   = VW_Normal;
  NewSight = FALSE;
  NewAction = FALSE;
  FirstActionOpSinceLastSight = FALSE;
  ClockStopped = FALSE;
  StoppedClockMSec = 0;
  LastStartClockTime = Time(-1,0); /* If a problem, change back to 0,0 */
  SecondLastStartClockTime = LastStartClockTime;
  CurrentTime = 0;
  LastSightTime = LastSoundTime = LastActionOpTime = Time(0, 0);
  PlayModeTime = 0;
  PlayMode = PM_Before_Kick_Off;

  Action = new Command;
  LastAction = new Command;
  RequestResend = FALSE;

  last_dashes = prev_dashes = dashes = 0;
  last_turns  = prev_turns  = turns  = 0;
  last_kicks  = prev_kicks  = kicks  = 0;
  last_says   = prev_says   = says   = 0;
  last_moves  = prev_moves  = moves  = 0;
  last_turn_necks   = prev_turn_necks   = turn_necks   = 0;
  lost_kicks=0;
  lost_dash=0;
  lost_turns=0;
  lost_turn_necks=0;

  last_tackles = prev_tackles = tackles = 0;
  last_pointtos = prev_pointtos = Arm.count = 0;
	
  attention_to_player = Unum_Unknown;
  tackle_expires = 0;

  TheirTeamName[0] = '\n';

  conf=0;

  body_ang = 0;
  neck_rel_ang = 0;
  SetPlay=true;//add by AI
  pass_to_myself=false;//add by AI
  time_set_pass_to_myself=-1;//add by AI
  player_that_must_see=Unum_Unknown;//add by AI
  ang_to_see=0;//add by AI
  must_see_ang=false;//add by AI
}

/*********************************************************************************/

PlayerInfo::~PlayerInfo()
{
  if (CP_save_log)
    fclose(SaveLogFile);

  if (CP_save_sound_log)
    fclose(SaveSoundLogFile);

  if (CP_save_action_log_level > 0)
    fclose(SaveActionLogFile);
  
  delete Action;
  delete LastAction;
}

/*********************************************************************************/

void PlayerInfo::Initialize()
{
  for(int i=0;i<SP_team_size*2;i++){
    players_type.push_back(DEFAULT_HETRO_PLAYER_TYPE_NUMBER);
  }
	
  TheirSide = ( MySide == 'l' ? 'r' : 'l' );
  MyTeamNameLen = strlen(MyTeamName);

  TestVersion = (VP_test || ( MySide == 'l' && VP_test_l ) || ( MySide == 'r' && VP_test_r )) ? TRUE : FALSE;
  if ( TestVersion == TRUE ) printf("%d : test version\n",MyNumber);
  if ( VP_train_DT == TRUE ) printf("%d : training DT\n",MyNumber);

  MyScore    = IP_my_score;
  TheirScore = IP_their_score;

  if (CP_save_log){
    sprintf(SaveLogFileName,"%sLog_%s_%d_%c.log",LogsDirectory,MyTeamName,(int)MyNumber,MySide);
    SaveLogFile = fopen(SaveLogFileName,"w");
    if (SaveLogFile==NULL)
      {
	my_error ("Can't open file %s, logging disabled!!!",SaveLogFileName);
	CP_save_log=FALSE;
      }
    else
      SaveLogCounter = 0;
  }

  if (CP_save_sound_log){
    sprintf(SaveSoundLogFileName,"%sLog_%s_%d_%c_sounds.log",LogsDirectory,MyTeamName,(int)MyNumber,MySide);
    SaveSoundLogFile = fopen(SaveSoundLogFileName,"w");
    if (SaveSoundLogFile==NULL)
      {
	my_error ("Can't open file %s, sound logging disabled!!!",SaveSoundLogFileName);
	CP_save_sound_log=FALSE;
      }
    else
      SaveSoundLogCounter = 0;
  }

  if (CP_save_action_log_level > 0){
    sprintf(SaveActionLogFileName,"%sLog_%s_%d_%c-actions.log",LogsDirectory,MyTeamName,(int)MyNumber,MySide);
    SaveActionLogFile = fopen(SaveActionLogFileName,"w");
    if (SaveActionLogFile==NULL)
      {
	my_error("Can't open file %s, action logging disabled!!!",SaveActionLogFileName);
	CP_save_action_log_level=0;
      }
    else
      {
	SaveActionLogCounter = 0;
      }
  }

  TimerInterval = SP_simulator_step/CP_interrupts_per_cycle;

  stamina = SP_stamina_max;
  effort = 1;
  recovery = 1;

  neck_rel_ang = 0;

  RecoveryDecThreshold = SP_stamina_max * SP_recover_dec_thr;
  EffortDecThreshold   = SP_stamina_max * SP_effort_dec_thr;
  EffortIncThreshold   = SP_stamina_max * SP_effort_inc_thr;

  my_vel_time = my_pos_time = 0;

  LastInterruptTime = 0;
  InterruptsThisCycle = 0;						  

  Initialized = TRUE;
}
//HETRO PLAYERS - by AI //////////////////////////////////////////
int PlayerInfo::SetMyPlayerType(int type){
  int temp=players_type[MyNumber-1];
  players_type[MyNumber-1]=type;
  //MUST add update for my parametrs
  return temp;
}
//-//////////////////////////////////////////////////////////////


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

#ifndef NO_ACTION_LOG

#define MAX_LOG_LINE 250

/* will be timestamped automatically */
void PlayerInfo::LogAction2(int level, const char* str)
{
  return;//TEMP
  
  if (!Initialized)
    return; /* the log files hasn't been opened yet! */

  if (level <= 0 ||
      level > CP_save_action_log_level)
    return;

  fprintf(Mem->SaveActionLogFile, "%d.%d %s%s\n", CurrentTime.t, CurrentTime.s,
	  repeat_char('-', level / 10), str);
  if (Mem->SaveActionLogCounter++ % Mem->CP_save_action_freq == 0){
    fclose(Mem->SaveActionLogFile);
    Mem->SaveActionLogFile = fopen(Mem->SaveActionLogFileName,"a");
  }

}

void PlayerInfo::LogAction3(int level, const char* str, char* param)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, param);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, char* param1, char* param2)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, param1, param2);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, char* param1, float param2)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, param1, param2);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction5(int level, const char* str, char* param1, char* param2,char* param3)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, param1, param2, param3);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction3(int level, const char* str, char c1)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, c1);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction3(int level, const char* str, float f1)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, float f1, int d1)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, d1);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, float f1, float f2)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, f2);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction5(int level, const char* str, float f1, float f2, float f3)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, f2, f3);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction6(int level, const char* str, float f1, float f2, float f3, float f4)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, f2, f3, f4);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction7(int level, const char* str, float f1, float f2, float f3, float f4, float f5)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, f2, f3, f4, f5);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction8(int level, const char* str, float f1, float f2, float f3, float f4, float f5, float f6)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, f1, f2, f3, f4, f5, f6);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction3(int level, const char* str, int d1)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, int d1, int d2)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, d2);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction4(int level, const char* str, int d1, float f1)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, f1);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction5(int level, const char* str, int d1, float f1, float f2)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, f1, f2);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction6(int level, const char* str, int d1, float f1, float f2, float f3)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, f1, f2, f3);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction7(int level, const char* str, int d1, float f1, float f2, float f3, float f4)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, f1, f2, f3, f4);
  LogAction2(level, outstring);
}

void PlayerInfo::LogAction7(int level, const char* str, int d1, int d2, float f1, float f2, float f3)
{
  char outstring[MAX_LOG_LINE];
  sprintf(outstring, str, d1, d2, f1, f2, f3);
  LogAction2(level, outstring);
}


#endif /* ifndef NO_ACTION_LOG */

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

void PlayerInfo::SetPlayMode(Pmode mode)
{
  /* If clock is starting, save the old time */
  if ( ClockStopped ){
    ClockStopped = FALSE;
    SecondLastStartClockTime = LastStartClockTime;
    LastStartClockTime = LastActionOpTime;
    StoppedClockMSec = 0;

    sanitize_time(CurrentTime);
    sanitize_time(LastSightTime);
    sanitize_time(LastSoundTime);
    sanitize_time(sense_time);
  }
  if (( PlayMode!=PM_My_Goalie_Free_Kick && PlayMode!=PM_Their_Goalie_Free_Kick)||
      (mode!=PM_My_Free_Kick&&mode!=PM_Their_Free_Kick)){
    //  if ( (PlayMode!=PM_My_Goalie_Free_Kick && PlayMode!=PM_Their_Goalie_Free_Kick) ||
    //       PlayModeTime != CurrentTime ){ /* Already set the play mode for this time */
    LastPlayMode = PlayMode;
    PlayMode = mode;
    PlayModeTime = CurrentTime;
    if(PlayMode!=LastPlayMode)
      setPlay.ResetScenario();
    if(mode==PM_Play_On){
      SetPlay=false;
    }else
      SetPlay=true;
  }

  if ( mode == PM_Before_Kick_Off || mode == PM_My_Offside_Kick || mode == PM_Their_Offside_Kick ||
       mode== PM_My_Back_Pass||mode==PM_Their_Back_Pass||mode==PM_My_Free_Kick_Fault||mode==PM_Their_Free_Kick_Fault||
       mode==PM_My_Catch_Fault||mode==PM_Their_Catch_Fault)
    {
      if ( StoppedClockMSec != 0 ) my_error("StoppedClockMSec should have been reset already");
      ClockStopped = TRUE;
    }

  if (mode == PM_Half_Time || mode == PM_Extended_Time)
    reset_stamina();
}

/*********************************************************************************/

void PlayerInfo::sanitize_time(Time &tm)
{
  if ( !LastStartClockTime ) return;

  /* This is to take care of times that were prematurely updated before we knew that
     the clock was about to start again */
  if ( tm.t == LastStartClockTime.t && tm.s > LastStartClockTime.s ){
    tm = Time( LastStartClockTime.t + (tm.s - LastStartClockTime.s), 0 );
  }
}

/*********************************************************************************/

void PlayerInfo::EstimateMyPos()
{
  /* Takes me from previous time to time */
  if ( MyConf() && MyVelConf() ){
    pos += vel;
    //    LogAction3(10,"Update filter in %s",(char*)__FUNCTION__);//TEMP
    //    updateParticleAgent(vel,true);
  }
}

/*********************************************************************************/

void PlayerInfo::EstimateMyVel(Time time)
{
  if ( my_vel_time == time && vel_conf == CP_max_conf )
    return;

  /* Takes me from previous time to time */
  if ( SensedInfoKnown(time) ){
    float old_speed = vel.mod();
    if ( old_speed )
      vel = vel * GetMySensedSpeed(time)/old_speed; /* don't change the direction */
    else
      vel = Polar2Vector( GetMySensedSpeed(time), MyBodyAng() );  /* use my direction */
    vel_conf = CP_max_conf;
  }
  else if ( vel_conf < CP_max_conf && SensedInfoKnown(time-1) ) {
    vel = Polar2Vector( GetMySensedSpeed(time-1)*GetMyPlayerDecay(), MyBodyAng() );
    vel_conf = CP_conf_decay;
  }
  else if ( !MyVelConf() )
    return;
  else if ( my_vel_time == time-1 ){
    vel *= GetMyPlayerDecay();
    vel_conf *= CP_conf_decay;
  }
  else if ( my_vel_time > time-10 ){  /* missed up to 10 cycles */
    while ( my_vel_time < time && MyVelConf() ){
      vel *= GetMyPlayerDecay();
      vel_conf *= CP_conf_decay;
      ++my_vel_time;
    }
  }
  else
    my_error("Having trouble in vel update -- must have missed at least 10 cycles %.1f %.1f    %f",
	     (float)my_vel_time.t,(float)my_vel_time.s,MyVelConf());

  my_vel_time = time;
}

/*********************************************************************************/

Vector PlayerInfo::NewVelFromDash(Vector old_vel, float dash_power)
{
  float effective_power = MyEffort() * dash_power;
  effective_power *= GetMyDashPowerRate();
  Vector new_vel = old_vel +  Polar2Vector( effective_power, MyBodyAng() );

  if ( new_vel.mod() > GetMyPlayerSpeedMax() )
    new_vel *= ( GetMyPlayerSpeedMax()/new_vel.mod() );

  return new_vel;
}

/*********************************************************************************/

void PlayerInfo::VerifyDash(float *dash_power)
{
  /* Check if recovery going down, or max_speed exceeded */

  float available_power, needed_power = *dash_power;
  if ( needed_power < 0 ){ needed_power *= -2; }
  if ( needed_power < 0 ) my_error("power should be positive now");

  float new_stamina = MyStamina() -  MyEffort() * needed_power;
  if ( new_stamina <= SP_recover_dec_thr * SP_stamina_max && recovery > SP_recover_min ){
    /* printf("%d:%d.%d ",MyNumber,CurrentTime.t,CurrentTime.s); */
    /* printf("WARNING: recovery about to go to %.3f\n",recovery - SP_recover_dec); */
    ;
  }
  if ( new_stamina <= SP_effort_dec_thr * SP_stamina_max && effort > GetMyEffortMin() ){
    /* printf("WARNING: effort about to go to %.2f\n",MyEffort() - SP_effort_dec); */
  }
  if ( new_stamina < 0 ){
    /* printf("%d:%d.%d ",MyNumber,CurrentTime.t,CurrentTime.s); */
    /* printf("WARNING: not enough stamina for dash\n"); */
    available_power = (MyStamina()+GetMyExtraStamina())/MyEffort();
    if ( *dash_power >= 0 ) { *dash_power = available_power; }
    else { *dash_power = -available_power/2; }
  }

  if ( NewVelFromDash( MyVel(), *dash_power ).mod() > GetMyPlayerSpeedMax() ){
    /* printf("%d:%d.%d ",MyNumber,CurrentTime.t,CurrentTime.s); */
    /* printf("WARNING: can't move that fast (assuming vel and dash in same dir)\n"); */
    /* printf("my speed %f   dash_power %f   ",MySpeed(),*dash_power); */
    *dash_power = signf(*dash_power)*(GetMyPlayerSpeedMax() - MySpeed())/(MyEffort()*GetMyDashPowerRate());
    /* printf("new dash_power %f\n",*dash_power); */
  }
}

/*********************************************************************************/

void PlayerInfo::UpdateFromMyAction(Time time)
{
  /* Assume vel and pos are correct for time -- going to time+1 */
  if ( !MyConf() ) my_error("Can't update from action if not localized");
  /* But I'm pretty good at estimating... up conf_decay?? */
  if ( !NewAction || !(LastActionValid(time)) ) my_error("No action at that time");

  /* AngleDeg delta_ang, expected_delta; */
  switch(LastActionType()){
  case CMD_turn:
    if ( my_pos_time > time ) break;
    /* be careful not to estimate in a turn that's already been seen --
       server updates turns instantaneously */
    /* THIS SHOULDN'T HAPPEN ANYMORE */
    /*     delta_ang = GetNormalizeAngleDeg(ang - my_last_ang); */
    /*     expected_delta = LastActionAngle()/(1.0 + GetMyInertiaMoment() * MySpeed()); */

    /* only if the change is closer to 0 than to the expected change */
    /*     if ( fabs(delta_ang) < fabs(delta_ang-expected_delta) ){ */
    /*        body_ang += expected_delta; */
    body_ang += LastActionAngle()/(1.0 + GetMyInertiaMoment() * MySpeed());
    NormalizeAngleDeg(&body_ang);
    /*     } */
    /*     else */
    /*       my_error("Turns should NOT happen instantaneously anymore"); */
    break;
  case CMD_dash:
    if ( my_vel_time > time ) break;
    vel = NewVelFromDash( vel, LastActionPower() );
    break;
  default: ;
  }
}

/*********************************************************************************/

void PlayerInfo::update_self_estimate(Time time)
{
  update_self_neck_rel_ang(time);

  if ( !MyConf() ){
    vel_conf = 0;   /* If don't know my position, can't know my velocity */
    return;
  }

  if (CP_use_new_position_based_vel) {
    if ( my_pos_time == time ){ /* just vel */
      if ( my_vel_time == time )
	return;
      if ( NewAction && LastActionValid(my_vel_time) )
	UpdateFromMyAction(my_vel_time);

      EstimateMyVel(time);
    }
  } else {
    if ( my_pos_time == time ){ /* just vel */
      if ( my_vel_time == time ) my_error("my pos and vel already updated\n");
      if ( NewAction && LastActionValid(my_vel_time) )
	UpdateFromMyAction(my_vel_time);

      EstimateMyVel(time);
    }
  }
  //last condition add by AI becouse if for example
  //my_pos_time =(4,6) and time=(5,0) will be infinity cycle
  while ( my_pos_time < time && (my_pos_time.t==time.t||(my_pos_time.t<time.t && my_pos_time.s<=time.s)) ){
    if ( NewAction && LastActionValid(my_pos_time) )
      UpdateFromMyAction(my_pos_time);

    ++my_pos_time;

    EstimateMyPos();
    EstimateMyVel(time);

    conf *= CP_conf_decay;
  }
}

/*********************************************************************************/

void PlayerInfo::update_self_neck_rel_ang(Time time){

  if ( SensedInfoKnown(time) )
    SetMyNeckRelAng(GetMySensedNeckAngle(time));
  else if ( SensedInfoKnown(time-1) ){
    /* Bring it up to date from the action */
    AngleDeg neck_ang = GetMySensedNeckAngle(time-1);
    if ( TurnNeck.valid(time-1) ){
      neck_ang += TurnNeck.angle;
      if ( neck_ang < SP_min_neck_angle )
	neck_ang = SP_min_neck_angle;
      if ( neck_ang > SP_max_neck_angle )
	neck_ang = SP_max_neck_angle;
    }
    SetMyNeckRelAng(neck_ang);
  }
  else
    /* could write an "estimate_neck_angle" that updates from the last known time */
    /* could also assume neck unchanged */
    ;/*my_error("Don't know neck angle at time %d.%d or %d.%d",
       time.t,time.s,(time-1).t,(time-1).s);*/
}


/*********************************************************************************/

void PlayerInfo::update_stamina(Time time)
{
  if ( NewAction && LastActionType() == CMD_dash )
    stamina -= (LastActionPower() > 0) ? LastActionPower() : (-2.0 * LastActionPower());

  if ( stamina < 0 ) stamina = 0;

  if ( stamina <= SP_recover_dec_thr * SP_stamina_max && recovery > SP_recover_min ) {
    recovery -= SP_recover_dec;
  }

  if ( SensedInfoKnown(time) ){
    stamina = GetMySensedStamina(time);
    effort  = GetMySensedEffort(time);
  }
  else {
    if ( stamina <= SP_effort_dec_thr * SP_stamina_max && effort > GetMyEffortMin() )
      effort -= SP_effort_dec;
    if(effort<GetMyEffortMin())
      effort=GetMyEffortMin();
    if (stamina >= SP_effort_inc_thr * SP_stamina_max && effort < GetMyEffortMax()){
      effort += SP_effort_inc;
      if ( effort > GetMyEffortMax() )
	effort = GetMyEffortMax();
    }
    stamina += recovery * GetMyStaminaIncMax();
    if ( stamina > SP_stamina_max )
      stamina = SP_stamina_max;
  }
}

/*********************************************************************************/

void PlayerInfo::reset_stamina()
{
  stamina = SP_stamina_max;
  effort = recovery = 1.0;
}

/*********************************************************************************/

Time PlayerInfo::update_time(int time)
{
  LastTime = CurrentTime;

  if ( ClockStopped ){
    if ( CurrentTime.t != time ){
      if ( CurrentTime.t == time - 1 ) /* Sometimes happens in offsides mode */
	CurrentTime = Time(time,0);
      else		// sometimes happens - needs more carefull initializing
	my_error("server time should be the same %d %d %d",CurrentTime.t, CurrentTime.s, time);
    }
    else
      CurrentTime.s = StoppedClockMSec/SP_simulator_step;
  }
  else if ( LastStartClockTime.t == time )
    CurrentTime = LastStartClockTime;
  else
    CurrentTime = Time(time,0);

  return CurrentTime;
}



/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

Bool PlayerInfo::SightPredictedEarlyThisCycle()
{
  if ( InterruptsThisCycle > CP_interrupts_per_cycle/2 )
    /* already past the beginning of the cycle */
    return FALSE;

  /* Number of full cycles since last sight * simulator_step */
  if ( MySightInterval() - ((CurrentTime-LastSightTime)-1)*SP_simulator_step <= SP_simulator_step/2 )
    return TRUE;

  return FALSE;
}

/*********************************************************************************/

Bool PlayerInfo::GotSightFromCurrentPosition()
{
  if (FirstActionOpSinceLastSight &&
      /* sight from this time or didn't change view angle last time step */
      /* could replace valids with MyNeckGlobalAng() == my_last_neck_global_ang)) */
      /* but when the angle's estimated, it might be off by up to 10 degrees */
      (LastSightTime == CurrentTime ||
       (!LastAction->valid(CurrentTime-1) && !TurnNeck.valid(CurrentTime-1))))
    return TRUE;

  return FALSE;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/

AngleDeg PlayerInfo::MyViewAngle(Time time)
{
  AngleDeg view_angle = SP_visible_angle;
  Vwidth width;

  if ( time < ViewWidthTime )
    width = LastViewWidth;
  else
    width = ViewWidth;

  if ( width == VW_Narrow ) view_angle /= 2;
  else if ( width == VW_Wide ) view_angle *= 2;

  return view_angle/2;
}

/*********************************************************************************/

Bool PlayerInfo::InViewAngle(Time time, AngleDeg ang, float buffer)
{
  if ( fabs(ang) < MyViewAngle(time) - buffer ) return TRUE;
  return FALSE;
}

/*********************************************************************************/

int PlayerInfo::MySightInterval()
{
  int interval = SP_send_step;

  if ( ViewWidth == VW_Narrow ) interval /= 2;
  else if ( ViewWidth == VW_Wide ) interval *= 2;

  if ( ViewQuality == VQ_Low ) interval /=2;

  return interval;
}

/*********************************************************************************/

int PlayerInfo::PredictedNextSightInterval()
{
  int interval = MySightInterval();
  if ( interval < SP_simulator_step )   /* 37 or 75 */
    return 1;
  if ( interval == 3*SP_simulator_step )     /* 300 */
    return 3;
  if ( interval == 1.5*SP_simulator_step )   /* 150 */
    return (LastSightInterval <= 1 ? 2 : 1);

  my_error("Sight interval should be 37, 75, 150, or 300: %d",MySightInterval());
  return 0;
}

/*********************************************************************************/
int PlayerInfo::NumCyclesToTurn(AngleDeg ang,float speed,AngleDeg body_ang){
  AngleDeg turn_ang=GetNormalizeAngleDeg(body_ang-ang);
  float sign=signf(turn_ang);
  float moment=sign<0?SP_max_moment:SP_min_moment;
  int num_cyc=0;

  do{
    turn_ang+=EffectiveTurn(moment, speed);
    speed*=GetMyPlayerDecay();
    num_cyc++;
  }while(signf(turn_ang)==sign);
  return num_cyc;
}
////////////////////////////////////////////////////////////////////////////////////

void PlayerInfo::SetMySensedInfo(float st, float e, float sp, float ha, int k, int d, int tu, int sa, int tn,
				 ArmInfo ar, int fc,int att,int tex, int tk, int m,Time ti)
{
  if ( sense_time == ti )
    return;

  prev_sense_time = sense_time;
  sense_time      = ti;

  prev_stamina    = last_stamina;
  last_stamina    = st;
  prev_effort     = last_effort;
  last_effort     = e;
  prev_speed      = last_speed;
  last_speed      = sp;
  prev_neck_rel_ang = last_neck_rel_ang;
  last_neck_rel_ang = ha;

  //  neck_rel_ang    = ha;  /** Want to do this here??? No! **/

  prev_kicks     = last_kicks;
  last_kicks     = k;
  if ( last_kicks != kicks ){
    if (!ClockStopped){
      my_error("Server missed a kick at time %d (%d %d)",prev_sense_time.t,last_kicks,kicks);
      lost_kicks++;
    }
    LastAction->type = CMD_none;
    kicks = last_kicks;
    Mem->GetBall()->forget_past_kick(LastAction->time);
  }

  prev_dashes    = last_dashes;
  last_dashes    = d;
  if ( last_dashes != dashes ){
    if (!ClockStopped){
      my_error("Server missed a dash at time %d (%d %d)",prev_sense_time.t,last_dashes,dashes);
      lost_dash++;
    }
    LastAction->type = CMD_none;
    dashes = last_dashes;
  }

  prev_turns     = last_turns;
  last_turns     = tu;
  if ( last_turns != turns ){
    if (!ClockStopped){
      my_error("Server missed a turn at time %d (%d %d)",prev_sense_time.t,last_turns,turns);
      lost_turns++;
    }
    LastAction->type = CMD_none;
    turns = last_turns;
  }

  prev_turn_necks     = last_turn_necks;
  last_turn_necks     = tn;
  if ( last_turn_necks != turn_necks ){
    if (!ClockStopped){
      my_error("Server missed a turn_neck at time %d (%d %d)",prev_sense_time.t,last_turn_necks,turn_necks);
      lost_turn_necks++;
    }
    TurnNeck.type = CMD_none;
    turn_necks = last_turn_necks;
  }
  prev_moves=last_moves;
  last_moves=m;
  if(last_moves!=moves){
    moves=last_moves;
    pos=last_move_pos;
    //    LogAction3(10,"InitParticleAgent in %s",(char*)__FUNCTION__); //TEMP
    //    InitParticlesAgent(last_move_pos);
  }
  prev_says      = last_says;
  last_says      = sa;
  if ( last_says != says ){
    says = last_says;
  }

  //////////
  attention_to_player=fc;
  prev_attentions     = last_attentions;
  last_attentions     = att;
  if ( last_attentions != attentions ){
    if (!ClockStopped)
      my_error("Server missed an attention_to at time %d (%d %d)",prev_sense_time.t,last_attentions,attentions);
    AttentionTo.type = CMD_none;
    attentions = last_attentions;
  }
  tackle_expires=tex;
  prev_tackles     = last_tackles;
  last_tackles     = tk;
  if ( last_tackles != tackles ){
    if (!ClockStopped)
      my_error("Server missed a tackle at time %d (%d %d)",prev_sense_time.t,last_tackles,tackles);
    LastAction->type = CMD_none;
    tackles = last_tackles;
  }
  Arm.movable=ar.movable;
  Arm.expires=ar.expires;
  Arm.dist=ar.dist;
  Arm.dir=ar.dir;
  prev_pointtos     = last_pointtos;
  last_pointtos     = ar.count;
  if ( last_pointtos != Arm.count ){
    if (!ClockStopped)
      my_error("Server missed a pointto at time %d (%d %d)",prev_sense_time.t,last_pointtos,Arm.count);
    LastAction->type = CMD_none;
    Arm.count = last_pointtos;
  }

}

/*********************************************************************************/

float PlayerInfo::GetMySensedSpeed(Time time){

  if (time == sense_time)
    return last_speed;
  if (time == prev_sense_time)
    return prev_speed;

  my_error("Don't know my speed at time %d",time.t);
  return 0;
}

/*********************************************************************************/

float PlayerInfo::GetMySensedStamina(Time time){

  if (time == sense_time)
    return last_stamina;
  if (time == prev_sense_time)
    return prev_stamina;

  my_error("Don't know my stamina at time %d",time.t);
  return 0;
}

/*********************************************************************************/

float PlayerInfo::GetMySensedEffort(Time time){

  if (time == sense_time)
    return last_effort;
  if (time == prev_sense_time)
    return prev_effort;

  my_error("Don't know my effort at time %d",time.t);
  return 0;
}

/*********************************************************************************/

float PlayerInfo::GetMySensedNeckAngle(Time time){

  if (time == sense_time)
    return last_neck_rel_ang;
  if (time == prev_sense_time)
    return prev_neck_rel_ang;

  my_error("Don't know my neck angle at time %d",time.t);
  return 0;
}

/*********************************************************************************/

int PlayerInfo::GetMySensedKicks(Time time){

  if (time == sense_time)
    return last_kicks;
  if (time == prev_sense_time)
    return prev_kicks;

  my_error("Don't know my kicks at time %d",time.t);
  return 0;
}

/*********************************************************************************/

int PlayerInfo::GetMySensedDashes(Time time){

  if (time == sense_time)
    return last_dashes;
  if (time == prev_sense_time)
    return prev_dashes;

  my_error("Don't know my dashes at time %d",time.t);
  return 0;
}

/*********************************************************************************/

int PlayerInfo::GetMySensedTurns(Time time){

  if (time == sense_time)
    return last_turns;
  if (time == prev_sense_time)
    return prev_turns;

  my_error("Don't know my turns at time %d",time.t);
  return 0;
}

/*********************************************************************************/

int PlayerInfo::GetMySensedSays(Time time){

  if (time == sense_time)
    return last_says;
  if (time == prev_sense_time)
    return prev_says;

  my_error("Don't know my says at time %d",time.t);
  return 0;
}
/*********************************************************************************/
int PlayerInfo::GetMySensedMoves(Time time){

  if (time == sense_time)
    return last_moves;
  if (time == prev_sense_time)
    return prev_moves;

  my_error("Don't know my moves at time %d",time.t);
  return 0;
}

/*********************************************************************************/

int PlayerInfo::GetMySensedTurnNecks(Time time){

  if (time == sense_time)
    return last_turn_necks;
  if (time == prev_sense_time)
    return prev_turn_necks;

  my_error("Don't know my turn_necks at time %d",time.t);
  return 0;
}

/*********************************************************************************/

float PlayerInfo::CorrectDashPowerForStamina(float dash_power, float stamina, float, float)
{
  float new_power;
  if (dash_power >= 0) {
    new_power = Min( dash_power, stamina-(EffortDecThreshold+CP_tired_buffer) );
    if ( new_power < 0 ) new_power = 0;
  } else {
    new_power = Min( -dash_power, stamina-(EffortDecThreshold+CP_tired_buffer) / 2.0);
    if ( new_power < 0 ) new_power = 0;

    new_power = -new_power;
  }

  return new_power;
}

/*********************************************************************************/

Bool PlayerInfo::CanFaceAngleFromNeckWithNeck(AngleDeg ang)
{
  AngleDeg total_ang = MyNeckRelAng() + ang;
  NormalizeAngleDeg(&total_ang);
  if ( total_ang > SP_min_neck_angle && total_ang < SP_max_neck_angle )
    return TRUE;
  return FALSE;
}

/*********************************************************************************/

Bool PlayerInfo::CanFaceAngleFromBodyWithNeck(AngleDeg ang)
{
  NormalizeAngleDeg(&ang);
  if ( ang > SP_min_neck_angle && ang < SP_max_neck_angle )
    return TRUE;
  return FALSE;
}

/*********************************************************************************/

Bool PlayerInfo::CanSeeAngleFromNeckWithNeck(AngleDeg ang)
{
  AngleDeg total_ang = MyNeckRelAng() + ang;
  NormalizeAngleDeg(&total_ang);
  if ( total_ang > SP_min_neck_angle - MyViewAngle() &&
       total_ang < SP_max_neck_angle + MyViewAngle() )
    return TRUE;
  return FALSE;
}

/*********************************************************************************/

Bool PlayerInfo::CanSeeAngleFromBodyWithNeck(AngleDeg ang)
{
  if (ang > 180 || ang < -180) {
    my_error("Passing unnormalized angle to CanSeeAngleFromBodyWithNeck: %.1f", ang);
    NormalizeAngleDeg(&ang);
  }
  if ( ang > SP_min_neck_angle - MyViewAngle() &&
       ang < SP_max_neck_angle + MyViewAngle() )
    return TRUE;
  return FALSE;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
void PlayerInfo::UpdatePredictedStaminaWithDash(float* pStamina, float* pEffort,
						float* pRecovery, float dash_power)
{
  if (dash_power > 0)
    *pStamina -= dash_power;
  else
    *pStamina -= 2 * dash_power;
  if (*pStamina < 0) *pStamina = 0;

  if ( *pStamina <= SP_recover_dec_thr * SP_stamina_max && *pRecovery > SP_recover_min ) {
    *pRecovery -= SP_recover_dec;
  }

  if ( *pStamina <= SP_effort_dec_thr * SP_stamina_max && *pEffort > GetMyEffortMin() )
    *pEffort -= SP_effort_dec;
  if(*pEffort<GetMyEffortMin())
    *pEffort=GetMyEffortMin();
  if (*pStamina >= SP_effort_inc_thr * SP_stamina_max && *pEffort < GetMyEffortMax()){
    *pEffort += SP_effort_inc;
    if ( *pEffort > GetMyEffortMax())
      *pEffort = GetMyEffortMax();
  }
  *pStamina += *pRecovery * GetMyStaminaIncMax();
  if ( *pStamina > SP_stamina_max )
    *pStamina = SP_stamina_max;
}

/*********************************************************************************/


Vector PlayerInfo::MyPredictedPositionAtMaxSpeed(int steps)
{
  if ( !MyConf() ) my_error("Can't estimate #DEDEDEfuture if don't know present (max speed)");

  Vector new_position = MyPos();
  Vector max_velocity = Polar2Vector(GetMyPlayerSpeedMax(),MyBodyAng());
  for (int i=0; i<steps; i++){
    new_position += max_velocity;
  }
  return new_position;
}

/*********************************************************************************/

Vector PlayerInfo::MyPredictedPositionWithTurn(float turn_ang,
					       int steps, float dash_power,
					       bool with_turn,
					       int idle_cycles)
{
  if ( !MyConf() ) my_error("Can't estimate future if don't know present");

  float curr_turn_ang = GetNormalizeAngleDeg(turn_ang);
  float corrected_dash_power = dash_power;
  float effective_power;
  float predicted_stamina = MyStamina();
  float predicted_effort = MyEffort();
  float predicted_recovery = MyRecovery();
  float myang = MyBodyAng();
  Vector position = MyPos();
  Vector velocity;
  if ( !MyVelConf() ) velocity = 0;
  else                velocity = MyVel();
  /* debug code
     cout << "steps: " << steps << "\tpow: " << dash_power << "\tmyang: " << myang
     << "\tposition: " << position << "\tvel: " << velocity
     << "\tturn?: " << turn_first << "\tturn_ang: " << turn_angle
     << "\tstam: " << predicted_stamina << "\teff: " << predicted_effort
     << "\trec: " << predicted_recovery << endl; */

  for (int i=0; i<steps; i++){
    corrected_dash_power = CorrectDashPowerForStamina(dash_power,predicted_stamina);
    /* cout << " in func: i=" << i << "\tpos" << position << endl; */
    if (i < idle_cycles) {
      /* do nothing, we're idling! */
      effective_power = 0;
    } else if (with_turn &&
	       (i==0 || curr_turn_ang != 0.0)) {
      float this_turn = MinMax(-EffectiveTurn(SP_max_moment, velocity.mod()),
			       curr_turn_ang,
			       EffectiveTurn(SP_max_moment, velocity.mod()));
      myang += this_turn;
      curr_turn_ang -= this_turn;
      effective_power = 0;
    } else if (fabs(corrected_dash_power) > predicted_stamina)
      effective_power = Sign(corrected_dash_power) * predicted_stamina ;
    else
      effective_power = corrected_dash_power;

    effective_power *= predicted_effort;
    effective_power *= GetMyDashPowerRate();
    velocity += Polar2Vector( effective_power, myang );
    /* cout << " in func: i=" << i << "\tvel" << velocity << endl; */

    if ( velocity.mod() > GetMyPlayerSpeedMax() )
      velocity *= ( GetMyPlayerSpeedMax()/velocity.mod() );

    position += velocity;
    velocity *= GetMyPlayerDecay();

    UpdatePredictedStaminaWithDash(&predicted_stamina, &predicted_effort,
				   &predicted_recovery, corrected_dash_power);

  }
  /* cout << "returning " << position << endl; */
  return position;

}

/*********************************************************************************/

Vector PlayerInfo::MyPredictedPositionWithQueuedActions()
{
  /* Only goes one step in the future so far (other function assumes repeated dashes) */
  if ( Action->valid() && Action->type == CMD_dash )
    return MyPredictedPosition(1,Action->power);
  else
    return MyPredictedPosition();
}

/*********************************************************************************/

AngleDeg PlayerInfo::MyPredictedBodyAngleWithQueuedActions()
{
  /* Only goes one step in the future so far (other function assumes repeated dashes) */
  if ( Action->valid() && Action->type == CMD_turn )
    return GetNormalizeAngleDeg(MyBodyAng() + EffectiveTurn(Action->angle));
  else
    return MyBodyAng();
}

/********************************************************************************/

AngleDeg PlayerInfo::PredictedPointRelAngFromBodyWithQueuedActions(Vector point)
{
  Vector   pred_my_pos             = MyPredictedPositionWithQueuedActions();
  AngleDeg pred_my_body_ang        = MyPredictedBodyAngleWithQueuedActions();
  Vector   pred_relative_point_pos = point - pred_my_pos;
  AngleDeg target_abs_ang          = pred_relative_point_pos.dir();
  AngleDeg target_rel_ang          = target_abs_ang - pred_my_body_ang;
  NormalizeAngleDeg(&target_rel_ang);

  return target_rel_ang;
}

/*********************************************************************************/

int PlayerInfo::PredictedCyclesToPoint(Vector pt, float dash_power)
{
  float corrected_dash_power = dash_power;
  float effective_power;
  float predicted_stamina = MyStamina();
  float predicted_effort = MyEffort();
  float predicted_recovery = MyRecovery();
  float myang = MyBodyAng();
  Vector position = MyPos();
  Vector velocity;
  if ( !MyVelConf() ) velocity = 0;
  else                velocity = MyVel();

  for (int i=0; TRUE; i++) {
    if (position.dist(pt) <= CP_at_point_buffer)
      return i;

    /* decide if we should turn */
    float targ_ang = (pt-position).dir() - myang;
    if (fabs(GetNormalizeAngleDeg(targ_ang)) > CP_max_go_to_point_angle_err) {
      /* turning */
      float this_turn = MinMax(-EffectiveTurn(SP_max_moment, velocity.mod()),
			       targ_ang,
			       EffectiveTurn(SP_max_moment, velocity.mod()));
      myang += this_turn;
      corrected_dash_power = 0; //so that stamina is updated correctly
    } else {
      /* dashing */
      corrected_dash_power = CorrectDashPowerForStamina(dash_power,predicted_stamina);
      if (fabs(corrected_dash_power) > predicted_stamina)
	effective_power = Sign(corrected_dash_power) * predicted_stamina ;
      else
	effective_power = corrected_dash_power;

      effective_power *= predicted_effort;
      effective_power *= GetMyDashPowerRate();
      velocity += Polar2Vector( effective_power, myang );
    }

    if ( velocity.mod() >GetMyPlayerSpeedMax() )
      velocity *= ( GetMyPlayerSpeedMax()/velocity.mod() );

    position += velocity;
    velocity *= GetMyPlayerDecay();

    UpdatePredictedStaminaWithDash(&predicted_stamina, &predicted_effort,
				   &predicted_recovery, corrected_dash_power);

  }

}

/*********************************************************************************/
int PlayerInfo::NumTurnsToAngle(float targ_body_ang, float curr_body_ang, float curr_speed)
{
  int steps;

  NormalizeAngleDeg(&targ_body_ang);
  NormalizeAngleDeg(&curr_body_ang);

  for (steps = 0;
       fabs(targ_body_ang - curr_body_ang) > CP_max_go_to_point_angle_err;
       steps++) {
    AngleDeg this_turn = targ_body_ang - curr_body_ang;
    NormalizeAngleDeg(&this_turn);
    this_turn = signf(this_turn)*Min(fabs(this_turn), MaxEffectiveTurn(curr_speed));
    Mem->LogAction5(210, "NumTurnsToAngle: curr: %.1f  targ: %.1f  turn: %.1f",
		    curr_body_ang, targ_body_ang, this_turn);
    curr_body_ang += this_turn;
    NormalizeAngleDeg(&curr_body_ang);
    curr_speed *= GetMyPlayerDecay();//AI: may be must SP_player_decay?
  }

  return steps;
}
/*********************************************************************************/
//AI: particle filter/////////////////////////////////////
void PlayerInfo::InitParticlesAgent(Vector pos){
  for(int i=0;i<numParticlesPosAgent;i++)
    particlesPosAgent[i]=pos;
}

////////////////////////////////////////////////////////
void PlayerInfo::updateParticleAgent(Vector vel,bool bAfterSense){
  static Vector old_vel;
  for(int i=0;i<numParticlesPosAgent;i++){
    if(!bAfterSense)
      particlesPosAgent[i]-=old_vel;
    particlesPosAgent[i]+=vel;
  }
  old_vel=vel;
}
//////////////////////////////////////////////////////////
Vector PlayerInfo::averageParticles(Vector array[],int size){
  double x=0,y=0;
  if(size==0)
    return Vector(777.0,777.0);

  for(int i=0;i<size;i++){
    x+=array[i].x;
    y+=array[i].y;
  }
  return Vector(x/size,y/size);

}
/////////////////////////////////////////////////////////
void PlayerInfo::resampleParticleInfo(int first){
  for(int i=first;i<numParticlesPosAgent;i++)
    particlesPosAgent[i]=particlesPosAgent[(int)(drand48()*first)];
}
/////////////////////////////////////////////////////////
bool PlayerInfo::getMinMaxDistQuantizeValue( double fOutput, double *fMin,
					     double *fMax,   double x1, double x2 )
{
  fOutput -= 1.0e-10;
  *fMin = exp( invQuantizeMin( log( invQuantizeMin(fOutput,x2) ), x1 ) );
  fOutput += 2.0e-10;
  *fMax = exp( invQuantizeMax( log( invQuantizeMax(fOutput,x2) ), x1 ) );
  return true;
}
///////////////////////////////////////////////////////////
double PlayerInfo::invQuantizeMin( double fOutput, double fQuantizeStep )
{
  return (rint( fOutput / fQuantizeStep )-0.5 )*fQuantizeStep;
}
//////////////////////////////////////////////////////////////////////////
double PlayerInfo::invQuantizeMax( double fOutput, double fQuantizeStep )
{
  return (rint( fOutput/fQuantizeStep) + 0.5 )*fQuantizeStep;
}

