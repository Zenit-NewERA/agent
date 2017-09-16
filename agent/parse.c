/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : parse.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergey Serebyakov
 *
 *    $Revision: 2.10 $
 *
 *    $Id: parse.C,v 2.10 2004/05/10 14:18:15 anton Exp $
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
//	 parse.C
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
#include "utils.h"
#include "parse.h"
#include "SetPlay.h"
#include "Playposition.h"
#include <string>

//#define IGNORE_WARNINGS

//------------------------------------------------------------------------//

void Parse(char *SensoryInfo)
{
  const bool TEST = FALSE;
 
  SenseType sense_type;
  int     time;
  Time tm;

  static bool FIRST_TIME = TRUE;

  if( !strncmp(SensoryInfo,"(see",4) ) 			sense_type = See_Msg;	 else   
    if( !strncmp(SensoryInfo,"(sense_body",4) ) 	sense_type = Sense_Msg;  else
      if( !strncmp(SensoryInfo,"(hear",4) ) 		sense_type = Hear_Msg;   else
	if( !strncmp(SensoryInfo,"(server_param",4) ) sense_type = SP_Msg;	 else
	  if( !strncmp(SensoryInfo,"(player_param",11) )sense_type = PP_Msg;     else
	    if( !strncmp(SensoryInfo,"(player_type",11) ) sense_type = PT_Msg;     else
	      if( !strncmp(SensoryInfo,"(change_player_type",17) ) sense_type = PT_Change; else
		if( !strncmp(SensoryInfo,"(score",6) ) 		sense_type = Score_Msg; else
		  if( !strncmp(SensoryInfo,"(warning",6) ) 		sense_type = Warning_Msg; else
		    if( !strncmp(SensoryInfo,"(error",5) ) 		sense_type = Error_Msg; else
		      if(!strncmp(SensoryInfo,"(ok",3)) sense_type=Ok_Msg; else

		      my_error ("got unknown message-don't know what to do? %s ",SensoryInfo);


  //time - server's current cycle

  if(sense_type != SP_Msg && sense_type != PP_Msg &&
     sense_type != PT_Msg&&sense_type!=PT_Change &&
     sense_type !=Warning_Msg&& sense_type !=Error_Msg && sense_type!=Ok_Msg)
    {

      time = get_int(&SensoryInfo);     // the time,when appropriate message came.         

      //this needs for accurate going into play (else agent gives error message about time)
      if(FIRST_TIME) {
	Mem->LogAction2(200,"Parse: FIRST_TIME");
	Mem->CurrentTime.t=time;FIRST_TIME = FALSE;}
      else
	tm = Mem->update_time(time); 
 
    }

  // tm - new Current Time

  switch ( sense_type )
    {
    case See_Msg:
      if ( !Mem->LastActionOpTime )     break;   // Don't parse until I've started counting time   
      if ( tm == Mem->LastSightTime )   break; // Don't parse a second sight from the same cycle 
      if ( Mem->NewSight == TRUE )
        {
	  Mem->ClearSeenInfo();
	  Mem->LogAction2(190,"Sight from last cycle lying around -- clearing it");
        }
      Parse_Sight(tm, SensoryInfo); 
      Mem->LastSightInterval = tm - Mem->LastSightTime;
      Mem->LastSightTime = tm;
      Mem->NewSight = TRUE;
      break;
    case Sense_Msg: 
      Parse_Sense(tm, SensoryInfo);      
      Mem->LastSenseTime = tm;
      break;
    case Hear_Msg:  
      if ( !Mem->LastActionOpTime ) break; // Don't parse until I've started counting time 
      Parse_Sound(tm, SensoryInfo); 
      Mem->LastSoundTime = tm;  
      break;
    case Score_Msg:
      if( TEST )cout<<"Score msg( (score Time ...) )"<<endl;   
      sscanf(SensoryInfo,"(score %i %i %i)",&time,&Mem->IP_my_score,&Mem->IP_their_score);
      break; 
    case SP_Msg:
      Mem->Parse_Server_Param_message(SensoryInfo);
      break;
    case PP_Msg:
      Mem->Parse_Player_Param_message(SensoryInfo);
      break;
    case PT_Msg:
      Mem->Parse_Player_Type_message(SensoryInfo);
      break;
    case PT_Change:
      Parse_Change_Player_Type(SensoryInfo);
      break;
    case Ok_Msg:
      Parse_Ok_Msg(SensoryInfo);
      break;
    case Warning_Msg:
#ifndef IGNORE_WARNINGS
      my_error ("Got warning message: %s",SensoryInfo);
#endif
    case Error_Msg:
      my_error ("Got error message: %s",SensoryInfo);
      break;
    }
  
  if(sense_type != SP_Msg && sense_type != PP_Msg && sense_type != PT_Msg && sense_type !=Warning_Msg && sense_type !=Error_Msg)
    Mem->LastSenseType = sense_type;

}

//------------------------------------------------------------------------//
//------------------------------------------------------------------------//

