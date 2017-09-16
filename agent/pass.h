/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : pass.h
 *
 *    AUTHOR     : Sergey Serebyakov
 *
 *    $Revision: 2.6 $
 *
 *    $Id: pass.h,v 2.6 2004/06/22 17:06:16 anton Exp $
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
#ifndef __PASS_H
#define __PASS_H

#include "client.h"
#include "kick.h"
#include "behave.h"
#include "Processor.h"
#include "Interception.h"
#include <sstream>
#include <iomanip>
#include <vector>




/////////////////
#define PASSTEST
/////////////////
#ifdef PASSTEST
#define DEBUG(x) x;
#else
#define DEBUG(x)
#endif
/////////////////
#define makechar(x) const_cast<char*>(x.str().c_str())
/////////////////
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
namespace PassSkills {
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
  class PassInfo {
  private:
    Unum 		receiver;
    Vector	passPos;
    float 	speed;			  //min pass speed
    float		receiverCyc;	//min receiver cycles
    Unum 		opponent;		  //most danger opponent
    float		opponentCyc;	//min opponent cycles
    float		conf;
    Time		time;
		float   priority;
		float   congestion;
  public:
		PassInfo();
   ~PassInfo() {};
			 
    Unum 		GetReceiver()			  const	{ return receiver; 		};
    Vector 	GetPassPos()	  		const	{ return passPos; 		};
    float 	GetSpeed()				  const	{ return speed; 			};
    float		GetReceiverCycles()	const	{ return receiverCyc; };
    Unum		GetOpponent()		    const { return opponent; 		};
    float		GetOpponentCycles()	const	{ return opponentCyc; };
    float 	GetConfidence()			const	{ return conf; 			  };
    Time 		GetTime()						const	{ return time; 				};
		float   GetPriority()			  const { return priority;    };
		float		GetCongestion()			const { return congestion;  };
  
    void SetReceiver(Unum receiver) 					{ this->receiver = receiver; 			 };
    void SetReceiverCycles(float receiverCyc)	{ this->receiverCyc = receiverCyc; };
    void SetPassPos(Vector passPos)						{ this->passPos = passPos; 				 };	
    void SetSpeed(float speed)         				{ this->speed = speed; 						 };
    void SetOpponent(Unum opponent)		     		{ this->opponent = opponent; 			 };
    void SetOpponentCycles(float opponentCyc)	{ this->opponentCyc = opponentCyc; };
    void SetConfidence(float conf)					  { this->conf = conf; 							 };
    void SetTime(Time time) 						  	  { this->time = time; 							 };
		void SetPriority(float priority)		  	  { this->priority = priority;			 };
		void SetCongestion(float congestion)			{ this->congestion = congestion;   };
		
    void SetInfo(Unum receiver,float rCycles,float conf,Vector passPos,float speed,Unum opp,float oCycles,float priority);
    bool Valid() { return time==Mem->CurrentTime&&receiver!=Unum_Unknown && speed>0 && speed<=Mem->SP_ball_speed_max; };
    void Clear();
    void Log(int level);
    void operator=(const PassInfo& info);
    bool operator<(const PassInfo& info) const	{ return priority<info.GetPriority(); };
    bool operator>(const PassInfo& info) const	{ return priority>info.GetPriority(); };
  };
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
struct Controler {
  //constant variables
  Unum   num;
  char   side;
  //changeable parameters
  Vector plPos;	//it can be adjusted to match the position valid value - defferent compensation can be given
  Vector prPoint;
  float  distToLine;
  //controling segment
  float  distFrom;
  float  distTo;
	//passSpeeds;
	float  minSpeed;
	float  maxSpeed;
	
  bool  Teammate()	{ return side==Mem->MySide; };
  Controler(Unum num, char side) {
    this->num=num;
    this->side=side;
    plPos=Mem->PlayerAbsolutePosition(side,num);
    prPoint=Vector(0,0);
    distToLine=distFrom=distTo=0.0f;
		minSpeed=maxSpeed=1.0f;
  };
  Controler() {
  	this->num=Unum_Unknown;
    this->side='?';
    plPos=prPoint=Vector(0,0);
    distToLine=distFrom=distTo=0.0f;
		minSpeed=maxSpeed=1.0f;
  };
};
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
typedef vector<Controler>::iterator ControlerIter;
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
  const int X=5;
  const int Y=4;
  //-------------------------------------------------------------------------------------------------------------
  class TransitionPriorities {
  public:
    TransitionPriorities()  {};
    ~TransitionPriorities() {};
		
