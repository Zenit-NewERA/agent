/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : pass.C
 *
 *    AUTHOR     : Sergey Serebyakov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: pass.C,v 2.7 2004/06/22 17:06:16 anton Exp $
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
 
#include "pass.h"
#include <cmath>
#include "Offense.h"
#include <algorithm>
#include <functional>
#include "Playposition.h"
#include "Handleball.h"
//----------------------------------------------------
namespace PassSkills {
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PassInfo::PassInfo() {
	receiver = Unum_Unknown;
  speed = 2.0f;
  receiverCyc = InfCycles;
  opponent	 = Unum_Unknown;
  opponentCyc = InfCycles;
  conf = 0.0f;
  time = -1;
	priority=0;
	passPos.ins(52.5,0);
	congestion=0.0f;
}		
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void PassInfo::SetInfo(	Unum receiver, float rCycles, float conf, Vector passPos, 
 												float speed, Unum opp, float oCycles,float priority ) {
	this->receiver = receiver;
  this->receiverCyc = rCycles;
  this->conf = conf;
  this->passPos = passPos;
  this->speed = speed;
  this->opponent = opp;
  this->opponentCyc = oCycles;
	this->priority=priority;
  time=Mem->CurrentTime;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void PassInfo::Clear() {
	receiver=opponent=Unum_Unknown;
	receiverCyc=opponentCyc=0;
  speed=1.0f;
  conf=0.0f;
  time=-1;
	priority=0;
	passPos.ins(52.0,0);
	congestion=0.0f;
};		
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void PassInfo::Log(int level) {
  static char str[300];
  sprintf(str,"angle %.2f recv_p (%.2f,%.2f) recv %.0f cycles %.2f opp %.0f cycles %.2f conf %.2f speed %.2f priority %.8f congest %.9f",
  	(-Mem->MyPos()+passPos).dir(),
	  passPos.x,
	  passPos.y,
	  float(receiver),
	  receiverCyc,
	  float(opponent),
	  opponentCyc,
	  conf,
	  speed,
		priority,
		congestion);
		
    Mem->LogAction2(level,str );
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void PassInfo::operator=(const PassInfo& info) {
  receiver=info.GetReceiver();
  passPos=info.GetPassPos();
  speed=info.GetSpeed();
  receiverCyc=info.GetReceiverCycles();
  opponent=info.GetOpponent();
  opponentCyc=info.GetOpponentCycles();
  conf=info.GetConfidence();
  time=info.GetTime();
	priority=info.GetPriority();
	congestion=info.GetCongestion();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TransitionPriorities::Initialize(float transitionPriorities[Y][X]) {
  for( int y=0;y<Y;y++ )
    for( int x=0;x<X;x++ )
			this->transitionPriorities[y][x]=transitionPriorities[y][x];
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
  void PassPriorities::Initialize() {
    reverse=false;
    ballX=ballY=0;
	
    //Vector dist(Mem->SP_pitch_length/X, Mem->SP_pitch_width/Y);
    Vector dist(Mem->SP_pitch_length, Mem->SP_pitch_width/2);
    maxDist=dist.mod();
	
    float  p[Y][X];
		
    //[0][0]
    p[0][0]=0.15;p[0][1]=0.50;p[0][2]=0.55;p[0][3]=0.80;p[0][4]=0.85;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.45;p[1][3]=0.75;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.40;p[2][3]=0.70;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.35;p[3][3]=0.60;p[3][4]=0.65;
    transitions[0][0].Initialize(p);
	
    //[0][1]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.55;p[0][3]=0.80;p[0][4]=0.85;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.50;p[1][3]=0.75;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.70;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.65;p[3][4]=0.60;
    transitions[0][1].Initialize(p);
	
    //[0][2]
    p[0][0]=0.25;p[0][1]=0.35;p[0][2]=0.65;p[0][3]=0.80;p[0][4]=0.85;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.60;p[1][3]=0.75;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.20;p[2][2]=0.55;p[2][3]=0.70;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.15;p[3][2]=0.40;p[3][3]=0.45;p[3][4]=0.50;
    transitions[0][2].Initialize(p);
		
    //[0][3]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.55;p[0][3]=0.80;p[0][4]=0.95;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.50;p[1][3]=0.85;p[1][4]=0.90;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.70;p[2][4]=0.75;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.60;p[3][4]=0.65;
    transitions[0][3].Initialize(p);

    //[0][4]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.55;p[0][3]=0.70;p[0][4]=0.75;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.50;p[1][3]=0.85;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.80;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.60;p[3][4]=0.65;
    transitions[0][4].Initialize(p);
	
    //[1][0]
    p[0][0]=0.20;p[0][1]=0.95;p[0][2]=0.90;p[0][3]=0.85;p[0][4]=0.70;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.60;p[1][3]=0.65;p[1][4]=0.80;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.40;p[2][3]=0.50;p[2][4]=0.75;
    p[3][0]=0.10;p[3][1]=0.15;p[3][2]=0.35;p[3][3]=0.45;p[3][4]=0.55;
    transitions[1][0].Initialize(p);
	
    //[1][1]
    p[0][0]=0.15;p[0][1]=0.40;p[0][2]=0.95;p[0][3]=0.90;p[0][4]=0.75;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.60;p[1][3]=0.80;p[1][4]=0.85;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.65;p[2][4]=0.70;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.35;p[3][3]=0.50;p[3][4]=0.55;
    transitions[1][1].Initialize(p);
	
    //[1][2]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.50;p[0][3]=0.75;p[0][4]=0.85;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.55;p[1][3]=0.80;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.70;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.60;p[3][4]=0.65;
    transitions[1][2].Initialize(p);
	
    //[1][3]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.50;p[0][3]=0.70;p[0][4]=0.80;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.55;p[1][3]=0.85;p[1][4]=0.95;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.75;p[2][4]=0.90;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.60;p[3][4]=0.65;
    transitions[1][3].Initialize(p);
	
    //[1][4]
    p[0][0]=0.15;p[0][1]=0.35;p[0][2]=0.55;p[0][3]=0.70;p[0][4]=0.85;
    p[1][0]=0.05;p[1][1]=0.30;p[1][2]=0.50;p[1][3]=0.80;p[1][4]=0.90;
    p[2][0]=0.00;p[2][1]=0.25;p[2][2]=0.45;p[2][3]=0.75;p[2][4]=0.95;
    p[3][0]=0.10;p[3][1]=0.20;p[3][2]=0.40;p[3][3]=0.60;p[3][4]=0.65;
    transitions[1][4].Initialize(p);
	
  }
  //----------------------------------------------------
  //----------------------------------------------------
  void PassPriorities::InitBallPos(Vector ballPos) {
    if( ballPos.y==0 ) ballPos.y=-0.0001;
    reverse=( ballPos.y>0 ? true : false );
	
    if( reverse )
      ballPos.y=-ballPos.y;
	
    //init ball region
    ballX=int(Mem->SP_pitch_length/(ballPos.x+Mem->SP_pitch_length/2));
    ballY=int(Mem->SP_pitch_width/(ballPos.y+Mem->SP_pitch_width/2));
	
    ballX=max(0,ballX);ballX=min(X,ballX);
    ballY=max(0,ballY);ballY=min(Y,ballY);
  }
  //----------------------------------------------------
  //----------------------------------------------------
  float PassPriorities::GetPriority(Vector pos) {
    if( !Mem->FieldRectangle.IsWithin(pos) ) return 0.0f;

    //pass position
    static int x;
    static int y;
	
    if( reverse )
      pos.y=-pos.y;
	
    x=int(Mem->SP_pitch_length/(pos.x+Mem->SP_pitch_length/2));
    y=int(Mem->SP_pitch_width/(pos.y+Mem->SP_pitch_width/2));
	
    x=max(0,x);x=min(X,x);
    y=max(0,y);y=min(Y,y);
	
    return transitions[ballY][ballX].GetPriority(x,y)+0.05f;
  }
  //----------------------------------------------------
  //----------------------------------------------------
  PassInfo Pass::explore_pass_to_teammate(Unum teammate,bool super_kick) {
    PassInfo info;
    info.SetReceiver(teammate);

    if( teammate==Unum_Unknown || teammate==Unum_Teamless ||
	Mem->TeammatePositionValid(teammate)<min_pos_valid ||
	Mem->TeammateDistance(teammate)>max_pass_distance ||
	Mem->OwnPenaltyArea.IsWithin(Mem->TeammateAbsolutePosition(teammate))||
	Pos.GetPlayerType(teammate)<=PT_Defender
	) return info;

    float t;
	
    Vector recvpos=Pos.GetSmartPassPoint(int(teammate),&t);
    //Pos.GetTmPos(teammate).x>30.0f?Pos.GetSmartPassPoint(int(teammate),&t):Pos.GetTmPos(teammate);
    //Mem->TeammateAbsolutePosition(teammate);
    Vector passpos=recvpos;

    float passspeed;
    passspeed=min(	0.9f*Mem->SP_ball_speed_max,
			Mem->VelAtPt2VelAtFoot(passpos, 2.0f) );
    if(Pos.GetMinOppAngleAbs(Mem->BallAbsolutePosition(),recvpos)>90.0f)
      passspeed=min(	0.9f*Mem->SP_ball_speed_max,
			Mem->VelAtPt2VelAtFoot(passpos, 1.4f) );

    if(super_kick)
      passspeed=Mem->SP_ball_speed_max;
		
    Vector ballvel;
    ballvel=Polar2Vector(passspeed, (recvpos-Mem->MyPos()).dir() );	
    Unum  danger=Unum_Unknown;
    info.SetTime(Mem->CurrentTime);
    info.SetPassPos(passpos);
    info.SetSpeed(passspeed);
    info.SetConfidence(Pos.TmPassConf(teammate,Mem->BallAbsolutePosition(),ballvel,&danger));
    info.SetOpponent(danger);

    return info;
  }
  //----------------------------------------------------
  float Pass::PassConf(Unum teammate,PassInfo* pInfo) {
    if( !dpasses[teammate-1].Valid() )
      dpasses[teammate-1]=explore_pass_to_teammate(teammate);			

    if( pInfo!=0 )	
      *pInfo=dpasses[teammate-1];

    return dpasses[teammate-1].GetConfidence();
  }
  //----------------------------------------------------
  PassInfo Pass::GetPassInfo(Unum teammate) {
    if( !dpasses[teammate-1].Valid() )
      dpasses[teammate-1]=explore_pass_to_teammate(teammate);			

    return dpasses[teammate-1];
  }
  //----------------------------------------------------
  void Pass::SetPassRoutes(bool doLog) {
	
    passAngles.clear();
    teammates.clear();
    opponents.clear();
	
    priorities.InitBallPos(Mem->BallAbsolutePosition());
	
    AngleDeg 		 ang;
    AngleDeg   		 minAngle = -100.0f;
    AngleDeg   		 maxAngle =  100.0f;
		
    //this is a place for correction minimal and maximal angles
    //end of the procedure
    if( Mem->MyX()>Mem->SP_pitch_length-Mem->SP_penalty_area_length ) 
      if( Mem->MyY()>0 )
	minAngle = -120;
      else
	maxAngle =  120;
    //creat list of teammates, opponents and pass angles
    for( int pl=1; pl<=11; pl++ ) {
      //filter for opponents
      if( Mem->OpponentPositionValid(pl) ) opponents.push_back( Controler(pl,Mem->TheirSide) );

      //filter for teammates
      //some other conditions will be checked in future calculations. see code for details
      if( Mem->TeammatePositionValid(pl)<MinPosValid() || 
	  pl==Mem->MyNumber ||
	  pl==Mem->OurGoalieNum || 
	  Mem->TeammateDistance(pl)>GetMaxPassDistance() ||
	  Mem->OwnPenaltyArea.IsWithin(Mem->TeammateAbsolutePosition(pl))  ||
	  !Mem->FieldRectangle.IsWithin(Mem->TeammateAbsolutePosition(pl)) ||
	  Pos.GetTmPos(pl).x-2.0f>Mem->my_offside_line ||
	  Mem->TeammateDistance(pl)<=2 ) continue;
		
      ang=Mem->AngleToGlobal(Mem->TeammateAbsolutePosition(pl));
      if( ang>=minAngle && ang<=maxAngle ) {
	passAngles.push_back(ang);
      }
		
      teammates.push_back( Controler(pl,Mem->MySide) );			
	
    }

    self=Controler(Mem->MyNumber, Mem->MySide);
    self.prPoint=self.plPos=Mem->BallAbsolutePosition();
    self.distToLine=self.distFrom=self.distTo=0.0f;
	
    priorities.InitBallPos(Mem->BallAbsolutePosition());
	
    //if there are no angles, just quit
    if( passAngles.size()==0 ) return;
    //if there is only one pass angle, do the folowing		
    if( passAngles.size()==1 ) {
		
      for( int id=-3;id<=3;id++ ) {
	if( id==0 ) continue;
	ang=GetNormalizeAngleDeg(passAngles[0]+15*id);
	if( ang>=minAngle && ang<=maxAngle ) 
	  passAngles.push_back(ang);
      }
		
    }else {
      //remember the amount of pass angles
      int size=passAngles.size();
      //sort pass angles to find minimal and maximal angles
      sort( passAngles.begin(), passAngles.end() );
      //so, there are at least 2 pass angles.First, process first and last angles
      for( int id=1;id<=3;id++ ) {
	ang=GetNormalizeAngleDeg(passAngles[0]-15*id);
	if( ang>=minAngle && ang<=maxAngle ) 
	  passAngles.push_back(ang);
			
	ang=GetNormalizeAngleDeg(passAngles[size-1]+15*id);
	if( ang>=minAngle && ang<=maxAngle ) 
	  passAngles.push_back(ang);
			
      }
      //now, calculate the angles between the main angles
      for( int id=0;id<size-1;id++ ) {
		
	float diff=GetDiff(passAngles[id], passAngles[id+1]);
			
	if( diff<10.0f ) {
	  //there are no routes
	}else if( diff<30.0f ) {
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+diff/2));			
	}else if( diff<45 ) {
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+diff/3));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+2*diff/3));			
	}else if( diff<60 ) {
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+diff/4));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+2*diff/4));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+3*diff/4));			
	}else if( diff<90 ) {
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+diff/5));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+2*diff/5));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+3*diff/5));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+4*diff/5));			
	}else if( diff<120 ) {
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+diff/6));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+2*diff/6));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+3*diff/6));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+4*diff/6));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+5*diff/6));						
	}else{
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+15));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+30));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id]+45));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id+1]-15));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id+1]-30));			
	  passAngles.push_back(GetNormalizeAngleDeg(passAngles[id+1]-45));			
	}
      }
		
    }
	
    sort( passAngles.begin(), passAngles.end() );
	
    //graphical visualization(turn it off for real games)
    //next code just draws the pass lines 
    /*
      if( doLog ) {
      float   dist;
      Ray 	  ballCourse;
      Vector  intersection;
      for( unsigned int id=0;id<passAngles.size();id++ ) {
      ballCourse=Ray(Mem->BallAbsolutePosition(),passAngles[id]);
      intersection=Mem->FieldRectangle.RayIntersection(ballCourse);
      dist=Min(Mem->BallAbsolutePosition().dist(intersection), GetMaxPassDistance());				
      intersection=Mem->BallAbsolutePosition()+Polar2Vector(dist,passAngles[id]);
      logGr.AddLine(Mem->BallAbsolutePosition(), intersection,"A3A3FF");
      }
      }
    */
    //next code draws teammates and opponents (just circles of different colors)
	
    if( doLog ) {
      for( unsigned int id=0;id<teammates.size();id++ ) {
	logGr.AddCircle(Mem->TeammateX(teammates[id].num), Mem->TeammateY(teammates[id].num), 1.4f, "2816EB");
      }
		
      for( unsigned int id=0;id<opponents.size();id++ ) {
	logGr.AddCircle(Mem->OpponentX(opponents[id].num), Mem->OpponentY(opponents[id].num), 1.4f, "DD3739");
      }
    }
	
  }
  //----------------------------------------------------
  bool Pass::ExplorePassRoutes(bool doLog) {
    if( passAngles.size()==0 ) {
      Mem->LogAction2(10,"Pass::no pass angles");
      return false;
    }
	
    vector<Controler> controlers;
    vector<Controler> lineControlers;
    allControlers.clear();
	
    Vector origin=Mem->BallAbsolutePosition();
    unsigned int size;
    vector<Controler>::iterator iter;
    vector<Controler>::iterator player;
    float m,c,c_t,a,b,minDist;
    float maximalPassDistance;
    bool  lastControler;
    Ray   ray;
		
    for( unsigned int id=0;id<passAngles.size();id++ ) {
      //set possible controlers
      if( !SetControlers(controlers,id) ) continue;
      lineControlers.clear();
      //let's discover the true controlers
      //the first controller is always the player in ball possession mode
      lineControlers.push_back(self);
		
      maximalPassDistance=GetMaxPassDistance();
      if( Mem->FieldRectangle.IsWithin(origin) ) {
	ray.SetValues(origin, passAngles[id]);
	maximalPassDistance=min( maximalPassDistance,
				 origin.dist(Mem->FieldRectangle.RayIntersection(ray)) );
      }

      lastControler=false;		
      while(true) {
	player  = controlers.end();
	size    = lineControlers.size();
	a	    = lineControlers[size-1].distToLine;
	minDist = origin.dist(lineControlers[size-1].prPoint);
	c 	    = maximalPassDistance-lineControlers[size-1].distFrom;
			
	for( iter=controlers.begin();iter<controlers.end();iter++ ) {
	  if( origin.dist((*iter).prPoint)<=minDist ) continue;
					
	  m=lineControlers[size-1].prPoint.dist((*iter).prPoint);
	  b=(*iter).distToLine;
	  c_t=0.5f*(m-(a*a-b*b)/m);
	  if( c_t>c ) continue;
					
	  c=c_t;
	  player=iter;
	}
			
	if( player==controlers.end() ) break;
			
	lineControlers[size-1].distTo=origin.dist(lineControlers[size-1].prPoint)+c;
			
	if( size!=1 )
	  if( IsLastControler(&lineControlers[size-1],id) ) {
	    lastControler=true;
	    break;
	  }
			
	lineControlers.push_back(*player);
	lineControlers[lineControlers.size()-1].distFrom=lineControlers[lineControlers.size()-2].distTo;
			
	controlers.erase(player, player+1);
      }
      if( !lastControler )
	lineControlers[lineControlers.size()-1].distTo=maximalPassDistance;
		
      allControlers.push_back(lineControlers);
    }
	
    //explore the current situation
    //the return is the number of possible passes
	
    bool success=GetPassInformation(true);
	
    //next code uses the posibility of sending simple graphic commands to monitor via sserver or straight
    //it's done for testing purposes, and in the real games it should not be used
	
    //next code draws the pass lines in different colors - it depends on the player side, controlling the line segment
	
    if( doLog ) {
      for( unsigned int idA=0;idA<allControlers.size();idA++ ) {
	for( unsigned int idC=0;idC<allControlers[idA].size();idC++ ) {
	  if( allControlers[idA][idC].side==Mem->MySide ) {
	    logGr.AddLine(origin+Polar2Vector(allControlers[idA][idC].distFrom,passAngles[idA]), origin+Polar2Vector(allControlers[idA][idC].distTo,passAngles[idA]),"2816EB");
	  }else{
	    logGr.AddLine(origin+Polar2Vector(allControlers[idA][idC].distFrom,passAngles[idA]), origin+Polar2Vector(allControlers[idA][idC].distTo,passAngles[idA]),"DD3739");
	  }
	  //logGr.AddLine(origin+Polar2Vector(allControlers[idA][idC].distFrom,passAngles[idA]), Mem->PlayerAbsolutePosition(allControlers[idA][idC].side,allControlers[idA][idC].num),"E9EFFF");
	}
      }
    }
	
    return success;
  }
  //----------------------------------------------------
  int Pass::GetPassInformation( bool doLog ) {
    //clear previous pass information
    for( int id=0;id<11;id++ )
      passInfos[id].clear();
	
    Vector origin=Mem->BallAbsolutePosition();
    const float  distDiff=1.5f;
    static const float maxPassSpeed=2.4f;
    float passDist;
    PassInfo info;
    Unum dangerOpp;
    vector<PassInfo> tmpInfos;
	
    for( unsigned int idA=0;idA<allControlers.size();idA++ ) {
		
      if( allControlers[idA].size()<=1 ) {
	//there is no controlers or just one - that player, which controls the ball - i don't care about him
	//allControlers[idA].clear();
	continue;
      }
      //erase this player from list - to make scene more clear
      allControlers[idA].erase(allControlers[idA].begin(), allControlers[idA].begin()+1);
		
      for( unsigned int idC=0;idC<allControlers[idA].size();idC++ ) {
		
	if( allControlers[idA][idC].side==Mem->TheirSide ) continue;
							
	//so, iter points to a player of my side 				
	info.Clear();
	info.SetTime(Mem->CurrentTime);
	info.SetReceiver(allControlers[idA][idC].num);
	passDist=allControlers[idA][idC].distFrom+1.0f;
	tmpInfos.clear();
			
	while( true ) {
	  if( passDist>=allControlers[idA][idC].distTo-1.0f ) break;
	  if( GetDiff(Mem->AngleToGlobal(allControlers[idA][idC].plPos),passAngles[idA])<=4.0f &&  
	      passDist>=Mem->TeammateDistance(allControlers[idA][idC].num)+0.9f*Mem->SP_kickable_area ) break;
					
	  info.SetPassPos(origin+Polar2Vector(passDist,passAngles[idA]));				
	  //filters for pass position
	  if( !Mem->FieldRectangle.shrink(2.0f).IsWithin(info.GetPassPos()) ||
	      Mem->MyPos().dist(info.GetPassPos())<=4 ||
	      Mem->OwnPenaltyArea.IsWithin(info.GetPassPos()) ) {
						
	    passDist+=distDiff;
	    continue;
						
	  }
	  //temporal saving till ball speed will be found
	  //why +2? - reaction buffer for teammate
	  info.SetReceiverCycles((float)NumCyclesToPosition(&allControlers[idA][idC],info.GetPassPos())+2.0f);					
				
	  dangerOpp=FastestOppToPosition(info.GetPassPos());
	  info.SetOpponent(dangerOpp);
	  if( dangerOpp!=Unum_Unknown ) {
	    float oppDist=Mem->OpponentAbsolutePosition(dangerOpp).dist(info.GetPassPos());
	    info.SetOpponentCycles( oppDist/Mem->SP_player_speed_max+0.5f );
	  }else{
	    info.SetOpponentCycles(InfCycles);
	  }
				
	  //why +1? - i want teammate to reach the pass position in one cycle before the ball does
	  //plus 1 - buffer
	  info.SetSpeed( Min(passDist*(1-Mem->SP_ball_decay)/(1-pow(Mem->SP_ball_decay,info.GetReceiverCycles()+2)),Mem->SP_ball_speed_max) );
	  //if initial speed is too big, pass infos fails
	  if( info.GetSpeed()>maxPassSpeed ) {
	    passDist+=distDiff;
	    continue;
	  }
	  //final calculations, when initial ball speed is known
	  info.SetReceiverCycles( SolveForLengthGeomSeries(info.GetSpeed(),Mem->SP_ball_decay,passDist) );
				
	  info.SetConfidence(interception.ball_control(info.GetReceiverCycles(), info.GetOpponentCycles()));
	  if( info.GetConfidence()>PassThreshold() ) 
	    tmpInfos.push_back(info);
				
	  passDist+=distDiff;
	}
	//if we have only one pass position in line segment, skip this segment
	//if( tmpInfos.size()<=1 ) continue;
			
	for( unsigned int id=0;id<tmpInfos.size();id++ )
	  AddPassInfo(tmpInfos[id].GetReceiver(),tmpInfos[id]);
      }
		
    }
	
    //just draws pass positions, represented by small circles, on the field
	
    if( doLog ) 
      for( int id=0;id<11;id++ ) 
	for( unsigned int idP=0;idP<passInfos[id].size();idP++ ) 
	  logGr.AddCircle(passInfos[id][idP].GetPassPos(),0.2,"000000",true);
			
    return NumPassInfos();
  }
  //----------------------------------------------------
  bool Pass::SetControlers(vector<Controler>& controlers, unsigned int angleID) {
    controlers.clear();
	
    unsigned int id;
    Ray ray(Mem->BallAbsolutePosition(), passAngles[angleID]);
    Controler controler;
	
    for( id=0;id<teammates.size();id++ ) {
      controler=teammates[id];
      controler.plPos=PlayerPosition(controler.num, controler.side, angleID);
      if( !PlayerValid(ray, controler) ) continue;
      controlers.push_back(controler);
    }
	
    for( id=0;id<opponents.size();id++ ) {
      controler=opponents[id];
      controler.plPos=PlayerPosition(controler.num, controler.side, angleID);
      if( !PlayerValid(ray, controler) ) continue;
      controlers.push_back(controler);
    }
	
    return !(controlers.size()==0);
  }
  //----------------------------------------------------
  bool Pass::PlayerValid(Ray& ray, Controler& controler) {
    if( GetDiff(ray.dir(), (controler.plPos-ray.Origin()).dir())>90 ) return false;
    controler.prPoint=Line(ray).ProjectPoint(controler.plPos);
    controler.distToLine=controler.plPos.dist(controler.prPoint);	
    return true;
  }
  //----------------------------------------------------
  Vector Pass::PlayerPosition(Unum num, char side, unsigned int angleID) {
    //you can make some adjustments here to stay well with player position valid
    //(some compensations can be given)
    //remember, that ray, represented by angleID, starts at ball position
    //i don't care about pass position in this function
    return Mem->PlayerAbsolutePosition(side, num);
  }
  //----------------------------------------------------
  Unum Pass::FastestOppToPosition(Vector position) {
    Unum opp=Unum_Unknown;
    float minDist=Mem->SP_pitch_length, tmpDist;
	
    for( unsigned int id=0;id<opponents.size(); id++ )
      if( (tmpDist=position.dist(Mem->OpponentAbsolutePosition(opponents[id].num)))<minDist ) {
	minDist=tmpDist;
	opp=opponents[id].num;			
      }
    return opp;
  }
  //----------------------------------------------------
  bool Pass::IsLastControler(Controler* controler, unsigned int angleID) {
    float maxSpeed=100.0f, speed;
    Vector pos,tmpPos;
	
    tmpPos=Mem->BallAbsolutePosition()+Polar2Vector(controler->distFrom,passAngles[angleID]);
    speed=NumCyclesToPosition(controler,tmpPos);
    if( speed<=maxSpeed ) {
      maxSpeed=speed;
      pos=tmpPos;
    }

    tmpPos=Mem->BallAbsolutePosition()+Polar2Vector(controler->distTo,passAngles[angleID]);
    speed=NumCyclesToPosition(controler,tmpPos);
    if( speed<=maxSpeed ) {
      maxSpeed=speed;
      pos=tmpPos;
    }
		
    float tmpDist=Mem->BallAbsolutePosition().dist(controler->prPoint);
    if( tmpDist>=controler->distFrom && tmpDist<=controler->distTo ) {
      speed=NumCyclesToPosition(controler,controler->prPoint);
      if( speed<=maxSpeed ) {
	maxSpeed=speed;
	pos=controler->prPoint;
      }
    }
	
    if( speed<=1.0f ) return true;
		
    speed=Mem->BallAbsolutePosition().dist(pos)*(1-Mem->SP_ball_decay)/(1-pow(Mem->SP_ball_decay,speed));

    return speed>=0.95f*Mem->SP_ball_speed_max;
  }
  //----------------------------------------------------
  int Pass::NumPassInfos() {
    int numPassInfos=0;
	
    for( int id=0;id<11;id++ )
      numPassInfos+=passInfos[id].size();
	
    return numPassInfos;
  }
  //----------------------------------------------------
  int Pass::NumCyclesToPosition(Controler* controler, Vector position) {

	
    float buffer=0.8f*Mem->SP_kickable_area+Mem->SP_player_size;
	
    if( controler->side==Mem->MySide && controler->num==Mem->OurGoalieNum ) buffer=0.8f*Mem->SP_catch_area_l+Mem->SP_player_size;
    if( controler->side==Mem->TheirSide && controler->num==Mem->TheirGoalieNum ) buffer=0.8f*Mem->SP_catch_area_l+Mem->SP_player_size;
		
    return Mem->PlayerPredictedCyclesToPoint( 	controler->side, 
						controler->num,
						position,
						Mem->SP_max_power,
						buffer	);
	
    //next code has some BUGS!!!!!!!(DO NOT USE THAT)
    static Vector plPos;
    static float  bdAng;
    static float  angToPos;
    static Vector plVel;
    static float  speed=Mem->SP_max_power*Mem->SP_dash_power_rate;
	
    plPos=controler->plPos;
    if( Mem->PlayerBodyAngleValid(controler->side, controler->num) )
      bdAng=Mem->PlayerAbsoluteBodyAngle(controler->side, controler->num);
    else {
      if( controler->side==Mem->TheirSide ) 
	bdAng=angToPos;
      else
	bdAng=GetNormalizeAngleDeg(angToPos+180.0f);
    }
    if( Mem->PlayerVelocityValid(controler->side, controler->num) )
      plVel=Mem->PlayerAbsoluteVelocity(controler->side, controler->num);
    else
      plVel=Vector(0,0);

    int i;
    for( i=0;i<100;i++ ) {
      if( position.dist(plPos)<=buffer ) return i;
      angToPos=(position-plPos).dir();			
      if( GetDiff(angToPos, bdAng)>1.0f )	{
	float diff=GetDiff(angToPos,bdAng);
	if( diff>Mem->CP_max_go_to_point_angle_err) {
	  float this_turn = MinMax(-Mem->EffectiveTurn(Mem->SP_max_moment, plVel.mod()),
				   diff,
				   Mem->EffectiveTurn(Mem->SP_max_moment, plVel.mod()));
	  bdAng += this_turn;
	  NormalizeAngleDeg(&bdAng);
	  continue;
	}
      }
		
      plVel+=Polar2Vector(min(speed,plPos.dist(position)), bdAng);
      if( plVel.mod()>Mem->SP_player_speed_max ) plVel=plVel*Mem->SP_player_speed_max/plVel.mod();
      plPos+=plVel;
		
    }
    return i;
  }
  //----------------------------------------------------
  float Pass::GetPassPriority(PassInfo& info) {

    return 0.5f*GetCongestionPriority( info )+
      0.35f*GetTransitionPriority( info )+
      0.15f*GetPositioningPriority(info );

  }
  //----------------------------------------------------
  float Pass::GetCongestionPriority(Vector pos, Unum pl, float r) {
    //min congestion 0.0f
    //max congestion 3.0f
    //gamma          0.7f
    //congestion is normalized to interval [0,1]
	
    static float congestion1;	//around the pass point 
    static float congestion2;	//arount the player position
    static float gamma=0.7f;
    static float maxCong=2.0f;
    static float dist;
	
    congestion1=congestion2=0.0f;
    for( unsigned int id=0;id<opponents.size();id++ ) {
      if(!Mem->OpponentPositionValid(opponents[id].num))//add by AI
	continue;
      //first, let's see the pass position
      dist=pos.dist(Mem->OpponentAbsolutePosition(opponents[id].num));
      if( dist<=r ) 
	congestion1+=1/dist;
		
      //now, let's check the player position
      if(Mem->TeammatePositionValid(pl))//add by AI
	dist=Mem->TeammateAbsolutePosition(pl).dist(Mem->OpponentAbsolutePosition(opponents[id].num));
      else
	dist=1000.0f;
      if( dist<=r ) 
	congestion2+=1/dist;			
    }
	
    congestion1=min(congestion1,maxCong)/maxCong;
    congestion2=min(congestion2,maxCong)/maxCong;
	
    return 1.0f-(gamma*congestion1+(1-gamma)*congestion2);
  }
  //----------------------------------------------------
  float Pass::GetPositioningPriority(PassInfo& info) {
    static float passDist;
    static float angleDiff;
    static float maxAngleDiff=45.0f;
    static float gamma=0.6f;

    if(!Mem->TeammatePositionValid(info.GetReceiver()))//add by AI
      return 0.0f;
    passDist=Mem->BallAbsolutePosition().dist(info.GetPassPos());
    angleDiff=GetDiff( Mem->AngleToGlobal(
					  Mem->TeammateAbsolutePosition(info.GetReceiver())),
		       Mem->AngleToGlobal(info.GetPassPos()) );
	
    passDist  = min(passDist,GetMaxPassDistance())/GetMaxPassDistance();
    passDist  = -2.8f*passDist*passDist+2.8*passDist+0.3;

    Vector theirGoal = Vector(Mem->SP_pitch_length/2,0);
    Vector teamPos   = Mem->TeammateAbsolutePosition(info.GetReceiver());
    Vector passPos   = info.GetPassPos();
	
    if( Pos.GetPlayerType(info.GetReceiver())==PT_Forward && passPos.dist(theirGoal)<teamPos.dist(theirGoal)) {
      angleDiff = 1.0f-Min(fabs(Min(angleDiff,maxAngleDiff)-30.0f),20.0f)/20.0f;
    }else{
      angleDiff = 1.0f-min(angleDiff,maxAngleDiff)/maxAngleDiff;
    }

    return gamma*passDist+(1-gamma)*angleDiff;
  }
  //----------------------------------------------------
  void Pass::pass(Unum teammate)
  {
    if( !dpasses[teammate-1].Valid() )
      dpasses[teammate-1]=explore_pass_to_teammate(teammate);				

    dpasses[teammate-1].Log(10);
    bool fast=(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.98f&&
      Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<4.5f)||Mem->ClosestOpponentToBallDistance()<2.0f;

    SK_Res res = smartkick(	dpasses[teammate-1].GetSpeed(),
				GetKickAngle(dpasses[teammate-1].GetPassPos()),
				fast?SK_Fast:SK_Safe );

    if( res==SK_KickDone ) {
      Mem->passvel=Polar2Vector( dpasses[teammate-1].GetSpeed(),
				 (dpasses[teammate-1].GetPassPos()-Mem->MyPos()).dir() );
      //    if( dpasses[teammate-1].Receiver()!=Mem->MyNumber )
      //      Mem->SayNow(ST_pass_decision, dpasses[teammate-1].Receiver());
    }
    //  else {
    //    if( dpasses[teammate-1].Receiver()!=Mem->MyNumber )
    //      Mem->SayNow(ST_pass_intention, dpasses[teammate-1].Receiver());
    //  }
  }
  //----------------------------------------------------
  //----------------------------------------------------	
  void Pass::pass(AngleDeg angle, float speed, Unum tm) {
    bool fast=(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.98f&&
      Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<4.5f)||Mem->ClosestOpponentToBallDistance()<2.0f;

    SK_Res res = smartkickg( speed, angle, fast?SK_Fast:SK_Safe);
    Mem->LogAction8(10,"Pass to tm %.0f (conf %.2f) at (%.2f,%.2f) with vel=%.2f and ang=%.2f",
		    float(tm),Mem->TeammatePositionValid(tm),Mem->TeammateX(tm),Mem->TeammateY(tm),speed,angle);
    if( res==SK_KickDone ) {
      Mem->passvel=Polar2Vector( speed, angle );
    }
	}
  //----------------------------------------------------
  //----------------------------------------------------
  void Pass::pass(PassInfo& info) {
    pass(Mem->AngleToGlobal(info.GetPassPos()),info.GetSpeed(), info.GetReceiver());
  }
  //----------------------------------------------------
  //----------------------------------------------------
  Unum Pass::BestPass() {
    //explore_all_passes();
    Unum best=Unum_Unknown;
    float x=-100;

    for(int player=1; player<=11; player++) {
      if( player==Mem->MyNumber ) continue;
      if( !dpasses[player-1].Valid() ) continue;
      dpasses[player-1].Log(10);
      if( dpasses[player-1].GetConfidence()<pass_threshold ) continue;
      if( dpasses[player-1].GetPassPos().x<Mem->MyX() ) continue;		
		
      if( dpasses[player-1].GetPassPos().x>x ) {
	best=player;
	x=dpasses[player-1].GetPassPos().x;
      }		
    }
    return best;
  }
  //----------------------------------------------------
  //----------------------------------------------------	
  float Pass::PassFromPoint2Point(Vector from, Vector to, Unum& opponent) {

    opponent = Unum_Unknown;
    float passvel,
      min_conf = 1.0f,
      tmp_conf;

    for(int opp=1; opp<=11; opp++) {
      if( !Mem->OpponentPositionValid(opp) ) continue;
      if( !InBetween(Mem->OpponentAbsolutePosition(opp), from, to) ) continue;

      if( opp==Mem->TheirGoalieNum )
	tmp_conf = interception.neuro_goalie_intercept(opp, from, to, 0.8f, passvel);
      else
	tmp_conf = interception.neuro_player_intercept(opp, from, to, 0.8f, passvel);

      if( tmp_conf<min_conf ) {
	min_conf = tmp_conf;
	opponent=opp;
      }
    }
	
    return min_conf;
  }
  //----------------------------------------------------
  //----------------------------------------------------	
  void Pass::Test() {
	
    // draw the pass regions	
		
    Vector dist(Mem->SP_pitch_length/5, Mem->SP_pitch_width/4);
    for( int id=0;id<=5;id++ ) {
      logGr.AddLine(	Vector(-Mem->SP_pitch_length/2+id*dist.x,-Mem->SP_pitch_width/2),
			Vector(-Mem->SP_pitch_length/2+id*dist.x,Mem->SP_pitch_width/2),
			"402FF5" );
    }
	
    for( int id=0;id<=4;id++ ) {
      logGr.AddLine(	Vector(-Mem->SP_pitch_length/2,-Mem->SP_pitch_width/2+id*dist.y),
			Vector( Mem->SP_pitch_length/2,-Mem->SP_pitch_width/2+id*dist.y),
			"402FF5" );
    }
	
	
    //draw the positions
	
    /*
      for( int y=0;y<Y;y++ )
      for( int x=0;x<X;x++ )
      logGr.AddCircle(priorities.Positions(y,x),0.5,"000000",true);
	
    */
	
	
  }
  //----------------------------------------------------
  //----------------------------------------------------	
  void Pass::GetPassMessage(  char* msg,
			      sayType st,
			      Unum teammate ) {
    float ballx=Mem->BallX(),
      bally=Mem->BallY(),
      ballvelx=Min(Mem->BallAbsoluteVelocity().x,Mem->SP_ball_speed_max),
      ballvely=Min(Mem->BallAbsoluteVelocity().x,Mem->SP_ball_speed_max),
      myx=Mem->MyX(),
      myy=Mem->MyY();
    int temp;

    msg[0]=Mem->code(int(st));

    switch( st ) {
    case ST_pass_intention:
      //teammate, which is considered as a receiver
      msg[1]=Mem->code(int(teammate)+15);
      //now i code ball position
      temp=int((ballx+signf(ballx)*0.05)*10)+int(Mem->SP_pitch_length/2)*10;
      msg[2]=Mem->code(temp/73);
      msg[3]=Mem->code(temp%73);
      msg[4]=Mem->code(int(bally+signf(bally)*0.5)+int(Mem->SP_pitch_width/2));
      //now i code my position
      temp=int((myx+signf(myx)*0.05)*10)+int(Mem->SP_pitch_length/2)*10;
      msg[5]=Mem->code(temp/73);
      msg[6]=Mem->code(temp%73);
      msg[7]=Mem->code(int(myy+signf(myy)*0.5)+int(Mem->SP_pitch_width/2));
      //here we have 2 bytes free
      break;

    case ST_pass_decision: {
      //here i code receiver uniform
      msg[1]=Mem->code(int(teammate)+15);
      //now i code ball position
      temp=int((ballx+signf(ballx)*0.05)*10)+int(Mem->SP_pitch_length/2)*10;
      msg[2]=Mem->code(temp/73);
      msg[3]=Mem->code(temp%73);
      msg[4]=Mem->code(int(bally+signf(bally)*0.5)+int(Mem->SP_pitch_width/2));
      //now i code pass speed
      Vector passspeed=Mem->passvel;

      ballvelx=Min(passspeed.x,Mem->SP_ball_speed_max),
	ballvely=Min(passspeed.y,Mem->SP_ball_speed_max);
      ballvelx=(ballvelx/Mem->SP_ball_speed_max+1)*35;
      ballvely=(ballvely/Mem->SP_ball_speed_max+1)*35;

      msg[5]=Mem->code( int( ballvelx+0.5f ) );
      msg[6]=Mem->code( int( ballvely+0.5f ) );
      //now i code my position
      temp=int((myx+signf(myx)*0.05)*10)+int(Mem->SP_pitch_length/2)*10;
      msg[7]=Mem->code(temp/73);
      msg[8]=Mem->code(temp%73);
      msg[9]=Mem->code(int(myy+signf(myy)*0.5)+int(Mem->SP_pitch_width/2));
      break;
    }
    default:
      my_error("Get message::unknown type of message: %.0f", float(st));
    }
  }
  //----------------------------------------------------
  //----------------------------------------------------	
  void Pass::ParsePassMessage(  char* msg,
				sayType st,
				Unum from,
				Time t ) {

    float ballx, bally, ballvelx, ballvely, teamx, teamy;
    Unum teammate;

    switch( st ) {
    case ST_pass_intention:
      //here i decode teammate, who is receiver of the pass
      teammate=Mem->decode(msg[0])-15;
      //now i decode ball position
      ballx=(Mem->decode(msg[1])*73+Mem->decode(msg[2]))/10.0-int(Mem->SP_pitch_length/2);
      bally=Mem->decode(msg[3])-Mem->SP_pitch_width/2;
      //this is. where i save information
      Mem->HearBall(ballx, bally, 1.0f,0.0f,t-1);
      //now i want to decode position of the passer
      teamx=(Mem->decode(msg[4])*73+Mem->decode(msg[5]))/10.0-int(Mem->SP_pitch_length/2);
      teamy=Mem->decode(msg[6])-Mem->SP_pitch_width/2;
      Mem->HearPlayer(Mem->MySide, from, teamx, teamy,1.0f,0.0f,t-1);
      Mem->SetTeamRecvInfo(teammate, from, Mem->CurrentTime);
      break;

    case ST_pass_decision:
      //here i decode teammate, who is receiver of the pass
      teammate=Mem->decode(msg[0])-15;
      //now i decode ball position
      ballx=(Mem->decode(msg[1])*73+Mem->decode(msg[2]))/10.0-int(Mem->SP_pitch_length/2);
      bally=Mem->decode(msg[3])-Mem->SP_pitch_width/2;
      //this is a place to decode pass speed
      ballvelx=( float( Mem->decode(msg[4]) )/35-1 )*Mem->SP_ball_speed_max;
      ballvely=( float( Mem->decode(msg[5]) )/35-1 )*Mem->SP_ball_speed_max;
      Mem->HearBall(ballx, bally, 1.0f, ballvelx, ballvely, 1.0f, 0.0f, t-1);
      //this is a place to decode position of the passer
      teamx=(Mem->decode(msg[6])*73+Mem->decode(msg[7]))/10.0-int(Mem->SP_pitch_length/2);
      teamy=Mem->decode(msg[8])-Mem->SP_pitch_width/2;
      Mem->HearPlayer(Mem->MySide, from, teamx, teamy,1.0f,0.0f,t-1);
      //this is, where i save information
      Mem->SetTeamRecvInfo(teammate, from, Mem->CurrentTime);
    default:
      my_error("parse_message::not right type of message:%.0f", float(st));
    }
  }
  //----------------------------------------------------
  //----------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------		
