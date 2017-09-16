/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemPlayer.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.5 $
 *
 *    $Id: MemPlayer.h,v 2.5 2004/03/05 07:03:44 anton Exp $
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
/* -*- Mode: C++ -*- */

/* MemPlayer.h
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

#ifndef __player_h
#define __player_h

#include "netif.h"
#include "types.h"
#include "MemOption.h"
#include "utils.h"
#include "geometry.h"
#include <vector>

class Command {
public:

  CMDType type;
  
  float power;
  float angle;
  float x;
  float y;
  Vqual qual;
  Vwidth width;
  Unum player;

  char  command[MAXMESG];
  Time  time;

  Command() { type = CMD_none; }

  inline Bool valid() { return (Bool) (type != CMD_none); }
  inline Bool valid(Time t) { return (Bool) (type != CMD_none && t == time); }
};


struct ArmInfo
{
  int movable;
  int expires;
  float dist;
  AngleDeg dir;
  int count;
};

/*****************************************/
/*****************************************/
/*****************************************/

class PlayerInfo : public OptionInfo{
public:

  PlayerInfo();
  virtual ~PlayerInfo();
  void Initialize();

  void SetPlayMode(Pmode mode);
  void sanitize_time(Time &tm);

  void EstimateMyPos();
  void EstimateMyVel(Time time);
  Vector NewVelFromDash(Vector old_vel, float dash_power);
  void UpdateFromMyAction(Time time);
  void update_self_estimate(Time time);
  void update_self_neck_rel_ang(Time time);
  void update_stamina(Time time);
  void reset_stamina();
  Time update_time(int time);

  virtual void VerifyDash(float *dash_power);

  /* You can specify a flag at compile time so that all the LogAction calls
     disappear
     If you add a LogAction, make sure to add it in both places */
  /* The # at the end of the function name is the total # of args */
#ifdef NO_ACTION_LOG
  inline void nothing_func(void) {}
#define LogAction1(x1) nothing_func()	
#define LogAction2(x1,x2) nothing_func()	
#define LogAction3(x1,x2,x3) nothing_func()	
#define LogAction4(x1,x2,x3,x4) nothing_func()	
#define LogAction5(x1,x2,x3,x4,x5) nothing_func()	
#define LogAction6(x1,x2,x3,x4,x5,x6) nothing_func()	
#define LogAction7(x1,x2,x3,x4,x5,x6,x7) nothing_func()	
#define LogAction8(x1,x2,x3,x4,x5,x6,x7,x8) nothing_func()	
#define LogAction9(x1,x2,x3,x4,x5,x6,x7,x8,x9) nothing_func()	
#define LogAction10(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) nothing_func()	
#else	
  void LogAction2(int level, const char* str); /* will be timestamped automatically */
  void LogAction3(int level, const char* str, char* param);
  void LogAction4(int level, const char* str, char* param1, char* param2);
  void LogAction4(int level, const char* str, char* param1, float param2);
  void LogAction5(int level, const char* str, char* param1, char* param2,char* param3);
  void LogAction3(int level, const char* str, char c1);
  void LogAction3(int level, const char* str, float f1);
  void LogAction4(int level, const char* str, float f1, int d1);
  void LogAction4(int level, const char* str, float f1, float f2);
  void LogAction5(int level, const char* str, float f1, float f2, float f3);
  void LogAction6(int level, const char* str, float f1, float f2, float f3, float f4);
  void LogAction7(int level, const char* str, float f1, float f2, float f3, float f4, float f5);
  void LogAction8(int level, const char* str, float f1, float f2, float f3, float f4, float f5, float f6);
  void LogAction3(int level, const char* str, int d1);
  void LogAction4(int level, const char* str, int d1, int d2);
  void LogAction4(int level, const char* str, int d1, float f1);
  void LogAction5(int level, const char* str, int d1, float f1, float f2);
  void LogAction6(int level, const char* str, int d1, float f1, float f2, float f3);
  void LogAction7(int level, const char* str, int d1, float f1, float f2, float f3, float f4);
  void LogAction7(int level, const char* str, int d1, int d2, float f1, float f2, float f3);
#define LogsDirectory "./logs/"
#endif	
  
