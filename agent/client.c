/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : client.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.17 $
 *
 *    $Id: client.C,v 2.17 2004/08/29 14:07:21 anton Exp $
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


// ORIGINAL:
//	 client.C
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

#include "client.h"
#include "parse.h"
#include "behave.h"
#include "Playposition.h"

//-Global variables -- don't want to reallocate buffers each time---------------//

sigset_t  sigiomask, sigalrmask;

Memory    *Mem;

char      recvbuf[MAXMESG];	
char      sendbuf[MAXMESG];	

char      *GLOBAL_sense_body_message = static_cast<char*>("(sense_body)");

int       alrsigs_since_iosig=0;  

//------------------------------------------------------------------------------//

int main(int argc, char *argv[])
{
		
  Mem = new Memory();
  if ( Mem==0 )
    {
      my_error(" Couldnt allocate memory for Mem or buffer variable");
      exit(0);
    }

  Mem->GetOptions(argc,argv);
  
  Socket sock = init_connection(Mem->SP_host,Mem->SP_port);
  
  Mem->sock = &sock;
  
  if(Mem->sock->socketfd == -1) 
    {
      my_error("Can't open connection for player");
      exit(-1);
    }

  send_initialize_message();

  int res=2;
  while(res==2){	
    if ( wait_message(recvbuf, Mem->sock) == 0 ){
      my_error("Havn't got init message:don't know my side and my number");
      close_connection(Mem->sock);
      exit(-1);
    }
    res=parse_initialize_message(recvbuf);//return 1 if ok
    if (!res) {
      my_error("Could not parse initialize message");
      close_connection(Mem->sock);
      exit(-1);
    }
  }

  Mem->Initialize();
  //////////////////////
  if (Mem->MyNumber!=1) Mem->CP_goalie=FALSE;

  sigset_t sigfullmask = init_handler();	          
 
  while ( Mem->ServerAlive == TRUE && wait_for_signals(&sigfullmask) );

  if (Mem->sock->socketfd != -1) close_connection(Mem->sock);
  printf("Shutting down player %d\n",Mem->MyNumber);
  //  statistica::stat.SaveStatistic();

}

//------------------------------------------------------------------------------//

//------------------------------Send initialize message-------------------------//

void send_initialize_message()
{
  if (Mem->IP_reconnect)			//--recconect--
    sprintf(sendbuf, "(reconnect %s %d)", Mem->MyTeamName, Mem->IP_reconnect);
  else 							//--goalie connects--
    if ( Mem->CP_goalie == TRUE && Mem->SP_version >= 4.00)
      {
	sprintf(sendbuf, "(init %s (version %.2f) (goalie))", Mem->MyTeamName,Mem->SP_version);
      }
    else						//--player connects--  
      sprintf(sendbuf, "(init %s (version %.2f))", Mem->MyTeamName,Mem->SP_version);

  if(send_message(sendbuf, Mem->sock) == -1) 
    {
      my_error("Failed to send init command"); 
      abort(); 
    }
}

//------------------------------------------------------------------------------//

//--------------------------Parse initialize message----------------------------// 

int parse_initialize_message(char *recvbuf)//0 -error 1 -ok 2 -wait
{
  char mode[100];
  
  if ( !(strncmp(recvbuf,"(init",4)) )    // init msg arrived:
    {
      sscanf(recvbuf,"(init %c %d %[^)]",&Mem->MySide, &Mem->MyNumber, mode);
      Mem->ServerAlive = TRUE;
    }
  else 
    if ( !(strncmp(recvbuf,"(reconnect",4)) ) //recconect msg arrived:
      {
        sscanf(recvbuf,"(reconnect %c %[^)]",&Mem->MySide, mode);
        Mem->MyNumber = Mem->IP_reconnect;
        printf("reconnecting to %d on side %c!\n",Mem->MyNumber,Mem->MySide);
        Mem->ServerAlive = TRUE;
      }
    else 
      {
	my_error("Didn't get an init message: '%s'",recvbuf);
	Mem->ServerAlive=TRUE;
	if(Mem->PlayMode!=PM_Before_Kick_Off){
	  my_error("Game start but i still have no init massage!!!");
	  Mem->ServerAlive = FALSE;
	  return 0;
	}
	return 2;
      }

  if ( Mem->CP_goalie && Mem->FP_goalie_number != Mem->MyNumber )
    my_error("goalie number inconsistent with me being goalie");

  if ( !Mem->CP_goalie && Mem->FP_goalie_number == Mem->MyNumber )
    my_error("I should be the goalie");
  

  if ( mode[0] == 'b' ) 		// "before_kick_off"  mode:
    {  
      Mem->SetPlayMode(PM_Before_Kick_Off);
      if ( Mem->MySide == 'l' )
	Mem->KickOffMode = KO_Mine;
      else 
	Mem->KickOffMode = KO_Theirs;
    }
  else                  		// Act as if the game's in progress (perhaps needs changing?)
    Mem->SetPlayMode(PM_Play_On);
  return 1;
}

