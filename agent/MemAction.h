/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemAction.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.12 $
 *
 *    $Id: MemAction.h,v 2.12 2004/06/22 17:06:16 anton Exp $
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

/* MemAction.h
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

#ifndef _MEMACTION_H_
#define _MEMACTION_H_

#include "MemPosition.h"

/****************************************************************************************/

typedef struct PLAYERINTERCEPTINFO 
{
  Time time;
  float dash_pow;
  int lookahead;
  InterceptRes res;
  int numCyc;
  Vector pos;
  float dash_pow_to_use;
  float ang_error;
} PlayerInterceptInfo;

typedef struct{
  Vector vBallPos;
  Vector vBallVel;
  Vector vPlayerPos;
  Vector vPlayerVel;
  AngleDeg fPlayerAng;
  float stamina;
  float effort;
  float recovery;
}InterceptionInput;

class TurnKickCommand {
public:
  TurnKickCommand() {
    type=CMD_none;
    time=-1;
    turnneckneeded=false;
  };
  ~TurnKickCommand()	{};
public:
  void kick(float kickpower, AngleDeg kickangle) ;
  void dash(float dashpower);
  void turn(AngleDeg turnangle);
  void turnneck(AngleDeg turnneckangle);

  void Reset();

  bool Valid();
  bool Valid(Time checktime);
public:
  CMDType CommandType();
  bool TurnNeckNeeded();

  float Power() { return power; };
  float Angle() { return angle; };	
  float NeckAngle() { return turnneckangle; };
public:
  CMDType 	type;
  Time 		time;
  float 		angle;
  float 		power;
  bool 		turnneckneeded;
  float 		turnneckangle;
};

const int LA_Default = -2;
const int LA_BestSoFar = -1;




/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/

class ActionInfo : public PositionInfo{
public:

  //AI: new functions
  float EvaluateClearAngle(Vector end);

  void Initialize();

  // these are for hard kicking 
  Time HKTime;
  int HKStep;
  int HKStepNext;
  TurnDir HKrot;

  float VelAtPt2VelAtFoot(Vector pt, float targ_vel_at_pt);

  Unum OurBreakaway();
  Unum TheirBreakaway();

  /* these are for ball interception */
  InterceptRes PlayerInterceptionResult(char side, Unum num, float dash_pow);
  InterceptRes PlayerInterceptionResult(char side, Unum num)
  { return PlayerInterceptionResult(side, num, 
				    (side==MySide && TeammateTired(num)) ? GetPlayerStaminaInc(side,num) : SP_max_power); }
  Bool PlayerInterceptionAble(char side, Unum num, float dash_pow);
  Bool PlayerInterceptionAble(char side, Unum num)
  { return PlayerInterceptionAble(side, num, 
				  (side==MySide && TeammateTired(num)) ? GetPlayerStaminaInc(side,num) : SP_max_power); }
  int PlayerInterceptionNumberCycles(char side, Unum num, float dash_pow);
  int PlayerInterceptionNumberCycles(char side, Unum num)
  { return PlayerInterceptionNumberCycles(side, num, 
					  (side==MySide&&TeammateTired(num)) ? GetPlayerStaminaInc(side,num):SP_max_power); }
  Vector PlayerInterceptionPoint(char side, Unum num, float dash_pow);
  Vector PlayerInterceptionPoint(char side, Unum num)
  { return PlayerInterceptionPoint(side, num, 
				   (side==MySide && TeammateTired(num)) ? GetPlayerStaminaInc(side,num) : SP_max_power); }
  float PlayerInterceptionDashPower(char side, Unum num, float dash_pow);
  float PlayerInterceptionDashPower(char side, Unum num)
  { return PlayerInterceptionDashPower(side, num, 
				       (side==MySide && TeammateTired(num)) ? GetPlayerStaminaInc(side,num) : SP_max_power); }
  float PlayerInterceptionAngleError(char side, Unum num, float dash_pow);
  float PlayerInterceptionAngleError(char side, Unum num)
  { return PlayerInterceptionAngleError(side, num,
					(side==MySide && TeammateTired(num)) ? GetPlayerStaminaInc(side,num) : SP_max_power); }
  