void Parse_Sense(Time time, char *SenseInfo)
{
  // set this to FALSE if you are running not in a test mode
  const bool TEST = FALSE;
  //  const bool TEST = TRUE;

  if( TEST ) {cout<<"Message"<<endl<<SenseInfo<<endl;}
  if( TEST ) cout<<"-------------"<<endl;

  get_word(&SenseInfo);  
  SenseInfo += 10;       // "view_mode " 

  switch ( SenseInfo[0] )
    {
    case 'h': Mem->ViewQuality = VQ_High; 
      if( TEST ) {cout<<"ViewQuality: "<<"High"<<endl;}	             
      break;  
    case 'l': Mem->ViewQuality = VQ_Low;  
      if( TEST ) {cout<<"ViewQuality: "<<"Low"<<endl;}
      break; 
    default:  my_error("Unknown view quality");
    }

  Mem->LastViewWidth = Mem->ViewWidth;
  Mem->ViewWidthTime = time;

  get_next_word(&SenseInfo);  

  switch ( SenseInfo[1] )
    {
    case 'o': Mem->ViewWidth = VW_Normal; 
      if( TEST ) {cout<<"ViewWidth: "<<"Normal"<<endl;}
      break;  
    case 'a': Mem->ViewWidth = VW_Narrow; 
      if( TEST ) {cout<<"ViewWidth: "<<"Narrow"<<endl;}
      break;  
    case 'i': Mem->ViewWidth = VW_Wide;   
      if( TEST ) {cout<<"ViewWidth: "<<"Wide"<<endl;}
      break;  
    default:  my_error("Unknown view quality");
    }
   
  float stamina = get_float(&SenseInfo);
  if( TEST ) {cout<<"stamina: "<<stamina<<endl;}

  float effort  = get_float(&SenseInfo);
  if( TEST ) {cout<<"effort: "<<effort<<endl;}

  float speed   = get_float(&SenseInfo);
  if( TEST ) {cout<<"speed: "<<speed<<endl;}

  float direction_of_speed = get_float(&SenseInfo);		//this new var
  if( TEST ) {cout<<"direction_of_speed: "<<direction_of_speed<<endl;}

  float head_angle = get_float(&SenseInfo);
  if( TEST ) {cout<<"head_angle: "<<head_angle<<endl;}

  int kicks  =   get_int(&SenseInfo);
  if( TEST ) {cout<<"kicks: "<<kicks<<endl;}

  int dashes =   get_int(&SenseInfo);
  if( TEST ) {cout<<"dashes: "<<dashes<<endl;}

  int turns  =   get_int(&SenseInfo);
  if( TEST ) {cout<<"turns: "<<turns<<endl;}

  int says   =   get_int(&SenseInfo);
  if( TEST ) {cout<<"says: "<<says<<endl;}

  int turn_necks   =   get_int(&SenseInfo);
  if( TEST ) {cout<<"turn_necks: "<<turn_necks<<endl;}


  int catchs = get_int(&SenseInfo);					//this new var
  if( TEST ) {cout<<"catchs: "<<catchs<<endl;}

  int moves = get_int(&SenseInfo);					//this new var
  if( TEST ) {cout<<"moves: "<<moves<<endl;}

  int change_views = get_int(&SenseInfo);			//this new var
  if( TEST ) {cout<<"change_views: "<<change_views<<endl;}

  ArmInfo arm;
  // if ( TEST ){cout <<"---before arm:"<<SenseInfo<<endl;}
  arm.movable=get_int(&SenseInfo);
  arm.expires=get_int(&SenseInfo);
  arm.dist=get_int(&SenseInfo);
  arm.dir=get_int(&SenseInfo);
  arm.count=get_int(&SenseInfo);
  if ( TEST ){cout <<"Arm: movable "<<arm.movable<<" expires "<< arm.expires<<" dist "<<
		arm.dist<<" dir "<<arm.dir<<" count "<<arm.count<<endl;}
  //  Mem->LogAction7(200,"Sensed Arm: movable %d expires %d dist %d dir %d count %d",arm.movable, arm.expires,arm.dist,arm.dir,arm.count);
  //  if ( TEST ){cout <<"---after arm:"<<SenseInfo<<endl;}

  advance_to('(',&SenseInfo); //to "(focus"
  if ( TEST ){cout <<"---advance 1:"<<SenseInfo<<endl;}
  advance_to('(',&(++SenseInfo));    //to "(target"
  if ( TEST ){cout <<"---advance 2:"<<SenseInfo<<endl;}
  advance_to(' ',&(SenseInfo));      //to space after target

  //  get_next_word(&SenseInfo);//focus
  //  get_next_word(&SenseInfo);//target
  SenseInfo++; //space
  Unum focus;
  char side;
  char buf[50];
  buf[26]=0;
  if ( TEST ){cout <<"---before attention to:"<<SenseInfo<<endl;}
  if (SenseInfo[0]=='n')
    {
      focus=Unum_Unknown;
      get_next_word(&SenseInfo);//"none"
      side='n';
      if ( TEST ){cout <<"---attention to none "<<SenseInfo<<endl;}
    }
  else
    {
      side=SenseInfo[0];
      SenseInfo++;
      focus=get_int(&SenseInfo);
      if (side!=Mem->MySide) focus=-focus;
    }
  int attentions=get_int(&SenseInfo);
  if (TEST) {cout <<"Attention to "<<focus<<" attentions "<<attentions<<endl;}
  if ( TEST ){cout <<"---before tackle:"<<SenseInfo<<endl;}
  int tackle_exp=get_int(&SenseInfo);
  int tackles=get_int(&SenseInfo);
  if (TEST) {cout <<"Tackle expires "<<tackle_exp<<" tackles "<<tackles<<endl;}

  Mem->SetMySensedInfo (stamina,effort,speed,head_angle,kicks,dashes,turns,says,turn_necks,
			arm,focus,attentions,tackle_exp,tackles,moves,time);
}
//------------------------------------------------------------------------//

