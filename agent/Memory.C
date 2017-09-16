/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Memory.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.2 $
 *
 *    $Id: Memory.C,v 2.2 2004/01/09 14:15:17 anton Exp $
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

/* Memory.C
 * CMUnited99 (soccer client for Robocup99)
 * Peter Stone <pstone@cs.cmu.edu>
 * Computer Science Department
 * Carnegie Mellon University
 * Copyright (C) 1999 Peter Stone
 *
 * CMUnited-99 was created by Peter Stone, Patrick Riley, and Manuela Veloso
 *
 * You may copy and distribute this program freely as long as you retain this notice.
 * If you make any changes or have any comments we would appreciate a message.
 * For more information, please see http://www.cs.cmu.edu/~robosoccer/
 */

#include "Memory.h"
#include "client.h"
#include <iostream>

void Memory::Initialize()
{
  PlayerInfo::Initialize();//it is in MemPlayer.C
  PositionInfo::Initialize();//this is in MemPositon.C
  ActionInfo::Initialize();//this is in MemAction.C
  Communicate::Initialize();//this is in Communicate.C
}
