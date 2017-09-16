/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : kick.C
 *
 *    AUTHOR     : Sergey Serebryakov
 *
 *    $Revision: 2.11 $
 *
 *    $Id: kick.C,v 2.11 2004/06/22 17:06:16 anton Exp $
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

#include <cmath>
#include "Memory.h"
#include "client.h"
#include "kick.h"
#include "behave.h"
#include "Handleball.h"

////////////////////////////////////////////////////////////////////
bool consider_posvalid = true;
////////////////////////////////////////////////////////////////////
//----------------------------------------------
AngleDeg GetNormalize360(AngleDeg angle) {
  while( angle>360 ) 	angle-=360;
  while( angle<0 ) 		angle+=360;	

  return angle;
}
//----------------------------------------------
AngleDeg GetAngleDiff(AngleDeg angle1, AngleDeg angle2) {
  angle1 = GetNormalize360(angle1);
  angle2 = GetNormalize360(angle2);
	
  AngleDeg result = fabs(angle1-angle2);
  if( result>180.0f )
    result=fabs(result-360.0f);

  return GetNormalize360(result);
}
//----------------------------------------------
//----------------------------------------------
//----------------------------------------------
void Kick::PlayerMovementCorrection(float& kickspeed, AngleDeg& kickangle) {
  //if agent is going to kick ball such way, that next cycle it hopes
  //to be still in ball possession(turn ball for example),
  //call this function, as player also can move in some direction.
  Vector newkickvel = Polar2Vector(kickspeed, kickangle+Mem->MyBodyAng())+Mem->MyVel();
  kickspeed = newkickvel.mod();
  kickangle = newkickvel.dir()-Mem->MyBodyAng();
}
//----------------------------------------------------
float Kick::kickCorrectionCoeff(Vector ballpos) {
  //Any kick, made by agent, is reduced by server.It depends on ball position
  //in kickable area around the player.This functions calculates the appropriate coeff.
  //(for more information see Soccer Server Manual, kick model chapter.)
  float distball = ballpos.dist(Mem->MyPos())-Mem->GetMyPlayerSize()-Mem->SP_ball_size;	
  float dirdiff =  fabs( GetNormalizeAngleDeg( Mem->AngleToFromBody(ballpos) ) );
  return 1.0f-0.25f*(dirdiff/180.0f)-0.25f*(distball/Mem->GetMyKickableMargin());
}
//----------------------------------------------------
float Kick::kickMaxNoise(float kickpower) {
  //As described above, kick is reduced by some value, which depends
  //on ball's relative position to player.Also server represents noise factor.
  //this function allows to find max noise interval for appropriate power
  //(for more information see Soccer Server Manual, noise chapter)
  float maxrand = Mem->GetMyKickRand()*kickpower/Mem->SP_max_power;	
  //next, kick rand vector calculated as follows:
  //Vector kicknoise(drand(-maxrand,maxrand), drand(-maxrand,maxrand));
  //where drand is a macros, defined as follows(for more information see
  //file random.h[cc] from soccer server application)
  //#define RANDOMBASE		1000
  //#define drand(h,l)	(((((h)-(l)) * ((double)(random()%RANDOMBASE) / (double)RANDOMBASE))) + (l))
  return maxrand;
}
//----------------------------------------------------
float Kick::KickPowerCorrection(float power, Vector ballpos) {
  return power/kickCorrectionCoeff(ballpos);
    
}
//----------------------------------------------------
float Kick::kickball(float kickspeed, AngleDeg kickangle, TurnKickCommand& command) {
  //kickangle is relative to player's body, calculated with func GetKickAngle!
  //This kick represents the lowest layer
  //in our agents kick model.This function trys to send ball to appropriate direction
  //with some desired speed.It can be used as for real kicks, e.i. passes, shoots
  //and stuff, as for close to player ball manipulations.It calculates kick
  //power correction coeff(see above).
  //No matter about noise, added by server.In future some day i hope i will solve this
  //problem.if ball can't be pushed to appropriate direction with desired speed, we do
  //our best with speed,but angle will be always correct, idea is from CMU.

  kickangle+=Mem->MyBodyAng();
  NormalizeAngleDeg(&kickangle);
  Vector dVel;	//velocity, we kick the ball, to give it appropriate speed
  if( Mem->BallVelocityValid() )
    dVel = Polar2Vector(kickspeed, kickangle)-Mem->BallAbsoluteVelocity();
  else
    dVel = Polar2Vector(kickspeed, kickangle);		

  AngleDeg angle = GetNormalizeAngleDeg(dVel.dir()-Mem->MyBodyAng());
  float kickpower = dVel.mod()/Mem->SP_kick_power_rate;
  kickpower = KickPowerCorrection(kickpower);	
  //let's see case, when power is too big
  if( kickpower>Mem->SP_max_power ) {
    //		Mem->LogAction3(10,"kickball - kickpower %.2f is too big, correcting", kickpower);
    AngleDeg sp_ballvel=Mem->BallAbsoluteVelocity().dir();
    AngleDeg sp_kickvel = kickangle;		
    AngleDeg dirdiff = GetAngleDiff(sp_ballvel,sp_kickvel);
    if( !Mem->BallVelocityValid() || Mem->BallSpeed()<FLOAT_EPS || dirdiff<=2.0f) {
      //	Mem->LogAction2(10,"kickball - ball vel not valid or ball speed is small or ball path is correct,check bounds for power");		
      check_bounds(kickpower,Mem->SP_min_power, Mem->SP_max_power);
    }else {
      Vector ball=Mem->BallAbsolutePosition();
      float acc=Mem->SP_max_power*Mem->SP_kick_power_rate*kickCorrectionCoeff(ball);
      dVel=Polar2Vector(acc,GetNormalizeAngleDeg(kickangle-ASin(Mem->BallAbsoluteVelocity().rotate(-kickangle).y/acc)));
      kickpower=Mem->SP_max_power;
      angle=GetNormalizeAngleDeg(dVel.dir()-Mem->MyBodyAng());
    }  
  }

  command.kick(kickpower, angle);

  float ballspeed =( Polar2Vector( command.Power()*Mem->SP_kick_power_rate*kickCorrectionCoeff(),command.Angle()+Mem->MyBodyAng() )+ Mem->BallAbsoluteVelocity() ).mod();
  return ballspeed;		
}
//----------------------------------------------------
bool Kick::ExecuteKickCommand(TurnKickCommand& command) {
  if( !command.Valid() ){
    my_error("ExecuteKickCommand- command is not valid");
    return false;
  }

  switch( command.CommandType() ) {
  case CMD_dash:
    dash(command.Power());
    break;
  case CMD_turn:
    turn(command.Angle());
    break;
  case CMD_kick:
    kick(command.Power(), command.Angle());
    break;
  default:
    if( !command.TurnNeckNeeded() ) {
      my_error("ExecuteKickCommand- unimplemented command type!");
      return false;
    }
  }

  if( command.TurnNeckNeeded() ) {
    turn_neck(command.NeckAngle());
  }
  return true;
}
//----------------------------------------------------
void Kick::SetKickData(float kickspeed, AngleDeg kickangle, SK_Mode mode) {
  this->speed = kickspeed;
  this->angle = kickangle;
  this->sk_mode = mode;
  last_kick_time = Mem->CurrentTime;
}
//----------------------------------------------------
bool Kick::KickInProgress() {
  bool res = Mem->BallKickable()&&last_kick_time==Mem->CurrentTime-1;
  //next check needs for controlling case, when ball is kickable, but desired speed
  //was achieved previous cycle, so we do not want to waste time, executing one more kick,
  //but call positioning skills

  //	Mem->LogAction4(10,"Kick::kick in progress:vel %.2f ang %.2f mode %.0f", speed, angle);
  return res&&sk_result!=SK_KickDone;
}
//----------------------------------------------------
SK_Res Kick::smartkick(float kickspeed, Vector kicktarget, SK_Mode sk_mode,bool only_calc) {
  //	this->kicktarget = kicktarget;
  //	SetKickData(kickspeed, GetKickAngle(kicktarget), sk_mode);
  return smartkick(kickspeed, GetKickAngle(kicktarget), sk_mode,only_calc);	
}
//----------------------------------------------------
SK_Res Kick::smartkick(float kickspeed, AngleDeg kickangle, SK_Mode sk_mode,bool only_calc) {
	
  //	if( last_kick_time!=Mem->CurrentTime-1 ) {
  //		sk_result = SK_DidNothing;
  //		//let's translate angle to point
  //		Ray balltravel(Mem->BallAbsolutePosition(), kickangle+Mem->MyBodyAng());
  //		kicktarget = Mem->FieldRectangle.RayIntersection(balltravel);
  //	}
	
  TurnKickCommand command;
  SK_Res kick_result;

  switch( sk_mode ) {
  case SK_Fast:
    kick_result = smartkick_fast(kickspeed, kickangle, command,only_calc);
    break;
  case SK_Safe:
    kick_result =  smartkick_safe(kickspeed, kickangle, command,only_calc);
    break;
  case SK_SetPlay:
    kick_result =  smartkick_setplay(kickspeed, kickangle, command);
    break;			
  default:
    my_error("Smart Kick - unimplemented kick type");
    kick_result = SK_DidNothing;
    break;
  }
  if( kick_result==SK_DidNothing || kick_result==SK_LostBall ) {
    //nothing to be done
  } else {
    if(!only_calc)
      ExecuteKickCommand(command);
    else{
      Mem->Action->type = CMD_kick;
      Mem->Action->power = command.Power();
      Mem->Action->angle = command.Angle();
      Mem->Action->time = Mem->CurrentTime;
    }
    
  }
  SetKickData(kickspeed, kickangle, sk_mode);
  return kick_result;
}
//----------------------------------------------------
SK_Res Kick::smartkick_fast(float kickspeed, AngleDeg kickangle, TurnKickCommand& command,bool) {
  //this skill gives out agent ability to play in a game in "one touch"
  command.Reset();
  float actualspeed;
  //	if( kicktarget==Vector(0.0f,0.0f) )
  actualspeed = kickball(kickspeed, kickangle, command);
  //	else
  //		actualspeed = kickball(kickspeed, GetKickAngle(kicktarget), command);	
  // next stuff checks for possible collisions and ball controls next cycle
  // if for any reason you would like in such cases continue controlling ball,
  // i.e. continue kicking, uncomment lines "sk_result = SK_KickInProgress;"
  // else agent, in spite of  possible ball controlling next cycle, will make
  // positioning.
  Vector ballpredpos = Mem->BallAbsolutePosition()+Polar2Vector(actualspeed, kickangle+Mem->MyBodyAng());
  if( Mem->MyPredictedPosition().dist(ballpredpos)<Mem->GetMyPlayerSize()+Mem->SP_ball_size ) {
    //possibly, collision may accure next cycle
    //Mem->LogAction2(10,"smartkick_1 - the collision may be next cycle");
    //sk_result = SK_KickInProgress;
  } else 	
    if( Mem->MyPredictedPosition().dist(ballpredpos)<Mem->GetMyKickableArea() ) {
      //possibly, next cycle i still will be in  a ball possession mode
      //Mem->LogAction2(10,"smartkick_1 - possibly ball will be kickable next cycle");
      //sk_result = SK_KickInProgress;
    }else {
      // all correct
      //sk_result = SK_KickDone;		
    }
  //	Mem->LogAction4(10,"smartkick_fast - kick with actual speed %.2f, when asked is %.2f", actualspeed, kickspeed);
  sk_result = SK_KickDone;
  return sk_result;	
}
//----------------------------------------------------
AngleDeg Kick::get_adjust_angle(AngleDeg kickangle) {
  kickangle+=Mem->MyBodyAng();
  AngleDeg adj_kickangle = GetNormalize360(kickangle);

  AngleDeg adj_angle1 = GetNormalize360( adj_kickangle+135.0f );
  AngleDeg adj_angle2 = GetNormalize360( adj_kickangle-135.0f );
		
  AngleDeg ballangle = GetNormalize360( Mem->BallAngleFromBody()+Mem->MyBodyAng() );
  AngleDeg adj_angle;

  Unum opponent = Unum_Unknown;
  AngleDeg oppang=0.0f;
  float dist = 100.0f;

  if( Mem->NumTeamlessPlayers()>0 ) {
    opponent = Unum_Teamless;
    oppang = GetNormalize360( Mem->MyBodyAng()+Mem->AngleToFromBody(Mem->ClosestTeamlessPlayerPosition()) );
    dist = Mem->DistanceTo(Mem->ClosestTeamlessPlayerPosition());
  }
  if( Mem->ClosestOpponent()!=Unum_Unknown ) {
    if( Mem->OpponentDistance(Mem->ClosestOpponent())<dist ) {
      opponent = Mem->ClosestOpponent();
      oppang = GetNormalize360( Mem->MyBodyAng()+Mem->OpponentAngleFromBody(opponent) );
    }
  }
  if( opponent==Unum_Unknown )	{
    //adjust ball to angle, that the ball moves shortest distance
    if( fabs(adj_angle1-ballangle)<fabs(adj_angle2-ballangle) )
      adj_angle = adj_angle1;
    else
      adj_angle = adj_angle2;				
  }else{
    //adjust ball such way, that opponent have not got chance to get ball
    if(/* fabs(adj_angle1-oppang)<fabs(adj_angle2-oppang) */GetAngleDiff(adj_angle1, oppang)<GetAngleDiff(adj_angle2, oppang))
      adj_angle = adj_angle2;
    else
      adj_angle = adj_angle1;						
  }

  return GetNormalizeAngleDeg( adj_angle-Mem->MyBodyAng() );
}
//----------------------------------------------------
bool Kick::accelerate_ball(AngleDeg kickangle, Vector& position) {
  Vector solution1, solution2;
  Ray ballpath(Mem->BallAbsolutePosition(), kickangle+Mem->MyBodyAng());
  int num_of_solutions = ballpath.CircleIntersect(Mem->GetMyKickableArea(), Mem->MyPredictedPosition(), &solution1, &solution2);
		
  if( num_of_solutions==0 ) {
    return false;
  }else {
    if( num_of_solutions==1 )
      position=solution1;
    else
      position=solution2;
  }
  return true;
}
//----------------------------------------------------
//so, if you are trying to kick as hard as possible, next information is for you:
// minimum ball speed can be	2.20157
// maximum ball speed can be	Mem->SP_ball_speed_max
// avarage ball speed is 		2.54177
// as hard as possible usually means ball vel is 2*Mem->SP_ball_speed_max
SK_Res Kick::smartkick_safe(float kickspeed, AngleDeg kickangle, TurnKickCommand& command,bool only_calc) {
  command.Reset();
  float actualspeed;
  //	if( kicktarget==Vector(0.0f,0.0f) )
  actualspeed = kickball(kickspeed, kickangle, command);
  //	else
  //		actualspeed = kickball(kickspeed, GetKickAngle(kicktarget), command);		

  if(!only_calc){
    Mem->LogAction4(10,"smartkick_safe:preinformation - ball speed is %.2f, kick power is %.2f", Mem->BallSpeed(), command.Power());
    Mem->LogAction4(10, "smartkick_safe:actual speed is %.2f, while requested one is %.2f", actualspeed, kickspeed);
  }
  
  if( actualspeed>=1.9f || actualspeed>=0.99*kickspeed ) {
    //actual speed is desired or lies in suitable interval
    //check for collisions.at this kick collisions are forbided.
    //so if sollision apeares, turnball to some angle
    if( (Mem->BallAbsolutePosition()+Polar2Vector(actualspeed, kickangle+Mem->MyBodyAng())).dist(Mem->MyPredictedPosition())<Mem->SP_ball_size+Mem->GetMyPlayerSize() ) {
      //so, it seemes,as collision will be, so turn ball
      AngleDeg turnangle;
      TurnKickCommand tb_command;
      turnangle = GetNormalizeAngleDeg(Mem->BallAngleFromBody()-30.0f);

      turnball(turnangle, TURN_CLOSEST, command, SK_Safe);
      if(!only_calc)
	Mem->LogAction2(10,"smartkick_safe - turning ball, cos during kick out collision may appear");
      sk_result = SK_BallAdjusted;
      return sk_result;
    } else {
      //can kick out without any problems
      if(!only_calc)
	Mem->LogAction2(10,"smartkick_safe - kicking ball out");
      sk_result = SK_KickDone;
      return sk_result;
    }
  }else{
    // it's a place to make smart kick!
    // the idea is:
    // move ball to special position, stop it if necceccery, accelerate ball,
    // and at last kick out
		
    //let's do a little hack!
    //----------------------
    if( kickspeed<=2.0f ) {
      AngleDeg ballang = Mem->BallAngleFromBody();
      if( fabs(ballang)>5.0f ) {
	turnball(0.0f, TURN_AVOID, command, SK_Safe, Mem->GetMyOptCtrlDist());
        sk_result = SK_BallAdjusted;
	return sk_result;
      }
      else {    	
	stopball(command);
	sk_result = SK_BallStoped;
	return sk_result;
      }
    }
    //----------------------       		
    //first let's see, if ball needs to be adjusted!
    AngleDeg ballangle = GetNormalize360( Mem->BallAngleFromBody()+Mem->MyBodyAng() );
    AngleDeg adj_angle = get_adjust_angle(kickangle);

    if( GetAngleDiff(adj_angle+Mem->MyBodyAng(), ballangle)>10.0f && command.Power()>0.05f*Mem->SP_max_power ) {
      //ball adjusting is needed
      turnball(GetNormalizeAngleDeg(adj_angle), TURN_CLOSEST, command, SK_Safe);
      sk_result = SK_BallAdjusted;
      if(!only_calc)
	Mem->LogAction4(10,"smartkick_safe: turning ball to angle %.2f, while kick angle is %.2f", GetNormalizeAngleDeg(adj_angle), kickangle);
      return sk_result;	
    }
    // so, if we're  here, we just need to accelerate ball, cos
    // kick out code is written above
    Vector position;
    bool can_accel = accelerate_ball(kickangle, position);

		
    if( !can_accel ) {
      //ups, somethins's wrong, just kick ball out
      if(!only_calc)
	Mem->LogAction2(10,"smartkick_safe - something's wrong with ball acceleration");
      sk_result = SK_KickDone;
      return sk_result;
    }
		
    float ballspeed = 0.8f*Mem->BallAbsolutePosition().dist(position);
    actualspeed = kickball(ballspeed, kickangle, command);	
    //    Mem->LogAction3(10, "smartkick_safe::acceleration::actual speed is %.2f", actualspeed);
//     if( actualspeed<1.0f ) {
//       Mem->LogAction4(10,"smartkick_safe : stopping ball,cos while acceleration actual speed is %.2f, while requested one is %.2f",
// 		      actualspeed, ballspeed);
//       actualspeed = stopball(command);
//       Mem->LogAction5(10,"smartkick_safe: stopping ball with power %.2f (act vel %.2f) ang %.2f", command.Power(), actualspeed, command.Angle());
//       sk_result = SK_BallStoped;
//       return sk_result;
//     }
    //   Mem->LogAction4(10,"smartkick_safe : let's rock ball, actual speed is %.2f, while requested one is %.2f", actualspeed, ballspeed);
    sk_result = SK_BallAdjusted;
    return sk_result;
  }
}
//----------------------------------------------------
SK_Res Kick::smartkick_setplay(float kickspeed, AngleDeg kickangle, TurnKickCommand& command) {
  command.Reset();
  float actualspeed;
  //	if( kicktarget==Vector(0.0f,0.0f) )
  actualspeed = kickball(kickspeed, kickangle, command);
  //	else
  //		actualspeed = kickball(kickspeed, GetKickAngle(kicktarget), command);		

  if( actualspeed>=2.0f || actualspeed>=0.99*kickspeed ) {
    sk_result = SK_KickDone;
    return sk_result;
  }

  float ballangle = Mem->BallAngleFromBody();
  if( fabs(ballangle)<5.0f ) {
    my_error("setplay_kick::can't kick that hard????");
    sk_result = SK_KickDone;
    return sk_result;
	
  }
  //so, here let's turn body to ball to get more effective kick power
  command.turn(ballangle);

  sk_result = SK_BallAdjusted;
  return sk_result;		
}
//----------------------------------------------------
bool Kick::stopball() {
  // the idea, of how to stop ball, was taken from CMUnited99 robocup team.
  // thanks them.The main thing is to kick ball such a way, that after kick
  // ball speed becomes 0.0, and kick angle will stay as before kick.
  // If agent is moving, a player movment correction is needed.
  TurnKickCommand command;
  stopball(command);
  ExecuteKickCommand(command);
  //	Mem->LogAction5(10,"Stopball - power %.2f (vel %.2f) ang %.2f", command.Power(), actualspeed, command.Angle());
  return true;
}
//----------------------------------------------------
float Kick::stopball(TurnKickCommand& command) {
  command.Reset();
  float 	kickspeed = 0.0f,
    kickangle = -Mem->BallAngleFromBody();//minus add by AI

  if( !Mem->BallVelocityValid() ) {
    //AI: ok try this
    Mem->LogAction2(10,"stopball::kick then ball vel not valid");
    kickspeed=Mem->BallDistance();
    kickangle=GetNormalizeAngleDeg((Mem->MyPos()-Mem->BallAbsolutePosition()).dir()-Mem->MyBodyAng());
  }

  PlayerMovementCorrection(kickspeed, kickangle);
  float actualspeed = kickball(kickspeed, kickangle, command);
  return actualspeed;
}
//----------------------------------------------------
bool Kick::moveball(Vector pos) {
  //here params must be ok!

  TurnKickCommand moveballcom;
  if( moveball(pos, moveballcom) ) {
    ExecuteKickCommand(moveballcom);
    return true;
  }
  return false;
}
//----------------------------------------------------
bool Kick::moveball(Vector pos, TurnKickCommand& command) {
  command.Reset();

  AngleDeg angle = GetKickAngle(pos);
  float dist = Mem->BallAbsolutePosition().dist(pos);

  PlayerMovementCorrection(dist, angle);	
  float actualspeed = kickball(dist, angle, command);
  if( actualspeed<0.95*dist || actualspeed>1.05*dist) return false;

  return true;	
}
//----------------------------------------------------
SK_Res Kick::turnball(AngleDeg angle, TurnDir dir, SK_Mode sk_mode, float tb_radius) {
  TurnKickCommand command;
  SK_Res res = turnball(angle, dir, command, sk_mode, tb_radius);

  if( res!=SK_DidNothing || res!=SK_LostBall ) {
    ExecuteKickCommand(command);
  }	

  return res;
}
//----------------------------------------------------
SK_Res Kick::turnball(AngleDeg angle, TurnDir dir, TurnKickCommand& command, SK_Mode sk_mode, float tb_radius) {

  if( dir==TURN_NONE&&sk_mode==SK_Safe ) {
    my_error("turnball - turn dir can't be none, if mode is safe");
  }
	
  command.Reset();
  NormalizeAngleDeg(&angle);	

  float kickspeed;
  float kickangle;
  float actualspeed;
  Vector tb_pos;	//turnball position
  AngleDeg tb_angle;
  SK_Res kickres;
  AngleDeg tb_angle_max;

  if( Mem->ViewWidth==VW_Narrow )
    tb_angle_max = 180.0f;
  else if( Mem->ViewWidth==VW_Normal )
    tb_angle_max = 50.0f;
  else		//even with 5.0f max angle for turnball with wide angle of view there is a great chance that the ball will be lost
    tb_angle_max = 5.0f;		

  AngleDeg ccw_angle = angle-Mem->BallAngleFromBody();
  AngleDeg cw_angle = Mem->BallAngleFromBody()-angle;

  if( cw_angle<0.0f ) cw_angle+=360.0f;
  if( ccw_angle<0.0f ) ccw_angle+=360.0f;


  if( dir==TURN_CLOSEST ) {
    if (cw_angle < ccw_angle)
      dir = TURN_CW;
    else
      dir = TURN_CCW;
  }
		
  tb_angle	= Mem->MyBodyAng()+angle;

  if( (dir==TURN_CW && cw_angle>tb_angle_max) || (dir==TURN_CCW && ccw_angle>tb_angle_max) ) {
    //trying to turn ball too big angle, correcting...
    tb_angle = Mem->MyBodyAng()+Mem->BallAngleFromBody()+dir*tb_angle_max;
		
  }
  NormalizeAngleDeg(&tb_angle);

  if( !Mem->BallKickable() || !Mem->BallPositionValid() )
    return SK_LostBall;

  tb_pos = Polar2Vector(tb_radius, tb_angle)+Mem->MyPos();				
  kickangle = GetKickAngle(tb_pos);
  kickspeed = Mem->BallAbsolutePosition().dist(tb_pos);

  PlayerMovementCorrection(kickspeed, kickangle);
  actualspeed = kickball(kickspeed, kickangle, command);

  if( sk_mode==SK_Fast ) {
    //execute at once. with big turn ball angle in case of restricted kick power
    //collisions may accure.it happens when ball vel is big, and when it has
    // opposite angles with kick angle.
    kickres = SK_KickDone;
  } else if( sk_mode==SK_Safe ){
    if( actualspeed<0.95*kickspeed || command.Power()>0.92f*Mem->SP_max_power ) {
      //can't turn ball to appropriate direction with only one kick
      //in case of current big ball velocity
      //so just stop ball
			
      //recuirsive variant
      //turnball(Mem->BallAngleFromBody()+dir*10.0f, dir, command, sk_mode);	
      //but do not want, let's do another way
      tb_pos = Polar2Vector(tb_radius, GetNormalizeAngleDeg(Mem->MyBodyAng()+Mem->BallAngleFromBody()))+Mem->MyPos();				
      kickangle = GetKickAngle(tb_pos);
      kickspeed = Mem->BallAbsolutePosition().dist(tb_pos);

      PlayerMovementCorrection(kickspeed, kickangle);
      actualspeed = kickball(kickspeed, kickangle, command);

      kickres = SK_BallAdjusted; 	
    }else{
      kickres = SK_KickDone;
    }	
  } else {
    my_error("turnball - unimplemented kick mode");
    kickres = SK_DidNothing;
  }

  if( kickres!=SK_DidNothing ) {
    if( Mem->TimeToTurnForScan() ) {
      //let's try to turn neck to point ballpos	
      AngleDeg turnangle =
	Mem->PredictedPointRelAngFromBodyWithQueuedActions(tb_pos);
      if( fabs(GetNormalizeAngleDeg(Mem->MyNeckRelAng()-turnangle)) < 1.0f ) {
	//the turn neck is not needed.we are already close enouth
      } else {
	if( Mem->CanFaceAngleFromBodyWithNeck(turnangle) ) {
	  //turn neck
	  command.turnneck( GetNormalizeAngleDeg(turnangle - Mem->MyNeckRelAng()) );
	}	
	//can't turn neck, do something else
      }
    }	//TimeToTurnForScan()
  }	//kickres!=SK_DidNothing
  return kickres;
}
//----------------------------------------------------
float Kick::maxspeed_at1kick(AngleDeg kickangle) {
  TurnKickCommand command;
  float cando = kickball(2*Mem->SP_ball_speed_max, kickangle, command);
  //	Mem->LogAction4(10, "maxspeed_at1kick::can reach vel %.2f at angle %.2f", cando, kickangle);
  return cando;
}
//----------------------------------------------------
int Kick::get_kick_cycles(float kickspeed, AngleDeg kickangle) {
	
  if( maxspeed_at1kick(kickangle)>=0.9f*kickspeed ) return 1;
  if( kickspeed<2.1f ) return 3;	

  return 4;
}
//----------------------------------------------------
// checks only next cycle after kick, which lasts only one cycle
bool Kick::can_kickout(float kickspeed, AngleDeg kickangle) {
  //	Mem->LogAction4(10, "can_kickout::kickspeed %.2f kickangle %.2f", kickspeed, kickangle);
  int num_kickcycles = get_kick_cycles(kickspeed, kickangle);
  Unum opponents[11];
  int num_opp = Mem->SortPlayersByDistanceToPoint('t', Mem->BallAbsolutePosition(), &opponents[0]);
  if( num_kickcycles==1 ) {
    TurnKickCommand kickoutcom;
    float actualspeed = kickball(kickspeed, kickangle, kickoutcom);
    Vector ballpos = Mem->BallAbsolutePosition()+Polar2Vector(actualspeed, GetNormalizeAngleDeg(kickangle+Mem->MyBodyAng()));
    if( Mem->NumTeamlessPlayers()>0 ) {
      Vector teamless_pos=Mem->ClosestTeamlessPlayerPosition();
      if( teamless_pos.dist(ballpos)<Mem->SP_kickable_area+0.2f ) return false;
    }
    for( int opponent=0; opponent<num_opp; opponent++ )	 {
      Vector opppos = Mem->OpponentAbsolutePosition(opponents[opponent]);
      if( opppos.dist(ballpos)<Mem->GetOpponentKickableArea(opponents[opponent])+Mem->GetOpponentPlayerSpeedMax(opponents[opponent])/2 ) return false;
    }
    return true;
  }
  return false;
}
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
void Grid::Initialize() {
  int circles, angles;

  float mindist = Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.2f;
  float maxdist = Mem->GetMyKickableArea()-0.2f;

  float stepdist = (maxdist-mindist)/(float)CIRCLES;
  float stepangle = (360.0f-(float)ANGLES)/(float)ANGLES;

  float kickeffect;
  float distball, dirdiff;
  Vector position;

  for( circles=0; circles<(int)CIRCLES; circles++ ) {
    for( angles=0; angles<(int)ANGLES; angles++ ) {
      position = Polar2Vector(circles*stepdist+mindist,GetNormalizeAngleDeg((AngleDeg)stepangle*angles));

      distball = position.dist(Vector(0.0f, 0.0f))-Mem->GetMyPlayerSize()-Mem->SP_ball_size;	
      dirdiff =  fabs( GetNormalizeAngleDeg((AngleDeg)stepangle*angles) );

      kickeffect = 1.0f-0.25f*(dirdiff/180.0f)-0.25f*(distball/Mem->GetMyKickableMargin());

      cells[circles][angles].SetPos(position);
      cells[circles][angles].SetSpeed(Mem->SP_max_power*Mem->SP_kick_power_rate*kickeffect);
      cells[circles][angles].Unlock();			
    }
  }
}
//----------------------------------------------------
void Grid::Move(Vector dir) {
  int circles, angles;

  for( circles=0; circles<(int)CIRCLES; circles++ ) {
    for( angles=0; angles<(int)ANGLES; angles++ ) {
      cells[circles][angles].Move(dir);
    }
  }
}
//----------------------------------------------------
void Grid::Rotate(AngleDeg angle) {
  int circles, angles;

  for( circles=0; circles<(int)CIRCLES; circles++ ) {
    for( angles=0; angles<(int)ANGLES; angles++ ) {
      cells[circles][angles].Rotate(angle);
    }
  }
}
//----------------------------------------------------
void Grid::SetGridinfo() {
  Initialize();
  Rotate(Mem->MyBodyAng());
  Move(Mem->MyPos());

  time = Mem->CurrentTime;
  all_unlocked = true;


  Rectangle shrinked_field = Mem->FieldRectangle;
  shrinked_field=shrinked_field.shrink(0.1f);		

  int circles, angles;	

  //let's see about line limits
  for( circles=0; circles<(int)CIRCLES; circles++ ) {
    for( angles=0; angles<(int)ANGLES; angles++ ) {
      if( !shrinked_field.IsWithin( cells[circles][angles].Position() ) ) {
	cells[circles][angles].Lock();
	all_unlocked = false;
      }
    }
  }

  //now let's see about opponents
  int num_opponents = 3;		
	
  Unum opponents[11];
  int num_opp = Mem->SortPlayersByDistanceToPoint('t', Mem->MyPos(), &opponents[0]);	
  num_opponents = Min(num_opponents, num_opp);
  int pos;
  Unum opponent;

  for( pos=0; pos<num_opponents; pos++ ) {
    opponent = opponents[pos];	
    float feel_distance = Mem->GetOpponentKickableArea(opponent)+Mem->GetOpponentPlayerSpeedMax(opponent)/2;
    float dist_to_me=Mem->OpponentDistance(opponent);
    if( dist_to_me>=feel_distance ) continue;
    //so, opponent can limit some points, so let's see which		
    for( circles=0; circles<(int)CIRCLES; circles++ ) {
      for( angles=0; angles<(int)ANGLES; angles++ ) {
	if( cells[circles][angles].Position().dist(Mem->OpponentAbsolutePosition(opponent))<=feel_distance ) {
	  cells[circles][angles].Lock();
	  all_unlocked = false;					
	}
      }
    }
    //-----------------
  }
}
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
bool GridKick::can_move_to_pos(Vector targetpos) {
  AngleDeg ang = GetKickAngle(targetpos);
  float speed = Mem->BallAbsolutePosition().dist(targetpos);

  float speed1kick = maxspeed_at1kick(ang);
  if( speed1kick>=0.95*speed ) return true;
	
  return false;
}
//----------------------------------------------------
bool GridKick::GetKick(Vector& position, AngleDeg angle, float speed) {
  //grid must be initialized here!!!!
  int circles, angles;
  Vector ballpos = Mem->BallAbsolutePosition();
  //first, find closest position from grid!

  for( circles=0; circles<grid.GetCircles(); circles++ ) {
    for( angles=0; angles<grid.GetAngles(); angles++ ) {
      if( grid.cells[circles][angles].Locked() ) continue;
      if( !can_move_to_pos(grid.cells[circles][angles].Position()) ) continue;
      Vector vel = grid.cells[circles][angles].Position()-ballpos;
      Vector needvel = Polar2Vector(speed, angle+Mem->MyBodyAng());			
      Vector mustkick = needvel-vel;
      if( mustkick.mod()>0.95*grid.cells[circles][angles].GetSpeed() ) {
	position =grid.cells[circles][angles].Position();
	return true;
      }
    }
  }

		

	
  return false;
}
//----------------------------------------------------
SK_Res GridKick::gridkick(float speed, AngleDeg angle, SK_Mode mode) {
  if( mode==SK_SetPlay || mode==SK_Fast) {
    Mem->LogAction2(10, "smartkick(grid)::fast or setplay kick");
    return smartkick(speed, angle, mode);
  }

  grid.Initialize();
  grid.Rotate(Mem->MyBodyAng());
  grid.Move(Mem->MyPos());

  if( grid.UnLocked() ) return smartkick(speed, angle, mode);

  Vector position;	
	
  float speed1kick = maxspeed_at1kick(angle);
  if( speed1kick>=0.95*speed || speed1kick>=2.0f) {
    Mem->LogAction2(10,"smartkick(grid)::kick out kick");
    return smartkick(speed, angle, SK_Fast);	
  }
											
  if( !GetKick(position, angle, speed) ) {	
    Mem->LogAction2(10,"smartkick(grid)::grid fails");
    return smartkick(speed, angle, mode);
  }

  SetKickData(speed, angle, mode);

  speed = Mem->BallAbsolutePosition().dist(position);
  angle = GetKickAngle(position);

  Mem->LogAction4(10,"smartkick(grid)::moving ball to position x %.2f y %.2f", position.x, position.y);	

  return smartkick(speed, angle, SK_Fast);
}
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
//----------------------------------------------------
////////////////////////////////////////////////////////////////////
float GetPowerForBreakaway2Point(Vector point) {
  int my_cyc = Mem->TeammatePredictedCyclesToPoint(Mem->MyNumber,point,100.0f,0.5f);
  if( my_cyc <= 0 )	return 0.0f;

  int tmp;
  float i;
  int difference = 100;
  float speed = 0.0f;
  for(i=3.0f;i>=0.0f;i-=0.3f) {
    int ball_cyc =
      (int)(ceil(SolveForLengthGeomSeries(i, Mem->SP_ball_decay,
					  Mem->BallAbsolutePosition().dist(point))));
    if(abs(my_cyc-ball_cyc) <= difference ) {
      difference =abs(my_cyc-ball_cyc);
      speed = i;
      tmp = ball_cyc;
    }
  }
  return speed;
}

