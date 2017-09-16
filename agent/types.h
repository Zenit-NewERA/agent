/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : type.h
 *
 *    AUTHOR     : Anton Ivanov, Sergey Serebyakov
 *
 *    $Revision: 2.9 $
 *
 *    $Id: types.h,v 2.9 2004/06/22 17:06:16 anton Exp $
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
// ORIGINAL:
//	 types.h
//	 CMUnited99 (soccer client for Robocup99)
// 	 Peter Stone <pstone@cs.cmu.edu>
// 	 Computer Science Department
// 	 Carnegie Mellon University
//	 Copyright (C) 1999 Peter Stone
//
// 	 CMUnited-99 was created by Peter Stone, Patrick Riley, and Manuela Veloso
//
// 	 You may copy and distribute this program freely as long as you retain this notice.
// 	 If you make any changes or have any comments we would appreciate a message.
// 	 For more information, please see http://www.cs.cmu.edu/~robosoccer/



#ifndef _TYPES_H_
#define _TYPES_H_

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <sstream>
using namespace std;

#define NULLCHAR	'\000'			// perhaps we should take it off!

typedef int Unum;  // Uniform number
typedef int Pnum;  // Position number
typedef int SPnum; // Setplay position number

enum SenseType{
  See_Msg,	//(see Time ...)
  Hear_Msg,	//(hear Time ...)
  Sense_Msg,	//(sense_body Time...)
  Score_Msg,	//(score Time ...)
  SP_Msg,	//(server_param ...)
  PP_Msg,	//(player_param ...)
  PT_Msg,	//(player_type ...)
  PT_Change,    //(change_player_type ...)
  Ok_Msg,       //(ok ...)
  Warning_Msg,    //(warning ...)
  Error_Msg
};

enum Sender{ 			//SND - SeNDer
  SND_online_coach_ours,
  SND_online_coach_theirs,
  SND_referee,
  SND_self,
  SND_direction
};

enum CMDType {
  CMD_none,
  CMD_dash,
  CMD_turn,
  CMD_kick,
  CMD_catch,
  CMD_move,
  CMD_bye,
  CMD_change_view,
  CMD_turn_neck,
  CMD_say,
  CMD_sense_body,
  CMD_pointto,
  CMD_tackle,
  CMD_attentionto
};

enum ObjType{
  OBJ_Line,
  OBJ_Ball,
  OBJ_Marker,
  OBJ_Marker_Behind,  // Not seen
  OBJ_Player
};

enum Vqual{
  VQ_Low,
  VQ_High
};

enum Vwidth{
  VW_Narrow,
  VW_Normal,
  VW_Wide,
  VW_Unknown
};

enum RequestType{
  RT_Unknown,
  RT_MustSee,
  RT_WithoutTurn,
  RT_WithTurn,      
  RT_WithOrWithoutTurn
};

enum ObjectType {
  OT_Unknown,  
  OT_Ball,
  OT_Teammate,
  OT_Opponent,
  OT_Position,
  OT_BallBuffer,
  OT_TeammateBuffer,
  OT_OpponentBuffer,
  OT_PositionBuffer
};
enum Kmode{
  KO_Mine,
  KO_Theirs
};

enum Pmode{
  PM_No_Mode,
  PM_Before_Kick_Off,
  PM_My_Kick_Off,
  PM_Their_Kick_Off,
  PM_My_Kick_In,
  PM_Their_Kick_In,
  PM_My_Corner_Kick,
  PM_Their_Corner_Kick,
  PM_My_Goal_Kick,
  PM_Their_Goal_Kick,
  PM_My_Free_Kick,
  PM_Their_Free_Kick,
  PM_My_Goalie_Free_Kick,     /* not a real play mode */
  PM_Their_Goalie_Free_Kick,  /* not a real play mode */
  PM_Drop_Ball,
  PM_My_Offside_Kick,
  PM_Their_Offside_Kick,
  PM_Play_On,
  PM_Half_Time,
  PM_Time_Up,
  PM_Extended_Time,
  PM_My_Back_Pass,
  PM_Their_Back_Pass,
  PM_My_Free_Kick_Fault,
  PM_Their_Free_Kick_Fault,
  PM_My_Catch_Fault,
  PM_Their_Catch_Fault,
 
