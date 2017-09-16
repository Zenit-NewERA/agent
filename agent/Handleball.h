/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Handleball.h
 *
 *    AUTHOR     : Sergei Serebryakov
 *
 *    $Revision: 2.10 $
 *
 *    $Id: Handleball.h,v 2.10 2004/06/27 10:48:00 anton Exp $
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
#ifndef __HANDLEBALL_H
#define __HANDLEBALL_H
//-------------------------------------------------------------------------------------------------
#include "shoot.h"
#include "pass.h"
#include "ClearBall.h"
#include "cross.h"
#include <list>
#include "behave.h"
#include "Interception.h"
#include "dribble.h"
#include <sstream>
//-------------------------------------------------------------------------------------------------
typedef enum REGIONS {
	Reg1=0,
	Reg2,
	Reg3,
	Reg4,
	Reg5,
	Reg6,
	Reg7,		
	Reg8,
	Reg9,
	Reg10,
	Reg11,
	Reg12		
} RegionsID;
//-------------------------------------------------------------------------------------------------
using namespace PassSkills;
class Regions {
public:
	Regions() { reversed=false; };
 ~Regions() {};
		 
	void Init();
	int  GetBallRegionID(Vector ballPos);
	int  GetVectorRegionID(Vector pos);
	
	void SetReversed(bool val=true)		{ reversed=val; };
private:
	Rectangle regions[12];
	bool reversed;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class Handleball : 	public PassSkills::newPass::Pass,
			public ShootSkills::Shoot,
			public ClearBall,
			public Cross {
public:
  Handleball();				
 ~Handleball() { if( dribble!=0 ) delete dribble; };
  bool  Initialize();
		 
  bool  Handle();
  bool  ShootDecision();
	void 	PassDecision();
private:
  void PlayInArea1();
	void PlayInArea2();
	void PlayInArea3();
	void PlayInArea4();	//our penalty area
	void PlayInArea5();
	void PlayInArea6();	
	void PlayInArea7();	//their penalty area

	bool DribbleForwardInOurPenaltyArea(); //checks and executes

	Time lastHbAction,
     	 startHbAction;

	Regions  regions;
	Dribble* dribble;
	int      ballRegionID;
	AngleDeg crossAng;
	float    myCongestion;

	//separated passes
	PassInfo* mostPossibleShooter;
	PassInfo* oneKickPassConf;
	PassInfo* oneKickPassCong;
	//direct forward passes
	PassInfo* passMaxConfidence[12];
	PassInfo* passMinCongestion[12];
	PassInfo* passMaxPriority[12];
	//direct through passes
	PassInfo* passThrMaxConfidence[12];
	PassInfo* passThrMinCongestion[12];
	PassInfo* passThrMaxPriority[12];
	//direct back passes
	PassInfo* backPassMaxConfidence[12];
	PassInfo* backPassMinCongestion[12];
	PassInfo* backPassMaxPriority[12];
	//through back passes
	PassInfo* backPassThrMaxConfidence[12];
	PassInfo* backPassThrMinCongestion[12];
	PassInfo* backPassThrMaxPriority[12];
	
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class Actions : public Handleball {
public:
  Actions() {};
  ~Actions() {};
  bool Initialize();
private:
};
extern Actions actions;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const AngleDeg WRONG_ANGLE  = 360.0f;
const float priorityHuge    = 100.0f;
const float priorityHight   = 1.0f;
const float priorityMedium  = 0.01f;
const float priorityLow     = 0.0001f;
const float basePriority    = 1.1f;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//structure of one request
struct Request{
  AngleDeg    angle;     //global angle
  float       priority;

  ObjectType type;
  Unum       unum;
  Vector     position;
	bool operator==(const Request& r) const { return angle==r.angle&&priority==r.priority&&type==r.type&&unum==r.unum&&position==r.position; };
	void operator=(const Request& r) { angle=r.angle;priority=r.priority;type=r.type;unum=r.unum;position=r.position; };
  void Log(int logLevel,bool canSee);
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
extern AngleDeg ballAngleBuffer;
extern AngleDeg playerAngleBuffer;
extern AngleDeg positionAngleBuffer;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetBuffersByDefault();
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetMediumBuffers();
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SetHugeBuffers();
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
typedef list<Request> ListOfRequests;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct ObjectInfo {
  bool        valid;
  bool        validWrtId;
  float       priority;
  Vwidth      vWidth;
  RequestType rType;
  bool        lastPosKnown;
  Vector      lastPos;
  float       lastPosValid;
  AngleDeg    buffAngle;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct Position {
  Vector      position;
  float       priority;
  Vwidth      vWidth;
  RequestType rType;
  bool        validWrtId;
  float       lastPosValid;
  bool operator==(const Position& p) const {
    return position==p.position&&priority==p.priority&&vWidth==p.vWidth&&rType==p.rType&&
      lastPosValid==p.lastPosValid&&validWrtId==p.validWrtId;
  };
  void operator=(const Position& p);
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
typedef list<Position> ListOfPositions;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RemoveIfNotValidWrtId {
public:
  RemoveIfNotValidWrtId();
  ~RemoveIfNotValidWrtId();
  bool operator() (class Position& position) { return !position.validWrtId; };
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct ValuedPosition{
  Vector position;
  float  conf;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LowConfidence {
public:
  LowConfidence(float threshold) { this->threshold=threshold; };
  ~LowConfidence() {};
  bool operator() (const ValuedPosition& position) const {
    return position.conf<threshold;
  }
private:
  float threshold;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
typedef list<ValuedPosition> ListOfValuedPositions;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct MPObject {//Most Prioritized Object
  ObjectType type;
  Unum       unum;
  Position   position;
  bool operator==(const MPObject& object) const;
  void operator=(const MPObject& object);
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class CanNotSee{
public:
	CanNotSee( AngleDeg angle, AngleDeg bodyAngle ){ maxAngle=angle;this->bodyAngle=bodyAngle; };
 ~CanNotSee() {};
	bool operator() (const Request& request) const {
		return GetDiff(bodyAngle,request.angle)>maxAngle;
	}
private:
	AngleDeg maxAngle;
  AngleDeg bodyAngle;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LessAngle {
public:
  LessAngle()	{};
  ~LessAngle()	{};
  bool operator() (const Request& r1, const Request& r2) const {
    return r1.angle<r2.angle;
  }
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HightConfOrBadId {
public:
  HightConfOrBadId(float basePriority) { this->basePriority=basePriority; };
  ~HightConfOrBadId() {};
  bool operator() (const Position& position) const {
    return (position.rType!=RT_MustSee && position.priority<=basePriority) || (position.validWrtId==false);
  }
private:
  float basePriority;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct IdInfo {
  ObjectType type;
  Unum       unum;
  Vector     position;
  float      conf;
  float      threshold;
  int        id;
  int        counter;
  int        counterLimit;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LowConfOrThresholdIsPassed {
public:
  LowConfOrThresholdIsPassed () {};
  ~LowConfOrThresholdIsPassed () {};
  bool operator() (class IdInfo& info) const { return info.conf<0.5 || info.conf<info.threshold || info.counter>=info.counterLimit; };
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
typedef list<IdInfo> ListOfIdInfos;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const int numConfAngles=360;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class VisualControl {
public:
  VisualControl();      
  ~VisualControl();     

  void   Observe();
  void   SearchBall();    

  void   AddPosition(Vector position, int cyclesFactor, int id=-1, RequestType rType=RT_WithoutTurn, Vwidth vWidth=VW_Unknown);
  void   AddTeammate(Unum teammate, int cyclesFactor,  RequestType rType=RT_WithoutTurn,Vwidth vWidth=VW_Unknown,int id=-1);   
  void   AddOpponent(Unum opponent, int cyclesFactor, RequestType rType=RT_WithoutTurn,Vwidth vWidth=VW_Unknown,int id=-1);   
  void   AddBall(int cyclesFactor, RequestType rType=RT_WithoutTurn, Vwidth vWidth=VW_Unknown,int id=-1);

  void   SetViewWidth(Vwidth vWidth)     { this->vWidth = vWidth; };  
  void   SetPositionDecay(float decay)   { positionConfDecay=decay; };
  void   SetPositionThreshold(float thr) { positionThreshold=thr; };
  void   SetDistThreshold(float thr)     { distThreshold=thr; };
  Vwidth GetBestViewWidth();
  float  AngleConf(AngleDeg angle);                       
	float  AngleDecay()										 { return angleConfDecay; };

  void   UpdateConfs(Time time, bool newSight);
private:
  bool     TurnNeck();        
  void     SetBufferAngles();
  void     AdjustViewWidth(MPObject& object);
  MPObject GetMostPrioritizedObject(RequestType rType);
  bool     IsIdActive(int id);
  void     SetBodyTurningAngles();         
  void     SetViewWidth(MPObject& object);
  void     SetViewWidth(MPObject& object,Vwidth vWidth);
  void     AddAngle(AngleDeg angle, float priority,ObjectType type);   
  void     AddAngle(AngleDeg angle, float priority,ObjectType type,Unum unum);   
  void     AddAngle(AngleDeg angle, float priority,ObjectType type,Vector position);   
  void     RemoveObject(MPObject& object);

  AngleDeg GetGlobalAngle(AngleDeg relAngle) { return GetNormalizeAngleDeg(bodyNewAngle+relAngle); };
	AngleDeg GetRelativeAngle(AngleDeg globalAngle) { return GetNormalizeAngleDeg(globalAngle-bodyNewAngle); };
  AngleDeg GetAngleForNeck();
  AngleDeg GetViewAngle(Vwidth view_width);                         

  float    GetPlayerPriority(int numOfCycles, Unum num, bool isOpp);     
  float    GetBallPriority(int numOfCycles);                             
  float    GetPositionPriority(Vector position,int cyclesFactor);          
  float    PositionValid(Vector position);

  bool     CanSeeAngle(AngleDeg globalAngle);         
  bool     CanSeePosition(Vector targetPosition);     
  bool     CanSeeBall();                             
  bool     CanSeeTeammate(int unum);                 
  bool     CanSeeOpponent(int unum);                 
  bool     CanSeeObject(MPObject& object);
  float    FilterListOfRequests(ListOfRequests& list);
	bool     IsRequestValid(const ListOfRequests& list, const Request& request);
  bool     DidNotSee(MPObject& object);

  AngleDeg AdjustNeckAngle(AngleDeg globalAngle);     
  AngleDeg AdjustViewAngle(AngleDeg globalAngle, AngleDeg deltaAngle);     

  bool     CanHandleAddedObjects();
  bool     CanHandleObjectsThatMustSee();
  bool     CanHandleObjectsThatMusntSee();

  void     AddObjectsToList(int filter, MPObject& object);
  void     AddTeammateToList(int teammate, float priority, float priorityFactor);    
  void     AddOpponentToList(int opponent, float priority, float priorityFactor);
  void     AddBallToList(float priority, float priorityFactor);
  void     AddPositionToList(Vector position, float priority, float priorityFactor); 

  void     WideLook();                  
  bool     PassLook();
  bool     OffSideLook();
  bool     TheirGoalieLook();
  bool     BallLook();
  void     SimpleLook();

  void     Log(int logLevel=10);    
  void     LogPlayer(int logLevel, int id, bool isOpp);   
  void     LogBall(int logLevel);                           
  void     LogPosition(int logLevel, Position& position);     
  void     LogMPObject(int logLevel,MPObject& object);

  int      PlayerBufferCycles(float posValid);
  int      BallBufferCycles(float posValid);
private:
	bool mustRequestsExist;
	bool otherRequestsExist;

  ListOfRequests        requests;
  ListOfPositions       positions;
  ListOfValuedPositions valuedPositions;
  ListOfIdInfos         idInfos;

  ObjectInfo   opponents[11];
  ObjectInfo   teammates[11];
  ObjectInfo   ball;

  MPObject lastMPObject;
  Vwidth   vWidth;
  AngleDeg viewAngle;
  Time     lastObservationTime;
  AngleDeg bodyTurnAngle;
  AngleDeg bodyNewAngle;
  bool     considerBodyTurn;
  Time     lastSearchBallTime;
  int      searchBallDirection;
  Vector   myPosition;
  float    positionConfDecay;
  float    positionThreshold;
  float    distThreshold;
  Time     positionsConfUpdateTime;
  float    anglesConf[numConfAngles];
  Time     anglesConfUpdateTime;
  float    angleConfDecay;
  bool     correction;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
extern VisualControl eye;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#endif