namespace newPass{
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------		
Pass::Pass() {
	ballDecays=0;
  passThreshold=0.5001f;
	minPassDistance=2;
	maxPassDistance=25;
	playerValidThreshold=0.95;
	baseAngleValidThreshold=0.74f;		//angle conf decay=0.93
  additionalAngleValidThreshold=0.69f;
	bufferAngle=15.0f;
	maxBallEndSpeedSlow=1.5f;
	maxBallEndSpeedFast=2.0f;
		
	ballDecays = new float[NUMBER_OF_STEPS];
	if( ballDecays==0 ) return;
			
	lastHopePass.Clear();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::Initialize() {
	ballDecays[0]=1;
	ballDecays[1]=Mem->SP_ball_decay;
	for( int id=2;id<NUMBER_OF_STEPS;id++ )
		ballDecays[id]=ballDecays[id-1]*Mem->SP_ball_decay;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Pass::~Pass() {
	if( ballDecays!=0 ) delete ballDecays;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::FindPasses() {
	opponents.clear();
	teammates.clear();
	allPassInfos.clear();
	throughPassInfos.clear();
	backPasses.clear();
	backThroughPasses.clear();
	lastHopePass.Clear();
		
	if( !SetPassableTeammatesAndConsideredOpponents() ) {
		Mem->LogAction2(10,"no potential receivers->no pass oppotunity");
		return;
	}
	ExploreDirectPasses();
	ExlploreThroughtPasses();	
	
	//kind of hack-:)
	float congestion;
	float minCongestion=1000;
	int numPlayers;
	int minNumPlayers=100;
	Unum teammate=Unum_Unknown;
	for( unsigned int id=0;id<teammates.size();id++ ) {
		if( Pos.GetTmPos(teammates[id]).x+3<Mem->MyX() ) continue;
		numPlayers = Mem->NumOpponentsInConeToTeammate(.3,teammates[id]);
		if( numPlayers<minNumPlayers ) {
			teammate=teammates[id];
			minNumPlayers=numPlayers;
			minCongestion = Mem->CongestionWithOnlyOpponents(Pos.GetTmPos(teammates[id]));
		}else if( numPlayers==minNumPlayers ) {
			congestion=Mem->CongestionWithOnlyOpponents(Pos.GetTmPos(teammates[id]));
			if( congestion<minCongestion ) {
				minCongestion=congestion;
				teammate=teammates[id];
			}
		}
	}
	
	if( teammate==Unum_Unknown || minNumPlayers>=1 ) return;
	
	float passSpeed=GetPassSpeed(teammate,Pos.GetTmPos(teammate),maxBallEndSpeedFast);
	lastHopePass.SetInfo(teammate,0,0.9,Pos.GetTmPos(teammate),passSpeed,Unum_Unknown,0,0.0f);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::ComputeBaseAngleValidThreshold(int cyclesThreshold) {
	float angleDecay=eye.AngleDecay();
	baseAngleValidThreshold=pow(angleDecay,cyclesThreshold-1)+0.0001f;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::ComputeAdditionalAngleValidThreshold(int cyclesThreshold) {
	float angleDecay=eye.AngleDecay();
	additionalAngleValidThreshold=pow(angleDecay,cyclesThreshold-1)+0.0001f;		
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::IsAnglePassable(AngleDeg globalAngle) const {
	return eye.AngleConf(globalAngle)>=baseAngleValidThreshold && 
				 eye.AngleConf(globalAngle+bufferAngle)>=additionalAngleValidThreshold &&
				 eye.AngleConf(globalAngle-bufferAngle)>=additionalAngleValidThreshold;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::pass(PassInfo& info) {
	info.Log(10);
	bool fast=(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.98f&&
		   Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<4.5f)||Mem->ClosestOpponentToBallDistance()<2.0f;

	smartkickg( info.GetSpeed() , Mem->AngleToGlobal(info.GetPassPos()) ,fast?SK_Fast:SK_Safe );
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::IsTeammatePassable(Unum tm) const {
	if( Mem->TeammatePositionValid(tm)<playerValidThreshold ) {
		Mem->LogAction4(10,"teammate %.0f is not passable because of conf %.3f",float(tm),Mem->TeammatePositionValid(tm));
		return false;
	}
	if( tm==Mem->MyNumber || tm==Mem->OurGoalieNum ) {
		Mem->LogAction3(10,"teammate %.0f is goalie or me",float(tm));
		return false;
	}
	float  dist = Mem->TeammateDistance(tm);
	Vector pos  = Mem->TheirPenaltyArea.IsWithin(Pos.GetTmPos(tm))?Pos.GetSmartPassPoint(tm):Pos.GetTmPos(tm);//Mem->TeammateAbsolutePosition(tm);
	if( pos.x>Mem->my_offside_line ||(Mem->my_offside_opp!=Unum_Unknown&&pos.x+2.0f>Mem->my_offside_line)){//mod by AI
		Mem->LogAction3(10,"teammate %.0f is too close or beyond the offside line",float(tm));
		return false;
	}
	if( !Mem->FieldRectangle.IsWithin(pos) ) {
		Mem->LogAction3(10,"teammate %.0f is outta field",float(tm));
		return false;
	}
	if( dist<minPassDistance || dist>maxPassDistance ) {
		Mem->LogAction3(10,"teammate %.0f is too close or too far",float(tm));
		return false;
	}
	if( Mem->OwnPenaltyArea.IsWithin(pos) ) return IsTeammatePassableInOurPenaltyArea(tm);
	if( Mem->TeammateX(tm)<Mem->MyX()-5 )   return IsTeammateThatIsBackOfMePassable(tm);		
	return true;			
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::IsTeammatePassableInOurPenaltyArea(Unum tm) const {
	Mem->LogAction3(10,"teammate %.0f is in our penalty area",float(tm));
	return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::IsTeammateThatIsBackOfMePassable(Unum tm) const {
  if( Mem->TheirPenaltyArea.IsWithin(/*Mem->TeammateAbsolutePosition(tm)*/
				     Mem->TheirPenaltyArea.IsWithin(Pos.GetTmPos(tm))?Pos.GetSmartPassPoint(tm):Pos.GetTmPos(tm)) ) return true;
	Mem->LogAction3(10,"teammate %.0f is back of me",float(tm));			
	return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::IsOpponentConsidered(Unum opp) const {
	return (bool)Mem->OpponentPositionValid(opp);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::SetPassableTeammatesAndConsideredOpponents() {
	teammates.clear();
	opponents.clear();
	InterceptionInput data;
	data.vBallPos = Mem->BallAbsolutePosition();
	if( Mem->BallVelocityValid() )
		data.vBallVel=Mem->BallAbsoluteVelocity();
	else
		data.vBallVel=Vector(0,0);
	for( int player=1;player<=11;player++ ) {
		if( IsTeammatePassable(player) ) {
		  logGr.AddCircle(/*Mem->TeammateAbsolutePosition(player)*/Pos.GetTmPos(player),2,"FD2D68");
			teammates.push_back(player);
			data.vPlayerPos=Pos.GetTmPos(player);//Mem->TeammateAbsolutePosition(player);
			if( Mem->TeammateVelocityValid(player) )
				data.vPlayerVel=Mem->TeammateAbsoluteVelocity(player);
			else
				data.vPlayerVel=Vector(0,0);
			if( Mem->TeammateBodyAngleValid(player) )
				data.fPlayerAng=Mem->TeammateAbsoluteBodyAngle(player);
			else
				data.fPlayerAng=0;
			teamInfo[player-1] = data;					
		}
		if( IsOpponentConsidered(player) ) {
			logGr.AddCircle(Mem->OpponentAbsolutePosition(player),2,"1A23D3");				
			opponents.push_back(player);
			data.vPlayerPos=Mem->OpponentAbsolutePosition(player);
			if( Mem->OpponentVelocityValid(player) )
				data.vPlayerVel=Mem->OpponentAbsoluteVelocity(player);
			else
				data.vPlayerVel=Vector(0,0);
			if( Mem->OpponentBodyAngleValid(player) )
				data.fPlayerAng=Mem->OpponentAbsoluteBodyAngle(player);
			else
				data.fPlayerAng=0;
			oppInfo[player-1] = data;					
		}
	}
	return teammates.size()!=0;		
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::PassConfidence(float tmCycles, float oppCycles) {
	float pow=(oppCycles-tmCycles)/3;
	return 1/(1+exp(-pow));
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::GetMaxPassSpeed(Vector passPos,float maxEndPassSpeed) {
	float dist = passPos.dist(Mem->BallAbsolutePosition());
	return Min(Mem->SP_ball_speed_max,maxEndPassSpeed*Mem->SP_ball_decay+dist*(1-Mem->SP_ball_decay));
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::GetPassSpeed(Unum player, Vector passPos, float maxEndPassSpeed,int* cycles) {
	//it's not neccesery that the agent will get the ball in passPos position
	//peharps, it will get it in earlier position.
	static float logQ=log(Mem->SP_ball_decay);
	static float oneMq=1-Mem->SP_ball_decay;
	float  dist=passPos.dist(Mem->BallAbsolutePosition());
	float passSpeed=GetMaxPassSpeed(passPos,maxEndPassSpeed);
	int cyclesToPosition = Max(1,Min(NUMBER_OF_STEPS-1,PredictPlayerCyclesToPosition(player,Mem->MySide,passPos)))+1;
	float newPassSpeed=dist*(Mem->SP_ball_decay-1)/(ballDecays[cyclesToPosition]-1);
	passSpeed = Min(passSpeed,newPassSpeed);
	cyclesToPosition=int(log(1-dist*oneMq/passSpeed)/logQ+0.5f);
	if( cycles!=0 ) *cycles=cyclesToPosition;
	return passSpeed;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int Pass::PredictPlayerCyclesToPosition(Unum player,char side, Vector position) {
	if( !Mem->PlayerPositionValid(side,player) ) return InfCycles;		
	Vector playerPos=Mem->PlayerAbsolutePosition(side,player);
	float dist=playerPos.dist(position);
	if( dist<0.5f ) return 0;
	
	int additionalCycles=1;//acceleration
	if( Mem->PlayerBodyAngleValid(side,player) ) {
		AngleDeg ang=Mem->PlayerAbsoluteBodyAngle(side,player);
		ang=GetDiff(ang,(position-playerPos).dir());
	
		if( ang>20&&ang<100 ) additionalCycles+=1;
		else if( ang>=100 )   additionalCycles+=2;
	}
	
	return additionalCycles+int(ceil(dist/Mem->SP_player_speed_max));
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::Test() {
	//test pass speed
		//begin
/*				if( Mem->MyNumber==1 ) {
				static Vector passPos=Vector(0,0);
				if( Mem->BallKickable() ) {
					if( !Mem->TeammatePositionValid(2) ) return;
					Vector pos = Mem->TeammateAbsolutePosition(2);
					float dist = range_random(0,15);
					float angle=range_random(-180,180);
					passPos=pos+Polar2Vector(dist,angle);
					float  speed=GetPassSpeed(2,passPos);
					PassInfo info;info.SetSpeed(speed);info.SetPassPos(passPos);
					pass(info);
					Mem->LogAction5(10,"pass info: poa (%.2f, %.2f) speed %.3f",passPos.x, passPos.y,speed);				
				}else{
					if( Mem->FastestTeammateToBall()==1 && Mem->BallDistance()<5 ) {
						get_ball();
						return;
					}
				}
				logGr.AddCircle(passPos,1,"FF3F0A",false);
			}else{
				if( !Mem->TeammatePositionValid(1) ) return;
				if( Mem->BallKickable() ) {
					Mem->LogAction4(10,"ball pos (%.2f, %.2f)",Mem->BallX(), Mem->BallY());
					stopball();
					return;
				}
				if( Mem->FastestTeammateToBall()==2 ) {
					get_ball();
					return;
				}
				face_only_body_to_ball();
				return;
			}*/
		//end		
		
		//test pass priorities
		//begin		
/*			if( Mem->MySide=='r' ) {
					scan_field_with_body();
					return;
				}
				if( Mem->MyNumber==1 ) {
				if( Mem->BallKickable() ) {
					if( !Mem->TeammatePositionValid(2) ) return;
					scan_field_with_body();
					PassInfo info = FindBestPass();					*/
			/*		
					Vector bestPass=Vector(0,0);
					Vector pos;
					float p;
					float maxPriority=0.0f;
				  for( unsigned int id=0;id<passAngles.size();id++ ) {
						for( int t=0;t<6;t++ ) {
							pos=Mem->BallAbsolutePosition()+Polar2Vector(t*5,passAngles[id]);
							p=GetPassPriority(0.9,Mem->MyNumber,pos);
							if( p>maxPriority ) {
								maxPriority=p;
								bestPass=pos;
							}
						}
					}
					logGr.AddCircle(bestPass,0.2,"FF3F0A",true);*/
/*				}else{
					if( Mem->FastestTeammateToBall()==1 && Mem->BallDistance()<5 ) {
						get_ball();
						return;
					}else{
						scan_field_with_body();
					}
				}
			}else{
				if( !Mem->TeammatePositionValid(1) ) return;
				if( Mem->BallKickable() ) {
					Mem->LogAction4(10,"ball pos (%.2f, %.2f)",Mem->BallX(), Mem->BallY());
					if( Mem->BallMoving() ) stopball();
					return;
				}
				if( Mem->FastestTeammateToBall()==2 ) {
					get_ball();
					return;
				}
				face_only_body_to_ball();
				return;
			}*/
		//end
		
		//test pass confience, pass routes, pass sectors and so on...
		//begin
				if( Mem->MySide=='r' ) {
					scan_field_with_body();
					return;
				}
				if( Mem->MyNumber==1 ) {
				if( Mem->BallKickable() ) {
					opponents.clear();
					teammates.clear();
					allPassInfos.clear();
					for( int id=2;id<=11;id++ )
						if( Mem->TeammatePositionValid(id) ) teammates.push_back(id);
						
					ExlploreThroughtPasses();
					scan_field_with_body();
				}else{
					if( Mem->FastestTeammateToBall()==1 && Mem->BallDistance()<5 ) {
						get_ball();
						return;
					}else{
						scan_field_with_body();
					}
				}
			}else{
				if( !Mem->TeammatePositionValid(1) ) {
					scan_field_with_body();
					return;
				}
				if( Mem->BallKickable() ) {
					Mem->LogAction4(10,"ball pos (%.2f, %.2f)",Mem->BallX(), Mem->BallY());
					if( Mem->BallMoving() ) stopball();
					return;
				}
				if( Mem->FastestTeammateToBall()==2 ) {
					get_ball();
					return;
				}
				face_only_body_to_ball();
				return;
			}
		//end				
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::GetCongestionPriority(Vector passPos) {
	float congestion=0.0f;
	float dist;
	for( unsigned int id=0;id<opponents.size();id++ )  {
		dist=Max(0.1,passPos.dist(Mem->OpponentAbsolutePosition(opponents[id])));			
		if( dist<=15 ) 
			congestion+=1.0f/(dist*dist);
	}
	//Mem->LogAction3(10,"congestion %.2f",congestion);
	return Min(congestion,500.0f)/500.0f;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Unum Pass::GetMostDangerOpponent(Vector passPos, float& cycles) {
	Unum opponent=Unum_Unknown;
	cycles=InfCycles;
	float plCycles;
	for( unsigned int id=0;id<opponents.size();id++ ) {
		plCycles=PredictPlayerCyclesToPosition(opponents[id],Mem->TheirSide,passPos);
		if( plCycles<cycles ) {
			cycles=plCycles;
			opponent=opponents[id];
		}
	}
	return opponent;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::ExplorePass(Unum receiver, Vector from,Vector passPos,float& passSpeed,int& tmCycles, Unum& danger, int& cycles) {
	float maxEndSpeed = maxBallEndSpeedSlow;
	if( GetDiff((passPos-from).dir(),Mem->AngleToGlobal(/*Mem->TeammateAbsolutePosition(receiver)*/
							    Mem->TheirPenaltyArea.IsWithin(Pos.GetTmPos(receiver))?Pos.GetSmartPassPoint(receiver):Pos.GetTmPos(receiver)))<=1.0f ) 
		if( passPos.dist(from)>10 ) 
				maxEndSpeed=maxBallEndSpeedFast;		
		
	passSpeed = GetPassSpeed(receiver,passPos,maxEndSpeed,&tmCycles);
	float passDir = (passPos-Mem->BallAbsolutePosition()).dir();
	Vector ballVel=Polar2Vector(passSpeed,passDir);
	danger=Unum_Unknown;
	cycles=InfCycles;
	PlayerInterceptInfo info;
	for( unsigned int id=0;id<opponents.size();id++ ) {
			oppInfo[opponents[id]-1].vBallVel=ballVel;
			if( Mem->GetPlayerInterceptionPoint(Mem->TheirSide,opponents[id],&info,oppInfo[opponents[id]-1] )==1) {
				if( info.numCyc<cycles ) {
					cycles=info.numCyc;
					danger=opponents[id];
				}
			}
	}
	return PassConfidence(tmCycles,cycles);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::ExploreDirectPasses() {
	Vector   passPos;
	float    conf;
	PassInfo info;
		
	for( unsigned int id=0;id<teammates.size();id++ ) {
			
	  passPos=Mem->TheirPenaltyArea.IsWithin(Pos.GetTmPos(teammates[id]))?Pos.GetSmartPassPoint(teammates[id]):Pos.GetTmPos(teammates[id]);
	  //Mem->TeammateAbsolutePosition(teammates[id]);
		if( Mem->TeammateVelocityValid(teammates[id]) )
				passPos+=Mem->TeammateAbsoluteVelocity(teammates[id]);
		
		if( !IsAnglePassable(Mem->AngleToGlobal(passPos)) ) continue;
		conf = GetPassInformation(Mem->BallAbsolutePosition(),passPos,teammates[id],info);
		if( conf>=passThreshold ) {
			logGr.AddCircle(passPos,0.3,"1A23D3",true);									
			AddPass(info);	
			info.Log(10);				
		}else{
			logGr.AddCircle(passPos,0.3,"000000",true);						
		}
		
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::ExlploreThroughtPasses() {
	Vector   origPos;
	float    conf;
	PassInfo info;
	std::vector<Vector> positions;
		
	for( unsigned int id=0;id<teammates.size();id++ ) {
		positions.clear();	
		origPos=Mem->TheirPenaltyArea.IsWithin(Pos.GetTmPos(teammates[id]))?Pos.GetSmartPassPoint(teammates[id]):Pos.GetTmPos(teammates[id]);
		//Mem->TeammateAbsolutePosition(teammates[id]);
		if( Mem->TeammateVelocityValid(teammates[id]) )
				origPos+=Mem->TeammateAbsoluteVelocity(teammates[id]);
		
		if( !GetPassPositions(Mem->BallAbsolutePosition(),origPos,positions) ) continue;
		for( unsigned int posID=0;posID<positions.size();posID++ ) {
		  if( !Mem->IsPointInBounds(positions[posID]) ||Mem->DistanceTo(positions[posID])<Mem->GetMyKickableArea())//AI: add last cond
			  continue;
			conf = GetPassInformation(Mem->BallAbsolutePosition(),positions[posID],teammates[id],info);
			if( conf>=passThreshold ) {
				logGr.AddCircle(info.GetPassPos(),0.3,"1A23D3",true);									
				AddPass(info);	
				info.Log(10);
			}else{
				logGr.AddCircle(info.GetPassPos(),0.3,"000000",true);						
			}
		}
		
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::PassFromPoint2Point(Vector from, Vector to, Unum& opponent) {
	opponent = Unum_Unknown;
  float passvel,
     		min_conf = 1.0f,
      	tmp_conf;

  for(int opp=1; opp<=11; opp++) {
    if( !Mem->OpponentPositionValid(opp) ) continue;
    if( !InBetween(Mem->OpponentAbsolutePosition(opp), from, to) ) continue;

    if( opp==Mem->TheirGoalieNum )
			tmp_conf = interception.neuro_goalie_intercept(opp, from, to, 0.8f, passvel);
    else
			tmp_conf = interception.neuro_player_intercept(opp, from, to, 0.8f, passvel);

    if( tmp_conf<min_conf ) {
			min_conf = tmp_conf;
			opponent=opp;
    }
  }
	return min_conf;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float Pass::GetPassInformation(Vector from, Vector passPos, Unum teammate, PassInfo& info) {
	int   tmCycles    = InfCycles;
	Unum  danger      = Unum_Unknown;
	int   oppCycles   = InfCycles;		
	float passSpeed=Mem->SP_ball_speed_max;		
		
	float conf = ExplorePass(teammate,from,passPos,passSpeed,tmCycles,danger,oppCycles);
		
	info.SetInfo(teammate,tmCycles,conf,passPos,passSpeed,danger,oppCycles,GetPassPriority(passPos));		
	info.SetCongestion(GetCongestionPriority(passPos));		
	return conf;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Pass::GetPassPositions(Vector from, Vector origPos, std::vector<Vector>& positions) {
	positions.clear();
		
	static float dists[]=  { 4.0f, 8.0f, 12.0f ,16.0f };
	
	static float angles[8][6] = { {  90 , 45 , 0  ,-45 ,-90 ,-135 },
																{  45 , 0  ,-45 ,-90 ,-135,-180 },
																{  0  ,-45 ,-90 ,-135,-180, 135 },
													 			{ -45 ,-90 ,-135,-180, 135, 90  },
													 			{ -90 ,-135,-180, 135, 90 , 45  },
														 		{ -135,-180, 135, 90 , 45 , 0   },
														 		{ -180, 135, 90 , 45 , 0  ,-45  },
														 		{  135, 90 , 45 , 0  ,-45 ,-90  } };
	
	AngleDeg angleFrom  = (from-origPos).dir();	
	AngleDeg angleTo    = (origPos-from).dir();																	
	AngleDeg dribbleAng = (Vector(52.5,signf(origPos.y)*7.5)-origPos).dir();	
	
	Vector   passPos;
	float    distTo;
	
	int ID=0;
																															
	if( angleFrom>=-45 && angleFrom<0 )					ID=0;
	else if( angleFrom>=-90 && angleFrom<-45 )	ID=1;
	else if( angleFrom>=-135 && angleFrom<-90 )	ID=2;
	else if( angleFrom>=-180 && angleFrom<-135 )ID=3;
	else if( angleFrom>=0 && angleFrom<45 )			ID=7;
	else if( angleFrom>=45 && angleFrom<90 )		ID=6;
	else if( angleFrom>=90 && angleFrom<135 )		ID=5;		
	else 																				ID=4;		
			
	Mem->LogAction5(10,"my id %.0f angle to %.2f angle from %.2f",float(ID),angleTo, angleFrom);
																
	for( int distID=0;distID<4;distID++ ) {
			
		passPos=origPos+Polar2Vector(dists[distID],angleFrom);		
		distTo=from.dist(passPos);
		if( IsAnglePassable((passPos-from).dir()) && InBetween(passPos,from,origPos) && Mem->IsPointInBounds(passPos) )
			positions.push_back(passPos);			
			
		passPos=origPos+Polar2Vector(dists[distID],dribbleAng);		
		distTo=from.dist(passPos);
		if( IsAnglePassable((passPos-from).dir()) && distTo>=minPassDistance && distTo<=maxPassDistance && Mem->IsPointInBounds(passPos) )
			positions.push_back(passPos);			
			
		for( int angID=0;angID<6;angID++ ) {
			passPos=origPos+Polar2Vector(dists[distID],angles[ID][angID]);		
			distTo=from.dist(passPos);
			if( IsAnglePassable((passPos-from).dir()) && distTo>=minPassDistance && distTo<=maxPassDistance && Mem->IsPointInBounds(passPos) )
				positions.push_back(passPos);
		}
	}
	
	return positions.size()!=0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::AddThroughPassInfo(const PassInfo& info) {
	if( Mem->TheirPenaltyArea.IsWithin(info.GetPassPos()) || info.GetPassPos().x+4>Mem->MyX() )
		throughPassInfos.push_back(info);
	else
		backThroughPasses.push_back(info);		
	
	
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Pass::AddPass(const PassInfo& info)  { 
	if( Mem->TheirPenaltyArea.IsWithin(info.GetPassPos()) || info.GetPassPos().x+4>Mem->MyX() )
		allPassInfos.push_back(info);
	else
		backPasses.push_back(info);		
};								
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
}
}
