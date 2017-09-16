/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : ClearBall.h
 *
 *    AUTHOR     : Sergey Serebyakov,Anton Ivanov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: ClearBall.h,v 2.3 2004/03/05 07:03:44 anton Exp $
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
#ifndef __CLEARBALL_H
#define	__CLEARBALL_H

#include "client.h"
#include "behave.h"
#include "types.h"
#include "kick.h"
#include "Playposition.h"
using namespace std;

class ClearBall : public virtual Kick{
private:
  struct ClearStatistica {
    inline void defense_clear()  { defense_clears++; };
    inline void offense_clear()  { offense_clears++; };
    inline void goal_clear()     { goal_clears++; };
    inline void midfield_clear() { midfield_clears++;}

    ClearStatistica() {
      defense_clears=0;
      offense_clears=0;
      goal_clears=0;
      midfield_clears=0;

    };
    ~ClearStatistica() {};
    inline void Log() {
      float all= defense_clears+offense_clears+goal_clears+midfield_clears;
      Mem->LogAction2(10,"CLEAR_STATISTICA");
      Mem->LogAction4(10,"deffense clears: %.0f (%.2f %)", float(defense_clears), float(defense_clears)/all);
      Mem->LogAction4(10,"offense  clears: %.0f (%.2f %)", float(offense_clears), float(offense_clears)/all);
      Mem->LogAction4(10,"goal     clears: %.0f (%.2f %)", float(goal_clears), float(goal_clears)/all);
      Mem->LogAction4(10,"midfield clears: %.0f (%.2f %)", float(midfield_clears), float(midfield_clears)/all);
      Mem->LogAction3(10,"total    clears: %.0f"         , all);
    };
  private:
    int defense_clears;
    int offense_clears;
    int goal_clears;
    int midfield_clears;
  };
  ClearStatistica clear_statistica;
  typedef enum CLEAR_TYPE {
    DefenseClear,
    MidfieldClear,
    OffenseClear,
    GoalClear
  } ClearType;

  struct ClearInfo {
    AngleDeg  angle;
    ClearType type;
  };
  char clear_log_message[70];

  AngleDeg defense_clear();
  AngleDeg midfield_clear();
  AngleDeg offense_clear();
  AngleDeg goal_clear();

  ClearInfo clear_info();
public:
  ClearBall() {};
  ~ClearBall() {};

  void clear_ball(bool fast_clear=false);
  inline AngleDeg clear_angle() { return clear_info().angle; };
  inline void log_clear() { clear_statistica.Log(); };
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#endif //__CLEARBALL_H
