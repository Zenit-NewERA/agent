/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemOption.C
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.7 $
 *
 *    $Id: MemOption.C,v 2.7 2004/05/10 14:18:16 anton Exp $
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
  
/* MemOption.C
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

#include "netif.h"
#include "utils.h"
#include "MemOption.h"

void HPlayerType::Log(){
  cout<< "player_speed_max="<<player_speed_max<<
    " stamina_inc_max="<<stamina_inc_max<<
    " player_decay="<<player_decay<<
    " inertia_moment="<<inertia_moment<<
    " dash_power_rate="<<dash_power_rate<<
    " player_size="<<player_size<<
    " kickable_margin="<<kickable_margin<<
    " kick_rand="<<kick_rand<<
    " extra_stamina="<<extra_stamina<<
    " effort_max="<<effort_max<<
    " effort_min="<<effort_min<<endl;
}
/* setting defaults to match version 7.00 server.conf */
OptionInfo::~OptionInfo(){
  delete [] SP_hetro_player_type;
}
//~/////////////////////////////////////////////////
OptionInfo::OptionInfo(){

  VP_test_l   = FALSE;
  VP_test_r   = FALSE;
  VP_test     = FALSE;
  VP_train_DT = FALSE;
  VP_use_DT   = FALSE;

  IS_use_native_ACGT = true;

  IP_my_score             = 0;
  IP_their_score          = 0;
  IP_reconnect            = 0;

  sprintf(MyTeamName,"ERA-Polytech");

  CP_test_shot_y = 0;

  /* no option flags for these */
  SP_pitch_length 	  = 105.0;
  SP_pitch_width 	  = 68.0;
  SP_pitch_margin	  = 5.0;
  SP_penalty_area_length  = 16.5;
  SP_penalty_area_width   = 40.32;
  SP_goal_area_length	  = 5.5;
  SP_goal_area_width	  = 18.32;					
  SP_penalty_spot_dist    = 11.0;
  SP_corner_arc_r	  = 1.0;
  SP_free_kick_buffer     = 9.15;
  SP_after_goal_wait  	  = 50;
  SP_feel_distance        = 3.0;
  SP_num_lines            = 4;
  SP_num_markers          = 55;
  SP_unum_far_length      = 20.0;
  SP_unum_too_far_length  = 40.0;
  SP_team_far_length      = 40.0;
  SP_team_too_far_length  = 60.0;

  SP_version              = 9.0;
  SP_team_size            = 11;
  SP_half                 = 1;
  sprintf(SP_host,        "localhost");
  sprintf(CP_formation_conf,"./formations.conf");
  sprintf(CP_pass_weights_file,"./passweights.conf");
  SP_goal_width           = 14.02;       
  SP_player_size          = 0.3;					//Change:AI
  SP_player_decay         = 0.4;					//***********
  SP_player_rand 	  = 0.1;						//**************
  SP_player_weight	  = 60.0;						//*************
  SP_player_speed_max     = 1.2;					//Change:AI
  SP_stamina_max	  = 4000.0;						//Change:AI
  SP_stamina_inc          = 45.0;					//Change:AI
  SP_recover_dec_thr      = 0.3;					//*******************
  SP_recover_dec          = 0.002; 					//Change:AI
  SP_recover_min          = 0.5;					//Change:AI
  SP_effort_dec_thr       = 0.3;					//Change:AI					
  SP_effort_min           = 0.6;					//Change:AI
  SP_effort_dec           = 0.005;					//Change:AI
  SP_effort_inc_thr       = 0.6;					//Change:AI
  SP_effort_inc           = 0.01;					//Change:AI
  SP_ball_size            = 0.085;					//**************		
  SP_ball_decay           = 0.94;					//Change:AI	
  SP_ball_rand            = 0.05;					//**************	
  SP_ball_weight          = 0.2;					//****************	
  SP_ball_speed_max       = 2.7;					//Change:AI	
  SP_dash_power_rate      = 0.006;					//Change:AI	
  SP_kick_power_rate      = 0.027;					//Change:AI
  SP_kickable_margin      = 0.7;					//Change:AI	
  SP_kickable_area        = SP_kickable_margin + SP_ball_size + SP_player_size;		//********
  SP_catch_prob           = 1.0;					//*********************8
  SP_catch_area_l         = 2.0;					//********************
  SP_catch_area_w         = 1.0;					//*******************	
  SP_catch_ban_cycle      = 5;
  SP_max_power            = 100;					//********
  SP_min_power            = -100;					//Change:AI	
  SP_max_moment           = 180;					//***************
  SP_min_moment           = -180;					//********************
  SP_min_neck_angle       = -90.0;					//*******************
  SP_max_neck_angle       = 90.0;					//*****************
  SP_min_neck_moment      = -180.0;					//****************
  SP_max_neck_moment      = 180.0;					//*************
  SP_visible_angle        = 90.0;					//***********************
  SP_audio_cut_dist       = 50.0;	
  SP_dist_qstep           = 0.1;	
  SP_land_qstep           = 0.01;	
  SP_ckmargin             = 1.0;					//*******************
  SP_wind_dir             = 0.0;					//********************
  SP_wind_force           = 0.0;					//********************
  SP_wind_rand            = 0.0;					//*******************	
  SP_wind_none            = FALSE;					//*************88
  SP_wind_random          = FALSE;					//
  SP_half_time            = 300;
  SP_port                 = 6000;
  SP_coach_port           = 6001;
  SP_olcoach_port         = 6002;
  SP_simulator_step       = 100;
  SP_send_step            = 150;
  SP_recv_step            = 10;
  SP_say_msg_size         = 10;//change by AI
  SP_hear_max             = 2;
  SP_hear_inc             = 1;
  SP_hear_decay           = 2;
  SP_coach_mode           = FALSE;
  SP_coach_w_referee_mode = FALSE;
  SP_say_coach_cnt_max    = 128;
  SP_say_coach_msg_size   = 128;
  SP_send_vi_step         = 100;
  SP_look_step            = 100;
  SP_use_offside          = FALSE;
  SP_forbid_kickoff_offside = TRUE;
  SP_verbose              = TRUE;
  SP_offside_area         = 9.15;
  SP_inertia_moment       = 5.0;				//************
  SP_sense_body_step      = 100;
  SP_offside_kick_margin  = 9.15;
  SP_record_messages      = FALSE;
  SP_clang_win_size		  =	300;
  SP_clang_define_win     = 1;
  SP_clang_meta_win       = 1;
  SP_clang_advice_win     = 1;
  SP_clang_info_win       = 1;
  SP_clang_mess_delay     = 50;
  SP_clang_mess_per_cycle = 1;
  SP_player_accel_max     = 1.0;
  SP_log_times            = FALSE;
  SP_goalie_max_moves     = 2;
  SP_baccel_max           = 2.7;
  SP_kick_rand            = 0.0;
  SP_slowness_on_top_for_left_team = 1.0;
  SP_slowness_on_top_for_left_team = 1.0;
  SP_synch_mode = FALSE;
  SP_synch_micro_sleep = 1;
  SP_ball_accel_max   =2.7;
  SP_drop_ball_time =200;
  SP_game_logging=TRUE;
  SP_send_comms=TRUE;
  SP_text_logging=TRUE;
  SP_game_log_version=3;
  sprintf(SP_text_log_dir,"./");
  sprintf(SP_game_log_dir,"./");
  sprintf(SP_text_log_fixed_name,"rcssserver");
  sprintf(SP_game_log_fixed_name,"rcssserver");
  SP_text_log_fixed=FALSE;
  SP_game_log_fixed=FALSE;
  SP_text_log_dated=TRUE;
  SP_game_log_dated=TRUE;
  //  sprintf(SP_log_date_format,"%Y%m%d%H%M-");
  SP_text_log_compression=0;
  SP_game_log_compression=0;
  sprintf(SP_landmark_file,"Ohh");
  //new
  SP_recover_init=0 ;
  SP_effort_init=0 ;
  SP_team_actuator_noise=0 ;
  SP_prand_factor_l=1 ;
  SP_prand_factor_r=1 ;
  SP_kick_rand_factor_l=1 ;
  SP_kick_rand_factor_r =1 ;
  SP_control_radius=1 ;
  SP_control_radius_width=1.7 ;
  SP_quantize_step_dir=-1 ;
  SP_quantize_step_dist_team_l=-1 ;
  SP_quantize_step_dist_team_r=-1 ;
  SP_quantize_step_dist_l_team_r=-1 ;
  SP_quantize_step_dist_l_team_l=-1 ;
  SP_quantize_step_dir_team_l=-1 ;
  SP_quantize_step_dir_team_r=-1 ;
  SP_wind_ang= 0;
  SP_lcm_step=300 ;
  SP_old_coach_hear=0 ;
  SP_slow_down_factor=1 ;
  SP_synch_offset=60 ;
  SP_start_goal_l=0 ;
  SP_start_goal_r=0 ;
  SP_fullstate_l=0 ;
  SP_fullstate_r=0 ;
  SP_profile=0 ;
  SP_point_to_ban=5 ;
  SP_point_to_duration=20 ;
  SP_tackle_dist=2.5 ;
  SP_tackle_back_dist=0.5 ;
  SP_tackle_width=1.25 ;
  SP_tackle_exponent=6 ;
  SP_tackle_cycles=10 ;
  SP_clang_rule_win=1;
  SP_clang_del_win=1;
  SP_freeform_send_period  =20;
  SP_freeform_wait_period  =600;
  SP_max_goal_kicks     = 3;
  SP_back_passes        =TRUE;
  SP_free_kick_faults   =TRUE;
  SP_proper_goal_kicks  =FALSE;
  SP_stopped_ball_vel   =0.01;
  SP_tackle_power_rate   =0.027;
  //AI: new at 9.0 and higher
  SP_connect_wait=300;
  SP_game_over_wait=100;
  SP_keepaway_start=-1;
  SP_kick_off_wait=100;
  SP_nr_extra_halfs=2;
  SP_nr_normal_halfs=2;
  SP_pen_before_setup_wait=30;
  SP_pen_setup_wait=100;
  SP_pen_ready_wait=50;
  SP_pen_taken_wait=100;
  SP_pen_nr_kicks=5;
  SP_pen_max_extra_kicks=10;
  sprintf(SP_keepaway_log_dir,"./");
  sprintf(SP_keepaway_log_fixed_name,"rcssserver");
  sprintf(SP_team_l_start," ");
  sprintf(SP_team_r_start," ");
  SP_auto_mode=FALSE;
  SP_keepaway=FALSE;
  SP_keepaway_width=20.0f;
  SP_keepaway_length=20.0f;
  SP_keepaway_logging=TRUE;
  SP_keepaway_log_fixed=FALSE;
  SP_keepaway_log_dated=TRUE;
  SP_pen_allow_mult_kicks=FALSE;
  SP_pen_random_winner=FALSE;
  SP_penalty_shoot_outs=TRUE;
  SP_pen_dist_x=11.0f;
  SP_pen_max_goalie_dist_x=4.0f;
  sprintf(SP_module_dir," ");
  SP_pen_coach_moves_players=TRUE;
  //AI: hetro players parametrs
  SP_player_types=7;
  SP_pt_max=3;
  SP_random_seed=-1.0;
  SP_subs_max=3;
  SP_dash_power_rate_delta_max=0;
  SP_dash_power_rate_delta_min=0;
  SP_effort_max_delta_factor=-0.002;
  SP_effort_min_delta_factor=-0.002;
  SP_extra_stamina_delta_max=100;
  SP_extra_stamina_delta_min=0;
  SP_inertia_moment_delta_factor=25;
  SP_kick_rand_delta_factor=0.5;
  SP_kickable_margin_delta_max=0.2;
  SP_kickable_margin_delta_min=0;
  SP_new_dash_power_rate_delta_max=0.002;
  SP_new_dash_power_rate_delta_min=0;
  SP_new_stamina_inc_max_delta_factor=-10000;
  SP_player_decay_delta_max=0.2;
  SP_player_decay_delta_min=0;
  SP_player_size_delta_factor=-100;
  SP_player_speed_max_delta_max=0;
  SP_player_speed_max_delta_min=0;
  SP_stamina_inc_max_delta_factor=0;
	
  CP_goalie               = FALSE;
  CP_save_log             = TRUE;
  CP_save_freq            = 10;
  CP_save_sound_log       = TRUE;
  CP_save_sound_freq      = 10;
  CP_save_action_log_level = 40; /* 0 means save nothing */
  CP_save_action_freq      = 40;
  CP_send_ban_recv_step_factor = 3.0;
  CP_interrupts_per_cycle = 2;
  CP_interrupts_left_to_act = 2;
  CP_max_conf             = 1.0;
  CP_min_valid_conf       = 0.5;
  CP_conf_decay           = 0.98;
  CP_player_conf_decay    = 0.99;
  CP_ball_conf_decay      = 0.9;
  CP_max_player_move_factor = 4;
  CP_max_say_interval     = 100;
  CP_ball_moving_threshold = .2; /* experimentally checked -- ball still, player fast => .15 ball speed */
  CP_dodge_angle_buffer    = 25;
  CP_dodge_distance_buffer = 3.5;
  CP_dodge_power           = 30;
  CP_dodge_angle           = 90;
  sprintf(CP_tree_stem,      "pass");
  CP_DT_evaluate_interval  = 10;
  CP_say_tired_interval    = 20;
  CP_tired_buffer          = 10;
  CP_set_plays             = FALSE;
  CP_Setplay_Delay         = 5;
  CP_Setplay_Say_Delay     = SP_hear_decay*5;
  CP_Setplay_Max_Delay     = 100;
  CP_Setplay_Time_Limit    = 150;
  CP_kickable_buffer       = .1;
  CP_mark_persist_time     = 100;
  CP_track_min_distance    = 0;
  CP_track_max_distance    = 15;
  CP_pull_offsides         = FALSE;
  CP_pull_offsides_when_winning = TRUE;
  CP_spar                  = TRUE;
  CP_mark                  = TRUE;
  CP_communicate           = TRUE;
  CP_change_view_for_ball_cycles = 2;
  CP_defer_kick_to_teammate_buffer = .05;
  CP_scan_overlap_angle    = 2;

  CP_pull_offside_threshold = 5;
  CP_pull_offside_buffer = 3;

  CP_ball_forget_angle_buf = 3;
  CP_player_forget_angle_buf = 5;
  CP_ball_forget_dist_buf = 1;
  CP_player_forget_dist_buf = 1;

  CP_beat_offsides_buffer = 20;
  CP_beat_offsides_threshold = 30;
  CP_beat_offsides_max_x = 25;
  CP_congestion_epsilon = .01;
  CP_back_pass_opponent_buffer = 10;
  CP_back_pass_offside_buffer = 10;
  CP_min_less_congested_pass_dist = 7;
  
  /* pat added these */
  CP_use_new_position_based_vel = TRUE;
  CP_stop_on_error = TRUE;
  
  CP_opt_ctrl_dist        = SP_player_size + 0.8f *SP_kickable_margin;
  CP_KickTo_err           = 3;
  CP_closest_margin       = SP_player_size + 2.0f*SP_ball_size;
  CP_dokick_factor        = .22;
  CP_max_turn_kick_pow    = 20;

  CP_max_ignore_vel       = .005;
  CP_kick_time_space      = 1;
  CP_max_est_err          = .3;
  CP_max_go_to_point_angle_err = 5;
  CP_holdball_kickable_buffer = .1;
  CP_stop_ball_power          = 30;
  CP_possessor_intercept_space = 4;
  CP_can_keep_ball_cycle_buffer = 0;

  CP_hard_kick_dist_buffer = .1;
  CP_max_hard_kick_angle_err  = 5;

  CP_hardest_kick_player_ang = 90; //angle relative to direction of ball
  CP_hardest_kick_ball_dist = .831; //kickable_area * .6 
  CP_hardest_kick_ball_ang = 15; // this is realtive to the direction of travel 
  CP_max_dash_help_kick_angle = 60;

  CP_max_int_lookahead    = 20;
  CP_intercept_step       = 2;
  CP_my_intercept_step    = 2;
  CP_intercept_aim_ahead  = 1;
  CP_no_turn_max_cyc_diff = -1;//AI: we long play with 2
  CP_no_turn_max_dist_diff = 1.0;
  CP_turnball_opp_worry_dist = 5; 
  CP_collision_buffer = .1;
  CP_behind_angle  = 80;
  CP_time_for_full_rotation = 24; /* average guestimate */
  CP_ball_vel_invalidation_factor = 4.0;//AI:все время играли 2.0

  CP_move_imp_1v1_initial =  0.0;
  CP_move_imp_1v1_inc =       .2;
  CP_move_imp_1v1_threshold = 1.0 ;
  CP_at_point_buffer = 1;
  CP_overrun_dist = 3;
  CP_def_block_dist = 1.0;//chang by AI(for anti_dribble). was 2
  CP_def_block_dist_ratio = .5;
  CP_overrun_buffer = 2.5;
  CP_cycles_to_kick = 4;
  CP_breakaway_buffer = 3;
  CP_our_breakaway_kickable_buffer = 1.5;
  CP_their_breakaway_front_kickable_buffer = 5.0;
  CP_their_breakaway_back_kickable_buffer = 2.0;

  CP_breakaway_approach_x = 35;
  CP_breakaway_approach_y = 8;
  CP_breakaway_targ_valid_time = 3;
  CP_breakaway_min_goalie_steal_time = 6;
  CP_breakaway_kick_run_min_cycles = 7;
  CP_breakaway_kick_run_max_cycles = 14;
  CP_our_breakaway_min_cone_dist_wid = 18;
  CP_their_breakaway_min_cone_dist_wid = 12;
  CP_breakaway_middle_buffer = 3;
  CP_breakaway_kick_run_worry_dist = 10;
  CP_breakaway_mode = 0;
  
  CP_static_kick_dist_err = .3;//old: .14
  CP_static_kick_ang_err = 15;//old: 5

  CP_goalie_baseline_buffer = 1;
  CP_goalie_scan_angle_err = 10;
  CP_goalie_at_point_buffer = .1;
  CP_goalie_vis_angle_err = 5;
  CP_goalie_max_shot_distance = 40;
  CP_goalie_min_pos_dist = 15;
  CP_goalie_max_pos_dist = SP_pitch_length * .75;
  CP_goalie_max_forward_percent = .75;
  CP_goalie_ball_ang_for_corner = 90;
  CP_goalie_max_come_out_dist = 10;
  CP_goalie_ball_dist_for_corner = SP_penalty_area_length;
  CP_goalie_ball_dist_for_center = SP_pitch_length / 2;
  CP_goalie_free_kick_dist = 3;
  CP_goalie_go_to_ball_cone_ratio = .25;
  CP_goalie_warn_space = 10;
  CP_goalie_comes_out = TRUE;
  CP_goalie_catch_wait_time = 2;
  CP_goalie_opponent_dist_to_block = 15;
  CP_goalie_position_weight_dist = 10;
  CP_goalie_narrow_sideline_cyc = 3;
  CP_goalie_no_buffer_dist = 10;

  CP_clear_ball_ang_step = 5.0;
  CP_clear_ball_cone_ratio = .5;
  CP_clear_ball_max_dist = 30;
  CP_clear_offensive_min_horiz_dist = 20;
  CP_clear_offensive_min_angle = 60;

  CP_should_cross_corner_dist = 14; //pitch_width /2 - penalty_area_w / 2
  CP_should_cross_baseline_buffer = 3;
  CP_should_move_to_cross_corner_dist = 20; 
  CP_cross_pt_x = 36;  //pitch_length / 2 - penalty_area_l
  CP_cross_pt_y = 9; //goalie_area_w / 2
  CP_cross_target_vel = .5;

  CP_dont_dribble_to_middle_min_x = 20;

  CP_good_shot_distance = 20;
  CP_shot_distance = 30;
  CP_cycles_to_kick_buffer = 1;
  CP_better_shot_cyc_diff = 5;
  //CP_breakaway_shot_distance = 16;
  CP_shot_speed = 2.0; //our average shot speed
  CP_shot_goalie_react_buffer = 5;
  CP_good_shot_goalie_react_buffer = 2;
  //add by AI
  CP_offside_value=0.5;//0-old offside calculate metod, 1- maybe good again Cyberoos20001
  CP_dist_to_wide_view=30.0;//dist from that we change view width to wide
  CP_use_anti_dribble=TRUE;//must be good against Chain dribble
  CP_penalty_mode=FALSE;//set TRUE then player must kick penalty
  CP_midfielders_close_to_defense=FALSE;//if TRUE then midfielder will be close to our defense line : good tested
  CP_min_clang_ver=7;
  CP_max_clang_ver=9;

  CP_test_pass_angle    = 0.0f;
  CP_test_pass_velocity = 0.0f;
  CP_test_should_pass   = FALSE;


  //Added by CoolSerg
  CP_team_receive_time = 20;

  //from trainer
  CP_lp_power=100.0;
  CP_lp_angle=0.0;
  CP_last_reciev=0;

  sprintf(FP_initial_formation, "433");
  sprintf(FP_formation_when_tied, "initial");
  sprintf(FP_formation_when_losing, "initial");
  sprintf(FP_formation_when_losing_lots, "initial");
  sprintf(FP_formation_when_winning, "initial");
  sprintf(FP_initial_hc_method, "Obey");
  sprintf(FP_initial_mc_method, "Obey");
  FP_initial_player_1_pos     = 1;
  FP_initial_player_2_pos     = 2;
  FP_initial_player_3_pos     = 3;
  FP_initial_player_4_pos     = 4;
  FP_initial_player_5_pos     = 5;
  FP_initial_player_6_pos     = 6;
  FP_initial_player_7_pos     = 7;
  FP_initial_player_8_pos     = 8;
  FP_initial_player_9_pos     = 9;
  FP_initial_player_10_pos    = 10;
  FP_initial_player_11_pos    = 11;
  FP_goalie_number            = 1; //modifide by AI

}

