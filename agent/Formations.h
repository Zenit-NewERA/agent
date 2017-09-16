/*************************************************************************
 *
 *    DESCRIPTION : Contain information  about team formations
 *
 *    FILE 	 : Formations.h
 *
 *    AUTHOR     : Anton Ivanov
 *
 *    $Revision: 2.11 $
 *
 *    $Id: Formations.h,v 2.11 2004/06/22 17:06:16 anton Exp $
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
#ifndef FORMATIONS_H
#define FORMATIONS_H

#include "geometry.h"
#include "client.h"
#include <fstream>
#include <string>

enum Ftype{
  FT_None=-1,
  FT_Default=1,
  FT_433=0,
  FT_433_OFFENSE=1,
  FT_523_DEFENSE
};

struct FormationLine{
  float from_x;
  float base;
  float attr;

  FormationLine(){}
  FormationLine(float f,float b,float a):from_x(f),base(b),attr(a){}
  bool LoadData(istream& file); 
};
  
struct FormationData{
  FormationLine* line_x[11];
  FormationLine* line_y[11];
  int num_x_lines;
  int num_y_lines;
  
  float max_x[11];
  float min_x[11];
  Ptype types[11];
  Pside sides[11];
  Ftype formation;

  int activity_digraph[10][10];
  
  FormationData();
  virtual ~FormationData();
  
  bool LoadData(ifstream& file,Ftype formation,int num_fracture_x,int num_fracture_y); 
  
private:
  Ptype conv_char_to_ptype(char c) const;
  Pside conv_char_to_pside(char c) const;

};

class Formations {
public:
  Formations();
  virtual ~Formations();
	
  static void LoadConfigFile();
  static Ftype GetFormationByString(string str);

  typedef int Iterator;

  string& GetPlayerName(){ return PlayerName;}
  void MoveToInitialPos() const {move(GetInitialPos().x,GetInitialPos().y);}
  Vector GetInitialPos()const{
    return Vector(begin_pos[num_of_begin_pos-1][Mem->MyNumber-1][0],
		  begin_pos[num_of_begin_pos-1][Mem->MyNumber-1][1]);
  }

  Vector GetHomePosition(Unum player=Mem->MyNumber);
  Vector GetHomePosition(const Vector& ball_pos,Unum player=Mem->MyNumber);

  virtual int IsActivityByFastest(Unum tm,Unum fastest)const;

  Ptype GetMyType() const { return GetPlayerType();}
  Pside GetMySide()const { return GetPlayerSide();}
  Ptype GetPlayerType(Unum player=Mem->MyNumber) const { return formations[current_formation].types[int(player)-1];}
  Pside GetPlayerSide(Unum player=Mem->MyNumber) const { return formations[current_formation].sides[int(player)-1];}
  virtual float GetFormationDash(Unum player=Mem->MyNumber)const;

  Unum GetPlayerNumber(Iterator& iter,int PT_mask=PT_All,int PS_mask=PS_All) const;//PT_All - all roles; PS_All - all wings
  Iterator begin() const{ return 0;}
  Iterator end() const{ return 11;}

  void SetCurrentFormation(Ftype fm);
  int GetFormation()const{return current_formation;}

  bool IsOpponentGoalieGoOutOfPenaltyArea();

  static const int num_of_formations=3;

  typedef float bp[11][2];
protected:
  Vector GetMyGoalieFreeKickHomePos(Unum player=Mem->MyNumber);
  Vector GetMyKickInHomePos(Unum player=Mem->MyNumber);
  Vector GetCornerKickHomePos(Unum player=Mem->MyNumber);
  Vector GetMyGoalieKickHomePos(Unum player=Mem->MyNumber);
  Vector GetMyFreeKickHomePos(Unum player=Mem->MyNumber);

  Vector GetTheirSetPlayHomePos(Unum player=Mem->MyNumber);
  Vector GetTheirGoalieFreeKickHomePos(Unum player=Mem->MyNumber);
  Vector GetTheirCornerKickHomePos(Unum player=Mem->MyNumber);
  Vector GetTheirKickInHomePos(Unum player=Mem->MyNumber);

  int IsStrongOpponent();
  bool IsOpponentUseTackle();
  bool IsOpponentActiveUseDribble();

  bool important_action;
private:
  void InitBDPMatrix();
  Vector GetBDP(const Vector& ball);
  Vector _GetHomePosition(int index,bool fixed_ball_pos=false, const Vector& ball_pos=Vector(0,0));

  static bp *begin_pos;
  static int num_of_begin_pos;

  static const int NUM_HEIGHT_BLOCKS=3;
  static const int NUM_WIDTH_BLOCKS=4;
  static string PlayerName;//name of player
  int current_formation;	// разный для Pos и SetPlay и потому в стандартах всегда используем 433
  static FormationData formations[num_of_formations];
  Vector BDPMatrix[NUM_HEIGHT_BLOCKS][NUM_WIDTH_BLOCKS];
  Vector HomePos[11];
  Time   updateHomePos[11];
  
  struct Formation_Str{
    string name;
    Ftype type;
  };
  static Formation_Str form_str[num_of_formations];
};

#endif
