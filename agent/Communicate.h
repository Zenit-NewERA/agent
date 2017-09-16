/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Communicate.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.5 $
 *
 *    $Id: Communicate.h,v 2.5 2004/06/03 10:04:59 anton Exp $
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

#ifndef COMMUNICATE_H
#define COMMUNICATE_H

#include "MemAction.h"
#include <string>
#include <map>
#include <set>
/**
 *@author Anton Ivanov
 */
enum sayType{//remermber that we have only 9 bytes!!!
  ST_mypos_and_conf_ball_and_conf=0,
  ST_ball_stop_mypos_and_conf_ball_and_conf,
  ST_mypos_and_conf_player_and_conf,
  ST_player_and_conf_ball_and_conf,
  ST_ball_and_conf_ballvel_and_conf,
  ST_ask,
  ST_mypos_and_conf,
  ST_start_scenario,
  ST_stop_scenario,
  ST_pass_decision,
  ST_pass_intention=10,
  ST_begin_through_pass,
  ST_answer_through_pass,
  ST_request_for_rest,
  ST_opponent_kick_ball,
  ST_opponent_have_ball,
  ST_kickable_and_player_and_conf,

  ST_opp1_player_and_conf=20,
  ST_opp2_player_and_conf,
  ST_opp3_player_and_conf,
  ST_opp4_player_and_conf,
  ST_opp5_player_and_conf,
  ST_opp6_player_and_conf,
  ST_opp7_player_and_conf,
  ST_opp8_player_and_conf,
  ST_opp9_player_and_conf,
  ST_opp10_player_and_conf,
  ST_opp11_player_and_conf,
	
  ST_tm1_player_and_conf=40,
  ST_tm2_player_and_conf,
  ST_tm3_player_and_conf,
  ST_tm4_player_and_conf,
  ST_tm5_player_and_conf,
  ST_tm6_player_and_conf,
  ST_tm7_player_and_conf,
  ST_tm8_player_and_conf,
  ST_tm9_player_and_conf,
  ST_tm10_player_and_conf,
  ST_tm11_player_and_conf,

  ST_fastest_tm1=51,
  ST_fastest_tm2,
  ST_fastest_tm3,
  ST_fastest_tm4,
  ST_fastest_tm5,
  ST_fastest_tm6,
  ST_fastest_tm7,
  ST_fastest_tm8,
  ST_fastest_tm9,
  ST_fastest_tm10,
  ST_fastest_tm11,

  ST_none
};//maximum - 74 items

enum aType{
  AT_ball_pos=0,
  AT_player_pos,
  AT_my_pos,
  AT_my_pos_ball_pos,
  AT_ball_pos_player_pos,
  AT_ball_pos_and_vel,
  AT_most_danger_opponent_pos_to_pass,//parametr - number of teammate to pass
  AT_start_set_play_scenario1,
  AT_start_set_play_scenario2,
  AT_none
};

class Msg;

class Communicate : public ActionInfo  {
public: 
  enum GameType{
    GT_zone1=0,//at our goal
    GT_zone2,
    GT_zone3,
    GT_zone4
  };

  Communicate();
  void Initialize(void);
  bool HaveSomethingToSay(void);
  char* SayBuffer(void) const { return const_cast<char*> (say_buffer.c_str());}
  string get_msg(void)const { return msg;}
  void ParsePlayerSound(char* msg,Time time);
  void Say(aType type,Unum p=Unum_Unknown){askT=type;mustAsk=true;askPlayer=p;}
  void SayNow(aType type, Unum p=Unum_Unknown);
  void SayNow(sayType saytype, Unum teammate=Unum_Unknown);
  void SayNow(char* msg);//must have 10 bytes
  void SayNow(string msg){SayNow(msg.c_str());}
  bool HaveSomethingToSayNow(void) const { return have_to_say_now;}
	
  static char code(int value);
  static int decode(char value);

  GameType GetGameType(int& mask);
  
  int SelectOpponents(Unum* opp);

  void CorrectXY(float& x,float& y);

  void AddBallPos(Msg& m);//2+1+1=4 bytes
  void AddBallVel(Msg& m);//2+2+1=5 bytes
  void AddMyPos(Msg& m);//2+1+1=4 bytes
  void AddPlayerPos(Unum player,Msg& m,bool send_number=true);//2+1+1+1=5 bytes (or 4 if not send number)
	