//------------------------------------------------------------------------------//

//-------set time interval between the sensor receiving and command sending-----// 

inline void set_timer()
{
  struct itimerval itv;
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = Mem->TimerInterval * 1000;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = Mem->TimerInterval * 1000;
  
  if( setitimer( ITIMER_REAL, &itv, NULL) != 0 ) 
    my_error("Set_timer: can't set timer"); 
}

inline void set_timer(int usec) 
{
  struct itimerval itv;
  itv.it_interval.tv_sec = 0;
  itv.it_interval.tv_usec = Mem->TimerInterval * 1000;
  itv.it_value.tv_sec = 0;
  itv.it_value.tv_usec = usec;
  
  if( setitimer( ITIMER_REAL, &itv, NULL) != 0 ) 
    my_error("Set_timer: can't set timer"); 
}

//------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//

sigset_t init_handler()     		
{ 
  sigemptyset(&sigalrmask);
  sigaddset(&sigalrmask, SIGALRM);
  sigemptyset(&sigiomask);
  sigaddset(&sigiomask, SIGIO);
  
  struct sigaction sigact;
  sigact.sa_flags = 0;
  sigact.sa_mask = sigiomask;

#ifdef Solaris
  sigact.sa_handler = (void (*)(int))sigalrm_handler; 
#else
  sigact.sa_handler = (void (*)(int))sigalrm_handler; 
#endif

  sigaction(SIGALRM, &sigact, NULL);
  sigact.sa_mask = sigalrmask;

#ifdef Solaris
  sigact.sa_handler = (void (*)(int))sigio_handler; 
#else
  sigact.sa_handler = (void (*)(int))sigio_handler; 
#endif

  sigaction(SIGIO, &sigact, NULL);

  set_timer();

  sigprocmask(SIG_UNBLOCK, &sigiomask, NULL);
  sigprocmask(SIG_UNBLOCK, &sigalrmask, NULL);

  sigset_t sigsetmask;
  sigprocmask(SIG_BLOCK, NULL, &sigsetmask);   // Get's the currently unblocked signals 
  return sigsetmask;   
}

//------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//
// suspend the process until one of the signals comes through 
// could check for situation to kill client, return FALSE     
// i.e. too many actions with no sensory input coming in      
//------------------------------------------------------------------------------//

Bool wait_for_signals(sigset_t *mask)
{
  sigsuspend(mask);
  return TRUE;
}

//------------------------------------------------------------------------------//

//---------SIGIO handler: receive and parse messages from server----------------// 

void sigio_handler() 
{
  sigprocmask(SIG_BLOCK, &sigalrmask, NULL);  


  Time StartTime = Mem->CurrentTime;
  
  
  while (receive_message(recvbuf, Mem->sock) == 1) 
    {
      Parse(recvbuf);
    }
	
  if ( Mem->CurrentTime - StartTime > 1 && StartTime.s == 0 && Mem->CurrentTime.s == 0 && StartTime.t != 0)
    {
      my_error("Received several steps at once -- missing action ops!!! (%d %d)",StartTime.t,StartTime.s);
    }

  sigprocmask(SIG_UNBLOCK, &sigalrmask, NULL);  
 
  alrsigs_since_iosig=0;

}

//------------------------------------------------------------------------------//

//-------SIGALRM handler: extract and send first command in commandlist---------// 