  PM_My_PenaltySetup,
  PM_Their_PenaltySetup,
  PM_My_PenaltyReady,
  PM_Their_PenaltyReady,
  PM_My_PenaltyTaken,
  PM_Their_PenaltyTaken,
  PM_My_PenaltyMiss,
  PM_Their_PenaltyMiss,
  PM_My_PenaltyScore,
  PM_Their_PenaltyScore
 
};

enum Bool{
  FALSE,
  TRUE
};

enum SideLine{
  SL_Left,
  SL_Right,
  SL_Top,
  SL_Bottom,

  SL_No_Line
};


enum MarkerType{
  Goal_L,
  Goal_R,

  Flag_C,
  Flag_CT,
  Flag_CB,
  Flag_LT,
  Flag_LB,
  Flag_RT,
  Flag_RB,

  Flag_PLT,
  Flag_PLC,
  Flag_PLB,
  Flag_PRT,
  Flag_PRC,
  Flag_PRB,

  Flag_GLT,
  Flag_GLB,
  Flag_GRT,
  Flag_GRB,

  Flag_TL50,
  Flag_TL40,
  Flag_TL30,
  Flag_TL20,
  Flag_TL10,
  Flag_T0,
  Flag_TR10,
  Flag_TR20,
  Flag_TR30,
  Flag_TR40,
  Flag_TR50,

  Flag_BL50,
  Flag_BL40,
  Flag_BL30,
  Flag_BL20,
  Flag_BL10,
  Flag_B0,
  Flag_BR10,
  Flag_BR20,
  Flag_BR30,
  Flag_BR40,
  Flag_BR50,

  Flag_LT30,
  Flag_LT20,
  Flag_LT10,
  Flag_L0,
  Flag_LB10,
  Flag_LB20,
  Flag_LB30,

  Flag_RT30,
  Flag_RT20,
  Flag_RT10,
  Flag_R0,
  Flag_RB10,
  Flag_RB20,
  Flag_RB30,

  No_Marker
};


enum Ptype{
  PT_None=0,
  PT_Goaltender=1<<0,
  PT_Sweeper=1<<1,
  PT_Defender=1<<2,
  PT_Midfielder=1<<3,
  PT_Forward=1<<4,
  PT_All=PT_Goaltender|PT_Sweeper|PT_Defender|PT_Midfielder|PT_Forward
};

enum Pside{
  PS_None=0,
  PS_Left=1<<0,
  PS_Center=1<<1,
  PS_Right=1<<2,
  PS_All=PS_Left|PS_Center|PS_Right
};

enum Fside{ // Side of the field
  FS_Right,
  FS_Left
};

enum DodgeType{
  DT_none,
  DT_all,
  DT_unless_with_ball,
  DT_only_with_ball
};

// these are for things in kick.*

typedef enum TURNDIR
  { TURN_NONE = 0,
    TURN_CW = -1,
    TURN_CCW = 1,
    TURN_CLOSEST = 10,
    TURN_AVOID = 11 		// avoid any opponents
  } TurnDir;

typedef enum KICKTORES
  { KT_None,
    KT_Success,
    KT_DidKick,
    KT_DidNothing,
    KT_TurnedToBall,
    KT_LostBall
  } KickToRes;

typedef enum KICKMODE
  { KM_None,
    KM_HardestKick,
    KM_Hard,
    KM_Moderate,
    KM_Quickly,
    KM_QuickestRelease
  } KickMode;


// these are for things in intercept.*

typedef enum INTERCEPTRES
  { BI_None,        // no value yet
    BI_Invalid,     // could not get an answer
    BI_Failure,     // won;t be able to intercept ball
    BI_CanChase,    // we're getting there - returned a GoToPoint command
    BI_ReadyToKick,  // ball is in kickable area, we haven;t done anything yet
    BI_OnlyTurn     //i need make only turn to intercept ball  
  } InterceptRes;

typedef enum ACTIONQUEUERES
  {
    AQ_ActionQueued,
    AQ_ActionNotQueued
  } ActionQueueRes;


#endif
