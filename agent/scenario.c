/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : scenario.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.10 $
 *
 *    $Id: scenario.C,v 2.10 2004/06/26 08:26:08 anton Exp $
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

#include "scenario.h"
#include "dribble.h"
#include "Communicate.h"
#include <algorithm>

////////////////////////////////////////////////////
const float Action::end_vel_in_clear_ball=1.4f;
const float Action::add_dist_in_pnh=1.5f;
const float Action::end_vel_in_pass_na_hod_kick=1.0f;
const float Action::pass_treshold=0.52f;
////////////////////////////////////////////////////
map<int,int> CurrentScenarioInfo::current_pl_map;

extern int yyparse(void* param);
ScenarioSet Scenarios;
/////////////////////////////////////////////////////
Val::operator float()const{
  if(type==t_float)
    return val;
  if(type==none){
    my_error("Value: type = none !!!");
    return 0.0f;
  }
  if(type==ballx||type==bally){
    if(!Mem->BallPositionValid()){
      my_error("Value: ball position is not valid");
      return 0.0f;
    }
    return type==ballx?Mem->BallX():Mem->BallY();
  }
  if(type==offside)
    return Mem->my_offside_line;
  if(type>=10&&type<18){
    if(CurrentScenarioInfo::current_pl_map.find(type-9)==CurrentScenarioInfo::current_pl_map.end()){
      my_error("Value: no tm in scenario with number %d",int(type-9));
      return 0.0f;
    }
    return Pos.GetTmPos(CurrentScenarioInfo::current_pl_map[type-9]).x;
  }
  if(type>=20&&type<28){
    if(CurrentScenarioInfo::current_pl_map.find(type-19)==CurrentScenarioInfo::current_pl_map.end()){
      my_error("Value: no tm in scenario with number %d",int(type-19));
      return 0.0f;
    }
    return Pos.GetTmPos(CurrentScenarioInfo::current_pl_map[type-19]).y;
  }
  my_error("Value: wrong type");
  return 0.0f;
}
/////////////////////////////////////////////////////
Point::operator Vector()const{
  Unum tm;
  switch(type){
  case number:
    return Vector(val1,val2);
  case ball: if(!Mem->BallPositionValid()){
    my_error("Point: ball position is not valid");
    return Vector(.0,.0);
  }
    return Mem->BallAbsolutePosition();
  case player:if(!my_side){
    if(!Mem->OpponentPositionValid(player_pos)){
      my_error("Point: Opponent %.0f position not valid",float(player_pos));
      return Vector(.0,.0);
    }
    return Mem->OpponentAbsolutePosition(player_pos);
  }
    tm=CurrentScenarioInfo::current_pl_map[player_pos];
    if(tm==0){
      my_error("Point:no player in scenario with number %d",player_pos);
      return Vector(.0f,.0f);
    }
    return Pos.GetTmPos(tm);
  case add_p:
    return Vector(*add_point)+Vector(val1,val2);
  default:
    my_error("Wrong type in Point!");
    return Vector(.0,.0);
  }
}
////////////////////////////////////////////////////////////
Point& Point::operator=(const Point& p){
  if(this!=&p){//protection of copy to yourself
    type=p.type;
    val1=p.val1;
    val2=p.val2;
    player_pos=p.player_pos;
    my_side=p.my_side;
    if(p.type==add_p){
      add_point=new Point(*p.add_point);
    }
  }
  return *this;
}
////////////////////////////////////////////////////////////
bool Region::IsIn(Vector pointval) const{
  if(type==rectangle){//becouse can not init rectangle in case
    Vector p1=point1,p2=point2;
    Rectangle rec(p1.x,p2.x,p1.y,p2.y);
    return rec.IsWithin(pointval);
  }
  switch(type){
  case point: return (Vector(point1)-pointval).mod()<=Mem->CP_at_point_buffer;

  case arc:   return (Vector(point1)-pointval).mod()>=radius1&&(Vector(point1)-pointval).mod()<=radius2&&
                (pointval-Vector(point1)).dir()>=ang1&&(pointval-Vector(point1)).dir()<=ang2;

  case reg:     for(deque<Region>::const_iterator i=region_set.begin();i!=region_set.end();i++)
    if((*i).IsIn(pointval)) return true;
    return false;
  default:
    my_error("Wrong type in Region!");
    return false;
  }
}
////////////////////////////////////////////////////////////
Vector Region::Center() const{
  if(type==rectangle){
    Vector p1=point1,p2=point2;
    Rectangle rec(p1.x,p2.x,p1.y,p2.y);
    return rec.Center();
  }
  switch(type){
  case point:  return point1;
  case arc: return Vector(point1)+Polar2Vector(radius1+(radius2-radius1)*(1-fabs(ang1-ang2)/360.0f),GetNormalizeAngleDeg((ang1+ang2)/2));
  case reg:    return (*region_set.begin()).Center();
  default:
    my_error("Wrong type in Region!");
    return false;
  }

}
////////////////////////////////////////////////////////////
PTSType::operator Ptype()const{
  int res=0;
  for(deque<Ptype>::const_iterator i=ptdata.begin();i!=ptdata.end();i++)
    res|=*i;
  return Ptype(res);
}
///////////////////////////////////////////////////////////
PTSType::operator Pside() const{
  int res=0;
  for(deque<PSType>::const_iterator i=psdata.begin();i!=psdata.end();i++){
    Pside temp=PS_All;
    switch(*i){
    case center: temp=PS_Center;
      break;
    case left: temp=PS_Left;
      break;
    case right: temp=PS_Right;
      break;
    case all: temp=PS_All;
      break;
    case wingnb:
      if(Mem->BallPositionValid())
	temp=Mem->BallY()<=0?PS_Right:PS_Left;
      break;
    case wingwb:
      if(Mem->BallPositionValid())
	temp=Mem->BallY()>0?PS_Right:PS_Left;
      break;
    }
    res|=temp;
  }
  return Pside(res);
}
/////////////////////////////////////////////////////
bool Condition::Check() const{
  int num=0,work;
  Formations::Iterator iter=Pos.begin();
  switch(type){
  case t_bpos:
    if(!Mem->BallPositionValid()){
      my_error("ball position is not valid!!");
      return false;
    }
    return reg.IsIn(Mem->BallAbsolutePosition());
  case t_ppos:
    while(iter!=Pos.end()){
      Unum tm=Pos.GetPlayerNumber(iter,Ptype(pt),Pside(pt));
      if(tm==Unum_Unknown)
	break;
      if(!Mem->TeammatePositionValid(tm))
	continue;
      if(reg.IsIn(Pos.GetTmPos(tm)))
	num++;
    }
    return unum_min<=num&&max>=num;
  case t_bowner:
    while(iter!=Pos.end()){
      Unum tm=Pos.GetPlayerNumber(iter,Ptype(pt),Pside(pt));
      if(tm==Unum_Unknown)
	break;
      if(Pos.FastestTm()==tm)
	return true;
    }
    return false;
  case t_and:
    for(deque<Condition>::const_iterator i=cond_set.begin();i!=cond_set.end();i++)
      if(!(*i).Check())
	return false;
    return true;
  case t_or:
    for(deque<Condition>::const_iterator i=cond_set.begin();i!=cond_set.end();i++)
      if((*i).Check())
	return true;
    return false;
  case t_not:
    if(!cond_set.back().Check())
      return true;
    return false;
  case t_ppos_plset:
    for(set<int>::const_iterator i=pl_set.begin();i!=pl_set.end();i++){
      if(team){
	if(CurrentScenarioInfo::current_pl_map.find(*i)==CurrentScenarioInfo::current_pl_map.end()){
	  my_error("No tm with  number %d in scenario!!!",*i);
	  work=0;
	}else
	  work=CurrentScenarioInfo::current_pl_map[*i];
      }else
	work=*i;//for opponents
      if(work<1||work>11||!Mem->PlayerPositionValid(GetTeam(team),work))
	continue;
      if(reg.IsIn(GetTeam(team)==Mem->MySide?Pos.GetTmPos(work):Mem->OpponentAbsolutePosition(work)))
	num++;
    }
    return unum_min<=num&&max>=num;
  case t_bowner_unum:
    if(CurrentScenarioInfo::current_pl_map.find(unum_min)==CurrentScenarioInfo::current_pl_map.end()){
      my_error("No tm with  number %d in scenario!!!",unum_min);
      return false;
    }
    return Pos.FastestTm()==CurrentScenarioInfo::current_pl_map[unum_min];
  default:
    my_error("Wrong condition type!!!!!");
    return false;
  }
}
////////////////////////////////////////////////////////////////////////
float Action::GetOptimalPos(bool is_pnh,Vector& target,Unum& res,float& vel)const{
  typedef set<int>::const_iterator CI;
  float max_conf=-1.0f,conf,res_vel;
  Unum temp = Unum_Unknown,
			 opp  = Unum_Unknown;
  if(pl_set.empty()){
    my_error("Action: pl_set is empty");
    return 0.0f;
  }
  for(CI iter=pl_set.begin();iter!=pl_set.end();iter++){
    if(CurrentScenarioInfo::current_pl_map.find(*iter)==CurrentScenarioInfo::current_pl_map.end()
       ||!Mem->TeammatePositionValid(temp=CurrentScenarioInfo::current_pl_map[*iter]))
      continue;
    Vector t=Pos.GetSmartPassPointInScenario(temp,&res_vel);
    if(is_pnh)
      t+=Polar2Vector(add_dist_in_pnh,0.0);
    if((conf=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),t,opp))>max_conf){
      max_conf=conf;
      target=t;
      res=temp;
      vel=res_vel;
    }
  }
  return max_conf;
}
////////////////////////////////////////////////////////////////////////
bool Action::Execute()const{
  Dribble dribble;
  Vector target;
  Unum res;
  float temp,vel;
  SKMODE mode=SK_Safe;
  if(Mem->ClosestOpponentToBallDistance()<4.0f)
    mode=SK_Fast;
  
  switch(type){
  case  pass_to_players:
    if((temp=GetOptimalPos(false,target,res,vel))>pass_treshold){
      Mem->LogAction4(10,"Scenario:pass to tm %.0f with conf %.2f",float(res),temp);
      Mem->LogAction7(30,"orig pos (%.2f,%.2f) with conf %.2f; predict pos (%.2f,%.2f)",Mem->TeammateX(res),Mem->TeammateY(res),
		      Mem->TeammatePositionValid(res),Pos.GetTmPos(res).x,Pos.GetTmPos(res).y);
      actions.smartkick(vel,actions.GetKickAngle(target),mode);
      return true;
    }
    Mem->LogAction4(10,"Scenario::conf in pass action to tm %.0f is too small: %.2f",float(res),temp);
    return false;
  case  pass_na_hod:
    if((temp=GetOptimalPos(true,target,res,vel))>pass_treshold){
      Mem->LogAction4(10,"Scenario:pass_na_hod to tm %.0f with conf %.2f",float(res),temp);
      Mem->LogAction7(30,"orig pos (%.2f,%.2f) with conf %.2f; predict pos (%.2f,%.2f)",Mem->TeammateX(res),Mem->TeammateY(res),
		      Mem->TeammatePositionValid(res),target.x,target.y);
      actions.smartkick(vel,actions.GetKickAngle(target),mode);
      return true;
    }
    Mem->LogAction4(10,"Scenario::conf in pass_na_hod action to tm %.0f is too small: %.2f",float(res),temp);
    return false;
  case move_ball:
    if(bm_set.find(score)!=bm_set.end()){
      if(actions.MyShootConf()>=0.8f){
	Mem->LogAction3(10,"Scenario::action must score now (with shoot conf : %.2f)",actions.MyShootConf());
	actions.shoot();
	return true;
      }
      Mem->LogAction3(10,"Scenario:shoot conf is too small: %.2f",actions.MyShootConf());
    }
    if(bm_set.find(clear)!=bm_set.end()){
      Mem->LogAction4(10,"Scenario::action must kick ball to point (%.2f,%.2f)",reg.Center().x,reg.Center().y);
      temp=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),reg.Center(),res);
      if(temp>=0.28f){
	Mem->LogAction3(10,"Scenario::conf of kick is %.2f",temp);
	//           if(Mem->BallSpeed()>0.5f||!Mem->BallWillBeKickable()){
	//             Mem->LogAction2(10,"Scenario: ball not will be kickable, so stop ball");
	//             actions.stopball();
	//             return true;
	//           }
	actions.smartkick(Mem->VelAtPt2VelAtFoot(reg.Center(),end_vel_in_clear_ball),reg.Center(),mode);
	return true;
      }
      Mem->LogAction3(10,"Scenario::conf of kick %.2f is too small",temp);
    }
    if(bm_set.find(Action::dribble)!=bm_set.end()){
      Mem->LogAction4(10,"Scenario::action must go dribble to point (%.2f,%.2f)",reg.Center().x,reg.Center().y);
      if(reg.Center()==Vector(777.0f,777.0f))
	target=Dribble::SelectDribbleTarget();
      else
	target=reg.Center();
      dribble.SetDribblePos(target);
      dribble.SetPriority(0.7f);
      if(dribble.GoBaby()||(fabs(Mem->MyY())<4.0f&&microKicks.CanMicroClearBall(false)))
	return true;
    }
    if(bm_set.find(hold)!=bm_set.end()){
      if(!reg.IsIn(Mem->MyPos())){
	Mem->LogAction4(10,"Scenario::action must go dribble to point (%.2f,%.2f) befor hold ball",
			reg.Center().x,reg.Center().y);
	if(reg.Center()==Vector(777.0f,777.0f))
	  target=Dribble::SelectDribbleTarget();
	else
	  target=reg.Center();
	dribble.SetDribblePos(target);
	dribble.SetPriority(0.7f);
	return (dribble.GoBaby()||(fabs(Mem->MyY())<4.0f&&microKicks.CanMicroClearBall(false)));
      }
      Mem->LogAction2(10,"Scenario::action hold ball");
      dribble.hold_ball();
      return true;
    }
    return false;
  case go_to_pos:
    Mem->LogAction2(10,"Scenario::action go to pos");
    Pos.MoveToPos(reg.Center(),7.0,0.0);
    return true;
  case mark:
    if(pl_set.empty()){
      Mem->LogAction2(10,"Scenario: must mark,but player set is empty!!!");
      return false;
    }
    Mem->LogAction3(10,"Scenario: mark opponent %d",*pl_set.begin());
    Pos.MarkOpponent(*pl_set.begin(),Defense::mark_ball,2.0f);
    return true;
  case none:
    if(with_ball)
      return false;//just call HandleBall
    else{
      face_only_body_to_ball();//may be no good(call GoToHomePosition() ? )
      return true;
    }
  default:
    my_error("Action %.0f is not ready or wrong action type",float(type));
    return false;
  }
}
//////////////////////////////////////////////////////////
bool Action::GetTargetPos(Vector& target,float& ang_err){
  if(WithBall())
    return false;
  if(type==go_to_pos){
    target=reg.Center();
    ang_err=7.0f;
    return true;
  }
  if(type==mark){
    if(pl_set.empty())
      return false;
    target=Mem->OpponentAbsolutePosition(*pl_set.begin()) +
      Polar2Vector(2.0f, (Mem->BallAbsolutePosition()- Mem->OpponentAbsolutePosition(*pl_set.begin())).dir());
    ang_err=30.0f;
    return true;
  }
  return false;
}
//////////////////////////////////////////////////////////
bool Step::IsValid() const {
  if(name.length()==0){
    my_error("Step has no name!!!");
    return false;
  }
  if(actions.empty()){
    my_error("Step %s has no action for player with ball",const_cast<char*> (name.c_str()));
    return false;
  }
  return true;
}
////////////////////////////////////////////////////////////
string Step::NextStep() const{
  for(unsigned int i=0;i<leave_conditions.size();i++){
    if(leave_conditions[i].cond.Check())
      return leave_conditions[i].name;
  }
  return name;
}
///////////////////////////////////////////////////////////
bool Step::Execute(){
  typedef deque<Action>::const_iterator CI;
  if(Pos.FastestTm()==Mem->MyNumber){
    if(!Mem->BallKickable()){
      Mem->LogAction3(20,"Step %s: ball not kickable but i`m fastest, so go to ball",const_cast<char*> (name.c_str()));
      get_ball();
      return true;
    }
    Mem->LogAction3(20,"Step %s: try execute one of action",const_cast<char*> (name.c_str()));
    for(CI iter=actions.begin();iter!=actions.end();iter++){
      if((*iter).Execute())
        return true;
    }
    Mem->LogAction3(20,"Step %s: no action can execute, so call HandleBall",const_cast<char*> (name.c_str()));
    return false;
  }
  //take my number in current scenario
  typedef map<int,int>::const_iterator MI;
  Unum number=Unum_Unknown;
  for(MI iter=CurrentScenarioInfo::current_pl_map.begin();iter!=CurrentScenarioInfo::current_pl_map.end();iter++){
    if(iter->second==Mem->MyNumber)
      number=iter->first;
  }
  if(number==Unum_Unknown){
    Mem->LogAction2(20,"Scenario: i`m not in this scenario");
    return false;
  }
  if(no_ball_actions.find(number)==no_ball_actions.end()){
    Mem->LogAction3(20,"For player with number %d",number);
    Mem->LogAction3(20,"Step %s: no action set define",const_cast<char*> (name.c_str()));
    return false;
  }
  //execute my action
  Mem->LogAction3(20,"Step %s: try execute one of action",const_cast<char*> (name.c_str()));
  for(CI iter=no_ball_actions[number].begin();iter!=no_ball_actions[number].end();iter++){
    if((*iter).Execute())
      return true;
  }
  Mem->LogAction3(20,"For player with number %d",number);
  Mem->LogAction3(20,"Step %s: no action can execute",const_cast<char*> (name.c_str()));
  return false;
}
///////////////////////////////////////////////////////////////////////////////////////
bool Step::GetTmTargetPos(Unum tm,Vector& target,float& ang_err){
  if(Pos.FastestTm()==tm){
    if(Mem->BallKickableForTeammate(tm)||!Mem->TeammateInterceptionAble(tm)){
      target=Mem->BallAbsolutePosition();
      ang_err=5.0f;
      return true;
    }
    target=Mem->TeammateInterceptionPoint(tm);
    ang_err=5.0f;
    return true;
  }
  typedef map<int,int>::const_iterator MI;
  typedef deque<Action>::iterator I;
  Unum number=Unum_Unknown;
  for(MI iter=CurrentScenarioInfo::current_pl_map.begin();iter!=CurrentScenarioInfo::current_pl_map.end();iter++){
    if(iter->second==tm)
      number=iter->first;
  }
  if(number==Unum_Unknown||no_ball_actions.find(number)==no_ball_actions.end())
    return false;
  for(I iter=no_ball_actions[number].begin();iter!=no_ball_actions[number].end();iter++)
    if((*iter).GetTargetPos(target,ang_err))
      return true;
  return false;
}
///////////////////////////////////////////////////////////////////////////////////////
Unum InitTmType::GetNumber()const{
  Formations::Iterator iter=Pos.begin();
  Vector pos;
  Unum tm=Unum_Unknown;
  set<Unum> valid_tm,valid_tm2;
  if(fastest){
    tm=Pos.FastestTm();
    if(tm==Unum_Unknown||Pos.GetPlayerType(tm)<=PT_Defender)
      return Unum_Unknown;
    valid_tm.insert(tm);
  }
  if(!PType.Empty()){
    while(iter!=Pos.end()){
      tm=Pos.GetPlayerNumber(iter,Ptype(PType),Pside(PType));
      if(tm==Unum_Unknown)
	break;
      if(valid_tm.empty())
	valid_tm2.insert(tm);
      else
	if(valid_tm.find(tm)!=valid_tm.end())
	  valid_tm2.insert(tm);
    }
    if(valid_tm2.empty())
      return Unum_Unknown;
    swap(valid_tm,valid_tm2);
    valid_tm2.clear();
  }
  //теперь в valid_tm хранится множество потенциально подходящих номеров (если пусто - то пока подходят все)
  if(reg.IsValid()){
    for(int i=1;i<=11;i++){
      if(i==Mem->OurGoalieNum||!Mem->TeammatePositionValid(i))
	continue;
      pos=Pos.GetTmPos(i);
      if(reg.IsIn(pos)){
	if(valid_tm.empty())
	  valid_tm2.insert(i);
	else
	  if(valid_tm.find(i)!=valid_tm.end())
	    valid_tm2.insert(i);
      }
    }
    if(valid_tm2.empty())
      return Unum_Unknown;
    swap(valid_tm,valid_tm2);
  }
  return *valid_tm.begin();
}
///////////////////////////////////////////////////////////////////////////////////////
bool Scenario::IsValid() const{
  if(init_cond.empty()){
    Mem->LogAction3(20,"Scenario %s:empty init conditions list, so can not start",const_cast<char*> (name.c_str()));
    return false;
  }
  if(steps.empty()){
    Mem->LogAction3(20,"Scenario %s:have no steps, so can not start",const_cast<char*> (name.c_str()));
    return false;
  }
  typedef map<string,Step>::const_iterator CI;
  for(CI i=steps.begin();i!=steps.end();i++)
    if(!i->second.IsValid()){
      Mem->LogAction3(20,"It was error in scenario %s",const_cast<char*> (name.c_str()));
      return false;
    }
  return true;
}
///////////////////////////////////////////////////////////////////////////////////////
//think that we have 0 or 1 condition in init_cond
bool Scenario::CanBegin()const{
  typedef deque<Condition>::const_iterator CI;
  for(CI iter=init_cond.begin();iter!=init_cond.end();iter++){
    if((*iter).Check())
      return true;
  }
  return false;
}
//////////////////////////////////////////////////////////////////////////////////////
int Scenario::CheckTmPresent(){
  typedef map<int,InitTmType>::const_iterator CI;
  vector<int> tm;
  for(CI iter=init_tm_map.begin();iter!=init_tm_map.end();iter++){
    tm.push_back(iter->second.GetNumber());
    if(tm.back()==Unum_Unknown)
      return iter->first;
  }
  if(tm.size()>1){
    for(vector<int>::iterator iter=tm.begin();iter!=(tm.end()-1);iter++)
      if(count(iter+1,tm.end(),*iter)>0)
	return *iter;
  }   
  return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
void Scenario::InitScenario(bool init_global_map){
  current_step=first_step;
  if(!init_global_map)
    return;
  CurrentScenarioInfo::current_pl_map.clear();
  typedef map<int,InitTmType>::const_iterator CI;
  for(CI iter=init_tm_map.begin();iter!=init_tm_map.end();iter++){
    Unum tm=iter->second.GetNumber();
    if(tm==Unum_Unknown)
      continue;
    CurrentScenarioInfo::current_pl_map[iter->first]=tm;
  }
}
/////////////////////////////////////////////////////////////////////////////////////
int Scenario::GetTmNumberInScenario(Unum number)const{//if return 0 - tm not in scenario
  if(number==Unum_Unknown)
    return 0;
  typedef map<int,int>::const_iterator CI;
  for(CI iter=CurrentScenarioInfo::current_pl_map.begin();iter!=CurrentScenarioInfo::current_pl_map.end();iter++)
    if(number==iter->second)
      return iter->first;
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
Scenario::RetType Scenario::Execute(){
  while(1){
    string temp=steps[current_step].NextStep();
    if(temp=="end"){
      Mem->LogAction4(20,"Scenario %s:stop scenario, becouse condition in step %s is valid",
		      const_cast<char*> (name.c_str()),const_cast<char*> (current_step.c_str()));
      return stop;
    }
    if(steps.find(temp)==steps.end()){
      Mem->LogAction4(20,"Scenario %s:stop scenario, becouse can not find step with name \"%s\"",
		      const_cast<char*> (name.c_str()),const_cast<char*> (temp.c_str()));
      return stop;
    }
    if(current_step==temp)
      break;
    else{
      Mem->LogAction5(20,"Scenario %s:change step to %s becouse condition in step %s is valid",
		      const_cast<char*> (name.c_str()),const_cast<char*> (temp.c_str()),const_cast<char*> (current_step.c_str()));
      current_step=temp;
    }
  }  
  if(steps[current_step].Execute())
    return exec;
  else
    return cont;
}
//////////////////////////////////////////////////////////////////////////////
bool Scenario::GetTmTargetPos(Unum num,Vector& target,float& ang_err){
  if(!GetTmNumberInScenario(num))
    return false;
  string temp=steps[current_step].NextStep();
  if(temp=="end"||steps.find(temp)==steps.end())
    return false;
  return steps[temp].GetTmTargetPos(num,target,ang_err);
}
//////////////////////////////////////////////////////////////////////////////
int ScenarioSet::GetTmNumberInScenario(Unum number)const{
  if(!IsScenarioGoing())
    return 0;
  return current_scenario->GetTmNumberInScenario(number);
}
//////////////////////////////////////////////////////////////////////////////
bool ScenarioSet::GetTmTargetPos(Unum tm,Vector& target,float& ang_err){
  if(!IsScenarioGoing())
    return false;
  return current_scenario->GetTmTargetPos(tm,target,ang_err);
}
//////////////////////////////////////////////////////////////////////////////
bool ScenarioSet::InitScenarioSet(){
  ifstream file("./scenario.data");
  if(file==0){
    cerr<<"I have no file scenario.data"<<endl;
    my_error("Can not find file ./scenario.data");
    return false;
  }
  ScenarioLexer lexer(&file);
  int res=yyparse(&lexer);
  if(res){
    if(lexer.builder.scenarios.empty())
      my_error("Error was in first scenario");
    else
      my_error("Last correct loaded scenario is %s",const_cast<char*> (lexer.builder.scenarios.back().GetName().c_str()));
    return false;
  }
  if(Mem->MyNumber==1)
    cout<<"Loaded "<<scenarios.size()<<" scenario."<<endl;
  Mem->LogAction3(10,"Scenario: number of loaded scenarios: %d",int(scenarios.size()));

  scenario_going=false;
  IsValid.resize(scenarios.size());
  for(unsigned int i=0;i<scenarios.size();i++){
    if(scenarios[i].IsValid())
      IsValid[i]=true;
    else
      IsValid[i]=false;
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////////
void ScenarioSet::SayStartScenario()const{//call only after init scenario
  char msg[10];
  Msg m(msg);
  m<<char(ST_start_scenario);
  int scenario_num=-1;
  for(unsigned int i=0;i<scenarios.size();i++)
    if(current_scenario->GetName()==scenarios[i].GetName()){
      scenario_num=i;
      break;
    }
  if(scenario_num<0||scenario_num>Communicate::max_num){
    my_error("Wrong number of scenario %d, then try say of begin",scenario_num);
    return;
  }
  m<<char(scenario_num);
  for(int i=1;i<=8;i++)
    if(CurrentScenarioInfo::current_pl_map.find(i)!=CurrentScenarioInfo::current_pl_map.end())//for not adding empty fields (bad for communication)
      m<<char(CurrentScenarioInfo::current_pl_map[i]);
    else
      m<<char(0);
  Mem->SayNow(msg);
}
//////////////////////////////////////////////////////////////////////////////
void ScenarioSet::HearStartScenario(char* msg){
  Msg m(msg);
  char scenario_num,temp;
  m>>scenario_num;
  current_scenario=&scenarios[scenario_num];
  scenario_going=true;
  Mem->LogAction3(10,"Hear, that must start scenario %s",const_cast<char*> (current_scenario->GetName().c_str()));
  CurrentScenarioInfo::current_pl_map.clear();
  for(int i=1;i<=8;i++){
    m>>temp;
    if(temp!=0)
      CurrentScenarioInfo::current_pl_map[i]=temp;
  }
  current_scenario->InitScenario(false);
}
/////////////////////////////////////////////////////////////////////////////
void ScenarioSet::SayStopScenario()const{
  char msg[10];
  Msg m(msg);
  m<<char(ST_stop_scenario);
  Mem->AddBallPos(m);
  Mem->AddBallVel(m);
  Mem->SayNow(msg);
}
/////////////////////////////////////////////////////////////////////////////
void ScenarioSet::HearStopScenario(){
  if(Pos.FastestTm()==Mem->MyNumber&&IsScenarioGoing()&&CheckForStopScenario()){//somebody wrong say, that must stop scenario
    Mem->LogAction2(100,"Scenario: tm was wrong then stop scenario, so continue it");
    SayStartScenario();//hack
    return;
  }
  StopScenario();
}
//////////////////////////////////////////////////////////////////////////////
void ScenarioSet::StopScenario(){
  if(scenario_going){
    CurrentScenarioInfo::current_pl_map.clear();
    if(Pos.FastestTm()==Mem->MyNumber&&Mem->BallPositionValid()>0.9f&&Mem->BallVelocityValid()>0.9f){
      Mem->LogAction2(10,"Scenario:I`m fastest to ball, so say that must stop scenario");
      SayStopScenario();
    }
    scenario_going=false;
  }
}
/////////////////////////////////////////////////////////////////////////////
bool ScenarioSet::SayInScenario(string& buffer){
  if(!GetTmNumberInScenario(Mem->MyNumber))
    return false;
  Msg m(const_cast <char*> (buffer.c_str()));
  //   int t=Mem->CurrentTime.t,i=0;
  //   int num_tm_in_scenario=CurrentScenarioInfo::current_pl_map.size();
  //   t%=num_tm_in_scenario;//0 - num_tm_in_scenario-1
  //   typedef map<int,int>::const_iterator CI;
  //   for(CI iter=CurrentScenarioInfo::current_pl_map.begin();iter!=CurrentScenarioInfo::current_pl_map.end();iter++,i++){
  //     if(Mem->MyNumber==iter->second&&t==i){//can say
  if(Pos.FastestTm()==Mem->MyNumber){
    Mem->LogAction2(100,"Scenario: i`m fastest, so say about ball pos and vel");
    if(Mem->BallKickable()){
      m<<char(ST_kickable_and_player_and_conf);
      Unum opp[2],player;
      int num=Mem->SelectOpponents(opp);
      if(num==0)
	player=Mem->MyNumber;
      else
	player=-opp[0];
      Mem->AddBallPos(m);
      Mem->AddPlayerPos(player,m);
    }else{	
      m<<char(ST_fastest_tm1+Mem->MyNumber-1);
      Mem->AddBallPos(m);
      Mem->AddBallVel(m);
    }
    return true;
  }
  Unum opp=Unum_Unknown;
  actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),Mem->MyPos(),opp);
  if(opp==Unum_Unknown){
    Mem->LogAction2(100,"Scenario: i don`t know danger opp");
    m<<char(ST_mypos_and_conf_ball_and_conf);
    Mem->AddMyPos(m);
    Mem->AddBallPos(m);
    return true;
  }
  Mem->LogAction2(100,"Scenario: say about danger opponent");
  m<<char(ST_mypos_and_conf_player_and_conf);
  Mem->AddMyPos(m);
  Mem->AddPlayerPos(-opp,m);
  return true;
  //     }
  //   }
  //return false;
}
/////////////////////////////////////////////////////////////////////////////
bool ScenarioSet::CheckForStopScenario(){//return false if stop scenario
  bool stop=false;
  Vector ball=Mem->BallAbsolutePosition();
  if(Mem->PlayMode!=PM_Play_On){
    stop=true;
  }else
    if(Pos.FastestTm()==Mem->MyNumber){//difference for softly stop scenario for players without ball
      Unum tm=Mem->ClosestTeammateTo(ball);
      if(tm!=Unum_Unknown&&GetTmNumberInScenario(tm)&&tm!=Mem->MyNumber)//hack
	return true;
      if(Pos.IsDefense()&&Pos.FastestOpp()!=Unum_Unknown&&!Mem->BallKickable()){
	Mem->LogAction3(30,"Scenario: opp %d intercept the ball, so stop scenario",Pos.FastestOpp());
	stop=true;
      }else
	if(Pos.FastestTm()==Unum_Unknown||!GetTmNumberInScenario(Pos.FastestTm())){
	  Mem->LogAction3(10,"Scenario: tm %d is fastest to ball, but he is not in scenario, so stop scenario",Pos.FastestTm());
	  stop=true;
	}
    }else{
      if(Pos.OppInter()&&Pos.TmCycles()>(Pos.OppCycles()+4)&&(Mem->OpponentAbsolutePosition(Pos.FastestOpp())-ball).mod()<3.0f){
	Mem->LogAction3(30,"Scenario: opp %d intercept the ball (strong), so stop scenario",Pos.FastestOpp());
	stop=true;
      }else
	if(Pos.FastestTm()==Unum_Unknown||!GetTmNumberInScenario(Pos.FastestTm())){
	  int min_cyc=1000;
	  for(int i=1;i<=11;i++){
	    if(Mem->TeammatePositionValid(i)&&GetTmNumberInScenario(i)&&Mem->TeammateInterceptionAble(i)&&min_cyc>Mem->TeammateInterceptionNumberCycles(i))
	      min_cyc=Mem->TeammateInterceptionNumberCycles(i);
	  }
	  if(Pos.FastestTm()==Unum_Unknown||(Pos.TmCycles()+4)<min_cyc){
	    Mem->LogAction3(10,"Scenario: tm %d is fastest to ball (strong), but he is not in scenario, so stop scenario",Pos.FastestTm());
	    stop=true;
	  }
	}
    }
  if(stop){
    StopScenario();
    return false;
  }
  return true;
}
/////////////////////////////////////////////////////////////////////////////
bool ScenarioSet::StartScenario(){
  static bool loadCheck=false;
  if(!loadCheck){
    loadCheck=true;
    init=InitScenarioSet();
    if(!init)
      my_error("error in scenario init");
  }
  if(!init)
    return false;
  if(scenario_going){
    if(!CheckForStopScenario())
      return false;
    return true;
  }
  if(!Mem->BallPositionValid()||Pos.IsDefense())
    return false;
  vector<Scenario*> ready_scenarios;
  for(unsigned int i=0;i<scenarios.size();i++){
    if(IsValid[i]&&!scenarios[i].CheckTmPresent()){
      scenarios[i].InitScenario();
      if(scenarios[i].CanBegin())
        ready_scenarios.push_back(&scenarios[i]);
    }
  }
  if(ready_scenarios.empty())
    return false;
  sort(ready_scenarios.begin(),ready_scenarios.end(),Scenario_Cmp());
  scenario_going=true;
  current_scenario=ready_scenarios[0];
  Mem->LogAction3(10,"Start scenario %s",const_cast<char*> (current_scenario->GetName().c_str()));
  current_scenario->InitScenario();
  Mem->LogAction3(30,"My number in this scenario is %d",GetTmNumberInScenario(Mem->MyNumber));
  if(!GetTmNumberInScenario(Mem->MyNumber))
    return true;//i`m not in scenario
  if(Pos.FastestTm()==Mem->MyNumber){
    Mem->LogAction2(10,"Scenario: i`m fastest to ball and say that must start scenario");
    SayStartScenario();
  }
  return true;
}
///////////////////////////////////////////////////////////////////
bool ScenarioSet::ExecuteScenario(){
  if(!init)
    return false;
  if(scenario_going){
    if(!GetTmNumberInScenario(Mem->MyNumber)){
      Mem->LogAction3(10,"Scenario %s is going, but i`m not in scenario, so continue",const_cast<char*> (current_scenario->GetName().c_str()));
      return false;
    }
    Mem->LogAction3(10,"Work scenario %s",const_cast<char*> (current_scenario->GetName().c_str()));
    Scenario::RetType ret=current_scenario->Execute();
    if(ret==Scenario::cont)
      return false;
    if(ret==Scenario::exec)
      return true;
    //for stop
    StopScenario();
  }
  return false;
}