//------------------------------------------------------------------------//

void Parse_Sight(Time time, char *SightInfo)
{
  //  const bool TEST = TRUE;  

  /* if( TEST ) 
     {
     cout<<SightInfo<<endl;
     return;
     }
  */
  const int NOCHNGINFO = -500;
  const int NOFACEINFO = -500;

  float dist, ang;//,dir;
  float dirChng; 
  float distChng;
  ObjType object_type;  
  char player_side;
  Unum player_number;
  float facedir;
  float neckdir;
  MarkerType marker;
  SideLine  line;
  Vqual view_qual;
  MarkerType closestMarker = No_Marker;
  Bool processThisMarker;
  float closestMarkerDist;

  float pointdir;
  Bool pointing=FALSE;
  Bool tackling=FALSE;

#include <iostream>
  //=================================================================================================
  //=================================================================================================
 
  while ( *SightInfo != ')' ){

    dirChng = NOCHNGINFO;
    facedir = NOFACEINFO;
    neckdir = NOFACEINFO;
    player_number = player_side = 0;

    get_word(&SightInfo);            /* " ((" */

    if ( *SightInfo=='g' ){
      object_type = OBJ_Marker;
      SightInfo+=2;          /* "goal " */
      if ( *SightInfo=='r' )         marker = Goal_R; else
	if ( *SightInfo=='l' )         marker = Goal_L; else
	  my_error("goal ?");
    } else
      if ( *SightInfo=='G' ){
	object_type = OBJ_Marker_Behind;  marker = Mem->ClosestGoal();  
      } else
	if ( *SightInfo=='f' ){
	  object_type = OBJ_Marker;
	  SightInfo+=2;          /* "flag " */
	  if ( *SightInfo=='r' ){
	    SightInfo+=2;
	    if ( *SightInfo=='0' )       marker = Flag_R0;   else
	      if ( *SightInfo=='b' ){
		SightInfo+=1;
		if ( *SightInfo==')' )     marker = Flag_RB; 
		else{
		  SightInfo+=1;
		  if ( *SightInfo=='1' )   marker = Flag_RB10; else
		    if ( *SightInfo=='2' )   marker = Flag_RB20; else
		      if ( *SightInfo=='3' )   marker = Flag_RB30; else
			my_error("flag r b ?");
		}  
	      } else
		if ( *SightInfo=='t' ){
		  SightInfo+=1;
		  if ( *SightInfo==')' )     marker = Flag_RT; 
		  else{
		    SightInfo+=1;
		    if ( *SightInfo=='1' )   marker = Flag_RT10; else
		      if ( *SightInfo=='2' )   marker = Flag_RT20; else
			if ( *SightInfo=='3' )   marker = Flag_RT30; else
			  my_error("flag r t ?");
		  }  
		} else
		  my_error("flag r ?");
	  } else
	    if ( *SightInfo=='l' ){
	      SightInfo+=2;
	      if ( *SightInfo=='0' )       marker = Flag_L0;   else
		if ( *SightInfo=='b' ){
		  SightInfo+=1;
		  if ( *SightInfo==')' )     marker = Flag_LB; 
		  else{
		    SightInfo+=1;
		    if ( *SightInfo=='1' )   marker = Flag_LB10; else
		      if ( *SightInfo=='2' )   marker = Flag_LB20; else
			if ( *SightInfo=='3' )   marker = Flag_LB30; else
			  my_error("flag l b ?");
		  }  
		} else
		  if ( *SightInfo=='t' ){
		    SightInfo+=1;
		    if ( *SightInfo==')' )     marker = Flag_LT; 
		    else{
		      SightInfo+=1;
		      if ( *SightInfo=='1' )   marker = Flag_LT10; else
			if ( *SightInfo=='2' )   marker = Flag_LT20; else
			  if ( *SightInfo=='3' )   marker = Flag_LT30; else
			    my_error("flag l t ?");
		    }  
		  } else
		    my_error("flag l ?");
	    } else
	      if ( *SightInfo=='t' ){
		SightInfo+=2;
		if ( *SightInfo=='0' )       marker = Flag_T0;   else
		  if ( *SightInfo=='l' ){
		    SightInfo+=2;
		    if ( *SightInfo=='1' )     marker = Flag_TL10; else
		      if ( *SightInfo=='2' )     marker = Flag_TL20; else
			if ( *SightInfo=='3' )     marker = Flag_TL30; else
			  if ( *SightInfo=='4' )     marker = Flag_TL40; else
			    if ( *SightInfo=='5' )     marker = Flag_TL50; else
			      my_error("flag t l ?");
		  } else
		    if ( *SightInfo=='r' ){
		      SightInfo+=2;
		      if ( *SightInfo=='1' )     marker = Flag_TR10; else
			if ( *SightInfo=='2' )     marker = Flag_TR20; else
			  if ( *SightInfo=='3' )     marker = Flag_TR30; else
			    if ( *SightInfo=='4' )     marker = Flag_TR40; else
			      if ( *SightInfo=='5' )     marker = Flag_TR50; else
				my_error("flag t r ?");
		    } else
		      my_error("flag t ?");
	      } else
		if ( *SightInfo=='b' ){
		  SightInfo+=2;
		  if ( *SightInfo=='0' )       marker = Flag_B0;   else
		    if ( *SightInfo=='l' ){
		      SightInfo+=2;
		      if ( *SightInfo=='1' )     marker = Flag_BL10; else
			if ( *SightInfo=='2' )     marker = Flag_BL20; else
			  if ( *SightInfo=='3' )     marker = Flag_BL30; else
			    if ( *SightInfo=='4' )     marker = Flag_BL40; else
			      if ( *SightInfo=='5' )     marker = Flag_BL50; else
				my_error("flag b l ?");
		    } else
		      if ( *SightInfo=='r' ){
			SightInfo+=2;
			if ( *SightInfo=='1' )     marker = Flag_BR10; else
			  if ( *SightInfo=='2' )     marker = Flag_BR20; else
			    if ( *SightInfo=='3' )     marker = Flag_BR30; else
			      if ( *SightInfo=='4' )     marker = Flag_BR40; else
				if ( *SightInfo=='5' )     marker = Flag_BR50; else
				  my_error("flag b r ?");
		      } else
			my_error("flag b ?");
		} else
		  if ( *SightInfo=='c' ){
		    SightInfo+=1;
		    if ( *SightInfo==')' )       marker = Flag_C;
		    else{
		      SightInfo+=1;
		      if ( *SightInfo=='b' )     marker = Flag_CB; else
			if ( *SightInfo=='t' )     marker = Flag_CT; else
			  my_error("flag c ?");
		    }
		  } else
		    if ( *SightInfo=='p' ){
		      SightInfo+=2;
		      if ( *SightInfo=='r' ){
			SightInfo+=2;
			if ( *SightInfo=='t')      marker = Flag_PRT; else
			  if ( *SightInfo=='c')      marker = Flag_PRC; else
			    if ( *SightInfo=='b')      marker = Flag_PRB; else	    
			      my_error("flag p r ?");
		      } else
			if ( *SightInfo=='l' ){
			  SightInfo+=2;
			  if ( *SightInfo=='t')      marker = Flag_PLT; else
			    if ( *SightInfo=='c')      marker = Flag_PLC; else
			      if ( *SightInfo=='b')      marker = Flag_PLB; else	    
				my_error("flag p l ?");
			} else
			  my_error("flag p ?");
		    } else
		      if ( *SightInfo=='g' ){
			SightInfo+=2;
			if ( *SightInfo=='l' ){
			  SightInfo+=2;
			  if ( *SightInfo=='t' )     marker = Flag_GLT; else
			    if ( *SightInfo=='b' )     marker = Flag_GLB; else
			      my_error("flag g l ?");
			} else
			  if ( *SightInfo=='r' ){
			    SightInfo+=2;
			    if ( *SightInfo=='t' )     marker = Flag_GRT; else
			      if ( *SightInfo=='b' )     marker = Flag_GRB; else
				my_error("flag g r ?");
			  } else
			    my_error("flag g ?");
		      } else
			my_error("flag ?");
	} else
	  if ( *SightInfo=='F' ){
	    object_type = OBJ_Marker_Behind;
	    marker = Mem->ClosestFlagTo(); /* could be No_Marker */
	  } else
	    if ( *SightInfo=='l' ){
	      object_type = OBJ_Line;
	      SightInfo+=2;          /* "line " */
	      if ( *SightInfo=='r' )         line   = SL_Right;  else
		if ( *SightInfo=='l' )         line   = SL_Left;   else
		  if ( *SightInfo=='t' )         line   = SL_Top;    else
		    if ( *SightInfo=='b' )         line   = SL_Bottom; else
		      my_error("line ?");
	    } else
	      if ( *SightInfo=='p' || *SightInfo=='P' ){
		object_type = OBJ_Player;
		SightInfo+=1;                          /* "player" */
		if ( *SightInfo == ' ' ){              /* there's a team */ 
		  SightInfo+=2;//change AI:kill first "
		  if ( !strncmp(SightInfo,Mem->MyTeamName,Mem->MyTeamNameLen) )
		    player_side = Mem->MySide;
		  else{
		    if ( Mem->TheirTeamName[0] == '\n' ){
		      int a=0;
		      while ( isalpha(*SightInfo) ) Mem->TheirTeamName[a++]=*SightInfo++;
		    }
		    player_side = Mem->TheirSide;
		  }
		  while ( *SightInfo != ' ' && *SightInfo != ')' ) SightInfo++; /* advance past team name */
		  if ( *SightInfo== ' ' ){             /* there's a number */
		    player_number = get_int(&SightInfo);
		  }
		  //		my_error("see player %d of side %c",player_number,player_side);
		  while ( *SightInfo != ' ' && *SightInfo != ')' &&*SightInfo !='g' ) SightInfo++; /* advance past team name */
		  if ( *SightInfo=='g'||*(SightInfo+1)=='g')
		    {
		      if (player_side==Mem->MySide) Mem->OurGoalieNum=player_number;
		      else Mem->TheirGoalieNum=player_number;
		      //			Mem->LogAction4 (10,"Goalie of side %c = %d",player_side,player_number);

		    }
		}
	      } else
		if ( *SightInfo=='b' || *SightInfo=='B' )
		  object_type = OBJ_Ball;
		else
		  {
		    my_error("SightInfo:unknown object");
		  }

    advance_to(')',&SightInfo);             // advance to end of object 
    ang = get_float(&SightInfo);

    if ( *SightInfo != ')' )                  // 'high' quality
      { 
	view_qual = VQ_High;
	dist = ang;
	ang = get_float(&SightInfo);
      }
    else 
      {
	view_qual = VQ_Low;
      }
    if ( view_qual != Mem->ViewQuality ) my_error("View quality %d correct?",view_qual);

    float temp[5];
    int i;
    for (i=0;SightInfo[0]!=')'&&SightInfo[1]!='t';i++)
      {
	temp[i]=get_float(&SightInfo);
	//     Mem->LogAction3(100,"Parsing step %d",i);
      }
    switch (i)
      {
      case 5:
	pointdir=temp[4];
	pointing=TRUE;
	Mem->LogAction4(50,"See: player %d points to %f",player_number,pointdir);
      case 4:
	if (object_type != OBJ_Player) my_error("Only players should have facedir");
	facedir=temp[2];
	neckdir=temp[3];
	distChng=temp[0];
	dirChng=temp[1];
	break;
      case 2:
	distChng=temp[0];
	dirChng=temp[1];
	break;
      case 1:
	if (object_type != OBJ_Player) my_error("Only players can point");
	pointdir=temp[0];
	pointing=TRUE;
	Mem->LogAction4(50,"See: player %d points to %f",player_number,pointdir);
	break;
      default: ;

      }
    if ( /**SightInfo != 't' && */*(SightInfo+1) == 't' )
      {
	if (object_type != OBJ_Player) my_error("Only players can tackle");
	tackling=TRUE;
	Mem->LogAction3(50,"See: player %d tackling!",player_number);
	SightInfo+=2;
      }

    if ( *SightInfo != ')' ) my_error("Should be done with object info here");

    SightInfo++;                                // ")" 


    switch (object_type){
    case OBJ_Marker:
    case OBJ_Marker_Behind:

      // Want to save 2 closest for triangulation  
      // don't want marker_behind unless necessary 

      // If it was a Marker_Behind and we don't know which one 

      if ( marker == No_Marker )
	{
	  if ( object_type != OBJ_Marker_Behind ) my_error("Should know the marker");
	  break;
	}

      processThisMarker = FALSE;

      if ( view_qual == VQ_Low ) 					// Low quality  
	{              
	  // DON'T BOTHER PROCESSING ANY??? I don't think it helps ... 
	  // COULD process 2---then triangulate 
	  //if ( closestMarkerDist > 0 )
	  //{ 
	  //  closestMarkerDist = 0;                   // Only process 1
	  //  processThisMarker = TRUE; 
	  //}
	}
      else										// high quality  
	{
	  bool outside=false;//add by AI
	  if(Mem->MyConf()&&Mem->PlayMode==PM_Play_On&&!Mem->FieldRectangle.IsWithin(Mem->MyPos())&&
	     (marker!=Flag_C/*marker==Flag_CB||marker==Flag_CT||marker==Flag_RB||marker==Flag_LB||marker==Flag_LT||marker==Flag_RT*/))
	    outside=true;
	  if ( (closestMarker == No_Marker || dist < closestMarkerDist)&&!outside) {
	    closestMarker = marker;
	    processThisMarker = TRUE;
	    closestMarkerDist = dist;
	    Mem->ClosestMarker = marker;
	  }
	  processThisMarker = TRUE;//AI:we must now calculate all markers
	}
      if ( processThisMarker )
	{
	  if ( view_qual == VQ_Low )              			   // low quality
	    Mem->SeeMarker(marker, ang, time);
	  else 	// if (dirChng == NOCHNGINFO)                  // high quality
	    Mem->SeeMarker(marker, dist, ang, time);   /* No motion info*/
	}
      break;
    case OBJ_Line:
      if ( *SightInfo != ')' )
	/* There's another line coming.  Assuming lines happen
	   last in the visual string and the closer line comes first */
	; 
      else if ( view_qual == VQ_Low )           /* low quality   */
	Mem->SeeLine(line, ang, time);
      else                                           /* high quality  */
	Mem->SeeLine(line, dist, ang, time);
      break;
    case OBJ_Ball:
      if ( view_qual == VQ_Low )                /* low quality   */
	Mem->SeeBall(ang, time);
      else if ( dirChng == NOCHNGINFO )                   /* high quality  */
	Mem->SeeBall(dist, ang, time);
      else                                           /* know direction*/
	Mem->SeeBall(dist, ang, distChng, dirChng, time);         
      break;
    case OBJ_Player:
      if ( !player_side ){                      /* Too far for team or num */
	if ( view_qual == VQ_Low )                /* low quality   */
	  Mem->SeePlayer(ang, time);
	else if ( dirChng == NOCHNGINFO )                   /* high quality  */
	  Mem->SeePlayer(dist, ang, time);
	else                                           /* know direction*/
	  my_error("Shouldn't know dirChng when the player's far");
      }  

      else{
	if ( !player_number ){                  /* Too far for number     */
	  if ( view_qual == VQ_Low )                /* low quality   */
	    Mem->SeePlayer(player_side, ang, time);
	  else if ( dirChng == NOCHNGINFO )                   /* high quality  */
	    Mem->SeePlayer(player_side, dist, ang, time);
	  else                                           /* know direction*/
	    my_error("Shouldn't know dirChng when the team member's far");
	}

	else{                                   /* Know side AND number   */
	  if ( view_qual == VQ_Low )                /* low quality   */
	    Mem->SeePlayer(player_side, player_number, ang, time);
	  else if ( dirChng == NOCHNGINFO ){                  /* high quality  */
	    printf("%s\n",SightInfo-30);
	    my_error("Should know dirChng when know number");
	    Mem->SeePlayer(player_side, player_number, dist, ang, time);
	  }
	  else                                           /* know direction*/
	    Mem->SeePlayer(player_side, player_number, dist, ang, distChng, dirChng, facedir, neckdir, time);

	}
	if (pointing) Mem->SeePlayerPointing(player_side, player_number,pointdir,time);
	if (tackling) Mem->SeePlayerTackling(player_side, player_number,time);
      }
    }
  }

} //--------------------------function's end


