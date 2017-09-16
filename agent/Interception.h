/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Intreception.h
 *
 *    AUTHOR     : Sergey Serebyakov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Interception.h,v 2.3 2004/03/05 07:03:44 anton Exp $
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
#ifndef __INTERCEPTION_H_
#define __INTERCEPTION_H_

#include "Memory.h"
#include "client.h"
#include <vector>
#include "behave.h"
#include "Processor.h"
#include "kick.h"
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
extern const int InfCycles;
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
namespace InterceptionSkills {
  //---------------------------------------------------
  //---------------------------------------------------
  class Intercepter {
  public:
    Intercepter(char side, Unum num, Vector pos, Vector vel, AngleDeg ang, bool ang_valid, bool me) {
      Set(side, num, pos, vel, ang, ang_valid, me);
    };
    Intercepter();
    ~Intercepter()	{};

    void 			Set(char side, Unum num, Vector pos, Vector vel, AngleDeg ang, bool ang_valid, bool me);
    void 			SetAng(AngleDeg ang)	{ this->ang=ang; };

    char 			Side()		{ return side; };
    Unum			Num()			{ return num; };
    Vector		Pos()			{ return pos; };
    Vector 		Vel()			{ return vel; };
    AngleDeg 	Ang()			{ return ang; };
    bool 			AngValid(){ return ang_valid; };
    bool			IsMe()		{ return me; };

    void Log(int level=10);
  private:
    char			side;
    Unum			num;
    Vector 		pos;
    Vector 		vel;
    AngleDeg 	ang;
    bool 			ang_valid;
    bool 			me;
  };
  //---------------------------------------------------
  //---------------------------------------------------
  class Interception {
  public:
    Interception()  { time=-1; };
    ~Interception();
    bool InitNetworks();
  public:
    void SetPlayerPosInfo();
    Intercepter& Opponent(int num)	{ return opponents[num-1]; };
    Intercepter& Teammate(int num)	{ return teammates[num-1]; };

    float ball_control(float t_cycles, float o_cycles);
    float intercept_time(Vector ballpos, Vector ballvel, Vector playerpos, float playervel, float buffer=Mem->SP_kickable_area );

    float neuro_goalie_intercept(Unum goalie, Vector kickfrom, Vector kickto, float kickspeed);
    float neuro_player_intercept(Unum player, Vector kickfrom, Vector kickto, float kickspeed);
    float neuro_goalie_intercept(Unum goalie, Vector kickfrom, Vector kickto, float threshold, float& kickspeed);
    float neuro_player_intercept(Unum player, Vector kickfrom, Vector kickto, float threshold, float& kickspeed);
  private:
    enum {
      Speeds = 4,
      Distances = 3
    };
    enum {
      Speed15 = 0,
      Speed18 = 1,
      Speed22 = 2,
      Speed25 = 3
    };
    enum {
      Dist10 = 0,
      Dist15 = 1,
      Dist20 = 2
    };
    CProcessor* shoot_nets[Speeds][Distances];
    CProcessor* pass_nets[Speeds][Distances];

    Intercepter opponents[11];
    Intercepter teammates[11];
    Time time;
  };
  //---------------------------------------------------
  //---------------------------------------------------
}//InterceptionSkills
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
template<typename A> inline void Swap(A& x, A& y){
  A tmp =x; x = y; y = tmp;
}
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
extern InterceptionSkills::Interception interception;
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
#endif//__INTERCEPTION_H_
