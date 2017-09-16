/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : shoot.C
 *
 *    AUTHOR     : Sergey Serebyakov, Anton Ivanov
 *
 *    $Revision: 2.15 $
 *
 *    $Id: shoot.C,v 2.15 2004/08/29 14:07:21 anton Exp $
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
#include "shoot.h"
//----------------------------------------------------
namespace ShootSkills {
  ShootInfo ShootInfo::operator = ( ShootInfo info ) {
    SetShooter( info.GetShooter() );
    SetPosition( info.GetPosition() );
    SetConfidence( info.GetConfidence() );
    SetSpeed( info.GetSpeed() );
    SetGoalie( info.GetGoalie() );
    SetTime( info.GetTime() );
    return *this;
  }
  //----------------------------------------------------
  Shoot::Shoot() {
    num_considered_routes=20;
    time=-1;
    //next param depends on opponent's goalie craft
    shoot_threshold = 0.55f;//0.85f
    max_shoot_distance = 35.0f;//25.0f;
    shoot_speed = 2.0f;//minimal shoot speed
    //next coeffs are for computing std deviation of ball travalling distance
    //used when we are shooting to goal
    //MathCad helped much here with its regress function
    coeffs[4] = -0.000001463f;
    coeffs[3] =  0.00009068f;
    coeffs[2] =  0.0007054f;
    coeffs[1] =  0.045f;
    coeffs[0] =  0.025f;
  }
  //----------------------------------------------------	
  void Shoot::Initialize() {
    //let's initialize corners of the opponents goal
    left_corner=Mem->MySide=='l'  ? Mem->MarkerPosition( Flag_GRT ) : Mem->MarkerPosition( Flag_GLB );
    right_corner=Mem->MySide=='l' ? Mem->MarkerPosition( Flag_GRB ) : Mem->MarkerPosition( Flag_GLT );
  }
  //----------------------------------------------------	
  float Shoot::stddev(float distance) {
    // used for calculation std deviation of ball from desired path
    //check_bounds(distance, 0.0f, 32.0f);
    distance = Min(distance, 32.0f);
    return coeffs[4]*int_pow(distance,4)+coeffs[3]*int_pow(distance,3)+coeffs[2]*int_pow(distance,2)+coeffs[1]*distance+coeffs[0];
  }
  //----------------------------------------------------
  float Shoot::pnorm(float value, float mo, float stddev) {
    //be carefull, as next formula works when mo equals to zero
    return 0.5+0.5f*signf(value)*erf(value*signf(value)/(stddev*sqrt(2.0f)));
  }
  //----------------------------------------------------
  float Shoot::probability_of_deviation( Vector from, Vector to ) {
    //the original idea is from UvA RoboCup Team.
    float globalang = GetNormalizeAngleDeg( (to-from).dir() );

    float left_prob=0.0f,
      right_prob=0.0f;		
    float left_dist,
      right_dist;
    float stddev_value;

    if( fabs(globalang)<2.0f ) {
      //so, we are shooting approximately perpendicular to goal line
      //first, let's find sko for appropriate computation
      stddev_value = stddev( from.dist(to) );
      left_dist = -to.dist(left_corner);
      right_dist = to.dist(right_corner);

      //remember about 3 sigma rule!
      if( fabs(left_dist)<3.0f*stddev_value ) 
	left_prob = pnorm(left_dist, 0.0f, stddev_value);					
      if( right_dist<3.0f*stddev_value ) 
	right_prob = 1-pnorm(right_dist, 0.0f, stddev_value);					

      return 1-left_prob-right_prob;
    }
    //so, here we need more complex computations
    Line shootline;
    shootline.LineFromTwoPoints(from, to);
	
    Vector left_prpoint  = shootline.ProjectPoint(left_corner);
    Vector right_prpoint = shootline.ProjectPoint(right_corner);
	
    left_dist  = -left_prpoint.dist(left_corner);	
    right_dist =  right_prpoint.dist(right_corner);	

    //remember about 3 sigma rule!
    stddev_value = stddev(from.dist(left_prpoint));
    if( fabs(left_dist)<3.0f*stddev_value ) 
      left_prob = pnorm(left_dist, 0.0f, stddev_value);					

    stddev_value = stddev(from.dist(right_prpoint));
    if( right_dist<3.0f*stddev_value ) 
      right_prob = 1-pnorm(right_dist, 0.0f, stddev_value);					

    return 1-left_prob-right_prob;
  }
  //----------------------------------------------------
  void Shoot::suppose_player_position(AngleDeg shootang, Vector from ,Unum opponent, int /*kick_cycles*/, Vector& plpos) {
    static Rectangle rect=Mem->FieldRectangle;
    if(!Mem->OpponentPositionValid(opponent))
      return;
    rect=rect.expand(2);
    Ray ray(from, shootang);
    int t=NumOfCyclesThenILastSeePlayer(-opponent);
    Vector target=rect.RayIntersection(ray);
    if( target.x==0 && target.y==0 ) return;
    if(Mem->OpponentDistanceToBall(opponent)>3.0f)//cond add by AI
      plpos=plpos+Polar2Vector(Min(t*Mem->GetOpponentPlayerSpeedMax(opponent),
				   Mem->OpponentAbsolutePosition(opponent).dist(target)), (target-plpos).dir());
  }
  //----------------------------------------------------
  float Shoot::verify_interception_info(Intercepter& intercepter, PlayerInterceptInfo& pInfo, AngleDeg shootangle) {
    if( Mem->IsSuccessRes(pInfo.res) ) return pInfo.numCyc;

    static float plcyc;
    plcyc=InfCycles;
    //	static AngleDeg diff;    if(

    //	if( intercepter.AngValid() ) {
    //		diff=GetDiff(shootangle, intercepter.Ang());
    //		if( diff<=90 )
    //			plcyc=plcyc-10*(1.0f-diff/90.0f);
    //		else
    //			plcyc=plcyc+10*( (diff-90.0f)/90.0f );
    //	}
    return plcyc;
  }
  //----------------------------------------------------
  //////////////////////////////////////////////////////////////////////
  float Shoot::shoot_to_point( Vector from, Vector to,
			       float& shootspeed, Unum& danger,Unum tm) {
    if( from.dist(to)>max_shoot_distance ) return 0.0f;

    static AngleDeg shootangle;
    static Vector ballpos;
    static Vector ballvel;
    static float ballcyc;
    static Vector plpos;
    static PlayerInterceptInfo pInfo;
    static Intercepter intercepter;
    static float plcyc;

    shootangle = GetNormalizeAngleDeg( (to-from).dir() );
    ballpos=from;
    ballvel=Polar2Vector(shootspeed,shootangle);
    ballcyc=SolveForLengthGeomSeries(shootspeed,Mem->SP_ball_decay,to.dist(from));
    danger=Unum_Unknown;
    if(ballcyc<0)
      return 0.0f;

    float min_prob=1.0f,
      prob;
    if(Pos.close_goalie_intercept(ballpos+ballvel)){
      Mem->LogAction6(10,"shoot_to_point: predict intercept goalie in next cycle with to=(%.2f;%.2f);from=(%.2f;%.2f)",
		      to.x,to.y,from.x,from.y);
      min_prob=0.4f;
    }
    float sight_sign=1.0f;
    if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum))
      sight_sign=signf(Mem->OpponentX(Mem->TheirGoalieNum));
    
    for( unsigned int id=0;id<opponents.size();id++ ) {
      AngleDeg oppang;
      intercepter=interception.Opponent(opponents[id]);
      if(!Mem->OpponentPositionValid(opponents[id]))
	plpos=Vector(48.0*sight_sign,0.0);
      else
	plpos=intercepter.Pos();
      
      oppang=(plpos-from).dir();
      NormalizeAngleDeg(&oppang);
      Mem->LogAction6(10,"shoot_to_point:calc for opponent %.0f (conf=%.2f); diff_ang=%.2f; dist_to_playre=%.2f",
		      float(opponents[id]),Mem->OpponentPositionValid(opponents[id]),GetDiff(shootangle,oppang),from.dist(plpos));
      if( (GetDiff(shootangle,oppang)>30.0f ||from.dist(plpos)<=2.5f)&&
	  opponents[id]!=Mem->TheirGoalieNum) continue;
      if(opponents[id]==Mem->TheirGoalieNum&& GetDiff(shootangle,oppang)<(from.dist(to)>10.0f?40.0f:42.0)&&
	 from.dist(plpos)<14.0f&&signf(Mem->OpponentX(opponents[id])-Mem->BallX())==sight_sign){//AI:hack
	min_prob=0.4f;
	danger=opponents[id];
      }

      if(tm==Mem->MyNumber)
	suppose_player_position(shootangle, ballpos, opponents[id], 1/*get_kick_cycles(shoot_speed,shootangle)*/, plpos);

      pInfo=Mem->ActiveCanGetThere(	Mem->SP_max_power,Mem->CP_max_int_lookahead, ballpos,ballvel,
					intercepter.Side(), intercepter.Num(), plpos, intercepter.Vel(), intercepter.Ang(),
					intercepter.AngValid(),intercepter.IsMe() );
      plcyc=verify_interception_info(intercepter, pInfo, shootangle);
      Mem->LogAction6(50,"shoot_to_point: opp %.0f has %.0f cycles at %.2f angle (ball cycles=%.0f)",
		      float(intercepter.Num()),float(plcyc),shootangle,float(ballcyc));
      prob=/*(opponents[id]==Mem->TheirGoalieNum?interception.ball_control(ballcyc, plcyc):(*/(ballcyc<plcyc?0.99f:0.1f);//mod by AI
      if( prob<min_prob ) {
	min_prob=prob;
	danger=opponents[id];
      }

      if(opponents[id]==Mem->TheirGoalieNum&&intercepter.AngValid() ) {//first cond add by AI
	intercepter.SetAng(GetNormalizeAngleDeg(intercepter.Ang()+180.0f));
	pInfo=Mem->ActiveCanGetThere(	Mem->SP_max_power,Mem->CP_max_int_lookahead, ballpos,ballvel,
					intercepter.Side(), intercepter.Num(), plpos, intercepter.Vel(),
					intercepter.Ang(), intercepter.AngValid(),intercepter.IsMe() );
	plcyc=verify_interception_info(intercepter, pInfo, shootangle);
	prob=(ballcyc<plcyc?0.99f:0.1f);//mod by AI//interception.ball_control(ballcyc, plcyc);
	if( prob<min_prob ) {
	  min_prob=prob;
	  danger=opponents[id];
	}
      }
    }

    return min_prob*probability_of_deviation(from, to);
  }
  //----------------------------------------------------
  ShootInfo Shoot::not_limited_shoot(Unum teammate) {
    ShootInfo info;
    Mem->LogAction2(10,"shoot - shoot isn't limited by opponents");                 	
    info.SetShooter( teammate );
    info.SetTime( Mem->CurrentTime );
    info.SetSpeed(shoot_speed);
    info.SetGoalie(Unum_Unknown);

    Vector tmppos;
    float  tmpprob,
      prob=0,
      shootconf=0.0f;
    float  mindist=Mem->SP_pitch_length,
      dist;
    Vector from = Mem->TeammateAbsolutePosition(teammate),
      shootpos=Mem->MarkerPosition(Mem->RM_Their_Goal);
    float step= fabs(left_corner.y)*2/num_considered_routes;

    for( float y=left_corner.y+2.0f; y<=right_corner.y-2.0f; y+=step ) {
      tmppos.ins(Mem->SP_pitch_length/2,y);
      dist=from.dist(tmppos);
      tmpprob=probability_of_deviation(from, tmppos);
      if( tmpprob>prob || tmpprob>0.95f ) {
	if( dist<mindist ) {
	  mindist=dist;
	  shootpos=tmppos;
	  shootconf=tmpprob;				
	}
      }		
    }
    info.SetConfidence(shootconf);
    info.SetPosition(shootpos);
    
    if((Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum))&&teammate==Mem->MyNumber&&Mem->MyX()<45.0f){//AI:hack
      info.SetConfidence(0.11f);
    }
    
    return info;
  }
  //----------------------------------------------------
  ShootInfo Shoot::explore_player(Unum teammate,float sight_sign,Vector from)
  {
    interception.SetPlayerPosInfo();
    ShootInfo info;

    if((Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum))&&teammate==Mem->MyNumber&&
       signf(Mem->MyX()-45.0f)!=sight_sign){//AI:hack
      info.SetConfidence(0.11f);
      info.SetShooter( teammate );
      info.SetTime( Mem->CurrentTime );
      info.SetPosition(Vector(Mem->SP_pitch_length/2.0f*sight_sign,0.0f));
      info.SetConfidence(0.0f);
      info.SetSpeed(Mem->SP_ball_speed_max);
      info.SetGoalie(Mem->TheirGoalieNum);
      return info;
    }
    if( Pos.GetTmPos(teammate).dist(Vector(Mem->SP_pitch_length/2.0f*sight_sign,0.0f))>max_shoot_distance ) {
      info.SetShooter( teammate );
      info.SetTime( Mem->CurrentTime );
      info.SetPosition(Vector(Mem->SP_pitch_length/2.0f*sight_sign,0.0f));
      info.SetConfidence(0.0f);
      info.SetSpeed(Mem->SP_ball_speed_max);
      info.SetGoalie(Mem->TheirGoalieNum);
      return info;
    }

    if( time!=Mem->CurrentTime ) {
      time=Mem->CurrentTime;
      opponents.clear();
      AngleDeg lower=Mem->MyBodyAng()+Mem->AngleToFromBody(left_corner)-45.0f;
      AngleDeg upper=Mem->MyBodyAng()+Mem->AngleToFromBody(right_corner)+45.0f;

      NormalizeAngleDeg(&lower);
      NormalizeAngleDeg(&upper);

      for( int pl=1;pl<=Mem->SP_team_size;pl++ ) {
      	AngleDeg oppang;
      	if( !Mem->OpponentPositionValid(pl) ) continue;
      	oppang=Mem->MyBodyAng()+Mem->OpponentAngleFromBody(pl);
      	NormalizeAngleDeg(&oppang);
      	if( (oppang>=lower && oppang<=upper &&teammate==Mem->MyNumber)||pl==Mem->TheirGoalieNum)
      	  opponents.push_back(pl);
      }
    }
    if( opponents.size()==0 ) return not_limited_shoot(teammate);
	
    float temp_vel=0;
    Unum  		tmpdanger;
    Vector    tmppos;
    if(from.x==777.0f)
      from=(teammate!=Mem->MyNumber?Pos.GetSmartPassPoint(teammate,&temp_vel):Mem->BallAbsolutePosition());
    Unum cl_opp=Mem->ClosestOpponentToBall();

    
    SK_Mode sk=SelectShootMode();

    
    float     tmpspeed=shoot_speed,
      prob=0.0f,
      tmpprob,
      minprob=1.0f;

    float goalie_sign=(Mem->TheirGoalieNum==Unum_Unknown||Mem->OpponentPositionValid(Mem->TheirGoalieNum)<0.8f)?
      1.0f:signf(Mem->OpponentY(Mem->TheirGoalieNum));
    
    float step=(teammate==Mem->MyNumber?.5f:6.0f)*goalie_sign;
    float begin=(teammate==Mem->MyNumber?-7.0f:-6.0f)*goalie_sign;
    info.SetShooter( teammate );
    info.SetTime( Mem->CurrentTime );
    info.SetFrom(from);
    shootinfos.clear();

    Mem->LogAction3(10,"shoot calculations for teammate %d",teammate);
    Vector predBallPos,predBallVel;
    Vector myPredPos=Mem->MyPredictedPosition();
    int goalie_cyc=Mem->TheirGoalieNum==Unum_Unknown?100:NumOfCyclesThenILastSeePlayer(-Mem->TheirGoalieNum);
    
    float const DANGER_ANG_DIFF=45.0f;
    
    for( float y=begin; (step<0?y>=-begin:y<=-begin); y+=step ) {
      tmppos.ins(Mem->SP_pitch_length/2*sight_sign,y);
      if(teammate==Mem->MyNumber){
	smartkick( Mem->SP_ball_speed_max*2,
		   tmppos,
		   sk ,true);//AI: тут тонкий момент по предсказанию сокрости мяча в следующем цикле (в свясизи с отсутствием соответствующего интерфейса!)
	GetBallPosVelInNextCycle(predBallPos,predBallVel);
      }
      
      tmpspeed=((predBallPos-myPredPos).mod()>Mem->GetMyKickableArea()&&teammate==Mem->MyNumber&&Mem->BallKickable())?predBallVel.mod():shoot_speed;
      tmpspeed=max(tmpspeed-goalie_cyc*0.2f,shoot_speed);
      
      Mem->LogAction4(20,"for y=%.2f select speed=%.2f",
		      y,tmpspeed);
     
      tmpprob=shoot_to_point(from, tmppos,tmpspeed, tmpdanger,teammate);

      info.SetConfidence(tmpprob);
      info.SetPosition(tmppos);
      info.SetSpeed(tmpspeed);
      info.SetGoalie(tmpdanger);
      Mem->LogAction6(20,"shoot prob to point (%.2f,%.2f) is %.2f; danger opp %.0f",tmppos.x,tmppos.y, tmpprob,float(tmpdanger));

      if(teammate==Mem->MyNumber&&tmpprob>0.98f&&tmpdanger==Mem->TheirGoalieNum&&
	 GetDiff((tmppos-from).dir(),(Mem->OpponentAbsolutePosition(tmpdanger)-from).dir())>min(180.0f,DANGER_ANG_DIFF+goalie_cyc*2)){//hack
	return info;
      }
      
      shootinfos.push_back(info);
    }
    int size=(int)shootinfos.size();
    // Mem->LogAction3(10,"shoot shootinfos size is %.0f",float(size));
    int	 id,
      id_min,
      _id_min,
      id_min_,
      id_max=-1;

    for( id=0; id<size;id++ ) {
      if( shootinfos[id].GetConfidence()<minprob ) {
	minprob=shootinfos[id].GetConfidence();
	id_min=id;
      }
    }
    //	Mem->LogAction3(10,"shoot id_min is %.0f", float(id_min));

    _id_min=id_min-1;
    if( _id_min<=-1 ) _id_min++;
    //	Mem->LogAction3(10,"shoot _id_min is %.0f", float(_id_min));

    id_min_=id_min+1;
    if( id_min_>=size ) id_min_--;
    //	Mem->LogAction3(10,"shoot id_min_ is %.0f", float(id_min_));

    for( id=_id_min; id>=0; id-- ) {    

      if( shootinfos[id].GetConfidence()>=prob ) {
	prob=shootinfos[id].GetConfidence();
	id_max=id;
      }		
    }
    for( id=id_min_; id<size; id++ ) {
      if( shootinfos[id].GetConfidence()>=prob ) {
	prob=shootinfos[id].GetConfidence();
	id_max=id;
      }
    }
    if(id_max==-1) {
      Mem->LogAction2(10,"id_max is inknown, choosing randomly");
      return shootinfos[int(range_random(0, size))];
    }
    //add by AI
    if(teammate==Mem->MyNumber&&Mem->TheirGoalieNum!=Unum_Unknown&&
       Mem->OpponentPositionValid(Mem->TheirGoalieNum)<1.0f&&fabs(shootinfos[id_max].GetPosition().y)<5.0f&&
       /*shootinfos[id_max].GetGoalie()==Mem->TheirGoalieNum&&(Mem->OpponentX(Mem->TheirGoalieNum)-45.0f*sight_sign)==sight_sign&&
	 (Mem->BallX()-48.0f*sight_sign)!=sight_sign*/Mem->OpponentX(Mem->TheirGoalieNum)>45.0f&&Mem->BallX()<45.0f&&Mem->BallX()>40.0f&&
       GetDiff((from-shootinfos[id_max].GetPosition()).dir(),(from-Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)).dir())<45.0f){
      Mem->LogAction4(10,"explore_player: CORRECT SHOOT POS, fo danger opponent (orig y=%.2f; conf=%.2f)",
		      shootinfos[id_max].GetPosition().y,shootinfos[id_max].GetConfidence());
      shootinfos[id_max].SetPosition(Vector(Mem->SP_pitch_length/2*sight_sign,signf(shootinfos[id_max].GetPosition().y)*5.0f));
    }
    
    //так как при  дальнем пасе врать успевает переместиться, то должны скорректировать вероятность попадания по воротам
    if(teammate!=Mem->MyNumber&&Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)
       &&!IsGoalieActive()){
      const float COEFF=0.04f;
      float old=shootinfos[id_max].GetConfidence();
      float c=Max(0,sight_sign*47.0f-from.x)*COEFF;
      shootinfos[id_max].SetConfidence(Max(0,old-c));
      Mem->LogAction4(10,"explore_player: old val %.2f; correct val: %.2f",old,shootinfos[id_max].GetConfidence());
    }
    Mem->LogAction5(20,"Max shoot conf = %.2f at y=%.2f; danger opp=%.0f",
		    shootinfos[id_max].GetConfidence(),shootinfos[id_max].GetPosition().y,float(shootinfos[id_max].GetGoalie()));
    return shootinfos[id_max];
  }
  //----------------------------------------------------	
  float Shoot::TeammateShootConf(Unum teammate,float sight_sign,Vector from) {
    if( !shooters[teammate-1].Valid(from) )
      shooters[teammate-1]=explore_player(teammate,sight_sign,from);		
    return shooters[teammate-1].GetConfidence();
  }
  //----------------------------------------------------	
  AngleDeg Shoot::MyShootAngle() {
    if( !shooters[Mem->MyNumber-1].Valid() )
      shooters[Mem->MyNumber-1]=explore_player(Mem->MyNumber);		
    return Mem->AngleToGlobal(shooters[Mem->MyNumber-1].GetPosition() );	
  }
  //----------------------------------------------------	
  void Shoot::shoot() {
    if( !shooters[Mem->MyNumber-1].Valid() )
      shooters[Mem->MyNumber-1]=explore_player(Mem->MyNumber);			

    shooters[Mem->MyNumber-1].Log(10);
    //add by AI
    SK_Mode sk=SelectShootMode();
    if(sk==SK_Fast){
      Mem->LogAction2(10,"shoot: Select fast kick");
    }
    //end addition AI
    smartkick( Mem->SP_ball_speed_max*2,
	       GetKickAngle( shooters[Mem->MyNumber-1].GetPosition() ),
	       sk );
  }
  //----------------------------------------------------
  SK_Mode  Shoot::SelectShootMode()
  {
    Unum opp=Mem->ClosestOpponentToBall();
    SK_Mode sk=SK_Safe;
    if((opp!=Unum_Unknown&&Mem->EstimatedCyclesToSteal(opp)<=1)||
       (Mem->OpponentPositionValid(Mem->TheirGoalieNum)&&Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<=4.0f)||
       Mem->ClosestOpponentToBallDistance()<2.5f){
      sk=SK_Fast;
    }
    return sk;
  }
  
  //----------------------------------------------------
}//ShootSkills