    void  Initialize(float transitionPriorities[Y][X]);
    float GetPriority(int xid, int yid) { return transitionPriorities[yid][xid]; };
  private:
    float  transitionPriorities[Y][X];
  };
  //-------------------------------------------------------------------------------------------------------------
  class PassPriorities {
  public:
    void Initialize();

    void   InitBallPos(Vector ballPos);
    float  GetPriority(Vector pos);
  private:
    TransitionPriorities 	transitions[2][5];		
    bool   	reverse;
    int		ballX;
    int		ballY;
    float	maxDist;
  };
  //-------------------------------------------------------------------------------------------------------------
  class Pass : public virtual Kick {
  public:
    Pass()  { 	pass_threshold=0.52f;
    max_pass_distance=30.0f;
    min_pos_valid=0.94f; 
    };
    virtual ~Pass()	 {};
    void    Initialize() { priorities.Initialize(); };  	
  public:
    float    PassConf(Unum teammate,PassInfo* pInfo=0);
    PassInfo GetPassInfo(Unum teammate);
    Unum	 BestPass();

    void	 pass(Unum teammate);
    void     pass(AngleDeg angle, float speed, Unum teammate);
    void	 pass(PassInfo& info);

    //communication stuff
    void GetPassMessage(char* msg, sayType st, Unum teammate=Unum_Unknown);
    void ParsePassMessage(char* msg, sayType st, Unum from, Time t);	

    float PassFromPoint2Point(Vector from, Vector to, Unum& opponent);
    //shortcuts to private params
    float GetMaxPassDistance() { return max_pass_distance; };
    float MinPosValid()				 { return min_pos_valid; };
    float PassThreshold()			 { return pass_threshold; };
  public:
    PassInfo explore_pass_to_teammate(Unum teammate,bool super_kick=false);
    ////////////////////////////////////////////////////////
    void 	SetPassRoutes(     bool doLog=false );
    bool	ExplorePassRoutes( bool doLog=false );
    float	GetPassPriority(PassInfo& info);
    vector<PassInfo>& GetPlayerInfos(int player) { return passInfos[player-1]; };
	
    void    Test();
    ////////////////////////////////////////////////////////
  private:
    ////////////////////////////////////////////////////////
    int		  GetPassInformation( bool doLog=false );
    bool      SetControlers(vector<Controler>& controlers, unsigned int angleID);
    bool	  PlayerValid(Ray& ray, Controler& controler);
    Vector	  PlayerPosition(Unum num, char side, unsigned int angleID);

    Unum	  FastestOppToPosition(Vector position);
    bool	  IsLastControler(Controler* controler, unsigned int angleID);
    void	  AddPassInfo(Unum player, PassInfo& info) { passInfos[player-1].push_back(info); };
    int 	  NumPassInfos();
	
    int		  NumCyclesToPosition(Controler* controler, Vector position);
    //priority stuff	
    float	GetCongestionPriority(Vector pos, Unum pl, float r=8.0f);
    float	GetCongestionPriority(PassInfo& info, float r=8.0f) { return GetCongestionPriority(info.GetPassPos(),info.GetReceiver(),r); };
	
    float   GetTransitionPriority(Vector pos)					{ return priorities.GetPriority(pos); };
    float   GetTransitionPriority(PassInfo& info)				{ return priorities.GetPriority(info.GetPassPos()); };
	
    float   GetPositioningPriority(PassInfo& info);
    ////////////////////////////////////////////////////////

    float    pass_threshold;
    float    max_pass_distance;
    float    min_pos_valid;
	
    PassPriorities priorities;

    PassInfo dpasses[11];

    vector<Controler>	 		teammates;
    vector<Controler>	 		opponents;
    vector<AngleDeg> 	 		passAngles;

    Controler 					self;
    vector< vector<Controler> > allControlers;
    vector<PassInfo>			passInfos[11];
  };

namespace newPass {
		
const int NUMBER_OF_STEPS = 50;			
class Pass:public virtual Kick {
public:
	Pass();
 ~Pass();
  void     Initialize();
	void	 	 FindPasses();
	std::vector<PassInfo>& Passes() 				   { return allPassInfos; 			};
	std::vector<PassInfo>& ThroughPasses() 	   { return throughPassInfos; 	};
	std::vector<PassInfo>& BackPasses()				 { return backPasses; 			  };	
	std::vector<PassInfo>& BackThroughPasses() { return backThroughPasses;  };
	PassInfo&							 LastHopePass()			 { return lastHopePass; };
	
