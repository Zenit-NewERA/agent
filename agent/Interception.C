/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Interception.C
 *
 *    AUTHOR     : Sergei Serebyakov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Interception.C,v 2.3 2004/03/05 07:03:44 anton Exp $
 *
 ************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Interception.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
InterceptionSkills::Interception interception;
const int InfCycles = 1000;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
namespace InterceptionSkills {
  //---------------------------------------------------
  //---------------------------------------------------
  void Intercepter::Set(char side, Unum num, Vector pos, Vector vel, AngleDeg ang, bool ang_valid, bool me) {
    this->side=side;
    this->num=num;
    this->pos=pos;
    this->vel=vel;
    this->ang=ang;
    this->ang_valid=ang_valid;
    this->me=me;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  Intercepter::Intercepter() {
    side='?';
    num=Unum_Unknown;
    pos.ins(0,0);
    vel.ins(0,0);
    ang=0.0f;
    ang_valid=false;
    me=false;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  void Intercepter::Log(int level) {
    if( side==Mem->MySide ) {
      Mem->LogAction7(level,"PlPosInfo::teammate %.0f(%.2f, %.2f) vel(%.2f, %.2f)", float(num),pos.x, pos.y, vel.x, vel.y);
      Mem->LogAction5(level, "				  ang valid %.2f ang %.2f me %.0f",float(ang_valid), ang, float(me) );
    }else{
      Mem->LogAction7(level,"PlPosInfo::opponent %.0f(%.2f, %.2f) vel(%.2f, %.2f)", float(num),pos.x, pos.y, vel.x, vel.y);
      Mem->LogAction5(level, "				  ang valid %.2f ang %.2f me %.0f",float(ang_valid), ang, float(me) );
    }
  }
  //---------------------------------------------------
  //---------------------------------------------------
  void Interception::SetPlayerPosInfo() {
    if( time==Mem->CurrentTime )	return;
    time=Mem->CurrentTime;

    Vector pos, vel;
    float  ang;
    int ang_valid;
    bool me;

    for( int pl=1;pl<=11;pl++ ) {
      //opponents
      if( Mem->OpponentPositionValid(pl) ) {
	pos=Mem->OpponentAbsolutePosition(pl);
	if( Mem->OpponentVelocityValid(pl) )
	  vel=Mem->OpponentAbsoluteVelocity(pl);
	else
	  vel.ins(0,0);
	if( Mem->OpponentBodyAngleValid(pl) ) {
	  ang=Mem->OpponentAbsoluteBodyAngle(pl);
	  ang_valid=TRUE;
	}else{
	  ang=0.0f;
	  ang_valid=FALSE;
	}
	me=false;
	opponents[pl-1].Set(Mem->TheirSide, pl, pos, vel, ang, ang_valid, me);
      }
      //teammates
      if( Mem->TeammatePositionValid(pl) ) {
	pos=Mem->TeammateAbsolutePosition(pl);
	if( Mem->TeammateVelocityValid(pl) )
	  vel=Mem->TeammateAbsoluteVelocity(pl);
	else
	  vel.ins(0,0);
	if( Mem->TeammateBodyAngleValid(pl) ) {
	  ang=Mem->TeammateAbsoluteBodyAngle(pl);
	  ang_valid=TRUE;
	}else{
	  ang=0.0f;
	  ang_valid=FALSE;
	}
	me=(pl==Mem->MyNumber);
	teammates[pl-1].Set(Mem->MySide, pl, pos, vel, ang, ang_valid, me);
      }
    }
  }
  //---------------------------------------------------
  //---------------------------------------------------
  Interception::~Interception() {
    int ids, idd;
    for( ids=0; ids<(int)Speeds; ids++ ) {
      for( idd=0; idd<(int)Distances; idd++ ) {
	if( shoot_nets[ids][idd]!=0 )  delete shoot_nets[ids][idd];
	if( pass_nets[ids][idd]!=0 )  delete pass_nets[ids][idd];
      }		
    }
  }
  //---------------------------------------------------
  //---------------------------------------------------
  bool Interception::InitNetworks() {
    //first, init shoot networks
  
    ifstream file;
    file.open("./weights/shootweights/shootweights.conf");
    if( !file ) {
      cout<<"INITIALIZATION ERROR::file shootweights.conf could not be opened"<<endl;
      return false;
    }

    char  string[100];
    float speed,
      distance;
    int 	ids,
      idd;

    for( ids=0; ids<(int)Speeds; ids++ )
      for( idd=0; idd<(int)Distances; idd++ )
	shoot_nets[ids][idd]=pass_nets[ids][idd]=0;
	
    //this is a place to read datas from opened file
    while( true ) {
      file>>string;
      //-------------------------------------
      if( string[0]=='#' ) {
	file.getline(string,sizeof(string));
	continue;
      }
      //-------------------------------------	
      if( string[0]==' ' ) {
	continue;
      }
      //-------------------------------------	
      if( !strcmp(string,"[EndFile]") ) {
	break;		
      }
      //-------------------------------------
      if( !strcmp(string,"[Network]") ) {
	file>>speed;
	file>>distance;

	if( speed==2.5f )	 		ids=(int)Speed25;
	else if( speed==2.2f )ids=(int)Speed22;
	else if( speed==1.8f )ids=(int)Speed18;
	else 									ids=(int)Speed15;

	if( distance==10 )			idd=(int)Dist10;
	else if( distance==15 )	idd=(int)Dist15;
	else										idd=(int)Dist20;

	shoot_nets[ids][idd] = new (nothrow)CProcessor;
	if( shoot_nets[ids][idd]==0 ) {
	  cout<<"SHOOT INIT FAIL-NOT ENOUGHT MEM???"<<endl;
	  return false;
	}				
	shoot_nets[ids][idd]->LoadData(file);
	continue;
      }
      //-------------------------------------
    }
    file.close();

    //now init pass networks
    file.open("./weights/passweights/passweights.conf");
    if( !file ) {
      cout<<"INITIALIZATION ERROR::file passweights.conf could not be opened"<<endl;
      return false;
    }
	
    //this is a place to read datas from opened file
    while( true ) {
      file>>string;
      //-------------------------------------
      if( string[0]=='#' ) {
	file.getline(string,sizeof(string));
	continue;
      }
      //-------------------------------------	
      if( string[0]==' ' ) {
	continue;
      }
      //-------------------------------------	
      if( !strcmp(string,"[EndFile]") ) {
	break;		
      }
      //-------------------------------------
      if( !strcmp(string,"[Network]") ) {
	file>>speed;
	file>>distance;

	if( speed==2.5f )	 		ids=(int)Speed25;
	else if( speed==2.2f )ids=(int)Speed22;
	else if( speed==1.8f )ids=(int)Speed18;
	else 									ids=(int)Speed15;

	if( distance==10 )			idd=(int)Dist10;
	else if( distance==15 )	idd=(int)Dist15;
	else										idd=(int)Dist20;

	pass_nets[ids][idd] = new (nothrow)CProcessor;
	if( pass_nets[ids][idd]==0 ) {
	  cout<<"PASS INIT FAIL(NOT ENOUGHT MEM??"<<endl;
	  return false;
	}				
	pass_nets[ids][idd]->LoadData(file);
	continue;
      }
      //-------------------------------------
    }
    return true;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::ball_control(float t_cycles, float o_cycles) {
    //idea is from paper "an architecture for action selection in robotic soccer"
    //by Peter Stone and David McAllester, but slightly modified

    //float pow=(o_cycles-t_cycles)/(0.99f*min(o_cycles, t_cycles)+0.5f);
    float pow=(o_cycles-t_cycles)/3;
    return 1/(1+exp(-pow));
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::intercept_time( Vector ballpos, Vector ballvel,
				      Vector playerpos, float playervel,
				      float buffer ) {

    //uses Newton method to calculate interception time
    //idea is from  paper, described above
    if( ballpos.dist(playerpos)<Mem->SP_kickable_area ) return 0.0f;

    Vector u,v,k,f;
    float gi,ti,ti1,gdi;
    float q = Mem->SP_ball_decay;
    float vu; //scalar multiplication
    float pow;
    int cycles = 0;
    int maxcycles=8;

    v=ballvel/(1-q);
    f=ballpos+v;
    k=f-playerpos;

    Line line;
    line.LineFromRay(Ray(ballpos, ballvel.dir()));
    ti=line.dist(playerpos);

    pow=::pow(q,ti);
    u=Polar2Vector( 1.0f,(f-v*pow-playerpos).dir() );
    vu=v.mod()*u.mod()*Cos(GetAngleDiff(v.dir(), u.dir()));
    gdi=-vu*log(q)*pow-playervel;

    if( Cos(GetAngleDiff(ballvel.dir(), u.dir()))<0 || (k-v*::pow(q,ti)).mod()-playervel*ti<0) ti=0;
    else if( (k-v*::pow(q,ti)).mod()-playervel*ti>0 && gdi<0 ) {
      //stay
    }
    else ti=200;

    pow=::pow(q,ti);
    gi=(k-v*pow).mod()-playervel*ti;
    do{
      cycles++;
      if( cycles>=maxcycles ) break;
      u=Polar2Vector( 1.0f,(f-v*pow-playerpos).dir() );
      vu=v.mod()*u.mod()*Cos(GetAngleDiff(v.dir(), u.dir()));

      gdi=-vu*log(q)*pow-playervel;

      ti1=ti-gi/gdi;
      ti=ti1;
      pow=::pow(q,ti);
      gi=(k-v*pow).mod()-playervel*ti;

    }while(fabs(gi)>=buffer );
    if( cycles==maxcycles ) return InfCycles;

    return ti1;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::neuro_goalie_intercept(	Unum goalie, Vector kickfrom,
						Vector kickto, float threshold,
						float& kickspeed) {
    Line ballpath;
    Vector opppos = Mem->OpponentAbsolutePosition(goalie);

    ballpath.LineFromTwoPoints(kickfrom, kickto);
    Vector pr_point = ballpath.ProjectPoint(opppos);
	
    double input[2];
    int tmp;
	
    input[0] = kickfrom.dist(pr_point);
    input[1] = opppos.dist(pr_point);

    float dist = kickfrom.dist(kickto);

    int dist_id;

    if( dist>=17.0f ) 		 dist_id = Dist20;
    else if( dist>=13.0f ) dist_id = Dist15;
    else 									 dist_id = Dist10;

    float prob;

    kickspeed=1.5f;
    prob=shoot_nets[Speed15][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=1.8f;
    prob=shoot_nets[Speed18][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=2.2f;
    prob=shoot_nets[Speed22][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=2.5f;
    prob=shoot_nets[Speed25][dist_id]->Process(input, tmp);
    return prob;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::neuro_player_intercept(	Unum player, Vector kickfrom,
						Vector kickto, float threshold,
						float& kickspeed) {
    Line ballpath;
    Vector opppos = Mem->OpponentAbsolutePosition(player);

    ballpath.LineFromTwoPoints(kickfrom, kickto);
    Vector pr_point = ballpath.ProjectPoint(opppos);
	
    double input[2];
    int tmp;
	
    input[0] = kickfrom.dist(pr_point);
    input[1] = opppos.dist(pr_point);

    float dist = kickfrom.dist(kickto);

    int dist_id;

    if( dist>=17.0f ) 			dist_id = Dist20;
    else if( dist>=13.0f )  dist_id = Dist15;
    else 										dist_id = Dist10;

    float prob;

    kickspeed=1.5f;
    prob=pass_nets[Speed15][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=1.8f;
    prob=pass_nets[Speed18][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=2.2f;
    prob=pass_nets[Speed22][dist_id]->Process(input, tmp);
    if( prob>=0.99*threshold ) return prob;

    kickspeed=2.5f;
    prob=pass_nets[Speed25][dist_id]->Process(input, tmp);
    return prob;
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::neuro_goalie_intercept(	Unum goalie,
						Vector kickfrom,
						Vector kickto,
						float kickspeed ) {
    Line ballpath;
    Vector opppos = Mem->OpponentAbsolutePosition(goalie);

    ballpath.LineFromTwoPoints(kickfrom, kickto);
    Vector pr_point = ballpath.ProjectPoint(opppos);

    double input[2];
    int tmp;

    input[0] = kickfrom.dist(pr_point);
    input[1] = opppos.dist(pr_point);

    float dist = kickfrom.dist(kickto);

    int dist_id;

    if( dist>=17.0f ) 		 dist_id = Dist20;
    else if( dist>=13.0f ) dist_id = Dist15;
    else 									 dist_id = Dist10;

    if( kickspeed<=1.6f )
      return shoot_nets[Speed15][dist_id]->Process(input, tmp);

    if( kickspeed<=1.9f )
      return shoot_nets[Speed18][dist_id]->Process(input, tmp);

    if( kickspeed<=2.3f )
      return shoot_nets[Speed22][dist_id]->Process(input, tmp);

    return shoot_nets[Speed25][dist_id]->Process(input, tmp);
  }
  //---------------------------------------------------
  //---------------------------------------------------
  float Interception::neuro_player_intercept( Unum player,
					      Vector kickfrom,
					      Vector kickto,
					      float kickspeed ) {
    Line ballpath;
    Vector opppos = Mem->OpponentAbsolutePosition(player);

    ballpath.LineFromTwoPoints(kickfrom, kickto);
    Vector pr_point = ballpath.ProjectPoint(opppos);

    double input[2];
    int tmp;

    input[0] = kickfrom.dist(pr_point);
    input[1] = opppos.dist(pr_point);

    float dist = kickfrom.dist(kickto);

    int dist_id;

    if( dist>=17.0f ) 			dist_id = Dist20;
    else if( dist>=13.0f )  dist_id = Dist15;
    else 										dist_id = Dist10;

    if( kickspeed<=1.6f )
      return pass_nets[Speed15][dist_id]->Process(input, tmp);

    if( kickspeed<=1.9f )
      return pass_nets[Speed18][dist_id]->Process(input, tmp);

    if( kickspeed<=2.3f )
      return pass_nets[Speed22][dist_id]->Process(input, tmp);

    return pass_nets[Speed25][dist_id]->Process(input, tmp);
  }
  //---------------------------------------------------
  //---------------------------------------------------
}//namespace InterceptionSkills