  Socket           *sock;

  char             SaveLogFileName[MAX_FILE_LEN];
  FILE             *SaveLogFile;
  int              SaveLogCounter;
  char             SaveSoundLogFileName[MAX_FILE_LEN];
  FILE             *SaveSoundLogFile;
  int              SaveSoundLogCounter;
  char             SaveActionLogFileName[MAX_FILE_LEN];
  FILE             *SaveActionLogFile;
  int              SaveActionLogCounter;

  char             MySide;            // 'l' or 'r'                          
  char             TheirSide;         // 'l' or 'r'                          
  char             TheirTeamName[MAX_TEAMNAME_LEN];    // The name of their team                 
  int              MyTeamNameLen;     // strlen of MyTeamName
  Unum		   MyNumber;          // uniform number                      
  int              Initialized;					   
  Bool             ServerAlive;       // is the server going?
  Bool             TestVersion;       // is this the test version of the program?			   
  Bool             CoachActive;       // is there an on-line coach? 

  int              TimerInterval;
  Bool             ClockStopped;
  int              StoppedClockMSec;  // time elapsed in before_kick_off_mode (msec)

  Time             CurrentTime;       // Most recently observed time         
  Time             LastTime;          // previous time				   
  Time             LastSightTime;     // last time I saw			      
  Time             LastSenseTime;     // last time I sensed			   
  Time             LastSoundTime;     // last time I heard		      	   

  /* Don't need these after server 5.23
     void SetRealTimeOfLastSend(struct timeval& tv)
     { real_time_of_last_send = tv;}
     struct timeval GetRealTimeOfLastSend()
     { return real_time_of_last_send;}
     Bool TooSoonForAnotherSend();
  */
  
  int              LastSightInterval; // cycles between last 2 sights 
  inline Time      PreviousSightTime() { return LastSightTime - LastSightInterval; }  // previous time I saw 

  Command       *Action;
  Command       *LastAction;
  Command       ChangeView;
  Command       TurnNeck;
  Command 			PointTo;				///
  Command 			AttentionTo;		///


  inline Bool   ResendNeeded() 
  { return (RequestResend && CurrentTime == LastActionTime() 
	    && ResendTime == CurrentTime && LastActionType() == ResendType) ? TRUE : FALSE; }
  Bool          RequestResend;
  CMDType       ResendType;
  Time          ResendTime;

  inline AngleDeg LastActionAngle() { return LastAction->angle; }
  inline float    LastActionPower() { return LastAction->power; }
  inline float    LastActionX()     { return LastAction->y; }
  inline float    LastActionY()     { return LastAction->x; }
  inline CMDType  LastActionType()  { return LastAction->type; }
  inline Time     LastActionTime()  { return LastAction->time; }
  inline Bool     LastActionValid() { return LastAction->valid(); }
  inline Bool     LastActionValid(Time t) { return LastAction->valid(t); }

  /* have I already called a turn-neck? */
  inline Bool TurnNeckThisCycle() { return TurnNeck.time == CurrentTime ? TRUE : FALSE; }

  Time             LastBehaveTime;           // last time into behave		   	  
  Time             LastActionOpTime;         // last time I could have acted	     
  Time             LastInterruptTime;        // last time into the alarm handler
  int              InterruptsThisCycle;	     // number of times in so far					  
  Time             LastStartClockTime;       // time when server clock started again
  Time             SecondLastStartClockTime; // time when server clock started again

  bool		   SetPlay;//add by AI
  bool             pass_to_myself;//add by AI
  Time 		   time_set_pass_to_myself;//add by AI
  Unum		   player_that_must_see;//add by AI
  AngleDeg     ang_to_see;//add by AI
  bool 		   must_see_ang;//add by AI
  Vector           last_move_pos;
  Bool             NewSight;						      
  Bool             NewAction;
  Bool             FirstActionOpSinceLastSight;
  Bool             SightPredictedEarlyThisCycle();
  Bool             GotSightFromCurrentPosition();
  inline Bool      TimeToTurnForScan() 
  { return (SightPredictedEarlyThisCycle() || GotSightFromCurrentPosition()) ? TRUE : FALSE; }

