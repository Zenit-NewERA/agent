/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : parse.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.4 $
 *
 *    $Id: parse.h,v 2.4 2004/04/19 08:00:01 anton Exp $
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

// ORIGINAL:
//	 parse.h
//	 CMUnited99 (soccer client for Robocup99)
// 	 Peter Stone <pstone@cs.cmu.edu>
// 	 Computer Science Department
// 	 Carnegie Mellon University
//	 Copyright (C) 1999 Peter Stone
// 
// 	 CMUnited-99 was created by Peter Stone, Patrick Riley, and Manuela Veloso
// 
// 	 You may copy and distribute this program freely as long as you retain this notice.
// 	 If you make any changes or have any comments we would appreciate a message.
// 	 For more information, please see http://www.cs.cmu.edu/~robosoccer/
 
#ifndef _PARSE_H_
#define _PARSE_H_

//------------------------------------------------------------------------//

void Parse (char *SensoryInfo);

// void  Parse_Server_Param_message(char *ServerInfo);
// void  Parse_Player_Param_message(char *PlayerInfo);
// void  Parse_Player_Type_message(char *PlayerType);

void  Parse_Sight(Time time, char *SightInfo);
void  Parse_Sense(Time time, char *SenseInfo);
void  Parse_Sound(Time time, char *SoundInfo); 

void  Parse_Referee_Sound(char *RefereeSound);
void  Parse_Trainer_Sound(char *msg);
void  Parse_My_Coach_Sound(Time time, char *msg);
void  Parse_Their_Coach_Sound(Time time, char *msg);
void Parse_Change_Player_Type(char* msg);
void Parse_Ok_Msg(char* msg);

//------------------------------------------------------------------------//

#endif
