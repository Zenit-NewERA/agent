/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : Memory.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: Memory.h,v 2.3 2004/03/05 07:03:44 anton Exp $
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
/* -*- Mode: C++ -*- */

/* Memory.h
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


#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "Communicate.h"

#include <fstream>
class Memory : public Communicate{
public:
  void Initialize(); // depends on the size of the teams
};



#endif