/****************************************************************************************/

void Parse_Sound(Time time, char *SoundInfo)
{
  /*	Mem->LogAction3(10,"COACH::%s", SoundInfo);
	if( SoundInfo[1]=='c' ) {
	SoundInfo += 8;
	Mem->LogAction3(10,"COACH::%s", SoundInfo);
	if (strncmp(SoundInfo, "training", 8) == 0)
	Parse_Trainer_Sound(SoundInfo);
	return;		
	}
  */  if ( SoundInfo[1] == 'r' )		// Referee msg 
    { 					  
      SoundInfo += 9;             
      if (strncmp(SoundInfo, "training", 8) == 0)
	Parse_Trainer_Sound(SoundInfo);
      else if ( islower(SoundInfo[0]) )
	Parse_Referee_Sound(SoundInfo);  
      else
	my_error("Referee sounds should start with lower case letters!");
      return;
    }
  else if ( SoundInfo[1] == 'o' )
    {  									// Online coach message 
      SoundInfo += 14; 	               		// online_coach_ 
      if ( SoundInfo[0] == Mem->MySide )
	{
	  advance_to(' ',&SoundInfo);   		// advance to end of side 
	  SoundInfo++;
	  Parse_My_Coach_Sound(time,SoundInfo);
	}
      else 
	if ( SoundInfo[0] == Mem->TheirSide )
	  {
	    advance_to(' ',&SoundInfo);   /* advance to end of side */
	    SoundInfo++;
	    Parse_Their_Coach_Sound(time,SoundInfo);
	  }
	else 
	  my_error("online_coach_?");
      return;
    }
  Mem->ParsePlayerSound(SoundInfo, time);
}




