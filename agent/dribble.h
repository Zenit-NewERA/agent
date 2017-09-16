/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : dribble.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.12 $
 *
 *    $Id: dribble.h,v 2.12 2004/06/22 17:06:16 anton Exp $
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


#include "Memory.h"
#include "kick.h"
#include "Handleball.h"

#ifndef _DRIBBLE_
#define _DRIBBLE_
//call functions of dribble only then ball is kickable!
//exception: kick_to_myself_in_progress()

enum DribbleType { only_control_dribble,no_control_dribble,no_avoid, avoid_1,avoid_2,avoid_all};
//only_control_dirbble - try to go only with control_ball
//no_control_dribble - try to go only with kick to myself
//no_avoid - not avoid opponent
//avoid_1 - try avoid 1 close opponent
//avoid_2 - try avoid 2 close opponent (ohh, yes ;)
//avoid_all - try avoid miximum of 5 opponents (are you crazy?)
class Dribble{
public:
  //priority - 0 very safe dribble
  Dribble(const Vector& pos=SelectDribbleTarget(),float p=0.5f);
  Dribble(AngleDeg ang,float p=0.5f);

  bool GoBaby(DribbleType type=no_avoid,float side=1.0f);//true - can go dribbling, false - do nothing
  bool DribbleDance(float side=1.0f);

  float SetPriority(float p) { float temp=priority; priority=p;dist_stop_controlling_dribble=(1-priority)*10.0f; return temp;} //return old value
  float GetPriority() const{ return priority;}
  AngleDeg SetDribbleAngle(AngleDeg da);//return old value
  AngleDeg GetDribbleAngle() const  { return dribble_angle;}
  Vector SetDribblePos(const Vector& pos);//return old value
  Vector GetDribblePos()const{ return dribble_target;}
  AngleDeg SetDribbleTurnAngleError(AngleDeg ang){AngleDeg t=dribble_turn_ang_err;dribble_turn_ang_err=ang;return t;}
  AngleDeg GetDribbleTurnAngleError()const {return dribble_turn_ang_err;}//return angle diff then we start correct body angle
  void SetStrongForward(){StrongForward=true;}//we can run only to dribble_angle
  void ResetStrongForward(){StrongForward=false;}//then we avoid we can go not to dribble_angle (or dynamic change dribble_angle)
  bool GetStrongForward(){ return StrongForward;}

  inline bool PNHDribble(Vector to,int fixed_cyc=max_cycles_to_run);
  inline bool PNHDribble(AngleDeg ang,int fixed_cyc=max_cycles_to_run);
  bool CanPNHDribble(AngleDeg ang,int fixed_cyc,Vector* vel,Vector to=Vector(-100,-100));//to-если не хотим пройти эту точку

  static bool kick_to_myself_in_progress();
  
  static Vector PredictDribbleStopPosition(Unum tm,Vector target,float prior,bool print_log=true,int* cycles=0);

  static Vector SelectDribbleTarget(Vector tm_pos,AngleDeg body_ang,bool print_log,float side_sign=1.0f);
  static Vector SelectDribbleTarget(){return SelectDribbleTargetForTeammate(Mem->MyNumber);}
  static Vector SelectDribbleTargetForTeammate(Unum tm,bool print_log=true);
  

  bool hold_ball(int max_close_opp=1);//param - max opp that may be close to us in hold ball
private:

  typedef struct _ClosePlayer{
    Unum opp;
    Vector pos;
    float body_ang_valid;
    AngleDeg body_angle;
    Vector vel;
    _ClosePlayer(Vector p=Vector(0.0f,0.0f)): opp(Unum_Unknown),pos(p),body_ang_valid(false),vel(Vector(.0f,.0f)){};
  }ClosePlayer;

  int CorrectCyclesForStamina(int cycles) ;
  Vector CorrectPosForStamina(Vector pos, int cycles) ;
  bool OpponentBehindUs(Unum opp,float buffer=3.0f) const{ return OpponentBehindUs(Mem->OpponentAbsolutePosition(opp),buffer,opp);}
  bool OpponentBehindUs(Vector pos,float buffer=3.0f, Unum opp=Unum_Unknown) const;

  ActionQueueRes controlling_dribbling(Vector* BallPredPos=0);
  void SetControlDribbleView(float dist_to_me,Vector ballVel);
  bool BallOnTrace(Vector ballPos=Mem->BallAbsolutePosition(),Vector MyPos=Mem->MyPos());
  int GetBallAreaNum(Vector ballPos=Mem->BallAbsolutePosition() ,Vector MyPos=Mem->MyPos());

  enum Trace {none,left,right};

  Trace SelectTrace(Vector my_pos=Mem->MyPos());
  bool MustChangeTrace()const;
  Vector GetTracePoint(Trace trace,Vector my_pos=Mem->MyPos());

  bool CanControllingDribble() const;//befor call OppIgnore[] must be initialize
  bool CanAgressiveControllingDribble(float side=1.0f,bool print_log=true) ;

  bool Covered(Unum opp,Vector ballpos,Vector opppos,Vector mypos,AngleDeg oppangle,Vector oppvel,bool isGoalie=false) const;
  Vector CorrectForCollision(Vector pos,Vector mypos,Vector origpos) const;
  float WeightByDistance(float distance) const;
  bool AvoidEnemy(const ClosePlayer* close_opp,int num);
  int SelectAvoidOpponents(ClosePlayer* opp,float max_dist=5.0f);
  Vector GetPredictedOpponentPosition(Unum opp,Vector opppos,Vector mypos,AngleDeg oppangle, Vector oppvel) const;
  void TurnWithBall(AngleDeg ang,Vector* BallPredPos=0);

  ActionQueueRes PNHDribbleExecute(AngleDeg ang,Vector vel);
  bool PNHDribble(AngleDeg ang=-777.0f,Vector pos=Vector(-100.0f,-100.0f),int max_cyc=max_cycles_to_run);

  AngleDeg dribble_angle;
  AngleDeg orig_dribble_angle;
  Vector dribble_target;

  float priority;//1 - ignore most opponents; if >0.5 use CanAgressiveControllingDribble()
  Kick kick;
  Trace trace;
  bool OppIgnore[11];
  bool StrongForward;

  float dist_line1;
  float dist_line2;
  float reliable_kick_area;

  static const float too_close_to_side_line;
  static const int max_cycles_to_run;

  static const bool watch_way_to_go;

  static const int max_num;//maximum of avoid opponents

  AngleDeg dribble_turn_ang_err;
  float dist_stop_controlling_dribble;//dist then we start avoid opponents
};
#endif
