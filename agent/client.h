/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : client.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: client.h,v 2.7 2004/06/22 17:06:16 anton Exp $
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
//	 client.h
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


#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "Memory.h"

//------------------------------------------------------------------------------//

extern Memory *Mem;


void      send_initialize_message();
int      parse_initialize_message(char *);
void      parse_server_param_message(char *);	
	
Bool      wait_for_signals(sigset_t *);
sigset_t  init_handler();
void      sigio_handler(); 
void      sigalrm_handler();
void      send_action();
void      resend_last_action();


void 	  turn(AngleDeg ang);
void      dash(float power);
void      kick(float power, AngleDeg dir);
void      goalie_catch(AngleDeg dir);
void      move(float x, float y);
inline void      move(Vector p) { move(p.x,p.y); }
void      disconnect();

void      turn_neck(AngleDeg ang);
void      change_view(Vqual qual, Vwidth width);
inline void      change_view(Vqual qual)   { change_view(qual,Mem->ViewWidth);    }
inline void      change_view(Vwidth width) { change_view(Mem->ViewQuality,width); }

void pointto (float dist=0, AngleDeg dir=0);
void tackle (float power);

void attentionto(Unum player=Unum_Unknown);
void send_support_clang(int minver=Mem->CP_min_clang_ver,int maxver=Mem->CP_max_clang_ver);
void GetBallPosVelInNextCycle(Vector& pos,Vector& vel);
//------------------------------------------------------------------------------//

#endif
