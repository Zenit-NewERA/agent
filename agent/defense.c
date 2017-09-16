/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : defense.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.25 $
 *
 *    $Id: defense.C,v 2.25 2004/06/22 17:06:16 anton Exp $
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
/* -*- Mode: C++ -*- */

#include "behave.h"
#include "geometry.h"
#include "Memory.h"
#include "client.h"
#include "types.h"
#include "kick.h"
#include "dribble.h"

#include "defense.h"

const float Defense::max_dist_than_start_mark=10.0f;
const float Defense::penalty_area_treshold=-33.0f;
///////////////////////////////////////////////////////
Defense::Defense(){
  pressing_time=-1;
  pressing_opponent=Unum_Unknown;
  last_marking_time=-1;
  start_rest=-1;
  cycles_to_rest=0;
}
//////////////////////////////////////////////////////////
Unum Defense::GetOptimalTmToBallFromSet(int pt,int ps)
{
  Iterator iter=begin();
  Unum ClosestPlayer = Unum_Unknown;
  float min_dist=100.0f,dist;
  if(Mem->BallVelocityValid()){
    while(iter!=end()){
      Unum i=GetPlayerNumber(iter,pt,ps);
      if(i==Unum_Unknown)
	break;
      if ( Mem->TeammatePositionValid(i) && (dist=Mem->TeammateDistanceTo(i,Mem->BallEndPosition())) < min_dist&&
	   (i != Mem->FP_goalie_number || Mem->CP_goalie) ){
	min_dist =dist;
	ClosestPlayer = i;
      }
    }
  }
  return ClosestPlayer;
}
//////////////////////////////////////////////////////////////////////
bool Defense::PressingWithoutDefenders()
{
  const float MAX_DIST_TO_PRESSING=15.0f;
  Unum ClosestPlayer = GetOptimalTmToBallFromSet(PT_Midfielder|PT_Forward,PS_All);
  float max_dash=Mem->SP_max_power;
  
  if(Mem->DistanceTo(Mem->BallEndPosition())>MAX_DIST_TO_PRESSING&&Mem->BallX()<Mem->MyX()&&Mem->MyStamina()<Mem->SP_stamina_max*0.6f)//hack
    max_dash=GetFormationDash();
  Mem->LogAction3(10,"Fastest player to ball from midfielders and forwards is %.0f",
		  float(ClosestPlayer));
  if(GetPlayerType()==PT_Forward&&Mem->MyStamina()<Mem->SP_stamina_max*0.7f/*&&Mem->BallX()+10.0f<Mem->MyX()*/){
    Mem->LogAction2(10,"I forward and tired to go pressing");
    return false;
  }
  
  if(ClosestPlayer==Mem->MyNumber){
    Mem->LogAction3(10,"I`m closest to fastest opp %.0f so go to ball",float(FastestOpp()));
    DefenseGetBall(max_dash);
    return true;
  }
  if(Mem->ClosestTeammateToBall()==Mem->MyNumber){
    Mem->LogAction2(10,"I`m closest to ball so go to it");
    DefenseGetBall(max_dash);
    return true;
  }
    
  if(ClosestPlayer==Unum_Unknown)
    if(SelectOptimalPlayer(PT_Midfielder|PT_Forward)==Mem->MyNumber){
      Mem->LogAction2(10,"I don`t know fastest opp, so go to ball");
      DefenseGetBall(max_dash);
      return true;
    }
  return false;
}
///////////////////////////////////////////////////////////////////////
bool Defense::GoToActiveDefense(){
  Unum tm=Mem->FastestTeammateToBall();
  if(Mem->OpponentWithBall()!=Unum_Unknown&&Mem->ClosestTeammateToBall()==Mem->MyNumber){
    Mem->LogAction2(10,"GoToActiveDefense: ball is kickable for opponent and i closest, so go to it");
    DefenseGetBall();
    return true;
  }
  if(Mem->MyInterceptionAble()){
    if(tm==Mem->MyNumber){
      Mem->LogAction2(10,"GoToActiveDefense:i am fastest and i go to ball");
      DefenseGetBall();
      return true;  	
    }
    //    if(tm!=Unum_Unknown&&tm!=Mem->MyNumber&&Mem->TeammateInterceptionAble(tm)&&
    //      Mem->TeammateInterceptionNumberCycles(tm)>5&&Mem->NumTeammatesCloserToBall()<=1){
    //      Mem->LogAction3(10,"GoToActiveDefense:i am second to ball, so go to it (first %d)",int(tm));
    //      DefenseGetBall();
    //      return true;
    //    }
  }
  return false;
}
///////////////////////////////////////////////////////////////////////
bool Defense::CanGoToActiveDefense(Unum* opp){
  if(GetMyType()>PT_Defender)
    return true;
  if(Mem->MyStamina()<=0.5f*Mem->SP_stamina_max)
    return false;
  Vector home_pos=GetHomePosition();
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i)) continue;
    Vector opp_pos=Mem->OpponentAbsolutePosition(i);
    if(FastestOpp()==i||opp_pos.x>(Mem->BallX()+2.0))
      continue;
    if((opp_pos-home_pos).mod()>max_dist_than_start_mark&&
       (opp_pos.x>home_pos.x||fabs(opp_pos.y-home_pos.y)>15.0f))
      continue;
    Mem->LogAction3(30,"CanGoToActiveDefense: opp %d is danger so can`t go to active_defense",i);
    if(opp!=0)
      *opp=i;
    return false;
  }
  if((GetOptimalTmToBallFromSet(PT_Defender|PT_Sweeper,PS_All)!=Mem->MyNumber&&Mem->ClosestTeammateToBall()!=Mem->MyNumber)||
     (OppInter()&&Mem->MyInterceptionAble()&&OppCycles()<Mem->MyInterceptionNumberCycles()+2&&
      (Mem->BallEndPosition()-GetHomePosition()).mod()>max_dist_than_start_mark))
    return false;
  return true;
}
///////////////////////////////////////////////////////////////////////
Unum Defense::SelectWingMarkOpp(Unum tm,Vector homePos,float off_thr,bool& at_offside){
  Unum OppAtOffside=Unum_Unknown;
  float distOppAtOffside=100.0f;
  Unum optOpp=Unum_Unknown;
  float optVal=100.0f;
  Line l;
  l.LineFromTwoPoints(homePos,Vector(homePos.x,77.0f));

  set<Unum> opp_set;
  opp_set=GetNotBlockedOpponents(tm,homePos);
  if(opp_set.empty())
    for(int i=1;i<=11;i++){
      if(!Mem->OpponentPositionValid(i))
	continue;
      opp_set.insert(i);
    }
  
  for(set<Unum>::const_iterator i=opp_set.begin();i!=opp_set.end();i++){
    Unum teammate=GetDefenderWithClosestHomePos(l.ProjectPoint(Mem->OpponentAbsolutePosition(*i)),homePos);
    if(teammate!=tm){
      if(tm==Mem->MyNumber)
	Mem->LogAction4(10,"SelectWingMarkOpp: opp %.0f must mark by tm %.0f",
			float(*i),float(teammate));
      continue;
    }
    float opp_x=Mem->OpponentAbsolutePosition(*i).x;
    if((IsOffense()&&(homePos.x-off_thr)>opp_x)||
       (IsDefense()&&((Mem->their_offside_line-off_thr)>opp_x||(homePos.x-off_thr-5.0f)>opp_x))){
      if(tm==Mem->MyNumber)
	Mem->LogAction3(50,"SelectWingMarkOpp: think that opp %d in offside position",*i);
      if((Mem->OpponentAbsolutePosition(*i)-homePos).mod()<distOppAtOffside){
	distOppAtOffside=(Mem->OpponentAbsolutePosition(*i)-homePos).mod();
	OppAtOffside=*i;
      }
      continue;
    }
    Vector opp=Mem->OpponentAbsolutePosition(*i);
    if(fabs(opp.x-homePos.x)>max_dist_than_start_mark&&
       fabs(opp.x-Mem->TeammateX(teammate))>max_dist_than_start_mark*0.8f){
      if(tm==Mem->MyNumber)
	Mem->LogAction5(10,"SelectWingMarkOpp: ignore opp %.0f, becouse of dist (to_home=%.2f; to_me=%.2f)",
			float(*i),fabs(opp.x-homePos.x),fabs(opp.x-Mem->TeammateX(teammate)));
      continue;
    }
    
    float opt_temp=1/fabs(Mem->OpponentY(*i));
    if(opt_temp<optVal){
      optVal=opt_temp;
      optOpp=*i;
    }
  }
  if(OppAtOffside!=Unum_Unknown&&optOpp!=Unum_Unknown&&Mem->their_offside_line<=Mem->OpponentX(OppAtOffside)){
    Mem->LogAction3(10,"SelectWingMarkOpp: opp %d may be not in offside, so mark him",OppAtOffside);
    at_offside=true;
    return OppAtOffside;
  }
  if(optOpp!=Unum_Unknown){
    at_offside=false;
    return optOpp;
  }
  if(OppAtOffside!=Unum_Unknown)
    at_offside=true;
  return OppAtOffside;
}
///////////////////////////////////////////////////////////////////////
Unum Defense::SelectCentralMarkOpp(Unum teammate,Vector homePos,float off_thr,bool& at_offside){
  Unum OppAtOffside=Unum_Unknown;
  float distOppAtOffside=100.0f;
  Unum optOpp=Unum_Unknown;
  float optVal=-1.0f;

  Line l;
  l.LineFromTwoPoints(homePos,Vector(homePos.x,77.0f));

  set<Unum> opp_set;
  opp_set=GetNotBlockedOpponents(teammate,homePos);
  if(opp_set.empty())
    for(int i=1;i<=11;i++){
      if(!Mem->OpponentPositionValid(i))
	continue;
      opp_set.insert(i);
    }
  
  for(set<Unum>::const_iterator i=opp_set.begin();i!=opp_set.end();i++){
    Unum tm=GetDefenderWithClosestHomePos(l.ProjectPoint(Mem->OpponentAbsolutePosition(*i)),homePos);
    if(tm!=teammate){
      if(teammate==Mem->MyNumber)
	Mem->LogAction4(10,"SelectCentralMarkOpp: opponent %.0f must mark tm %.0f",
			float(*i),float(tm));
      continue;
    }
    float opp_x=Mem->OpponentAbsolutePosition(*i).x;
    if((IsOffense()&&(homePos.x-off_thr)>opp_x)||
       (IsDefense()&&((Mem->their_offside_line-off_thr)>opp_x||(homePos.x-off_thr-5.0f)>opp_x))){
      if(teammate==Mem->MyNumber)
	Mem->LogAction3(50,"SelectCentralMarkOpp: think that opp %d in offside position",*i);
      if((Mem->OpponentAbsolutePosition(*i)-homePos).mod()<distOppAtOffside){
	distOppAtOffside=(Mem->OpponentAbsolutePosition(*i)-homePos).mod();
	OppAtOffside=*i;
      }
      continue;
    }
    if(fabs(Mem->OpponentX(*i)-homePos.x)>max_dist_than_start_mark&&
       fabs(Mem->OpponentX(*i)-Mem->TeammateX(teammate))>max_dist_than_start_mark*0.8f){
      if(teammate==Mem->MyNumber)
	Mem->LogAction5(10,"SelectCentralMarkOpp: ignore opp %.0f, becouse of dist (to_home=%.2f; to_me=%.2f)",
			float(*i),fabs(Mem->OpponentX(*i)-homePos.x),fabs(Mem->OpponentX(*i)-Mem->TeammateX(teammate)));
      continue;
    }
    Iterator iter=begin();
    tm=GetPlayerNumber(iter,PT_Defender|PT_Sweeper,Mem->BallY()>0?PS_Left:PS_Right);
    if(tm==Unum_Unknown)
      my_error("SelectCentralMarkOpp:Have no defender of right wing");
    else{
      if(SelectWingMarkOpp(tm,Vector(homePos.x,GetHomePosition(tm).y),off_thr,at_offside)==*i){
	if(teammate==Mem->MyNumber)
	  Mem->LogAction4(10,"SelectCentralMarkOpp:Don`t mark opp %.0f becouse wing tm %.0f must mark him",
			  float(*i),float(tm));
      }else{
	float ocenka=TheirPositionValue(Mem->OpponentAbsolutePosition(*i));
	if(teammate==Mem->MyNumber)
	  Mem->LogAction4(10,"SelectCentralMarkOpp: for opp %.0f ocenka=%.4f",
			  float(*i),ocenka);
	if(ocenka>optVal){
	  optVal=ocenka;
	  optOpp=*i;
	}
      }
    }
  }
  if(optOpp!=Unum_Unknown){
    at_offside=false;
    return optOpp;
  }
  if(OppAtOffside!=Unum_Unknown)
    at_offside=true;
  return OppAtOffside;
}
///////////////////////////////////////////////////////////////////////
bool Defense::WingDefenderMark(Vector homePos,bool at_begin_pos){
  if(!(GetMyType()<=PT_Defender&&GetMySide()!=PS_Center))
    return false;
  if((IsRest()&&Mem->BallX()-Mem->MyX()>15.0f)||IsOffense()||Mem->BallX()>0.0f)
    return false;
  bool at_offside;//init in SelectWingMarkOpp
  float off_thr=0.0f;
  if(!at_begin_pos){
    off_thr=2.0;
  }
  Unum tm1,tm2;
	
  Unum optOpp=SelectWingMarkOpp(Mem->MyNumber,homePos,off_thr,at_offside);
  if(optOpp==Unum_Unknown)
    return false;
  Get2Defenders(homePos,tm1,tm2);
  Mem->LogAction4(10,"WingDefenderMark: ball is between %.0f and %.0f teammates",float(tm1),float(tm2));
  if(tm1==Mem->MyNumber||tm2==Mem->MyNumber||FastestTm()==Mem->MyNumber){//then ball is between me and orther teammate
    Unum tm_block=GetDefenderWithClosestHomePos(Mem->BallAbsolutePosition(),homePos);
    if(tm_block!=Unum_Unknown&&tm_block!=Mem->MyNumber){
      Mem->LogAction3(10,"WingDefenderMark: tm %d must block ball",tm_block);
    }else{
      if(FastestOpp()==optOpp&&(CanGoToActiveDefense()||homePos.x>Mem->OpponentX(optOpp)-5.0f)){
	Mem->LogAction3(10,"WingDefenderMark:: opp %d is fastest, so block him",optOpp);
	DefenseGetBall();
	return true;
      }
      float coef1=1/(Max(0.001,fabs(homePos.x-Mem->BallX()))*0.5f),coef2=1/Max(0.001,fabs(homePos.x-Mem->OpponentX(optOpp)));
      float y=Mem->BallY()+Sign(Mem->OpponentY(optOpp)-Mem->BallY())*
	((coef2/coef1)*fabs(Mem->BallY()-Mem->OpponentY(optOpp)))/(1+coef2/coef1);
      y=MinMax(-32.0f,y,32.0f);
      Mem->LogAction6(10,"WingDefenderMark: stand between ball and opp %.0f; coef1=%.4f;coef2=%.4f;y=%.2f",
		      float(optOpp),coef1,coef2,y);
      if(!MoveToPos(Vector(homePos.x,y),30.0,0.0,false))
	face_only_body_to_ball();
      eye.AddOpponent(optOpp,1);
      eye.AddBall(1);
      return true;
    }
  }
	
  if(fabs(Mem->BallY()-Mem->MyY())>30.0f){
    Mem->LogAction2(10,"WingDefenderMark: ball is on other wing, so can rest");
    float dash=Mem->GetMyStaminaIncMax()*Mem->MyStamina()/Mem->SP_stamina_max;
    MoveToPos(homePos,5.0f,0.0f,dash);
    return true;
  }
  if(at_offside){
    if(Mem->OpponentX(optOpp)-Mem->their_offside_line<-4.0f||fabs(Mem->BallY()-Mem->OpponentY(optOpp))>7.0f){
      Mem->LogAction3(20,"WingDefenderMark: opp %.0f is too far in offside or ball is too far",float(optOpp));
      return false;
    }
    if(fabs(Mem->BallY()-Mem->MyY())>15.0f&&(Mem->MyX()-homePos.x)>=0.0f&&
       Pos.TheirPositionValue(Mem->MyPos())>=Pos.TheirPositionValue(Mem->OpponentAbsolutePosition(optOpp))){
      Mem->LogAction2(20,"WingDefenderMark: can rest if tired at offside opp");
      face_only_body_to_ball();
      return true;
    }
    Mem->LogAction3(20,"WingDefenderMark: try mark opp %.0f at offside",float(optOpp));
    Line l;
    l.LineFromTwoPoints(Mem->OpponentAbsolutePosition(optOpp),Mem->BallAbsolutePosition());
    if(!MoveToPos(Vector(homePos.x,l.ProjectPoint(homePos).y),30.0,0.0,false))
      face_only_body_to_ball();
    eye.AddOpponent(optOpp,5,RT_WithTurn);
    eye.AddBall(1);
    return true;
  }
	
  if(!at_begin_pos){
    Mem->LogAction4(20,"WingDefenderMark: try mark opp %.0f (conf: %.2f) in central zone",float(optOpp),Mem->OpponentPositionValid(optOpp));
    float x= homePos.x;
    float y=Mem->OpponentY(optOpp)-Sign(Mem->OpponentY(optOpp))*1.0;
    if(Mem->MyX()>=Mem->OpponentX(optOpp)){
      y=Mem->MyY();
      important_action=true;
    }else{
      if(fabs(Mem->BallY()-Mem->MyY())>15.0f&&(Mem->MyX()-homePos.x)>=0.0f&&
	 Pos.TheirPositionValue(Mem->MyPos())>=Pos.TheirPositionValue(Vector(x,y))){
	Mem->LogAction2(20,"WingDefenderMark: can rest if tired");
	face_only_body_to_ball();
	return true;
      }
    }

    if(!MoveToPos(Vector(x,y),30.0,0.0,false))
      face_only_body_to_ball();
    eye.AddOpponent(optOpp,5,RT_WithTurn);
    eye.AddBall(1);
    return true;
  }
	
  MarkType type=mark_bisector;
  if(fabs(Mem->OpponentAbsolutePosition(optOpp).y-Mem->BallAbsolutePosition().y)<15.0)
    type=mark_ball;
  float old=buffer_at_point;
  buffer_at_point=2.0f;
  MarkOpponent(optOpp,type,3.0f);
  eye.AddOpponent(optOpp,5,RT_WithTurn);
  eye.AddBall(1);
  buffer_at_point=old;
  return true;
}
///////////////////////////////////////////////////////////////////////
void Defense::Get2Defenders(Vector homePos,Unum& tm1,Unum& tm2){
  Vector pos=Mem->BallAbsolutePosition();
  Line l;l.LineFromTwoPoints(Vector(homePos.x,0.0),Vector(homePos.x,7.0));
  Vector pr_dot=l.ProjectPoint(pos);
  //think that homePos.x is shared, but homePos.y is individual
  Iterator iter=begin();
  float dist1=100.0f,dist2=100.0f;
  tm1=tm2=Unum_Unknown;
  while(iter!=end()){
    Unum tm=GetPlayerNumber(iter,PT_Defender|PT_Sweeper);
    if(tm==Unum_Unknown)
      break;
    if((pr_dot-Vector(homePos.x,GetHomePosition(tm).y)).mod()<dist1){
      tm1=tm;
      dist1=(pr_dot-Vector(homePos.x,GetHomePosition(tm).y)).mod();
      if(dist1<dist2){
	dist1=dist2;
	dist2=(pr_dot-Vector(homePos.x,GetHomePosition(tm).y)).mod();
	tm1=tm2;
	tm2=tm;
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////
set<Unum> Defense::GetNotBlockedOpponents(Unum teammate,Vector homePos)
{
  set<Unum> res;
  
  for(int opp=1;opp<=11;opp++){
    if(Mem->OpponentPositionValid(opp)<0.6f||Mem->OpponentX(opp)-homePos.x>20.0f)
      continue;
    Iterator iter=begin();
    bool blocked=false;
    while(iter!=end()){
      Unum tm=GetPlayerNumber(iter,PT_Sweeper|PT_Defender);
      if(tm==Unum_Unknown)
	break;
      if(tm==teammate)
	continue;
      if(blocked=IsTmBlockTarget(tm,Vector(homePos.x,GetHomePosition(tm).y),Mem->OpponentAbsolutePosition(opp))){
	Mem->LogAction4(30,"GetNotBlockedOpponents: opponent %.0f is blocked by %.0f tm",
			float(opp),float(tm));
	break;
      }
    }
    if(!blocked){
      Mem->LogAction3(30,"GetNotBlockedOpponents: opponent %.0f is NOT blocked by any tm",
		      float(opp));
      res.insert(opp);
    }
  }  
  return res;  
}

///////////////////////////////////////////////////////////////////////
Unum Defense::GetDefenderWithClosestHomePos(Vector target,const Vector& homePos){
  Iterator iter=begin();
  Unum closest=Unum_Unknown;
  float dist=100.0f;
  while(iter!=end()){
    Unum tm=GetPlayerNumber(iter,PT_Sweeper|PT_Defender);
    if(tm==Unum_Unknown) break;
    if((target-Vector(homePos.x,GetHomePosition(tm).y)).mod()<dist&&
       (Mem->ClosestTeammateTo(target)==tm||IsTmBlockTarget(tm,Vector(homePos.x,GetHomePosition(tm).y),target))){
      dist=(target-Vector(homePos.x,GetHomePosition(tm).y)).mod();
      closest=tm;
    }
  }
  return closest;
}
///////////////////////////////////////////////////////////////////////
bool Defense::IsTmBlockTarget(Unum tm,Vector homePos,Vector target){
  if(!Mem->TeammatePositionValid(tm)) return false;
  Vector tm_pos=Mem->TeammateAbsolutePosition(tm);
  Line l;l.LineFromTwoPoints(homePos,Vector(homePos.x,77.0));
  Vector pr_point=l.ProjectPoint(target);
  if((tm_pos-pr_point).mod()+2.0f<(target-pr_point).mod()||(tm_pos-pr_point).mod()<2.0f)
    return true;
  else
    return false;
}
///////////////////////////////////////////////////////////////////////
Unum Defense::SelectCentralBallMark(const Vector& homePos,Unum tm){//second is me
  Line l;l.LineFromTwoPoints(homePos,Vector(homePos.x,homePos.y+7.0));
  Vector pr_dot=l.ProjectPoint(Mem->BallAbsolutePosition());
  Vector target(homePos.x,pr_dot.y);
  if(Mem->TeammatePositionValid(tm)<=0.9)
    return Mem->MyNumber;
  Vector myPos=homePos;
  if((myPos-target).mod()-1.0f<=(Vector(homePos.x,GetHomePosition(tm).y)-target).mod())
    return Mem->MyNumber;
  return tm;
}
///////////////////////////////////////////////////////////////////////
bool Defense::PassiveCentralDefense(Vector homePos){//must go to absolute position
  const float mark_dist=0.0f;//was 3.5f

  if(GetMyType()>PT_Defender||GetMySide()!=PS_Center)
    return false;
  if((IsRest()&&Mem->BallX()-Mem->MyX()>15.0f)||IsOffense()||Mem->BallX()>0.0f)
    return false;
  Unum tm1,tm2;
  bool at_offside;
  Unum opp=SelectCentralMarkOpp(Mem->MyNumber,homePos,2.0,at_offside);
  if(opp==Unum_Unknown)
    return false;
  Get2Defenders(homePos,tm1,tm2);
  Mem->LogAction4(10,"PassiveCentralDefense: ball is between %.0f and %.0f teammates",float(tm1),float(tm2));
  if(tm1==Mem->MyNumber||tm2==Mem->MyNumber||FastestTm()==Mem->MyNumber){//then ball is between me and other teammate
    Iterator iter=begin();
    Unum tm=GetPlayerNumber(iter,PT_Defender|PT_Sweeper,Mem->GetBallSide());
    if(tm==Unum_Unknown)
      my_error("Have no defender of target wing in PassiveCentralDefense");
    else
      if(SelectWingMarkOpp(tm,Vector(homePos.x,GetHomePosition(tm).y),2.0,at_offside)==FastestOpp()&&Mem->TeammatePositionValid(tm)>0.9f){
	Mem->LogAction3(10,"PassiveCentralDefense: tm %d must care about ball in this situation",int(tm));
      }else{
	if(FastestOpp()==opp){
	  Mem->LogAction3(10,"WingDefenderMark:: opp %d is fastest, so block him",opp);
	  DefenseGetBall();
	  return true;
	}
	Line l;l.LineFromTwoPoints(homePos,Vector(homePos.x,homePos.y+7.0));
	Vector pr_dot=l.ProjectPoint(Mem->BallAbsolutePosition());
	if(SelectCentralBallMark(homePos,tm1==Mem->MyNumber?tm2:tm1)!=Mem->MyNumber&&
	   Mem->DistanceTo(pr_dot)>Mem->TeammateDistanceTo(tm1==Mem->MyNumber?tm2:tm1,pr_dot)){
	  Mem->LogAction3(10,"PassiveCentralDefense: tm %d must care about ball in this situation",int(tm1==Mem->MyNumber?tm2:tm1));
	}else{
	  float coef1=1/(Max(0.001,fabs(homePos.x-Mem->BallX()))*0.3f),coef2=1/Max(3.0f,fabs(homePos.x-Mem->OpponentX(opp)));
	  float y=Mem->BallY()+Sign(Mem->OpponentY(opp)-Mem->BallY())*
	    ((coef2/coef1)*fabs(Mem->BallY()-Mem->OpponentY(opp)))/(1+coef2/coef1);
	  Mem->LogAction6(10,"CentralDefnseMark: stand between ball and opp %.0f; coef1=%.4f;coef2=%.4f;y=%.2f",
			  float(opp),coef1,coef2,y);
	  if(!MoveToPos(Vector(homePos.x,y),30.0,0.0,false))
	    face_only_body_to_ball();
	  eye.AddOpponent(opp,5,RT_WithTurn);
	  eye.AddBall(1);
	  return true;
	}
      }
  }
  if(at_offside)
    return false;
  Mem->LogAction4(10,"PassiveCentralDefense: must mark opponent %.0f (pos conf: %.2f)",float(opp),Mem->OpponentPositionValid(opp));
  float x= homePos.x;
  float y=Mem->OpponentY(opp);
  if(x+mark_dist>Mem->OpponentX(opp))
    x=Mem->OpponentX(opp)-mark_dist;
  if(Mem->MyX()>Mem->OpponentX(opp)){
    y=Mem->MyY();
    important_action=true;
    if(!MoveToPos(Vector(x,y),30.0,0.0,false))
      face_only_body_to_ball();
    eye.AddOpponent(opp,1);
    eye.AddBall(1);
    return true;
  }
  float coef1=1/Max(0.001,fabs(homePos.y-Mem->MyY())),coef2=1/Max(0.001,fabs(homePos.x-Mem->OpponentX(opp)));
  float soft_y=Mem->OpponentY(opp)+Sign(Mem->OpponentY(opp)-Mem->MyY())*
    ((coef2/coef1)*fabs(homePos.y-Mem->MyY()))/(1+coef2/coef1);
  Mem->LogAction6(10,"CentralDefnseMark: try softly mark opp %.0f; coef1=%.4f;coef2=%.4f;y=%.2f",
		  float(opp),coef1,coef2,soft_y);
  float dash=Mem->OpponentDistanceToBall(opp)>15.0f?Mem->GetMyStaminaIncMax():Mem->SP_max_power;
  if(!MoveToPos(Vector(homePos.x,soft_y),30.0,0.0,dash))
    face_only_body_to_ball();
  eye.AddOpponent(opp,5,RT_WithTurn);
  eye.AddBall(1);
  return true;
}
//////////////////////////////////////////////////////////////////////
void Defense::BeginDef(){  
  if(GetMyType()==PT_Forward&&Mem->MyStamina()<Mem->SP_stamina_max*0.7f&&!Mem->TheirPenaltyArea.expand(5.0f).IsWithin(Mem->BallAbsolutePosition())){
    Mem->LogAction2(10,"Forward tired and can rest");
    return;
  }
  if(CanGoToActiveDefense()&&GoToActiveDefense())
    return;
  GoToHomePosition(GetHomePosition(),45.0f);
}
////////////////////////////////////////////////////////////
bool Defense::BlockMovingOpponent(Unum opp){
  Vector opp_pos=Mem->OpponentAbsolutePosition(opp);
  Vector target=Mem->BallAbsolutePosition();
  if(Mem->OpponentVelocityValid(opp)>0.95f&&Mem->OpponentSpeed(opp)>0.2f){
    Vector opp_vel=Mem->OpponentAbsoluteVelocity(opp);
    Mem->LogAction5(10,"BlockMovingOpponent:Opponent has speed (%.2f,%.2f) with conf %.2f",
		    opp_vel.x,opp_vel.y,Mem->OpponentVelocityValid(opp));
    target=Mem->FieldRectangle.RayIntersection(Ray(opp_pos,opp_vel));
  }
  Line l;l.LineFromTwoPoints(target,opp_pos);
  Vector res;
  Vector my_body_int;
  if(l.RayIntersection(Ray(Mem->MyPos(),Mem->MyBodyAng()),&my_body_int)&&
     l.InBetween(my_body_int,target,opp_pos)){
    Mem->LogAction2(10,"BlockMovingOpponent:try to go without turn");
    res=my_body_int;
  }else{
    res=BlockAddVector(opp_pos,target,Mem->MyPos(),opp_pos);
  }
  
  float min_x=-48.0f;
  if(Mem->OurGoalieNum!=Unum_Unknown&&Mem->TeammatePositionValid(Mem->OurGoalieNum)>0.95f)
    min_x=Mem->TeammateX(Mem->OurGoalieNum)+(Mem->BallX()<=Mem->TeammateX(Mem->OurGoalieNum)?4.0f:1.0f);
  res.x=max(res.x,min_x);
  
  MoveToPos(res,30.0,5.0);
  eye.AddOpponent(opp,5,RT_WithTurn);
  eye.AddBall(1);
  return true;

}
////////////////////////////////////////////////////////////
Vector Defense::PredictOpponentPositionAtOurPenaltyArea(Unum opp,int numCyc,Vector )
{
  Mem->LogAction4(10,"PredictOpponentPositionAtOurPenaltyArea: can not see opp %.0f with %.0f cycles",
		  float(opp),float(numCyc));
  if(numCyc<1)
    return Mem->OpponentAbsolutePosition(opp);
  float offside=Mem->their_offside_line;
  Vector pred=Mem->OpponentPredictedPosition(opp,numCyc,Polar2Vector(Mem->SP_max_power*Mem->SP_dash_power_rate,180));
  pred.x=max(pred.x,offside);
  Mem->LogAction4(10,"PredictOpponentPositionAtOurPenaltyArea: think, that he go to pos (%.2f;%.2f)",
		  pred.x,pred.y);
  return pred;
}
//////////////////////////////////////////////////////////////////////
bool Defense::Mark(){
  Unum mark;
  if((mark=check_for_free_opponents_in_own_penalty_area())!=Unum_Unknown){
    float his_pos_val=Mem->OpponentVelocityValid(mark);
    Mem->LogAction6(10, "Mark: mark free opponent %.0f in own penalty area(pos val: %.2f; speed: %.2f (speed val: %.2f))",
		    float(mark),Mem->OpponentPositionValid(mark),Mem->OpponentSpeed(mark),his_pos_val);
    float dist_to_him=Mem->DistanceTo(Mem->OpponentAbsolutePosition(mark));
    Mem->LogAction5(20,"His position = (%.2f,%.2f); dist_to_him=%.2f",
		    Mem->OpponentX(mark),Mem->OpponentY(mark),Mem->DistanceTo(Mem->OpponentAbsolutePosition(mark)));
    last_marking_time=Mem->CurrentTime;
    Vwidth vw=((dist_to_him<Mem->SP_feel_distance&&his_pos_val==1.0f)||(dist_to_him<Mem->SP_feel_distance/2.0f&&his_pos_val>0.94f))?VW_Narrow:VW_Normal;
    Vector fast_pos=GetFastDefensePos(Mem->BallAbsolutePosition());
    if(Mem->BallX()<Mem->MyX()&&Mem->OpponentDistanceTo(mark,fast_pos)<Mem->DistanceTo(fast_pos)&&Mem->DistanceTo(fast_pos)>2.0f){//hack
      Mem->LogAction4(10,"Mark: this opp is closer to our fast pos then me (%.2f<%.2f)",
		      Mem->OpponentDistanceTo(mark,fast_pos),Mem->DistanceTo(fast_pos));
      MoveToPos(fast_pos,30.0f,3.0f,true);
      eye.AddPosition(Mem->OpponentAbsolutePosition(mark),1,9,RT_WithoutTurn,vw);
      return true;
    }
    
    
    MarkType mt=mark_bisector;
    if(Mem->BallX()<Mem->OpponentX(mark)&&Mem->OpponentX(mark)>-36.0f){
      Mem->LogAction2(10,"Mark: select mark goal");
      mt=mark_goal;
    }
    if(Mem->OpponentVelocityValid(mark)>0.95f&&Mem->OpponentSpeed(mark)>0.2){
      Mem->LogAction2(10,"Mark: opponent is moving, so block opponent on speed");
      BlockMovingOpponent(mark);
      eye.AddOpponent(mark,1);
    }else{
      Vector pos=PredictOpponentPositionAtOurPenaltyArea(mark,
							 NumOfCyclesThenILastSeePlayer(-mark),
							 Mem->BallAbsolutePosition());
      eye.AddPosition(Mem->OpponentAbsolutePosition(mark),1,9,RT_WithoutTurn,vw);
      MarkType m_type=mark_bisector;
      if(pos.x<=-45.0f)
	m_type=mark_ball;
      MarkOpponent(mark,m_type,1.0,pos);
    }
    eye.AddBall(1,RT_WithoutTurn,vw);
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////
bool Defense::ICanNotMark(){
  //  Unum tm1,tm2;
  //Get2Defenders(GetFastDefensePos(Mem->BallAbsolutePosition()),tm1,tm2);
  if(fabs(Mem->BallY())>Mem->SP_goal_width/2-2.0f&&/*tm1!=Mem->MyNumber&&tm2!=Mem->MyNumber&&*/
     check_for_free_opponents_in_own_penalty_area()!=Unum_Unknown){
    Mem->LogAction2(50,"I must mark, so can not go to ball");
    return false;
  }
  return true;
}
////////////////////////////////////////////////////////////
bool Defense::BlockCrossAtOurPenaltyArea(Vector ball){
  if(GetMyType()>=PT_Midfielder)
    return false;
  Vector pos=Mem->MyPos();
  float ballSign=signf(ball.y-pos.y);
  float max_ocenka=0.0f,ocenka;
  Unum max_opp=Unum_Unknown;
  for(int i=1;i<=Mem->SP_team_size;i++){
    if(!Mem->OpponentPositionValid(i)||i==Mem->TheirGoalieNum||
       Mem->OpponentX(i)>-33.0f||ballSign==signf(Mem->OpponentY(i)-pos.y)||fabs(ball.y-Mem->OpponentY(i))>25.0f)
      continue;
    ocenka=1/fabs(Mem->OpponentY(i)-pos.y);//may be correct this ocenka :)
    if(ocenka>max_ocenka){
      max_ocenka=ocenka;
      max_opp=i;
    }
  }
  if(max_opp==Unum_Unknown){
    Mem->LogAction2(10,"BlockCrossAtOurPenaltyArea: can not find opponent to block");
    return false;
  }
  Mem->LogAction4(10,"BlockCrossAtOurPenaltyArea: block opp %.0f (conf %.2f)",
		  float(max_opp),Mem->OpponentPositionValid(max_opp));
  Vector opp_pos=PredictOpponentPositionAtOurPenaltyArea(max_opp,
							 NumOfCyclesThenILastSeePlayer(-max_opp),
							 ball);
  Vector target=Mem->BallAbsolutePosition();
  if(Mem->OpponentVelocityValid(max_opp)>0.95f&&Mem->OpponentSpeed(max_opp)>0.2f){
    Vector opp_vel=Mem->OpponentAbsoluteVelocity(max_opp);
    Mem->LogAction5(10,"BlockCrossAtOurPenaltyArea:Opponent has speed (%.2f,%.2f) with conf %.2f",
		    opp_vel.x,opp_vel.y,Mem->OpponentVelocityValid(max_opp));
    target=Mem->FieldRectangle.RayIntersection(Ray(opp_pos,opp_vel));
  }
  Line l;l.LineFromTwoPoints(target,opp_pos);
  Vector res;
  Vector my_body_int;
  if(l.RayIntersection(Ray(Mem->MyPos(),Mem->MyBodyAng()),&my_body_int)&&
     l.InBetween(my_body_int,target,opp_pos)&&l.dist(Mem->MyPos())>2.0f&&
     opp_pos.dist(my_body_int)>2.0f){
    Mem->LogAction2(10,"BlockCrossAtOurPenaltyArea:try to go without turn");
    res=my_body_int;
  }else{
    res=BlockAddVector(opp_pos,target,Mem->MyPos(),opp_pos);
  }

  float min_x=-48.0f;
  if(Mem->OurGoalieNum!=Unum_Unknown&&Mem->TeammatePositionValid(Mem->OurGoalieNum)>0.95f)
    min_x=Mem->TeammateX(Mem->OurGoalieNum)+(Mem->BallX()<=Mem->TeammateX(Mem->OurGoalieNum)?4.0f:1.0f);
  res.x=max(res.x,min_x);
  
  MoveToPos(res,20.0,0.0,true);
  eye.AddPosition(opp_pos,1,9);
  eye.AddBall(1,RT_WithTurn);
  return true;
}
////////////////////////////////////////////////////////////
void Defense::AtOurPenaltyArea(){
  Vector ball=Mem->BallAbsolutePosition();
  if(Mem->MyInterceptionAble()&&
     ((Mem->ClosestTeammateToBall()==Mem->MyNumber&&Mem->BallKickableForOpponent(FastestOpp(),2.0f))||
      (FastestTm()==Mem->MyNumber&&TmPoint().dist(Mem->MyPos())<=7.0f))/*&&ICanNotMark()*/){
    Mem->LogAction2(10,"AtOurPenaltyArea:i am closest(or fastest) and i go to ball");
    DefenseGetBall();
    return;  	
  }
  int num_cyc=FastestTm()!=Unum_Unknown&&Mem->MyX()<Mem->TeammateX(FastestTm())?5:2;
  if (IsDefense()&&
      ((Mem-> MyInterceptionAble()&&(Mem->MyInterceptionNumberCycles()-TmCycles())<=num_cyc&&TmCycles()>2&&ICanNotMark())||
       Mem->ClosestTeammateTo(Mem->BallEndPosition())==Mem->MyNumber)){
    Mem->LogAction2(10, "Mode: I`m second to ball so go to take ball");
    DefenseGetBall();
    return;
  }

  if(Mark())
    return;

//   Unum tm1,tm2;
//   Get2Defenders(GetFastDefensePos(ball),tm1,tm2);
//   Mem->LogAction4(10,"AtOurPenaltyArea: ball is between %.0f and %.0f teammates",float(tm1),float(tm2));
//   if(IsDefense()&&(tm1==Mem->MyNumber||tm2==Mem->MyNumber)&&GetMySide()==PS_Center&&
//      GetMyType()<=PT_Defender&&fabs(ball.y)<Mem->SP_penalty_area_width/2){
//     DefenseGetBall();
//     return;
//   }

  if(!TmInter()&&Mem->BallDistance()<=6.0f){
    Mem->LogAction2(10,"AtOurPenaltyArea: i don`t know who is fastest from our tm, so go to ball");
    DefenseGetBall();
    return;
  }
//   if(IsDefense()&&GetMySide()!=PS_Center&&GetMyType()==PT_Defender){
//     if((GetMySide()==PS_Left&&ball.y<-Mem->SP_goal_area_width/2)||(GetMySide()==PS_Right&&ball.y>Mem->SP_goal_area_width/2)){
//       Mem->LogAction2(10,"AtOurPenaltyArea: ball in my zone, so go to him");
//       DefenseGetBall();
//       return;
//     }
//   }
  if(BlockCrossAtOurPenaltyArea(ball)){
    return;
  }
  Mem->LogAction2(10,"AtOurPenaltyArea: go to fast home position");
  RequestType rt=RT_WithoutTurn;
  Vector target=GetFastDefensePos(ball);
  if(fabs(ball.y)<12.0f&&ball.x>-40.0f)
    target.x=ball.x-4.0f;
  
  if(GetMyType()==PT_Forward)
    target=Vector(-1.0f,GetHomePosition(Vector(50.0,0.0)).y);
  if(!MoveToPos(target,10.0,0.0,true)){
    face_only_body_to_ball();
  }
  if(Mem->DistanceTo(target)<2.0f)
    rt=RT_WithTurn;
  //eye.AddPosition(Vector(-Mem->SP_pitch_length/2,-signf(ball.y)*Mem->SP_goal_width/2),5,1,rt,VW_Normal);
  eye.AddPosition(Vector(-Mem->SP_pitch_length/2+Mem->SP_penalty_area_length,-signf(ball.y)*Mem->SP_penalty_area_width/2),
		  5,2,rt,VW_Normal);
  eye.AddPosition(Vector(Mem->MyX()+10.0f,Mem->MyY()),5,1,rt,VW_Normal);
  eye.AddBall(2,rt);
}
////////////////////////////////////////////////////////////
void Defense::DefenderPlay(){
  Vector ball=Mem->BallAbsolutePosition();

  CheckForStartRest();

  if(ball.x>25.0f){
    Mem->LogAction2(10,"Defense: type - begin_def");
    BeginDef();
    return;
  }
  if(IsPlayInOurPenaltyArea()){
    Mem->LogAction2(10,"Defense: type - at_our_penalty_area.");
    AtOurPenaltyArea();
    return;
  }

  Unum tm1,tm2;
  Get2Defenders(Mem->MyPos(),tm1,tm2);
  if(Mem->MyX()>ball.x&&Mem->BallPositionValid()>0.6f&&TmCycles()>0){
    Mem->LogAction2(10,"Defense: type - go_to_our_penalty_area");
    Mem->LogAction4(10,"go_to_our_penalty_area: ball is between %.0f and %.0f teammates",float(tm1),float(tm2));
    if((tm1==Mem->MyNumber||tm2==Mem->MyNumber)&&(Mem->ClosestTeammateToBall()==Mem->MyNumber/*||FastestTm()==Unum_Unknown*/)){
      DefenseGetBall();
      return;
    }
    if(GoToActiveDefense()){
      return;
    }
    RequestType rt=RT_WithoutTurn;
    if(!MoveToPos(GetFastDefensePos(ball),10.0,0.0,true)){
      rt=RT_WithTurn;
      face_only_body_to_ball();
    }
    eye.AddPosition(Vector(-Mem->SP_pitch_length/2,-signf(ball.y)*Mem->SP_goal_width/2),5,1,rt,VW_Normal);
    eye.AddPosition(Vector(-Mem->SP_pitch_length/2+Mem->SP_penalty_area_length,-signf(ball.y)*Mem->SP_penalty_area_width/2),
		    5,2,rt,VW_Normal);
    eye.AddBall(2);
    return;
  }//end of go to our penalty area

  Unum danger_opp=Unum_Unknown;
  if(CanGoToActiveDefense(&danger_opp)){
    DefenseGetBall();
    return;
  }
  Iterator iter=begin();
  Unum tm=GetPlayerNumber(iter,PT_Sweeper);
  float x=GetHomePosition().x;
  if(tm!=Unum_Unknown)
    x=GetHomePosition(tm).x;
  if(IsRest()){
    Mem->LogAction3(10,"Defense: we have rest, so x=%.2f",Min(x,x_to_rest));
    x=Min(x,x_to_rest);
  }else{
    Mem->LogAction3(10,"Defense: our line is=%.2f",x);
  }
  if(Mem->MyX()<penalty_area_treshold&&tm1!=Mem->MyNumber&&tm2!=Mem->MyNumber&&IsOffense()){
    float dash=100.0f;
    if(GetMyType()==PT_Sweeper)
      dash=45.0f;
    else if(!Mem->TeammatePositionValid(tm)||Mem->TeammateX(tm)-Mem->MyX()<=3.0f)
      dash=45.0f;
    Mem->LogAction2(10,"DefenderPlay: softly move from offside area");
    MoveToPos(Vector(x,Mem->MyY()),10.0f,0.0,dash);
    return;
  }

  if(WingDefenderMark(Vector(x,GetHomePosition().y),false))
    return;

  if(PassiveCentralDefense(Vector(x,GetHomePosition().y)))
    return;
  
  //ok:идем в домашнюю позицию с различной скоростью
  
  eye.AddPosition(Vector(Mem->MyX(),-signf(Mem->MyY())*35.0f),5,7,RT_WithoutTurn,VW_Normal);
  if(danger_opp!=Unum_Unknown){
    eye.AddOpponent(danger_opp,5,RT_WithTurn,VW_Normal);
  }else  
    if(fabs(Mem->MyY())<25.0f)
      eye.AddPosition(Vector(Mem->MyX(),signf(Mem->MyY())*35.0f),5,8,RT_WithoutTurn,VW_Normal);
    else
      eye.AddPosition(Vector(Mem->MyX()-5.0f,Mem->MyY()),5,8,RT_WithoutTurn,VW_Normal);
  
  float dash=GetFormationDash(Mem->MyNumber);
  if(Mem->their_offside_line-Mem->MyX()<-5.0f/*Mem->TeammatePositionValid(tm)&&(Mem->TeammateX(tm)-Mem->MyX())>5.0f*/)
    dash=100.0f;
  else if(IsRest()){
    float dash=Mem->GetMyStaminaIncMax()*Mem->MyStamina()/Mem->SP_stamina_max;
    MoveToPos(Vector(x,GetHomePosition().y),5.0f,0.0f,dash);
    return;
  }
  Mem->LogAction2(10,"DefenderPlay: Go to home position");
  if(!MoveToPos(Vector(x,GetHomePosition().y),7.0,0.0,dash))
    face_only_body_to_ball();
  return;

}
////////////////////////////////////////////////////////////
bool Defense::MidfielderMark(Vector homePos){
  const float MAX_X_DIST=10.0f;
  const float CLOSE_MARK_DIST=7.0f;
  const float Y_BUF=7.0f;
  const float MAX_PROJECT_DIST=7.0f;
  const float DIST_FROM_OPPONENT_THEN_MARK=2.0f;
  
  Unum closestOpp[11];
  if(Mem->MyStamina()<Mem->SP_stamina_max*0.75){
    Mem->LogAction2(10,"MidfielderMark: i`m quite tired so do not go to mark");
    return false;
  }

  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('t' ,Mem->MyPos(),closestOpp);
  float y=fabs(Mem->MyY()-homePos.y)<Y_BUF?Mem->MyY():homePos.y;
  Line l;
  Vector my_pos(Mem->MyX(),y);
  Unum opt_opp=Unum_Unknown;
  float min_x=100.0f;
  for(int i=0;i<numClosestOpp;i++){
    if(!Mem->OpponentPositionValid(closestOpp[i])||FastestOpp()==closestOpp[i]||
       Mem->ClosestTeammateTo(Mem->OpponentAbsolutePosition(closestOpp[i]))!=Mem->MyNumber)
      continue;
    if(Mem->OpponentX(closestOpp[i])-7.0f<Mem->their_offside_line||(Mem->OpponentX(closestOpp[i])-Mem->BallX())>MAX_X_DIST)
      continue;
    if(Mem->DistanceTo(Mem->OpponentAbsolutePosition(closestOpp[i]))<CLOSE_MARK_DIST){
      Mem->LogAction5(10,"MidfielderMark: opp %.0f (conf %.2f) has dist %.2f, so mark him",float(closestOpp[i]),
		      Mem->OpponentPositionValid(closestOpp[i]),Mem->DistanceTo(Mem->OpponentAbsolutePosition(closestOpp[i])));
      MarkOpponent(closestOpp[i],mark_ball,DIST_FROM_OPPONENT_THEN_MARK,
		   Mem->OpponentAbsolutePosition(closestOpp[i])+Polar2Vector(1.5f,180.0f));
      eye.AddOpponent(closestOpp[i],5,RT_WithTurn,VW_Narrow);
      eye.AddBall(1,RT_WithTurn);
      return true;
    }
    if(signf(Mem->BallY()-y)==signf(Mem->OpponentY(closestOpp[i])-y))
      continue;
    l.LineFromTwoPoints(Mem->BallAbsolutePosition(),Mem->OpponentAbsolutePosition(closestOpp[i]));
    Vector project=l.ProjectPoint(my_pos);
    if(project.dist(my_pos)>MAX_PROJECT_DIST)
      continue;
    if(Mem->OpponentX(closestOpp[i])<min_x){
      min_x=Mem->OpponentX(closestOpp[i]);
      opt_opp=closestOpp[i];
    }
  }
  if(opt_opp==Unum_Unknown)
    return false;
  Mem->LogAction5(10,"MidfielderMark: opp %.0f (conf %.2f) has project dist %.2f, so block cross",
		  float(opt_opp),Mem->OpponentPositionValid(opt_opp),min_x);
  MoveToPos(Mem->OpponentAbsolutePosition(opt_opp)+Polar2Vector(1.5f,180.0f),10.0f,0.0f);
  eye.AddOpponent(opt_opp,5,RT_WithTurn,VW_Narrow);
  eye.AddBall(1,RT_WithTurn);
  return true;
}
////////////////////////////////////////////////////////////
bool Defense::FlashPressing(){
  Unum closestTm[11];
  int numClosestOpp=Mem->SortPlayersByDistanceToPoint('m' ,Mem->BallAbsolutePosition(),closestTm);
  bool ok=false;
  for(int i=0;i<(numClosestOpp>3?3:numClosestOpp);i++)
    if(closestTm[i]==Mem->MyNumber)
      ok=true;
  if(!ok)
    return true;
  if(FastestOpp()!=pressing_opponent||
     (Mem->BallX()>-30.0&&GetMyType()<=PT_Defender)||
     !Mem->OpponentPositionValid(FastestOpp())||
     (Mem->OpponentAbsolutePosition(FastestOpp())-Mem->BallAbsolutePosition()).mod()>4.0)
    return true;
  return false;
}
////////////////////////////////////////////////////////////
bool Defense::IsPlayInOurPenaltyArea(){
  if(BreakawayOnWings()||BreakawayOnCenter())
    return true;
  float dist=Mem->ClosestOpponentToBallDistance();
  bool our_pen_free=true;
  float treshold=fabs(Mem->BallY()-Mem->MyY())>15.0f?penalty_area_treshold-1.5f:penalty_area_treshold;
  for(int i=1;i<=11;i++){
    if(!Mem->OpponentPositionValid(i)||Mem->OpponentX(i)>min(Mem->BallX()-2.0f,-35.0f))
      continue;
    if(Mem->their_offside_line<Mem->OpponentX(i)){
      Mem->LogAction4(10,"IsPlayInOurPenaltyArea:Opponent %.0f with conf %.2f is try block us in our penalty area",
		      float(i),Mem->OpponentPositionValid(i));
      our_pen_free=false;
      break;
    }
  }
  
  if(!our_pen_free)
    treshold=-25.0f;
  return (IsDefense()||dist<3.0f)&&Mem->BallX()<treshold;
}
//////////////////////////////////////////////////////////////////////
bool Defense::BreakawayOnWings(){
  Vector ball=Mem->BallAbsolutePosition();
  if(ball.x>-25.0||fabs(ball.y)<Mem->SP_penalty_area_width/2||IsOffense())
    return false;
  for(int i=1;i<=11;i++){
    if(Mem->TeammatePositionValid(i)&&Mem->TeammateX(i)<ball.x&&fabs(Mem->TeammateY(i)-ball.y)<8.0)
      return false;
  }
  Mem->LogAction2(10,"Think, that we have breakaway on wing");
  return true;
}
//////////////////////////////////////////////////////////////////////
bool Defense::BreakawayOnCenter()
{
  Vector ball=Mem->BallAbsolutePosition();
  if(ball.x>-25.0||fabs(ball.y)>=Mem->SP_penalty_area_width/2||IsOffense()||
     (Mem->MyX()-Mem->BallX()<5.0f&&Mem->MyX()<-35.0f))
    return false;
  for(int i=1;i<=11;i++){
    if(Mem->TeammatePositionValid(i)&&i!=Mem->TheirGoalieNum&&
       (IsPointInCone(Mem->TeammateAbsolutePosition(i),0.5f,Vector(-52.5,Mem->BallY()),Mem->BallAbsolutePosition())||
	Mem->TeammateDistanceToBall(i)<=2.5f))
      return false;
  }
  Mem->LogAction2(10,"Think, that we have breakaway in center");
  return true;  
}
////////////////////////////////////////////////////////////
bool Defense::CloseTackleBehavior()
{
  if(Mem->BallDistance()>4.0f||!IsDefense()||Mem->BallPositionValid()!=1.0f)
    return false;
//   if(GetMyType()<=PT_Defender&&Mem->MyInterceptionAble()&&Mem->MyInterceptionNumberCycles()<=1&&
//      Mem->BallVelocityValid()==1.0f&&&&
//      (FastestOpp()==Unum_Unknown||Mem->MyX()>Mem->OpponentX(FastestOpp())))//HACK
//     return false;
  float tackle_probability=Mem->GetTackleProb(Mem->BallAbsolutePosition(),Mem->MyPos(),Mem->MyBodyAng());
  if(GetMyType()<=PT_Defender&&Mem->OpponentWithBall()!=Unum_Unknown){
    if((Mem->OwnPenaltyArea.IsWithin(Mem->MyPos())&&tackle_probability<0.9f)||Mem->MyX()<Mem->OpponentX(FastestOpp()))
       return false;
  }
  if(tackle_probability>0.75f){
    Mem->LogAction3(10,"CloseTackleBehavior:make tackle with probability %.2f",tackle_probability);
    tackle(fabs(Mem->MyBodyAng())>90.0f?-100.0f:100.0f);
    return true;
  }
  Unum opp=Mem->ClosestOpponentToBall();
  if(Mem->BallVelocityValid()<1.0f||(opp!=Unum_Unknown&&Mem->BallKickableForOpponent(opp)))
    return false;
  Vector ball=Mem->BallPredictedPosition();
  int cyc=Mem->NumCyclesToTurn((ball-Mem->MyPos()).dir());
  Mem->LogAction3(10,"CloseTackleBehavior: num cycles to turn to ball =%.0f",float(cyc));
  if(cyc==1&&(tackle_probability=Mem->GetTackleProb(ball,Mem->MyPredictedPosition(),(ball-Mem->MyPos()).dir()))>0.8f){
    Mem->LogAction3(10,"CloseTackleBehavior: want to make tackle in next cycle with prob %.2f",tackle_probability);
    face_only_body_to_point(ball);
    return true;
  }
  if(cyc==1&&(tackle_probability=Mem->GetTackleProb(ball,Mem->MyPredictedPosition(1,Mem->SP_max_power),
						    Mem->MyBodyAng()))>0.8f){
    Mem->LogAction3(10,"CloseTackleBehavior: want to make tackle in next cycle with prob %.2f",tackle_probability);
    dash(Mem->SP_max_power);
    return true;
  }
  return false;
}
////////////////////////////////////////////////////////////
void Defense::defense(){  
  if(FastestTm()==Mem->MyNumber&&TmCycles()<=OppCycles()){
    Mem->LogAction2(10,"Defense: i`m fastest and go to ball");
    get_ball();
    return;
  }

  //проверяем на возможность подката в ближайшие два цикла
  if(CloseTackleBehavior())
    return;
	
  if(pressing_time==Mem->CurrentTime-1){
    if(FlashPressing()){
      Mem->LogAction2(10,"Defense: flash pressing");
      pressing_time=-1;
    }else{
      Mem->LogAction3(10,"Defense: pressing opponent %d",int(pressing_opponent));
      pressing_time=Mem->CurrentTime;
      DefenseGetBall();
      return;
    }
  }
  if(GetMyType()<=PT_Defender){
    DefenderPlay();
    return;
  }
  //for other players
  Vector ball=Mem->BallAbsolutePosition();
  if(ball.x>20.0f){
    Mem->LogAction2(10,"Defense: type - begin_def");
    BeginDef();
    return;
  }
  if(IsPlayInOurPenaltyArea()/*ball.x<-32.0&&GetMyType()!=PT_Forward*/){
    Mem->LogAction2(10,"Defense: type - at_our_penalty_area.");
    if(GetMyType()==PT_Midfielder&&Mem->GetBallSide()==GetMySide()&&Mem->BallVelocityValid()>0.8f){
      Mem->LogAction2(10,"Then play at our penalty arae say about ball");
      char msg[10]={0,0,0,0,0,0,0,0,0,0};
      Msg m(msg);
      m<<ST_ball_and_conf_ballvel_and_conf;
      Mem->AddBallPos(m);
      Mem->AddBallVel(m);
      Mem->SayNow(msg);
    }
    
    AtOurPenaltyArea();
    return;
  }
  Mem->LogAction2(10,"Defense: play in central zone");

  if(PressingWithoutDefenders())
    return;
  
  if(GetMyType()==PT_Forward&&Mem->MyStamina()<Mem->SP_stamina_max*0.8f&&Mem->MyX()<Mem->my_offside_line-2.0f&&Mem->BallX()+7.0f<Mem->MyX()){
    Mem->LogAction2(10,"Forward tired and can rest");
    return;
  }
  if(GetMyType()==PT_Forward){
    float y=0.0f;
    if(Mem->BallY()>10.0f)
      y=30.0f;
    else if(Mem->BallY()<-10.0f)
      y=-30.0f;
    GoToHomePosition(Vector((Mem->BallX()>-1.0f?GetHomePosition().x:-1.0f),GetHomePosition(Vector(0.0f,y)).y));
    return;
  }
  Iterator iter=begin();
  Unum tm=GetPlayerNumber(iter,PT_Sweeper);
  float x=GetHomePosition().x;
  if(Mem->CP_midfielders_close_to_defense&&tm!=Unum_Unknown)
    x=(GetHomePosition(tm).x+x)/2;

  if(MidfielderMark(Vector(x,GetHomePosition().y)))
    return;

  GoToHomePosition(Vector(x,GetHomePosition().y));

}
//////////////////////////////////////////////////////////////////////
Vector Defense::GetOpponentTarget()
{
  Vector target=Vector(-52.5,OppPoint().y);
  if(Mem->BallX()<-30.0f||Mem->TheirBreakaway())
    target=Vector(-52.5,0.0);
  Unum opp=FastestOpp();
  if(Mem->BallX()>-20.0f||fabs(Mem->BallY())<15.0f){ 
    float bodyConf=Mem->OpponentBodyAngleValid(opp);
    AngleDeg bodyAng=bodyConf?Mem->OpponentAbsoluteBodyAngle(opp):777.0f;
    Mem->LogAction5(20,"GetOpponentTarget: opponent %.0f has body ang=%.2f (conf %.2f)",
		    float(opp),bodyAng,Mem->OpponentBodyAngleValid(opp));
    if(bodyConf>0.9f&&(bodyAng<=-145||bodyAng>=145)){
      float y=Mem->FieldRectangle.RayIntersection(Ray(Mem->OpponentAbsolutePosition(opp),bodyAng)).y;
      target=Vector(-Mem->SP_pitch_length/2.0f,y);
      Mem->LogAction3(20,"GetOpponentTarget: select y=%.2f",y);
    }
  }
  return target;
}
////////////////////////////////////////////////////////////////////////////
bool Defense::IsILastChance()
{
  Iterator iter=begin();
  Unum goalie=GetPlayerNumber(iter,PT_Goaltender);
  for(int i=1;i<=11;i++){
    if(!Mem->TeammatePositionValid(i)||i==goalie||i==Mem->MyNumber)
      continue;
    if(IsPointInCone(Mem->TeammateAbsolutePosition(i),0.3f,Vector(-Mem->SP_pitch_length/2.0f,Mem->BallY()),Mem->BallAbsolutePosition())){
      Mem->LogAction3(10,"Teammate %.0f help me, so i`m not alone",float(i));
      return false;
    }
  }
  Mem->LogAction2(10,"IsILastChance: seems that i alone");
  return true;
}
//////////////////////////////////////////////////////////////////////
void Defense::DefenseGetBall(float max_dash){
  //  const int MAX_CYC_DIFF_THEN_GET_BALL=10;
  if(FastestOpp()==Unum_Unknown||!Mem->OpponentPositionValid(FastestOpp())||
     !OppInter()||(Mem->MyInterceptionAble()&&(OppCycles()>=Mem->MyInterceptionNumberCycles()||(Mem->BallX()>-30.0f&&!IsILastChance()&&
     OppCycles()+2>=Mem->MyInterceptionNumberCycles()&&Mem->DistanceTo(Mem->OpponentAbsolutePosition(FastestOpp()))<5.0f)))){
    if(Mem->MyInterceptionAble()){
      Mem->LogAction2(10,"DefenseGetBall:i`m fastest, so go to ball");
      get_ball();
      return;
    }
    my_error("I can not intercept ball in DefenseGetBall!!!");
    MoveToPos(Mem->BallAbsolutePosition(),15.0,0.0,max_dash);
    return;
  }

  Vector pos=Mem->OpponentAbsolutePosition(FastestOpp());
  Vector target=GetOpponentTarget();
  Mem->LogAction5(10,"DefenseGetBall: i must block opp %d with ball; target=(%.2f;%.2f)",FastestOpp(),target.x,target.y);
  Line l;
  l.LineFromTwoPoints(OppPoint(),target);
  Vector pr_dot=l.ProjectPoint(Mem->MyPos());

  Iterator iter=begin();
  Unum tm=GetPlayerNumber(iter,PT_Sweeper);
  float x=Mem->their_offside_line;
  if(tm!=Unum_Unknown)
    x=GetHomePosition(tm).x;

  if(Mem->TheirPenaltyArea.IsWithin(Mem->MyPos())){
    Mem->LogAction2(40,"opp at their p_area so get ball");
    if(Mem->MyInterceptionAble())
      get_ball();
    else
      MoveToPos(Mem->BallEndPosition(),15.0,3.0,false,max_dash);
    eye.AddOpponent(FastestOpp(),1,RT_WithoutTurn,VW_Narrow);
    eye.AddBall(1,RT_WithoutTurn);
    return;
  }
//   Vector ball=Mem->BallAbsolutePosition();
//   if(OppCycles()<=1){
//     Mem->LogAction2(40,"DefenseGetBall:opp controlling ball");
//     int cyc=Mem->PredictedCyclesToPoint(ball);
//     if(cyc<=3){
//       Mem->LogAction3(40,"DefenseGetBall:my cyc is %.0f, so go to ball",float(cyc));
//       MoveToPos(ball,20.0f,0.0f);
//       eye.AddBall(1);
//       return;
//     }
//   }

//   if(CloseBallTake())
//     return;
  if(fabs(OppPoint().x-x)<10.0f&&GetMyType()>=PT_Midfielder){
    Mem->LogAction2(40,"opp too close to our defense line");
    MoveToPos(OppPoint(),15.0,3.0,false,max_dash);
    eye.AddOpponent(FastestOpp(),1,RT_WithoutTurn,VW_Narrow);
    eye.AddBall(1,RT_WithoutTurn);
    return;
  }
  AngleDeg ang=GetNormalizeAngleDeg((pr_dot-OppPoint()).dir()-(Mem->MyPos()-OppPoint()).dir());
  if(fabs(ang)<15.0f||((pos.x+2.0f)<OppPoint().x&&(OppCycles()>3||FastestOpp()==Mem->TheirGoalieNum))){
    Vector add=(target-OppPoint())*0.3f/(target-OppPoint()).mod();
    Mem->LogAction5(10,"DefenseGetBall: i`m on line (ang is %.2f) or ball is back of opponent, so go to ball. Add (%.2f,%.2f)",
		    (float)fabs(ang),add.x,add.y);
    if(!MoveToPos(OppPoint()+add,15.0,0.0,max_dash)){
      AngleDeg turn_ang=GetNormalizeAngleDeg((target-OppPoint()).dir()+180-Mem->MyBodyAng());
      if(fabs(turn_ang)>5){
	Mem->LogAction3(30,"DefenseGetBall:i`m at point and must turn to tackle (ang %.2f)",turn_ang);
	turn(turn_ang);
      }
    }
    eye.AddOpponent(FastestOpp(),1,RT_WithoutTurn,VW_Narrow);
    eye.AddBall(1,RT_WithoutTurn);
    return;
  }

  bool need_turn=Mem->OpponentBodyAngleValid(FastestOpp())>0.9f&&
    fabs(GetNormalizeAngleDeg(Mem->OpponentAbsoluteBodyAngle(FastestOpp())-target.dir()))>40.0f;
  if(Mem->OpponentWithBall()!=Unum_Unknown&&need_turn&&OppCycles()+3>=Mem->MyInterceptionNumberCycles()
     &&Mem->DistanceTo(Mem->OpponentAbsolutePosition(FastestOpp()))<6.0f){
    Mem->LogAction3(10,"DefenseGetBall: opp %.0f need turn so move just to it",float(FastestOpp()));
    MoveToPos(Mem->OpponentAbsolutePosition(FastestOpp())+Polar2Vector(0.1f,(target-Mem->OpponentAbsolutePosition(FastestOpp())).dir()),30.0f,3.0f,true);
    return;
  }
  
  bool ready;
  if(Mem->MyX()<-20.0f||IsOpponentActiveUseDribble()){
    ready=l.dist(Mem->MyPos())>0.8f;
  }else{
    ready=l.dist(Mem->MyPos())+1.0f>pr_dot.dist(pos);
  }
    
  if(ready){
    Vector res=BlockAddVector(OppPoint(),target,Mem->MyPos(),pos);
    Mem->LogAction2(10,"DefenseGetBall: i`m not ready, so go to block their breakaway");
    MoveToPos(res,30.0,0.0,false,max_dash);
  }else{
    Mem->LogAction2(10,"DefenseGetBall:i`m ready, so go fast to ball");
    Vector addition=(target-OppPoint())*1.0f/(target-OppPoint()).mod();
    MoveToPos(OppPoint()+addition,30.0,3.0,false,max_dash);
  }
  eye.AddOpponent(FastestOpp(),1,RT_WithoutTurn,VW_Narrow);
  eye.AddBall(1,RT_WithoutTurn);

}
//////////////////////////////////////////////////////////////////////
bool Defense::CloseBallTake()
{
  Vector ball=Mem->BallAbsolutePosition();
  Vector ballPred=Mem->BallPredictedPosition();
  Vector opp=Mem->OpponentAbsolutePosition(FastestOpp());
  if(OppCycles()>0||Mem->DistanceTo(ball)>4.0f||ball.x<opp.x)
    return false;
  int add_cyc=0;
  
  Vector target=Vector(opp.x-5.0f,opp.y);
  if(Mem->OpponentBodyAngleValid(FastestOpp())>0.95f&&fabs(GetNormalizeAngleDeg(target.dir()-Mem->OpponentAbsoluteBodyAngle(FastestOpp())))>20.0f)
    add_cyc++;
  float max_acc=Mem->GetOpponentDashPowerRate(FastestOpp())*Mem->GetOpponentEffortMax(FastestOpp())*Mem->SP_max_power;
  float opp_kickable_area=Mem->GetOpponentKickableArea(FastestOpp());
  if((Mem->OpponentPredictedPosition(FastestOpp())-ballPred).mod()>opp_kickable_area||
     (add_cyc==0&&(Mem->OpponentPredictedPosition(FastestOpp(),1,Polar2Vector(max_acc,target.dir()))-ballPred).mod()>opp_kickable_area))
    add_cyc++;
  Vector stop=Vector(opp.x-2.0*Mem->GetMyPlayerSize(),opp.y);
  int my_cyc;
  if((my_cyc=Mem->PredictedCyclesToPoint(stop))<=add_cyc+1){
    if(go_to_point( stop, 0.4f, Mem->SP_max_power,DT_none,false )==AQ_ActionQueued){
      Mem->LogAction4(10,"CloseBallTake: opp cyc=%.0f; my cyc=%.0f; go to stop pos",float(add_cyc+1),float(my_cyc));
    }else{
      Mem->LogAction2(10,"Already at target position, so face to ball");
      face_only_body_to_point(ball);
    }
    return true;
  }
  return false;
}
///////////////////////////////////////////////////////////////////////////////
Vector Defense::BlockAddVector(Vector start_pos,Vector target,Vector my_pos,Vector opp_pos)
{
  float add=0.0f;
  Line l;
  l.LineFromTwoPoints(start_pos,target);
  Vector pr_dot=l.ProjectPoint(my_pos);
  Mem->LogAction6(30,"BlockAddVector:start_pos=(%.2f,%.2f); opp_pos=(%.2f,%.2f)",
		  start_pos.x,start_pos.y,opp_pos.x,opp_pos.y);
  if(!l.InBetween(pr_dot,target,start_pos+Vector(-1.0f,0.0f))){
    add=(opp_pos+Vector(-1.0f,0.0f)-pr_dot).mod();
    Mem->LogAction3(30,"I`m backward of opp, so add %.2f m",add);
  }
  float d2=l.dist(my_pos);
  float d1=pr_dot.dist(opp_pos);
  float d3=d1<FLOAT_EPS?0.0f:(Sqr(d2)-Sqr(d1))/(2*d1);
  Vector vector_dif=((target-pr_dot)*(d3+1.0f+add))/(target-pr_dot).mod();
  Vector res=vector_dif+pr_dot;
  //AI:корректируем, чтобы не сближаьться с вратарем
  res.x=min(Mem->BallX(),max(res.x,-Mem->SP_pitch_length/2.0f+Mem->SP_goal_area_length+2.0f));
  //  res.y=l.get_y(res.x);
  
  return res;
}
///////////////////////////////////////////////////////////////////////////////
Vector Defense::GetFastDefensePos(const Vector& ball){
  float dy=0.0f;
  if(ball.y>Mem->SP_goal_area_width/2)
    dy=1.0f;
  if(ball.y<-Mem->SP_goal_area_width/2)
    dy=-1.0f;
  float x=GetMyType()<=PT_Defender?-43.0f:-30.0f;
  if(GetMyType()==PT_Midfielder&&GetMySide()==PS_Center)
    x=-37.0f;
  else
    if(GetMyType()==PT_Forward)
      x=-5.0f;
  Vector res=Vector(x,GetHomePosition().y+dy*3.0);
  if(GetMySide()!=PS_Center&&GetMyType()==PT_Defender)
    res.y=(GetMySide()==PS_Left?-1.0f:1.0f)*Mem->SP_goal_width/2;
  Mem->LogAction5(50,"GetFastDefensePos: new def pos: (%.2f,%.2f) dy=%.0f)",res.x,res.y,dy);
  return res;
}
//////////////////////////////////////////////////////////////////////////////
bool Defense::MarkOpponent(Unum opp,MarkType type,float dist,Vector markPos)
{
  if(markPos.x==777.0f)
    markPos=Mem->OpponentAbsolutePosition(opp);
  Mem->LogAction4(30, "Marking opponent %.0f;type: %.0f",float(opp),float(type));
  AngleDeg mark_ang;
  AngleDeg  goal_ang = (Mem->MarkerPosition(Mem->RM_My_Goal) - markPos).dir();
  AngleDeg  ball_ang = (Mem->BallAbsolutePosition()          - markPos).dir();
  switch(type){
  case mark_bisector:
    mark_ang = AngleBisect(goal_ang,ball_ang);
    break;
  case mark_goal:
    mark_ang=goal_ang;
    break;
  case mark_ball:
    mark_ang=ball_ang;
  };

  markPos+=Polar2Vector(dist, mark_ang);
  AngleDeg my_ang=(markPos-Mem->MyPos()).dir();
  Mem->LogAction5(30,"MarkOpponent: ball_ang=%.2f; my_ang=%.2f; diff_ang=%.2f",ball_ang,my_ang,GetDiff(ball_ang,my_ang));
  if(GetDiff(my_ang,ball_ang)>135.0f&&Mem->MyX()<-30.0f){
    if(MoveBackToPos(markPos,10.0,0.0))
      return true;
  }else{  
    if ( MoveToPos (markPos,10.0f,0.0f,false))
      return true;
  }

  if(Mem->MyX()<-30.0f){
    Mem->LogAction2(30,"MarkOpponent: already at pos but must turn body to goal");
    face_only_body_to_point(Vector(-52.5,Mem->MyY()));
  }else{
    Mem->LogAction2(30,"MarkOpponent: already at pos but must turn body to ball");
    face_only_body_to_ball();
  }
  return true;
}
//////////////////////////////////////////////////////////////////////////////
void Defense::CheckForStartRest(){
  if(IsRest())
    return;
  if(Mem->MyStamina()<=Mem->SP_stamina_max*.6f&&
     Mem->MyX()>-Mem->SP_pitch_length/2+Mem->SP_penalty_area_length&&
     Mem->MyX()<-15.0f&&(Mem->BallX()-Mem->MyX())>=15.0f){
    char msg[10]={0,0,0,0,0,0,0,0,0,0};
    Msg m(msg);
    m<<char(ST_request_for_rest);
    float x=max(-20.0f,Mem->MyX());
    m<<TransferXCoor(x);
    char time_for_recovery=(char)Min(Communicate::max_num,(Mem->SP_stamina_max-Mem->MyStamina())/Mem->GetMyStaminaIncMax());
    m<<time_for_recovery;
    Mem->AddMyPos(m);
    Mem->LogAction4(10,"CheckForStartRest: must rest; x=%.2f; time_for_rest=%.0f",x,float(time_for_recovery));
    if(GetMyType()==PT_Sweeper){
      Mem->LogAction2(10,"As sweeper start rest");
      Mem->SayNow(msg);
      x_to_rest=x;
      cycles_to_rest=time_for_recovery;
      start_rest=Mem->CurrentTime+1;
    }else if(SweeperAttentionAtTime(Mem->CurrentTime)==Mem->MyNumber){
      Mem->LogAction2(10,"I may send request for rest");
      Mem->SayNow(msg);
    }
  }
}
//////////////////////////////////////////////////////////////////////////////
Unum Defense::SweeperAttentionAtTime(Time time){
  //change attention to next defender in each cycle
  int num_defenders=0;
  for(int i=1;i<=Mem->SP_team_size;i++)
    if(GetPlayerType(i)==PT_Defender) num_defenders++;
  Iterator iter=begin();
  int send_value=time%num_defenders,index=0;
  while(iter!=end()){
    Unum tm=GetPlayerNumber(iter,PT_Defender);
    if(tm==Unum_Unknown) break;
    if(index==send_value) return tm;
    index++;
  }
  my_error("Error in SweeperAttentionAtTime");
  return Unum_Unknown;
}
//////////////////////////////////////////////////////////////////////////////
Unum Defense::GetDefenseAttention(){//return Unum_Unknown if not need to use this protocol
  if((Mem->BallX()-Mem->MyX())<15.0f)
    return Unum_Unknown;
  if(GetMyType()==PT_Sweeper){
    return SweeperAttentionAtTime(Mem->CurrentTime);
  }else if(GetMyType()==PT_Defender){
    Iterator iter=begin();
    Unum sweeper=GetPlayerNumber(iter,PT_Sweeper);
    if(sweeper==Unum_Unknown){
      my_error("In this formation we have not sweeper so not use protocol of defense");
      return Unum_Unknown;
    }
    return sweeper;
  }else{
    my_error("Wrong call of GetDefenseAttention");
    return Unum_Unknown;
  }
}
//////////////////////////////////////////////////////////////////////////////
bool Defense::RecievedDefenseCommunicate(sayType st,Msg m,Unum from,Time t){
  if(st==ST_request_for_rest){
    char tmp;
    float x;
    m>>TransferXCoor(&x)>>tmp;
    Mem->GetMyPos(m,from,t);
    if(GetPlayerType(from)==PT_Sweeper){
      cycles_to_rest=int(tmp);
      x_to_rest=x;
      start_rest=Mem->CurrentTime;
      Mem->LogAction4(50,"Recieved msg from sweeper: rest at %.2f for %.0f cycles",x_to_rest,float(cycles_to_rest));
    }else if(GetMyType()==PT_Sweeper){
      if(!IsRest()||(start_rest+cycles_to_rest<Mem->CurrentTime+1+int(tmp))){
	Mem->LogAction5(10,"Recieved request to rest from %.0f (x=%.2f; time=%.0f)",float(from),x,float(tmp));
	char msg[10]={0,0,0,0,0,0,0,0,0,0};
	Msg mLocal(msg);
	mLocal<<char(ST_request_for_rest)<<TransferXCoor(x)<<tmp;
	Mem->AddMyPos(mLocal);
	cycles_to_rest=int(tmp);
	x_to_rest=x;
	start_rest=Mem->CurrentTime+1;
	Mem->LogAction2(10,"As sweeper start rest");
	Mem->SayNow(msg);
      }				
    }
    return true;
  }
  return false;
}

