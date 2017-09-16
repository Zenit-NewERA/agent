/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Offense.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: Offense.h,v 2.7 2004/06/22 17:06:16 anton Exp $
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

#ifndef ___OFFENSE_H
#define ___OFFENSE_H
#include "geometry.h"
#include "positioning.h"

class Offense:public virtual Positioning{
public:
  enum Type{
    t_go_to_pos,
    t_get_ball,
    t_turn_body_to_ball
  };
  struct Holder{
    Holder(){type=t_turn_body_to_ball;name="turn body to ball (in constructor";}
    Holder(Type t,string n):type(t),name(n){}
    Holder(Type t,Vector p,float ae,float d,string n):type(t),pos(p),ang_error(ae),dash(d),name(n){}
    Type type;
    Vector pos;
    float ang_error;
    float dash;
    string name;
  };

  void offense(Unum num=Mem->MyNumber);//==MyNumber - execute (and log), else only estimate (result in holder and no log)
protected:
  Holder holder;
private:  
  bool PositioningOnOpponentDefenseLine(Unum tm);
  bool WingForwardBehavior(Unum tm);
  bool WingMidfielderBehavior(Unum tm);
  bool ForwardPlayInCentralZone(Unum tm);
  bool CorrectPositionToHelpTmBreakaway(Unum tm);
  
  int fast_attack(Unum num);

  bool positioning_in_their_penalty_area(Unum tm);
  bool PlayAtTheirPenaltyArea(Unum tm);

  bool open_for_pass(Unum tm);
  bool open_on_checkpoints(void);

  bool can_open_for_pass(Unum tm,Vector from);
  Vector get_dir_to_open_for_pass(Vector from,Vector to);
  void open_from_point2point(Unum tm,Vector from,Vector to);
};

#endif