////////////////////////////////////////////////////////////////////
float leftFunc(float x){
  return Mem->SP_penalty_area_width/4+x*0.3;
}
////////////////////////////////////////////////////////////////////
double kick_to_corner_power(void){//true - we make kick else return false
  if(!Mem->BallKickable()){
    my_error("Can not kick to corner when ball not kickable!");
    return false;
  }
  const float max_pow_to_use=Mem->SP_max_power;
  PlayerInterceptInfo myInfo;
  int max_lookahead=Mem->CP_max_int_lookahead;
  Vector BallVelmodif,BallVel,BallPos;

  float myY=Mem->MyY();
  Vector target(Mem->SP_pitch_length/2-5,Mem->SP_pitch_width/2-5);
  if(myY<0){
    myY=-myY;
    target.y=-target.y;
  }
  float ang=Mem->AngleToGlobal(target);
  if (Mem->BallVelocityValid())
    BallVel = Mem->BallAbsoluteVelocity();
  else
    BallVel = Vector(0,0);

  for(float power=100.0;power>=30.0;power-=5.0){
    BallVelmodif = BallVel+Polar2Vector( Mem->BallKickRate()*power, ang );
    BallPos=Mem->BallPredictedPosition(1,power,ang-Mem->MyBodyAng());

    myInfo = Mem->CloseBallInterception(max_pow_to_use, max_lookahead,
					BallPos, BallVelmodif);
    if (myInfo.res == BI_None)
      myInfo=Mem->ActiveCanGetThere(max_pow_to_use,max_lookahead,
				    BallPos,BallVelmodif,
				    Mem->MySide,Mem->MyNumber,
				    Mem->MyPos(),Mem->MyVel(),Mem->MyBodyAng(),TRUE,TRUE);
    if(Mem->IsSuccessRes(myInfo.res) && (Mem->MyPos()-target).mod()>=((Mem->MyPos()-myInfo.pos).mod()-1.0)){
      Mem->LogAction3(20,"kick_to_corner_power:Kick to corner with target power:%f",power);
      return BallVelmodif.mod();			
      //smart_kick_hard_abs(ang,Mem->BestKickMode,BallVelmodif.mod(),TURN_AVOID);
							
      Mem->pass_to_myself=true;
      actions.smartkick(BallVelmodif.mod(), target, SK_Fast);
      return true;
    }else if(Mem->IsSuccessRes(myInfo.res)&&((Mem->MyPos()-target).mod()<((Mem->MyPos()-myInfo.pos).mod()-1.0))){
      Mem->LogAction4(20,"kick_to_corner_power:We have such thing:%f<%f",(Mem->MyPos()-target).mod(),(float)((Mem->MyPos()-myInfo.pos).mod()-1.0));
    }
    /*		Mem->LogAction4(20,"Kick to point in corner:(%f,%f)",target.x,target.y);
		smart_kick_hard_abs(ang,Mem->BestKickMode(ang),GetPowerForBreakaway2Point(target),TURN_AVOID);
		//		kick_ball(ang,Mem->BestKickMode(ang),GetPowerForBreakaway2Point(target),TURN_AVOID);
		Mem->pass_to_myself=true;
		Mem->time_set_pass_to_myself=Mem->CurrentTime;
		return true;
    */		
  }
  return false;
}
///////////////////////////////////////////////////////////////////
bool kick_to_corner_at_begin(void){//true - we make kick else return false
  if(!Mem->BallKickable()){
    my_error("Can not kick to corner then ball not kickable!");
    return false;
  }
  const float max_pow_to_use=Mem->SP_max_power;
  PlayerInterceptInfo myInfo;
  int max_lookahead=Mem->CP_max_int_lookahead;
  Vector BallVelmodif,BallVel,BallPos;

  float myY=Mem->MyY();
  Vector target=Vector(Mem->SP_pitch_length/2-5,myY);
  if(fabs(myY)<15.0){
    target=Vector(Mem->SP_pitch_length/2-5,20.0);
    if(myY<0)
      target.y=-target.y;
  }
  if(myY<0){
    myY=-myY;
  }
  float ang=Mem->AngleToGlobal(target);
  if (Mem->BallVelocityValid())
    BallVel = Mem->BallAbsoluteVelocity();
  else
    BallVel = Vector(0,0);

  if(Mem->MyX()<(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length) && Mem->MyX()>0.0)
    if(myY<Mem->SP_pitch_width/2 && myY>leftFunc(Mem->MyX())){
      Unum opp=Mem->ClosestOpponentTo(target);
      if((opp!=Unum_Unknown &&Mem->OpponentPositionValid(opp)&& Mem->OpponentDistanceTo(opp,target)<(Mem->DistanceTo(target)-5.0))||
	 Mem->NumOpponentsInCone(0.6,target,Mem->MyPos())>0){
	return false;
      }
      for(float power=100.0;power>=30.0;power-=5.0){
	BallVelmodif = BallVel+Polar2Vector( Mem->BallKickRate()*power, ang );
	BallPos=Mem->BallPredictedPosition(1,power,ang-Mem->MyBodyAng());

	myInfo = Mem->CloseBallInterception(max_pow_to_use, max_lookahead,
					    BallPos, BallVelmodif);
	if (myInfo.res == BI_None)
	  myInfo=Mem->ActiveCanGetThere(max_pow_to_use,max_lookahead,
					BallPos,BallVelmodif,
					Mem->MySide,Mem->MyNumber,
					Mem->MyPos(),Mem->MyVel(),Mem->MyBodyAng(),TRUE,TRUE);
	if(Mem->IsSuccessRes(myInfo.res) && (Mem->MyPos()-target).mod()>=((Mem->MyPos()-myInfo.pos).mod()-1.0)){
          Mem->LogAction3(20,"kick_to_corner_at_begin:Kick to corner with target power:%f",power);
	  actions.smartkick(BallVelmodif.mod(), ang-Mem->MyBodyAng(), SK_Fast);

	  Mem->pass_to_myself=true;
	  Mem->time_set_pass_to_myself=Mem->CurrentTime;
	
	  return true;
	}else if(Mem->IsSuccessRes(myInfo.res)&&((Mem->MyPos()-target).mod()<((Mem->MyPos()-myInfo.pos).mod()-1.0))){
	  Mem->LogAction4(20,"kick_to_corner_at_begin:We have such thing:%f<%f",(Mem->MyPos()-target).mod(),(float)((Mem->MyPos()-myInfo.pos).mod()-1.0));
	}
      }
      /*		Mem->LogAction4(20,"Kick to point in corner:(%f,%f)",target.x,target.y);
			smart_kick_hard_abs(ang,Mem->BestKickMode(ang),GetPowerForBreakaway2Point(target),TURN_AVOID);
			//		kick_ball(ang,Mem->BestKickMode(ang),GetPowerForBreakaway2Point(target),TURN_AVOID);
			Mem->pass_to_myself=true;
			Mem->time_set_pass_to_myself=Mem->CurrentTime;
			return true;
      */		
    }
  return false;
}
//////////////////////////////////////////////////////////////////////////////
bool kick_to_corner(void){//true - we make kick else return false
  if(!Mem->BallKickable()){
    my_error("Can not kick to corner then ball not kickable!");
    return false;
  }
  const float max_pow_to_use=Mem->SP_max_power;
  PlayerInterceptInfo myInfo;
  int max_lookahead=Mem->CP_max_int_lookahead;
  Vector BallVelmodif,BallVel,BallPos;

  float myY=Mem->MyY();
  Vector target(Mem->SP_pitch_length/2-5,Mem->SP_pitch_width/2-5);
  if(myY<0){
    myY=-myY;
    target.y=-target.y;
  }
  float ang=Mem->AngleToGlobal(target);
  if (Mem->BallVelocityValid())
    BallVel = Mem->BallAbsoluteVelocity();
  else
    BallVel = Vector(0,0);

  if(Mem->MyX()<(Mem->SP_pitch_length/2-Mem->SP_penalty_area_length) && Mem->MyX()>0.0)
    if(myY<Mem->SP_pitch_width/2 && myY>leftFunc(Mem->MyX())){
      Unum opp=Mem->ClosestOpponentTo(target);
      if((opp!=Unum_Unknown &&Mem->OpponentPositionValid(opp)&& Mem->OpponentDistanceTo(opp,target)<(Mem->DistanceTo(target)-10.0))||
	 Mem->NumOpponentsInCone(0.5,target,Mem->MyPos())>1){
	return false;
      }
      for(float power=100.0;power>=30.0;power-=5.0){
	BallVelmodif = BallVel+Polar2Vector( Mem->BallKickRate()*power, ang );
	BallPos=Mem->BallPredictedPosition(1,power,ang-Mem->MyBodyAng());

	myInfo = Mem->CloseBallInterception(max_pow_to_use, max_lookahead,
					    BallPos, BallVelmodif);
	if (myInfo.res == BI_None)
	  myInfo=Mem->ActiveCanGetThere(max_pow_to_use,max_lookahead,
					BallPos,BallVelmodif,
					Mem->MySide,Mem->MyNumber,
					Mem->MyPos(),Mem->MyVel(),Mem->MyBodyAng(),TRUE,TRUE);
	if(Mem->IsSuccessRes(myInfo.res) && (Mem->MyPos()-target).mod()>=((Mem->MyPos()-myInfo.pos).mod()-1.0)){
	  Mem->LogAction3(20,"kick_to_corner:Kick to corner with target power:%f",power);


	  actions.smartkick(BallVelmodif.mod(), ang-Mem->MyBodyAng(), SK_Fast);
	  Mem->pass_to_myself=true;
	  Mem->time_set_pass_to_myself=Mem->CurrentTime;

	  return true;
	}else if(Mem->IsSuccessRes(myInfo.res)&&((Mem->MyPos()-target).mod()<((Mem->MyPos()-myInfo.pos).mod()-1.0))){
	  Mem->LogAction4(20,"kick_to_corner:We have such thing:%f<%f",(Mem->MyPos()-target).mod(),(float)((Mem->MyPos()-myInfo.pos).mod()-1.0));
	}
      }
    }
  return false;
}
/* should be used when we have free kicks and stuff */
/* kick_ang: absolute angle for the direction that the ball will be kicked */
/* returns whether we are in the right place or not */
Bool go_to_static_ball(float kick_ang)
{
  Line l;

  Mem->LogAction5(50, "go_to_static_ball: ball at (%.1f, %.1f) for kick angle %.2f",
		  Mem->BallX(), Mem->BallY(), kick_ang);
  
  if (!Mem->BallPositionValid())
    my_error("go_to_static_ball: lost ball");

  /* we can be pretty tolerant of angle errors, but distance errors start to matter
     real quickly */
  float dist=(Mem->PlayMode==PM_My_Kick_In?Mem->CP_hardest_kick_ball_dist:Mem->GetMyPlayerSize()+Mem->SP_ball_size+0.1f);
  Vector targ_pt = Mem->BallAbsolutePosition() +
    Polar2Vector(dist , kick_ang);

  /* we want to try and face the ball at all times */
  turn_neck(Mem->LimitTurnNeckAngle(Mem->BallAngleFromNeck()));
  
  if (Mem->DistanceTo(targ_pt) <= Mem->CP_static_kick_dist_err) {
    if(Mem->MySpeed()>0){
      Pos.StopNow();
      return FALSE;
    }
    return TRUE;
  }

  /* if we are real far from the ball, just use the regular go_to_point */
  if (Mem->BallDistance() > 2 * Mem->GetMyKickableArea()) {
    Mem->LogAction2(60, "go_to_static_ball: far away, using go_to_point");
    float power=Mem->MyStamina()<Mem->SP_stamina_max*0.7?Mem->GetMyStaminaIncMax():Mem->SP_max_power;//add by AI
    if (go_to_point(targ_pt, 0 /* no buffer */, power)!= AQ_ActionQueued)
      my_error("go_to_static_ball: far away, why didn't go_to_point do anything?");
    return FALSE;
  }

  /* make sure that we go around the ball */
//   l.LineFromTwoPoints(Mem->MyPos(), targ_pt);
//   Vector proj_ball_pt = l.ProjectPoint(Mem->BallAbsolutePosition());
//   if (proj_ball_pt.dist(Mem->BallAbsolutePosition()) <=
//       Mem->GetMyPlayerSize() + Mem->SP_ball_size + Mem->CP_collision_buffer &&
//       l.InBetween(proj_ball_pt, Mem->MyPos(), targ_pt)) {
//     /* we'll do a 90 degree dodge -we always go right */
//     Vector dodge_pt = Mem->MyPos() +
//       Polar2Vector(Mem->GetMyPlayerSize(), Mem->BallAngleFromBody() + Mem->MyBodyAng() + 90);
//     Mem->LogAction2(60, "go_to_static_ball: dodging the ball");
//     if (go_to_point(dodge_pt, 0 /* no buffer */, Min(Mem->GetMyStaminaIncMax(),Mem->SP_max_power))
// 	!= AQ_ActionQueued)
//       my_error("go_to_static_ball: dodging, why didn't go_to_point do anything?");
//     return FALSE;
//   }

  /* now we need to get to the target_point */
  /* first see if we need to turn */
  l.LineFromRay(Mem->MyPos(), Mem->MyBodyAng());
  float ang = Mem->AngleToFromBody(targ_pt);
  if (fabs(ang) > 90 ||
      (l.dist(targ_pt) > Mem->CP_static_kick_dist_err &&
       ang > Mem->CP_static_kick_ang_err)) {
    if(Mem->MySpeed()>0){
      Pos.StopNow();
      return FALSE;
    }
    Mem->LogAction2(60, "go_to_static_ball: turning to target_point");
    turn(Mem->AngleToFromBody(targ_pt));
    return FALSE;
  }
  
  /* now calculate the speed we should be going to land right on the point */
  float targ_speed =
    SolveForFirstTermInfGeomSeries(Mem->GetMyPlayerDecay(), Mem->DistanceTo(targ_pt));
  float dash_pow =
    MinMax(-Mem->GetMyStaminaInc() / 2,
	   (targ_speed - Mem->MySpeed()) / Mem->GetMyDashPowerRate(),
	   Mem->GetMyStaminaInc());
  Mem->LogAction5(60, "go_to_static_ball: targ_speed: %.2f\tMySpeed: %.2f\tdash_pow: %.2f",
		  targ_speed, Mem->MySpeed(), dash_pow);
  if (fabs(dash_pow) > 1) {
    dash(dash_pow);
    return FALSE;
  }

  return TRUE;
}
///////////////////////////////////////////////////////////////
/*
  template<typename A> void Swap(A& element1,A& element2) {
  A temp_element = element1;
  element1=element2;
  element2=temp_element;
  }
*/
//----------------------------------------------
template<typename A,typename B> void Sort(A* elements,B* keys,int size) {
  bool replaced = true;
  int i;
  while(replaced==true) {
    replaced = false;
    for(i=1;i<=size;i++) {
      if( elements[i-1] > elements[i] ) {
	Swap(elements[i-1],elements[i]);
	Swap(keys[i-1],keys[i]);
	replaced = true;
      }
    }
  }
}
//----------------------------------------------
void Sort( float* elements,int* keys,int size ) {
  bool replaced = true;
  int i;
  while(replaced==true) {
    replaced = false;
    for(i=1;i<=size;i++) {
      if( elements[i-1] > elements[i] ) {
	Swap(elements[i-1],elements[i]);
	Swap(keys[i-1],keys[i]);
	replaced = true;
      }
    }
  }


}
//----------------------------------------------
ActionQueueRes go_to_point_with_speed(Vector p, float velocity , float buffer, DodgeType dodge) {
  Mem->LogAction3(10,"GO_TO_POINT::My speed is %f",Mem->MyVel().mod());
  if( velocity<=0.0f || velocity >=1.0f ) {
    my_error("New velocity must be valid here");
    return AQ_ActionNotQueued;
  }
  float speed = Mem->MyVel().mod();
  if( speed<=0.0f || speed >=1.0f )
    my_error("My velocity must be valid here??");
		

  if( speed >= velocity ) {
    Mem->LogAction4(10,"My speed is %f,while desired speed is %f.Doing nothing",
		    speed, velocity);
    return AQ_ActionNotQueued;	
  }
  Vector my_vel = Mem->MyVel(), new_vel;
  float my_dash = 0;
  float dash;
  for(dash = 100.0f;dash>=0.0f;dash -= 5.0f) {
    new_vel = Mem->NewVelFromDash(my_vel, dash);
    Mem->LogAction4(10,"GO_TO_POINT: dash %f, new vel %f",dash,new_vel.mod());
    if( new_vel.mod()>velocity ) continue;
    my_dash = dash;
    break;
  }
  Mem->LogAction3(10,"GO_TO_POINT::New dash is %f",my_dash);		
  return go_to_point(p, buffer, my_dash, dodge);	
}
//----------------------------------------------
int BallPredictedCyclesToPoint(Vector target, float ball_vel, int cycles_buffer) {
  float ball_travel = Mem->DistanceTo(target);
  int ball_cycles = int(log((ball_vel+ball_travel*(Mem->SP_ball_decay-1))/ball_vel)/log(Mem->SP_ball_decay));	
  return ball_cycles+cycles_buffer;
}
//----------------------------------------------
int OpponentPredictedCyclesToIntercept(Vector target, float ball_vel, Unum opponent) {
  //	Mem->LogAction3(10,"___________________Prediction of opponent %.0fcycles to intercept",(float)opponent);
  Vector ball_pos = Mem->BallAbsolutePosition();
  AngleDeg ang = Mem->AngleToFromBody(target)+Mem->MyBodyAng();
  Vector opp_pos = Mem->OpponentAbsolutePosition(opponent);
  /*
    if( !InBetween(opp_pos, Mem->MyPos(), target) ) {
    return (int)(opp_pos.dist(target)/(0.75*Mem->GetOpponentPlayerSpeedMax(opponent))+0.5f);
    }
  */
  Vector current_ball_pos = ball_pos;
  float ball_velocity = Max(ball_vel, 2.4f);
  int ball_cycles = 0;
  int opponent_cycles;
  //	Mem->LogAction4(10,"___________________from pos (%.1f, %.1f)", ball_pos.x, ball_pos.y);
  bool stop = false;

  while(stop==false) {
    current_ball_pos = current_ball_pos+Polar2Vector(ball_velocity, ang);		
    ball_velocity=ball_velocity*Mem->SP_ball_decay;
    ball_cycles++;
    //		opponent_cycles = Mem->OpponentPredictedCyclesToPoint(opponent, current_ball_pos, 0.9*Mem->SP_max_power, Mem->GetOpponentKickableArea(opponent));
    opponent_cycles = (int)(opp_pos.dist(current_ball_pos)/(0.75*Mem->GetOpponentPlayerSpeedMax(opponent))+0.5f);
    //		Mem->LogAction6(10,"___________________pt (%.1f, %.1f) ball %.0f opp %.0f",current_ball_pos.x, current_ball_pos.y, (float)ball_cycles, (float)opponent_cycles );
    if( opponent_cycles<=ball_cycles ) {
      //        	Mem->LogAction4(10,"___________________opp can intercept early at pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
    if( !InBetween(current_ball_pos, ball_pos, target) ) {
      //            Mem->LogAction4(10,"___________________opp can't intercept early to pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
    if( (current_ball_pos-target).mod()<=Mem->GetOpponentKickableArea(opponent) ) {
      //            Mem->LogAction4(10,"___________________ball faster get's to pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
  }
  return opponent_cycles;
}

int PredictedCyclesToIntercept( Vector from, Vector to, float ball_vel, Unum opponent) {
  //	Mem->LogAction3(10,"___________________Prediction of opponent %.0fcycles to intercept",(float)opponent);
  Vector ball_pos = from;
  AngleDeg ang = (to-from).dir();
  Vector opp_pos = Mem->OpponentAbsolutePosition(opponent);
  float at_pt_buffer = (opponent==Mem->TheirGoalieNum) ? 2*Mem->GetOpponentKickableArea(opponent) : Mem->GetOpponentKickableArea(opponent);

  Vector current_ball_pos = ball_pos;
  float ball_velocity = Max(ball_vel, 2.4f);
  int ball_cycles = 0;
  int opponent_cycles;
  //	Mem->LogAction4(10,"___________________from pos (%.1f, %.1f)", ball_pos.x, ball_pos.y);
  bool stop = false;

  while(stop==false) {
    current_ball_pos = current_ball_pos+Polar2Vector(ball_velocity, ang);		
    ball_velocity=ball_velocity*Mem->SP_ball_decay;
    ball_cycles++;
    //		opponent_cycles = Mem->OpponentPredictedCyclesToPoint(opponent, current_ball_pos, 0.9*Mem->SP_max_power, Mem->GetOpponentKickableArea(opponent));
    opponent_cycles = (int)(opp_pos.dist(current_ball_pos)/(0.80*Mem->GetOpponentPlayerSpeedMax(opponent))+0.5f);
    //		Mem->LogAction6(10,"___________________pt (%.1f, %.1f) ball %.0f opp %.0f",current_ball_pos.x, current_ball_pos.y, (float)ball_cycles, (float)opponent_cycles );
    if( opponent_cycles<=ball_cycles ) {
      //        	Mem->LogAction4(10,"___________________opp can intercept early at pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
    if( !InBetween(current_ball_pos, ball_pos, to) ) {
      //            Mem->LogAction4(10,"___________________opp can't intercept early to pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
    if( (current_ball_pos-to).mod()<=at_pt_buffer ) {
      //            Mem->LogAction4(10,"___________________ball faster get's to pt (%.1f, %.1f)", current_ball_pos.x, current_ball_pos.y);
      stop = true;
    }
  }
  return opponent_cycles;
}
//----------------------------------------------------
//--------------------------------------------------------------
template<typename Class> void  check_bounds(Class& x, Class min, Class max) {
  x = x < max ? x : max;
  x = x > min ? x : min;
}
//--------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Visualization::GraphicLogSystem logGr;

namespace Visualization {

  void WorldModel::ResetDynamicInfo() {
    ball.posValid=0;
    ball.velValid=0;

    for( int id=0;id<11;id++ ) {
      teammates[id].posValid=0;
      teammates[id].velValid=0;
      teammates[id].side=Mem->MySide;

      opponents[id].posValid=0;
      opponents[id].velValid=0;
      opponents[id].side=Mem->TheirSide;
    }
  }

  void WorldModel::ClearGraphicInfo() {
    circles.clear();
    lines.clear();
    points.clear();
    strings.clear();
  }

#if defined USE_AGENT_LIBRARY
  void Graphics::CheckCoordinates() {
    unsigned int id;

    if( Mem->MySide=='l' ) {
      worldModel.Ball().y*=-1;
      for( int id=0;id<11;id++ ) {
	worldModel.Teammate(id).y*=-1;
	worldModel.Opponent(id).y*=-1;
      }		
    }else{
      worldModel.Ball().x*=-1;
      for( int id=0;id<11;id++ ) {
	worldModel.Teammate(id).x*=-1;
	worldModel.Opponent(id).x*=-1;
      }		
    }

    for( id=0;id<worldModel.Lines().size();id++ ) {
      if( Mem->MySide=='l' ) {
	worldModel.GetLine(id).y1*=-1;
	worldModel.GetLine(id).y2*=-1;
      }else{
	worldModel.GetLine(id).x1*=-1;
	worldModel.GetLine(id).x2*=-1;
      }
    }

    for( id=0;id<worldModel.Circles().size();id++ ) {
      if( Mem->MySide=='l' ) {
	worldModel.GetCircle(id).y*=-1;
      }else{
	worldModel.GetCircle(id).x*=-1;
      }
    }

    for( id=0;id<worldModel.Points().size();id++ ) {
      if( Mem->MySide=='l' ) {
	worldModel.GetPoint(id).y*=-1;
      }else{
	worldModel.GetPoint(id).x*=-1;
      }
    }

    for( id=0;id<worldModel.Strings().size();id++ ) {
      if( Mem->MySide=='l' ) {
	worldModel.GetString(id).y*=-1;
      }else{
	worldModel.GetString(id).x*=-1;
      }
    }
  }
#endif

  inline void Graphics::LineString(char* msg, Line& line){
    sprintf(msg,"LINE col=%s (%.1f,%.1f,%.1f,%.1f);", line.color, line.x1,line.y1,line.x2,line.y2);
  };

  inline void Graphics::CircleString(char* msg, Circle& circle){
    sprintf(msg,"CIRCLE col=%s fil=%.0f (%.1f,%.1f,%.1f);", circle.color, float(circle.filled), circle.x, circle.y, circle.r);
  };

  inline void Graphics::PointString(char* msg, Point& point){
    sprintf(msg,"POINT col=%s (%.1f,%.1f);", point.color, point.x, point.y);
  };

  inline void Graphics::StringString(char* msg, String& string) {
    sprintf(msg,"STRING col=%s (%.1f,%.1f,\"%s\");",string.color,string.x,string.y,string.string);
  }

  void Graphics::Draw() {

    char msg[MAX_MESSAGE_LENGTH];
    char msgToSend[MAX_MESSAGE_LENGTH];

    strcpy(msgToSend,"_2D_ CLEAR;");

    unsigned int id;

    for( id=0;id<worldModel.Lines().size();id++ ) {
      LineString(msg,worldModel.GetLine(id));
      strcat(msgToSend,msg);
    }

    for( id=0;id<worldModel.Circles().size();id++ ) {
      CircleString(msg,worldModel.GetCircle(id));
      strcat(msgToSend,msg);
    }

    for( id=0;id<worldModel.Points().size();id++ ) {
      PointString(msg,worldModel.GetPoint(id));
      strcat(msgToSend,msg);
    }

    for( id=0;id<worldModel.Strings().size();id++ ) {
      StringString(msg,worldModel.GetString(id));
      strcat(msgToSend,msg);
    }
    //	cout<<msgToSend<<endl;
#if defined USE_AGENT_LIBRARY
    send_message(msgToSend, Mem->sock);
#endif
#if defined USE_MONITOR_LIBRARY
    SendToMonitor(msgToSend);
#endif
  }

  SmartGraphics::SmartGraphics() {
    //colors by default
    strcpy(textColor,"000000\0");		//black color
    arrowCircleRadius=0.1f;
    strcpy(black,"000000\0");			//black color

    ballInnerRadius=0.3f;
    ballOuterRadius=1.15f;
    strcpy(ballColor,"ffffff\0");  	//white color
    strcpy(ballVelColor,"000000\0");//black color
    strcpy(ballRouteColor,"DED9FF\0");// the very light blue color

    playerInnerRadius=0.3f;
    playerOuterRadius=1.085f;

    strcpy(leftteamColor,"ffff00\0");  
    strcpy(leftteamGoalieColor,"00ff00\0");
    strcpy(leftteamVelColor,"000000\0");// black color
    strcpy(leftteamRouteColor,"111DFF\0");// blue color

    strcpy(rightteamColor,"00ffff\0");
    strcpy(rightteamGoalieColor,"ff99ff\0");
    strcpy(rightteamVelColor,"000000\0");// black color
    strcpy(rightteamRouteColor,"111DFF\0");	
  }

#if defined USE_AGENT_LIBRARY
  void SmartGraphics::InitializeColors() {
  }

  void SmartGraphics::AddArrow(Vector begin, Vector end, char color[COLOR_NAME_MAX]) {
    AddLine(begin, end, color);
    AddCircle(end,arrowCircleRadius,color,true);
  };

  void SmartGraphics::AddBallPos() {
    if( !Mem->BallPositionValid() )	 return;

    AddCircle(Mem->BallAbsolutePosition(), ballInnerRadius, ballColor, true);
    AddCircle(Mem->BallAbsolutePosition(), ballOuterRadius, ballColor, false);

    sprintf(text,"(%.1f,%.1f)Pv(%.2f)", Mem->BallX(), Mem->BallY(), (float)Mem->BallPositionValid() );
    String string(Mem->BallX(),Mem->BallY(),text,textColor);
    AddString(string);
  };

  void SmartGraphics::AddBallVel() {
    if( !Mem->BallPositionValid() || !Mem->BallVelocityValid() ) return;

    Vector ballPos=Mem->BallAbsolutePosition();
    Vector ballVel=Mem->BallAbsoluteVelocity();

    AddCircle(ballPos, ballInnerRadius, ballColor, true);
    AddCircle(ballPos, ballOuterRadius, ballColor, false);
    AddArrow(ballPos,ballPos+ballVel,ballVelColor);

    sprintf(text,"V(%.1f,%.1f)Vv(%.2f)", ballVel.x, ballVel.y, Mem->BallVelocityValid() );
    String string(ballVel.x,ballVel.y,text,textColor);
    AddString(string);
  }

  void SmartGraphics::AddBallPosAndVel() {
    if( !Mem->BallPositionValid() || !Mem->BallVelocityValid() ) return;

    Vector ballPos=Mem->BallAbsolutePosition();
    Vector ballVel=Mem->BallAbsoluteVelocity();

    AddCircle(ballPos, ballInnerRadius, ballColor, true);
    AddCircle(ballPos, ballOuterRadius, ballColor, false);
    AddArrow(ballPos,ballPos+ballVel,ballVelColor);

    sprintf(text,"P(%.1f,%.1f)Pv(%.2f)V(%.1f,%.1f)Vv(%.2f)", ballPos.x, ballPos.y, Mem->BallPositionValid(), ballVel.x, ballVel.y, Mem->BallVelocityValid());
    String string(ballPos.x,ballPos.y,text,textColor);
    AddString(string);

  };

  void SmartGraphics::AddBallRoute() {

    if( !Mem->BallPositionValid() || !Mem->BallVelocityValid() ) return;

    Vector ballPos=Mem->BallAbsolutePosition();
    Vector ballVel=Mem->BallAbsoluteVelocity();

    AddCircle(ballPos, ballInnerRadius, ballColor, true);
    AddCircle(ballPos, ballOuterRadius, ballColor, false);

    float decay=Mem->SP_ball_decay;
    float speed=ballVel.mod();

    float distToTravel=0;
    int steps=0;

    while(true) {
      steps++;
      distToTravel+=speed;
      speed*=decay;
      if( speed<=0.15 ) break;
      if( steps>=50 ) break;
    }
    Vector end=ballPos+Polar2Vector(distToTravel,ballVel.dir());

    AddArrow(ballPos,end,ballVelColor);	
  };

  void SmartGraphics::AddBallPosAndVelAndRoute() {
    if( !Mem->BallPositionValid() || !Mem->BallVelocityValid() ) return;

    Vector ballPos=Mem->BallAbsolutePosition();
    Vector ballVel=Mem->BallAbsoluteVelocity();

    AddCircle(ballPos, ballInnerRadius, ballColor, true);
    AddCircle(ballPos, ballOuterRadius, ballColor, false);
    AddArrow(ballPos,ballPos+ballVel,ballVelColor);

    sprintf(text,"P(%.1f,%.1f) V(%.1f,%.1f)", ballPos.x, ballPos.y, ballVel.x, ballVel.y);
    String string(ballPos.x,ballPos.y,text,textColor);
    AddString(string);

    float decay=Mem->SP_ball_decay;
    float speed=ballVel.mod();

    float distToTravel=0;
    int steps=0;

    while(true) {
      steps++;
      distToTravel+=speed;
      speed*=decay;
      if( speed<=0.15 ) break;
      if( steps>=50 ) break;
    }
    Vector end=ballPos+Polar2Vector(distToTravel,ballVel.dir());

    AddArrow(ballPos,end,ballVelColor);	
  }

  void SmartGraphics::AddTeammatePos(Unum num) {
    if( !Mem->TeammatePositionValid(num) ) return;

    Vector teamPos=Mem->TeammateAbsolutePosition(num);

    if( num==Mem->OurGoalieNum )
      AddCircle(teamPos, playerOuterRadius, Mem->MySide=='l'?leftteamGoalieColor : rightteamGoalieColor, true);
    else
      AddCircle(teamPos, playerOuterRadius, Mem->MySide=='l'?leftteamColor : rightteamColor, true);

    AddCircle(teamPos, playerOuterRadius, black, false);
    AddCircle(teamPos, playerInnerRadius, black, false);

    sprintf(text,"%.0f P(%.1f,%.1f)Pv(%.2f)", float(num), teamPos.x, teamPos.y, Mem->TeammatePositionValid(num));
    String string(teamPos.x,teamPos.y,text,textColor);
    AddString(string);	
  }

  void SmartGraphics::AddTeammatePosAndVel(Unum num) {
    if( !Mem->TeammatePositionValid(num) || !Mem->TeammateVelocityValid(num) ) return;

    Vector teamPos=Mem->TeammateAbsolutePosition(num);
    Vector teamVel=Mem->TeammateAbsoluteVelocity(num);	

    if( num==Mem->OurGoalieNum )
      AddCircle(teamPos, playerOuterRadius, Mem->MySide=='l'?leftteamGoalieColor : rightteamGoalieColor, true);
    else
      AddCircle(teamPos, playerOuterRadius, Mem->MySide=='l'?leftteamColor : rightteamColor, true);

    AddCircle(teamPos, playerOuterRadius, black, false);
    AddCircle(teamPos, playerInnerRadius, black, false);

    AddArrow(teamPos, teamPos+teamVel, Mem->MySide=='l'?leftteamVelColor : rightteamVelColor);

    sprintf( text,"%.0f P(%.1f,%.1f)Pv(%.2f)V(%.1f,%.1f)Vv(%.2f)", float(num), teamPos.x, teamPos.y, Mem->TeammatePositionValid(num), teamVel.x, teamVel.y, Mem->TeammateVelocityValid(num) );
    String string(teamPos.x,teamPos.y,text,textColor);
    AddString(string);
  }

  void SmartGraphics::AddTeammateRoute(Unum num) {
    if( !Mem->TeammatePositionValid(num) ) return;

    AddArrow(Mem->TeammateAbsolutePosition(num), Pos.GetHomePosition(num), Mem->MySide=='l' ? leftteamRouteColor:rightteamRouteColor);
  }

  void SmartGraphics::AddOpponentPos(Unum num) {
    if( !Mem->OpponentPositionValid(num) ) return;

    Vector oppPos=Mem->OpponentAbsolutePosition(num);

    if( num==Mem->TheirGoalieNum )
      AddCircle(oppPos, playerOuterRadius, Mem->TheirSide=='l' ? leftteamGoalieColor:rightteamGoalieColor, true);
    else
      AddCircle(oppPos, playerOuterRadius, Mem->TheirSide=='l' ? leftteamColor:rightteamColor, true);

    AddCircle(oppPos, playerOuterRadius, black, false);
    AddCircle(oppPos, playerInnerRadius, black, false);

    sprintf(text,"%.0f P(%.1f,%.1f)Pv(%.2f)", float(num), oppPos.x, oppPos.y, Mem->OpponentPositionValid(num));
    String string(oppPos.x,oppPos.y,text,textColor);
    AddString(string);
  }

  void SmartGraphics::AddOpponentPosAndVel(Unum num) {
    if( !Mem->OpponentPositionValid(num) || !Mem->OpponentVelocityValid(num) ) return;

    Vector oppPos=Mem->OpponentAbsolutePosition(num);
    Vector oppVel=Mem->OpponentAbsoluteVelocity(num);

    if( num==Mem->TheirGoalieNum )
      AddCircle(oppPos, playerOuterRadius, Mem->TheirSide=='l' ? leftteamGoalieColor:rightteamGoalieColor, true);
    else
      AddCircle(oppPos, playerOuterRadius, Mem->TheirSide=='l' ? leftteamColor:rightteamColor, true);

    AddCircle(oppPos, playerOuterRadius, black, false);
    AddCircle(oppPos, playerInnerRadius, black, false);

    AddArrow(oppPos, oppPos+oppVel, Mem->TheirSide=='l' ? leftteamVelColor:rightteamVelColor);

    sprintf( text,"%.0f P(%.1f,%.1f)Pv(%.2f)V(%.1f,%.1f)Vv(%.2f)", float(num), oppPos.x, oppPos.y, Mem->OpponentPositionValid(num), oppVel.x, oppVel.y, Mem->OpponentVelocityValid(num) );
    String string(oppPos.x,oppPos.y,text,textColor);
    AddString(string);
  }
#endif

  void SmartGraphics::ConstructGraphicFromWorldModel() {
    //construct the ball first
    if( worldModel.Ball().posValid>0 ) {
      AddCircle(worldModel.Ball().x, worldModel.Ball().y, ballInnerRadius, ballColor, true);
      AddCircle(worldModel.Ball().x, worldModel.Ball().y, ballOuterRadius, ballColor, false);
    }

    DynamicInfo player;
    for( int i=0;i<=10;i++ ) {
      player=worldModel.Teammate(i);
      if( player.posValid ) {
	if( player.goalie==true )
	  AddCircle(player.x, player.y, playerOuterRadius, player.side=='l' ? leftteamGoalieColor : rightteamGoalieColor, true);
	else
	  AddCircle(player.x, player.y, playerOuterRadius, player.side=='l' ? leftteamColor : rightteamColor, true);

	AddCircle(player.x, player.y, playerOuterRadius, black, false);
	AddCircle(player.x, player.y, playerInnerRadius, black, false);

	//			sprintf(text,"%.0f P(%.1f,%.1f)Pv(%.2f)", float(i+1), player.x, player.y, player.posValid);
	sprintf(text,"%.0f", float(i+1));

	String string(player.x,player.y,text,textColor);
	AddString(string);
      }

      player=worldModel.Opponent(i);
      if( player.posValid ) {
	if( player.goalie==true )
	  AddCircle(player.x, player.y, playerOuterRadius, player.side=='l' ? leftteamGoalieColor : rightteamGoalieColor, true);
	else
	  AddCircle(player.x, player.y, playerOuterRadius, player.side=='l' ? leftteamColor : rightteamColor, true);

	AddCircle(player.x, player.y, playerOuterRadius, black, false);
	AddCircle(player.x, player.y, playerInnerRadius, black, false);

	//			sprintf(text,"%.0f P(%.1f,%.1f)Pv(%.2f)", float(i+1), player.x, player.y, player.posValid);
	sprintf(text,"%.0f", float(i+1));

	String string(player.x,player.y,text,textColor);
	AddString(string);
      }
    }
  }


  GraphicLogSystem::GraphicLogSystem() {
    doLogWorldModel=true;
    doLogGraphicCommands=true;
    last_access_time=0;
    initialized=false;
    logMode=LM_Unknown;
  }

  void GraphicLogSystem::Initialize(LogMode logMode, const char* fileName) {
    initialized=true;
    if( !doLogWorldModel&&!doLogGraphicCommands ) return;

    this->logMode=logMode;
    if( logMode==LM_Unknown ) return;

    char openMode[4];
    if( logMode==LM_Save )
      strcpy(openMode,"w+b\0");
    else
      strcpy(openMode,"r+b\0");

    file=fopen(fileName,openMode);
    if( file==0 ) {
      cout<<"error opening file "<<fileName<<endl;
      return;
    };
  }

  void GraphicLogSystem::BeginCycle() {
    if( file==0 ) return;

    worldModel.ClearGraphicInfo();
    worldModel.ResetDynamicInfo();
  }

  void GraphicLogSystem::EndCycle(int time) {
    if( file==0 ) return;

    if( logMode==LM_Save ) {
      UpdateWorldModelByAgent();
      WriteWorldModel();
    }else{
      if( GetDataForTime(time)==true ) {
	ConstructGraphicFromWorldModel();
	Draw();
      }
    }
  }

#if defined USE_AGENT_LIBRARY
  void GraphicLogSystem::UpdateWorldModelByAgent() {

    //ball first
    worldModel.Ball().goalie=false;
    worldModel.Ball().side='?';	
    if( Mem->BallPositionValid() ) {
      worldModel.Ball().posValid=Mem->BallPositionValid();
      worldModel.Ball().x=Mem->BallX();
      worldModel.Ball().y=Mem->BallY();

      if( Mem->BallVelocityValid() ) {
	worldModel.Ball().velValid=Mem->BallVelocityValid();
	worldModel.Ball().dx=Mem->BallAbsoluteVelocity().x;
	worldModel.Ball().dy=Mem->BallAbsoluteVelocity().y;
      }
    }

    //teammates second
    for( int id=1; id<=11; id++ ) {
      if( Mem->TeammatePositionValid(id) ) {
	worldModel.Teammate(id-1).posValid=Mem->TeammatePositionValid(id);
	worldModel.Teammate(id-1).x=Mem->TeammateX(id);
	worldModel.Teammate(id-1).y=Mem->TeammateY(id);
	worldModel.Teammate(id-1).side=Mem->MySide;
	if( id==Mem->OurGoalieNum )
	  worldModel.Teammate(id-1).goalie=true;
	else
	  worldModel.Teammate(id-1).goalie=false;				

	if( Mem->TeammateVelocityValid(id) ) {
	  worldModel.Teammate(id-1).velValid=Mem->TeammateVelocityValid(id);
	  worldModel.Teammate(id-1).dx=Mem->TeammateAbsoluteVelocity(id).x;
	  worldModel.Teammate(id-1).dy=Mem->TeammateAbsoluteVelocity(id).y;
	}
      }

      //opponents fird
      if( Mem->OpponentPositionValid(id) ) {
	worldModel.Opponent(id-1).posValid=Mem->OpponentPositionValid(id);
	worldModel.Opponent(id-1).x=Mem->OpponentX(id);
	worldModel.Opponent(id-1).y=Mem->OpponentY(id);
	worldModel.Opponent(id-1).side=Mem->TheirSide;
	if( id==Mem->TheirGoalieNum )
	  worldModel.Opponent(id-1).goalie=true;
	else
	  worldModel.Opponent(id-1).goalie=false;		

	if( Mem->OpponentVelocityValid(id) ) {
	  worldModel.Opponent(id-1).velValid=Mem->OpponentVelocityValid(id);
	  worldModel.Opponent(id-1).dx=Mem->OpponentAbsoluteVelocity(id).x;
	  worldModel.Opponent(id-1).dy=Mem->OpponentAbsoluteVelocity(id).y;
	}
      }
    }

    CheckCoordinates();
  }

  void GraphicLogSystem::WriteWorldModel() {

    if( !doLogWorldModel&&!doLogGraphicCommands || last_access_time==Mem->CurrentTime.t ||
	Mem->CurrentTime<1 || file==0 ) return;

    last_access_time=Mem->CurrentTime.t;

    //log the time first
    fwrite(&last_access_time,sizeof(last_access_time),1,file);

    int worldModelSize=worldModel.Size();
    cout<<"gonna save the size of the world model "<<worldModelSize<<endl;

    //now log the size of the world model
    fwrite(&worldModelSize,sizeof(worldModelSize),1,file);

    //now log DynamicInfos in order ball, teammates, opponents;
    int dynamicInfoSize=DynamicInfo::Size();
  	
    //log ball
    fwrite(&worldModel.Ball(),dynamicInfoSize,1,file);		


    //log teammates
    for( int i=0;i<11;i++ ) {
      fwrite(&worldModel.Teammate(i),dynamicInfoSize,1,file);		
    }

    //log opponents
    for( int i=0;i<11;i++ ) {
      fwrite(&worldModel.Opponent(i),dynamicInfoSize,1,file);
    }

    int id;
    int size;
    //now write graphic information in order lines, circles, points, strings
    size = worldModel.Lines().size();
    fwrite(&size,sizeof(int),1,file);

    for( id=0;id<size;id++ )
      fwrite(&worldModel.GetLine(id),Line::Size(),1,file);


    size = worldModel.Circles().size();
    fwrite(&size,sizeof(int),1,file);

    for( id=0;id<size;id++ )
      fwrite(&worldModel.GetCircle(id),Circle::Size(),1,file);

    size = worldModel.Points().size();
    fwrite(&size,sizeof(int),1,file);

    for( id=0;id<size;id++ )
      fwrite(&worldModel.GetPoint(id),Point::Size(),1,file);

    size = worldModel.Strings().size();
    fwrite(&size,sizeof(int),1,file);

    for( id=0;id<size;id++ )
      fwrite(&worldModel.GetString(id),String::Size(),1,file);		
	
  }
#endif

  bool GraphicLogSystem::GetDataForTime(int time) {
    std::cout<<"\nrequested time is "<<time<<endl;
    if( last_access_time==time ) return false;
    int 	readTime;
    int 	size;
    int		numSearches=0;
    DynamicInfo info;
    Line 		line;
    Circle 	circle;
    Point 	point;
    String 	string;
	

    //	if( last_access_time+1!=time )
    fseek(file,0,SEEK_SET);

    last_access_time=time;
    for(;;) {
      //first read the time;
      if( fread(&readTime, sizeof(readTime),1,file)==0) {
	cout<<"it seems that we have reached the end of file"<<endl;
	return false;
      };
      cout<<"read time is "<<readTime<<endl;
      fread(&size, sizeof(size),1,file);
      cout<<"read size of world is "<<size<<endl;
      int totalSize=0;
      if( readTime==time ) {
	//do initialization here
	int diSize=DynamicInfo::Size();
	//read ball first
	fread(&info, diSize,1,file);
	worldModel.Ball()=info;
	totalSize+=diSize;

	//read teammates
	for( int i=0;i<11;i++ ) {
	  fread(&info,diSize,1,file);
	  worldModel.Teammate(i)=info;
	  totalSize+=diSize;
	}

	//read opponents
	for( int i=0;i<11;i++ ) {
	  fread(&info,diSize,1,file);
	  worldModel.Opponent(i)=info;
	  totalSize+=diSize;
	}

	//now read lines
	fread(&size, sizeof(size),1,file);
	cout<<"num Lines "<<size<<endl;
	for( int i=0;i<size;i++ ) {
	  fread(&line, Line::Size(),1,file);
	  worldModel.AddLine(line);
	  totalSize+=Line::Size();
	}

	//now read circles
	fread(&size, sizeof(size),1,file);
	cout<<"num Circles "<<size<<endl;
	for( int i=0;i<size;i++ ) {
	  fread(&circle, Circle::Size(),1,file);
	  worldModel.AddCircle(circle);
	  totalSize+=Circle::Size();
	}

	//now read points
	fread(&size, sizeof(size),1,file);
	cout<<"num Points "<<size<<endl;
	for( int i=0;i<size;i++ ) {
	  fread(&point, Point::Size(),1,file);
	  worldModel.AddPoint(point);
	  totalSize+=Point::Size();
	}

	//now read strings
	fread(&size, sizeof(size),1,file);
	cout<<"num Strings "<<size<<endl;
	for( int i=0;i<size;i++ ) {
	  fread(&string, String::Size(),1,file);
	  worldModel.AddString(string);
	  totalSize+=String::Size();
	}

	cout<<"total size is "<<totalSize<<endl;
	return true;
      }

      numSearches++;
      if( numSearches>=50000 ) return false;
      fseek(file,size+sizeof(int)*4,SEEK_CUR);
    }
  }

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