  SenseType        LastSenseType;					   

  Vqual            ViewQuality;
  Vwidth           ViewWidth;
  Vwidth           LastViewWidth;
  Time             ViewWidthTime;
  

  Pmode            PlayMode;
  Time             PlayModeTime;
  Pmode            LastPlayMode;
  Kmode            KickOffMode;    
  int              MyScore;
  int              TheirScore;

  AngleDeg         MyViewAngle(Time time);
  inline AngleDeg  MyViewAngle() { return MyViewAngle(CurrentTime); }
  Bool             InViewAngle(Time time, AngleDeg ang, float buffer=5.0);
  inline Bool      InViewAngle(AngleDeg ang, float buffer=5.0) { return InViewAngle(CurrentTime,ang,buffer); }
  int              MySightInterval();
  int              PredictedNextSightInterval();

  inline void      SetMyPos(Vector p, Time t)   { pos = p; conf = 1; my_pos_time = t;}
  inline void      SetMyBodyAng(AngleDeg a)     { body_ang = a; }
  inline void      SetMyNeckRelAng(AngleDeg a)  { neck_rel_ang = a; }
  inline void      SetMyVel(Vector v, Time t)   { vel = v; vel_conf = 1; my_vel_time = t; }
  
  void SetMySensedInfo(float st, float e, float sp, float ha, int k, int d, int tu, int sa, int tn,
		       ArmInfo ar, int fc,int att,int tex, int tk, int m,Time ti);
  float GetMySensedSpeed(Time time);
  float GetMySensedStamina(Time time);
  float GetMySensedEffort(Time time);
  float GetMySensedNeckAngle(Time time);
  int   GetMySensedKicks(Time time);
  int   GetMySensedDashes(Time time);
  int   GetMySensedTurns(Time time);
  int   GetMySensedSays(Time time);
  int   GetMySensedMoves(Time time);
  int   GetMySensedTurnNecks(Time time);

  inline float MyStamina()  { return stamina; }
  inline float MyEffort()   { return effort; }
  inline float MyRecovery() { return recovery; }
  inline Bool  Tired()      
  { return ( MyStamina() < EffortDecThreshold + SP_max_power + CP_tired_buffer ) ? TRUE : FALSE; }

  float RecoveryDecThreshold;
  float EffortDecThreshold;
  float EffortIncThreshold;
  float CorrectDashPowerForStamina(float dash_power, float stamina, float effort, float recovery);
  inline float CorrectDashPowerForStamina(float dash_power, float stamina)
  { return CorrectDashPowerForStamina(dash_power, stamina, MyEffort(), MyRecovery()); }
  inline float CorrectDashPowerForStamina(float dash_power)
  { return CorrectDashPowerForStamina(dash_power, MyStamina(), MyEffort(), MyRecovery()); }
  void UpdatePredictedStaminaWithDash(float* pStamina, float* pEffort,
				      float* pRecovery, float dash_power);

  inline AngleDeg EffectiveTurn(AngleDeg actual_turn, float my_speed)
  { return 1.0 + actual_turn/(1.0 + GetMyInertiaMoment() * my_speed); }
  inline AngleDeg EffectiveTurn(AngleDeg actual_turn) 
  { return EffectiveTurn(actual_turn,MySpeed()); }
  inline AngleDeg MaxEffectiveTurn() /* how much we'll actually turn if we try max turn */
  { return EffectiveTurn(SP_max_moment); }
  inline AngleDeg MaxEffectiveTurn(float my_speed) /* how much we'll actually turn if we try max turn */
  { return EffectiveTurn(SP_max_moment, my_speed); }
  int NumCyclesToTurn(AngleDeg angle,float speed,AngleDeg body_ang); //add by AI
  int NumCyclesToTurn(AngleDeg angle,float speed){return NumCyclesToTurn(angle,speed,MyBodyAng());}//add by AI
  int NumCyclesToTurn(AngleDeg angle){return NumCyclesToTurn(angle,MySpeed(),MyBodyAng());}//add by AI
  bool IsBetterStopEndTurn(AngleDeg angle,float speed,AngleDeg body_ang){//add by AI
    return NumCyclesToTurn(angle,0.0f,body_ang)+1<NumCyclesToTurn(angle,speed,body_ang);
  }
  bool IsBetterStopEndTurn(AngleDeg angle,float speed){return IsBetterStopEndTurn(angle,speed,MyBodyAng());}
  bool IsBetterStopEndTurn(AngleDeg angle){return IsBetterStopEndTurn(angle,MySpeed(),MyBodyAng());}