  InterceptRes TeammateInterceptionResult(Unum num, float dash_pow)
  { return PlayerInterceptionResult(MySide, num, dash_pow); }
  InterceptRes TeammateInterceptionResult(Unum num)
  { return (num == MyNumber) ?
      MyInterceptionResult() :
    TeammateInterceptionResult(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power);
  }
  Bool TeammateInterceptionAble(Unum num, float dash_pow)
  { return PlayerInterceptionAble(MySide, num, dash_pow); }
  Bool TeammateInterceptionAble(Unum num)
  { return (num == MyNumber) ?
      MyInterceptionAble() :
    TeammateInterceptionAble(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power);
  }
  int TeammateInterceptionNumberCycles(Unum num, float dash_pow)
  { return PlayerInterceptionNumberCycles(MySide, num, dash_pow); }
  int TeammateInterceptionNumberCycles(Unum num)
  { return  (num == MyNumber) ?
      MyInterceptionNumberCycles() :
    TeammateInterceptionNumberCycles(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power);
  }
  Vector TeammateInterceptionPoint(Unum num, float dash_pow)
  { return PlayerInterceptionPoint(MySide, num, dash_pow); }
  Vector TeammateInterceptionPoint(Unum num)
  { return  (num == MyNumber) ?
      MyInterceptionPoint() :
    TeammateInterceptionPoint(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power); }
  float TeammateInterceptionDashPower(Unum num, float dash_pow)
  { return PlayerInterceptionDashPower(MySide, num, dash_pow); }
  float TeammateInterceptionDashPower(Unum num)
  { return  (num == MyNumber) ?
      MyInterceptionDashPower() :
    TeammateInterceptionDashPower(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power);
  }
  float TeammateInterceptionAngleError(Unum num, float dash_pow)
  { return PlayerInterceptionAngleError(MySide, num, dash_pow); }
  float TeammateInterceptionAngleError(Unum num)
  { return  (num == MyNumber) ?
      MyInterceptionAngleError() :
    TeammateInterceptionAngleError(num, TeammateTired(num) ? GetTeammateStaminaInc(num) : SP_max_power);
  }

  InterceptRes OpponentInterceptionResult(Unum num, float dash_pow)
  { return PlayerInterceptionResult(TheirSide, num, dash_pow); }
  InterceptRes OpponentInterceptionResult(Unum num)
  { return OpponentInterceptionResult(num, SP_max_power); }
  Bool OpponentInterceptionAble(Unum num, float dash_pow)
  { return PlayerInterceptionAble(TheirSide, num, dash_pow); }
  Bool OpponentInterceptionAble(Unum num)
  { return OpponentInterceptionAble(num, SP_max_power); }
  int OpponentInterceptionNumberCycles(Unum num, float dash_pow)
  { return PlayerInterceptionNumberCycles(TheirSide, num, dash_pow); }
  int OpponentInterceptionNumberCycles(Unum num)
  { return OpponentInterceptionNumberCycles(num, SP_max_power); }
  Vector OpponentInterceptionPoint(Unum num, float dash_pow)
  { return PlayerInterceptionPoint(TheirSide, num, dash_pow); }
  Vector OpponentInterceptionPoint(Unum num)
  { return OpponentInterceptionPoint(num, SP_max_power); }
  float OpponentInterceptionDashPower(Unum num, float dash_pow)
  { return PlayerInterceptionDashPower(TheirSide, num, dash_pow); }
  float OpponentInterceptionDashPower(Unum num)
  { return OpponentInterceptionDashPower(num, SP_max_power); }
  float OpponentInterceptionAngleError(Unum num, float dash_pow)
  { return PlayerInterceptionAngleError(TheirSide, num, dash_pow); }
  float OpponentInterceptionAngleError(Unum num)
  { return OpponentInterceptionAngleError(num, SP_max_power); }

  InterceptRes MyInterceptionResult(float dash_pow)
  { return PlayerInterceptionResult(MySide, MyNumber, dash_pow); }
  InterceptRes MyInterceptionResult()
  { return MyInterceptionResult(CorrectDashPowerForStamina(SP_max_power)); }
  Bool MyInterceptionAble(float dash_pow)
  { return PlayerInterceptionAble(MySide, MyNumber, dash_pow); }
  Bool MyInterceptionAble()
  { return MyInterceptionAble(CorrectDashPowerForStamina(SP_max_power)); }
  int MyInterceptionNumberCycles(float dash_pow)
  { return PlayerInterceptionNumberCycles(MySide, MyNumber, dash_pow); }
  int MyInterceptionNumberCycles()
  { return MyInterceptionNumberCycles(CorrectDashPowerForStamina(SP_max_power)); }
  Vector MyInterceptionPoint(float dash_pow)
  { return PlayerInterceptionPoint(MySide, MyNumber, dash_pow); }
  Vector MyInterceptionPoint()
  { return MyInterceptionPoint(CorrectDashPowerForStamina(SP_max_power)); }
  float MyInterceptionDashPower(float dash_pow)
  { return PlayerInterceptionDashPower(MySide, MyNumber, dash_pow); }
  float MyInterceptionDashPower()
  { return MyInterceptionDashPower(CorrectDashPowerForStamina(SP_max_power)); }
  float MyInterceptionAngleError(float dash_pow)
  { return PlayerInterceptionAngleError(MySide, MyNumber, dash_pow); }
  float MyInterceptionAngleError()
  { return MyInterceptionAngleError(CorrectDashPowerForStamina(SP_max_power)); }