void OptionInfo::GetOptions(int argc, char **argv)
{
  option_t opt[] = {
    {"test_shot_y",(void *)&CP_test_shot_y,V_FLOAT},
    {"use_native_ACGT",(void *)&IS_use_native_ACGT,V_ONOFF},
    {"test_l",        		(void *)&VP_test_l,             V_ONOFF},
    {"test_r",        		(void *)&VP_test_r,             V_ONOFF},
    {"test",        		(void *)&VP_test,               V_ONOFF},
    {"train_DT",                    (void *)&VP_train_DT,           V_ONOFF},
    {"use_DT",                      (void *)&VP_use_DT,             V_ONOFF},

    {"my_score",        		(void *)&IP_my_score,           V_INT},
    {"their_score",        		(void *)&IP_their_score,        V_INT},
    {"reconnect",                   (void *)&IP_reconnect,          V_INT},

    {"team_name",        		(void *)&MyTeamName,            V_STRING},
    {"goalie",        		(void *)&CP_goalie,             V_ONOFF},
    {"save_log",        		(void *)&CP_save_log,           V_ONOFF},
    {"save_freq",                   (void *)&CP_save_freq,          V_INT},
    {"save_sound_log",  		(void *)&CP_save_sound_log,     V_ONOFF},
    {"save_sound_freq",             (void *)&CP_save_sound_freq,    V_INT},
    {"save_action_log_level",       (void *)&CP_save_action_log_level, V_INT},
    {"save_action_freq",            (void *)&CP_save_action_freq, V_INT},
    {"send_ban_recv_step_factor",   (void *)&CP_send_ban_recv_step_factor, V_FLOAT},
    {"interrupts_per_cycle",        (void *)&CP_interrupts_per_cycle,   V_INT},
    {"interrupts_left_to_act",      (void *)&CP_interrupts_left_to_act,   V_INT},
    {"max_conf",                    (void *)&CP_max_conf,           V_FLOAT},
    {"min_conf",                    (void *)&CP_min_valid_conf,     V_FLOAT},
    {"conf_decay",                  (void *)&CP_conf_decay,         V_FLOAT},
    {"player_conf_decay",           (void *)&CP_player_conf_decay,  V_FLOAT},
    {"ball_conf_decay",             (void *)&CP_ball_conf_decay,    V_FLOAT},
    {"max_player_move_factor",      (void *)&CP_max_player_move_factor, V_FLOAT},
    {"max_say_interval",            (void *)&CP_max_say_interval,   V_INT},
    {"ball_moving_threshold",       (void *)&CP_ball_moving_threshold, V_FLOAT},
    {"dodge_distance_buffer",       (void *)&CP_dodge_distance_buffer, V_FLOAT},
    {"dodge_angle_buffer",          (void *)&CP_dodge_angle_buffer, V_FLOAT},
    {"dodge_power",                 (void *)&CP_dodge_power,        V_FLOAT},
    {"dodge_angle",                 (void *)&CP_dodge_angle,        V_FLOAT},
    {"tree_stem",                   (void *)&CP_tree_stem,          V_STRING},
    {"DT_evaluate_interval",        (void *)&CP_DT_evaluate_interval, V_INT},
    {"say_tired_interval",          (void *)&CP_say_tired_interval, V_INT},
    {"tired_buffer",                (void *)&CP_tired_buffer,       V_FLOAT},
    {"set_plays",                   (void *)&CP_set_plays,          V_ONOFF},
    {"set_play_delay",              (void *)&CP_Setplay_Delay,     V_INT},
    {"set_play_say_delay",          (void *)&CP_Setplay_Say_Delay,  V_INT},
    {"set_play_time_limit",         (void *)&CP_Setplay_Time_Limit, V_INT},
    {"kickable_buffer",             (void *)&CP_kickable_buffer,    V_FLOAT},
    {"mark_persist_time",           (void *)&CP_mark_persist_time,  V_INT},
    {"track_max_distance",          (void *)&CP_track_max_distance, V_FLOAT},
    {"track_min_distance",          (void *)&CP_track_min_distance, V_FLOAT},
    {"pull_offsides",               (void *)&CP_pull_offsides,      V_ONOFF},
    {"pull_offsides_when_winning",  (void *)&CP_pull_offsides_when_winning, V_ONOFF},
    {"spar",                        (void *)&CP_spar,            V_ONOFF},
    {"mark",                        (void *)&CP_mark,            V_ONOFF},
    {"communicate",                 (void *)&CP_communicate,        V_ONOFF},
    {"change_view_for_ball_cycles", (void *)&CP_change_view_for_ball_cycles, V_INT},
    {"defer_kick_to_teammate_buffer",(void *)&CP_defer_kick_to_teammate_buffer, V_FLOAT},
    {"scan_overlap_angle",          (void *)&CP_scan_overlap_angle, V_FLOAT},

    {"pull_offside_threshold",      (void *)&CP_pull_offside_threshold, V_FLOAT},
    {"pull_offside_buffer",         (void *)&CP_pull_offside_buffer,    V_FLOAT},

    {"ball_forget_angle_buf",       (void *)&CP_ball_forget_angle_buf,   V_FLOAT},
    {"player_forget_angle_buf",     (void *)&CP_player_forget_angle_buf, V_FLOAT},
    {"ball_forget_dist_buf",        (void *)&CP_ball_forget_dist_buf,    V_FLOAT},
    {"player_forget_dist_buf",      (void *)&CP_player_forget_dist_buf,  V_FLOAT},

    {"beat_offsides_buffer",        (void *)&CP_beat_offsides_buffer,    V_FLOAT},
    {"beat_offsides_threshold",     (void *)&CP_beat_offsides_threshold,    V_FLOAT},
    {"beat_offsides_max_x",         (void *)&CP_beat_offsides_max_x,    V_FLOAT},
    {"congestion_epsilon",          (void *)&CP_congestion_epsilon,    V_FLOAT},
    {"back_pass_opponent_buffer",   (void *)&CP_back_pass_opponent_buffer,    V_FLOAT},
    {"back_pass_offside_buffer",   (void *)&CP_back_pass_offside_buffer,    V_FLOAT},
    {"min_less_congested_pass_dist",(void *)&CP_min_less_congested_pass_dist,    V_FLOAT},

    {"use_new_position_based_vel",  (void *)&CP_use_new_position_based_vel, V_ONOFF},
    {"stop_on_error",               (void *)&CP_stop_on_error,      V_ONOFF},

    {"opt_ctrl_dist",               (void *)&CP_opt_ctrl_dist,      V_FLOAT},
    {"KickTo_err",                  (void *)&CP_KickTo_err,         V_FLOAT},
    {"closest_margin",              (void *)&CP_closest_margin,     V_FLOAT},
    {"dokick_factor",               (void *)&CP_dokick_factor,      V_FLOAT},
    {"max_turn_kick_pow",           (void *)&CP_max_turn_kick_pow,  V_FLOAT},
    {"kick_time_space",             (void *)&CP_kick_time_space,    V_INT},
    {"max_ignore_vel",              (void *)&CP_max_ignore_vel,     V_FLOAT},
    {"max_est_err",                 (void *)&CP_max_est_err,        V_FLOAT},
    {"holdball_kickable_buffer",    (void *)&CP_holdball_kickable_buffer, V_FLOAT},
    {"stop_ball_power",             (void *)&CP_stop_ball_power, V_INT},
    {"possessor_intercept_space",   (void *)&CP_possessor_intercept_space, V_INT},
    {"can_keep_ball_cycle_buffer",  (void *)&CP_can_keep_ball_cycle_buffer, V_INT},

    {"max_hard_kick_angle_err",     (void *)&CP_max_hard_kick_angle_err, V_INT},
    {"hard_kick_dist_buffer",       (void *)&CP_hard_kick_dist_buffer, V_FLOAT},
    {"hardest_kick_ball_ang",       (void *)&CP_hardest_kick_ball_ang,  V_INT},
    {"hardest_kick_ball_dist",      (void *)&CP_hardest_kick_ball_dist,  V_FLOAT},
    {"hardest_kick_player_ang",     (void *)&CP_hardest_kick_player_ang, V_INT},
    {"max_dash_help_kick_angle",    (void *)&CP_max_dash_help_kick_angle, V_FLOAT},

    {"max_go_to_point_angle_err",   (void *)&CP_max_go_to_point_angle_err, V_INT},
    {"max_int_lookahead",           (void *)&CP_max_int_lookahead,  V_INT},
    {"intercept_close_dist",        (void *)&CP_intercept_close_dist,  V_FLOAT},
    {"intercept_step",              (void *)&CP_intercept_step,     V_INT},	
    {"my_intercept_step",           (void *)&CP_my_intercept_step,  V_INT},	
    {"intercept_aim_ahead",         (void *)&CP_intercept_aim_ahead, V_INT},
    {"no_turn_max_cyc_diff",        (void *)&CP_no_turn_max_cyc_diff, V_INT},
    {"no_turn_max_dist_diff",       (void *)&CP_no_turn_max_dist_diff, V_FLOAT},

    {"turnball_opp_worry_dist",     (void *)&CP_turnball_opp_worry_dist,V_FLOAT},
    {"collision_buffer",            (void *)&CP_collision_buffer,  V_FLOAT},
    {"behind_angle",                (void *)&CP_behind_angle, V_FLOAT},
    {"ball_vel_invalidation_factor",(void *)&CP_ball_vel_invalidation_factor, V_FLOAT},
    {"time_for_full_rotation",      (void *)&CP_time_for_full_rotation, V_INT},

    {"move_imp_1v1_initial",        (void *)&CP_move_imp_1v1_initial, V_FLOAT},
    {"move_imp_1v1_inc",            (void *)&CP_move_imp_1v1_inc, V_FLOAT},
    {"move_imp_1v1_threshold",      (void *)&CP_move_imp_1v1_threshold, V_FLOAT},
    {"at_point_buffer",             (void *)&CP_at_point_buffer, V_FLOAT},
    {"overrun_dist",                (void *)&CP_overrun_dist, V_FLOAT},
    {"def_block_dist",              (void *)&CP_def_block_dist, V_FLOAT},
    {"def_block_dist_ratio",        (void *)&CP_def_block_dist_ratio, V_FLOAT},
    {"overrun_buffer",              (void *)&CP_overrun_buffer, V_FLOAT},
    {"cycles_to_kick",              (void *)&CP_cycles_to_kick, V_FLOAT},
    {"breakaway_buffer",            (void *)&CP_breakaway_buffer, V_FLOAT},
    {"our_breakaway_kickable_buffer",   (void *)&CP_our_breakaway_kickable_buffer, V_FLOAT},
    {"their_breakaway_front_kickable_buffer",   (void *)&CP_their_breakaway_front_kickable_buffer, V_FLOAT},
    {"their_breakaway_back_kickable_buffer",   (void *)&CP_their_breakaway_back_kickable_buffer, V_FLOAT},
    {"goalie_breakaway_kickable_buffer",   (void *)&CP_goalie_breakaway_kickable_buffer, V_FLOAT},

    {"breakaway_approach_x",        (void *)&CP_breakaway_approach_x,  V_FLOAT},
    {"breakaway_approach_y",        (void *)&CP_breakaway_approach_y,  V_FLOAT},
    {"breakaway_targ_valid_time",   (void *)&CP_breakaway_targ_valid_time,       V_INT},
    {"breakaway_min_goalie_steal_time", (void *)&CP_breakaway_min_goalie_steal_time, V_INT},
    {"breakaway_kick_run_min_cycles",(void *)&CP_breakaway_kick_run_min_cycles, V_INT},
    {"breakaway_kick_run_max_cycles",(void *)&CP_breakaway_kick_run_max_cycles, V_INT},
    {"their_breakaway_min_cone_dist_wid",  (void *)&CP_their_breakaway_min_cone_dist_wid,     V_FLOAT},
    {"our_breakaway_min_cone_dist_wid",  (void *)&CP_our_breakaway_min_cone_dist_wid,     V_FLOAT},
    {"breakaway_middle_buffer",      (void *)&CP_breakaway_middle_buffer, V_FLOAT},
    {"breakaway_kick_run_worry_dist",(void *)&CP_breakaway_kick_run_worry_dist, V_FLOAT},
    {"breakaway_mode",              (void *)&CP_breakaway_mode, V_INT},

    {"static_kick_dist_err",        (void *)&CP_static_kick_dist_err, V_FLOAT},
    {"static_kick_ang_err",         (void *)&CP_static_kick_ang_err, V_FLOAT},

    {"goalie_baseline_buffer",      (void *)&CP_goalie_baseline_buffer, V_FLOAT},
    {"goalie_scan_angle_err",       (void *)&CP_goalie_scan_angle_err, V_FLOAT},
    {"goalie_at_point_buffer",      (void *)&CP_goalie_at_point_buffer, V_FLOAT},
    {"goalie_vis_angle_err",        (void *)&CP_goalie_vis_angle_err, V_FLOAT},
    {"goalie_max_shot_distance",    (void *)&CP_goalie_max_shot_distance, V_FLOAT},
    {"goalie_min_pos_dist",         (void *)&CP_goalie_min_pos_dist, V_FLOAT},
    {"goalie_max_pos_dist",         (void *)&CP_goalie_max_pos_dist, V_FLOAT},
    {"goalie_max_forward_percent",  (void *)&CP_goalie_max_forward_percent, V_FLOAT},
    {"goalie_ball_ang_for_corner",  (void *)&CP_goalie_ball_ang_for_corner, V_FLOAT},
    {"goalie_max_come_out_dist",    (void *)&CP_goalie_max_come_out_dist, V_FLOAT},
    {"goalie_ball_dist_for_corner", (void *)&CP_goalie_ball_dist_for_corner, V_FLOAT},
    {"goalie_ball_dist_for_center", (void *)&CP_goalie_ball_dist_for_center, V_FLOAT},
    {"goalie_free_kick_dist",       (void *)&CP_goalie_free_kick_dist, V_FLOAT},
    {"goalie_go_to_ball_cone_ratio",(void *)&CP_goalie_go_to_ball_cone_ratio, V_FLOAT},
    {"goalie_warn_space",           (void *)&CP_goalie_warn_space, V_INT},
    {"goalie_comes_out",            (void *)&CP_goalie_comes_out, V_ONOFF},
    {"goalie_catch_wait_time",      (void *)&CP_goalie_catch_wait_time, V_INT},
    {"goalie_opponent_dist_to_block", (void *)&CP_goalie_opponent_dist_to_block, V_FLOAT},
    {"goalie_position_weight_dist", (void *)&CP_goalie_position_weight_dist, V_FLOAT},
    {"goalie_narrow_sideline_cyc",  (void *)&CP_goalie_narrow_sideline_cyc,  V_INT},
    {"goalie_no_buffer_dist",       (void *)&CP_goalie_no_buffer_dist, V_FLOAT},
    
    {"clear_ball_ang_step",         (void *)&CP_clear_ball_ang_step, V_FLOAT},
    {"clear_ball_cone_ratio",       (void *)&CP_clear_ball_cone_ratio, V_FLOAT},
    {"clear_ball_max_dist",         (void *)&CP_clear_ball_max_dist, V_FLOAT},
    {"clear_offensive_min_horiz_dist", (void *)&CP_clear_offensive_min_horiz_dist, V_FLOAT},
    {"clear_offensive_min_angle",   (void *)&CP_clear_offensive_min_angle, V_FLOAT},


    {"should_cross_corner_dist",    (void *)&CP_should_cross_corner_dist, V_FLOAT},
    {"should_cross_baseline_buffer",(void *)&CP_should_cross_baseline_buffer, V_FLOAT},
    {"should_move_to_cross_corner_dist", (void *)&CP_should_move_to_cross_corner_dist, V_FLOAT},
    {"cross_pt_x",                  (void *)&CP_cross_pt_x, V_FLOAT},
    {"cross_pt_y",                  (void *)&CP_cross_pt_y, V_FLOAT},
    {"cross_target_vel",            (void *)&CP_cross_target_vel, V_FLOAT},

    {"dont_dribble_to_middle_min_x",(void *)&CP_dont_dribble_to_middle_min_x, V_FLOAT},
    
    {"good_shot_distance",          (void *)&CP_good_shot_distance, V_FLOAT},
    {"shot_distance",               (void *)&CP_shot_distance, V_FLOAT},
    {"cycles_to_kick_buffer",       (void *)&CP_cycles_to_kick_buffer, V_INT},
    {"shot_speed",                  (void *)&CP_shot_speed,    V_FLOAT},
    {"shot_goalie_react_buffer",      (void *)&CP_shot_goalie_react_buffer,      V_INT},
    {"good_shot_goalie_react_buffer", (void *)&CP_good_shot_goalie_react_buffer, V_INT},
    {"better_shot_cyc_diff",        (void *)&CP_better_shot_cyc_diff,            V_INT},

    {"min_clang_ver",(void*)&CP_min_clang_ver,V_INT},
    {"max_clang_ver",(void*)&CP_max_clang_ver,V_INT},


    {"formation",                   (void *)&FP_initial_formation,  V_STRING},
    {"formation_when_losing",       (void *)&FP_formation_when_losing,  V_STRING},
    {"formation_when_losing_lots",  (void *)&FP_formation_when_losing_lots,  V_STRING},
    {"formation_when_winning",      (void *)&FP_formation_when_winning,  V_STRING},
    {"formation_when_tied",         (void *)&FP_formation_when_tied,  V_STRING},
    //add by AI
    {"offside_value",(void*)&CP_offside_value,V_FLOAT},
    {"dist_to_wide_view",(void*)&CP_dist_to_wide_view,V_FLOAT},
    {"use_anti_dribble",(void*)&CP_use_anti_dribble,V_ONOFF},
    {"penalty_mode",(void*)&CP_penalty_mode,V_ONOFF},
    {"midfielders_close_to_defense",(void*)&CP_midfielders_close_to_defense,V_ONOFF},
    //Added by Cool Serg	
    {"team_receive_time",(void*)&CP_team_receive_time,V_INT},
    {"test_pass_angle",(void*)&CP_test_pass_angle,V_FLOAT},
    {"test_pass_velocity",(void*)&CP_test_pass_velocity,V_FLOAT},
    {"test_should_pass",(void*)&CP_test_should_pass,V_ONOFF},
    {"home_change",                 (void *)&FP_initial_hc_method,  V_STRING},
    {"mark_change",                 (void *)&FP_initial_mc_method,  V_STRING},
    {"player_1_pos",                (void *)&FP_initial_player_1_pos,  V_INT},
    {"player_2_pos",                (void *)&FP_initial_player_2_pos,  V_INT},
    {"player_3_pos",                (void *)&FP_initial_player_3_pos,  V_INT},
    {"player_4_pos",                (void *)&FP_initial_player_4_pos,  V_INT},
    {"player_5_pos",                (void *)&FP_initial_player_5_pos,  V_INT},
    {"player_6_pos",                (void *)&FP_initial_player_6_pos,  V_INT},
    {"player_7_pos",                (void *)&FP_initial_player_7_pos,  V_INT},
    {"player_8_pos",                (void *)&FP_initial_player_8_pos,  V_INT},
    {"player_9_pos",                (void *)&FP_initial_player_9_pos,  V_INT},
    {"player_10_pos",               (void *)&FP_initial_player_10_pos, V_INT},
    {"player_11_pos",               (void *)&FP_initial_player_11_pos, V_INT},
    {"goalie_number",               (void *)&FP_goalie_number        , V_INT},

#include "serveroptions.h"

    {"formation_conf",	        (void*)&CP_formation_conf,      V_STRING},
    {"pass_weights_file",	        (void*)&CP_pass_weights_file,      V_STRING},
    {"\0",				NULL, 			       	0}
  } ;

  /* skip command name */
  argv++ ; argc-- ;

  /* first, search option '-file' */
  int i ;
  FILE *fp ;
  for(i = 0 ; i < argc ; i++) {
    if (!strcmp(*(argv + i),"-file")) {
      if ((fp = fopen(*(argv+i+1),"r")) == NULL) {
	cerr << "can't open config file " << *(argv+i+1) << endl ;
	break ;
      }

      char buf[100] ;
      while(fgets(buf,100,fp) != NULL) {
	/* ignore remark line */
	if (buf[0] == '#' || buf[0] == '\n')
	  continue ;

	/* replace from ':' to ' ' */
	char *t = buf ;
	while(*t != NULLCHAR) {
	  if (*t == ':') *t = ' ' ;
	  t++ ;
	}

	int n, p ;
	char com[256] ;
	char onoff[16] ;
	n = sscanf(buf,"%s", com) ;
	if (n < 1) {
	  cerr << "Illegal line : " << buf ;
	  continue ;
	}

	for (p = 0 ; opt[p].vptr != NULL ; p++) {
	  if (strcmp(com, opt[p].optname))
	    continue ;

	  /* match */
	  switch(opt[p].vsize) {
	  case V_INT:
	    n = sscanf(buf, "%s %d", com, (int *)opt[p].vptr) ;
	    break ;

	  case V_STRING:
	    n = sscanf(buf, "%s %s", com, (char *)opt[p].vptr) ;
	    break ;

	  case V_FLOAT:
	    n = sscanf(buf, "%s %f", com, (float *)opt[p].vptr) ;
	    break ;

	  case V_BOOL:
	    n = 2 ;
	    *((Bool *)opt[p].vptr) = TRUE ;
	    break ;

	  case V_ONOFF:
	    n = sscanf(buf, "%s %s", com, onoff) ;
	    if (n == 1) {
	      n = 2;
	      *((Bool *)opt[p].vptr) = TRUE;
	    } else
	      *((Bool *)opt[p].vptr) = (!strcmp(onoff, "on")) ? TRUE :FALSE;
	    break ;
	  }

	  if (n < 2)
	    cerr << "Illegal line (" << com << ") " << endl ;

	  break ;
	}
				
	if (opt[p].vptr == NULL)
	  cerr << "Illegal line (" << com << ") " << endl ;
      }

      fclose(fp) ;
      /* break ; */   // Without this, more than one file can be read
    }
  }

  /* next, analyze command line option */
  int p ;

  while (argc) {
    if (!strcmp(*argv, "-file")) {
      argv += 2 ;
      argc -= 2 ;
      continue ;
    }
			
    for (p = 0 ; opt[p].vptr != NULL ; p++) {
      if (strcmp(*argv + 1, opt[p].optname))
	continue ;

      /* match */
      argv++ ;
      argc-- ;

      switch(opt[p].vsize) {
      case V_INT:
	*((int *)opt[p].vptr) = atoi(*argv) ;
	break ;

      case V_STRING:
	strcpy((char *)opt[p].vptr, *argv) ;
	break ;

      case V_FLOAT:
	*((float *)opt[p].vptr) = atof(*argv) ;
	break ;

      case V_BOOL:
	*((Bool *)opt[p].vptr) = TRUE ;
	argv-- ;
	argc++ ;
	break ;

      case V_ONOFF:
	if (argc > 0 && (*argv)[0] != '-') {	  
	  *((Bool *)opt[p].vptr) = (!strcmp(*argv, "on")) ? TRUE : FALSE ;
	} else {
	  /* if there's nothing specified, then we set it to true */
	  *((Bool *)opt[p].vptr) = TRUE;
	  argv-- ;
	  argc++ ;
	}
	break ;
      }

      break ;
    }

    if (opt[p].vptr == NULL)
      cerr << "Unrecognized Option : " << *argv << endl ;

    argv++ ;
    argc-- ;
  }

  SP_half_time = SP_half_time * 1000 / SP_simulator_step ;
  SP_kickable_area = SP_kickable_margin + SP_ball_size + SP_player_size ;
  SP_hetro_player_type=0;
}