  Bool CanFaceAngleFromNeckWithNeck(AngleDeg ang);
  Bool CanFaceAngleFromBodyWithNeck(AngleDeg ang);
  inline Bool CanFaceGlobalAngleWithNeck  (AngleDeg ang)
  { return CanFaceAngleFromBodyWithNeck(GetNormalizeAngleDeg(ang - MyBodyAng())); }
  inline Bool CanFacePointWithNeck(Vector pt) 
  { return CanFaceAngleFromBodyWithNeck(AngleToFromBody(pt)); }

  Bool CanSeeAngleFromNeckWithNeck(AngleDeg ang);
  Bool CanSeeAngleFromBodyWithNeck(AngleDeg ang);
  inline Bool CanSeeGlobalAngleWithNeck  (AngleDeg ang)
  { return CanSeeAngleFromBodyWithNeck(GetNormalizeAngleDeg(ang - MyBodyAng())); }
  inline Bool CanSeePointWithNeck(Vector pt) 
  { return CanSeeAngleFromBodyWithNeck(AngleToFromBody(pt)); }

  inline AngleDeg LimitTurnNeckAngle(AngleDeg turn_ang)
  { return MinMax(SP_min_neck_angle - MyNeckRelAng(), GetNormalizeAngleDeg(turn_ang),
		  SP_max_neck_angle - MyNeckRelAng()); }
  
  inline Bool SensedInfoKnown(Time time) { return (Bool) (sense_time == time || prev_sense_time == time); }

  inline Vector    MyPos()   { if (!MyConf())    my_error("Don't know my pos"); return pos; }
  inline float     MyX()     { return MyPos().x; }
  inline float     MyY()     { return MyPos().y; }
  inline AngleDeg  MyBodyAng() { return body_ang; }
  inline AngleDeg  MyNeckGlobalAng() { return GetNormalizeAngleDeg(MyBodyAng() + MyNeckRelAng()); }
  inline AngleDeg  MyNeckRelAng() { return neck_rel_ang; }
  inline Vector    MyVel()   { if (!MyVelConf()) my_error("Don't know my vel"); return vel; }
  inline float     MySpeed() { return MyVel().mod(); }
  inline float     MyDir()   { return MyVel().dir(); }
  inline float     MyConf()  { return (conf > CP_min_valid_conf) ? conf : 0; }
  inline float     MyVelConf() { return (vel_conf > CP_min_valid_conf) ? vel_conf : 0; }
  inline Time      MyPosTime() { return my_pos_time; }
  inline Time      MyVelTime() { return my_vel_time; }
  inline Time      MyUpdateTime() { return Min(my_pos_time,my_vel_time); }

  inline float     DistanceTo(Vector vec)  
  { if (!MyConf()) my_error("DistanceTo: no conf"); return (vec - pos).mod(); }
  inline AngleDeg  AngleToFromBody   (Vector vec)
  { if (!MyConf()) my_error("AngleTo: no conf"); 
  AngleDeg angto = (vec - pos).dir() - MyBodyAng(); NormalizeAngleDeg(&angto); return angto; }
  inline AngleDeg  AngleToFromNeck   (Vector vec)
  { if (!MyConf()) my_error("AngleTo: no conf"); 
  AngleDeg angto = (vec - pos).dir() - MyNeckGlobalAng(); NormalizeAngleDeg(&angto); return angto; }
  inline AngleDeg  AngleToGlobal   (Vector vec)
  { if (!MyConf()) my_error("AngleTo: no conf"); 
  AngleDeg angto = (vec - pos).dir(); NormalizeAngleDeg(&angto); return angto; }
  inline Vector    BodyPolar2Gpos(float dist, AngleDeg ang) 
  { if (!MyConf()) my_error("Polar2Gpos: no conf"); 
  Vector rpos = Polar2Vector(dist,ang); return MyPos() + rpos.rotate(MyBodyAng()); }
  inline Vector    NeckPolar2Gpos(float dist, AngleDeg ang) 
  { if (!MyConf()) my_error("Polar2Gpos: no conf"); 
  Vector rpos = Polar2Vector(dist,ang); return MyPos() + rpos.rotate(MyNeckGlobalAng()); }

