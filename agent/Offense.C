/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Offense.C
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.14 $
 *
 *    $Id: Offense.C,v 2.14 2004/06/22 17:06:16 anton Exp $
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
#include "Handleball.h"
#include "Offense.h"
#include "dribble.h"
///////////////////////////////////////////////////////////////////////////////////////////////////
void Offense::offense(Unum num){  
  bool is_i=num==Mem->MyNumber;//must true for execute

  if(((is_i&&Mem->MyInterceptionAble())||(!is_i&&Mem->TeammateInterceptionAble(num)))&&FastestTm()==num){
    if(is_i){
      Mem->ResetReceiver();
      Mem->LogAction2(10,"Offense:I am fastest and I go to ball");
//       throughPass.StartThroughPass();
//       if(throughPass.IsIStartThroughPass())
// 	eye.AddPosition(throughPass.GetThroughPassTarget(),5,4,RT_WithoutTurn,VW_Normal);
      get_ball();
      return;
    }
    holder=Holder(t_get_ball,"get ball");
    return;  	
  }
  Unum temp=Mem->FastestOpponentToBall();
  Unum temp3=Mem->ClosestTeammateToBall();
  if((FastestTm()==num||FastestTm()==Unum_Unknown)&&(temp3==Unum_Unknown||temp3==num) &&(temp==Unum_Unknown|| Mem->OpponentDistanceTo(temp,Mem->BallAbsolutePosition())>6.0)){
    if(is_i){
      Mem->LogAction2(10,"Offense:go to nobody ball becouse im closest teammate");
      if(Mem->MyInterceptionAble())
	get_ball();
      else
	go_to_point(Mem->BallAbsolutePosition(),Mem->CP_at_point_buffer,100.0);
      return;
    }
    if(Mem->TeammateInterceptionAble(num))
      holder=Holder(t_get_ball,"get ball then nobody go to ball");
    else
      holder=Holder(t_go_to_pos,Mem->BallAbsolutePosition(),5.0f,100.0f,"go to ball position then nobody go to ball");
    return;
  }
  if(is_i&&Mem->TheirPenaltyArea.IsWithin(Mem->MyPos())&&fabs(Mem->TeammateY(FastestTm())-Mem->MyY())<10.0f&&
     Mem->MyInterceptionAble()&&Mem->MyInterceptionNumberCycles()<=5){//hack
    Mem->LogAction2(10,"I go to ball if tm skip ball");
    return;
  }
  
  if(is_i&&Mem->MyStamina()<=Mem->SP_stamina_max*0.7f&&Mem->MyX()<30.0f&&
     ((TmCycles()-2>OppCycles()&&GetMyType()==PT_Forward)||(IsOffense()&&GetMyType()==PT_Midfielder&&Mem->BallX()>Mem->MyX()+5.0f))&&
     Mem->MyX()>Mem->their_offside_line+2.0f){
    Mem->LogAction2(10,"I tired, so rest");
    face_only_body_to_ball();
    return;
  }
	
  if(PlayAtTheirPenaltyArea(num))
    return;

  if(fast_attack(num))
    return;

  if(PositioningOnOpponentDefenseLine(num))
    return;

  if(WingMidfielderBehavior(num))
    return;
  if(CorrectPositionToHelpTmBreakaway(num))
    return;
  
  if(is_i){
    Mem->LogAction2(10,"Offense:nothing to do - go to home position");
    GoToHomePosition();
    return;
  }
  holder=Holder(t_go_to_pos,GetHomePosition(num),5.0,GetFormationDash(num),"go to home pos at end");
  return;
}
//////////////////////////////////////////////////////////////////////
bool Offense::CorrectPositionToHelpTmBreakaway(Unum tm)
{
  const float Y_BUF=5.0f;
  bool is_i=tm==Mem->MyNumber;  
  if(Mem->BallX()>Mem->TeammateX(tm)||!TmInter()||fabs(Mem->BallY()-Mem->TeammateY(tm))>Y_BUF)
    return false;
  if(Mem->TeammateX(tm)-Mem->BallX()>15.0f)
    return false;
  Vector res=Mem->TeammateAbsolutePosition(tm);
  res.y-=signf(Mem->BallY()-res.y)*(Y_BUF-fabs(Mem->BallY()-res.y));
  res.y=signf(res.y)*min(Mem->SP_pitch_width/2.0f-2.0f,fabs(res.y));
  
  if(is_i){
    Mem->LogAction4(10,"CorrectPositionToHelpTmBreakaway: go to pos(%.2f,%.2f)",res.x,res.y);
    MoveToPos(res,20.0f,0.0,false);
  }else{
    holder=Holder(t_go_to_pos,res,20.0f,Mem->SP_max_power,
		  "go in CorrectPositionToHelpTmBreakaway");
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
bool Offense::WingMidfielderBehavior(Unum tm)
{
  bool is_i=tm==Mem->MyNumber;
  if(GetPlayerType(tm)!=PT_Midfielder||GetPlayerSide(tm)==PS_Center)
    return false;
  if(Mem->BallX()<Mem->TeammateX(tm)&&(FastestTm()==Unum_Unknown||GetPlayerType(FastestTm())<=PT_Midfielder))
    return false;
  if(is_i&&Mem->MyStamina()<Mem->SP_stamina_max*0.5f){
    Mem->LogAction2(10,"WingMidfielderBehavior: i tired to open in this function");
    return false;
  }
  
  Vector target;
  if(Mem->GetBallSide()==GetPlayerSide(tm)){
    target=Vector(Mem->my_offside_line-5.0f,fabs(Mem->TeammateY(tm)-GetHomePosition(tm).y)<2.0f?GetHomePosition(tm).y:Mem->TeammateY(tm));
  }else{
    target=Vector(Mem->my_offside_line-9.0f,GetHomePosition(tm).y);
  }
  if(fabs(Mem->BallY()-target.y)<7.0f){
    target.y-=signf(Mem->BallY()-target.y)*(7.0f-fabs(Mem->BallY()-target.y));
  }
  for(int i=1;i<=11;i++){
    if(i==tm||!Mem->TeammatePositionValid(i)||Mem->TeammateX(i)<Mem->TeammateX(tm))
      continue;
    if(fabs(Mem->TeammateY(i)-target.y)<5.0f){
      target.y-=signf(Mem->TeammateY(i)-target.y)*(5.0f-fabs(Mem->TeammateY(i)-target.y));
      break;
    } 
  }
  
  target.y=signf(target.y)*min(Mem->SP_pitch_width/2.0f-2.0f,fabs(target.y));
  
  if(is_i){
    Mem->LogAction4(10,"WingMidfielderBehavior: go to pos(%.2f,%.2f)",target.x,target.y);
    MoveToPos(target,20.0f,0.0,false);
  }else{
    holder=Holder(t_go_to_pos,target,20.0f,Mem->SP_max_power,
		  "go in WingMidfielderBehavior");
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
struct WidthSort
{
  WidthSort(float p,float y_opp):prior(p),y(y_opp){}
  bool operator < (const WidthSort& os)const{return prior<os.prior;}
  float prior;
  float y;
};
//////////////////////////////////////////////////////////////////////
bool Offense::PositioningOnOpponentDefenseLine(Unum tm)
{
  const float MAX_Y_DIST=15.0f;
  const float BUFFER_DIST_TO_BALL=5.0f;
  bool is_i=tm==Mem->MyNumber;
  if(GetPlayerType(tm)!=PT_Forward||Mem->my_offside_line-Mem->BallX()>15.0f||(is_i&&Mem->MyStamina()<Mem->SP_stamina_max*0.8f))
    return false;
  priority_queue<float> opp_list;
  float x_value=Mem->my_offside_line-1.0f-min(10.0f,(max(15.0f,Mem->my_offside_line-Mem->BallX())-15.0f)*0.5f);
  for(int i=1;i<=11;i++){
    if(Mem->OpponentPositionValid(i)<0.7f||fabs(Mem->OpponentX(i)-x_value)>7.0f)
      continue;
    if(is_i)
      Mem->LogAction5(10,"PositioningOnOpponentDefenseLine:add opp %.0f with conf %.2f (y=%.2f)",
		      float(i),Mem->OpponentPositionValid(i),Mem->OpponentY(i));
    opp_list.push(Mem->OpponentY(i));
  }
  opp_list.push(Mem->SP_pitch_width/2);
  opp_list.push(-Mem->SP_pitch_width/2);
  if(Mem->BallY()<Mem->SP_pitch_width/2-BUFFER_DIST_TO_BALL)
    opp_list.push(Mem->BallY()+BUFFER_DIST_TO_BALL);
  if(Mem->BallY()>-Mem->SP_pitch_width/2+BUFFER_DIST_TO_BALL)
    opp_list.push(Mem->BallY()-BUFFER_DIST_TO_BALL);
  priority_queue<WidthSort> width_list;
  float up,down,pos,width,prior;
  float home_y=GetHomePosition().y;
  while(opp_list.size()>2){
    up=opp_list.top();
    opp_list.pop();
    down=opp_list.top();
    width=up-down;
    pos=(up+down)/2.0f;
    if(fabs(up-home_y)+fabs(down-home_y)>2.0f*MAX_Y_DIST||width<4.0f||
       SelectOptimalPlayer(PT_Forward,PS_All,Vector(x_value,pos))!=tm||
       fabs(Mem->BallY()-pos)>25.0f)
      continue;
    prior=width/(Mem->TeammateDistanceTo(tm,Vector(x_value,pos))+Mem->BallAbsolutePosition().dist(Vector(x_value,pos)));
    if(Mem->BallY()<up&&Mem->BallY()>down){
      if(is_i)
	Mem->LogAction5(10,"Ignore (%.2f,%.2f) with priority %.2f becouse ball is where",down,up,prior);
      continue;
    }
    
    if(is_i)
      Mem->LogAction4(10,"Add y=%.2f with priority %.2f",pos,prior);
    width_list.push(WidthSort(prior,pos));
  }
  if(width_list.empty())
    return false;
  pos=width_list.top().y;
  if(fabs(Mem->TeammateY(tm)-Mem->BallY())<5.0f)
    pos=Mem->BallY()+signf(Mem->TeammateY(tm)-Mem->BallY())*5.0f;
  pos=MinMax(-Mem->SP_pitch_length/2+1.0f,pos,Mem->SP_pitch_length/2-1.0f);
  prior=width_list.top().prior;
  if(is_i){
    Mem->LogAction5(10,"Go to pos(%.2f,%.2f) with priority %.2f",x_value,pos,prior);
    MoveToPos(Vector(x_value,pos),20.0f,3.0,false,
	      min(Mem->SP_max_power,(Mem->MyStamina()/Mem->SP_stamina_max+0.1f)*Mem->SP_max_power));
    eye.AddPosition(Vector(x_value,-signf(Mem->MyY())*Mem->SP_pitch_width/2.0f),3,10,RT_WithoutTurn);
    if(fabs(Mem->MyY())<20.0f)
      eye.AddPosition(Vector(x_value,signf(Mem->MyY())*Mem->SP_pitch_width/2.0f),3,10,RT_WithoutTurn);
  }else{
    holder=Holder(t_go_to_pos,Vector(x_value,pos),20.0f,Mem->SP_max_power,
		  "go to free pos in defense line");
  }
  return true;
}
//////////////////////////////////////////////////////////////////////
bool Offense::WingForwardBehavior(Unum tm){//поведение нападающих на флангах
  bool is_i=tm==Mem->MyNumber;
  if(GetPlayerType(tm)!=PT_Forward/*||GetPlayerSide(tm)==PS_Center*/)
    return false;
  float offside=Mem->my_offside_line-1.0f;
  Vector home=Vector(Min(GetHomePosition(tm).x,offside),GetHomePosition(tm).y),ball=Mem->BallAbsolutePosition(),opp_pos;
  Unum opp=Mem->ClosestOpponentTo(home);
  if(opp==Unum_Unknown)
    opp_pos=Vector(-50.0f,0.0f);
  else
    opp_pos=Mem->OpponentAbsolutePosition(opp);
  if(is_i) Mem->LogAction4(30,"WingForwardBehavior: our opponent is %d (conf %.0f)",opp,Mem->OpponentPositionValid(opp));
  if(fabs(offside-ball.x)<7.0){
    if(Mem->OurBreakaway()){
      if(is_i) Mem->LogAction2(10,"WingForwardBehavior: we have breakaway, so go fast forward");
      if(fast_attack(tm)) return true;
    }
    //    if(Mem->TeammateX(tm)>opp_pos.x||(Mem->OurBreakaway()&&Mem->TeammateX(tm)<ball.x)){
    if(is_i){
      Mem->LogAction2(10,"WingForwardBehavior: breakaway on wing (or we backward of ball),so go fast forward");
      MoveToPos(Vector(offside,home.y),10.0f,0.0f);
      return true;
    }
    holder=Holder(t_go_to_pos,Vector(offside,home.y),10.0f,Mem->SP_max_power,"breakaway on wing (or we backward of ball),so go fast forward");
    return true;
    //    }
    //    int cyc;
    //    Vector pos=GetOptimalPointOnLine(Mem->BallEndPosition(&cyc),Vector(offside,home.y),Vector(ball.x-5.0,home.y));
    //    if(is_i){
    //      Mem->LogAction8(10,"WingForwardBehavior: open on wing line to pos (%.2f,%.2f). Start (%.2f%.2f) end (%.2f,%.2f)",
    //        pos.x,pos.y,offside,home.y,ball.x-5.0,home.y);
    //      MoveToPos(pos,10.0f,3.0f);
    //      return true;
    //    }
    //    holder=Holder(t_go_to_pos,pos,10.0f,Mem->SP_max_power,"open on wing line");
    //    return true;
  }
  //мяч вделеке от линии оффсайда
  return false;
  //  int cyc;
  //  AngleDeg res;
  //  Vector pos=GetOptimalPointOnLine(Mem->BallEndPosition(&cyc),
  //    Vector(home.x,home.y-Sign(home.y)*7.0),Vector(home.x,home.y+Sign(home.y)*7.0),&res);
  //  if(is_i){
  //    Mem->LogAction5(10,"WingForwardBehavior: open on wing line to pos (%.2f,%.2f). Angle %.2f",
  //      pos.x,pos.y,res);
  //    if(res<30.0f) return false;
  //    MoveToPos(pos,10.0f,3.0f);
  //    return true;
  //  }
  //  if(res<30.0f)
  //    return false;
  //    holder=Holder(t_go_to_pos,pos,10.0f,Mem->SP_max_power,"open on wing line");
  //  return true;
}
///////////////////////////////////////////////////////////
bool Offense::PlayAtTheirPenaltyArea(Unum num){
  Cross cross;
  if(Mem->BallX()<30.0)
    return false;
  if(GetPlayerType(num)>PT_Midfielder&&cross.open_for_cross(num,holder))
    return true;
  if(positioning_in_their_penalty_area(num))
    return true;
  //позиционируем полузащиту
  if(GetPlayerType(num)==PT_Midfielder&&GetPlayerSide(num)==PS_Center){
    if(num==Mem->MyNumber){
      Mem->LogAction2(10,"PlayAtTheirPenaltyArea: positioning close to penalty area");
      MoveToPos(Vector(33.0f,GetHomePosition(num).y),20.0,0.0);
    }else{
      holder=Holder(t_go_to_pos,Vector(33.0f,GetHomePosition(num).y),20.0f,Mem->SP_max_power,"center midfielder go closer");
    } 
    return true;
  }
    
  return false;
}
///////////////////////////////////////////////////////////
bool Offense::ForwardPlayInCentralZone(Unum tm){//very simple for Austrelia
  if(GetPlayerType(tm)!=PT_Forward) return false;
  float line=Mem->my_offside_line-1.0f,max=0.0f,conf;
  Vector home=GetHomePosition(tm);
  line=fabs(home.x-line)>15.0?home.x+15.0f:line;
  int number=1,cyc;
  Vector ball=Mem->BallEndPosition(&cyc),opt;
  Unum opp;
  bool on_line=false;
  if(fabs(ball.x-home.x)<5.0f){
    number=3;
    on_line=true;
  }
  for(int i=0;i<number;i++){
    for(int j=0;j<=6;j+=on_line?6:2){
      Vector pos(line-i*3.0f,home.y-Sign(home.y)*j);
      opp=Mem->ClosestOpponentTo(pos);
      if(opp==Unum_Unknown){
        max=1.0f;
        opt=pos;
        break;
      }
      if((conf=Mem->OpponentDistanceTo(opp,pos))>max){
        max=conf;
        opt=pos;
      }
      if(GetPlayerSide(tm)!=PS_Center) continue;
      pos.y=home.y+Sign(home.y)*j;
      opp=Mem->ClosestOpponentTo(pos);
      if((conf=Mem->OpponentDistanceTo(opp,pos))>max){
        max=conf;
        opt=pos;
      }
    }
  }
  if(max<=4.0f)
    return false;
  if(tm==Mem->MyNumber){
    Mem->LogAction5(10,"ForwardPlayInCentralZone: pos(%.2f,%.2f); dist: %.2f",opt.x,opt.y,max);
    if(!MoveToPos(opt,30.0,0.0,false))
      face_only_body_to_ball();
    return true;
  }
  holder=Holder(t_go_to_pos,opt,30.0f,Mem->SP_max_power,"go in ForwardPlayInCentralZone");
  return true;
}
//////////////////////////////////////////////////////////
bool Offense::positioning_in_their_penalty_area(Unum tm){
  bool is_i=tm==Mem->MyNumber;
  Vector ball=TmInter()?
    Dribble::PredictDribbleStopPosition(FastestTm(),Dribble::SelectDribbleTargetForTeammate(FastestTm(),tm==Mem->MyNumber),
					0.6f,is_i?true:false):Mem->BallAbsolutePosition();
  bool alien=TmInter()?GetPlayerType(FastestTm())<PT_Forward:false;
  if(is_i)
    Mem->LogAction4(10,"PITPA:: answer of dribble stop position is (%.2f;%.2f)",ball.x,ball.y);

  Vector res;
  float offside=Mem->my_offside_line-1.0f;
  if(GetPlayerType(tm)!=PT_Forward) return false;
  float x_line=max(offside,ball.x);
  if(GetPlayerSide(tm)!=PS_Center){
    res=Vector(x_line,(GetPlayerSide(tm)==PS_Left?-1.0f:1.0f)*7.0f);
    if(alien&&signf(ball.y)==signf(res.y)&&fabs(ball.y)>6.0f){
      res.y=ball.y-signf(ball.y)*2.0f;
      if(fabs(res.y)<6.0f)
	res.y=ball.y+signf(ball.y)*2.0f;
      res.x=x_line-2.0f;
      if(is_i)
	Mem->LogAction4(10,"PITPA:: alien is go with ball, so correct my pos to (%.2f;%.2f)",res.x,res.y);
    }
    
  }else{
    res=Vector(Min(48.0f,x_line),0.0f);
    if(alien&&fabs(ball.x)<4.0f){
      res.y=ball.y+signf(GetTmPos(tm).y-ball.y)*6.0f;
      res.x=x_line-2.0f;
      if(is_i)
	Mem->LogAction4(10,"PITPA:: alien is go with ball, so correct my pos to (%.2f;%.2f)",res.x,res.y);      
    }
    
  }
  if(!alien){
    Line l;
    l.LineFromTwoPoints(Pos.GetTmPos(tm),res);
    res.y=l.get_y(x_line);//таким образом пытаемся убрать лишнии повороты тела
  }
  
  //делаем поведение центр-форварда более четким
  Unum fastest=FastestTm();
  if(fastest!=Unum_Unknown&&Mem->TeammatePositionValid(fastest)&&fabs(GetTmPos(fastest).y)>5.0f&&!alien&&
     GetPlayerSide(fastest)==PS_Center&&Sign(GetTmPos(fastest).y)==Sign(GetHomePosition(tm).y)){
    res=Vector(Min(48.0f,x_line),0.0f);
  }
  //добавляем зрение
  if(is_i){
    Vwidth vw=VW_Normal;
    if(Mem->DistanceTo(Mem->BallAbsolutePosition())<7.0f)
      vw=VW_Narrow;
    eye.AddPosition(Vector(52.5,0.0),5,11,RT_WithoutTurn,vw);
    if(fabs(Mem->MyY())>6.0f)
      eye.AddPosition(Vector(Mem->MyX(),-signf(ball.y)*30.0f),5,12,RT_WithoutTurn,vw);
  }
  //корректируем x составляющую
  //  bool tired=tm==Mem->MyNumber&&Mem->MyStamina()<.5f*Mem->SP_stamina_max; 
//   if(!tired){
//     for(int i=1;i<=11;i++){
//       if(Mem->OpponentPositionValid(i)<0.8f||i==Mem->TheirGoalieNum)
// 	continue;
//       if(Mem->OpponentX(i)>Mem->TeammateX(tm)&&fabs(Mem->OpponentX(i)-ball.x)<=1.5f){
// 	if(is_i)
// 	  Mem->LogAction4(10,"PITPA: opp %.0f (conf %.2f) has danger in x",float(i),Mem->OpponentPositionValid(i));
// 	float y_dif=signf(Mem->BallY())*(Mem->OpponentY(i)-res.y);
// 	if(y_dif>-1.0f&&y_dif<4.0f){
// 	  if(is_i)
// 	    Mem->LogAction2(10,"PITPA: this opponent is BAD in Y");
// 	  res.x=min(ball.x,res.x-5.0f);
// 	  break;
// 	}
//       }
//     }
//   }
  
  //корректируем y составляющую
  Unum opp=Mem->ClosestOpponentTo(res);
  if(fabs(Mem->TeammateX(tm)-res.x)<=2&&opp!=Unum_Unknown&&Mem->OpponentX(opp)+0.5f>Mem->TeammateX(tm)&&
     fabs(Mem->TeammateY(tm)-Mem->OpponentY(opp))<=2.0f){
    if(is_i)
      Mem->LogAction3(10,"PITPA: opp %.0f is VARY bad gay, so try go to ball",float(opp));
    res.y=ball.y;
    if(GetMySide()!=Mem->GetBallSide()&&GetMySide()!=PS_Center)
      res.y=0.0f;
    res.x=x_line;
  }
  //корректируем у-составляющую, чтобы убрать пересечение с мячом
  if(fabs(ball.y-res.y)<6.0f)
    res.y=ball.y-signf(ball.y-Mem->TeammateY(tm))*6.0f;
 
  if(is_i){
    if(res.dist(Mem->MyPos())<3.0f)
      res.x=min(Mem->my_offside_line-(Mem->my_offside_opp==Unum_Unknown?1.0f:0.5f),res.x);
    Mem->LogAction4(10,"PITPA: go to (%.2f,%.2f)",res.x,res.y);
    if(!MoveToPos(res,20.0f,0.0))
      face_only_body_to_point(Vector(52.5,Mem->MyY()));
  }else
    holder=Holder(t_go_to_pos,res,20.0f,Mem->SP_max_power,"go in PITPA");
  return  true;
}
///////////////////////////////////////////////////////////
void Offense::open_from_point2point(Unum tm,Vector from,Vector to){
  Unum opp;
  bool is_i=tm==Mem->MyNumber;
  if(actions.PassFromPoint2Point(from,to,opp)>0.85&&(from-to).mod()>10.0){
    if(is_i){
      Mem->LogAction4(10,"open_form_point2point:go to point (%f,%f) for good pass",to.x,to.y);
      if(go_to_point(to,Mem->CP_at_point_buffer,GetFormationDash())==AQ_ActionNotQueued){
	Mem->LogAction3(10,"open_from_point2point: can recieved good pass so wait for ball(the more dangerouse opp: %d",opp);
	face_only_body_to_ball();
	return;      
      }else{
	return;
      }
    }else{
      holder=Holder(t_go_to_pos,to,5.0f,GetFormationDash(tm),"open for pass");
      return;
    }
  }else
    if((from-to).mod()<=10.0){
      if(is_i){
	Mem->LogAction2(10,"We too close for ball to wait for pass, so go to home position");
	GoToHomePosition();
	return;
      }else{
	holder=Holder(t_go_to_pos,GetHomePosition(tm),5.0f,GetFormationDash(tm),"player to close to ball");
	return;
      }
    }
  if(opp==Unum_Unknown){//is this possible?(may be only in test version)
    if(is_i){
      Mem->LogAction2(10,"open_from_point2point: I don't now opponent so i go to home position");
      GoToHomePosition();
      return;
    }else{
      holder=Holder(t_go_to_pos,GetHomePosition(tm),5.0f,GetFormationDash(tm),"opp unknown go to home pos");
      return;
    }
  }
  Line l;
  l.LineFromTwoPoints(from,to);
  bool plane=l.HalfPlaneTest(Mem->OpponentAbsolutePosition(opp));
  Line targetLine;
  Vector targetPoint;
  bool go_to_ball=false;
  if(fabs(l.angle())<45.0){
    targetLine.LineFromTwoPoints(to,Vector(to.x,to.y+5.0));		
    if(!plane){
      if(to.y>29.0)
	go_to_ball=true;
      targetPoint=Mem->FieldRectangle.BottomEdge().intersection(targetLine);
    }else{
      if(to.y<-29.0)
	go_to_ball=true;
      targetPoint=Mem->FieldRectangle.TopEdge().intersection(targetLine);
    }
  }else{
    targetLine.LineFromTwoPoints(to,Vector(to.x+5.0,to.y));		
    if(plane){
      if(to.x>45.0||(Mem->my_offside_line-2)<to.x)//too close to end of field or too close to offside line
	go_to_ball=true;
      targetPoint=Mem->FieldRectangle.RightEdge().intersection(targetLine);
    }else{
      if(to.x<-45.0)
	go_to_ball=true;
      targetPoint=Mem->FieldRectangle.LeftEdge().intersection(targetLine);
    }
  }
  if(go_to_ball){
    if(is_i){
      Mem->LogAction4(10,"open_from_point2point: to close to field rect (or to offside line), so go to point (%f,%f)",
		      from.x,from.y);
      go_to_point(from,Mem->CP_at_point_buffer,GetFormationDash());
      return;
    }else{
      holder=Holder(t_go_to_pos,from,5.0f,GetFormationDash(tm),"to close to field rect");
      return;
    }      
  }
  if(is_i){
    Mem->LogAction4(10,"open_from_point2point:go to point (%f,%f) (this point must be on field rect)  for good pass",
		    targetPoint.x,targetPoint.y);
    go_to_point(targetPoint,Mem->CP_at_point_buffer,GetFormationDash());
  }else{
    holder=Holder(t_go_to_pos,targetPoint,5.0f,GetFormationDash(tm),"open_from_point2point");
    return;
  }
}
///////////////////////////////////////////////////////////
Vector Offense::get_dir_to_open_for_pass(Vector from,Vector to){
  Unum opp;
  if(actions.PassFromPoint2Point(from,to,opp)>0.85){
    Mem->LogAction4(50,"get_dir_to_open_for_pass:go to point (%f,%f) for good pass",to.x,to.y);
    return Vector(0,0);
  }
  if(opp==Unum_Unknown){//is this possible?(may be only in test version)
    my_error("get_dir_to_open_for_pass:danger opponent is don't nown!!!");
    return Vector(0,0);
  }
  Line l,targetLine;
  l.LineFromTwoPoints(from,to);
  bool plane=l.HalfPlaneTest(Mem->OpponentAbsolutePosition(opp));
  bool go_to_ball=false;
  if(fabs(l.angle())<45.0){
    targetLine.LineFromTwoPoints(to,Vector(to.x,to.y+5.0));		
    if(!plane){
      if(to.y>29.0)
	go_to_ball=true;
      else
	return Vector(0,1);
    }else{
      if(to.y<-29.0)
	go_to_ball=true;
      else
	return  Vector(0,-1);
    }
  }else{
    targetLine.LineFromTwoPoints(to,Vector(to.x+5.0,to.y));		
    if(plane){
      if(to.x>45.0||(Mem->my_offside_line-2)<to.x)//too close to end of field or too close to offside line
	go_to_ball=true;
      else
	return Vector(1,0);
    }else{
      if(to.x<-45.0)
	go_to_ball=true;
      else
	return Vector(-1,0);
    }
  }
  Mem->LogAction4(50,"get_dir_to_open_for_pass: to close to field rect (or to offside line), so go to point (%f,%f)",from.x,from.y);
  return (to-from)/((to-from).mod());
}
///////////////////////////////////////////////////////////

bool Offense::can_open_for_pass(Unum tm,Vector from){
  if ( Mem->InOwnPenaltyArea(Mem->TeammateAbsolutePosition(tm)) ) return false;
  if ( Mem->TeammateDistanceTo(tm,Vector(-Mem->SP_pitch_length/2.0f,0.0)) < 25 ) return false;
  if(from.dist(Mem->TeammateAbsolutePosition(tm))>40.0||from.dist(Mem->TeammateAbsolutePosition(tm))<10.0) return false;
  Unum possesor=Mem->BallPossessor();
  if(possesor<0||possesor==Unum_Unknown||possesor==Mem->OurGoalieNum){
    return false;
  }
  return true;//TEMP
}
///////////////////////////////////////////////////////////
bool Offense::open_for_pass(Unum tm){
  //      if(open_on_checkpoints())//we MUST add it later
  //      	return true;
  Vector targetPoint=TmPoint();
  //we must open to targetPoint vector
  if(can_open_for_pass(tm,targetPoint)){
    open_from_point2point(tm,targetPoint,Mem->TeammateAbsolutePosition(tm));
    return true;
  }
  return false;
}
///////////////////////////////////////////////////////////
bool Offense::open_on_checkpoints(void){
  int begin=0;
  //    static Pass pass;
  bool ok=false;
  if((Mem->BallX()+5.0)<=Mem->MyX()||Mem->SetPlay){//in set play we can open behinde ball
    for(;begin<4;begin++){
      if((Mem->BallX()+5.0)>=(Mem->AttackCheckPointsX[begin]-Mem->SP_pitch_length/2)){
	ok=true;
	break;
      }
    }
    if(ok==false||begin==3)
      return false;
    begin++;//1 - 2 - 3
    float x;
    Unum t[11];
    float beginY=Mem->SP_pitch_width/2-Mem->AttackZoneYOffset;
    float endY=beginY-Mem->AttackZoneWidth;
    float delta=1.0;
    if(Mem->BallY()<0){
      beginY=-beginY;
      endY=-endY;
      delta=-1.0;
    }
    do{
      x=Mem->AttackCheckPointsX[begin]-Mem->SP_pitch_length/2;
      if((Mem->my_offside_line-Mem->CP_at_point_buffer)<x){//support offside
	Mem->LogAction4(40,"Open for pass: correct x for offside:old value = %f,new value = %f",x,Mem->XToOnsideX(x));
	x=Mem->XToOnsideX(x);
	begin=4;
      }
      int num=Mem->SortPlayersByDistanceToPoint('m',Vector(x,(beginY+endY)/2),t);
      if(((num>0&&t[0]==Mem->MyNumber)||(num>1&&t[0]==Mem->TeammateWithBall(5.0)&&t[1]==Mem->MyNumber))&&Mem->DistanceTo(Vector(x,(beginY+endY)/2))<30.0){
	float temp,max=-1;
	Unum opp;
	Vector res;
	for(float dy1=(endY+beginY)/2,dy2=dy1;fabs(dy1)<=fabs(beginY);dy1+=delta,dy2=-delta){
	  if((temp=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),Vector(x,dy1),opp))>max){
	    max=temp;
	    res=Vector(x,dy1);
	  }
	  if(dy1!=dy2 && (temp=actions.PassFromPoint2Point(Mem->BallAbsolutePosition(),Vector(x,dy2),opp))>max){
	    max=temp;
	    res=Vector(x,dy2);
	  }
	}
	Mem->LogAction4(10,"Open for pass: open in check point (%f,%f)",res.x,res.y);
	if(go_to_point(res,Mem->CP_at_point_buffer,100.0)==AQ_ActionQueued){
	  return true;
	}else{
	  Mem->LogAction2(20,"We alredy at check point and open for pass: face neck and body to ball");
	  face_only_body_to_ball();
	  return true;
	}
      }
      begin++;
    }while(begin<4);
  }
  return false;
}
///////////////////////////////////////////////////////////
int Offense::fast_attack(Unum num){
  //return mode= 0-action not queued;1 - already there;2 - go there
  Vector ball=Mem->BallAbsolutePosition();
  if(GetPlayerType(num)<=PT_Midfielder||ball.x<Pos.GetTmPos(num).x)
    return 0;

  float y=Pos.GetTmPos(num).y;
  if(ball.x>30.0){
    if(GetPlayerSide(num)==PS_Center)
      if(fabs(y-GetHomePosition(ball,num).y)>7.0f)
	y=GetHomePosition(Vector(ball.x,0.0),num).y;
    if(GetPlayerSide(num)!=PS_Center&&GetPlayerType(num)==PT_Forward)
      y=(GetPlayerSide(num)==PS_Left?-1.0f:1.0f)*7.0f;
  }
  if(fabs(ball.y-y)<6.0f)
    y=ball.y+signf(y-ball.y)*6.0f;
//   int cyc;
//   float ballEndX=Mem->BallEndPosition(&cyc).x-1.0f;
  y=signf(y)*min(Mem->SP_pitch_width/2.0f-2.0f,fabs(y));
  
  Vector res=Vector(Mem->my_offside_line-1/*Max(ballEndX,Mem->my_offside_line-1)*/,y);
  if(num==Mem->MyNumber){
    Mem->LogAction4(50,"fast_attack: new pos: (%.2f,%.2f)",res.x,res.y);
    if(!MoveToPos(res,15.0,0.0,false)){
      face_only_body_to_ball();
      return 1;
    }
  }else{
    holder=Holder(t_go_to_pos,res,15.0f,Mem->SP_max_power,"go in fast attack");
    return 2;
  }
  return 2;
}
/////////////////////////////////////////////////////////////////////////////
