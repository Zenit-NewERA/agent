/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : goalie.h
 *
 *    AUTHOR     : Anton Ivanov, Andrey Gusev, Sergey Serebyakov
 *
 *    $Revision: 2.9 $
 *
 *    $Id: goalie.h,v 2.9 2004/06/22 17:06:16 anton Exp $
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
#ifndef _GOALIE_H_
#define _GOALIE_H_

#include "geometry.h"
#include "client.h"

class Goalie {
public:
  Goalie();

  void Behave();
  void PenaltyBehavior(float dDist,float side);
  Vector GetSimpleInterceptionPoint();
private:
  bool CanIntercept();
  void GoalieScanField();
  void kick_off();
  bool MustDestroyOpponent();

  void SayAboutBall();

  bool CanCatch();
  void Catch();

  bool IsShoot(float side=1.0f);
  Vector GetShootPoint(float side=1.0f);
  void InterceptShoot(Vector target);
  bool IsBlindShoot();

  void DefendGoalLine( float dDist,float side=1.0f);
  Vector GetDefendPos(float dDist,float side=1.0f,Vector ball=Mem->BallAbsolutePosition());
  float GetDefendDist(Vector ball);
  bool DangerForCloseCorner(Vector* ball,float dDist,float side=1.0f);

  Time last_observe_time;
  Time last_catch_time;

  const float BASE;
};
extern Goalie goalie;
#endif
