/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : kick.h
 *
 *    AUTHOR     : Sergey Serebryakov
 *
 *    $Revision: 2.5 $
 *
 *    $Id: kick.h,v 2.5 2004/06/22 17:06:16 anton Exp $
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


#ifndef _KICK_H_
#define _KICK_H_

#include "client.h"
#include <vector>
using namespace std;

extern bool consider_posvalid;

typedef enum SKMODE {		//SKMODE - Smart Kick Mode
  SK_ModeUnknown,
  SK_Fast,
  SK_Safe,
  SK_SetPlay
} SK_Mode;

typedef enum SKRES {		//SKRES - Smart Kick Result
  SK_KickInProgress,
  SK_BallStoped,
  SK_BallAdjusted,
  SK_DidNothing,
  SK_KickDone,
  SK_LostBall
} SK_Res;


class Kick {
public:
  Kick()	{
    last_kick_time=-1;
  };
  ~Kick()	{};
public:
  //player movement correction is needed, when things, like turnball are called
  void  PlayerMovementCorrection(float& kickspeed, AngleDeg& kickangle);
  //kick power corection needs at every kick,as actual power of the kick
  //depends on relative to player ball position
  float KickPowerCorrection(float power, Vector ballpos = Mem->BallAbsolutePosition());
  float kickCorrectionCoeff(Vector ballpos = Mem->BallAbsolutePosition());
  //next method is used to calculate noise factor.How to use it,
  //unfortunatly,i do not know yet:-(
  float kickMaxNoise(float kickpower);
  AngleDeg GetKickAngle(Vector target) { return Mem->AngleToFromBody(Mem->MyPos()+target-Mem->BallAbsolutePosition()); };

public:
  //next kickball functions should not be used as a real kick ball function,
  //they just represent some lower level skills in out agent kick model
  float kickball(float kickspeed, AngleDeg kickangle, TurnKickCommand& command);

  SK_Res smartkick(float kickspeed, AngleDeg kickangle, SK_Mode sk_mode,bool only_calc=false);
  SK_Res smartkick(float kickspeed, Vector kicktarget, SK_Mode sk_mode,bool only_calc=false);

  //this func does the same as upper but angle is global
  SK_Res smartkickg(float kickspeed, AngleDeg kickangle, SK_Mode sk_mode) {
    return smartkick(kickspeed, kickangle-Mem->MyBodyAng(), sk_mode);
  };

  //just for convinient
  SK_Res smartkick() {
    return smartkick(GetSpeed(), GetAngle(), GetMode());
  };	
		
  bool accelerate_ball(AngleDeg kickangle, Vector& position);
	
  SK_Res smartkick_fast(float kickspeed, AngleDeg kickangle, TurnKickCommand& command,bool only_calc);
  SK_Res smartkick_safe(float kickspeed, AngleDeg kickangle, TurnKickCommand& command,bool only_calc);
  SK_Res smartkick_setplay(float kickspeed, AngleDeg kickangle, TurnKickCommand& command);

  //next funcs turn balls around player,see implementations for details
  SK_Res turnball(AngleDeg angle, TurnDir dir, TurnKickCommand& command, SK_Mode sk_mode, float tb_radius = Mem->GetMyOptCtrlDist());
  SK_Res turnball(AngleDeg angle, TurnDir dir, SK_Mode sk_mode, float tb_radius = Mem->GetMyOptCtrlDist());

  bool  stopball();
  float stopball(TurnKickCommand& command);

  bool moveball(Vector pos);
  bool moveball(Vector pos, TurnKickCommand& command);

public:
  bool ExecuteKickCommand(TurnKickCommand& command);
  void SetKickData(float kickspeed, AngleDeg kickangle, SK_Mode mode);
  bool KickInProgress();
public:
  int get_kick_cycles(float kickspeed, AngleDeg kickangle);
  float maxspeed_at1kick(AngleDeg kickangle);
  bool can_kickout(float kickspeed, AngleDeg kickangle);

  AngleDeg get_adjust_angle(AngleDeg kickangle);
public:
  float 		GetSpeed()			{ return speed; };
  AngleDeg    GetAngle()			{ return angle; };
  Vector 		GetTarget()			{ return kicktarget; };
  SK_Mode		GetMode()			{ return sk_mode; }
  Time		GetLastKickTime()	{ return last_kick_time; };
public:
  float 		speed;
  AngleDeg 	angle;
  Vector		kicktarget;
  Time 		last_kick_time;
  SK_Mode		sk_mode;
  SK_Res		sk_result;
};
//----------------------------------------------------