  /* just min of what's been done so far - returns -1 if nothing done */
  int GetInterceptionMinCyc();
  inline void ResetInterceptionMinCyc()  { IntMinCycTime -= 1; }
  inline int  GetInterceptionLookahead() { return InterceptLookahead; }
  void SetInterceptionLookahead(int newval);

  Bool BallPathInterceptValid();
  Vector BallPathInterceptPoint();
  Bool BallPathInterceptAmIThere(float buffer);
  Bool BallPathInterceptAmIThere()
  { return BallPathInterceptAmIThere(CP_at_point_buffer); }
  float BallPathInterceptDistance();
  /* careful! if ball is kickable, next func returns 0 */
  int BallPathInterceptCyclesForBall();
  Bool BallPathInterceptCanIGetThere(float max_pow = 100.0);

  KickMode BestKickModeAbs(AngleDeg abs_ang);
  KickMode BestKickMode(AngleDeg rel_ang)  /* Angle relative to body */
  { return BestKickModeAbs(GetNormalizeAngleDeg(rel_ang + MyBodyAng())); }

  int EstimatedCyclesToSteal(Unum opp, Vector ball_pos);  
  inline int EstimatedCyclesToSteal(Unum opp, AngleDeg ball_ang) //absolute angle
  { return EstimatedCyclesToSteal(opp, MyPos() +
				  Polar2Vector(GetMyOptCtrlDist(), ball_ang));
  }
  inline int EstimatedCyclesToSteal(Unum opp)
  {
    if (!BallPositionValid())
      my_error("EstimateCyclesToSteal: don;t know where ball is");
    return EstimatedCyclesToSteal(opp, BallAbsolutePosition());
  }

  
  Bool WillDashHelpKick(Vector pt, float dash_pow);
  Bool WillDashHelpKick(Vector pt)
  { return WillDashHelpKick(pt, SP_max_power); }


  
  Bool KickInProgress(); 
  void StartKick(AngleDeg target_angle, KickMode mode, float target_vel, TurnDir rot=TURN_AVOID);
  void StartShot(AngleDeg target_angle, KickMode mode, TurnDir rot=TURN_AVOID);
  void StartPass(Unum target, float target_vel, TurnDir rot=TURN_AVOID);  

  Bool           kick_in_progress;
  AngleDeg       kick_in_progress_abs_angle;
  float          kick_in_progress_target_vel;
  KickMode       kick_in_progress_mode;
  TurnDir        kick_in_progress_rotation;

  Unum  team_receiver;
  Unum  team_passer;
  Time  team_pass_time;
  Vector passvel;

  void SetTeamRecvInfo(Unum recv, Unum passer, Time set_time) {
    team_receiver	 = recv;
    team_passer = passer;
    team_pass_time = set_time;					
  };
  bool IsReceiver() { return team_receiver==MyNumber && CurrentTime.t-team_pass_time.t<=CP_team_receive_time; };
  void ResetReceiver() { team_receiver = Unum_Unknown; };
  bool TimeToResetReceiver() { return (CurrentTime.t-team_pass_time.t)>8; };

  bool ball_coming;
  Unum ball_intercepter;
  Time ball_info_time;

  void SetBallComingInfo(Unum intercepter, Time set_time) {
    ball_coming = true;
    ball_intercepter = intercepter;
    ball_info_time = set_time;		
  };
  bool IsBallComing() { return ball_intercepter==MyNumber && ball_coming && CurrentTime.t-ball_info_time.t<CP_team_receive_time; };
  void ResetBallComing() { ball_coming = false; };
  bool Need2WornTeammate() { return CurrentTime.t-ball_info_time.t<CP_team_receive_time && ball_coming; };

  Unum ScenarioCaptain;
  //ScenarioType CurrentScenario;

  Unum FastestTeammateToBall();
  Unum FastestOpponentToBall();
  Unum BallPossessor();   /* possessor means can get there quickest */
  char TeamInPossession();

  Unum LastBallPossessor;
  Time LastBallPossessorTime;

