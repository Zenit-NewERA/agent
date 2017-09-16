/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : positioning.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: positioning.h,v 2.7 2004/06/26 08:26:08 anton Exp $
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
#ifndef POSITIONING_H
#define POSITIONING_H

#include "Formations.h"
#include "geometry.h"

/**Help data and functions for positioning on the field
 *@author Anton Ivanov
 */

class Positioning : public Formations  {
public: 
  Positioning();
  virtual ~Positioning();
  bool GoToHomePosition(Vector pos=Vector(-200.0f,-200.0f),float dash=777.0f);//false - i already at this point

  float temp_PassConf(Vector from,Vector to,Unum* opp=0);
  float DotPassConf(Vector from,Vector to,Vector vel,Unum* opp=0);
  float TmPassConf(Unum tm,Vector from,Vector vel,Unum* opp=0);
  AngleDeg GetMinOppAngleAbs(Vector from,Vector to,Unum* opp=0);
  float OppInetrceptionValue(Unum opp,Vector from,Vector to,Vector vel);
  Vector GetOptimalPointOnLine(Vector from,Vector start,Vector end,AngleDeg* res_ang=0,AngleDeg StopAng=30.0f,float step=2.0f);

  virtual Vector GetTmPos(Unum tm)=0;

  float OurPositionValue(Vector pos);
  float TheirPositionValue(Vector pos);
	
  //bottom three functions return true if doing something
  bool MoveToPos(Vector pos,float err_ang,float d,bool stop_now,float max_power);
  bool MoveToPos(Vector pos,float err_ang,float d,bool stop_now=true){return MoveToPos(pos,err_ang,d,stop_now,777.0f);}
  bool MoveToPos(Vector pos,float err_ang,float d,float max_power){return MoveToPos(pos,err_ang,d,false,max_power);}
  bool MoveBackToPos(Vector pos,float err_ang,float d,float max_power=777.0);//dvigay popoy k pos

  void MoveToPosAlongLine( Vector pos, AngleDeg ang,float dDistThr, int iSign, AngleDeg angThr, AngleDeg angCorr );
	
  void StopNow();
  //closest tm to ball
  virtual Unum SelectOptimalPlayer(int PT_mask=PT_All,int PS_mask=PS_All,Vector p=Mem->BallAbsolutePosition());
  virtual float GetFormationDash(Unum tm=Mem->MyNumber);

  void UpdateFastestTmToBall(Unum fastest,float dist,float pconf,float vconf);
  void UpdateFastestPlayers();//call only one time at begin
	
  Unum FastestTm()const{return FastestTeammateToBall;}
  Unum FastestOpp()const{return FastestOpponentToBall;}
  int TmCycles()const{return tm_cyc;}
  int OppCycles()const{return opp_cyc;}
  Vector TmPoint()const{return tm_pos;}
  Vector OppPoint()const{return opp_pos;}
  bool TmInter()const{return KnowTmInter;}
  bool OppInter()const{return KnowOppInter;}

  bool IsDefense(){return OppCycles()<=TmCycles();}
  bool IsOffense(){return OppCycles()>TmCycles();}
	
  void SetSaveStamina(bool ss){save_stamina=ss;}//VARY DANGER
  bool GetSaveStamin()const{return save_stamina;}

  int IsActivityByFastest(Unum tm)const{return Formations::IsActivityByFastest(tm,FastestTm());}

  //функции проверки валидности позиции в следующем цикле
  enum CheckType{
    CT_Normal,
    CT_Agressive,
    CT_VeryAgressive,
    CT_PenaltyShoot
  };
  bool close_goalie_intercept(Vector predict_pos);
  bool CheckWithoutTackle(Vector predPos,CheckType type=CT_Agressive,bool print_log=true,float side=1.0f,float dist_stop_dribble=0.0f);
  bool CheckTackleOpportunity(Vector BallPredPos,bool print_log=true);
  bool CheckWithTackle(Vector BallPredPos,CheckType type,bool print_log=true,float side=1.0f, float dist_stop_dribble=0.0f);
  
  Unum check_for_free_opponents_in_own_penalty_area(void);
protected:
  bool save_stamina;
  float buffer_at_point;
  bool use_formation_dash;
private:
  bool DashToPoint(Vector pos,float max_power);
  //for communicate fastest tm
  Unum FastestTmFromCommunicate;
  float pos_conf,vel_conf,dist;
  bool from_comm;
  static int hear_time;

  Unum FastestTeammateToBall;
  Unum FastestOpponentToBall;
  int tm_cyc;
  int opp_cyc;
  Vector tm_pos;
  Vector opp_pos;
  bool KnowTmInter;
  bool KnowOppInter;
};

#endif
