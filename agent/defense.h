/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : defense.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.14 $
 *
 *    $Id: defense.h,v 2.14 2004/06/22 17:06:16 anton Exp $
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
#include "positioning.h"


#ifndef _DEFENCE_H
#define _DEFENCE_H

class Defense: public virtual Positioning{
public:
  Defense();
  ~Defense(){};
  enum MarkType{
    mark_goal=1,
    mark_bisector,
    mark_ball
  };

  void defense(void);
  bool IsPlayInOurPenaltyArea();
  bool MarkOpponent(Unum opp, MarkType type=mark_bisector,float dist=1.0f,Vector markPos=Vector(777.0f,777.0f));
	
  bool RecievedDefenseCommunicate(sayType st,Msg m,Unum from,Time t);
  Unum GetDefenseAttention();
private:
  void BeginDef();
  void AtOurPenaltyArea();

  void DefenderPlay();

  void DefenseGetBall(float max_dash=Mem->SP_max_power);
  bool CloseBallTake();
  Vector BlockAddVector(Vector start_pos,Vector target,Vector my_pos,Vector opp_pos);
  bool PressingWithoutDefenders();
  Unum GetOptimalTmToBallFromSet(int pt,int ps);
  bool GoToActiveDefense();
  Vector GetOpponentTarget();
  bool IsILastChance();

  Vector GetFastDefensePos(const Vector& ball);
  void Get2Defenders(Vector homePos,Unum& tm1,Unum& tm2);
  Unum GetDefenderWithClosestHomePos(Vector target,const Vector& homePos);
  bool IsTmBlockTarget(Unum tm,Vector homePos,Vector target); 
  Unum SelectWingMarkOpp(Unum tm,Vector homePos,float off_thr,bool& at_offside);//for return opp pos is valid
  Unum SelectCentralMarkOpp(Unum tm,Vector homePos,float off_thr,bool& at_offside);//for return opp pos is valid
  Unum SelectCentralBallMark(const Vector& homePos,Unum tm);
  set<Unum> GetNotBlockedOpponents(Unum teammate,Vector homePos);

  bool WingDefenderMark(Vector homePos,bool at_begin_pos);
  bool PassiveCentralDefense(Vector homePos);
  bool MidfielderMark(Vector homePos);
  bool BlockMovingOpponent(Unum opp);
  bool Mark();
  bool ICanNotMark();
  bool BlockCrossAtOurPenaltyArea(Vector ball);
  
  bool FlashPressing();
  bool BreakawayOnWings();
  bool BreakawayOnCenter();
  
  bool CloseTackleBehavior();

  Vector PredictOpponentPositionAtOurPenaltyArea(Unum opp,int numCyc,Vector ballPos);

  bool CanGoToActiveDefense(Unum* opp=0);
  Unum SweeperAttentionAtTime(Time time);
  void CheckForStartRest();
  bool IsRest(){return start_rest>0?Mem->CurrentTime-start_rest<=cycles_to_rest:false;}

  Time pressing_time;
  Unum pressing_opponent;
  Time last_marking_time;
	
  float x_to_rest;
  Time start_rest;
  int cycles_to_rest;

  static const float max_dist_than_start_mark;
  static const float penalty_area_treshold;
};

#endif