/* explode the line into argc and argv */
void OptionInfo::GetOptions(char* line) 
{
  const int MAXOPT = 100;
  char* argv[MAXOPT];
  int argc = 1; /* executable name */
  char* pc;

  advance_past_space(&line);
  while (*line != 0) {
    pc = line;
    get_token(&line);
    argv[argc] = new char[line-pc+1];
    strncpy(argv[argc], pc, line-pc);
    argv[argc][line-pc] = 0; /* null terminate */
    argc++;
    advance_past_space(&line);
  }

  argv[argc] = NULL;

  GetOptions(argc, argv);

  for (int i = 1; i<argc; i++)
    delete [] argv[i];
}

//////////////////////////////////////////////////////////////////////////
void OptionInfo::ParseParamsFromServer(char *Info,option_t* opt){
  int p;
  Info+=13;

  while (*Info!=')')
    {
      char buf[50];
      int i=0;
      while (*Info==' ' || *Info=='(')  Info++;
      while (*Info!=' ')  buf[i++]=*(Info++);
      buf[i]=0;

      for (p = 0 ; opt[p].vptr != NULL ; p++)
	{
  	
	  if (strcmp(buf, opt[p].optname))
	    continue ;

	  advance_past_space(&Info);
	  switch(opt[p].vsize)
	    {
	    case V_INT:
	      *(int *)opt[p].vptr=get_int (&Info) ;	
	      break ;

	    case V_STRING:
	      char* temp;
	      temp=(char *)opt[p].vptr;
	      while (*Info!=')')  *(temp++)=*(Info++);
	      *temp=0;
	      break ;

	    case V_FLOAT:
	      *(float *)opt[p].vptr=get_float (&Info) ;
	      break ;

	    case V_ONOFF:
	    case V_BOOL:
	
	      *((Bool *)opt[p].vptr) = (get_int (&Info)==1? TRUE : FALSE); ;
	      break ;
	    } //switch
	  break;
	} //for
      if (opt[p].vptr == NULL)
	cout<< "Unknown param from server "<<buf<<endl;
      advance_to (')',&Info);
      Info++;
    } //while	
}
/////////////////////////////////////////////////////////////////////////