  inline Bool AtPos(Vector p, float buffer) 
  { return (DistanceTo(p) <= buffer) ? TRUE : FALSE; }
  inline Bool AtPos(Vector p)
  { return AtPos(p, CP_at_point_buffer); }
  
  inline Vector Global2RelativeToMyBody(Vector v)  { return v.Global2Relative(MyPos(), MyBodyAng()); } 
  inline Vector Global2RelativeToMyNeck(Vector v)  { return v.Global2Relative(MyPos(), MyNeckGlobalAng()); } 
  inline Vector RelativeToMyBody2Global(Vector v)  { return v.Relative2Global(MyPos(), MyBodyAng()); } 
  inline Vector RelativeToMyNeck2Global(Vector v)  { return v.Relative2Global(MyPos(), MyNeckGlobalAng()); } 

  /* Cone stuff */
  Bool AmIInCone(float wid_dist_ratio, Vector end, Vector vert)
  { return IsPointInCone(MyPos(), wid_dist_ratio, end, vert); }
  
  /* Predicted Position */
  Vector MyPredictedPositionWithTurn(float angle,
				     int steps = 1, float dash_power = 0,
				     bool with_turn = TRUE,
				     int idle_cycles = 0);
  inline Vector MyPredictedPosition(int steps = 1, float dash_power = 0,
				    int idle_cycles = 0)
  { return MyPredictedPositionWithTurn(0.0, steps, dash_power, FALSE, idle_cycles); }
  Vector MyPredictedPositionWithQueuedActions();
  AngleDeg MyPredictedBodyAngleWithQueuedActions();
  AngleDeg PredictedPointRelAngFromBodyWithQueuedActions(Vector point);

  Vector MyPredictedPositionAtMaxSpeed(int steps = 1);

  /* How long to point */
  int PredictedCyclesToPoint(Vector pt, float dash_power);
  inline int PredictedCyclesToPoint(Vector pt)
  { return PredictedCyclesToPoint(pt, SP_max_power); }

  /* how long it takes to turn to an angle */
  int NumTurnsToAngle(float targ_body_ang, float curr_body_ang, float curr_speed);
  int NumTurnsToAngle(float targ_body_ang, float curr_body_ang)
  { return NumTurnsToAngle(targ_body_ang, curr_body_ang, MySpeed()); }
  int NumTurnsToAngle(float targ_body_ang)
  { return NumTurnsToAngle(targ_body_ang, MyBodyAng(), MySpeed()); }
  
  /* Which side of the field am I on? */
  inline Fside LocationSide(Vector p) { return p.y < 0 ? FS_Left : FS_Right; } 
  inline Fside MyLocationSide() { return LocationSide(MyPos()); }


  int              kicks;
  int              dashes;
  int              turns;
  int              says;
  int              turn_necks;
  int 					   tackles;
  int              attentions;
  int              moves;

  int lost_kicks;
  int lost_dash;
  int lost_turns;
  int lost_turn_necks;

  ArmInfo 	Arm;

  Unum 	attention_to_player;

  int tackle_expires;

  inline bool Tackling() {return tackle_expires>0;}
  inline int MyNumCyclesToEndTackling(){return tackle_expires;}
  inline AngleDeg  MyLastNeckGlobalAng() { return my_last_neck_global_ang; }
  float GetMyTackleConf(); //add by AI
  //AI-particle filter stuff//////////////////////////////////////////
  enum {numParticlesPosAgent=100};
  Vector particlesPosAgent[numParticlesPosAgent];
   