void sigalrm_handler()
{ 
  sigprocmask(SIG_BLOCK, &sigiomask, NULL);  						
  //  Mem->LogAction3(10," IN sigalrm_handler().Time :%d",Mem->CurrentTime.t);

  if ( Mem->LastInterruptTime != Mem->CurrentTime )
    { 

      if (!Mem->ClockStopped && Mem->CurrentTime-1 != Mem->LastInterruptTime && Mem->LastInterruptTime.t != 0)
	my_error("Missed a cycle??"); 
  
      if ( !Mem->ClockStopped && Mem->InterruptsThisCycle < Mem->CP_interrupts_per_cycle-1 && Mem->LastInterruptTime.t != 0) 
	my_error("Only %d interrupts last cycle",Mem->InterruptsThisCycle);				
   

      Mem->LastInterruptTime = Mem->CurrentTime; 
      Mem->InterruptsThisCycle = 0; 
    } 
  
  Mem->InterruptsThisCycle++; 



  //   Don't act until near the end of a cycle 
  //   there's some leeway in case there aren't enough interrupts in the cycle 

  if ( !Mem->ClockStopped && Mem->CP_interrupts_per_cycle - Mem->InterruptsThisCycle >		
       Mem->CP_interrupts_left_to_act) {
    //	  Mem->LogAction3(10,"To few interrupts this cycle: %d ",Mem->InterruptsThisCycle);
    return;
  }

  if (Mem->ClockStopped)
    Mem->StoppedClockMSec += Mem->TimerInterval;

  if (alrsigs_since_iosig++ > Mem->CP_interrupts_per_cycle * 50){    // was 20 !!!
    Mem->ServerAlive = FALSE;		
    return;
  }

  //   If a sight is definitely coming every cycle, don't act until getting the sight 
  //   Don't wait if we're in transition to a longer sight interval                   
 
  if ( Mem->MySightInterval() < Mem->SP_simulator_step && Mem->LastSightTime < Mem->CurrentTime &&
       !((Mem->ChangeView.valid() || Mem->ChangeView.valid(Mem->CurrentTime-1)) &&
	 (Mem->ChangeView.width > Mem->ViewWidth || Mem->ChangeView.qual > Mem->ViewQuality)) )   {
    Mem->LogAction4(200,"Waiting for sight... (%d %d)",
		    Mem->ChangeView.valid(),Mem->ChangeView.valid(Mem->CurrentTime-1));
    //	Mem->LogAction4(10,"My current time: %d,%d",Mem->CurrentTime.t,Mem->CurrentTime.s);
    //	Mem->LogAction3(10,"Number of interrupts %d",Mem->InterruptsThisCycle);	
    return;
  }

  if ( Mem->CurrentTime > Mem->LastActionOpTime )
    {
      if ( !Mem->ClockStopped && Mem->CurrentTime-1 != Mem->LastActionOpTime && Mem->LastActionOpTime != 0)
	my_error("Missed a cycle!!  (%d %d)",Mem->LastActionOpTime.t,Mem->LastActionOpTime.s);

      if ( Mem->NewSight ) Mem->FirstActionOpSinceLastSight = TRUE;
      //    Mem->LogAction2(10,"Doing decision....");
      //	Mem->LogAction4(10,"My current time: %d,%d",Mem->CurrentTime.t,Mem->CurrentTime.s);
      //	Mem->LogAction3(10,"Number of interrupts %d",Mem->InterruptsThisCycle);	

      Mem->update();		//it's in MemPosition.C

      behave();

      Mem->LastActionOpTime = Mem->CurrentTime;
      Mem->FirstActionOpSinceLastSight = FALSE;
    }


  //     Whether or not to wait between sending network packets is an interesting decsision.
  //     In versions of the server after 5.23, the server reads *all* commands off a socket at
  //     at every time step, so we could try to send all of our commands as soon as they are
  //     ready. However, on an ethernet, this can lead to lots of collisions and such, so it
  //     may degrade network performance
  //     To send everything without waiting, comment in this next line 

  //#define SEND_ALL_AT_ONCE
  
  //     the server now accepts multiple commands together  (after 5.23)
  //  if (0 && Mem->TooSoonForAnotherSend()) 
  //   {
  //     Mem->LogAction2(200, "It's too soon to send another command. Waiting");
  //   } else {
  
  
  if ( Mem->Action->valid() )    //if command not CMD_none - send it to server!
    {				
      send_action();
    }
  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */
  if ( Mem->ResendNeeded() )	// perhaps, need resending last action (if command(upper) not valid )
    {   
      resend_last_action();
    }
  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */

  if ( Mem->TurnNeck.valid() )  // as we can do it with other commands at the same cycles!
    {
      if ( Mem->TurnNeck.time < Mem->CurrentTime-1 ) my_error("old turn_neck");

      send_message( Mem->TurnNeck.command, Mem->sock );
      Mem->turn_necks++;
      Mem->TurnNeck.type = CMD_none; // so it's no longer valid 
    }

  //AI:update for communication
  if ( Mem->HaveSomethingToSay() ){
    send_message( Mem->SayBuffer(), Mem->sock );
  }

  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */
  if ( Mem->ChangeView.valid() )
    {
      if ( Mem->ChangeView.time < Mem->CurrentTime-1 ) my_error("old change_view");

      send_message( Mem->ChangeView.command, Mem->sock );
      Mem->ChangeView.type = CMD_none; 		//  so it's no longer valid 
    }
  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */
  if ( Mem->PointTo.valid() )
    {
      if ( Mem->PointTo.time < Mem->CurrentTime-1 ) my_error("old PointTo");

      send_message( Mem->PointTo.command, Mem->sock );
      Mem->Arm.count++;
      Mem->PointTo.type = CMD_none; 		//  so it's no longer valid
    }
  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */
  if ( Mem->AttentionTo.valid() )
    {
      if ( Mem->AttentionTo.time < Mem->CurrentTime-1 ) my_error("old AttentionTo");

      send_message( Mem->AttentionTo.command, Mem->sock );
      Mem->attentions++;
      Mem->AttentionTo.type = CMD_none; 		//  so it's no longer valid
    }
  /*
    #ifndef SEND_ALL_AT_ONCE
    else
    #endif	
  */
  if ( Mem->SP_sense_body_step > Mem->SP_simulator_step )  
    {
      //only if we won't get a sense_body each cycle by default 

      my_error("Sending sense_body");
      send_message(GLOBAL_sense_body_message,Mem->sock);  
    } 
  //}
  sigprocmask(SIG_UNBLOCK, &sigiomask, NULL);  

}

