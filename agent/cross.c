/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : cross.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.11 $
 *
 *    $Id: cross.C,v 2.11 2004/06/22 17:06:16 anton Exp $
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


#include "cross.h"
#include "Memory.h"
#include "pass.h"
#include "Handleball.h"
#include <list>
const float Cross::threshold=0.5f;
Cross::Cross(){
}
////////////////////////////////////////////////////////////////////////////////////////////
bool Cross::ThroughPass(){
  float max_conf=0.0f,conf;
  Vector optimal_pos;
  Unum tm=Unum_Unknown;
  Rectangle area;
  if(Mem->BallX()<0.0f)
    return false;
  else if(Mem->BallX()<35.0f)
    area=Rectangle(-20.0f,52.0f,-32.0f,32.0f);
  else
    area=Rectangle(30.0f,52.0f,-32.0f,32.0f);
  for(int i=1;i<=11;i++){
    if(!Mem->TeammatePositionValid(i)||i==Mem->OurGoalieNum||i==Mem->MyNumber||Pos.GetPlayerType(i)<PT_Midfielder)
      continue;
    float vel;
    Vector pos=Pos.GetSmartPassPoint(i,&vel);//Pos.GetTmPos(i);
    if(!area.IsWithin(pos)) continue;
    Unum close_opp=Mem->ClosestOpponentTo(pos);
    if(close_opp!=Unum_Unknown&&Mem->OpponentDistanceTo(close_opp,pos)<2.0f)
      continue;
    Vector target=Pos.GetTmTargetPoint(i);
    Mem->LogAction8(50,"ThroughPass: for tm %.0f orig pos (%.2f, %.2f) conf %.2f; target (%.2f,%.2f)",float(i),
		    pos.x,pos.y,Mem->TeammatePositionValid(i),target.x,target.y);
    Vector p;
    if(pos.x<30.0f)
      p=Vector(pos.x+7.0f,pos.y);//l.get_y(36.0f));
    else
      p=Vector(48.0f,pos.y);//l.get_y(48.0f));
    Line l;l.LineFromTwoPoints(pos,p);
    AngleDeg ang1=(pos-Mem->BallAbsolutePosition()).dir(),ang2=(p-Mem->BallAbsolutePosition()).dir();
    float dist=Max(p.dist(Mem->BallAbsolutePosition()),pos.dist(Mem->BallAbsolutePosition()));
    AngleDeg ang=GetDirectionOfWidestAngle(Mem->BallAbsolutePosition(),Min(ang1,ang2),Max(ang1,ang2),&conf,dist);
    Line l2(Ray(Mem->BallAbsolutePosition(),ang));
    Vector temp_pos=l.intersection(l2);
    Mem->LogAction7(50,"ThroughPass: for tm %.0f: largestAngle is %.2f, target ang %.2f; point is (%.2f,%.2f)",float(i),conf,ang,temp_pos.x,temp_pos.y);
    if(conf>max_conf){
      max_conf=conf;
      optimal_pos=temp_pos;
      tm=i;
    }
  }
  if(max_conf<=15.0f) return false;
  int n=Mem->TeammatePredictedCyclesToPoint(tm,optimal_pos);
  n+=int(n*0.1);//+10%
  float kick_speed=Min(Mem->SP_ball_speed_max,
                       SolveForFirstTermGeomSeries(Mem->SP_ball_decay,n,optimal_pos.dist(Mem->BallAbsolutePosition())));
  float end_speed=kick_speed*Exp(Mem->SP_ball_decay,n);
  Mem->LogAction5(10,"***ThroughPass: n=%.0f; kick_speed=%.2f; end_speed=%.2f",float(n),kick_speed,end_speed);
  end_speed=end_speed<1.0f?1.0f:end_speed;
  end_speed=end_speed>2.0f?2.0f:end_speed;
  Mem->LogAction6(10,"***ThroughPass: make pass to tm %.0f (point (%.2f,%.2f)) with end_speed %.2f",
		  float(tm),optimal_pos.x,optimal_pos.y,end_speed);
  actions.smartkick(Mem->VelAtPt2VelAtFoot(optimal_pos,end_speed),optimal_pos,SK_Fast);
  return true;
}
////////////////////////////////////////////////////////////////////////////////////////////
bool Cross::ThroughPassAtTheirPenaltyArea(){
  const float TRESHOLD=15.0f;
  float max_conf=0.0f,conf;
  Vector optimal_pos;
  Unum tm=Unum_Unknown;
  Rectangle area(33.0f,52.0f,-15.0f,15.0f);
  if(Mem->BallX()<30.0f)
    return false;
  Mem->LogAction2(10,"***Start ThroughPassAtTheirPenaltyArea");
  for(int i=1;i<=11;i++){
    if(Mem->TeammatePositionValid(i)<0.94f||i==Mem->OurGoalieNum||i==Mem->MyNumber||Pos.GetPlayerType(i)!=PT_Forward)
      continue;
    float vel;
    Vector pos=Pos.GetSmartPassPoint(i,&vel);//Pos.GetTmPos(i);
    if(!area.IsWithin(pos)) continue;
    Vector target=Pos.GetTmTargetPoint(i);

    Mem->LogAction8(50,"ThroughPassAtTheirPenaltyArea: for tm %.0f orig pos (%.2f, %.2f) conf %.2f; target (%.2f,%.2f)",
		    float(i),pos.x,pos.y,Mem->TeammatePositionValid(i),target.x,target.y);

    target.x=Min(51.0f,pos.x+5.0f);
    Line l;
    l.LineFromTwoPoints(pos,target);
    AngleDeg ang1=(pos-Mem->MyPos()).dir(),ang2=(target-Mem->MyPos()).dir();
    float dist=Max(target.dist(Mem->BallAbsolutePosition()),pos.dist(Mem->BallAbsolutePosition()));
    AngleDeg ang=GetDirectionOfWidestAngle(Mem->BallAbsolutePosition(),Min(ang1,ang2),Max(ang1,ang2),&conf,dist);
    Line l2(Ray(Mem->BallAbsolutePosition(),ang));
    Vector temp_pos=l.intersection(l2);
    if(temp_pos.x<min(Mem->BallX(),45.0f)||fabs(temp_pos.y-Mem->BallY())<5.0f)
      continue;
    Mem->LogAction7(50,"ThroughPassAtTheirPenaltyArea: for tm %.0f: largestAngle is %.2f, target ang %.2f; point is (%.2f,%.2f)",
		    float(i),conf,ang,temp_pos.x,temp_pos.y);
    
    int n=Mem->TeammatePredictedCyclesToPoint(i,temp_pos);
    n+=int(n*0.1f);//+10%
    float kick_speed=Min(Mem->SP_ball_speed_max,
			 SolveForFirstTermGeomSeries(Mem->SP_ball_decay,Max(1,n),temp_pos.dist(Mem->BallAbsolutePosition())));
    float end_speed=kick_speed*int_pow(Mem->SP_ball_decay,Max(1,n));
    if(end_speed<1.0f||end_speed>2.5f)
      continue;

    if(conf>TRESHOLD&&ang/actions.TeammateShootConf(i)>max_conf){
      max_conf=ang/actions.TeammateShootConf(i);
      optimal_pos=temp_pos;
      tm=i;
    }
  }
  if(tm==Unum_Unknown) return false;
  int n=Mem->TeammatePredictedCyclesToPoint(tm,optimal_pos);
  n+=int(n*0.1f);//+10%
  float kick_speed=Min(Mem->SP_ball_speed_max,
                       SolveForFirstTermGeomSeries(Mem->SP_ball_decay,Max(1,n),optimal_pos.dist(Mem->BallAbsolutePosition())));
  float end_speed=kick_speed*int_pow(Mem->SP_ball_decay,Max(1,n));
  end_speed=end_speed<1.0f?1.0f:end_speed;
  end_speed=end_speed>2.0f?2.0f:end_speed;
  Mem->LogAction6(10,"***ThroughPassAtTheirPenaltyArea: make pass to tm %.0f (point (%.2f,%.2f)) with end_speed %.2f",
		  float(tm),optimal_pos.x,optimal_pos.y,end_speed);
  Mem->LogAction5(10,"***ThroughPassAtTheirPenaltyArea: n=%.0f; kick_speed=%.2f; end_speed=%.2f",float(n),kick_speed,end_speed);
  actions.smartkick(Mem->VelAtPt2VelAtFoot(optimal_pos,end_speed),optimal_pos,SK_Fast);
  return true;
}
////////////////////////////////////////////////////////////////////////////////////////////
//idia of this function from UvA-Trealern 2002
AngleDeg Cross::GetDirectionOfWidestAngle(Vector posOrg,AngleDeg angMin,AngleDeg angMax,AngleDeg* angLargest,float dDist){
  list<float> v1,v2;
  float temp;
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i)) continue;
    if(Mem->OpponentDistanceToBall(i)<dDist){
      Mem->LogAction6(10,"GetDirectionOfWidestAngle:add opp %.0f with conf %.2f, dist %.2f and ang %.2f",
		      float(i),Mem->OpponentPositionValid(i),Mem->OpponentDistanceToBall(i),(Mem->OpponentAbsolutePosition(i)-posOrg).dir());
      v1.push_back((Mem->OpponentAbsolutePosition(i)-posOrg).dir());
    }
  }
  v1.sort();
  AngleDeg angGoalie;
  if(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)&&
     posOrg.x>Mem->SP_pitch_length/4/*&&posOrg.dist(Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum))<(dDist-5.0f)*/){
    angGoalie=(Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum)-posOrg).dir();
    Mem->LogAction5(100,"DirectionWidesAngle: min_ang:%.2f; max_ang: %.2f; goalie_ang: %.2f",angMin,angMax,angGoalie);
    AngleDeg add_ang=33.0f;
    if(Mem->DistanceTo(Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum))<10.0f)
      add_ang=40.0f;
    if(posOrg.y>0){
      angGoalie=GetNormalizeAngleDeg(angGoalie-add_ang);
      angMax=Max(angMin,Min(angGoalie,angMax));
    }else{
      angGoalie=GetNormalizeAngleDeg(angGoalie+add_ang);
      angMin=Min(angMax,Max(angGoalie,angMin));
    }
    Mem->LogAction4(100,"direction of widest angle after min_ang:%.2f; max_ang: %.2f",angMin,angMax);
  }
  float absMin=1000,absMax=1000;
  float angProjMin=angMin,angProjMax=angMax;
  float array[11+2];
  while(v1.size()>0){
    if(fabs(v1.front()-angMin)<absMin){
      absMin=fabs(v1.front()-angMin);
      angProjMin=angMin-absMin;
    }
    if(fabs(v1.front()-angMax)<absMax){
      absMax=fabs(v1.front()-angMax);
      angProjMax=angMax+absMax;
    }
    if(v1.front()>angMin&&v1.front()<angMax)
      v2.push_back(v1.front());
    v1.pop_front();
  }
  v1.push_back(0);
  while(v2.size()>0){
    temp=GetNormalizeAngleDeg(v2.front()-angProjMin)+360.0f;
    if(temp>360) temp-=360;
    v1.push_back(temp);
    v2.pop_front();
  }
  temp=GetNormalizeAngleDeg(angProjMax-angProjMin)+360.0f;
  if(temp>360) temp-=360;
  v1.push_back(temp);
  v1.sort();
  int i=0;
  while(v1.size()>0){
    array[i++]=v1.front();
    v1.pop_front();
  }
  float largest=-1000;
  float d;
  float ang=-777.0f;
  for(int j=0;j<i-1;j++){
    d=GetNormalizeAngleDeg((array[j+1]-array[j])/2);
    if(d>largest){
      ang=GetNormalizeAngleDeg(angProjMin+array[j]+d);
      largest=d;
    }
  }
  if(ang==-777.0f){
    ang=AngleBisect(angMin,angMax);
    if(angLargest!=0)
      *angLargest=largest;
  }else
    if(angLargest!=0)
      *angLargest=largest;
  return ang;
}
////////////////////////////////////////////////////////////////////////////////////////////
float Cross::GetCrossConf(Vector from,Vector to)const{
  Line l;
  l.LineFromTwoPoints(from,to);
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i))
      continue;
    Vector opp_pos=Mem->OpponentAbsolutePosition(i);
    if((from-opp_pos).mod()<=2.0f&&l.dist(opp_pos)>1.0f&&i!=Mem->TheirGoalieNum)
      continue;
    if(l.InBetween(l.ProjectPoint(opp_pos),from,to)&&
       ((l.dist(opp_pos)<2.0f&&i!=Mem->TheirGoalieNum&&opp_pos.x>=to.x)||(l.dist(opp_pos)<4.0f||i==Mem->TheirGoalieNum)))
      return 0.0f;
  }
  return 1.0f;
}
////////////////////////////////////////////////////////////////////////////////////////////
float Cross::PosPriorety(Vector pos)const{//return 0.0f -> 1.0f
  float res=0.0f;
  if(IsPointInGoalCone(pos)) return 0.0f;
  int steps;
  Vector ball=Mem->BallEndPosition(&steps);
  float offside=Mem->my_offside_line;
  offside=ball.x>offside?ball.x:offside;
  if(pos.x>offside) return 0.0f;
  float dx=pos.x;
  if(Mem->TheirGoalieNum==Unum_Unknown||!Mem->OpponentPositionValid(Mem->TheirGoalieNum)){
    res+=fabs(32.0f-float(pos.y))/(32.0f*1.25f);// 0 -> 0.8
  }else{
    Vector goalie=Mem->OpponentAbsolutePosition(Mem->TheirGoalieNum);
    float dif=Max(fabs(goalie.y-7.0)-fabs(pos.y-7.0),fabs(goalie.y+7.0)-fabs(pos.y+7.0));
    dif=dif>14.0f?14.0f:dif;
    dif=dif<-14.0f?-14.0f:dif;
    res+=dif/(14.0f*1.25f);//-0.8 -> 0.8
  }
  dx=dx<35.0f?35.0f:dx;
  dx=dx>52.0f?52.0f:dx;
  res+=(dx-35.0f)/(52.0f*5.0f);//res + 0.0 -> 0.2
  res=res<0.0?0.0:res;
  res=res>1.0?1.0:res;
  return res;
}
////////////////////////////////////////////////////////////////////////////////////////////
bool Cross::IsPointInGoalCone(Vector point)const{
  static Line l1,l2;
  static bool first=true;
  int num_cyc;
  if(first){
    first=false;
    l1.LineFromTwoPoints(Mem->BallAbsolutePosition(),
			 Mem->MySide=='l'  ? Mem->MarkerPosition( Flag_GRT ) : Mem->MarkerPosition( Flag_GLB ));
    l2.LineFromTwoPoints(Mem->BallAbsolutePosition(),
			 Mem->MySide=='l' ? Mem->MarkerPosition( Flag_GRB ) : Mem->MarkerPosition( Flag_GLT ));
  }
  AngleDeg ang=(point-Mem->BallEndPosition(&num_cyc)).dir();
  return Min(l1.angle(),l2.angle())<=ang&&Max(l1.angle(),l2.angle())>=ang;
}
////////////////////////////////////////////////////////////////////////////////////////////
void Cross::FillMatrix(Unum tm){
  Unum opp;
  int num_cyc;
  Vector ball=Mem->BallEndPosition(&num_cyc);
  int opp_cyc=1000;
  if(Pos.OppInter())
    opp_cyc=int((Mem->OpponentAbsolutePosition(Pos.FastestOpp())-ball).mod()/Mem->GetOpponentPlayerSpeedMax(Pos.FastestOpp())+2);
  for(int y=0;y<YMAX;y++){
    for(int x=0;x<XMAX;x++){
      Vector point(GetBeginX()+x,GetBeginY()+y);
      //step 1 and 2
      if(/*IsPointInGoalCone(point)||*/Mem->ClosestTeammateTo(point)!=tm/*||(Pos.GetTmPos(tm)-point).mod()>opp_cyc*Mem->GetTeammatePlayerSpeedMax(tm)*/)
        matrix[y][x]=0.0f;
      else
        matrix[y][x]=actions.PassFromPoint2Point(ball,point,opp)-threshold;
      //step 3
      if(matrix[y][x]>0.0)
        matrix[y][x]+=1-abs(y-YMAX/2)*2/YMAX+1-x/XMAX;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
float Cross::GetOptimalPoint(Unum tm,Vector& point){
  return 0;
  FillMatrix(tm);
  float max=0.0f;
  for(int y=0;y<YMAX;y++){
    for(int x=0;x<XMAX;x++){
      if(matrix[y][x]>max){
        max=matrix[y][x];
        point=Vector(GetBeginX()+x,GetBeginY()+y);
      }
    }
  }
  return max;
}
////////////////////////////////////////////////////////////////////////////////////////////
void Cross::StartCross() {
  bool fast=(Mem->TheirGoalieNum!=Unum_Unknown&&Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.98f&&
	     Mem->OpponentDistanceToBall(Mem->TheirGoalieNum)<4.5f)||Mem->ClosestOpponentToBallDistance()<2.0f;
  Mem->LogAction5(10,"cross: crossing to pos (%.2f,%.2f) (dist to goalie:%.2f)",
		  target.x,target.y,Mem->OpponentPositionValid(Mem->TheirGoalieNum)?Mem->OpponentDistanceToBall(Mem->TheirGoalieNum):100.0f);
  smartkick(Mem->VelAtPt2VelAtFoot(target,speed),GetKickAngle(target), fast?SK_Fast:SK_Safe);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Cross::cross_angle(AngleDeg& ang){
  Vector opt_pos;
  Unum opt_tm=Unum_Unknown;
  float max_val=0.0f;
  Vector ball=Mem->BallAbsolutePosition();
  bool can_cross=false;
  float vel,opt_vel;
  
  speed=1.4f;
  
  for(int i=1;i<=11;i++){
    if(!Mem->TeammatePositionValid(i)||i==Mem->MyNumber)
      continue;
    Vector pos=Pos.GetSmartPassPoint(i,&vel);
    
    if((ball.x-pos.x)<15.0f)
      can_cross=true;
    if(!Mem->TheirPenaltyArea.expand(IsGoalieActive()?6.0f:0.0f).IsWithin(pos))
      continue;
    if(GetCrossConf(ball,pos)&&max_val<PosPriorety(pos)){
      max_val=PosPriorety(pos);
      opt_pos=pos;
      opt_tm=i;
      opt_vel=vel;
    }
  }
  if(max_val>0.0f){
    ang=opt_pos.dir();
    target=opt_pos;
    speed=opt_vel;
    Mem->LogAction6(10,"Cross::max_val:%.2f; pos: (%.2f,%.2f); tm - %.0f",max_val,target.x,target.y,float(opt_tm));
    return true;
  }
  if((fabs(ball.y)<=4.0f&&ball.x>45.0f)||!can_cross){
    return false;
  }
  if(ball.x<40){
    bool active=IsGoalieActive();
    target=Vector(ball.x+1.0f,0.0f);
    if(active)
      target=Vector(ball.x,-Sign(ball.y)*20.0f);
    ang=target.dir();
    Mem->LogAction4(10,"Cross::x<40; pos=(%.2f,%.2f)",target.x,target.y);
    if(!active)
      speed=2.0f;
	
    return true;
  }
  if((ball.x<47&&ball.x>=40)||fabs(ball.y)<8.0f){
    target=Vector(ball.x,-Sign(ball.y)*20.0f);
    ang=target.dir();
    Mem->LogAction4(10,"Cross::x<47&&x>=40; pos=(%.2f,%.2f)",target.x,target.y);
    return true;
  }
  if(ball.x<=52.5&&ball.x>=47){
    if(Mem->OpponentPositionValid(Mem->TheirGoalieNum)>0.95f){
      Line l;
      float goal_buf=MinMax(3.0f,ball.x-Mem->OpponentX(Mem->TheirGoalieNum),7.0f);
      l.LineFromTwoPoints(ball,Vector(Mem->OpponentX(Mem->TheirGoalieNum)-goal_buf,Mem->OpponentY(Mem->TheirGoalieNum)));
      Mem->LogAction3(10,"Cross: selct goal_buf=%.2f",
		      goal_buf);
      target=Vector(l.get_x(-Sign(ball.y)*20.0f),
		    -Sign(ball.y)*20.0f);
    }else{
      target=Vector(45.0,-Sign(ball.y)*20.0f);
    }    
    ang=target.dir();
    speed=2.0f;
    Mem->LogAction4(10,"Cross::x<52.5&&x>=47; pos=(%.2f,%.2f)",target.x,target.y);
    return true;
  }
  Mem->LogAction2(10,"Cross:: all is close!!!");
  return false;
}
///////////////////////////////////////////////////////////
bool Cross::open_for_cross(Unum tm,Offense::Holder& holder){//call from offense
  Unum opp;
  int num_cyc;
  bool is_i=Mem->MyNumber==tm;
  Vector ball=Mem->BallEndPosition(&num_cyc),target;
  if(ball.x<GetBeginX()||Pos.GetPlayerType(tm)<PT_Forward)
    return false;
  if(actions.PassFromPoint2Point(ball,Pos.GetTmPos(tm),opp)>=threshold){
    if(is_i)
      Mem->LogAction2(10,"Cross: conf of pass is ok, so return false");
    return false;
  }
  statistica::TimeCounter tc;
  tc.Start();
  if(GetOptimalPoint(tm,target)>0.0){
    if(is_i){
      tc.Finish();
      Mem->LogAction5(10,"Cross:go to point (%.2f,%.2f); time %f",target.x,target.y,tc.GetTime());
      Pos.MoveToPos(target,15.0f,3.0f);
      return true;
    }
    holder.type=Offense::t_go_to_pos;
    holder.ang_error=15.0f;
    holder.pos=target;
    return true;
  }
  if(is_i){
    tc.Finish();
    Mem->LogAction3(10,"Cross:all points is 0 !!! (time %f)",tc.GetTime());
  }
  return false;
}