  void InitParticlesAgent(Vector pos);
  void updateParticleAgent(Vector vel,bool bAfterSense);
  Vector averageParticles(Vector posArray[],int size);
  void resampleParticleInfo(int first);
  bool getMinMaxDistQuantizeValue( double fOutput, double *fMin,double *fMax,double x1,double x2 );
  double invQuantizeMin( double fOutput, double fQuantizeStep );
  double invQuantizeMax( double fOutput, double fQuantizeStep );

  //add by AI: hetro players (functions only for my)
  static const int DEFAULT_HETRO_PLAYER_TYPE_NUMBER;

  int SetMyPlayerType(int type);//return old value
  int GetMyPlayerType()const{return players_type[MyNumber-1];}
	
  float GetMyPlayerSpeedMax(){return SP_hetro_player_type[GetMyPlayerType()].GetPlayerSpeedMax();}
  float GetMyStaminaIncMax(){return SP_hetro_player_type[GetMyPlayerType()].GetStaminaIncMax();}
  float GetMyPlayerDecay(){return SP_hetro_player_type[GetMyPlayerType()].GetPlayerDecay();}
  float GetMyInertiaMoment(){return SP_hetro_player_type[GetMyPlayerType()].GetInertiaMoment();}
  float GetMyDashPowerRate(){return SP_hetro_player_type[GetMyPlayerType()].GetDashPowerRate();}
  float GetMyPlayerSize(){return SP_hetro_player_type[GetMyPlayerType()].GetPlayerSize();}
  float GetMyKickableMargin(){return SP_hetro_player_type[GetMyPlayerType()].GetKickableMargin();}
  float GetMyKickRand(){return SP_hetro_player_type[GetMyPlayerType()].GetKickRand();}
  float GetMyExtraStamina(){return SP_hetro_player_type[GetMyPlayerType()].GetExtraStamina();}
  float GetMyEffortMax(){return SP_hetro_player_type[GetMyPlayerType()].GetEffortMax();}
  float GetMyEffortMin(){return SP_hetro_player_type[GetMyPlayerType()].GetEffortMin();}
  //addition
  float GetMyKickableArea(){return GetMyKickableMargin()+SP_ball_size+GetMyPlayerSize();}
  float GetMyStaminaInc(){return GetMyStaminaIncMax();}
  float GetMyOptCtrlDist(){return GetMyPlayerSize()+SP_ball_size+GetMyKickableMargin()/2;}
  float GetMyClosestMargin(){return GetMyPlayerSize()+2.0*SP_ball_size;}
protected:
  Time             my_pos_time;
  Time             my_vel_time;

  AngleDeg         my_last_body_ang;
  AngleDeg         my_last_neck_global_ang; 
  AngleDeg         my_last_neck_rel_ang;
  //add by AI
  vector<int> players_type;//num of hetro player type for ALL players
	
private:
  Vector           pos;
  AngleDeg         body_ang;
  AngleDeg         neck_rel_ang; /* neck angle relative to the body */
  Vector           vel;
  float            conf;
  float            vel_conf;

  float            stamina;
  float            effort;
  float            recovery;

  Time             sense_time;
  Time             prev_sense_time;

  float            last_speed; /* from sense_body */
  float            prev_speed;

  float            last_stamina;
  float            prev_stamina;

  float            last_effort;
  float            prev_effort;

  float            last_neck_rel_ang;
  float            prev_neck_rel_ang;

  int              last_kicks;
  int              prev_kicks;
  int              last_dashes;
  int              prev_dashes;
  int              last_turns;
  int              prev_turns;
  int              last_says;
  int              prev_says;
  int              last_turn_necks;
  int              prev_turn_necks;
  /////
  int              last_tackles;
  int              prev_tackles;
  int              last_pointtos;
  int              prev_pointtos;
  int              last_attentions;
  int              prev_attentions;
  int              last_moves;
  int              prev_moves;


  struct timeval   real_time_of_last_send; //the real time of the last send message

};

#endif