  //////////////////////////////////////////
  PlayerInterceptInfo  ActiveCanGetThere(float max_pow, int max_lookahead,
					 Vector vBallPos, Vector vBallVel,
					 char side, Unum num,
					 Vector vPlayerPos, Vector vPlayerVel,
					 float vPlayerAng, int PlayerAngValid,
					 bool IsThisMe);
  //AI: new generation interception stuff           
  PlayerInterceptInfo SmartCanGetThere(float max_pow,
				       Vector vBallPos, Vector vBallVel,
				       Vector vPlayerPos, Vector vPlayerVel,
				       float vPlayerAng);
  float SelectPriorityDot(Vector pos1,Vector pos2);
  int  GetInterceptionPoints( float max_pow,PlayerInterceptInfo* res,const InterceptionInput& data);
  int CorrectAllCycles(int numRoots,int* calcNumCycles,float max_pow,PlayerInterceptInfo* res,const InterceptionInput& data);
  PlayerInterceptInfo CorrectPos(int startStep,int dir,int stopStep,float max_pow,InterceptionInput data);
  void PrintInfoStruct(const char* prefix,const PlayerInterceptInfo& info);
  float   NewtonMethod(float start,float prev_root,const InterceptionInput& data,bool use_orig_newton);
  void GetPredictedBallPosVel(int steps,Vector origPos,Vector vel,Vector* newPos,Vector* newVel=0);
  float GetBallMoveCoeff(int steps);
  Vector MyInterceptPredictedPositionWithTurn(float turn_ang,
					      int steps, float dash_power,
					      bool with_turn,const InterceptionInput& data,
					      int idle_cycles=0);
  void AddFinalDot(const InterceptionInput& data,int* numRoots,PlayerInterceptInfo* res);
  inline float fcp(float x,const InterceptionInput& data);
  inline float fcpDiff(float x,const InterceptionInput& data);
  inline float fcb(float x,float velMod);
  inline float fcbDiff(float x,float velMod);
  inline float fcpEx(float x,float prev_root,const InterceptionInput& data);
  inline float fcpDiffEx(float x,float prev_root,const InterceptionInput& data);
  inline float fcbEx(float x,float prev_root,float velMod);
  inline float fcbDiffEx(float x,float prev_root,float velMod);
///PASS BASE///////////////////////////////////////////////////////////////////
  
  int GetPlayerInterceptionPoint(char side,Unum num,PlayerInterceptInfo* res,const InterceptionInput& data);
  inline float fcp_1DegStep(float x,AngleDeg ang,const InterceptionInput& data);
  float Sin_1DegStep(AngleDeg ang){return ang<0?-Sin_1DegStep_ar[-(int)ang]:Sin_1DegStep_ar[(int)ang];}
  float Cos_1DegStep(AngleDeg ang){return Cos_1DegStep_ar[(int)fabs(ang)];}
  float GetMaxSpeedWithIntercept(float p,AngleDeg ang,const InterceptionInput& data);
///END PASS BASE///////////////////////////////////////////////////////////////////  
  bool IsSuccessRes(InterceptRes res)
  { return (res == BI_CanChase || res == BI_ReadyToKick|| res==BI_OnlyTurn); }
  PlayerInterceptInfo CloseBallInterception(float max_pow, int max_lookahead,
					    Vector vBallPos, Vector vBallVel);
  bool GoalieCloseBallInterception(Vector vBallPos, Vector vBallVel);

  int GetClosestPointToBallPath(Vector* pvPt, float* pNumCycles,
				Vector PlayerPos, Vector BallPos,
				Vector BallVel);

  Time ChangeViewForHandleBallTime;
  PlayerInterceptInfo* VerifyIntInfo(char side, Unum num, float dash_pow);
private:
  ///PASS BASE///////////////////////////////
  float Sin_1DegStep_ar[181];
  float Cos_1DegStep_ar[181];

  ///////////////////////////////////
  Time           dribble_time; /* Last time we called DribbleTo */

  Time           DribbleTargetTime;
  Vector         DribbleTargetVector;
  Bool           DribbleTargetAvailable;
  ///////////////////////////////////



  Time           start_kick_time;
  Time           kick_in_progress_time;

  PlayerInterceptInfo* TeamIntInfo[MAX_PLAYERS];
  PlayerInterceptInfo* OppIntInfo[MAX_PLAYERS];

  Time Stored_Fastest_Teammate_Time;
  Unum Stored_Fastest_Teammate;
  Time Stored_Fastest_Opponent_Time;
  Unum Stored_Fastest_Opponent;

  int InterceptLookahead; /* can either be a positve number or a LA_constant
			     from above */
  int IntMinCyc;
  Time IntMinCycTime;
  void SetIntMinCyc(int newval);

  void BallIntercept_active(float max_pow_to_use, int max_lookahead,
			    char PlayerSide, Unum PlayerNum,
			    PlayerInterceptInfo* pInfo);

  PlayerInterceptInfo* GetPlayerIntInfo(char side, Unum num);


  Time BPItime;
  Bool BPIvalid;
  Bool BPIable; 
  float BPIdist;
  Vector BPIpoint;
  float BPIballcyc;

  void VerifyBPIInfo();
  void BallIntercept_passive(float max_pow_to_use,
			     PlayerInterceptInfo* pInfo);      


};

inline Bool ActionInfo::BallPathInterceptValid()
{
  VerifyBPIInfo();
  return BPIvalid;
}

#endif
