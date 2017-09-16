/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : shoot.h
 *
 *    AUTHOR     : Sergey Serebyakov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: shoot.h,v 2.7 2004/06/22 17:06:16 anton Exp $
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
 
#ifndef __SHOOT_H
#define __SHOOT_H


#include "client.h"
#include "kick.h"
#include "Processor.h"
#include "Interception.h"
#include "Playposition.h"
#include <queue>


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

namespace ShootSkills {
  using namespace InterceptionSkills;
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  class ShootInfo{
  private:
    Unum 		shooter;
    Vector 	position; 	//global position
    float		confidence;	//for min shoot speed at 1 cycle
    float 	speed;			//minimal shoot speed
    Unum	  goalie;
    Time 		set_time;      // time,when information was sat
    Vector from;
  public:
    Unum 		GetShooter() 	const	{ return shooter; };
    Vector	GetPosition()	const	{ return position; };
    float		GetConfidence()	const	{ return confidence; };
    float 	GetSpeed()     	const	{ return speed; };
    Unum		GetGoalie()		const	{ return goalie; };
    Time		GetTime()	   	const	{ return set_time; };
    Vector GetFrom()const{return from;}
  public:
    void		SetShooter(Unum shooter)			{ this->shooter = shooter; };
    void 		SetPosition(Vector position)		{ this->position = position; };
    void		SetConfidence(float confidence)		{ this->confidence = confidence; };
    void		SetSpeed(float speed)				{ this->speed = speed; };
    void 		SetGoalie(Unum goalie)				{ this->goalie = goalie; };
    void		SetTime(Time time)					{ this->set_time = time; };
    void SetFrom(Vector from) {this->from=from;}
    void		SetInfo(Unum shooter, Vector position, float confidence, float speed,
				Unum goalie, Time time ,Vector from) {
      SetShooter( shooter);
      SetPosition( position );
      SetConfidence( confidence );
      SetSpeed(speed);
      SetGoalie(goalie);
      SetTime( time );
      SetFrom(from);
    }
  public:
    ShootInfo() : 	shooter(Unum_Unknown),
			position(0.0f),
			confidence(0.0f),
			speed(0.0f),					
			goalie(Unum_Unknown),
			set_time(-1) ,
                        from(Vector(777.0f,777.0f)){};
    ~ShootInfo() {};
  public:
    bool Valid(Vector from=Vector(777.0f,777.0f)) {
      return ( set_time==Mem->CurrentTime &&	shooter!=Unum_Unknown &&this->from==from);
    };
    void Reset() {
      SetTime(-1);
    };
    void Log(int level) {
      Mem->LogAction7(level, "ShootInfo:pl %.0f x %.2f y %.2f sp %.2f conf %.2f", (float)shooter, position.x, position.y, speed, confidence);
      Mem->LogAction4(level, "ShootInfo:goalie %.0f pv %.2f",
		      float(goalie),
		      goalie==Unum_Unknown ? 0.0f : Mem->OpponentPositionValid(goalie));		
    };
    ShootInfo operator = ( ShootInfo info );
    bool operator<(const ShootInfo& info) const	{ return confidence<info.GetConfidence(); };
    bool operator>(const ShootInfo& info) const	{ return confidence>info.GetConfidence(); };
  };	//end of class Shootinfo
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  class Shoot : public virtual Kick {
  public:
    Shoot();
    virtual  ~Shoot() {};
    void      Initialize();
  public:
    float 	 	MyShootConf(float side_sign=1.0f,Vector from=Vector(777.0f,777.0f)) { return TeammateShootConf(Mem->MyNumber,side_sign,from); };
    AngleDeg 	MyShootAngle();
    float 	 	TeammateShootConf(Unum teammate,float side_sign=1.0f,Vector from=Vector(777.0f,777.0f));
    void 			LogShoot() { shooters[Mem->MyNumber-1].Log(10); };
    void 			shoot();
    float		  shoot_to_point(Vector from, Vector to, float& shootspeed, Unum& danger,Unum tm);

    float MaxShootDistance() 	{ return max_shoot_distance; };
    float ShootThreshold() 		{ return shoot_threshold; };
    ShootInfo explore_player(Unum teammate,float sight_sign=1.0f,Vector from=Vector(777.0f,777.0f));
    SK_Mode  SelectShootMode();
  private:
    ShootInfo not_limited_shoot(Unum teammate);

    void 			suppose_player_position(AngleDeg shootang, Vector from, Unum pl, int kick_cycles, Vector& plpos);
    float			verify_interception_info(Intercepter& intercepter, PlayerInterceptInfo& pInfo, AngleDeg shootangle);

    float     probability_of_deviation( Vector from, Vector to );
    float 	  pnorm(float value, float mo, float stddev);
    float 	  stddev(float distance);
    
  private:
    ShootInfo shooters[11];
    //shooting theshold
    float  shoot_threshold;
    //shoot speed
    float  shoot_speed;
    //maximal shooting distance
    float  max_shoot_distance;
    //the number of considered shooting routes
    int    num_considered_routes;
    //next things are for opponent's goal corners
    Vector left_corner;
    Vector right_corner;
    //some probability things
    //next array is for computing standart deviation
    float  coeffs[5];
    //opponents, which are considered as possible shoot intercepters
    vector<Unum> opponents;
    //shoot variants are stored here before choosing the best one
    vector<ShootInfo>	shootinfos;
    //time, when opponents are collected
    Time time;
  };
  ////////////
}//namespace ShootSkills
#endif
