/*************************************************************************
 *
 *    DESCRIPTION :
 *
 *    FILE 	 : Handleball.C
 *
 *    AUTHOR     : Sergei Serebryakov, Anton Ivanov
 *
 *    $Revision: 2.24 $
 *
 *    $Id: Handleball.C,v 2.24 2004/06/27 10:48:00 anton Exp $
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

#include "Handleball.h"
#include "Playposition.h"
#include "dribble.h"
#include "scenario.h"
#include <algorithm>
#include "kick.h"
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg ballAngleBuffer=15;
AngleDeg playerAngleBuffer=15;
AngleDeg positionAngleBuffer=10;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetBuffersByDefault() {
  ballAngleBuffer     = 15;
  playerAngleBuffer   = 15;
  positionAngleBuffer = 15;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetMediumBuffers() {
  ballAngleBuffer     = 35;
  playerAngleBuffer   = 35;
  positionAngleBuffer = 15;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetHugeBuffers() {
  ballAngleBuffer     = 80;
  playerAngleBuffer   = 80;
  positionAngleBuffer = 15;
} 
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Actions actions;
VisualControl eye;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Regions::Init() {
	float fieldLength   = Mem->SP_pitch_length/2;
	float fieldWidth    = Mem->SP_pitch_width/2;
	float penaltyLength = Mem->SP_penalty_area_length;
	float penaltyWidth  = Mem->SP_penalty_area_width/2;
		
	regions[0] =  Rectangle(-fieldLength, 0, -fieldWidth, -penaltyWidth);
	regions[1] =  Rectangle( 0, fieldLength-penaltyLength, -fieldWidth,-penaltyWidth);
	regions[2] =  Rectangle( fieldLength-penaltyLength, fieldLength, -fieldWidth,-penaltyWidth);
	regions[3] =  Rectangle(-fieldLength, -(fieldLength-penaltyLength), -penaltyWidth, penaltyWidth);		
	regions[4] =  Rectangle(-(fieldLength-penaltyLength), 0, -penaltyWidth, 0);				
	regions[5] =  Rectangle(0, fieldLength-penaltyLength-5.0f, -penaltyWidth, 0);				
	regions[6] =  Rectangle(fieldLength-penaltyLength-5.0f, fieldLength, -penaltyWidth, penaltyWidth);		
	regions[7] =  Rectangle(-(fieldLength-penaltyLength), 0, 0, penaltyWidth);				
	regions[8] =  Rectangle(0, fieldLength-penaltyLength, 0, penaltyWidth);
	regions[9] =  Rectangle(-fieldLength, 0, penaltyWidth, fieldWidth);
	regions[10] = Rectangle( 0, fieldLength-penaltyLength, penaltyWidth, fieldWidth);
	regions[11] = Rectangle( fieldLength-penaltyLength, fieldLength, penaltyWidth, fieldWidth);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int Regions::GetBallRegionID(Vector ballPos) {
	reversed = false;
	if( ballPos.y==0 ) ballPos.y=-0.0001;
	if( ballPos.y>=0 ) 
		reversed=true;
		
	return GetVectorRegionID(ballPos);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int Regions::GetVectorRegionID(Vector pos) {
	if( pos.y==0 ) pos.y=-0.0001;
	if( reversed ) pos.y=-pos.y;
			
	for( int id=0;id<11;id++ ) 
		if( regions[id].IsWithin(pos) )
			return (id+1);
	
	return -1;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Handleball::Handleball() {
	ballRegionID=5;
	dribble=0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Handleball::Initialize() {
  Shoot::Initialize();
  Pass::Initialize();
	regions.Init();
  return true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Handleball::Handle() {
  if( !Mem->BallKickable() ) return false;

  //  FuncEvarTime time(0,"Handle()");
			
	SetPassThreshold(0.5001);
	if( Mem->TheirPenaltyArea.IsWithin(Mem->MyPos()) ) SetPassThreshold(0.5);
			
  //-----------------			
  if( dribble==0 ) dribble = new Dribble;
  if( dribble==0 ) return false;
  //-----------------
  ballRegionID = regions.GetBallRegionID(Mem->BallAbsolutePosition());
  Mem->LogAction3(10,"Ball region %.0f",float(ballRegionID));
  if( ballRegionID==-1 ) {
    Mem->LogAction2(10,"ball region is -1, that's why complex hadleball's impossible, call clear ball");
    clear_ball();
    return true;
  }	
  //-----------------
  Unum closest=Mem->ClosestOpponent();
  bool teamless = Mem->NumTeamlessPlayers()>0 ? true : false;
  //Tackle add by AI
  float tackle_prob=0.0f;
  if(closest!=Unum_Unknown)
    tackle_prob=Mem->GetOpponentTackleProb(Mem->ClosestOpponentToBall(),Mem->BallAbsolutePosition());

  if( (closest!=Unum_Unknown &&Mem->OpponentPositionValid(closest)==1.0f&& Mem->BallKickableForOpponent(closest)) ||
      (teamless && Mem->ClosestTeamlessPlayerPosition().dist(Mem->BallAbsolutePosition())<=Mem->SP_kickable_area) ||
      (tackle_prob>0.7f&&Mem->OwnPenaltyArea.IsWithin(Mem->MyPos()))) {
    
    if(Mem->TheirPenaltyArea.IsWithin(Mem->MyPos()) ) {
      Line l(Ray(Mem->BallAbsolutePosition(),MyShootAngle()));
      float y=l.get_y(Mem->SP_pitch_length/2);
      if(Mem->OpponentPositionValid(Mem->TheirGoalieNum)&&fabs(y-Mem->OpponentY(Mem->TheirGoalieNum))<=7.0f&&fabs(Mem->MyY())>5.0f){
	Mem->LogAction3(10,"opponent goalie cover goal (%.2f)",(float)fabs(y-Mem->OpponentY(Mem->TheirGoalieNum)));
	AngleDeg ang;
	if( cross_angle(ang)) {
	  Mem->LogAction2(10,"Handleball:making cross then i cover");
	  StartCross();		
	  return true;
	}
      }
      Mem->LogAction2(10,"shoot when i am covered");
      smartkickg(Mem->SP_ball_speed_max*2.0f, MyShootAngle(), SK_Fast);
      return true;
    }
    //first, see, if pass can be made
    PassDecision();
    myCongestion = GetCongestionPriority(Mem->MyPos());
    Mem->LogAction3(10,"My congestion %.9f",myCongestion);
    if( Mem->MyX()>-30 && oneKickPassConf!=0 )	{
      Mem->LogAction2(10,"pass when i am covered");
      pass(*oneKickPassConf);
      return true;
    }
    //else just kick ball out
    Mem->LogAction2(10,"i am covered, clear at one kick");
    clear_ball(true);
    return true;
  }
  //AI: убираем совместные удары 
  closest=Mem->ClosestTeammateToBall(FALSE);
  if(closest!=Unum_Unknown&&Mem->BallKickableForTeammate(closest)&&
     (((Mem->my_offside_opp!=Unum_Unknown||Mem->TeammateX(closest)-Mem->MyX()<=0.1f)&&closest>Mem->MyNumber)
      ||(Mem->my_offside_opp==Unum_Unknown&&Mem->TeammateX(closest)-Mem->MyX()>0.1f))){
    Mem->LogAction4(10,"Handle:: ball is kickable for tm %.0f, so do not kick ball (x=%.2f)",
		    float(closest),Mem->TeammateX(closest));
    return false;
  }else
    Mem->LogAction4(10,"Handle: closest tm to ball is %.0f with dist %.2f",
		    float(closest),closest==Unum_Unknown?100.0f:Mem->TeammateDistanceToBall(closest));
  
  //AI: hack
  Mem->LogAction5(10,"Handle:dist to ball=%.4f; my_kickable_area=%.2f; after_dash_dist=%.4f",
		  Mem->DistanceTo(Mem->BallAbsolutePosition()),Mem->GetMyKickableArea(),
		  Mem->BallPredictedPosition().dist(Mem->MyPredictedPosition(1,Mem->CorrectDashPowerForStamina(Mem->SP_max_power))));
  if((Mem->GetMyKickableArea()-Mem->DistanceTo(Mem->BallAbsolutePosition()))<0.05f&&
     (Mem->GetMyKickableArea()-Mem->BallPredictedPosition().dist(Mem->MyPredictedPosition(1,Mem->CorrectDashPowerForStamina(Mem->SP_max_power))))>0.1f){
    Mem->LogAction2(10,"May be ball not kickable, so make addition dash");
    dash(Mem->CorrectDashPowerForStamina(Mem->SP_max_power));
    return true;
  }
  
  if( Mem->BallVelocityValid()<0.7) {
    Mem->LogAction2(10, "Handleball::ball velocity is not valid, stopping it");
    stopball();
    return true;
  }

  if((Mem->MyX()>42.0f&&MyShootConf()>0.55f)||(Mem->MyX()<=42.0f&&MyShootConf()>0.85f)) {
    shoot();
    return true;
  }
  if(microKicks.CanMicroAvoidGoalie())
    return true;
  //AI:переместил сюда, чтобы при ударе точно не теряли цикл
  PassDecision();
  myCongestion = GetCongestionPriority(Mem->MyPos());
  Mem->LogAction3(10,"My congestion %.9f",myCongestion);
	
  dribble->SetDribblePos(Dribble::SelectDribbleTarget());  
	
  if( mostPossibleShooter!=0 && TeammateShootConf(mostPossibleShooter->GetReceiver())>=0.6f ) {
    Mem->LogAction4(10,"most possible shooter %.0f with shoot conf %.2f, pass him in advance",
		    float(mostPossibleShooter->GetReceiver()),TeammateShootConf(mostPossibleShooter->GetReceiver()));
    pass(*mostPossibleShooter);
    return true;
  }
			
  if( Scenarios.ExecuteScenario() ) {
    Mem->LogAction2(10,"Handleball :: in scenarion");
    return true;
  }					

  switch( ballRegionID ) {
  case 1:
  case 10:
    PlayInArea1();
    break;
  case 2:
  case 11:
    PlayInArea2();
    break;
  case 3:
  case 12:
    PlayInArea3();
    break;
  case 4:
    PlayInArea4();
    break;
  case 5:
  case 8:
    PlayInArea5();
    break;
  case 6:
  case 9:
    PlayInArea6();
    break;
  case 7:
    PlayInArea7();
    break;
  default:
    Mem->LogAction3(10,"UNKNOWN ball region %.0f",float(ballRegionID));
    PlayInArea4();
  }
		
  return true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Handleball::DribbleForwardInOurPenaltyArea() {
	if( Mem->Tired() ) return false;
			
	Vector target  = Mem->MyPos()+Polar2Vector(50,0);
	Line   dribbleLine;
	dribbleLine.LineFromTwoPoints(Mem->MyPos(),target);
	
	for( int pl=1;pl<=11;pl++ ) 
		if( Mem->OpponentPositionValid(pl) ) {
			int cycles = NumOfCyclesThenILastSeePlayer(-pl);
			float dist = dribbleLine.dist(Mem->OpponentAbsolutePosition(pl));
			if( int(dist/Mem->SP_player_speed_max)<=cycles && Mem->OwnPenaltyArea.IsWithin(Mem->OpponentAbsolutePosition(pl)) ) return false;
		}
		
	Vector oldPos  = dribble->SetDribblePos(target);
	float oldPriority = dribble->SetPriority(0.4f);
	if( dribble->GoBaby() ) {
		Mem->LogAction2(10,"dribble forward in area 1 with priority 0.4");
		return true;
	}
	dribble->SetDribblePos(oldPos);
	dribble->SetPriority(oldPriority);
	return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea1() {
  if( passMaxConfidence[Reg2]!=0 && myCongestion>passMaxConfidence[Reg2]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 1 to 2 with max conf and less congestion");
    pass(*passMaxConfidence[Reg2]);
    return;
  }
  if( passMaxConfidence[Reg6]!=0 && myCongestion>passMaxConfidence[Reg6]->GetCongestion() && passMaxConfidence[Reg6]->GetPassPos().x>Mem->MyX()+5 ) {
    Mem->LogAction2(10,"pass from area 1 to 6 with max conf and less congestion");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
  if( passMaxConfidence[Reg1]!=0 && Mem->MyX()<passMaxConfidence[Reg1]->GetPassPos().x && myCongestion>passMaxConfidence[Reg1]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 1 to 1 with max conf and less congestion");
    pass(*passMaxConfidence[Reg1]);
    return;
  }
	dribble->SetPriority(0.3f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.3  in area 1, i am not tired");	
    return;
  }
	if( passMaxConfidence[Reg6]!=0 && myCongestion>passMaxConfidence[Reg6]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 1 to 6 with max conf and less congestion");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
  if( InDangerSituation() ) {
    Mem->LogAction2(10,"danger situation - clear ball in area 1");
    clear_ball();
    return;
  }
  dribble->SetPriority(0.5f);
  if( Mem->Tired()==FALSE &&  dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.5 in area 1, i am not tired");
    return;
  }
  if( passMaxConfidence[Reg2]!=0 ) {
    Mem->LogAction2(10,"pass from area 1 to 2 with max conf");
    pass(*passMaxConfidence[Reg2]);
    return;
  }
  if( passMaxConfidence[Reg6]!=0 ) {
    Mem->LogAction2(10,"pass from area 1 to 6 with max conf");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
  if( passMaxConfidence[Reg1]!=0 && Mem->MyX()<passMaxConfidence[Reg1]->GetPassPos().x ) {
    Mem->LogAction2(10,"pass from area 1 to 1 with max conf");
    pass(*passMaxConfidence[Reg1]);
    return;
  }
  if( passMaxConfidence[Reg9]!=0 ) {
    Mem->LogAction2(10,"pass from area 1 to 9 with max conf");
    pass(*passMaxConfidence[Reg9]);
    return;
  }
  if( passMaxConfidence[Reg5]!=0 ) {
    Mem->LogAction2(10,"pass from area 1 to 5 with max conf");
    pass(*passMaxConfidence[Reg5]);
    return;
  }
  if( passMaxConfidence[Reg8]!=0 ) {
    Mem->LogAction2(10,"pass from area 1 to 8 with max conf");
    pass(*passMaxConfidence[Reg8]);
    return;
  }
	if( Mem->MyX()>-25 ) {
		if( LastHopePass().Valid() ) {
			Mem->LogAction2(10,"do last hope pass in area 1");	
			pass(LastHopePass());
			return;
		}
	}
	if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.5 in area 1");
    return;
  }
  Mem->LogAction2(10,"can't do anything, clear ball in area 1");	
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea2() {
  if( mostPossibleShooter!=0 ) {
    Mem->LogAction2(10,"pass from area 2 to most possible shooter");
    pass(*mostPossibleShooter);
    return;
  }
  if( passMaxPriority[Reg7]!=0 ) {
    Mem->LogAction2(10,"pass from area 2 to 7 with max priority");
    pass(*passMaxPriority[Reg7]);
    return;
  }
  if( passMaxPriority[Reg3]!=0 && myCongestion>passMaxPriority[Reg3]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 2 to 3 with max priority and less congestion");
    pass(*passMaxPriority[Reg3]);
    return;
  }
  if( passMaxPriority[Reg2]!=0 && Mem->MyX()<passMaxPriority[Reg2]->GetPassPos().x && myCongestion>passMaxPriority[Reg2]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 2 to 2 with max priority and less congestion");
    pass(*passMaxPriority[Reg2]);
    return;
  }
  dribble->SetPriority(0.4f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.4  in area 2,i am not tired");	
    return;
  }
  if( InDangerSituation() ) {
    Mem->LogAction2(10,"danger situation - clear ball in area 2");
    clear_ball();
    return;
  }
  if( passMaxPriority[Reg3]!=0 ) {
    Mem->LogAction2(10,"pass from area 2 to 3 with max priority");
    pass(*passMaxPriority[Reg3]);
    return;
  }
  if( passMaxPriority[Reg2]!=0 && Mem->MyX()<passMaxPriority[Reg2]->GetPassPos().x ) {
    Mem->LogAction2(10,"pass from area 2 to 2 with max priority");
    pass(*passMaxPriority[Reg2]);
    return;
  }	
  if( passMaxConfidence[Reg6]!=0 ) {
    Mem->LogAction2(10,"pass from area 2 to 6 with max conf");
    pass(*passMaxConfidence[Reg6]);
    return;
  }	
  if( passMaxConfidence[Reg9]!=0 ) {
    Mem->LogAction2(10,"pass from area 2 to 9 with max conf");
    pass(*passMaxConfidence[Reg9]);
    return;
  }
  dribble->SetPriority(0.7f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.7  in area 2, i am not tired");	
    return;
  }
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( Mem->MyX()>25 ) {
		if( backPassMaxConfidence[Reg2]!=0 ) {
			Mem->LogAction2(10,"back direct pass from 2 to 2 before clear ball");	
    	pass(*backPassMaxConfidence[Reg2]);
			return;
		}
		if( backPassMaxConfidence[Reg6]!=0 ) {
			Mem->LogAction2(10,"back direct pass from 2 to 6 before clear ball");	
    	pass(*backPassMaxConfidence[Reg6]);
			return;
		}
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( LastHopePass().Valid() ) {
		Mem->LogAction2(10,"do last hope pass in area 2");	
		pass(LastHopePass());
		return;
	}
	if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.6  in area 2");	
    return;
  }
  Mem->LogAction2(10,"can't do anything, clear ball in area 2");			
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea3() {
  if( mostPossibleShooter!=0 ) {
    Mem->LogAction2(10,"pass from area 3 to most possible shooter");
    pass(*mostPossibleShooter);
    return;
  }
  if( passMaxConfidence[Reg7]!=0 ) {
    Mem->LogAction2(10,"pass from area 3 to 7 with max confidence");
    pass(*passMaxConfidence[Reg7]);
    return;
  }
  if( passMaxPriority[Reg3]!=0 && fabs(Mem->MyY())>fabs(passMaxPriority[Reg3]->GetPassPos().y) ) {
    Mem->LogAction2(10,"pass from area 3 to 3 with max priority");
    pass(*passMaxPriority[Reg3]);
    return;
  }
  dribble->SetPriority(0.4f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.4  in area 3, i am not tired");	
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg7]!=0 ) {
		Mem->LogAction2(10,"through forward pass from area 3 to 7 with max conf");
    pass(*passThrMaxConfidence[Reg7]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.4  in area 3");	
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg9]!=0 && myCongestion>passThrMaxConfidence[Reg9]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 3 to 9 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg9]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  if( cross_angle(crossAng) ) {
    if(microKicks.CanMicroCrossBall(GetTarget()))
      return;
    Mem->LogAction2(10,"crossing ball in area 3");
    StartCross();
    return;
  }
  if( passMaxConfidence[Reg9]!=0 ) {
    Mem->LogAction2(10,"pass from area 3 to 9 with max conf");
    pass(*passMaxConfidence[Reg9]);
    return;
  }
  if( passMaxPriority[Reg6]!=0 ) {
    Mem->LogAction2(10,"pass from area 3 to 6 with max priority");
    pass(*passMaxPriority[Reg6]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg6]!=0  ) {
		Mem->LogAction2(10,"through forward pass from area 3 to 6 with max conf");
    pass(*passThrMaxConfidence[Reg6]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( Mem->MyX()>25 ) {
		if( backPassMaxConfidence[Reg3]!=0 ) {
			Mem->LogAction2(10,"back direct pass from 3 to 3 before clear ball");	
    	pass(*backPassMaxConfidence[Reg3]);
			return;
		}
		if( backPassMaxConfidence[Reg2]!=0 ) {
			Mem->LogAction2(10,"back direct pass from 3 to 2 before clear ball");	
    	pass(*backPassMaxConfidence[Reg2]);
			return;
		}
		if( backPassMaxConfidence[Reg6]!=0 ) {
			Mem->LogAction2(10,"back direct pass from 3 to 6 before clear ball");	
    	pass(*backPassMaxConfidence[Reg6]);
			return;
		}
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( LastHopePass().Valid() ) {
		Mem->LogAction2(10,"do last hope pass in area 3");	
		pass(LastHopePass());
		return;
	}
		
  Mem->LogAction2(10,"can't do anything, clear ball in area 3");				
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea4() {
  if( passMaxConfidence[Reg1]!=0 ) {
    Mem->LogAction2(10,"pass from area 4 to 1 with max conf");
    pass(*passMaxConfidence[Reg1]);
    return;
  }
  if( passMaxConfidence[Reg5]!=0 ) {
    Mem->LogAction2(10,"pass from area 4 to 5 with max conf");
    pass(*passMaxConfidence[Reg5]);
    return;
  }
  if( passMaxConfidence[Reg8]!=0 ) {
    Mem->LogAction2(10,"pass from area 4 to 8 with max conf");
    pass(*passMaxConfidence[Reg8]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg1]!=0 ) {
		Mem->LogAction2(10,"through forward pass from area 4 to 1 with max conf");
    pass(*passThrMaxConfidence[Reg1]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	//----------------------------------------------------------------------------------------------
	
	if( InDangerSituation() && Mem->Tired()==TRUE ) {
    Mem->LogAction2(10,"danger situation and tired- clear ball in area 4");
    clear_ball();
    return;
  }
	//----------------------------------------------------------------------------------------------
	if( Mem->Tired()==FALSE ) {
		
		if( DribbleForwardInOurPenaltyArea() ) return;
			
		Vector dribbleTarget;
		//try dribble to wings to two different directions
		//first
		dribbleTarget = Vector(Mem->MyX(),signf(Mem->MyY())*Mem->SP_pitch_width/2);
		dribble->SetDribblePos(dribbleTarget);
		dribble->SetPriority(0.7f);
		if( dribble->GoBaby(only_control_dribble) ) {
	    Mem->LogAction2(10,"dribble with priority 0.7  in area 4 to wing(perp)");	
  	  return;
  	}
		//second
		dribbleTarget = Vector(2-Mem->SP_pitch_length/2,signf(Mem->MyY())*Mem->SP_pitch_width/2);
		dribble->SetDribblePos(dribbleTarget);  
		dribble->SetPriority(0.8f);
		if( dribble->GoBaby(only_control_dribble) ) {
	    Mem->LogAction2(10,"dribble with priority 0.7  in area 4 to wing(corner)");	
    	return;
  	}
		//else restore dribble target
		dribble->SetDribblePos(Dribble::SelectDribbleTarget());  
	}
	//----------------------------------------------------------------------------------------------
	if( InDangerSituation() ) {
    Mem->LogAction2(10,"danger situation - clear ball in area 4");
    clear_ball();
    return;
  }
  dribble->SetPriority(0.5f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.5  in area 4");	
    return;
  }
  if( passMaxConfidence[Reg10]!=0 ) {
    Mem->LogAction2(10,"pass from area 4 to 10 with max conf");
    pass(*passMaxConfidence[Reg10]);
    return;
  }
  dribble->SetPriority(0.7f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.7 in area 4");
    return;
  }
  Mem->LogAction2(10,"can't do anything, clear ball in area 4");			
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea5() {
  if( passMaxPriority[Reg2]!=0 && myCongestion>passMaxPriority[Reg2]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 5 to 2 with max priority and less congestion");
    pass(*passMaxPriority[Reg2]);
    return;
  }
  if( passMaxConfidence[Reg6]!=0 && myCongestion>passMaxConfidence[Reg6]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 5 to 6 with max conf and less congestion");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg2]!=0 && myCongestion>passThrMaxConfidence[Reg2]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 5 to 2 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg2]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( passMaxConfidence[Reg5]!=0 && Mem->MyX()+10<passMaxConfidence[Reg5]->GetPassPos().x  && myCongestion>passMaxConfidence[Reg5]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 5 to 5 with max conf and less congestion");
    pass(*passMaxConfidence[Reg5]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg1]!=0 && myCongestion>passThrMaxConfidence[Reg1]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 5 to 1 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg1]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  dribble->SetPriority(0.4f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.4  in area 5");	
    return;
  }
  if( passMaxConfidence[Reg5]!=0 && Mem->MyX()<passMaxConfidence[Reg5]->GetPassPos().x  && myCongestion>passMaxConfidence[Reg5]->GetCongestion() ) {
    Mem->LogAction2(10,"pass from area 5 to 5 with max conf and less congestion");
    pass(*passMaxConfidence[Reg5]);
    return;
  }
  if( InDangerSituation() && Mem->Tired()==TRUE ) {
    Mem->LogAction2(10,"danger situation and tired - clear ball in area 5");
    clear_ball();
    return;
  }
  if( passMaxPriority[Reg2]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 2 with max priority");
    pass(*passMaxPriority[Reg2]);
    return;
  }
  if( passMaxConfidence[Reg6]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 6 with max conf");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
  if( passMaxConfidence[Reg5]!=0 && Mem->MyX()<passMaxConfidence[Reg5]->GetPassPos().x ) {
    Mem->LogAction2(10,"pass from area 5 to 5 with max conf");
    pass(*passMaxConfidence[Reg5]);
    return;
  }
  if( passMaxConfidence[Reg9]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 9 with max conf");
    pass(*passMaxConfidence[Reg9]);
    return;
  }
  if( passMaxConfidence[Reg11]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 11 with max conf");
    pass(*passMaxConfidence[Reg11]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg6]!=0 && myCongestion>passThrMaxConfidence[Reg6]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 5 to 6 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg6]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg9]!=0 && myCongestion>passThrMaxConfidence[Reg9]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 5 to 9 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg9]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  dribble->SetPriority(0.7f);
  if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.6 in area 5");
    return;
  }
  if( passMaxPriority[Reg1]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 1 with max priority");
    pass(*passMaxPriority[Reg1]);
    return;
  }
  if( passMaxConfidence[Reg10]!=0 ) {
    Mem->LogAction2(10,"pass from area 5 to 10 with max conf");
    pass(*passMaxConfidence[Reg10]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg8]!=0 && myCongestion>passThrMaxConfidence[Reg8]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 5 to 8 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg8]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( Mem->MyX()>-25 ) {
		if( LastHopePass().Valid() ) {
			Mem->LogAction2(10,"do last hope pass");	
			pass(LastHopePass());
			return;
		}
	}
	
  Mem->LogAction2(10,"can't do anything, clear ball in area 5");			
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea6() {
  if( mostPossibleShooter!=0 ) {
    Mem->LogAction2(10,"pass from area 6 to most possible shooter");
    pass(*mostPossibleShooter);
    return;
  }		
  if( passMaxPriority[Reg7]!=0 &&passMaxPriority[Reg7]->GetPassPos().x-Mem->BallX()>7.0f) {
    Mem->LogAction2(10,"pass from area 6 to 7 with max priority");
    pass(*passMaxPriority[Reg7]);
    return;
  }
	
	if( fabs(Mem->my_offside_line-Mem->MyX())<7 )  //close to offside line, try this pass
		if( passThrMaxConfidence[Reg2]!=0 && myCongestion>passThrMaxConfidence[Reg2]->GetCongestion() ) 
			if( Pos.GetTmPos(passThrMaxConfidence[Reg2]->GetReceiver()).x<passThrMaxConfidence[Reg2]->GetPassPos().x ) {
				Mem->LogAction2(10,"through forward pass from area 6 to 2 when close to offside line with max conf and less congestion");
    		pass(*passThrMaxConfidence[Reg2]);
				return;
			}
			
	if( fabs(Mem->my_offside_line-Mem->MyX())<7 )  //close to offside line, try also this pass
		if( passThrMaxConfidence[Reg6]!=0 && myCongestion>passThrMaxConfidence[Reg6]->GetCongestion() ) 
			if( Pos.GetTmPos(passThrMaxConfidence[Reg6]->GetReceiver()).x<passThrMaxConfidence[Reg6]->GetPassPos().x ) {
				Mem->LogAction2(10,"through forward pass from area 6 to 6 when close to offside line with max conf and less congestion");
    		pass(*passThrMaxConfidence[Reg6]);
				return;
			}	
	
	
  dribble->SetPriority(0.6f);
  if( Mem->Tired()==FALSE && dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.6  in area 6");	
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg7]!=0 && myCongestion>passThrMaxConfidence[Reg7]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 6 to 7 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg7]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  if( Mem->MyX()>Mem->SP_pitch_length/2-Mem->SP_penalty_area_length-3 && cross_angle(crossAng) ) {
			
		//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
		if( passThrMaxConfidence[Reg7]!=0  ) {
			Mem->LogAction2(10,"through forward pass from area 6 to 7 with max conf before cross");
    	pass(*passThrMaxConfidence[Reg7]);
			return;
		}
		//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
		if(microKicks.CanMicroCrossBall(GetTarget()))
		  return;		
    Mem->LogAction2(10,"crossing ball in area 6");
    StartCross();
    return;
  }
  if( passMaxPriority[Reg3]!=0 ) {
    Mem->LogAction2(10,"pass from area 6 to 3 with max priority");
    pass(*passMaxPriority[Reg3]);
    return;
  }

	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg3]!=0 && myCongestion>passThrMaxConfidence[Reg3]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 6 to 3 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg3]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg2]!=0 && myCongestion>passThrMaxConfidence[Reg2]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 6 to 2 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg2]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
	if( passMaxConfidence[Reg2]!=0 && Mem->MyX()<passMaxConfidence[Reg2]->GetPassPos().x ) {
    Mem->LogAction2(10,"pass from area 6 to 2 with max conf");
    pass(*passMaxConfidence[Reg2]);
    return;
  }
	
  if( passMaxConfidence[Reg6]!=0 && Mem->MyX()<passMaxConfidence[Reg6]->GetPassPos().x ) {
    Mem->LogAction2(10,"pass from area 6 to 6 with max conf");
    pass(*passMaxConfidence[Reg6]);
    return;
  }
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg6]!=0 && myCongestion>passThrMaxConfidence[Reg6]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 6 to 6 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg6]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
  if( passMaxPriority[Reg9]!=0 ) {
    Mem->LogAction2(10,"pass from area 6 to 9 with max priority");
    pass(*passMaxPriority[Reg9]);
    return;
  }
	
	//NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
	if( passThrMaxConfidence[Reg9]!=0 && myCongestion>passThrMaxConfidence[Reg9]->GetCongestion() ) {
		Mem->LogAction2(10,"through forward pass from area 6 to 9 with max conf and less congestion");
    pass(*passThrMaxConfidence[Reg9]);
		return;
	}
	//END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  if( passMaxPriority[Reg2]!=0 ) {
    Mem->LogAction2(10,"pass from area 6 to 2 with max priority");
    pass(*passMaxPriority[Reg2]);
    return;
  }
  dribble->SetPriority(0.7f);
  if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.7  in area 6");	
    return;
  }
  if( Mem->Tired()==TRUE && InDangerSituation() ) {
    Mem->LogAction2(10,"danger situation and tired - clear ball in area 6");
    clear_ball();
    return;
  }
  if( passMaxConfidence[Reg11]!=0 ) {
    Mem->LogAction2(10,"pass from area 6 to 11 with max conf");
    pass(*passMaxConfidence[Reg11]);
    return;
  }
  dribble->SetPriority(0.7f);
  if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"dribble with priority 0.7 in area 6");
    return;
  }
	
	if( LastHopePass().Valid() ) {
		Mem->LogAction2(10,"do last hope pass");	
		pass(LastHopePass());
		return;
	}
		
  if( MyShootConf()>=0.5f ) {
    Mem->LogAction2(10,"in area 6 shoot with conf>=0.5");			
    shoot();
    return;
  }

	Mem->LogAction2(10,"can't do anything, clear ball in area 6");			
  clear_ball();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PlayInArea7() {
  //see, if shoot can be done
  if( MyShootConf()>0.8f ) {
    Mem->LogAction2(10,"play_at_their_penalty_area:shooting with great conf");
    shoot();
    return;
  }
  
  dribble->SetPriority(0.7f);
  if(fabs(Mem->MyY())<4.0f){
    if( dribble->GoBaby() ) {
      Mem->LogAction2(10,"play_at_their_penalty_area:dribble with priority 0.7");
      return;
    }
  }
    
  if( mostPossibleShooter!=0&&TeammateShootConf(mostPossibleShooter->GetReceiver())>=0.7f ) {
    Mem->LogAction3(10,"play_at_their_penalty_area: pass to tm %d with great shoot conf",mostPossibleShooter->GetReceiver());
    pass(*mostPossibleShooter);
    return;
  }
  if(fabs(Mem->MyY())>=4.0f){
    if( dribble->GoBaby() ) {
      Mem->LogAction2(10,"play_at_their_penalty_area:dribble to target with prior 0.7");
      return;
    }
  }

  if( MyShootConf()>=0.5f ) {
    Mem->LogAction2(10,"play_at_their_penalty_area:shooting with small conf");
    shoot();
    return;
  }

  
  if(ThroughPassAtTheirPenaltyArea())
    return;

  if( passMaxPriority[Reg7]!=0 ) {
    Mem->LogAction2(10,"pass from area 7 to 7 with max priority");
    pass(*passMaxPriority[Reg7]);
    return;
  }

  if(fabs(Mem->MyY())<5.0f&&Mem->MyX()>Mem->SP_pitch_length/2.0f-14.0f&&microKicks.CanMicroHardestShoot())//AI:must be tested
    return;
  //NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
  if( passThrMaxPriority[Reg7]!=0 ) {
    Mem->LogAction2(10,"through forward pass from area 7 to 7");
    pass(*passThrMaxPriority[Reg7]);
    return;
  }
  //END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
	
  //NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
  if( backPassThrMaxPriority[Reg7]!=0 ) {
    Mem->LogAction2(10,"through back pass from area 7 to 7");
    pass(*backPassThrMaxPriority[Reg7]);
    return;
  }
  //END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW  END NEW
		
  //see about cross opportunity
  if( cross_angle(crossAng)) {
    if(microKicks.CanMicroCrossBall(GetTarget()))
      return;
    Mem->LogAction2(10,"cros in area 7");
    StartCross();		
    return;
  }
  if( dribble->GoBaby() ) {
    Mem->LogAction2(10,"play_at_their_penalty_area:dribble with no control");
    return;
  }
		
  Mem->LogAction2(20,"Nothing to do: shoot at end to corner");
  float s1,s2;
  Unum o1,o2;
  float c1=shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5,-6.9),s1,o1,Mem->MyNumber);
  float c2=shoot_to_point(Mem->BallAbsolutePosition(),Vector(52.5,6.9),s2,o2,Mem->MyNumber);
  Mem->LogAction4(10,"play_at_their_penalty_area:: in central zone have such conf of kicks: c1=%.2f; c2= %.2f",c1,c2);
  if(c2>c1){
    actions.smartkick(Mem->SP_ball_speed_max*2,Vector(52.5,6.85),SK_Safe);
  }else{
    actions.smartkick(Mem->SP_ball_speed_max*2,Vector(52.5,-6.85),SK_Safe);
  }
}
//////////////////////////////////////////////////////////////
bool Handleball::ShootDecision() {
  return MyShootConf()>=ShootThreshold();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Handleball::PassDecision() {
  for( int id=0;id<12;id++ ) {
    passMaxConfidence[id]=0;
    passMinCongestion[id]=0;
    passMaxPriority[id]=0;
			
		passThrMaxConfidence[id]=0;
    passThrMinCongestion[id]=0;
    passThrMaxPriority[id]=0;

		backPassMaxConfidence[id]=0;
	  backPassMinCongestion[id]=0;
	  backPassMaxPriority[id]=0;
			
		backPassThrMaxConfidence[id]=0;
	  backPassThrMinCongestion[id]=0;
	  backPassThrMaxPriority[id]=0;
  }
	
  mostPossibleShooter=0;
  oneKickPassConf=0;
  oneKickPassCong=0;
	
  FindPasses();
	
  PassInfo* info=0;
  int       regID;
	unsigned int id;
	
	//direct passes
  for( id=0;id<Passes().size();id++ ) {
    info = &Passes()[id];
    //most possible shooter
    if( TeammateShootConf(info->GetReceiver())>=0.5f )
      if( mostPossibleShooter==0 || 
	  ( mostPossibleShooter!=0 && TeammateShootConf(info->GetReceiver())>TeammateShootConf(mostPossibleShooter->GetReceiver()) ) )
	mostPossibleShooter=info;		
    //pass at one kick
    if( maxspeed_at1kick(GetKickAngle(info->GetPassPos()))>=info->GetSpeed() ) {
				
      if( oneKickPassConf==0 || ( oneKickPassConf!=0 && oneKickPassConf->GetConfidence()<info->GetConfidence() ) )
	oneKickPassConf=info;
			
      if( oneKickPassCong==0 || ( oneKickPassCong!=0 && oneKickPassCong->GetCongestion()>info->GetCongestion() ) )
	oneKickPassCong=info;
			
    }
				
    regID = regions.GetVectorRegionID(info->GetPassPos())-1;
    if( regID==-2 ) continue;//AI: return -1 BUT U THEN SUB -1 !!!!!!!!!!!!
				
    if( passMaxConfidence[regID]==0 || 
	( passMaxConfidence[regID]!=0 && passMaxConfidence[regID]->GetConfidence()<info->GetConfidence() ) )
      passMaxConfidence[regID]=info;		
    if( passMinCongestion[regID]==0 || 
	( passMinCongestion[regID]!=0 && passMinCongestion[regID]->GetCongestion()>info->GetCongestion() ) )
      passMinCongestion[regID]=info;		
		
    if( passMaxPriority[regID]==0 || 
	( passMaxPriority[regID]!=0 && passMaxPriority[regID]->GetPriority()<info->GetPriority() ) )
      passMaxPriority[regID]=info;		
  }
	
	
	
	//forward through passes
  for( id=0;id<ThroughPasses().size();id++ ) {
		info = &ThroughPasses()[id];
		regID = regions.GetVectorRegionID(info->GetPassPos())-1;
    if( regID==-2 ) continue;
			
		if( passThrMaxConfidence[regID]==0 || 
				( passThrMaxConfidence[regID]!=0 && passThrMaxConfidence[regID]->GetConfidence()<info->GetConfidence() ) )
      			passThrMaxConfidence[regID]=info;		
		
    if( passThrMinCongestion[regID]==0 || 
				( passThrMinCongestion[regID]!=0 && passThrMinCongestion[regID]->GetCongestion()>info->GetCongestion() ) )
      			passThrMinCongestion[regID]=info;		
		
    if( passThrMaxPriority[regID]==0 || 
				( passThrMaxPriority[regID]!=0 && passThrMaxPriority[regID]->GetPriority()<info->GetPriority() ) )
      			passThrMaxPriority[regID]=info;		
	}
	
	//back direct passes
  for( id=0;id<BackPasses().size();id++ ) {
		info = &BackPasses()[id];
		regID = regions.GetVectorRegionID(info->GetPassPos())-1;
    if( regID==-2 ) continue;
			
		if( backPassMaxConfidence[regID]==0 || 
				( backPassMaxConfidence[regID]!=0 && backPassMaxConfidence[regID]->GetConfidence()<info->GetConfidence() ) )
      			backPassMaxConfidence[regID]=info;		
		
    if( backPassMinCongestion[regID]==0 || 
				( backPassMinCongestion[regID]!=0 && backPassMinCongestion[regID]->GetCongestion()>info->GetCongestion() ) )
      			backPassMinCongestion[regID]=info;		
		
    if( backPassMaxPriority[regID]==0 || 
				( backPassMaxPriority[regID]!=0 && backPassMaxPriority[regID]->GetPriority()<info->GetPriority() ) )
      			backPassMaxPriority[regID]=info;		
	}
	
	//back through passes
  for( id=0;id<BackThroughPasses().size();id++ ) {
		info = &BackThroughPasses()[id];
		regID = regions.GetVectorRegionID(info->GetPassPos())-1;
    if( regID==-2 ) continue;
			
		if( backPassThrMaxConfidence[regID]==0 || 
				( backPassThrMaxConfidence[regID]!=0 && backPassThrMaxConfidence[regID]->GetConfidence()<info->GetConfidence() ) )
      			backPassThrMaxConfidence[regID]=info;		
		
    if( backPassThrMinCongestion[regID]==0 || 
				( backPassThrMinCongestion[regID]!=0 && backPassThrMinCongestion[regID]->GetCongestion()>info->GetCongestion() ) )
      			backPassThrMinCongestion[regID]=info;		
		
    if( backPassThrMaxPriority[regID]==0 || 
				( backPassThrMaxPriority[regID]!=0 && backPassThrMaxPriority[regID]->GetPriority()<info->GetPriority() ) )
      			backPassThrMaxPriority[regID]=info;		
	}
	
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*
  void Handleball::sweeper()
  {
  static Dribble dribble;

  if( MostPossibleShooter().Valid() ) {
  Mem->LogAction2(10,"pass to most possible shooter");
  pass(MostPossibleShooter());
  return;
  }

  //  if(throughPass.KickInThroughPass())
  //  return;
	
  if( WingReceiverConf().Valid() ) {
  Mem->LogAction2(10,"pass to wingconf");
  pass(WingReceiverConf());
  return;
  }

  if( WingReceiverCong().Valid() ) {
  Mem->LogAction2(10,"pass to wingcong");
  pass(WingReceiverCong());
  return;
  }
	
  if( ForwardConf().Valid() ) {
  Mem->LogAction2(10,"pass to forwardconf");
  pass(ForwardConf());
  return;
  }

  if( ForwardCong().Valid() ) {
  Mem->LogAction2(10,"pass to forwardcong");
  pass(ForwardCong());
  return;
  }
	
  float mycongestion=Mem->Congestion(Mem->MyPos(),FALSE);
  float playercongestion=Mem->Congestion(info.GetPassPos(),TRUE);
		
  if( playercongestion<mycongestion ) {
  Mem->LogAction2(10,"pass(congestion is matter)");
  pass(info);
  return;
  }

	
  dribble->SetPriority(0.3f);
  dribble->SetDribblePos(Dribble::SelectDribbleTarget());
  if( dribble->GoBaby() ) {
  Mem->LogAction2(10,"dribble to goal");	
  return;
  }

	
	
  if( Mem->MyX()>Mem->SP_pitch_length/2-(Mem->SP_penalty_area_length+3) ) {
  AngleDeg ang;
  if( cross_angle(ang) && MyShootConf()<0.59 ) {
  Mem->LogAction2(10,"crossing ball");
  StartCross();
  return;
  }
  }

  if( Mem->TheirPenaltyArea.IsWithin(Mem->MyPos()) ) {
  Mem->LogAction2(10,"in penalty area - shoot");
  shoot();
  return;
  }


  if( InDangerSituation() ) {
  Mem->LogAction2(10,"danger situation - clear ball");
  clear_ball();
  return;
  }


  dribble->SetPriority(0.5f);
  dribble->SetDribblePos(Dribble::SelectDribbleTarget());
  if( dribble->GoBaby() ) {
  Mem->LogAction2(10,"dribble to goal's center");
  return;
  }

  if( Mem->DistanceTo(Mem->MarkerPosition(Mem->RM_My_Goal))>15 ) {
  int threshold=4;
  switch( Pos.GetMyType() ) {
  case PT_Sweeper:
  case PT_Defender:
  threshold=4;
  break;
  case PT_Midfielder:
  threshold=2;
  break;
  case PT_Forward:
  threshold=0;
  case PT_None:
  case PT_Goaltender:
  case PT_All:
  my_error("should not be here");
  break;
  }
		
  if( front_known_players<threshold ) {
  dribble->SetPriority(0.7f);
  if( dribble->GoBaby() ) {
  Mem->LogAction2(10,"dribble when know less 3 teammates");
  return;
  }		
  if( fabs(Mem->MyBodyAng())<50 &&
  ( last_hb_action!=Mem->CurrentTime-1 ||
  ( last_hb_action==Mem->CurrentTime-1 &&
  Mem->CurrentTime.t-start_hb_action.t<5)) )
  if( dribble->hold_ball(1) ) {
  if( last_hb_action!=Mem->CurrentTime-1 )					
  start_hb_action=Mem->CurrentTime;
  last_hb_action=Mem->CurrentTime;						
  Mem->LogAction2(10,"hold_ball when know less 3 teammates");	
  return;
  }
  }//front_known_players<=threshold
  }

  Mem->LogAction2(10,"clear at end");
  clear_ball();
  }
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  void Handleball::defender() {
  sweeper();
  }
  //-------------------------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------------------------
  void Handleball::forward()
  {
  static Dribble dribble;	

  if(Scenarios.ExecuteScenario())//OK: try to do it baby
  return;
  
  if( Mem->MyX()<Mem->SP_pitch_length/2-(Mem->SP_penalty_area_length+3) ) {
  dribble->SetPriority(0.5);
  dribble->SetDribblePos(Dribble::SelectDribbleTarget());
	  
  if( dribble->GoBaby() ) {
  Mem->LogAction2(10,"dribble");
  return;		
  }
  }

  //  if(throughPass.KickInThroughPass())
  //  return;
	
	
  if( MostPossibleShooter().Valid() && TeammateShootConf(MostPossibleShooter().GetReceiver())>MyShootConf() ) {
  Mem->LogAction2(10,"pass to shooter");
  pass(MostPossibleShooter());
  return;
  }
	
  if( WingReceiverConf().Valid() ) {
  Mem->LogAction2(10,"pass to wingconf");
  pass(WingReceiverConf());
  return;
  }

  if( WingReceiverCong().Valid() ) {
  Mem->LogAction2(10,"pass to wingcong");
  pass(WingReceiverCong());
  return;
  }

	
  if( info.GetReceiver()!=Unum_Unknown&&Mem->TeammatePositionValid(info.GetReceiver()) ) {
  Mem->LogAction2(10,"pass to info");
  pass(info);
  return;
  }
	
  if( Mem->MyX()>Mem->SP_pitch_length/2-(Mem->SP_penalty_area_length+3) ) {
  AngleDeg ang;
  if( cross_angle(ang) && MyShootConf()<0.59 ) {
  Mem->LogAction2(10,"crossing ball");
  StartCross();
  return;
  }
  }


  if( Mem->TheirPenaltyArea.IsWithin(Mem->MyPos()) ) {
  Mem->LogAction2(10,"in penalty area - shoot");
  shoot();
  return;
  }

  if( InDangerSituation() ) {
  Mem->LogAction2(10,"danger situation - clear ball");
  clear_ball();
  return;
  }

  dribble->SetPriority(0.7f);
  dribble->SetDribblePos(Dribble::SelectDribbleTarget());
  if( dribble->GoBaby() ) {
  Mem->LogAction2(10,"dribble to goal's center");
  return;
  }
  

  Mem->LogAction2(10,"clear at end");
  clear_ball();
  }*/
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool Actions::Initialize() {
  return Handleball::Initialize();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Request::Log(int logLevel, bool canSee) {
  if( canSee )
    Mem->LogAction4(logLevel,"angle %.3f  priority %.7f, can see",angle,priority);
  else
    Mem->LogAction4(logLevel,"angle %.3f  priority %.7f, can't see",angle,priority);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool MPObject::operator==(const MPObject& object) const {
  if( type==OT_Ball && object.type==OT_Ball ) return true;
  if( type==OT_Teammate && object.type==OT_Teammate && unum==object.unum ) return true;
  if( type==OT_Opponent && object.type==OT_Opponent && unum==object.unum ) return true;
  if( type==OT_Position && object.type==OT_Position && position==object.position ) return true;
  return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MPObject::operator=(const MPObject& object) {
  type=object.type;
  unum=object.unum;
  position=object.position;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Position::operator=(const Position& p) {
  position=p.position;
  priority=p.priority;
  vWidth=p.vWidth;
  rType=p.rType;
  lastPosValid=p.lastPosValid;
  validWrtId=p.validWrtId;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VisualControl::VisualControl() {
  requests.clear();
  positions.clear();
  for( int id=0;id<11;id++ ) {
    teammates[id].valid = false;
    teammates[id].lastPosKnown = false;
    teammates[id].lastPosValid=0.0f;
    teammates[id].buffAngle=0.0f;
    opponents[id].valid = false;
    opponents[id].lastPosKnown = false;
    opponents[id].lastPosValid=0.0f;
    opponents[id].buffAngle=0.0f;
  }
  ball.valid = false;
  ball.lastPosKnown = true;
  ball.lastPos.ins(0,0);
  ball.lastPosValid=0.0f;
  ball.buffAngle=0.0f;

  vWidth = VW_Unknown;
  viewAngle = 0;
  lastObservationTime = -1;
  lastSearchBallTime = -1;
  bodyTurnAngle = 0;
  bodyNewAngle = 0;
  considerBodyTurn = false;
  searchBallDirection=1;
  myPosition.ins(0,0);
  positionConfDecay=0.99f;
  angleConfDecay=0.93f;
  positionThreshold=0.5f;
  distThreshold = 1;
  positionsConfUpdateTime=-1;
  anglesConfUpdateTime=-1;
  for( int id=0;id<numConfAngles;id++ )
    anglesConf[id]=0.0f;
  mustRequestsExist=false;
  otherRequestsExist=false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VisualControl::~VisualControl() {
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::AngleConf(AngleDeg angle) {
  NormalizeAngleDeg(&angle);
  if( angle<0 ) angle+=360;
  float k=angle/360;
  if( k==1 ) k=0;
  return anglesConf[int(k*numConfAngles)];
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::UpdateConfs(Time time, bool newSight) {
    
  //  Mem->LogAction2(10,"in updateConfs");
  ListOfValuedPositions::iterator iter;
  AngleDeg myViewAngle = GetViewAngle( Mem->ViewWidth );
  AngleDeg neckAng     = Mem->MyNeckGlobalAng();
  AngleDeg targetAng;
  int id;
  if( myViewAngle==0 ) return;
  if( anglesConfUpdateTime==-1 ) anglesConfUpdateTime=time;
  float posDecay=positionConfDecay;
  float angDecay=angleConfDecay;

  if( positionsConfUpdateTime!=-1 )
    for( id=0;id<time.t-positionsConfUpdateTime.t-1;id++ ) posDecay*=positionConfDecay;
  for( id=0;id<time.t-anglesConfUpdateTime.t-1;id++ ) angDecay*=angleConfDecay;

  for( iter=valuedPositions.begin();iter!=valuedPositions.end();iter++ ) {
    iter->conf*=posDecay;
    if( newSight ) {
      targetAng = (iter->position-Mem->MyPos()).dir();
      if( GetDiff(neckAng,targetAng)<myViewAngle-1.5f  )
	iter->conf=1.0f;
    }
  }
  float angStep = (float)360/numConfAngles;
  for( id=0;id<numConfAngles;id++ ) {
    anglesConf[id]*=angleConfDecay;
    if( newSight&&GetDiff(neckAng,angStep*id)<=myViewAngle-1.5f ) anglesConf[id]=1.0f;
    if( anglesConf[id]<0.5f ) anglesConf[id]=0.0f;
  }

  for( ListOfIdInfos::iterator iter=idInfos.begin();iter!=idInfos.end();iter++ ) {
    iter->counter++;
    if( iter->type==OT_Ball ) iter->conf=Mem->BallPositionValid();
    else if( iter->type==OT_Teammate ) iter->conf=Mem->TeammatePositionValid(iter->unum);
    else if( iter->type==OT_Opponent ) iter->conf=Mem->OpponentPositionValid(iter->unum);
    else if( iter->type==OT_Position ) iter->conf=PositionValid(iter->position);    
  }
  if( newSight ) {
    /*    if( idInfos.size()!=0 ) {
	  ListOfIdInfos::iterator iter=idInfos.begin();
	  Mem->LogAction5(10,"in update: id %.0f conf %.5f threshold %.5f",float(iter->id),iter->conf,iter->threshold);
	  }*/
    valuedPositions.remove_if(LowConfidence(positionThreshold));
    idInfos.remove_if(LowConfOrThresholdIsPassed());
    //    Mem->LogAction3(10,"size is %.0f",float(idInfos.size()));
  }
  if( valuedPositions.size()==0 ) positionsConfUpdateTime=-1;
  else                            positionsConfUpdateTime=time;
  anglesConfUpdateTime=time;
  //  Mem->LogAction2(10,"out of updateConfs");
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddTeammateToList(int teammate, float priority, float priorityFactor) {
  if( !Mem->TeammatePositionValid(teammate) || teammate==Mem->MyNumber ) return;
  Vector targetPos =  Mem->TeammateAbsolutePosition(teammate) +
    ( Mem->TeammateVelocityValid(teammate) ?
      Mem->TeammateAbsoluteVelocity(teammate) : Vector(0,0) );
  AngleDeg targetAng = (targetPos-myPosition).dir();
  float buffer=Min(playerAngleBuffer,viewAngle-2);
  if( considerBodyTurn ) {
    AddAngle(targetAng,priorityFactor*priority,OT_Teammate,teammate);
    AddAngle(targetAng+buffer,0.4*priorityFactor*priority,OT_TeammateBuffer,teammate);
    AddAngle(targetAng-buffer,0.4*priorityFactor*priority,OT_TeammateBuffer,teammate);
  }else{
    if( !CanSeeAngle(targetAng) ) return;
    AddAngle(targetAng,priorityFactor*priority,OT_Teammate,teammate);
    AddAngle(AdjustViewAngle(targetAng,buffer),0.4*priorityFactor*priority,OT_TeammateBuffer,teammate);
    AddAngle(AdjustViewAngle(targetAng,-buffer),0.4*priorityFactor*priority,OT_TeammateBuffer,teammate);
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddOpponentToList(int opponent, float priority, float priorityFactor) {
  if( !Mem->OpponentPositionValid(opponent) ) return;
  Vector targetPos =  Mem->OpponentAbsolutePosition(opponent) +
    ( Mem->OpponentVelocityValid(opponent) ?
      Mem->OpponentAbsoluteVelocity(opponent) : Vector(0,0) );
  AngleDeg targetAng = (targetPos-myPosition).dir();
  float buffer=Min(playerAngleBuffer,viewAngle-2);
  if( considerBodyTurn ) {
    AddAngle(targetAng,priorityFactor*priority,OT_Opponent,opponent);
    AddAngle(targetAng+buffer,0.4*priorityFactor*priority,OT_OpponentBuffer,opponent);
    AddAngle(targetAng-buffer,0.4*priorityFactor*priority,OT_OpponentBuffer,opponent);
  }else{
    if( !CanSeeAngle(targetAng) ) return;
    AddAngle(targetAng,priorityFactor*priority,OT_Opponent,opponent);
    AddAngle(AdjustViewAngle(targetAng,buffer),0.4*priorityFactor*priority,OT_OpponentBuffer,opponent);
    AddAngle(AdjustViewAngle(targetAng,-buffer),0.4*priorityFactor*priority,OT_OpponentBuffer,opponent);
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddBallToList(float priority, float priorityFactor) {
  if( !Mem->BallPositionValid() ) return;
  Vector targetPos =  Mem->BallPredictedPositionWithQueuedActions();
  AngleDeg targetAng = (targetPos-myPosition).dir();
  float buffer=Min(ballAngleBuffer,viewAngle-2);
  if( considerBodyTurn ) {
    AddAngle(targetAng,priorityFactor*priority,OT_Ball);
    AddAngle(targetAng+buffer,0.4*priorityFactor*priority,OT_BallBuffer);
    AddAngle(targetAng-buffer,0.4*priorityFactor*priority,OT_BallBuffer);
  }else{
    if( !CanSeeAngle(targetAng) ) return;
    AddAngle(targetAng,priorityFactor*priority,OT_Ball);
    AddAngle(AdjustViewAngle(targetAng,buffer),0.4*priorityFactor*priority,OT_BallBuffer);
    AddAngle(AdjustViewAngle(targetAng,-buffer),0.4*priorityFactor*priority,OT_BallBuffer);
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddPositionToList(Vector position, float priority, float priorityFactor) {
  AngleDeg targetAng = (position-myPosition).dir();
  float buffer=Min(positionAngleBuffer,viewAngle-2);
  if( considerBodyTurn ) {
    AddAngle(targetAng,priorityFactor*priority,OT_Position,position);
    AddAngle(targetAng+buffer,0.4*priorityFactor*priority,OT_PositionBuffer,position);
    AddAngle(targetAng-buffer,0.4*priorityFactor*priority,OT_PositionBuffer,position);
  }else{
    if( !CanSeeAngle(targetAng) ) return;
    AddAngle(targetAng,priorityFactor*priority,OT_Position,position);
    AddAngle(AdjustViewAngle(targetAng,buffer),0.4*priorityFactor*priority,OT_PositionBuffer,position);
    AddAngle(AdjustViewAngle(targetAng,-buffer),0.4*priorityFactor*priority,OT_PositionBuffer,position);
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MPObject VisualControl::GetMostPrioritizedObject(RequestType rType) {
  MPObject object;
  object.type=OT_Unknown;
  float priority,maxPriority=-1;
  ListOfPositions::iterator pPos;
  int id;
  for( id=0;id<11;id++ ) {
    if( teammates[id].valid && (teammates[id].rType==rType||
				(rType==RT_WithOrWithoutTurn&&(
							       teammates[id].rType==RT_WithTurn||teammates[id].rType==RT_WithoutTurn))) ) {
      priority = teammates[id].priority;
      if( priority>maxPriority || (priority==maxPriority&&float(rand())/RAND_MAX<=0.5) ) {
        maxPriority=priority;
        object.unum=id+1;
        object.type = OT_Teammate;
      }//if( priority>maxPriority )
    }//if( teammates[id].valid && teammates[id].rType==rType )

    if( opponents[id].valid && (opponents[id].rType==rType||
				(rType==RT_WithOrWithoutTurn&&(
							       opponents[id].rType==RT_WithTurn||opponents[id].rType==RT_WithoutTurn))) ) {
      priority = opponents[id].priority;
      if( priority>maxPriority || (priority==maxPriority&&float(rand())/RAND_MAX<=0.5) ) {
        maxPriority=priority;
        object.unum=id+1;
        object.type = OT_Opponent;
      }//if( priority>maxPriority )
    }//if( teammates[id].valid && teammates[id].rType==rType )
  }//for( id=0;id<11;id++ )
  if( ball.valid && (ball.rType==rType||
		     (rType==RT_WithOrWithoutTurn&&(
						    ball.rType==RT_WithTurn||ball.rType==RT_WithoutTurn))) ) {
    priority=ball.priority;
    if( priority>maxPriority || (priority==maxPriority&&float(rand())/RAND_MAX<=0.5) ) {
      maxPriority=priority;
      object.type = OT_Ball;
    }//if( priority>maxPriority )
  }//if( ball.valid )
  for( pPos=positions.begin();pPos!=positions.end();pPos++ ) {
    priority = pPos->priority;
    Mem->LogAction3(10,"MPO priority %.6f",priority);
    if( pPos->rType==rType ||
	(rType==RT_WithOrWithoutTurn&&(
				       pPos->rType==RT_WithTurn||pPos->rType==RT_WithoutTurn)) ) {
      if( priority>maxPriority || (priority==maxPriority&&float(rand())/RAND_MAX<=0.5) ) {
        Mem->LogAction2(10,"cool");
        maxPriority=priority;
        object.type = OT_Position;
        object.position=*pPos;
      }//if( priority>maxPriority )
    }//if( pPos->rType==rType )
  }//for( pPos=positions.begin();pPos!=positions.end();pPos++ )
  return object;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//filter==1 - process RT_MustTurn requests
//filter==2 - process RT_WithOrWithoutTurn
void VisualControl::AddObjectsToList(int filter, MPObject& object) {
  ListOfPositions::iterator pPos;
  int   id;
  float priority;
  for( id=0;id<11;id++ ) {
    if( filter==1 ) {
      if( teammates[id].valid ) {
        priority = teammates[id].priority;
        if( teammates[id].rType==RT_MustSee ) {
          if( object.type==OT_Teammate && object.unum==id+1 ) AddTeammateToList(id+1,priority,priorityHuge);
          else                                                AddTeammateToList(id+1,priority,priorityHight);
        }else if( teammates[id].rType==RT_WithTurn||teammates[id].rType==RT_WithoutTurn ) {
          AddTeammateToList(id+1,priority,priorityMedium);
        }
      }else{//if( teammates[id].valid )
        if( Mem->TeammatePositionValid(id+1) && id+1!=Mem->MyNumber ) {
          priority = GetPlayerPriority(0,id+1,false);
          AddTeammateToList(id+1,priority,priorityLow);
        }//if( Mem->TeammatePositionValid(id+1) && id+1!=Mem->MyNumber )
      }//else{

      if( opponents[id].valid ) {
        priority = opponents[id].priority;
        if( opponents[id].rType==RT_MustSee ) {
          if( object.type==OT_Opponent && object.unum==id+1 ) AddOpponentToList(id+1,priority,priorityHuge);
          else                                                AddOpponentToList(id+1,priority,priorityHight);
        }else if( opponents[id].rType==RT_WithTurn||opponents[id].rType==RT_WithoutTurn ) {
          AddOpponentToList(id+1,priority,priorityMedium);
        }
      }else{//if( opponents[id].valid )
        if( Mem->OpponentPositionValid(id+1) ) {
          priority = GetPlayerPriority(0,id+1,true);
          AddOpponentToList(id+1,priority,priorityLow);
        }//if( Mem->OpponentPositionValid(id+1) )
      }//else{
    }//filter==1

    if( filter==2 ) {
      if( teammates[id].valid ) {
        priority = teammates[id].priority;
        if( teammates[id].rType==RT_WithTurn||teammates[id].rType==RT_WithoutTurn ) {
          if( object.type==OT_Teammate && object.unum==id+1 ) AddTeammateToList(id+1,priority,priorityHuge);
          else                                                AddTeammateToList(id+1,priority,priorityHight);
        }else if( teammates[id].rType==RT_MustSee ) {
          AddTeammateToList(id+1,priority,priorityMedium);
        }
      }else{//if( teammates[id].valid )
        if( Mem->TeammatePositionValid(id+1) && id+1!=Mem->MyNumber ) {
          priority = GetPlayerPriority(0,id+1,false);
          AddTeammateToList(id+1,priority,priorityLow);
        }//if( Mem->TeammatePositionValid(id+1) && id+1!=Mem->MyNumber )
      }//else{

      if( opponents[id].valid ) {
        priority = opponents[id].priority;
        if( opponents[id].rType==RT_WithTurn||opponents[id].rType==RT_WithoutTurn ) {
          if( object.type==OT_Opponent && object.unum==id+1 ) AddOpponentToList(id+1,priority,priorityHuge);
          else                                                AddOpponentToList(id+1,priority,priorityHight);
        }else if( opponents[id].rType==RT_MustSee ) {
          AddOpponentToList(id+1,priority,priorityMedium);
        }
      }else{//if( opponents[id].valid )
        if( Mem->OpponentPositionValid(id+1) ) {
          priority = GetPlayerPriority(0,id+1,true);
          AddOpponentToList(id+1,priority,priorityLow);
        }//if( Mem->OpponentPositionValid(id+1) )
      }//else{
    }//filter==2
  }//for( id=0;id<11;id++ )

  if( filter==1 ) {
    if( ball.valid ) {
      priority=ball.priority;
      if( ball.rType==RT_MustSee ) {
        if( object.type==OT_Ball ) AddBallToList(priority,priorityHuge);
        else                       AddBallToList(priority,priorityHight);
      }else if( ball.rType==RT_WithTurn||ball.rType==RT_WithoutTurn ){
        AddBallToList(priority,priorityMedium);
      }
    }else{//if( ball.valid )
      if( Mem->BallPositionValid() ) {
        priority = GetBallPriority(0);
        AddBallToList(priority,priorityLow);
      }
    }
  }//filter==1

  if( filter==2 ) {
    if( ball.valid ) {
      priority=ball.priority;
      if( ball.rType==RT_WithTurn||ball.rType==RT_WithoutTurn ) {
        if( object.type==OT_Ball ) AddBallToList(priority,priorityHuge);
        else                       AddBallToList(priority,priorityHight);
      }else if( ball.rType==RT_MustSee ){
        AddBallToList(priority,priorityMedium);
      }
    }else{//if( ball.valid )
      if( Mem->BallPositionValid() ) {
        priority = GetBallPriority(0);
        AddBallToList(priority,priorityLow);
      }
    }
  }//filter==2

  for( pPos=positions.begin();pPos!=positions.end();pPos++ ) {
    priority = pPos->priority;
    if( filter==1 ) {
      if( pPos->rType==RT_MustSee ) {
        if( object.type==OT_Position && *pPos==object.position ) AddPositionToList(pPos->position,priority,priorityHuge);
        else                                                     AddPositionToList(pPos->position,priority,priorityHight);
      }else if( pPos->rType==RT_WithTurn||pPos->rType==RT_WithoutTurn ){//if( pPos->rType==RT_MustSee )
        AddPositionToList(pPos->position,priority,priorityMedium);
      }
    }//filter==1

    if( filter==2 ) {
      if( pPos->rType==RT_WithTurn||pPos->rType==RT_WithoutTurn ) {
        if( object.type==OT_Position && *pPos==object.position ) AddPositionToList(pPos->position,priority,priorityHuge);
        else                                                     AddPositionToList(pPos->position,priority,priorityHight);
      }else if( pPos->rType==RT_MustSee ){//if( pPos->rType==RT_MustSee )
        AddPositionToList(pPos->position,priority,priorityMedium);
      }
    }//filter==1
  }//for( pPos=positions.begin();pPos!=positions.end();pPos++ )
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::RemoveObject(MPObject& object) {
  if( object.type==OT_Ball ) ball.valid=false;
  else if( object.type==OT_Teammate ) teammates[object.unum-1].valid=false;
  else if( object.type==OT_Opponent ) opponents[object.unum-1].valid=false;
  else if( object.type==OT_Position ) positions.remove(object.position);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::DidNotSee(MPObject& object) {
  if( object.type==OT_Ball && ball.lastPosValid>Mem->BallPositionValid() ) {
    Mem->LogAction2(10,"ball is mpo again, and did not see");
    return true;
  }
  if( object.type==OT_Teammate && lastMPObject.unum==object.unum && teammates[object.unum-1].lastPosValid>Mem->TeammatePositionValid(object.unum) ) {
    Mem->LogAction3(10,"teammate %.0f is mpo again, and did not see",float(object.unum));
    return true;
  }
  if( object.type==OT_Opponent && lastMPObject.unum==object.unum && opponents[object.unum-1].lastPosValid>Mem->OpponentPositionValid(object.unum) ) {
    Mem->LogAction3(10,"opponent %.0f is mpo again, and did not see",float(object.unum));
    return true;
  }

  if( object.type==OT_Position && lastMPObject.position.lastPosValid>object.position.lastPosValid ) {
    Mem->LogAction4(10,"position (%.2f,%.2f) is mpo again, and did not see",object.position.position.x,object.position.position.y);
    return true;
  }
  return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int VisualControl::PlayerBufferCycles(float posValid) {
  return (int)ceil(log(posValid)/log(Mem->CP_player_conf_decay));  
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
int VisualControl::BallBufferCycles(float posValid) {
  return (int)ceil(log(posValid)/log(Mem->CP_ball_conf_decay));    
}  
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SetBufferAngles() {
  float b;
  float a;
  for( int id=1;id<=11;id++ ) {
    if( Mem->TeammatePositionValid(id) && id!=Mem->MyNumber ) {
      b=Mem->TeammateDistance(id);
      a=Mem->SP_player_speed_max*PlayerBufferCycles( Mem->TeammatePositionValid(id) );
      teammates[id-1].buffAngle=ASin(a/b);
      //      Mem->LogAction5(10,"teammate %.0f buff %.2f conf %.2f",float(id),teammates[id-1].buffAngle,Mem->TeammatePositionValid(id));
    }
    if( Mem->OpponentPositionValid(id) ) {
      b=Mem->OpponentDistance(id);
      a=Mem->SP_player_speed_max*PlayerBufferCycles( Mem->OpponentPositionValid(id) );
      opponents[id-1].buffAngle=ASin(a/b);
      //      Mem->LogAction5(10,"opponent %.0f buff %.2f conf %.2f",float(id),opponents[id-1].buffAngle,Mem->OpponentPositionValid(id));
    }
  }
  if( Mem->BallPositionValid() ) {
    b=Mem->BallDistance();
    a=Mem->SP_ball_speed_max*BallBufferCycles( Mem->BallPositionValid() );
    ball.buffAngle=ASin(a/b);
    //    Mem->LogAction4(10,"ball buff %.2f conf %.2f",ball.buffAngle,Mem->BallPositionValid());
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AdjustViewWidth(MPObject& object) {
  float maxBuffAngle=0;
  Vwidth width;
  if( Mem->BallKickable() ) {
    SetMediumBuffers();
    SetViewWidth(object,VW_Normal);
    return;
  }
  
  if( object.type==OT_Ball ) maxBuffAngle=ball.buffAngle;
  else if( object.type==OT_Teammate ) maxBuffAngle=teammates[object.unum-1].buffAngle;
  else if( object.type==OT_Opponent ) maxBuffAngle=opponents[object.unum-1].buffAngle;
  else maxBuffAngle=15;

  //  Mem->LogAction3(10,"in avw buff ang is %.2f",maxBuffAngle);
      
  if( maxBuffAngle<=38 ) {
    SetMediumBuffers();
    width=VW_Normal;
  } else {
    SetHugeBuffers();
    width=VW_Wide;
  }
  SetViewWidth(object,width);  
}  
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanHandleObjectsThatMustSee() {
  MPObject object = GetMostPrioritizedObject(RT_MustSee);
  if( object.type==OT_Unknown ) {
    Mem->LogAction2(10,"CanHandleObjectsThatMustSee - no such objects at all");
    return false;//no objects at all
  }

  if( !correction && lastMPObject==object && DidNotSee(object) ) {
    AdjustViewWidth(object);
  }
  else {
    SetBuffersByDefault();
    SetViewWidth(object);
  }

  if( CanSeeObject(object) ) {
    Mem->LogAction2(10,"CanHandleObjectsThatMustSee - do not consider body turn");
    considerBodyTurn=false;
  }else{
    Mem->LogAction2(10,"CanHandleObjectsThatMustSee - consider body turn");
    considerBodyTurn=true;
  }
  LogMPObject(10,object);
  requests.clear();
  AddObjectsToList(1,object);
  if( requests.size()==0 ) Mem->LogAction2(10,"error - at least 1 element must be in list");
  if( !considerBodyTurn ) {
    if( TurnNeck() ) {
      lastMPObject=object;
      return true;
    }
    Mem->LogAction2(10,"CanHandleObjectsThatMustSee - internal error, wrong neck angle when don't consider body turn");
    return false;
  }else{
    AngleDeg neckAngle=GetAngleForNeck();
    if( neckAngle==WRONG_ANGLE ) {
      Mem->LogAction2(10,"CanHandleObjectsThatMustSee - internal error, wrong neck angle when consider body turn");
      return false;
    }
    turn(neckAngle-Mem->MyBodyAng());
    turn_neck_to_relative_angle(0);
    lastMPObject=object;
    return true;
  }
  return false;//???
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//if there are objects, that must see, it assigns 'em lowest priority factor
bool VisualControl::CanHandleObjectsThatMusntSee() {
  bool  tmpTeammates[11];
  bool  tmpOpponents[11];
  bool  tmpBall;
  ListOfPositions oldPositions = positions;
  int id;
  MPObject object;

  for( id=0;id<11;id++ ) {
    tmpTeammates[id] = teammates[id].valid;
    tmpOpponents[id] = opponents[id].valid;
  }
  tmpBall=ball.valid;
  Vwidth oldWidth = vWidth;
  while( true ) {
    object = GetMostPrioritizedObject(RT_WithOrWithoutTurn);
    if( object.type==OT_Unknown ) {
      Mem->LogAction2(10,"unknown, ups");
      break;
    }
    if( !correction && lastMPObject==object && DidNotSee(object) ) {
      AdjustViewWidth(object);
    }else {
      SetBuffersByDefault();
      SetViewWidth(object);
    }
    if( !CanSeeObject(object) ) {
      if( (object.type==OT_Ball && ball.rType==RT_WithoutTurn) ||
          (object.type==OT_Teammate && teammates[object.unum-1].rType==RT_WithoutTurn) ||
          (object.type==OT_Opponent && opponents[object.unum-1].rType==RT_WithoutTurn) ||
          (object.type==OT_Position && object.position.rType==RT_WithoutTurn) ) {
        Mem->LogAction2(10,"CanHandleObjectsThatMusntSee - removing, 'cause can't see and can't turn body for this request");
        LogMPObject(10,object);
        RemoveObject(object);
        continue;
      }
      considerBodyTurn = true;
      break;
    }else{
      considerBodyTurn = false;
      break;
    }
  }//while(true)
  positions=oldPositions;
  for( id=0;id<11;id++ ) {
    teammates[id].valid=tmpTeammates[id];
    opponents[id].valid=tmpOpponents[id];
  }
  ball.valid=tmpBall;
  if( object.type==OT_Unknown ) {
    Mem->LogAction2(10,"CanHandleObjectsThatMusntSee - no objects in list or can't execute request(s)");
    vWidth=oldWidth;
    change_view(vWidth);//add by AI
    viewAngle = GetViewAngle(vWidth);
    return false;
  }
  requests.clear();
  Mem->LogAction2(10,"CanHandleObjectsThatMusntSee - most priority object is:");
  LogMPObject(10,object);
  AddObjectsToList(2,object);
  if( !considerBodyTurn ) {
    if( TurnNeck() ) {
      lastMPObject=object;
      return true;
    }
    Mem->LogAction2(10,"CanHandleObjectsThatMusntSee - internal error, wrong neck angle when don't consider body turn");
    return false;
  }else{
    AngleDeg neckAngle=GetAngleForNeck();
    if( neckAngle==WRONG_ANGLE ) {
      Mem->LogAction2(10,"CanHandleObjectsThatMusntSee - internal error, wrong neck angle when consider body turn");
      return false;
    }
    turn(neckAngle-Mem->MyBodyAng());
    turn_neck_to_relative_angle(0);
    lastMPObject=object;
    return true;
  }
  return false;//???can we be here???
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanHandleAddedObjects() {
  if(	!mustRequestsExist && !otherRequestsExist ) return false;
			
  int id;
  ListOfPositions::iterator p;
  for( id=0;id<11;id++ )
    if( teammates[id].valid )  {
      if( teammates[id].rType==RT_Unknown )  teammates[id].rType=RT_WithoutTurn;
      if( teammates[id].vWidth==VW_Unknown ) teammates[id].vWidth=vWidth;
      LogPlayer(10,id,false);
      if( teammates[id].priority<=basePriority && teammates[id].rType!=RT_MustSee ) teammates[id].valid=false;
    }
  for( id=0;id<11;id++ )
    if( opponents[id].valid )  {
      if( opponents[id].rType==RT_Unknown )  opponents[id].rType=RT_WithoutTurn;
      if( opponents[id].vWidth==VW_Unknown ) opponents[id].vWidth=vWidth;
      LogPlayer(10,id,true);
      if( opponents[id].priority<=basePriority && opponents[id].rType!=RT_MustSee ) opponents[id].valid=false;
    }
  if( ball.valid )  {
    if( ball.rType==RT_Unknown )  ball.rType=RT_WithoutTurn;
    if( ball.vWidth==VW_Unknown ) ball.vWidth=vWidth;
    LogBall(10);
    if( ball.priority<=basePriority && ball.rType!=RT_MustSee ) ball.valid=false;
  }
  for( p=positions.begin();p!=positions.end();p++ ) {
    if( p->rType==RT_Unknown )  p->rType=RT_WithoutTurn;
    if( p->vWidth==VW_Unknown ) p->vWidth=vWidth;
    LogPosition(10,*p);
  }
  positions.remove_if(HightConfOrBadId(basePriority));
  if(	!mustRequestsExist && !otherRequestsExist ) return false;
  if( mustRequestsExist && CanHandleObjectsThatMustSee() )  return true;
  if( otherRequestsExist) return CanHandleObjectsThatMusntSee();
  return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SetBodyTurningAngles() {
  if( Mem->Action->valid()&&Mem->Action->type==CMD_turn) {
    bodyTurnAngle = Mem->Action->angle;
    if( Mem->MyVelConf() ) bodyTurnAngle /= (1 + Mem->GetMyInertiaMoment() * Mem->MySpeed());
    if( fabs(bodyTurnAngle)<0.1 ) bodyTurnAngle=0;
  }else{
    bodyTurnAngle=0;
  }
  bodyNewAngle = GetNormalizeAngleDeg( Mem->MyBodyAng()+bodyTurnAngle );
  myPosition = Mem->MyPredictedPositionWithQueuedActions();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::Observe() {  
  correction=false;
  int id;
  Unum opp=Mem->ClosestOpponentToBall();
  SetBuffersByDefault();
  SetBodyTurningAngles();
  SetBufferAngles();
  if( Mem->PlayMode==PM_Play_On&&Mem->LastSightTime==Mem->CurrentTime && !Mem->TimeToTurnForScan() ) {
    //my_error("SCAN ERROR (new) - got new sight this cycle, but it's not time to turn for scan???");
    goto SCANFIELD_;
  }
  if( Mem->PlayMode==PM_Play_On&&Mem->LastSightTime==Mem->CurrentTime-1&&lastObservationTime!=Mem->CurrentTime-1&&!Mem->TimeToTurnForScan() ) {
    //my_error("SCAN ERROR (new) - got new sight previous cycle, but there wasn't observation last cycle and");
    //my_error("SCAN ERROR (new) - it's not a time to scan this cycle????");
    goto SCANFIELD_;
  }
  if( Mem->CurrentTime-lastObservationTime>3 ) goto SCANFIELD_;
  if( !Mem->TimeToTurnForScan() ) {
    if( bodyTurnAngle==0 ) {
      Mem->LogAction2(10,"it's not a time to turn for scan,no body turn , keep the previous view width");
      goto FINALIZE;
    }
    Mem->LogAction2(10,"It's not a time to scan turn , but gonna turn body");
    AngleDeg neckAngle=Mem->MyNeckRelAng();
    neckAngle-=bodyTurnAngle;
    NormalizeAngleDeg(&neckAngle);
    if( fabs(neckAngle)<=90 ) {
      turn_neck_to_relative_angle(neckAngle);
      goto FINALIZE;
    }
    Mem->LogAction2(10,"Eye - it's not a time for scan,  but due to the body turn correction is needed");
    correction=true;
  }

 SCANFIELD_:
  lastObservationTime = Mem->CurrentTime;
  if( vWidth==VW_Unknown )      vWidth = GetBestViewWidth();
  viewAngle = GetViewAngle(vWidth);

  if( Mem->PlayMode==PM_Before_Kick_Off ) {
    WideLook();
    goto FINALIZE;
  }

  considerBodyTurn=false;
  lastMPObject.type=OT_Unknown;
  if( Mem->ViewWidth!=vWidth )  change_view(vWidth);

  //add by AI
  if(opp!=Unum_Unknown&&Mem->BallKickableForOpponent(opp,-(Mem->GetOpponentPlayerSize(opp)-Mem->SP_player_size))&&BallLook()){
    Mem->LogAction3(10,"look to ball then he is kickable for opponent %.0f",float(opp));
    goto FINALIZE;
  }
  if(Pos.FastestTm()!=Unum_Unknown&&Pos.FastestTm()!=Mem->MyNumber&&((Pos.TmCycles()<1||Mem->TeammateDistanceToBall(Mem->ClosestTeammateToBall())<3.0f)&&
								     Mem->DistanceTo(Mem->BallAbsolutePosition())<10.0f)&&BallLook()){
    Mem->LogAction3(10,"look to ball tm control ball close to us %.0f",float(Pos.FastestTm()));
    change_view(VW_Narrow);
    goto FINALIZE;
  }    
  if(Pos.FastestTm()==Mem->MyNumber&&Pos.TmCycles()<=1&&Mem->BallVelocityValid()<0.96f&&
     (Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum)||
      NumOfCyclesThenILastSeePlayer(-Mem->TheirGoalieNum)<=((Mem->OpponentX(Mem->TheirGoalieNum)-Mem->BallX())<7.0f?0:1))&&BallLook()){
    Mem->LogAction2(10,"ball look then bad his vel");
    goto FINALIZE;
  }
  //end addition

  
  if( !Mem->BallKickable() && Pos.FastestTm()==Mem->MyNumber && Pos.TmCycles()<2 && BallLook() ) {
    Mem->LogAction2(10,"first ball look success");
    goto FINALIZE;
  }
  if( CanHandleAddedObjects() ) {
    Mem->LogAction2(10,"agent processed added requests");
    goto FINALIZE;
  }
  if( TheirGoalieLook() ) {
    Mem->LogAction2(10,"their goalie look success");
    goto FINALIZE;
  }
  if( Mem->BallKickable() && PassLook() ) {
    Mem->LogAction2(10,"pass look success");
    goto FINALIZE;
  }
  if( OffSideLook() ) {
    Mem->LogAction2(10,"offside look success");
    goto FINALIZE;
  };
  if( !Mem->BallKickable() && BallLook() ) {
    Mem->LogAction2(10,"second ball look success");
    goto FINALIZE;
  }
  SimpleLook();
  Mem->LogAction2(10,"simple look success");
 FINALIZE:
  //before leaving save info for the next cycle
  requests.clear();
  positions.clear();
  for( id=0;id<11;id++ ) {
    teammates[id].valid = opponents[id].valid  = false;
    teammates[id].lastPosValid = Mem->TeammatePositionValid(id+1);
    opponents[id].lastPosValid = Mem->OpponentPositionValid(id+1);
  }

  ball.valid = false;
  ball.lastPosKnown = true;
  ball.lastPos = Mem->BallAbsolutePosition();
  ball.lastPosValid = Mem->BallPositionValid();

  vWidth = VW_Unknown;
  viewAngle = 0;
  bodyTurnAngle = 0;
  bodyNewAngle = 0;
  considerBodyTurn = false;
  searchBallDirection=1;
  mustRequestsExist=false;
  otherRequestsExist=false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeeAngle(AngleDeg globalAngle) {
  NormalizeAngleDeg(&globalAngle);
  return GetDiff(bodyNewAngle,globalAngle)<=90+viewAngle;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeePosition( Vector targetPosition) {
  AngleDeg targetAng = (targetPosition-myPosition).dir();
  AngleDeg bufferAng = positionAngleBuffer;
  AngleDeg relAngle = GetNormalizeAngleDeg(targetAng-bodyNewAngle);
  if( relAngle>=0 && relAngle+bufferAng>90+viewAngle ) return false;
  if( relAngle<0 && relAngle-bufferAng<-90-viewAngle ) return false;
  return CanSeeAngle(targetAng)&&CanSeeAngle(targetAng+bufferAng)&&CanSeeAngle(targetAng-bufferAng);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeeObject(MPObject& object) {
  if( object.type==OT_Ball )     return CanSeeBall();
  else if( object.type==OT_Teammate ) return CanSeeTeammate(object.unum);
  else if( object.type==OT_Opponent ) return CanSeeOpponent(object.unum);
  else if( object.type==OT_Position ) return CanSeePosition(object.position.position);
  else return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeeBall() {
  if( !Mem->BallPositionValid() ) return false;
  AngleDeg targetAng = (Mem->BallPredictedPositionWithQueuedActions()-myPosition).dir();
  AngleDeg bufferAng = ballAngleBuffer;
  AngleDeg relAngle = GetNormalizeAngleDeg(targetAng-bodyNewAngle);
  if( relAngle>=0 && relAngle+bufferAng>90+viewAngle ) return false;
  if( relAngle<0 && relAngle-bufferAng<-90-viewAngle ) return false;
  return CanSeeAngle(targetAng)&&CanSeeAngle(targetAng+bufferAng)&&CanSeeAngle(targetAng-bufferAng);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeeTeammate(int unum) {
  if( !Mem->TeammatePositionValid(unum) ) return false;
  Vector targetPos =  Mem->TeammateAbsolutePosition(unum) +
    ( Mem->TeammateVelocityValid(unum) ?
      Mem->TeammateAbsoluteVelocity(unum) : Vector(0,0) );
  AngleDeg targetAng = (targetPos-myPosition).dir();
  AngleDeg bufferAng = playerAngleBuffer;
  AngleDeg relAngle = GetNormalizeAngleDeg(targetAng-bodyNewAngle);
  if( relAngle>=0 && relAngle+bufferAng>90+viewAngle ) return false;
  if( relAngle<0 && relAngle-bufferAng<-90-viewAngle ) return false;
  return CanSeeAngle(targetAng)&&CanSeeAngle(targetAng+bufferAng)&&CanSeeAngle(targetAng-bufferAng);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::CanSeeOpponent(int unum) {
  if( !Mem->OpponentPositionValid(unum) ) return false;
  Vector targetPos =  Mem->OpponentAbsolutePosition(unum) +
    ( Mem->OpponentVelocityValid(unum) ?
      Mem->OpponentAbsoluteVelocity(unum) : Vector(0,0) );
  AngleDeg targetAng = (targetPos-myPosition).dir();
  AngleDeg bufferAng = playerAngleBuffer;
  AngleDeg relAngle = GetNormalizeAngleDeg(targetAng-bodyNewAngle);
  if( relAngle>=0 && relAngle+bufferAng>90+viewAngle ) return false;
  if( relAngle<0 && relAngle-bufferAng<-90-viewAngle ) return false;
  return CanSeeAngle(targetAng)&&CanSeeAngle(targetAng+bufferAng)&&CanSeeAngle(targetAng-bufferAng);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg VisualControl::AdjustViewAngle(AngleDeg globalAngle, AngleDeg deltaAngle) {
  AngleDeg relAngle = GetNormalizeAngleDeg(globalAngle-bodyNewAngle);
  relAngle+=deltaAngle;
  if( relAngle>90+viewAngle ) relAngle=90+viewAngle;
  if( relAngle<-(90+viewAngle) ) relAngle=-(90+viewAngle);
  return GetNormalizeAngleDeg(relAngle+bodyNewAngle);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg VisualControl::AdjustNeckAngle(AngleDeg globalAngle) {
  if( GetDiff(globalAngle,bodyNewAngle)<=90 ) return globalAngle;
  return GetGlobalAngle( 90*signf(GetRelativeAngle(globalAngle)) );
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::GetPlayerPriority(int numOfCycles, Unum num, bool isOpp ) {
  if( isOpp && !Mem->OpponentPositionValid(num) ) return 0.0f;
  if( !isOpp && !Mem->TeammatePositionValid(num) ) return 0.0f;
  float conf = Mem->CP_max_conf;
  conf*=pow(Mem->CP_player_conf_decay,numOfCycles);
  if( conf<Mem->CP_min_valid_conf ) conf=0;
  if( isOpp ) return basePriority+conf-Mem->OpponentPositionValid(num);
  else        return basePriority+conf-Mem->TeammatePositionValid(num);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::GetBallPriority(int numOfCycles) {
  if( !Mem->BallPositionValid() ) return 0.0f;
  float conf = Mem->CP_max_conf;
  conf*=pow(Mem->CP_ball_conf_decay,numOfCycles);
  if( conf<Mem->CP_min_valid_conf ) conf=0;
  return basePriority+conf-Mem->BallPositionValid();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::GetPositionPriority(Vector position,int cyclesFactor) {
  float conf = 1.0f;
  conf*=pow(positionConfDecay,cyclesFactor);
  if( conf<positionThreshold ) conf=0;
  return basePriority+conf-PositionValid(position);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::PositionValid(Vector position) {
  ListOfValuedPositions::iterator p;
  for( p=valuedPositions.begin();p!=valuedPositions.end();p++ )
    if( p->position.dist(position)<=distThreshold ) return p->conf;
  return 0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
float VisualControl::FilterListOfRequests(ListOfRequests& list) {
  ListOfRequests tmpList;
  ListOfRequests::iterator iter;
  float priority=0.0f;
  for( iter=list.begin();iter!=list.end();iter++ )
    if( IsRequestValid(list,*iter) ) 
      priority+=iter->priority;
    else
      tmpList.push_back(*iter);	
	
  for( iter=tmpList.begin();iter!=tmpList.end();iter++ )
    list.remove(*iter);	
  return priority;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::IsRequestValid(const ListOfRequests& list, const Request& request) {
  if( request.type==OT_Unknown ) return false;
  if( request.type==OT_Ball || request.type==OT_Teammate ||
      request.type==OT_Opponent || request.type==OT_Position ) return true;

  for( ListOfRequests::const_iterator iter=list.begin();iter!=list.end();iter++ ) {
    if( request.type==OT_BallBuffer && iter->type==OT_Ball ) return true;
    else if( request.type==OT_TeammateBuffer && iter->type==OT_Teammate && request.unum==iter->unum ) return true;
    else if( request.type==OT_OpponentBuffer && iter->type==OT_Opponent && request.unum==iter->unum ) return true;
    else if( request.type==OT_PositionBuffer && iter->type==OT_Position && request.position==iter->position ) return true;
  }
  return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg VisualControl::GetAngleForNeck() {
  if( requests.size()==0 ) return WRONG_ANGLE;
  AngleDeg neckAngle=WRONG_ANGLE;
  float    priority, 
    maxPriority=-100;
  ListOfRequests::iterator iter;
  ListOfRequests::iterator last;
  ListOfRequests  window;
  ListOfRequests  tempWindow;

  requests.sort(LessAngle());
  tempWindow.clear();
  window.clear();
  if( considerBodyTurn ) {
    iter=requests.begin();
    while( iter!=requests.end() && GetDiff(requests.end()->angle,iter->angle)<=viewAngle*2 )
      tempWindow.push_back( *(iter++) );
    copy(tempWindow.begin(),tempWindow.end(),back_inserter(requests));		
  }else {
    requests.remove_if( CanNotSee(90+viewAngle,bodyNewAngle) );
  }
	
  if( requests.size()==0 ) return WRONG_ANGLE;
  iter=requests.begin();
	
  do {
    if( window.size()==0 && iter==requests.end() ) break;//just for case
    if( window.size()==0 && iter!=requests.end()) window.push_back( *(iter++) );
    while( iter!=requests.end() && GetDiff(window.begin()->angle,iter->angle)<=viewAngle*2 ) 
      window.push_back( *(iter++) );
		
    if( window.size()!=0 ) {
      tempWindow=window;
      priority=FilterListOfRequests(tempWindow);
      if( (priority>maxPriority || priority==maxPriority && float(rand())/RAND_MAX<=0.5 )&& tempWindow.size()!=0 ) {
	last=tempWindow.end();last--;	
	neckAngle = (tempWindow.begin()->angle+last->angle)/2;
	maxPriority=priority;	
	if( considerBodyTurn )	neckAngle = GetNormalizeAngleDeg(neckAngle);				
	else										neckAngle = AdjustNeckAngle(GetNormalizeAngleDeg(neckAngle));			
      }
      window.pop_front();
    }
  } while(iter!=requests.end());
	
  if( neckAngle==WRONG_ANGLE ) return WRONG_ANGLE;
  return neckAngle;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::WideLook() {
  if( Mem->ViewWidth!=VW_Wide ) change_view(VW_Wide);
  turn_neck_to_relative_angle( -1.0*signf(Mem->MyNeckRelAng())*Mem->SP_max_neck_angle );
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::PassLook() {
  int   num_receivers=0;
  float priority;
  AddBallToList(GetBallPriority(0),priorityLow);
  for( int pl=1;pl<=11;pl++ ) {
    if( Mem->TeammatePositionValid(pl) && pl!=Mem->OurGoalieNum &&
        pl!=Mem->MyNumber &&	!Mem->OwnPenaltyArea.IsWithin(Mem->TeammateAbsolutePosition(pl)) &&
        Mem->FieldRectangle.IsWithin(Mem->TeammateAbsolutePosition(pl)) &&
        Mem->TeammateDistance(pl)>4 &&
        Mem->TeammateDistance(pl)<actions.GetMaxPassDistance() &&
        Mem->TeammateX(pl)+2>Mem->MyX() ) {
      if( CanSeeTeammate(pl) ) {
	priority=GetPlayerPriority(0,pl,false);
	Mem->LogAction4(10,"Pass Look can see teammate %.0f with priority %.6f",float(pl),priority);
	AddTeammateToList(pl,priority,priorityHight);
	num_receivers++;
      }
    }
    if( Mem->OpponentPositionValid(pl) &&
        Mem->OpponentDistance(pl)<actions.GetMaxPassDistance() &&
        Mem->OpponentDistance(pl)>Mem->SP_feel_distance &&
        Mem->OpponentX(pl)>Mem->MyX() ) {
      if( Mem->TheirGoalieNum==pl && Mem->MyX()>25 ) {
	AddOpponentToList(pl,GetPlayerPriority(0,pl,true),priorityHight);
      }else {
	if( Mem->OpponentDistance(pl)<10 )
	  AddOpponentToList(pl,GetPlayerPriority(0,pl,true),priorityHight);
	else
	  AddOpponentToList(pl,GetPlayerPriority(0,pl,true),priorityMedium);
      }
    }
  }
  if( num_receivers==0 ) return false;
  return TurnNeck();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::OffSideLook() {
  Unum num = Mem->my_offside_opp;
  if( num==Unum_Unknown || !Mem->OpponentPositionValid(num) ||
      fabs(Mem->MyX()-Mem->my_offside_line)>15 ||
      Pos.FastestTm()==Mem->MyNumber && Pos.TmCycles()<=5 ||
      Mem->my_offside_conf>=0.99f ) return false;
  if( !CanSeeTeammate(num) ) return false;
  Mem->LogAction3(10,"Offside Look - can see opponent %.0f",float(num));
  AddOpponentToList(num,GetPlayerPriority(0,num,true),priorityMedium);
  if(vWidth==VW_Unknown)
    my_error("OffsideLook:vWidth is VW_Unknown");
//   else
//     if( vWidth!=VW_Narrow )
//       AddBallToList(GetBallPriority(0),priorityLow);

  return TurnNeck();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::TheirGoalieLook() {
  Unum goalie = Mem->TheirGoalieNum;
  if( Mem->MyX()<30.0f||fabs(Mem->MyY())>25.0f||
      goalie==Unum_Unknown || Mem->OpponentPositionValid(goalie)==1.0f ) return false;
  if( !CanSeeOpponent(goalie) ) return false;
  Mem->LogAction3(10,"Their goalie Look - can see opponent %.0f",float(goalie));
  AddOpponentToList(goalie,GetPlayerPriority(0,goalie,true),priorityMedium);
  if(vWidth==VW_Unknown)
    my_error("TheirGoalieLook:vWidth is VW_Unknown");
  else
    if( vWidth!=VW_Narrow )
      for( int id=1;id<=11;id++ ) {
	if( Mem->TeammatePositionValid(id) && Mem->TheirPenaltyArea.expand(4).IsWithin(Mem->TeammateAbsolutePosition(id)) )
	  AddTeammateToList(id,GetPlayerPriority(0,id,false),priorityLow);
	if( Mem->OpponentPositionValid(id) && Mem->TheirPenaltyArea.expand(4).IsWithin(Mem->OpponentAbsolutePosition(id)) )
	  AddOpponentToList(id,GetPlayerPriority(0,id,true),priorityLow);
      }
  return TurnNeck();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::BallLook() {
  if( !Mem->BallPositionValid() ) return false;
  if( !CanSeeBall() ) return false;
  Mem->LogAction2(10,"Ball Look - can see ball ");
  AddBallToList(GetBallPriority(0),priorityMedium);
  if(vWidth==VW_Unknown)
    my_error("BallLook:vWidth is VW_Unknown");
  else if( vWidth!=VW_Narrow ){
    for( int id=1;id<=11;id++ ) {
      if( Mem->MyX()<Mem->BallX() ) {
	if( Mem->TeammatePositionValid(id) && Mem->MyX()<Mem->TeammateX(id) )
	  AddTeammateToList(id,GetPlayerPriority(0,id,false),priorityLow);
	if( Mem->OpponentPositionValid(id) && Mem->MyX()<Mem->OpponentX(id) )
	  AddOpponentToList(id,GetPlayerPriority(0,id,true),priorityLow);
      }else{
	AddTeammateToList(id,GetPlayerPriority(0,id,false),priorityLow);
	AddOpponentToList(id,GetPlayerPriority(0,id,true),priorityLow);
      }
    }
  }else{//AI: ОШИБКА КОГДА СМОТРИМ НА МЯЧ!!!!
    face_only_neck_to_ball();
    return true;
  }
  
  return TurnNeck();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SimpleLook() {
  AddBallToList(GetBallPriority(0),priorityMedium);
  for( int id=1;id<=11;id++ ) {
    AddTeammateToList(id,GetPlayerPriority(0,id,false),priorityLow);
    AddOpponentToList(id,GetPlayerPriority(0,id,true),priorityLow);
  }
  if( requests.size()!=0 && TurnNeck() ) return;
  Mem->LogAction2(10,"Simple Look - call scan_field_with_neck");
  scan_field_with_neck();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::TurnNeck() {
  AngleDeg neckAngle = GetAngleForNeck();
  requests.clear();
  if( neckAngle==WRONG_ANGLE ) return false;
  turn_neck_to_relative_angle(neckAngle-bodyNewAngle);
  return true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SearchBall() {
  Mem->LogAction2(10,"searching ball...");
  if( Mem->BallPositionValid() ) {
    ball.lastPosKnown=true;
    ball.lastPos = Mem->BallAbsolutePosition();
    if( Mem->BallVelocityValid() ) ball.lastPos+=Mem->BallAbsoluteVelocity();
  }
  if( ball.lastPosKnown ) {
    Mem->LogAction4(10,"last or current ball pos known (%.2f, %.2f)", ball.lastPos.x, ball.lastPos.y);
    if( Mem->ViewWidth==VW_Normal && Mem->LastSightTime!=Mem->CurrentTime &&
        Mem->PredictedNextSightInterval()==1 ) {
      Mem->LogAction2(10,"keeping normal view width, because sight will be in the next sim step");
    }else{
      if( Mem->ViewWidth!=VW_Normal ) change_view(VW_Normal);
      Mem->LogAction2(10,"setting view width to normal");
    }
    if( float(rand())/RAND_MAX<0.5 )
      searchBallDirection = 1;
    else
      searchBallDirection =-1;
    ball.lastPosKnown = false;
    AngleDeg turnAngle = Mem->AngleToFromBody(ball.lastPos);
    turn( turnAngle );
    turn_neck( -Mem->MyNeckRelAng() );
    lastSearchBallTime = Mem->CurrentTime;
    return;
  }

  if( Mem->PlayMode==PM_Play_On&&Mem->LastSightTime==Mem->CurrentTime && !Mem->TimeToTurnForScan() ) {
    //my_error("SEARCH BALL ERROR (new) - got new sight this cycle, but it's not time to turn for scan???");
    goto SEARCHBALL;
  }

  if( Mem->PlayMode==PM_Play_On&&Mem->LastSightTime==Mem->CurrentTime-1&&lastSearchBallTime!=Mem->CurrentTime-1&&!Mem->TimeToTurnForScan() ) {
    //my_error("SEARCH BALL ERROR (new) - got new sight previous cycle, but there wasn't searching last cycle and");
    //my_error("SEARCH BALL ERROR (new) - it's not a time to search this cycle????");
    goto SEARCHBALL;
  }
  if( !Mem->TimeToTurnForScan() ) {
    Mem->LogAction2(10,"it's not a time to turn for scan");
    return;
  }
 SEARCHBALL:
  lastSearchBallTime = Mem->CurrentTime;
  lastObservationTime = Mem->CurrentTime;

  turn( Mem->MyViewAngle()*2*searchBallDirection-3*Mem->CP_scan_overlap_angle );
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool VisualControl::IsIdActive(int id) {
  if( id==-1 ) return false;
  for( ListOfIdInfos::iterator iter=idInfos.begin();iter!=idInfos.end();iter++ )
    if( iter->id==id ) return true;
  return false;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddTeammate( Unum teammate, int cyclesFactor,  RequestType rType, Vwidth vWidth,int id ) {
  if( teammate==Unum_Unknown||!Mem->TeammatePositionValid(teammate) || teammate==Mem->MyNumber ) return;
  float priority = GetPlayerPriority(cyclesFactor,teammate,false);
  if( teammates[teammate-1].valid && teammates[teammate-1].priority>priority ) return;
  teammates[teammate-1].valid = true;
  teammates[teammate-1].priority = priority;
  teammates[teammate-1].vWidth = vWidth;
  teammates[teammate-1].rType = rType;
  teammates[teammate-1].validWrtId=!IsIdActive(id);
  if( teammates[teammate-1].validWrtId && id!=-1) {
    IdInfo info;
    info.type=OT_Teammate;
    info.unum=teammate;
    info.conf=Mem->TeammatePositionValid(teammate);
    info.threshold=pow(Mem->CP_player_conf_decay,cyclesFactor);
    info.id=id;
    info.counterLimit=cyclesFactor;
    info.counter=0;
    idInfos.push_back(info);
  }
  if( rType==RT_MustSee )
    mustRequestsExist=true;
  else
    otherRequestsExist=true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddOpponent( Unum opponent, int cyclesFactor, RequestType rType, Vwidth vWidth, int id ) {
  if( opponent==Unum_Unknown||!Mem->OpponentPositionValid(opponent) ) return;
  float priority = GetPlayerPriority(cyclesFactor,opponent,true);
  if( opponents[opponent-1].valid && opponents[opponent-1].priority>priority ) return;
  opponents[opponent-1].valid = true;
  opponents[opponent-1].priority = priority;
  opponents[opponent-1].vWidth = vWidth;
  opponents[opponent-1].rType = rType;
  opponents[opponent-1].validWrtId=!IsIdActive(id);
  if( opponents[opponent-1].validWrtId && id!=-1) {
    IdInfo info;
    info.type=OT_Opponent;
    info.unum=opponent;
    info.conf=Mem->OpponentPositionValid(opponent);
    info.threshold=pow(Mem->CP_player_conf_decay,cyclesFactor);
    info.id=id;
    info.counterLimit=cyclesFactor;
    info.counter=0;
    idInfos.push_back(info);
  }
  if( rType==RT_MustSee )
    mustRequestsExist=true;
  else
    otherRequestsExist=true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddBall(int cyclesFactor, RequestType rType, Vwidth vWidth,int id ) {
  if( !Mem->BallPositionValid() ) return;
  float priority = GetBallPriority(cyclesFactor);
  if( ball.valid && ball.priority>priority ) return;
  ball.valid=true;
  ball.priority = priority;
  ball.vWidth = vWidth;
  ball.rType = rType;
  ball.validWrtId=!IsIdActive(id);
  if( ball.validWrtId && id!=-1) {
    IdInfo info;
    info.type=OT_Ball;
    info.conf=Mem->BallPositionValid();
    info.threshold=pow(Mem->CP_ball_conf_decay,cyclesFactor);
    info.id=id;
    info.counterLimit=cyclesFactor;
    info.counter=0;
    idInfos.push_back(info);
  }
  if( rType==RT_MustSee )
    mustRequestsExist=true;
  else
    otherRequestsExist=true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddPosition(Vector position, int cyclesFactor,int id, RequestType rType, Vwidth vWidth) {
  Position posObject;
  bool  positionFound = false;
  float priority = GetPositionPriority(position,cyclesFactor);
  IdInfo info;
    
  posObject.position = position;
  posObject.priority = priority;
  //  Mem->LogAction3(10,"PRIORITY %.5f",priority);
  posObject.vWidth = vWidth;
  posObject.rType = rType;
  posObject.lastPosValid=0;
  posObject.validWrtId=!IsIdActive(id);
  
  if( !posObject.validWrtId ) {
    positions.push_back(posObject);
    if( rType==RT_MustSee )
      mustRequestsExist=true;
    else
      otherRequestsExist=true;
    return;
  }

  if( posObject.validWrtId && id!=-1) {
    info.type=OT_Position;
    info.position=position;
    info.conf=PositionValid(position);
    info.threshold=pow(positionConfDecay,cyclesFactor);
    info.id=id;
    info.counterLimit=cyclesFactor;
    info.counter=0;
    idInfos.push_back(info);
    //    Mem->LogAction3(10,"adding id info about position %.0f",float(idInfos.size()));
  }
  
  for( ListOfPositions::iterator p=positions.begin();p!=positions.end();p++ )
    if( p->position.dist(position)<distThreshold ) { //already in list
      if( p->priority<priority ) {
        p->position=position;
        p->priority=priority;
        p->vWidth=vWidth;
        p->rType=rType;
      }
      return;
    }
    
  for( ListOfValuedPositions::iterator iter=valuedPositions.begin();iter!=valuedPositions.end();iter++ )
    if( iter->position.dist(position)<distThreshold ) {
      positionFound=true;
      posObject.position=iter->position;
      posObject.lastPosValid=iter->conf;
      break;
    }

  if( !positionFound ) {
    ValuedPosition pos;
    pos.position=position;
    pos.conf=0;
    valuedPositions.push_back(pos);
  }
  positions.push_back(posObject);
  if( rType==RT_MustSee )
    mustRequestsExist=true;
  else
    otherRequestsExist=true;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddAngle(AngleDeg angle, float priority,ObjectType type) {
  Request request;
  request.priority = priority;
  request.angle    = GetNormalizeAngleDeg(angle);
  request.type=type;
  requests.push_back(request);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddAngle(AngleDeg angle, float priority,ObjectType type,Unum unum){
  Request request;
  request.priority = priority;
  request.angle    = GetNormalizeAngleDeg(angle);
  request.type=type;
  request.unum=unum;
  requests.push_back(request);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::AddAngle(AngleDeg angle, float priority,ObjectType type,Vector position) {
  Request request;
  request.priority = priority;
  request.angle    = GetNormalizeAngleDeg(angle);
  request.type=type;
  request.position=position;
  requests.push_back(request);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SetViewWidth(MPObject& object) {
  if( object.type==OT_Ball )          vWidth=ball.vWidth;
  else if( object.type==OT_Teammate ) vWidth=teammates[object.unum-1].vWidth;
  else if( object.type==OT_Opponent ) vWidth=opponents[object.unum-1].vWidth;
  else if( object.type==OT_Position ) vWidth=object.position.vWidth;
  else if( vWidth==VW_Unknown )       vWidth=GetBestViewWidth();
  viewAngle = GetViewAngle(vWidth);
  if( Mem->ViewWidth!=vWidth )  change_view(vWidth);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::SetViewWidth(MPObject& object,Vwidth vWidth) {
  if( object.type==OT_Ball )          ball.vWidth=vWidth;
  else if( object.type==OT_Teammate ) teammates[object.unum-1].vWidth=vWidth;
  else if( object.type==OT_Opponent ) opponents[object.unum-1].vWidth=vWidth;
  else if( object.type==OT_Position ) object.position.vWidth=vWidth;
  else if( vWidth==VW_Unknown )       vWidth=GetBestViewWidth();
  SetViewWidth(object);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Vwidth VisualControl::GetBestViewWidth() {
  Vector vel;
  Vector pos;
  Vector myPos;

  if( Mem->PlayMode==PM_Before_Kick_Off )  return VW_Wide;

  if( Mem->BallKickable() ) {
    if(Mem->Action->valid()&&Mem->Action->type==CMD_kick&&
       (Pos.FastestOpp()==Unum_Unknown||(Pos.FastestOpp()!=Unum_Unknown&&Pos.OppCycles()>=4))){
      Unum closest=Mem->ClosestOpponent();
      bool teamless = Mem->NumTeamlessPlayers()>0 ? true : false;
      float tackle_prob=0.0f;
      if(closest!=Unum_Unknown)
	tackle_prob=Mem->GetOpponentTackleProb(Mem->ClosestOpponentToBall(),Mem->BallAbsolutePosition());
	
      if( !((closest!=Unum_Unknown &&Mem->OpponentPositionValid(closest)==1.0f&& Mem->BallKickableForOpponent(closest)) ||
	    (teamless && Mem->ClosestTeamlessPlayerPosition().dist(Mem->BallAbsolutePosition())<=Mem->SP_kickable_area) ||
	    (tackle_prob>0.7f&&Mem->OwnPenaltyArea.IsWithin(Mem->MyPos())))) {

	vel=Mem->BallAbsoluteVelocity()+Polar2Vector( Mem->BallKickRate()*Mem->Action->power, Mem->MyBodyAng()+Mem->Action->angle );
	if(vel.mod()>Mem->SP_ball_speed_max)
	  vel*=Mem->SP_ball_speed_max/vel.mod();
	pos=Mem->BallAbsolutePosition()+vel;
	myPos=Mem->MyPos()+Mem->MyVel();
	if((myPos-pos).mod()>Mem->GetMyKickableArea()){
	  Mem->LogAction2(10,"visual control:After kick ball will leave me, opponents are not so danger, VW_Normal");
	  return VW_Normal;
	}
      }
    }
  }
  if( Mem->BallKickable()  ) {
    Mem->LogAction2(10,"visual control:Ball is kickable, VW_Narrow");
    return VW_Narrow;
  }

  if( Pos.FastestTm()==Mem->MyNumber&&Pos.TmCycles()<=2 ||
      (Pos.FastestTm()!=Unum_Unknown&&Mem->BallKickableForTeammate(Pos.FastestTm())&&Mem->DistanceTo(Mem->BallAbsolutePosition())<10.0f)) {
    Mem->LogAction2(10,"visual control:will have ball in 2 or less cycles (or tm control ball close to us), VW_Narrow");
    return VW_Narrow;
  }
  if( Mem->DistanceTo(Mem->BallPredictedPositionWithQueuedActions())>=Mem->CP_dist_to_wide_view  ) {
    Mem->LogAction2(10,"visual control:Ball is far away, VW_Wide");
    return VW_Wide;
  }
  if( Mem->DistanceTo(Mem->BallPredictedPositionWithQueuedActions())>=6 ) {
    Mem->LogAction2(10,"visual control:Ball is more than 6 meters, VW_Normal");
    return VW_Normal;
  }
  Mem->LogAction2(10,"visual control:Ball is close, but not gonna interecpt VW_Narrow");
  return VW_Narrow;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AngleDeg VisualControl::GetViewAngle(Vwidth viewWidth) {
  switch(viewWidth) {
  case VW_Wide:
    return 90.0f;
  case VW_Normal:
    return 45.0f;
  case VW_Narrow:
    return 22.5f;
  default:
    return 0.0f;
  }
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::Log(int logLevel) {
  if( logLevel>Mem->CP_save_action_log_level ) return;		
  ListOfRequests::iterator p;
  if( requests.size()==0 )
    Mem->LogAction2(10,"There are no visual requests in list");
  else
    for( p=requests.begin();p!=requests.end();p++ ) p->Log(logLevel,CanSeeAngle(p->angle));
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::LogPlayer(int logLevel, int id, bool isOpp) {
  if( logLevel>Mem->CP_save_action_log_level ) return;		
  ostringstream ostr;
  ObjectInfo* info;
  if( isOpp ) info=&opponents[id];
  else        info=&teammates[id];
  if( isOpp ) ostr<<"opponent "<<id+1;
  else ostr<<"teammate "<<id+1;
  ostr<<" priority '";
  if( isOpp ) ostr<<info->priority;
  else ostr<<info->priority;
  ostr<<"' viewWidth";
  if( info->vWidth==VW_Narrow ) ostr<<" 'narrow'";
  else if( info->vWidth==VW_Normal ) ostr<<" 'normal'";
  else if( info->vWidth==VW_Wide ) ostr<<" 'wide'";
  else ostr<<" 'unknown'";
  ostr<<" request type ";
  if( info->rType==RT_WithoutTurn ) ostr<<"'without turn'";
  else if( info->rType==RT_WithTurn ) ostr<<"'with turn'";
  else if( info->rType==RT_MustSee ) ostr<<"'must see now'";
  else ostr<<"'unknown'";
  if( isOpp ) {
    if( CanSeeOpponent(id+1) ) ostr<<" 'can see'";
    else ostr<<" 'can't see'";
  }else{
    if( CanSeeTeammate(id+1) ) ostr<<" 'can see'";
    else ostr<<" 'can't see'";
  }
  if( info->priority<basePriority ) ostr<<" 'conf hight'";
  ostr<<" valid wrtId "<<info->validWrtId;
  Mem->LogAction2(logLevel,ostr.str().c_str());
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::LogBall(int logLevel) {
  if( logLevel>Mem->CP_save_action_log_level ) return;		
  ostringstream ostr;
  ostr<<"ball priority '";
  ostr<<ball.priority;
  ostr<<"' viewWidth";
  if( ball.vWidth==VW_Narrow ) ostr<<" 'narrow'";
  else if( ball.vWidth==VW_Normal ) ostr<<" 'normal'";
  else if( ball.vWidth==VW_Wide ) ostr<<" 'wide'";
  else ostr<<" 'unknown'";
  ostr<<" request type ";
  if( ball.rType==RT_WithoutTurn ) ostr<<"'without turn'";
  else if( ball.rType==RT_WithTurn ) ostr<<"'with turn'";
  else if( ball.rType==RT_MustSee ) ostr<<"'must see now'";
  else ostr<<"'unknown'";
  if( CanSeeBall() ) ostr<<" 'can see'";
  else ostr<<" 'can't see'";
  if( ball.priority<basePriority ) ostr<<" 'conf hight'";
  ostr<<" valid wrtId "<<ball.validWrtId;
  Mem->LogAction2(logLevel,ostr.str().c_str());
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::LogPosition(int logLevel, Position& position) {
  if( logLevel>Mem->CP_save_action_log_level ) return;
  ostringstream ostr;
  ostr<<"position ("<<position.position.x<<","<<position.position.y<<") priority "<<position.priority;
  ostr<<" view width";
  if( position.vWidth==VW_Narrow ) ostr<<" 'narrow'";
  else if( position.vWidth==VW_Normal ) ostr<<" 'normal'";
  else if( position.vWidth==VW_Wide ) ostr<<" 'wide'";
  else ostr<<" 'unknown'";
  ostr<<" request type ";
  if( position.rType==RT_WithoutTurn ) ostr<<"'without turn'";
  else if( position.rType==RT_WithTurn ) ostr<<"'with turn'";
  else if( position.rType==RT_MustSee ) ostr<<"'must see now'";
  else ostr<<"'unknown'";
  if( CanSeePosition(position.position) ) ostr<<" 'can see'";
  else ostr<<" 'can't see'";
  if( position.priority<basePriority ) ostr<<" 'conf hight'";
  ostr<<" valid wrtId "<<position.validWrtId;
  Mem->LogAction2(logLevel,ostr.str().c_str());
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void VisualControl::LogMPObject(int logLevel,MPObject& object) {
  if( logLevel>Mem->CP_save_action_log_level ) return;
  ostringstream ostr;
  ostr<<"most prioritized object ";
  if( object.type==OT_Ball ) ostr<<"'ball'";
  else if( object.type==OT_Teammate ) ostr<<"'teammate'"<<object.unum;
  else if( object.type==OT_Opponent ) ostr<<"'opponent'"<<object.unum;
  else if( object.type==OT_Position ) ostr<<"'position ("<<object.position.position.x<<","<<object.position.position.y<<")'";
  Mem->LogAction2(logLevel,ostr.str().c_str());
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
