/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Playposition.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: Playposition.h,v 2.7 2004/06/22 17:06:16 anton Exp $
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

#ifndef PLAYPOSITION_H
#define PLAYPOSITION_H


/**Formations, defens, offense and just positioning
 *@author Anton Ivanov
 */

#include "defense.h"
#include "Offense.h"

class PlayPosition:public Defense,public Offense {
public:
  PlayPosition();
  bool PlayWithoutBall();//return true if this func must control turn neck
  int without_ball(Unum tm=Mem->MyNumber);

  Vector GetTmPos(Unum tm);
  void UpdateTmPredictPos();//must call first EVERY cycle
  Vector GetTmTargetPoint(Unum tm);//return point there tm going
  Vector GetSmartPassPoint(Unum tm,float* vel=0);//active use data from GetTmTragetPoint(), so need UpdateTmPredictPos()
  Vector GetSmartPassPointInScenario(Unum tm,float* vel);//active use data from GetTmTragetPoint(), so need UpdateTmPredictPos()
  inline float GetEndVelInSmartPassKick()const;

  void SetFormation();
private:
  bool CheckTopBehavior(Unum tm);
  void UpdateTmState(Unum tm);

  Vector pos[11];
  float posValid[11],lastPosValid[11];
  float bodyAng[11];
  float bodyAngValid[11],lastBodyAngValid[11];
  Vector vel[11];
  float velValid[11],lastVelValid[11];

  Holder info[11];
  Time infoUpdate[11];
};

extern PlayPosition Pos;
#endif