//----------------------------------------------------
AngleDeg GetNormalize360(AngleDeg angle);
AngleDeg GetAngleDiff(AngleDeg angle1, AngleDeg angle2);
//----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
class Cell {
public:
  Cell()	{
    Unlock();
    SetPos(Vector(0.0f, 0.0f));
  };
  ~Cell()	{};
public:
  bool Locked()					{ return locked; };
  bool NotLocked()				{ return !locked; };

  Vector Position() 				{ return position; };

  void Lock()						{ locked = true; };
  void Unlock()					{ locked = false; };
  void SetPos(Vector pos)			{ position = pos; };

  void Rotate(AngleDeg angle)		{ position.rotate(angle); };
  void Move(Vector dir)			{ position+=dir; };

  float GetSpeed()				{ return maxspeed; };

  //only for testing purposes
  void SetSpeed(float speed)		{ maxspeed=speed; };
private:
  bool 	locked;
  Vector 	position;
  float 	maxspeed;
};
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
class Grid {
public:
  Grid()	{
    all_unlocked = false;
    time=-1;
  };
  ~Grid()	{};
public:
  void Initialize();

  void Move(Vector dir);
  void Rotate(AngleDeg angle);

  bool UnLocked()			{ return time==Mem->CurrentTime && all_unlocked; };
  bool Locked()			{ return !UnLocked(); };

	
  void SetGridinfo();
public:
	
public:
  bool CellLocked(int circle, int angle)	{ return cells[circle][angle].Locked(); };

  int GetCircles()		{ return int(CIRCLES); };
  int GetAngles()			{ return int(ANGLES); };


public:
  enum {
    CIRCLES = 4,
    ANGLES = 25
  };

  Cell cells[CIRCLES][ANGLES];
	
  bool all_unlocked;
  Time time;
};
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
class GridKick : public Kick {
public:
  GridKick()	{};
  ~GridKick()	{};
public:
  bool GetKick(Vector& position, AngleDeg angle, float speed);	
  bool can_move_to_pos(Vector targetpos);
  SK_Res gridkick(float speed, AngleDeg angle, SK_Mode mode);
public:
  Grid grid;	
};
//-----------------------------------------------------
template<typename Class> extern
void  check_bounds(Class& x, Class min, Class max);
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
bool 	kick_to_corner(void);
double 	kick_to_corner_power(void);
bool 	kick_to_corner_at_begin(void);

Bool 	go_to_static_ball(float kick_ang);

inline Bool go_to_static_ball(Vector pt) {
  return go_to_static_ball( (pt - Mem->BallAbsolutePosition()).dir());
}

ActionQueueRes go_to_point_with_speed(Vector p, float velocity = 0.9f, float buffer = 0.0f, DodgeType dodge = DT_all);
int BallPredictedCyclesToPoint(Vector target, float ball_vel, int cycles_buffer=3);
int OpponentPredictedCyclesToIntercept(Vector target, float ball_vel, Unum opponent);
int TeammatePredictedCyclesToIntercept(Vector target, float ball_vel, Unum teammate);
int PredictedCyclesToIntercept( Vector from, Vector to, float ball_vel, Unum opponent);
template<typename A,typename B> void Sort(A* elements,B* keys,int size);
//template<typename A> void Swap(A& element1,A& element2);
void Sort( float* elements,int* keys,int size );

  
////////////////////////////////////////////////////
////////////////////////////////////////////////////
namespace Visualization {

  const int COLOR_NAME_MAX=7;
  const int MAX_MESSAGE_LENGTH=4096;
  const int MAX_STRING_LENGTH=200;

  class Point {
  public:
    static int Size() {
      return sizeof(float)*2+sizeof(char)*COLOR_NAME_MAX;
    };
    Point()	{ x=y=0;strcpy(color,"\0"); };
    Point( float x, float y, char color[COLOR_NAME_MAX]) {
      this->x=x;
      this->y=y;
      strcpy(this->color, color);
    };

    float x;
    float y;
    char color[COLOR_NAME_MAX];
  };

  class Circle {
  public:
    static int Size() {
      return sizeof(float)*3+sizeof(char)*COLOR_NAME_MAX+sizeof(bool);
    };
    Circle() { x=y=r=0;filled=false;strcpy(color,"\0"); };
    Circle( float x, float y, float r, bool filled, char color[COLOR_NAME_MAX] ) {
      this->x=x;
      this->y=y;
      this->r=r;
      this->filled=filled;
      strcpy(this->color, color);
    };

