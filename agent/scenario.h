/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : scenario.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.4 $
 *
 *    $Id: scenario.h,v 2.4 2004/03/26 09:55:13 anton Exp $
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
#ifndef SCENARIO_H
#define SCENARIO_H

#include "geometry.h"
#include "client.h"
#include "types.h"
#include <deque>
#include <FlexLexer.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include "Playposition.h"
/**
 *@author Anton Ivanov
 */
class Step;

namespace CurrentScenarioInfo{
  extern map<int,int> current_pl_map;
}
/**********************************************************************/
class Val{
public:
  enum Type{
    none=0,
    t_float=1,
    offside=2,//x coordinate of opp offside line
    ballx=3,
    bally=4,
    tm1x=10,tm2x,tm3x,tm4x,tm5x,tm6x,tm7x,tm8x,
    tm1y=20,tm2y,tm3y,tm4y,tm5y,tm6y,tm7y,tm8y
  };
  Val(){type=none;}
  Val(float v){val=v;type=t_float;}
  Val(Type t){type=t;}

  operator float()const;
private:
  Type type;
  float val;
};
/**********************************************************************/
class Point{
public:
  enum Type{
    number,
    ball,
    player,
    add_p
  };

  Point& operator=(const Point& p);
  Point(const Point& p){*this=p;}
  Point(Val v1=Val(Val::none),Val v2=Val(Val::none),Type t=number){val1=v1;val2=v2;type=t;}
  Point(bool s,Unum p){my_side=s;player_pos=p;type=player;}
  Point(Type t){type=t;}
  Point(Val v1,Val v2,const Point* point){add_point=new Point(*point);val1=v1;val2=v2;type=add_p;}
  ~Point(){if(type==add_p&&add_point!=0) delete add_point;}

  operator Vector()const;
private:
  Type type;
  Val val1,val2;
  Point* add_point;
  Unum player_pos;
  bool my_side;
};
/**********************************************************************/
class Region{
public:
  enum Type{
    point,
    rectangle,
    arc,
    reg,
    none
  };

  Region(){type=none;}
  Region(const Point& p){point1=p;type=point;}
  Region(const Point& p1,const Point& p2){point1=p1;point2=p2;type=rectangle;}
  Region(const Point& p, float r){point1=p;radius1=0.0f;radius2=r;ang1=-180.0f;ang2=180.0f;type=arc;}//make circle
  Region(const Point& p, float r1,float r2,float a1,float a2){point1=p;radius1=r1;radius2=r2;
  ang1=GetNormalizeAngleDeg(a1);ang2=GetNormalizeAngleDeg(a2);type=arc;}
  Region(const deque<Region>& region){region_set=region;type=reg;}

  bool IsIn(Point point)const{return IsIn(Vector(point));}
  bool IsIn(Vector point) const;

  Vector Center()const;
  
  bool IsValid()const{return type!=none;}
  void Clear(){region_set.clear();type=none;}
private:
  Type type;
  Point point1,point2;
  deque<Region> region_set;
  float radius1,radius2,ang1,ang2;
};
/**********************************************************************/
class PTSType{
public:
  void Add(Ptype pt){ptdata.push_back(pt);}
  operator Ptype() const;

  enum PSType{
    center,
    left,
    right,
    wingwb,
    wingnb,
    all
  };
  void Add(PSType ps){psdata.push_back(ps);}
  operator Pside() const;

  void Clear(){ptdata.clear();psdata.clear();}
  bool Empty()const{return ptdata.empty()||psdata.empty();}
private:
  deque<Ptype> ptdata;
  deque<PSType> psdata;
};
/**********************************************************************/
class InitTmType{
public:
  InitTmType(){fastest=false;}

  void SetPTSType(const PTSType& pt){PType=pt;}
  void SetRegion(const Region& r){reg=r;}
  void SetFastest(){fastest=true;}
  
  Unum GetNumber()const;

  void Clear(){fastest=false;PType.Clear();reg.Clear();}

private:
  bool fastest;
  PTSType PType;
  Region reg;
};
/**********************************************************************/
class Condition{
public:
  enum Type{
    t_bpos,
    t_ppos,
    t_bowner,
    t_and,
    t_or,
    t_not,
    t_ppos_plset,
    t_bowner_unum
  };