	float 	 PassThreshold()		         			const { return passThreshold;        	 			  };
	float 	 MinPassDistance()          			const { return minPassDistance;         			};
	float 	 GetMaxPassDistance()        			const { return maxPassDistance;         			};	
	float 	 PlayerValidThreshold()     		  const { return playerValidThreshold;     		  };
	float 	 BaseAngleValidThreshold()  			const { return baseAngleValidThreshold;       };
	float 	 AdditionalAngleValidThreshold() 	const { return additionalAngleValidThreshold; };
	AngleDeg BufferAngle()										const { return bufferAngle; 									};
	float    MaxBallEndSpeedSlow()			  		const { return maxBallEndSpeedSlow;						};
	float    MaxBallEndSpeedFast()			  		const { return maxBallEndSpeedFast;						};	
	
	void  SetPassThreshold(float value)        					{ passThreshold=value;   							 };
	void  SetMinPassDistance(float value)      					{ minPassDistance=value; 							 };
	void  SetMaxPassDistance(float value)      					{ maxPassDistance=value; 							 };
	void  SetPlayerValidThreshold(float value) 					{ playerValidThreshold=value; 				 };
	void  SetBaseAngleValidThreshold(float value) 			{ baseAngleValidThreshold=value; 			 };
	void  SetAdditionalAngleValidThreshold(float value) { additionalAngleValidThreshold=value; };
	void  SetBufferAngle(AngleDeg value)								{ bufferAngle=value; 									 };
	void  SetMaxBallEndSpeedSlow(float value)					 	{ maxBallEndSpeedSlow=value; 					 };	
	void  SetMaxBallEndSpeedFast(float value)					 	{ maxBallEndSpeedFast=value; 					 };		
	
	void  ComputeBaseAngleValidThreshold(int cyclesThreshold);
	void  ComputeAdditionalAngleValidThreshold(int cyclesThreshold);
	
	void  pass(PassInfo& info);
	void  Test();
	float PassFromPoint2Point(Vector from, Vector to, Unum& opponent);
	
	inline float GetPassPriority(Vector passPos) { return 1.0f-passPos.dist(Vector(54.0,0))/108; };
				 float GetCongestionPriority(const Vector passPos);	
private:
	float  GetPassSpeed(Unum player, Vector passPos,float maxEndPassSpeed, int* cycles=0);
	float  GetMaxPassSpeed(Vector passPos, float maxEndPassSpeed);
	int    PredictPlayerCyclesToPosition(Unum player,char side, Vector position);


	inline bool  IsAnglePassable(AngleDeg globalAngle) const;  
				 bool  IsTeammatePassable(Unum tm) const;
				 bool  IsTeammatePassableInOurPenaltyArea(Unum tm) const;
				 bool  IsTeammateThatIsBackOfMePassable(Unum tm) const;
	inline bool  IsOpponentConsidered(Unum opp) const;
				 bool  SetPassableTeammatesAndConsideredOpponents();
				 float PassConfidence(float tmCycles, float oppCycles);

				 Unum  GetMostDangerOpponent(Vector passPos, float& cycles);
				 float ExplorePass(Unum receiver, Vector from, Vector passPos,float& passSpeed,int& tmCycles, Unum& danger, int& cycles);
				 bool  IsPositionPassable(Vector passPos) { return IsAnglePassable(Mem->AngleToGlobal(passPos)); };
				 void  ExploreDirectPasses();
				 void  ExlploreThroughtPasses();
				 float GetPassInformation(Vector from, Vector passPos, Unum teammate, PassInfo& info);
				 bool  GetPassPositions(Vector from, Vector origPos, std::vector<Vector>& positions);
				 void  AddThroughPassInfo(const PassInfo& info);
				 void  AddPass(const PassInfo& info);								
	float 	 passThreshold;
	float 	 minPassDistance;
	float 	 maxPassDistance;
	float 	 playerValidThreshold;
	float    baseAngleValidThreshold;
	float    additionalAngleValidThreshold;
	AngleDeg bufferAngle;
	float    maxBallEndSpeedSlow;
	float    maxBallEndSpeedFast;
	
	std::vector<Unum> 			opponents;
	InterceptionInput				oppInfo[11];
	std::vector<Unum> 			teammates;
	InterceptionInput				teamInfo[11];
	std::vector<PassInfo> 	allPassInfos;
	std::vector<PassInfo> 	throughPassInfos;	
	std::vector<PassInfo> 	backPasses;	
	std::vector<PassInfo> 	backThroughPasses;	
	PassInfo								lastHopePass;
	
	float*   ballDecays;
};

}

}
#endif	//__PASS_H
