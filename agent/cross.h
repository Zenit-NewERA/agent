/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : cross.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: cross.h,v 2.7 2004/06/22 17:06:16 anton Exp $
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

#ifndef CROSS_H
#define CROSS_H

#include "geometry.h"
#include "kick.h"
#include "Playposition.h"
/**
 *@author Anton Ivanov
 */
class Cross : public virtual Kick {
public: 
  Cross();
  void StartCross();
  bool open_for_cross(Unum num,Offense::Holder& holder);
  bool open_for_cross(){
    Offense::Holder h;
    return open_for_cross(Mem->MyNumber,h);
  }
  AngleDeg GetAngle(){ return target.dir();}
  Vector GetTarget(){return target;}
  bool cross_angle(AngleDeg&);

  float GetCrossConf(Vector from,Vector to)const;
  float PosPriorety(Vector pos)const;

  static AngleDeg GetDirectionOfWidestAngle(Vector posOrg,AngleDeg angMin,AngleDeg angMax,AngleDeg* angLargest,float dDist);
  static bool ThroughPass();
  static bool ThroughPassAtTheirPenaltyArea();
private:

  static const int XMAX=15;
  static const int YMAX=20;
  float matrix[YMAX][XMAX];
  static const float threshold;

  float GetBeginX()const{return 51.0f-XMAX;}
  float GetEndX()const{return 51.0f;}
  float GetBeginY()const{return -YMAX/2;}
  float GetEndY()const{return YMAX/2;}

  bool IsPointInGoalCone(Vector point)const;
  void FillMatrix(Unum tm);
  float GetOptimalPoint(Unum tm,Vector& point);

  Vector target;
  float speed;
};

#endif