/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/

void Parse_Referee_Sound(char *msg)
{
  if (msg[strlen(msg) - 1]==')')
    msg[strlen(msg) - 1] = 0;
  else
    if (msg[strlen(msg) - 2]==')')
      msg[strlen(msg) - 2] = 0;

  Mem->LogAction3(200,"Referee message:%s",msg);

  switch( msg[0] ){
  case 'p': 
    if(msg[1]=='l'){
      Mem->SetPlayMode(PM_Play_On);            /* play_on */
    }else if(msg[10]=='t'){
      if(msg[14]==Mem->MySide)
	Mem->SetPlayMode(PM_My_PenaltySetup);
      else if(msg[14]==Mem->TheirSide)
	Mem->SetPlayMode(PM_Their_PenaltySetup);
      else
	my_error("penalty_setup_?");
    }else if(msg[8]=='r'){
      if(msg[14]==Mem->MySide)
	Mem->SetPlayMode(PM_My_PenaltyReady);
      else if(msg[14]==Mem->TheirSide)
	Mem->SetPlayMode(PM_Their_PenaltyReady);
      else
	my_error("penalty_ready_?");
    }else if(msg[8]=='t'){
      if(msg[14]==Mem->MySide)
	Mem->SetPlayMode(PM_My_PenaltyTaken);
      else if(msg[14]==Mem->TheirSide)
	Mem->SetPlayMode(PM_Their_PenaltyTaken);
      else
	my_error("penalty_taken_?");
    }else if(msg[8]=='m'){
      if(msg[13]==Mem->MySide)
	Mem->SetPlayMode(PM_My_PenaltyMiss);
      else if(msg[13]==Mem->TheirSide)
	Mem->SetPlayMode(PM_Their_PenaltyMiss);
      else
	my_error("penalty_miss_?");
    }else if(msg[10]=='o'){
      if(msg[14]==Mem->MySide)
	Mem->SetPlayMode(PM_My_PenaltyScore);
      else if(msg[14]==Mem->TheirSide)
	Mem->SetPlayMode(PM_Their_PenaltyScore);
      else
	my_error("penalty_score_?");
    }else if(msg[8]=='o'){  /*penalty_onfield*/
      setPlay.InitializePenaltyMode(msg[16]);
    }else if(msg[8]=='f'){
      if(msg[13]==Mem->MySide)
	Mem->LogAction2(10,"Our team make foul in penalty mode");
      else if(msg[13]==Mem->TheirSide)
	Mem->LogAction2(10,"Opponent make foul in penalty mode");
      else
	my_error("penalty_foul_?");
    }else      
      my_error("p..?");		
    break;
  case 'k': 
    if ( msg[5] == 'i' ){                                  /* kick_in */
      if ( msg[8] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Kick_In);
      else if ( msg[8] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Kick_In);
      else 
	my_error("kick_in_?");
    }
    else if ( msg[5] == 'o' ){                            /* kick_off */
      if ( msg[9] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Kick_Off);
      else if ( msg[9] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Kick_Off);
      else 
	my_error("kick_off_?");
    }
    else
      my_error("referee k..?");
    break;
  case 'g': 
    if ( msg[5] == 'k' ){                                 /* goal_kick */
      if ( msg[10] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Goal_Kick);
      else if ( msg[10] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Goal_Kick);
      else 
	my_error("goal_kick_?");
    }
    else if ( msg[5] == 'e' ){                           /* goalie_catch_ball */
      if ( msg[18] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Goalie_Free_Kick);
      else if ( msg[18] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Goalie_Free_Kick);
      else
	my_error("goalie_catch_ball_?");
    }
    else if ( msg[5] == Mem->MySide ){                    /* goal */
      Mem->MyScore++;
      //Mem->MyScore = get_int(&msg[7]);
      Mem->KickOffMode = KO_Theirs;
      Mem->SetPlayMode(PM_Before_Kick_Off);
    }
    else if ( msg[5] == Mem->TheirSide ){
      Mem->TheirScore++;
      //Mem->TheirScore = get_int(&msg[7]);
      Mem->KickOffMode = KO_Mine;
      Mem->SetPlayMode(PM_Before_Kick_Off);
    }
    else 
      my_error("referee g..?");
    break;
  case 'c':                                               
    if(msg[1]=='o'){/* corner_kick */
      if ( msg[12] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Corner_Kick);
      else if ( msg[12] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Corner_Kick);
      else 
	my_error("corner_kick_?");
    }else if(msg[1]=='a'){/*catch_fault */
      if(msg[12]==Mem->MySide){
	Mem->SetPlayMode(PM_My_Catch_Fault);
      }else if(msg[12]==Mem->TheirSide){
	Mem->SetPlayMode(PM_Their_Catch_Fault);
      }else
	my_error("catch_fault_?");
    }else
      my_error("c..?");
    break;
  case 'd': Mem->SetPlayMode(PM_Drop_Ball); break;        /* drop_ball */    
  case 'o':                                               /* offside */    
    if ( msg[8] == Mem->MySide )
      Mem->SetPlayMode(PM_Their_Offside_Kick);
    else if ( msg[8] == Mem->TheirSide )
      Mem->SetPlayMode(PM_My_Offside_Kick);
    else 
      my_error("offside_?");
    break;
  case 'f':
    if ( msg[5] == 'k' ){                                 /* free_kick */
      if ( msg[10] == Mem->MySide )
	Mem->SetPlayMode(PM_My_Free_Kick);
      else if ( msg[10] == Mem->TheirSide )
	Mem->SetPlayMode(PM_Their_Free_Kick);
      else if(msg[10]=='f'){
	if ( msg[16] == Mem->MySide ){                     /* free_kick_fault */
	  Mem->SetPlayMode(PM_My_Free_Kick_Fault);
	}else if ( msg[16] == Mem->TheirSide ){
	  Mem->SetPlayMode(PM_Their_Free_Kick_Fault);
	}else
	  my_error("free_kick_fault_?");
      }else				
	my_error("free_kick_?");
    }
    else if(msg[1]=='o'){
      Mem->SetPlayMode(PM_Before_Kick_Off);
      my_error("referee foul..?");
    }
    break;
  case 'h':                                               /* half_time */
    Mem->SetPlayMode(PM_Half_Time);  /* play_mode to before_kick_off        */
    if ( Mem->MySide == 'l' )
      Mem->KickOffMode = KO_Theirs;
    else 
      Mem->KickOffMode = KO_Mine;
    break;             
  case 'b':
    Mem->SetPlayMode(PM_Before_Kick_Off);
    if( msg[1]=='e' ) {				      /* before_kick_off */
      Mem->LogAction2(10,"Parse - from referee - before kick off");
    }else
      if( msg[1]=='a' ) {
	if( msg[10]==Mem->MySide ) {
	  Mem->SetPlayMode(PM_My_Back_Pass);
	}else
	  if( msg[10]==Mem->TheirSide) {
	    Mem->SetPlayMode(PM_Their_Back_Pass);
	  }else{
	    my_error("back_pass_?");
	  }
      }
    break; 
  case 't': 
    if ( msg[5] == 'u' ){                             /* time_up */
      Mem->SetPlayMode(PM_Time_Up); 
    }
    else if ( msg[5] == 'o' )                             /* time_over */
      {
	break;
      }
    else if ( msg[5] == 'e' ){                            /* time_extended */
      Mem->SetPlayMode(PM_Extended_Time);
      if ( Mem->MySide == 'l' )
	Mem->KickOffMode = KO_Mine;
      else 
	Mem->KickOffMode = KO_Theirs;
    } else 
      my_error("referee t..?");
    break;
  case 'i'://add by AI
    if(msg[19]==Mem->MySide)
      Mem->SetPlayMode(PM_My_Free_Kick);
    else if(msg[19]==Mem->TheirSide)
      Mem->SetPlayMode(PM_Their_Free_Kick);
    else
      my_error("indirect_free_kick_?");
    break;
  default: my_error("Referee msg ????");
  }
}