  Condition(){}
  Condition(const Region& r){reg=r;type=t_bpos;}
  Condition(const PTSType& ptype,int min,int max,const Region& r){pt=ptype;unum_min=min;this->max=max;reg=r;type=t_ppos;}
  Condition(const PTSType& ptype){pt=ptype;type=t_bowner;}
  Condition(const deque<Condition>& cs,Type t){cond_set=cs;type=t;}
  Condition(const Condition& cond,Type t){cond_set.push_back(cond);type=t;}
  Condition(bool team,const set<int>& ps,int min,int max,const Region& r){this->team=team;pl_set=ps;unum_min=min;this->max=max;reg=r;type=t_ppos_plset;}
  Condition(int unum){unum_min=unum;type=t_bowner_unum;}

  bool Check() const;
  char GetTeam(bool our_team) const{if(our_team) return Mem->MySide; else return Mem->TheirSide;}

private:
  Type type;
  deque<Condition> cond_set;
  set<int> pl_set;
  Region reg;
  PTSType pt;
  bool team;
  int unum_min,max;
};
/**********************************************************************/

//with ball and without ball together
class Action{
public:
  enum Type{
    pass_to_players,
    pass_na_hod,
    move_ball,
    go_to_pos,
    mark,
    none
  };
  enum BallMove{
    dribble,
    clear,
    hold,
    score
  };
  Action(const set<int>& ps,Type t){ pl_set=ps;type=t;if(t==mark) with_ball=false; else with_ball=true;}
  Action(const Region& r,const set<BallMove>& bm){ reg=r;bm_set=bm;type=move_ball;with_ball=true;}
  Action(const Region& r){reg=r;type=go_to_pos,with_ball=false;}
  Action(bool wb){type=none;with_ball=wb;}

  bool WithBall()const{return with_ball;}
  bool Execute()const;
  bool GetTargetPos(Vector& target,float& ang_err);
private:
  float GetOptimalPos(bool is_pnh,Vector& target,Unum& res,float& vel)const;
  static const float end_vel_in_clear_ball;
  static const float add_dist_in_pnh;
  static const float end_vel_in_pass_na_hod_kick;
  static const float pass_treshold;

  Type type;
  bool with_ball;
  set<int> pl_set;
  set<BallMove> bm_set;
  Region reg;
};
/**********************************************************************/
class Step{
public:
  struct LeaveCond{
    Condition cond;
    string name;
    LeaveCond(const Condition& c,const string& n){cond=c;name=n;}
  };
  Step(){}
  Step(const string& n,const deque<Action>& wba,const map<int,deque<Action> >& nba,const vector<LeaveCond>& lc){
    name=n;actions=wba;no_ball_actions=nba;leave_conditions=lc;
  }

  bool IsValid() const;
  bool Execute() ;
  string NextStep() const;
  string GetName()const{return name;}
  bool GetTmTargetPos(Unum tm,Vector& target,float& ang_err);
private:
  string name;//name of step
  deque<Action> actions;//for player with ball
  map<int,deque<Action> > no_ball_actions;//int - number of players in current_pl_map
  vector<LeaveCond> leave_conditions;//conditions to go to ather step or to stop scenario
};
/**********************************************************************/

class Scenario {
public:
  enum RetType{
    exec,
    cont,
    stop
  };
  Scenario(){}
  Scenario(const string& n,float p,const map<string,Step>& s,const deque<Condition>& c,const map<int,InitTmType>& itm,const string& fs){
    name=n;priority=p;steps=s;init_cond=c;init_tm_map=itm;first_step=fs;
  }
  friend class Scenario_Cmp;

  bool CanBegin()const;
  bool IsValid() const;
  void InitScenario(bool init_global_map=true);
  int CheckTmPresent();//return 0 if ok, otherwise return number of epsent tm
  RetType Execute();
  string GetName()const{return name;}
  int  GetTmNumberInScenario(Unum number)const;//return 0 - tm not in scenario (call only after init scenario)
  bool GetTmTargetPos(Unum num,Vector& target,float& ang_err);
private:
  string current_step;//name of current step
  string first_step;//first step of scenario