//------------------------------------------------------------------------------//

//------------insert turn/dash/kick commands in commandlist---------------------//

void turn(AngleDeg ang) 
{
  NormalizeAngleDeg(&ang); 

  // turn so that the actual turn is the desired turn 
  // pos.rotate( ang/(1.0 + GetMyInertiaMoment() * MySpeed()) ); 

  if ( Mem->MyVelConf() ) ang *= (1 + Mem->GetMyInertiaMoment() * Mem->MySpeed());

  if ( ang > Mem->SP_max_moment ) ang = Mem->SP_max_moment;
  if ( ang < Mem->SP_min_moment ) ang = Mem->SP_min_moment;

  if (ang < .1 && ang > -.1) 
    {
      Mem->Action->type = CMD_none;
      return;          // No turn           
    }

  Mem->Action->type = CMD_turn;
  Mem->Action->power = 0;
  Mem->Action->angle = ang;
  Mem->Action->time = Mem->CurrentTime;

  if (Mem->TurnNeck.time == Mem->CurrentTime)       // Already did a turn_neck    
    { 
      // adjust as much as possible for the turn 

      Mem->TurnNeck.angle -= ang;
      if ( Mem->MyNeckRelAng() + Mem->TurnNeck.angle < Mem->SP_min_neck_angle )
	Mem->TurnNeck.angle = Mem->SP_min_neck_angle - Mem->MyNeckRelAng();
      if ( Mem->MyNeckRelAng() + Mem->TurnNeck.angle > Mem->SP_max_neck_angle )
	Mem->TurnNeck.angle = Mem->SP_max_neck_angle - Mem->MyNeckRelAng();
    }

  sprintf(Mem->Action->command,"(turn %.2f)", ang);
  Mem->LogAction3(150, "turn %f", ang);
}  

//------------------------------------------------------------------------------//

//-------------------------------dash(float power)------------------------------//

void dash(float power) 
{
  if ( Mem->PlayMode == PM_Before_Kick_Off ) return;

  if (power > Mem->SP_max_power) my_error("Can't dash that fast: %.1f",power); 
  if (power < Mem->SP_min_power) my_error("Can't dash that 'slow': %.1f",power);

  // Factor for stamina--don't dash more than stamina or more than necessary to get you to max speed 

  Mem->VerifyDash(&power);

  if (fabs(power) < 1)
    {
      Mem->Action->type = CMD_none;
      return;                         // No dash           
    }
  
  Mem->Action->type = CMD_dash;
  Mem->Action->power = power;
  Mem->Action->angle = 0;
  Mem->Action->time = Mem->CurrentTime;

  sprintf(Mem->Action->command, "(dash %.2f)", power);
  Mem->LogAction3(150, "dash %f", power);
}