/****************************************************************************************/

/* the trainer send a string that is essentially command line options */
void Parse_Trainer_Sound(char *msg)
{
  msg += 9; /* 'training ' */
  msg[strlen(msg) - 2] = 0; /* cut off the newline and paren */
  Mem->GetOptions(msg);
  Mem->CP_last_reciev=Mem->CurrentTime.t;
  Mem->GetBall()->forget(); /* set 0 confidence in ball pos */
  //  cout << "Incorp trainer message" << endl;
  Mem->LogAction2(175, "Incorporated trainer sound");
}

/****************************************************************************************/

void Parse_My_Coach_Sound(Time , char *msg)
{
  //здесь сейчас разбираем только информацию о типах игроков противника
  Mem->LogAction3(10,"RECIEVED FROM ONLINE COACH: %s",msg);
  istringstream ist(msg);
  string temp;
  int msg_type,opp,player_type;
  char c;
  ist>>temp;
  if(temp=="(freeform"){
    ist.ignore(2);		// пропускаем " \""
    ist>>msg_type;
    Mem->LogAction3(20,"Type of recieved massege %d",msg_type);
    if(msg_type==0){
      while((c=ist.get())!='\"'){
	if(c==' ')
	  continue;
	if(c=='('){
	  ist>>opp>>player_type;
	  Mem->LogAction4(10,"Change opp %.0f to type %.0f",float(opp),float(player_type));
	  Mem->SetOpponentPlayerType(opp,player_type);
	  ist.ignore(1);	// пропускаем ")"
	}
      }
    }
  }     
}

/****************************************************************************************/

void Parse_Their_Coach_Sound(Time , char*)
{
}
/****************************************************************************************/
void Parse_Change_Player_Type(char* msg){
  istringstream ist(msg);
  string temp;
  ist>>temp;//"(change_player_type"
  Unum num;
  int type;
  ist>>num;
  char c;ist>>c;
  if(c==')'){
    Mem->LogAction3(100,"Opponent %d has change his type",num);
    return;
  }
  ist.putback(c);
  ist>>type;
  Mem->SetTeammatePlayerType(num,type);
  Mem->LogAction4(100,"Change teammate %d type to %d",num,type);
}
//////////////////////////////////////////////////////////////////////
void Parse_Ok_Msg(char* msg)
{
  istringstream ist(msg);
  string temp;
  ist>>temp;//"(ok"
  ist>>temp;
  if(temp=="clang")
    Mem->LogAction2(10,"recieved clang massege");
  else
    return;
  ist>>temp;//"(ver"
  ist>>Mem->CP_min_clang_ver;
  ist>>Mem->CP_max_clang_ver;
  Mem->LogAction4(10,"Set clang versions: min=%.0f; max=%.0f",float(Mem->CP_min_clang_ver),float(Mem->CP_max_clang_ver));
}