  string name;//name of scenario
  float priority;//priority of scenario
  map<string,Step> steps;//steps of scenario
  deque<Condition> init_cond;//condition of begin scenario(must be only one)
  map<int,InitTmType> init_tm_map;//numbers for tm
};
/**********************************************************************/
class Scenario_Cmp{
public:
  int operator() (Scenario* s1,Scenario* s2){
    return s1->priority>s2->priority;
  }
};
/**********************************************************************/
class ScenarioSet{
public:
  ScenarioSet(){scenario_going=init=false;}
  ScenarioSet(const vector<Scenario>& s){scenarios=s;scenario_going=init=false;}
  ScenarioSet(const ScenarioSet& ss){*this=ss;}

  bool IsScenarioInit()const{return init;}
  bool StartScenario();
  bool ExecuteScenario();
  int  GetTmNumberInScenario(Unum number)const;//return 0 if not in scenario
  bool IsScenarioGoing()const{return scenario_going;}
  bool GetTmTargetPos(Unum tm,Vector& target,float& ang_err);

  void SayStartScenario()const;
  void HearStartScenario(char* msg);
  void SayStopScenario()const;
  void HearStopScenario();
  bool SayInScenario(string& buffer);
private:
  bool InitScenarioSet();
  void StopScenario();
  bool CheckForStopScenario();

  bool scenario_going;//scenario now going
  bool init;//scenario database was read
  Scenario* current_scenario;
  vector<Scenario> scenarios;
  vector<bool> IsValid;//scenario is valid and can execute
};
/**********************************************************************/
extern ScenarioSet Scenarios;
/**********************************************************************/
class ScenarioBuilder{
public:
  friend class ScenarioSet;

  ScenarioBuilder(){}

  void BuildScenarioSet(){Scenarios=ScenarioSet(scenarios);scenarios.clear();}

  void BuildScenario(string name,float p){
    scenarios.push_back(Scenario(name,p,steps,init_cond,init_tm_map,first_step));
    steps.clear();init_cond.clear();init_tm_map.clear();
  }

  void BuildInitCond(){init_cond.push_back(conditions.back());conditions.pop_back();}

  void BuildInitTmFromPt(){tm_type.SetPTSType(PTStype);PTStype.Clear();}
  void BuildInitTmFromReg(){tm_type.SetRegion(regions.back());regions.pop_back();}
  void BuildInitTmFromFastest(){tm_type.SetFastest();}

  void BuildInitTmFromList(int num){init_tm_map[num]=tm_type;tm_type.Clear();}

  void BuildStep(string name){
    if(steps.empty()) first_step=name;
    steps[name]=Step(name,with_ball_actions,no_ball_actions,leave_cond);
    with_ball_actions.clear();no_ball_actions.clear();leave_cond.clear();
  }

  void BuildLeaveCondition(string name){leave_cond.push_back(Step::LeaveCond(conditions.back(),name));conditions.pop_back();}

  void BuildWithBallActions(){with_ball_actions=actions;actions.clear();}

  void BuildNoBallActions(int num){no_ball_actions[num]=actions;actions.clear();}

  void BuildPTPAction(){actions.push_back(Action(pl_set,Action::pass_to_players));pl_set.clear();}
  void BuildMBAction(){actions.push_back(Action(regions.back(),bm_set));regions.pop_back();bm_set.clear();}
  void BuildPNHAction(){actions.push_back(Action(pl_set,Action::pass_na_hod));pl_set.clear();}
  void BuildNoneWBAction(){actions.push_back(Action(true));}
  void BuildMTAction(){actions.push_back(Action(regions.back()));regions.pop_back();}
  void BuildMarkAction(){actions.push_back(Action(pl_set,Action::mark));pl_set.clear();}
  void BuildNoneNBAction(){actions.push_back(Action(false));}

  void AddBallMove(Action::BallMove bm){ bm_set.insert(bm);}