  void GetMyPos(Msg& m,Unum from,const Time& t);
  Unum GetPlayerPos(Msg& m,Unum from,const Time& t,Unum about=Unum_Unknown);//возвращает номер полученного игрока
  void GetBallPos(Msg& m,Unum from,const Time& t);
  void GetBallPosAndVel(Msg& m,Unum from,const Time& t,bool know_vel=false,const Vector& vel=Vector(.0f,.0f),float conf=-1.0f);

  static const int max_num=73;
private:
  Vector GetFrom(Unum from);

  void InitSayBuffer(void);
  void ParseOppMessage(char* msg,Time t);
  void ParseOurMessage(char* msg,Unum from,Time t);
  void ParseMsgBody(char* msg,sayType st,Unum from,Time t);
  void FillMsgBody(char* msg,sayType st);
  int SelectTypeOfMessage(void);
  int MustAnswer();

  bool IsOurTimeToSay(int t);
  int SelectDefenseOppponentsToSay(Unum* opp);

  void SayThatOpponentKickBall();

  void CorrectAttentionTo();
  Unum GetAttentionFromTmSet();

  void UpdateOppMap();
  void UpdateHearBallPosVel(float PosConf,float VelConf,const Time& t);

  void SetCommonTmSet();
  void SetScenarioTmSet();
	
  map<Unum,float> opp_map;
  Time update_opp_map;
  float HearBallPosConf,HearBallVelConf;
  Time HearBallPos,HearBallVel;

  set<int> tm_set;

  string say_buffer;
  string msg;
  Unum player;
  Unum askPlayer;
  Unum playerThatAskQuestion;
  bool mustAnswer;
  bool mustAsk;
  aType askT;
  aType answerT;
  Time LastSayingTime;
  Time LastHearTime;
  bool have_to_say_now;
  string say_now_buffer;
};
/******************************************************/
class TransferObject{
public:
  explicit TransferObject(float v){val=v;p=0;}
  explicit TransferObject(int v){val=float(v);}
  TransferObject(float* pointer){p=pointer;}
  virtual char GetFirstByte()const =0;
  virtual int GetByteNum()const =0;
  virtual char GetSecondByte()const{return Communicate::code(0);}

  virtual void ConstructValue(char* msg)const =0;

  float GetValue()const{return val;}
protected:
  static const int max_num=73;

  float val;
  float* p;
};
/*****************************************************/
class TransferXCoor:public TransferObject{
public:
  explicit TransferXCoor(float v=0.0f):TransferObject(v){}
  TransferXCoor(float* pointer):TransferObject(pointer){}
  char GetFirstByte()const;
  char GetSecondByte()const;
  int GetByteNum()const{return 2;}
  void ConstructValue(char* msg)const;
};
/*****************************************************/
class TransferYCoor:public TransferObject{
public:
  explicit TransferYCoor(float v=0.0f):TransferObject(v){}
  TransferYCoor(float* pointer):TransferObject(pointer){}
  char GetFirstByte()const;
  int GetByteNum()const{return 1;}
  void ConstructValue(char* msg)const;
};
/*****************************************************/
class TransferConf:public TransferObject{
public:
  explicit TransferConf(float v=0.0f):TransferObject(v){}
  TransferConf(float* pointer):TransferObject(pointer){}
  char GetFirstByte()const;
  int GetByteNum()const{return 1;}
  void ConstructValue(char* msg)const;
};
/*****************************************************/
class TransferVel:public TransferObject{
public:
  explicit TransferVel(float v=0.0f):TransferObject(v){}
  TransferVel(float* pointer):TransferObject(pointer){}
  char GetFirstByte()const;
  char GetSecondByte()const;
  int GetByteNum()const{return 2;}
  void ConstructValue(char* msg)const;
};
/*****************************************************/
class Msg{
public:
  Msg(char* m){msg=m;}
  Msg& operator>>(const TransferObject& out);
  Msg& operator>>(char& out){out=Communicate::decode(msg[0]);msg++;return *this;}
  Msg& operator<<(const TransferObject& in);
  Msg& operator<<(const char& c){msg[0]=Communicate::code(c);msg++;return *this;}
  char& operator[](const int& i){return msg[i];}
private:
  char* msg;
};
/******************************************************/

#endif