    float x;
    float y;
    float r;
    bool filled;
    char color[COLOR_NAME_MAX];	
  };

  class Line {
  public:
    static int Size() {
      return sizeof(float)*4+sizeof(char)*COLOR_NAME_MAX;
    };
    Line() { x1=y1=x2=y2=0;strcpy(color,"\0"); };
    Line( float x1, float y1, float x2, float y2, char color[COLOR_NAME_MAX] ) {
      this->x1=x1;
      this->y1=y1;
      this->x2=x2;
      this->y2=y2;
      strcpy(this->color, color);
    }

    float x1;
    float y1;
    float x2;
    float y2;
    char color[COLOR_NAME_MAX];		
  };

  class String {
  public:
    static int Size() {
      return sizeof(float)*2+sizeof(char)*(COLOR_NAME_MAX+MAX_STRING_LENGTH);
    };
    String() { x=y=0;strcpy(string,"\0");strcpy(color,"\0"); };
    String(float x, float y, char string[MAX_STRING_LENGTH], char color[COLOR_NAME_MAX] ){
      this->x=x;
      this->y=y;
      strcpy(this->string, string);
      strcpy(this->color, color);
    };

    float x;
    float y;
    char string[MAX_STRING_LENGTH];
    char color[COLOR_NAME_MAX];	
  };

  typedef vector<Line>		VectorOfLines;
  typedef vector<Circle>	VectorOfCircles;
  typedef vector<Point> 	VectorOfPoints;
  typedef vector<String> 	VectorOfStrings;

  class DynamicInfo {
  public:
    static int   Size(){
      return sizeof(char)+sizeof(bool)+sizeof(float)*6;
    };
    char  side;
    bool  goalie;
    float posValid;
    float x;
    float y;

    float velValid;
    float dx;
    float dy;
  };

  class WorldModel  {
  public:
    int Size() {
      return 23*DynamicInfo::Size()+lines.size()*Line::Size()+circles.size()*Circle::Size()+points.size()*Point::Size()+strings.size()*String::Size();
    }

    void ResetDynamicInfo();
    void ClearGraphicInfo();

    void AddLine(		Line line			)	{ lines.push_back(line); 			};
    void AddCircle(	Circle circle	)	{ circles.push_back(circle);	};
    void AddPoint(	Point point		)	{ points.push_back(point); 		};
    void AddString(	String string	)	{ strings.push_back(string);	};


    DynamicInfo& Ball()							{ return ball; 						};
    DynamicInfo& Teammate(Unum num)	{ return teammates[num]; 	};
    DynamicInfo& Opponent(Unum num)	{ return opponents[num]; 	};

    VectorOfLines& 	 Lines()			{ return lines; 	};
    VectorOfCircles& Circles()		{ return circles; };
    VectorOfPoints&	 Points()			{ return points; 	};
    VectorOfStrings& Strings()		{ return strings; };

    Line& 	GetLine(int id)		{ return lines[id]; 	};
    Circle& GetCircle(int id)	{ return circles[id]; };
    Point& 	GetPoint(int id)		{ return points[id]; 	};
    String& GetString(int id)	{ return strings[id]; };
  private:
    DynamicInfo ball;
    DynamicInfo teammates[11];
    DynamicInfo opponents[11];

    VectorOfLines 	lines;
    VectorOfCircles circles;
    VectorOfPoints 	points;
    VectorOfStrings strings;
  };


#define USE_AGENT_LIBRARY

  class Graphics  {
  public:
    Graphics()	{
      sendWorldModel=true;
      sendGraphicCommands=true;
    };
    ~Graphics()	{};

    void AddLine(Line line) { worldModel.AddLine(line); };
    void AddLine( float x1, float y1, float x2, float y2, char color[COLOR_NAME_MAX] ) {
      AddLine( Line(x1,y1,x2,y2, color) );
    };

    void AddPoint(Point point) { worldModel.AddPoint(point); };
    void AddPoint(float x, float y, char color[COLOR_NAME_MAX]) {
      AddPoint( Point(x,y,color) );
    };

    void AddCircle(Circle circle) { worldModel.AddCircle(circle); };
    void AddCircle(float x, float y,float r, char color[COLOR_NAME_MAX], bool filled=false) {
      AddCircle( Circle(x,y,r,filled,color) );
    };

