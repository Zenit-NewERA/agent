/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : netif.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.3 $
 *
 *    $Id: netif.h,v 2.3 2004/03/05 07:03:44 anton Exp $
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

/* netif.h
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

/*
 * Definitions for UDP c/s program.
 */
#ifndef __netif_h
#define __netif_h
// this i try by myself!
#include <cstdlib>
#include <cstdio>  //
#include <cstring> //
#include <unistd.h> //
#include <sys/types.h> //
#include <sys/socket.h> //
#include <sys/file.h>
#include <netinet/in.h> //
#include <arpa/inet.h>
#include <netdb.h> //
#include <fcntl.h> //
#include <errno.h> //
#include <signal.h>
#include <sys/wait.h>

#define DEFAULT_PORT_NUMBER	6000
#define MAXMESG			4096	/// (Alexy) was 2048 - enlarged to fit server_param message
#define MAXARG			200

#ifdef MEMSET
#define bzero(a, b) memset(a, 0, b)
#endif

#ifndef Linux
//extern "C" void usleep(unsigned long);
#endif

#ifdef __cplusplus
class Socket {
public:
  int			socketfd  ;
  struct sockaddr_in	serv_addr ;
};
#else
typedef struct socket{
  int			socketfd  ;
  struct sockaddr_in	serv_addr ;
}Socket ;
#endif

extern Socket init_connection(char* host, int port) ;
extern int    send_message(char* buf, Socket *sock) ;
extern int    receive_message(char *buf, Socket *sock) ;
extern void   close_connection(Socket *sock) ;

int wait_message(char *buf, Socket *sock);

#endif