  void BuildBposCondition(){conditions.push_back(Condition(regions.back()));regions.pop_back();}
  void BuildPposCondition(int min,int max){conditions.push_back(Condition(PTStype,min,max,regions.back()));regions.pop_back();PTStype.Clear();}
  void BuildBownerCondition(){conditions.push_back(Condition(PTStype));PTStype.Clear();}
  void BuildOrCondition(){Condition cond(conditions,Condition::t_or);conditions.clear();conditions.push_back(cond);}
  void BuildAndCondition(){Condition cond(conditions,Condition::t_and);conditions.clear();conditions.push_back(cond);}
  void BuildNotCondition(){Condition cond(conditions.back(),Condition::t_not);conditions.clear();conditions.push_back(cond);}
  void BuildPposExtCondition(int min,int max){conditions.push_back(Condition(team.back(),pl_set,min,max,regions.back()));team.pop_back();pl_set.clear();regions.pop_back();}
  void BuildBownerCondition(int unum){conditions.push_back(Condition(unum));}

  void AddToPlayerSet(int num){pl_set.insert(num);}
  void AddAllToPlayerSet(){for(int i=1;i<=11;i++) pl_set.insert(i);}

  void AddPlayerType(Ptype pt){PTStype.Add(pt);}
  void AddPlayerSide(PTSType::PSType ps){PTStype.Add(ps);}

  void BuildRectangleRegion(){Point p2=points.back();points.pop_back();Point p1=points.back();points.pop_back();regions.push_back(Region(p1,p2));}
  void BuildPointRegion(){Point p=points.back();points.pop_back();regions.push_back(Region(p));}
  void BuildCircleRegion(float r){Point p=points.back();points.pop_back();regions.push_back(Region(p,r));}
  void BuildArcRegion(float r1,float r2,float a1,float a2){regions.push_back(Region(points.back(),r1,r2,a1,a2));points.pop_back();}
  void BuildRegRegion(){Region reg(regions);regions.clear();regions.push_back(reg);}

  void BuildPoint(){Val v1=val.front();val.pop_front();Val v2=val.front();val.pop_front();points.push_back(Point(v1,v2));}
  void BuildAddPoint(){Val v1=val.front();val.pop_front();Val v2=val.front();val.pop_front();Point p(v1,v2,&points.back());points.pop_back();points.push_back(p);}
  void BuildPointFromBall(){points.push_back(Point(Point::ball));}
  void BuildPointFromPlayer(Unum player){points.push_back(Point(team.back(),player));team.pop_back();}

  void BuildValue(float v){val.push_back(Val(v));}
  void BuildValueBx(){val.push_back(Val(Val::ballx));}
  void BuildValueBy(){val.push_back(Val(Val::bally));}
  void BuildValueOffside(){val.push_back(Val(Val::offside));}
  void BuildValueTmX(int num){val.push_back(Val(Val::Type(Val::tm1x+num-1)));}
  void BuildValueTmY(int num){val.push_back(Val(Val::Type(Val::tm1y+num-1)));}

  void SetTeam(bool our_team){team.push_back(our_team);}
private:
  deque<Val> val;
  deque<Point> points;
  deque<Region> regions;
  set<int> pl_set;
  deque<bool> team;
  PTSType PTStype;
  InitTmType tm_type;
  deque<Condition> conditions;
  set<Action::BallMove> bm_set;
  deque<Action> actions,with_ball_actions;
  map<int,deque<Action> > no_ball_actions;
  vector<Step::LeaveCond> leave_cond;
  map<string,Step> steps;
  string first_step;
  map<int,InitTmType> init_tm_map;
  deque<Condition> init_cond;
  vector<Scenario> scenarios;

};
/**********************************************************************/
class ScenarioLexer:public yyFlexLexer{
public:
  ScenarioLexer(istream* input=0):yyFlexLexer(input){}
  virtual int yylex();

  typedef struct _holder{
    float fval;
    int   val;
    bool  bval;
    string str;
    Ptype pt;
  }Holder;
  Holder holder;
  ScenarioBuilder builder;
};
#endif