    void AddString( String string ) { worldModel.AddString(string); };

#if defined USE_AGENT_LIBRARY
    void AddLine(Vector pos1, Vector pos2, char color[COLOR_NAME_MAX]) {
      AddLine( Line(pos1.x, pos1.y, pos2.x, pos2.y, color) );
    };
    void AddLine(Ray ray, char color[COLOR_NAME_MAX]) {
      AddLine(ray.Origin(), Mem->FieldRectangle.RayIntersection(ray), color);
    };
    void AddLine(Vector origin, AngleDeg angle, char color[COLOR_NAME_MAX]) {
      AddLine(Ray(origin, angle), color);
    };

    void AddPoint(Vector pos, char color[COLOR_NAME_MAX]) {
      AddPoint( Point(pos.x, pos.y, color) );
    };

    void AddCircle(Vector center,float r, char color[COLOR_NAME_MAX], bool filled=false) {
      AddCircle( Circle(center.x, center.y, r, filled, color) );
    };

    void CheckCoordinates();
#endif
		
    void Draw();

    void SetSendWorldModel()		{ sendWorldModel=true; 	};
    void DeleteSendWorldModel()	{ sendWorldModel=false; };

    void SetSendGrapgicCommands()			{ sendGraphicCommands=true; 	};
    void DeleteSendGraphicCommands()	{ sendGraphicCommands=false; };
  protected:
    void LineString(char* msg, Line& line);
    void CircleString(char* msg, Circle& circle);
    void PointString(char* msg, Point& point);
    void StringString(char* msg, String& string);

    WorldModel worldModel;
  private:
#if defined USE_MONITOR_LIBRARY
    void SendToMonitor(const char* msg);
#endif

    bool sendWorldModel;
    bool sendGraphicCommands;
  };

  class SmartGraphics : public Graphics {
  public:
    SmartGraphics();
    ~SmartGraphics() {};

#if defined USE_AGENT_LIBRARY
    void InitializeColors();
	
    void AddBallPos() ;
    void AddBallVel();
    void AddBallPosAndVel();
    void AddBallRoute();
    void AddBallPosAndVelAndRoute();

    void AddTeammatePos(Unum num);
    void AddTeammatePosAndVel(Unum num);
    void AddTeammateRoute(Unum num);											

    void AddOpponentPos(Unum num);
    void AddOpponentPosAndVel(Unum num);

    void AddArrow(Vector begin, Vector end, char color[COLOR_NAME_MAX]);	
#endif
    void ConstructGraphicFromWorldModel();
  private:
    char  text[MAX_STRING_LENGTH];
    char  textColor[COLOR_NAME_MAX];
    float arrowCircleRadius;
	
    float ballInnerRadius;
    float ballOuterRadius;
    char  ballColor[COLOR_NAME_MAX];
    char  ballVelColor[COLOR_NAME_MAX];
    char  ballRouteColor[COLOR_NAME_MAX];

    float playerInnerRadius;
    float playerOuterRadius;

    char  leftteamColor[COLOR_NAME_MAX];
    char  leftteamGoalieColor[COLOR_NAME_MAX];
    char  leftteamVelColor[COLOR_NAME_MAX];
    char  leftteamRouteColor[COLOR_NAME_MAX];	

    char  rightteamColor[COLOR_NAME_MAX];
    char  rightteamGoalieColor[COLOR_NAME_MAX];
    char  rightteamVelColor[COLOR_NAME_MAX];
    char  rightteamRouteColor[COLOR_NAME_MAX];	

    char black[COLOR_NAME_MAX];
  };


  typedef enum LOG_MODE {
    LM_Save,
    LM_Load,
    LM_Unknown
  } LogMode;

  class GraphicLogSystem : public SmartGraphics {
  public:
    GraphicLogSystem();
    ~GraphicLogSystem() { if( file!=0 ) fclose(file); };

    void Initialize(LogMode logMode, const char* fileName);

    void BeginCycle();
    void EndCycle(int time);

  private:
    bool GetDataForTime(int time);
#if defined USE_AGENT_LIBRARY
    void WriteWorldModel();
    void UpdateWorldModelByAgent();
#endif
    FILE* 		file;
    int 			last_access_time;
    LogMode 	logMode;
    bool 			doLogWorldModel;
    bool 			doLogGraphicCommands;
    bool			initialized;
  };

}

extern Visualization::GraphicLogSystem logGr;
////////////////////////////////////////////////////
////////////////////////////////////////////////////



#endif
