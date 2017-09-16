/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : SetPlay.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.5 $
 *
 *    $Id: SetPlay.h,v 2.5 2004/04/19 08:00:01 anton Exp $
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
#include <map>
#include "types.h"
#include "positioning.h"

#ifndef _SETPLAY_H
#define _SETPLAY_H
class SetPlay: public Positioning{
public:
  enum GoalieActions{
    estimate=0,
    scenario_estimate=1,
    execute=2,
    flash=3,
    estimate_now=4
  };

  SetPlay();
  virtual ~SetPlay(){}
  Vector GetTmPos(Unum){return Vector(.0,.0);}//for making object of SetPlay
  bool Standart();
  bool ForGoalieMyKickOff(GoalieActions action);
  bool Recovery();
  void BeginScenario(int number){  IsScenario=true; num_scenario=number;}
  void ResetScenario(){ IsScenario=false;}
  bool ScenarioGoing(){ return IsScenario;}
	
  bool IsPenaltyGoing(){return start_penalty_mode;}
  void InitializePenaltyMode(char side);
private:
  void StartScenario(aType (SetPlay:: *f)());
  void StartScenario(aType scenario);

  aType SelectScenario_MyGoalieKickOff();
  aType SelectScenario_MyKickIn();
  aType SelectScenario_MyCornerKick();
  aType SelectScenario_MyGoalieKick();
  aType SelectScenario_MyKickFree();

  bool BeforKickOff();
  bool MyKickOff();
  bool TheirKickOff();
  bool HalfTime();
  bool MyGoalieFreeKick();
  bool MyKickIn();
  bool MyCornerKick();
  bool MyGoalieKick();
  bool MyFreeKick();
  bool TheirSetPlay();
  bool MyFault();//for back_pass, free_kick_fault and catch_fault
  bool TheirFault();//for back_pass, free_kick_fault and catch_fault

  bool MyPenaltySetup();
  bool TheirPenaltySetup();
  bool MyPenaltyReady();
  bool TheirPenaltyReady();
  bool MyPenaltyTaken();
  bool TheirPenaltyTaken();
  bool MyPenaltyScoreMiss();
  bool TheirPenaltyScoreMiss();
	
  Unum SelectOptimalPlayer(int PT_mask,int PS_mask);
  Unum MarkInSetPlay(Unum tm);
  bool AtTargetPos(Unum tm);
  bool GoToHomePosition(Vector pos=Vector(-200.0f,-200.0f));
  bool IsMyTimeToKickPenalty()const{return Mem->SP_team_size-our_kicks%Mem->SP_team_size==Mem->MyNumber;}
	
  typedef bool (SetPlay::*FP) ();
  map<Pmode,FP> functions;

  typedef Vector (Formations::*SPP)(Unum);
  map<Pmode,SPP> SP_functions;
  SPP CurrentHomePos;

  bool IsScenario;
  int num_scenario;

  bool start_penalty_mode;
  int penalty_side;
  int our_kicks;
  int their_kicks;
  float y_to_kick;
};

extern SetPlay setPlay;

#endif