//------------------------------------------------------------------------------//
//////////////////////////////////////////////////////////////////////
void GetBallPosVelInNextCycle(Vector& pos,Vector& vel)
{
  if(Mem->Action->valid()&&Mem->Action->type==CMD_kick&&Mem->OpponentWithBall()==Unum_Unknown){
    vel=Mem->BallAbsoluteVelocity()+Polar2Vector( Mem->BallKickRate()*Mem->Action->power, Mem->MyBodyAng()+Mem->Action->angle );
    if(vel.mod()>Mem->SP_ball_speed_max)
      vel*=Mem->SP_ball_speed_max/vel.mod();
    pos=Mem->BallAbsolutePosition()+vel;
  }else{
    vel=Polar2Vector(2.0f,0.0f);//hack
    pos=Mem->BallAbsolutePosition();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void SayThenLeaveWithBall(){
  if(!Mem->BallPositionValid()||!Mem->BallVelocityValid()) return;

  if(Mem->Action->valid()&&Mem->Action->type==CMD_kick&&Mem->OpponentWithBall()==Unum_Unknown){
    Vector vel,pos;
    GetBallPosVelInNextCycle(pos,vel);
    vel*=Mem->SP_ball_decay;
    Vector myPos=Mem->MyPos()+Mem->MyVel();
    if((myPos-pos).mod()>Mem->GetMyKickableArea()){
      char msg[10];
      Msg m(msg);
      m<<char(ST_ball_and_conf_ballvel_and_conf);
      Mem->LogAction6(50,"After kick ball will leave me, so say about ball. Pos (%.2f,%.2f) vel (%.2f,%.2f)",pos.x,pos.y,vel.x,vel.y);
      m<<TransferXCoor(pos.x)<<TransferYCoor(pos.y)<<TransferConf(1.0f/Mem->CP_ball_conf_decay);//last is hack
      m<<TransferVel(vel.x)<<TransferVel(vel.y)<<TransferConf(1.0f/Mem->CP_ball_conf_decay);
      Mem->SayNow(msg);
    }
  }
}
//------------kick(float power, AngleDeg dir)-----------------------------------//

void kick(float power, AngleDeg dir) 
{
  if ( !(Mem->BallKickable()) ) my_error("Can't kick a ball that's too far away");

  if ( Mem->PlayMode == PM_Before_Kick_Off ) return;

  if (power > Mem->SP_max_power) my_error("Can't kick that hard");
  if (power < 0 ) my_error("Can't kick < 0");
  NormalizeAngleDeg(&dir);

  Mem->Action->type = CMD_kick;
  Mem->Action->power = power;
  Mem->Action->angle = dir;
  Mem->Action->time = Mem->CurrentTime;

  SayThenLeaveWithBall();//add by AI

  sprintf(Mem->Action->command, "(kick %.2f %.2f)", power, dir);
  Mem->LogAction4(150, "kick %f %f", power, dir);
}

//------------------------------------------------------------------------------//

//-------------goalie_catch(AngleDeg dir)---------------------------------------//

void goalie_catch(AngleDeg dir) 
{
  if ( !(Mem->BallCatchable()) ) my_error("Can't catch a ball that's too far away");
  if ( !Mem->CP_goalie ) my_error("Only goalies can catch");

  if ( Mem->PlayMode == PM_Before_Kick_Off ) return;

  NormalizeAngleDeg(&dir);

  Mem->Action->type = CMD_catch;
  Mem->Action->power = 0;
  Mem->Action->angle = dir;
  Mem->Action->time = Mem->CurrentTime;

  sprintf(Mem->Action->command, "(catch %.2f)", dir);
  Mem->LogAction3(150, "catch %f",  dir);
}

//------------------------------------------------------------------------------//

//-----------------move(float x, float y)---------------------------------------//

void move(float x, float y) 
{
  if ( ! (Mem->PlayMode == PM_Before_Kick_Off ||
	  (Mem->CP_goalie && Mem->PlayMode == PM_My_Goalie_Free_Kick)) )
    my_error("Can only move in before kickoff mode (or after goalie catch)");

  // Perhaps here convert to a position on the field 

  if ( fabs(y) > Mem->SP_pitch_width/2 || x > 0 || x < -Mem->SP_pitch_length/2 )
    my_error("Must move to a place on the pitch");

  if ( Mem->PlayMode == PM_My_Goalie_Free_Kick && !Mem->OwnPenaltyArea.IsWithin(Vector(x,y)) )
    my_error("Must move to a place within penalty area");

  Mem->Action->type = CMD_move;
  Mem->Action->x = x;
  Mem->Action->y = y;
  Mem->Action->time =  Mem->CurrentTime;
  Mem->last_move_pos=Vector(x,y);
  
  sprintf(Mem->Action->command, "(move %.8f %.8f)", x, y);//change by AI: %.2 -> %.6 ONLY FOR TEST
  Mem->LogAction4(150, "move %f %f", x, y);
}

//------------------------------------------------------------------------------//

//---------------disconnect()---------------------------------------------------//

void disconnect()
{
  Mem->Action->type = CMD_bye;
  Mem->Action->time =  Mem->CurrentTime;

  sprintf(Mem->Action->command, "(bye)");
}

//------------------------------------------------------------------------------//

//--------------------------- tackle (float power) ---------------//

void tackle (float power)
{
  if(Mem->Tackling()) my_error ("can't tackle: Already tackling");
  Mem->Action->type=CMD_tackle;
  Mem->Action->power=power;
  Mem->Action->time = Mem->CurrentTime;
  sprintf(Mem->Action->command, "(tackle %.2f)",power);
  Mem->LogAction3(150, "tackle %.2f",power);
}
//------------------turn_neck(AngleDeg ang)-------------------------------------//

void turn_neck(AngleDeg ang) 
{
  NormalizeAngleDeg(&ang); 

  if (ang == 0) 
    {
      Mem->LogAction2(150, "Ignoring turn_neck 0");
      return;
    }  

  if ( ang > Mem->SP_max_neck_moment ) ang = Mem->SP_max_neck_moment;
  if ( ang < Mem->SP_min_neck_moment ) ang = Mem->SP_min_neck_moment;

  if ( Mem->MyNeckRelAng() + ang > Mem->SP_max_neck_angle ) 
    {
      ang = Mem->SP_max_neck_angle - Mem->MyNeckRelAng();
      my_error("Can't turn neck that much:correcting...");
    }
  
  if ( Mem->MyNeckRelAng() + ang < Mem->SP_min_neck_angle ) 
    {
      ang = Mem->SP_min_neck_angle - Mem->MyNeckRelAng();
      my_error("Can't turn neck that little:correcting");
    }
  
  Mem->TurnNeck.type = CMD_turn_neck;
  Mem->TurnNeck.power = 0;
  Mem->TurnNeck.angle = ang;
  Mem->TurnNeck.time = Mem->CurrentTime;  

  sprintf(Mem->TurnNeck.command,"(turn_neck %.2f)", ang);
  Mem->LogAction3(150, "turn_neck %f", ang);
}

//------------------------------------------------------------------------------//

//-------------change_view(Vqual qual, Vwidth width)----------------------------//

void change_view(Vqual qual, Vwidth width) 			
{
  if ( qual==Mem->ViewQuality  && width==Mem->ViewWidth )
    return;  // my_error("Nothing to change about view"); 

  Mem->ChangeView.type  = CMD_change_view;				//Comamnd ChangeView
  Mem->ChangeView.qual  = qual;
  Mem->ChangeView.width = width;
  Mem->ChangeView.time  = Mem->CurrentTime;

  char qual_string[10], width_string[10];
  //-----------
  switch (qual)
    {
    case VQ_High:
      sprintf(qual_string,"high"); 
      break;
    case VQ_Low:
      sprintf(qual_string,"low"); 
      break;
    }
  //-----------
  switch (width)
    {
    case VW_Narrow: 
      sprintf(width_string,"narrow");
      break;
    case VW_Normal:
      sprintf(width_string,"normal");
      break;
    case VW_Wide: 
      sprintf(width_string,"wide"); 
      break;
    default:
      my_error("change_view:wrong  width %d",width);
      return;
    }
  //-----------
  sprintf(Mem->ChangeView.command, "(change_view %s %s)", width_string, qual_string);
  Mem->LogAction4(150, "change_view %s %s",  width_string, qual_string);
}

//------------------------------------------------------------------------------//

//--------------------------- pointto (float dist, AngleDeg dir) ---------------//

void pointto (float dist, AngleDeg dir)
{
  if (Mem->Arm.movable>0) my_error(" can't do pointto: Arm is unmovable");
  if (dist==0)		//off
    {
      if (Mem->Arm.expires<=0) my_error ("can't do pointto off: Not pointing");
      Mem->PointTo.type = CMD_pointto;
      Mem->PointTo.x = 0;
      Mem->PointTo.angle = 0;
      Mem->PointTo.time = Mem->CurrentTime;
      sprintf(Mem->PointTo.command, "(pointto off)");
      Mem->LogAction2(150, "pointto off");
      return;
    }
  NormalizeAngleDeg(&dir);
  Mem->PointTo.type=CMD_pointto;
  Mem->PointTo.x=dist;
  Mem->PointTo.angle = dir;
  Mem->PointTo.time = Mem->CurrentTime;
  sprintf(Mem->PointTo.command, "(pointto %.2f %.2f)",dist, dir);
  Mem->LogAction4(150, "pointto %.2f %.2f)",dist, dir);
}
//------------------------------------------------------------------------------//

//--------------------------- attentionto (Unum player) ---------------//
void attentionto(Unum player)
{
  if (player==Unum_Unknown)			//off
    {
      Mem->AttentionTo.type=CMD_attentionto;
      Mem->AttentionTo.player=Unum_Unknown;
      Mem->AttentionTo.time=Mem->CurrentTime;
      sprintf(Mem->AttentionTo.command, "(attentionto off)");
      Mem->LogAction2(150, "attentionto off");
      Mem->attention_to_player=Unum_Unknown;
      return;
    }
  Mem->AttentionTo.type=CMD_attentionto;
  Mem->AttentionTo.player=player;
  Mem->AttentionTo.time=Mem->CurrentTime;
  sprintf(Mem->AttentionTo.command, "(attentionto %c %d)",player>0?Mem->MySide:Mem->TheirSide,abs(player));
  Mem->LogAction3(150, "Attention to teammate %d",abs(player));
  Mem->attention_to_player=player;
}


//------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//

void send_action()  									// send action to a server
{	
  if ( !(Mem->Action->valid(Mem->CurrentTime)) )     	//Action(class Command)(not in current cycle)
    my_error("Old action %d %d",Mem->Action->time.t,Mem->Action->time.s);

  send_message(Mem->Action->command, Mem->sock);  		//char command[MAXMESG]

  switch (Mem->Action->type)
    {							// almost clear!
    case CMD_kick:										//set_past_kick() - for what?
      Mem->kicks++;
      Mem->GetBall()->set_past_kick( Mem->LastActionPower(),
				     Mem->LastActionAngle(),
				     Mem->CurrentTime           );
      break;
    case CMD_dash: Mem->dashes++; break;
    case CMD_turn: Mem->turns++;  break;
    case CMD_tackle: Mem->tackles++; break;
    default: ;
    }

  Command *tmp = Mem->LastAction;						// needs comments:not clear!
  Mem->LastAction = Mem->Action;
  Mem->Action     = tmp;

  Mem->Action->type = CMD_none; // So it's invalid 
  Mem->NewAction = TRUE;
}

//------------------------------------------------------------------------------//

//------------------------------------------------------------------------------//

void resend_last_action()								//means reapiting?
{
  if ( Mem->LastActionType() == Mem->ResendType )
    {
      my_stamp; printf("resending\n");
      send_message(Mem->LastAction->command, Mem->sock);

      switch (Mem->LastActionType())
	{
	case CMD_kick: Mem->kicks++;  break;
	case CMD_dash: Mem->dashes++; break;
	case CMD_turn: Mem->turns++;  break;
	case CMD_tackle: Mem->tackles++; break;
	default: ;
	}
    }
  else 
    my_error("last action isn't a %d",Mem->ResendType);

  Mem->RequestResend = FALSE;
}
//////////////////////////////////////////////////////////////////////
void send_support_clang(int minver,int maxver)
{
  ostringstream str;
  str<<"(clang (ver "<<minver<<" "<<maxver<<"))";
  Mem->LogAction4(100,"Send support of clang: minver=%.0f; maxver=%.0f",float(minver),float(maxver));
  send_message((char*)str.str().c_str(), Mem->sock);
}
