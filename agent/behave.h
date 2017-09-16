/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : behave.h
 *
 *    AUTHOR     : Anton Ivanov, Sergei Serebyakov
 *
 *    $Revision: 2.8 $
 *
 *    $Id: behave.h,v 2.8 2004/06/22 17:06:16 anton Exp $
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

/* behave.h
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


#ifndef _BEHAVE_H_
#define _BEHAVE_H_

#include "geometry.h"
#include "types.h"
using namespace std;
//////////////////////////////////////////////////////////
class ThroughPass{
public:
  ThroughPass();
  void RecievedBeginThroughPassMsg(char* m,Unum from,Time time);
  Unum ChangeAttantionInFroughPassToTm();
  void RecievedAnswerThroughPass(char* m,Unum from,Time time);
  bool StartThroughPass();
  bool GoInThroughPass();
  bool KickInThroughPass();
  bool IsTmGoInThroughPass(Unum tm);
  Vector GetThroughPassTarget()const{return targetPos;}
  void StopThroughPass(){beginTime=-1;}
  bool IsIStartThroughPass()const;
  Unum GetReciever()const{ return to;}
private:
  Unum SelectThroughPass(int begin_cyc,Vector from,Vector* addition);
  float CalcThroughPassVel();
  inline float GetMaxEndPassVel()const;
  inline float GetMinEndPassVel()const;

  Time beginTime;
  Vector targetPos;
  Vector fromPos;
  Unum from;
  Unum to;
  int num_cyc;

  int answer_cyc;
  Time answerTime;
};
extern ThroughPass throughPass;
//////////////////////////////////////////////////////////////////////
class MicroKicks
{
public:
  bool CanMicroAvoidGoalie(float side=1.0f);
  bool CanMicroClearBall(bool can_stop_ball);
  bool CanMicroCrossBall(Vector target);
  bool CanMicroHardestShoot(float side=1.0f);
private:
  Vector SelectAvoidPosition(Vector avoid_pos,Vector target);  
};

extern MicroKicks microKicks;
////////////////////////////////////////////////////
bool play_game();
bool InDangerSituation();
//AI:new functions
bool PenaltyPlay();
bool IsGoalieActive(float penalty_side=1.0f);
void PenaltyPlayScanField();
int NumOfCyclesThenILastSeePlayer(float);
int NumOfCyclesThenILastSeePlayer(int);
void clear_ball();
void recover(void);
void get_on_sides();
void scan_field(void);
void scan_field_to_ball();
void my_update_offside_position(void);

void behave();

ActionQueueRes scan_field_with_body();
void turn_neck_to_relative_angle(AngleDeg ang);
void scan_field_with_neck();

ActionQueueRes face_only_body_to_point(Vector point);
void           face_only_neck_to_point(Vector point);
ActionQueueRes face_neck_to_point(Vector point);
ActionQueueRes face_neck_and_body_to_point(Vector point);

ActionQueueRes face_only_body_to_player(char side, Unum num);
void           face_only_neck_to_player(char side, Unum num);
ActionQueueRes face_neck_to_player(char side, Unum num);
ActionQueueRes face_neck_and_body_to_player(char side, Unum num);

ActionQueueRes face_only_body_to_opponent(Unum opponent);
void           face_only_neck_to_opponent(Unum opponent);
ActionQueueRes face_neck_to_opponent(Unum opponent);
ActionQueueRes face_neck_and_body_to_opponent(Unum opponent);

ActionQueueRes face_only_body_to_teammate(Unum teammate);
void           face_only_neck_to_teammate(Unum teammate);
ActionQueueRes face_neck_to_teammate(Unum teammate);
ActionQueueRes face_neck_and_body_to_teammate(Unum teammate);

ActionQueueRes face_only_body_to_ball();
void           face_only_neck_to_ball();
ActionQueueRes face_neck_to_ball();
ActionQueueRes face_neck_and_body_to_ball();

void get_ball();
void stop_ball();


ActionQueueRes go_to_point(Vector p, float buffer = 0, float dash_power = 100, DodgeType dodge = DT_all,bool to_ball=false);

#endif