void OptionInfo::Parse_Server_Param_message(char *ServerInfo)
{
  option_t opt[] = {
#include "serveroptions.h"
    {"\0",NULL,0}
  };
  ParseParamsFromServer(ServerInfo,opt);
}

//------------------------------------------------------------------------//
void OptionInfo::Parse_Player_Param_message(char *PlayerInfo)
{
  option_t opt[] = {
    {"player_types",(void*)&SP_player_types,V_INT},
    {"pt_max",(void*)&SP_pt_max,V_INT},
    {"random_seed",(void*)&SP_random_seed,V_FLOAT},
    {"subs_max",(void*)&SP_subs_max,V_INT},
    {"dash_power_rate_delta_max",(void*)&SP_dash_power_rate_delta_max,V_FLOAT},
    {"dash_power_rate_delta_min",(void*)&SP_dash_power_rate_delta_min,V_FLOAT},
    {"effort_max_delta_factor",(void*)&SP_effort_max_delta_factor,V_FLOAT},
    {"effort_min_delta_factor",(void*)&SP_effort_min_delta_factor,V_FLOAT},
    {"extra_stamina_delta_max",(void*)&SP_extra_stamina_delta_max,V_FLOAT},
    {"extra_stamina_delta_min",(void*)&SP_extra_stamina_delta_min,V_FLOAT},
    {"inertia_moment_delta_factor",(void*)&SP_inertia_moment_delta_factor,V_FLOAT},
    {"kick_rand_delta_factor",(void*)&SP_kick_rand_delta_factor,V_FLOAT},
    {"kickable_margin_delta_max",(void*)&SP_kickable_margin_delta_max,V_FLOAT},
    {"kickable_margin_delta_min",(void*)&SP_kickable_margin_delta_min,V_FLOAT},
    {"new_dash_power_rate_delta_max",(void*)&SP_new_dash_power_rate_delta_max,V_FLOAT},
    {"new_dash_power_rate_delta_min",(void*)&SP_new_dash_power_rate_delta_min,V_FLOAT},
    {"new_stamina_inc_max_delta_factor",(void*)&SP_new_stamina_inc_max_delta_factor,V_FLOAT},
    {"player_decay_delta_max",(void*)&SP_player_decay_delta_max,V_FLOAT},
    {"player_decay_delta_min",(void*)&SP_player_decay_delta_min,V_FLOAT},
    {"player_size_delta_factor",(void*)&SP_player_size_delta_factor,V_FLOAT},
    {"player_speed_max_delta_max",(void*)&SP_player_speed_max_delta_max,V_FLOAT},
    {"player_speed_max_delta_min",(void*)&SP_player_speed_max_delta_min,V_FLOAT},
    {"stamina_inc_max_delta_factor",(void*)&SP_stamina_inc_max_delta_factor,V_FLOAT},
    {"\0",NULL,0}
  };
  ParseParamsFromServer(PlayerInfo,opt);
}
////////////////////////////////////////////////////////////////////////////////
void OptionInfo::Parse_Player_Type_message(char *PlayerType)
{
  char* res=strstr(PlayerType,"id");
  if(res==0){
    cout<<"Parse_Player_Type_message: i`m not found id"<<endl;
    my_error("Parse_Player_Type_message: i`m not found id");
    return;
  }
  if(SP_hetro_player_type==0){
    SP_hetro_player_type=new HPlayerType[SP_player_types+1];		
    for(int i=0;i<SP_player_types+1;i++){
      HPlayerType hpt;
      hpt.Initialize(SP_player_speed_max,SP_stamina_inc,SP_player_decay,SP_inertia_moment,SP_dash_power_rate,
		     SP_player_size,SP_kickable_margin,SP_kick_rand,0,1.0,SP_effort_min);
      SP_hetro_player_type[i]=hpt;
    }
  }
  res++;//space
  int id=get_int(&res);
  option_t opt[]={
    {"id",(void*)&id,V_INT},//from avoid error msg
    {"player_speed_max",(void*)&SP_hetro_player_type[id].GetPlayerSpeedMax(),V_FLOAT},
    {"stamina_inc_max",(void*)&SP_hetro_player_type[id].GetStaminaIncMax(),V_FLOAT},
    {"player_decay",(void*)&SP_hetro_player_type[id].GetPlayerDecay(),V_FLOAT},
    {"inertia_moment",(void*)&SP_hetro_player_type[id].GetInertiaMoment(),V_FLOAT},
    {"dash_power_rate",(void*)&SP_hetro_player_type[id].GetDashPowerRate(),V_FLOAT},
    {"player_size",(void*)&SP_hetro_player_type[id].GetPlayerSize(),V_FLOAT},
    {"kickable_margin",(void*)&SP_hetro_player_type[id].GetKickableMargin(),V_FLOAT},
    {"kick_rand",(void*)&SP_hetro_player_type[id].GetKickRand(),V_FLOAT},
    {"extra_stamina",(void*)&SP_hetro_player_type[id].GetExtraStamina(),V_FLOAT},
    {"effort_max",(void*)&SP_hetro_player_type[id].GetEffortMax(),V_FLOAT},
    {"effort_min",(void*)&SP_hetro_player_type[id].GetEffortMin(),V_FLOAT},
    {"\0",NULL,0}
  };
  ParseParamsFromServer(PlayerType,opt);
  //set worst (for us) opponent params
  if(SP_hetro_player_type[id].GetPlayerSpeedMax()>SP_hetro_player_type[SP_player_types].GetPlayerSpeedMax())
    SP_hetro_player_type[SP_player_types].GetPlayerSpeedMax()=SP_hetro_player_type[id].GetPlayerSpeedMax();
  if(SP_hetro_player_type[id].GetStaminaIncMax()>SP_hetro_player_type[SP_player_types].GetStaminaIncMax())
    SP_hetro_player_type[SP_player_types].GetStaminaIncMax()=SP_hetro_player_type[id].GetStaminaIncMax();
  if(SP_hetro_player_type[id].GetPlayerDecay()>SP_hetro_player_type[SP_player_types].GetPlayerDecay())
    SP_hetro_player_type[SP_player_types].GetPlayerDecay()=SP_hetro_player_type[id].GetPlayerDecay();
  if(SP_hetro_player_type[id].GetInertiaMoment()<SP_hetro_player_type[SP_player_types].GetInertiaMoment())
    SP_hetro_player_type[SP_player_types].GetInertiaMoment()=SP_hetro_player_type[id].GetInertiaMoment();
  if(SP_hetro_player_type[id].GetDashPowerRate()>SP_hetro_player_type[SP_player_types].GetDashPowerRate())
    SP_hetro_player_type[SP_player_types].GetDashPowerRate()=SP_hetro_player_type[id].GetDashPowerRate();
  if(SP_hetro_player_type[id].GetPlayerSize()>SP_hetro_player_type[SP_player_types].GetPlayerSize())
    SP_hetro_player_type[SP_player_types].GetPlayerSize()=SP_hetro_player_type[id].GetPlayerSize();
  if(SP_hetro_player_type[id].GetKickableMargin()>SP_hetro_player_type[SP_player_types].GetKickableMargin())
    SP_hetro_player_type[SP_player_types].GetKickableMargin()=SP_hetro_player_type[id].GetKickableMargin();
  if(SP_hetro_player_type[id].GetKickRand()<SP_hetro_player_type[SP_player_types].GetKickRand())
    SP_hetro_player_type[SP_player_types].GetKickRand()=SP_hetro_player_type[id].GetKickRand();
  if(SP_hetro_player_type[id].GetExtraStamina()>SP_hetro_player_type[SP_player_types].GetExtraStamina())
    SP_hetro_player_type[SP_player_types].GetExtraStamina()=SP_hetro_player_type[id].GetExtraStamina();
  if(SP_hetro_player_type[id].GetEffortMax()>SP_hetro_player_type[SP_player_types].GetEffortMax())
    SP_hetro_player_type[SP_player_types].GetEffortMax()=SP_hetro_player_type[id].GetEffortMax();
  if(SP_hetro_player_type[id].GetEffortMin()>SP_hetro_player_type[SP_player_types].GetEffortMin())
    SP_hetro_player_type[SP_player_types].GetEffortMin()=SP_hetro_player_type[id].GetEffortMin();
}
