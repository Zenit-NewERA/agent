/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : MemOption.h
 *
 *    AUTHOR     : Anton Ivanov, Alexei Kritchoun, Sergei Serebyakov
 *
 *    $Revision: 2.5 $
 *
 *    $Id: MemOption.h,v 2.5 2004/04/19 08:00:01 anton Exp $
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
 
/* MemOption.h
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
 *
 */

#ifndef _OPTION_H_
#define _OPTION_H_

#include "types.h"

struct option_t {
  char	optname[50] ;
  void	*vptr ;
  int	vsize ;
};

enum InputType{
  V_INT,
  V_FLOAT,
  V_BOOL,
  V_STRING,	
  V_ONOFF,		
  V_NONE		
};

#define MAX_TEAMNAME_LEN  20
#define MAX_HOST_LEN      50
#define MAX_FILE_LEN      256
#define MAX_FP_LEN        20
#define MAX_TREE_STEM_LEN 50

/* Things to be read at startup that never change */
//add by AI
class HPlayerType{
public:
  HPlayerType():player_speed_max(0.0),stamina_inc_max(0),player_decay(0),inertia_moment(0),dash_power_rate(0),player_size(0),kickable_margin(0),
		kick_rand(0),extra_stamina(0),effort_max(0),effort_min(0){}
  void Initialize(float psm,float sim,float pd,float im,float dpr,float ps,float km,float kr,float es,float emax,float emin){
    player_speed_max=psm;
    stamina_inc_max=sim;
    player_decay=pd;
    inertia_moment=im;
    dash_power_rate=dpr;
    player_size=ps;
    kickable_margin=km;
    kick_rand=kr;
    extra_stamina=es;
    effort_max=emax;
    effort_min=emin;
  }
  void Log();
  float& GetPlayerSpeedMax(){return player_speed_max;}
  float& GetStaminaIncMax(){return stamina_inc_max;}
  float& GetPlayerDecay(){return player_decay;}
  float& GetInertiaMoment(){return inertia_moment;}
  float& GetDashPowerRate(){return dash_power_rate;}
  float& GetPlayerSize(){return player_size;}
  float& GetKickableMargin(){return kickable_margin;}
  float& GetKickRand(){return kick_rand;}
  float& GetExtraStamina(){return extra_stamina;}
  float& GetEffortMax(){return effort_max;}
  float& GetEffortMin(){return effort_min;}
		
private:
  float player_speed_max;
  float stamina_inc_max;
  float player_decay;
  float inertia_moment;
  float dash_power_rate;
  float player_size;
  float kickable_margin;
  float kick_rand;
  float extra_stamina;
  float effort_max;
  float effort_min;
};
//////////////////////////////////////////////
class OptionInfo {
public:
  OptionInfo();
  ~OptionInfo();
  void GetOptions(int, char**);
  void GetOptions(char*);
	
  void ParseParamsFromServer(char* info,option_t* msg);
  void Parse_Server_Param_message(char *ServerInfo);
  void Parse_Player_Param_message(char *PlayerInfo);
  void Parse_Player_Type_message(char *PlayerType);

  float CP_test_shot_y;

  /* Client version params */
  Bool  VP_test_l;
  Bool  VP_test_r;
  Bool  VP_test;
  Bool  VP_train_DT;
  Bool  VP_use_DT;

  /* Client Internal Structure params */
  bool IS_use_native_ACGT;	

  /* Initialize params */
  int   IP_my_score;
  int   IP_their_score;
  int   IP_reconnect;    /* If non-zero, reconnect to that unum */
  /* Client params */
  char  MyTeamName[MAX_TEAMNAME_LEN];
  Bool  CP_goalie;
  Bool  CP_save_log;
  int   CP_save_freq;
  Bool  CP_save_sound_log;
  int   CP_save_sound_freq;
  int   CP_save_action_log_level;
  int   CP_save_action_freq;
  float CP_send_ban_recv_step_factor;
  int   CP_interrupts_per_cycle;
  int   CP_interrupts_left_to_act;
  float CP_max_conf;
  float CP_min_valid_conf;
  float CP_conf_decay;
  float CP_player_conf_decay;
  float CP_ball_conf_decay;
  float CP_max_player_move_factor;  /* multiply by SP_player_speed_max to see how far a player can be 
				       from its last position and still be considered the same player */
  int   CP_max_say_interval;
  float CP_ball_moving_threshold;
  float CP_dodge_angle_buffer;
  float CP_dodge_distance_buffer;
  float CP_dodge_power;
  float CP_dodge_angle;
  char  CP_tree_stem[MAX_TREE_STEM_LEN];
  char  CP_formation_conf[256];
  char 	CP_pass_weights_file[256];
  int   CP_DT_evaluate_interval;
  int   CP_say_tired_interval;
  float CP_tired_buffer;
  Bool  CP_set_plays;
  int   CP_Setplay_Delay;
  int   CP_Setplay_Say_Delay;
  int   CP_Setplay_Max_Delay;
  int   CP_Setplay_Time_Limit;
  float CP_kickable_buffer;
  int   CP_mark_persist_time;
  float CP_track_max_distance;
  float CP_track_min_distance;
  Bool  CP_pull_offsides;
  Bool  CP_pull_offsides_when_winning;
  Bool  CP_spar;
  Bool  CP_mark;
  Bool  CP_communicate;
  int   CP_change_view_for_ball_cycles;
  float CP_defer_kick_to_teammate_buffer;
  float CP_scan_overlap_angle;

  float CP_pull_offside_threshold;
  float CP_pull_offside_buffer;
  
  float CP_ball_forget_angle_buf;
  float CP_player_forget_angle_buf;
  float CP_ball_forget_dist_buf;
  float CP_player_forget_dist_buf;
  
  /* pat added these */
  Bool CP_use_new_position_based_vel;
  Bool CP_stop_on_error;

  /* these parameters affect turnball */
  float CP_max_turn_kick_pow;
  float CP_opt_ctrl_dist;
  float CP_closest_margin;
  float CP_dokick_factor;

  /* these basically affect the way turnball stops */
  float CP_KickTo_err;
  float CP_max_ignore_vel;

  int   CP_kick_time_space;
  float CP_max_est_err;
  float CP_holdball_kickable_buffer;
  int   CP_stop_ball_power;
  int   CP_possessor_intercept_space;
  int   CP_can_keep_ball_cycle_buffer;
  
  float CP_hard_kick_dist_buffer;
  int   CP_max_hard_kick_angle_err;
  /* angle off perpendicualr to start ball for hardest kick */
  int   CP_hardest_kick_ball_ang; 
  float CP_hardest_kick_ball_dist; 
  int   CP_hardest_kick_player_ang;
  float CP_max_dash_help_kick_angle;
  
  int   CP_max_go_to_point_angle_err;
  int   CP_max_int_lookahead;
  float CP_intercept_close_dist;
  int   CP_intercept_step;
  int   CP_my_intercept_step;
  int   CP_intercept_aim_ahead;
  int   CP_no_turn_max_cyc_diff; /* used for normal interception */
  float CP_no_turn_max_dist_diff; /* used for ball_path intercept */
  float CP_turnball_opp_worry_dist; 
  float CP_collision_buffer;
  float CP_behind_angle;
  int   CP_time_for_full_rotation;
  float CP_ball_vel_invalidation_factor;

  float CP_move_imp_1v1_initial;
  float CP_move_imp_1v1_inc;
  float CP_move_imp_1v1_threshold;
  float CP_at_point_buffer;
  float CP_overrun_dist;
  float CP_def_block_dist;
  float CP_def_block_dist_ratio;
  float CP_overrun_buffer;
  float CP_breakaway_buffer;
  float CP_our_breakaway_kickable_buffer;  
  float CP_their_breakaway_front_kickable_buffer;  
  float CP_their_breakaway_back_kickable_buffer;  
  float CP_goalie_breakaway_kickable_buffer;  
  
  float CP_breakaway_approach_x;
  float CP_breakaway_approach_y;
  int   CP_breakaway_targ_valid_time;
  int   CP_breakaway_min_goalie_steal_time;
  int   CP_breakaway_kick_run_min_cycles;
  int   CP_breakaway_kick_run_max_cycles;
  float CP_their_breakaway_min_cone_dist_wid;
  float CP_our_breakaway_min_cone_dist_wid;
  float CP_breakaway_middle_buffer;
  float CP_breakaway_kick_run_worry_dist;
  int   CP_breakaway_mode; // used to test diff breakaway styles
  
  float CP_beat_offsides_buffer;
  float CP_beat_offsides_threshold;
  float CP_beat_offsides_max_x;
  float CP_congestion_epsilon;
  float CP_back_pass_opponent_buffer;
  float CP_back_pass_offside_buffer;
  float CP_min_less_congested_pass_dist;
  
  float CP_cycles_to_kick;

  /* parameters for moving to a standing ball */
  float CP_static_kick_dist_err;
  float CP_static_kick_ang_err;

  float CP_goalie_baseline_buffer;
  float CP_goalie_scan_angle_err;
  float CP_goalie_at_point_buffer;
  float CP_goalie_vis_angle_err;
  float CP_goalie_max_shot_distance;
  float CP_goalie_min_pos_dist;
  float CP_goalie_max_pos_dist;
  float CP_goalie_max_forward_percent;
  float CP_goalie_ball_ang_for_corner;  
  float CP_goalie_max_come_out_dist;
  float CP_goalie_ball_dist_for_corner;
  float CP_goalie_ball_dist_for_center;
  float CP_goalie_free_kick_dist;
  float CP_goalie_go_to_ball_cone_ratio;
  int   CP_goalie_warn_space;
  Bool  CP_goalie_comes_out;
  int   CP_goalie_catch_wait_time;
  float CP_goalie_opponent_dist_to_block;
  float CP_goalie_position_weight_dist;
  int   CP_goalie_narrow_sideline_cyc;
  float CP_goalie_no_buffer_dist;
  
  float CP_clear_ball_ang_step;
  float CP_clear_ball_cone_ratio;
  float CP_clear_ball_max_dist;
  float CP_clear_offensive_min_horiz_dist;
  float CP_clear_offensive_min_angle;

  float CP_should_cross_corner_dist;
  float CP_should_cross_baseline_buffer;
  float CP_should_move_to_cross_corner_dist;
  float CP_cross_pt_x;
  float CP_cross_pt_y;
  float CP_cross_target_vel;

  float CP_dont_dribble_to_middle_min_x;
  
  float CP_good_shot_distance;
  float CP_shot_distance;
  int   CP_cycles_to_kick_buffer;
  float CP_shot_speed;
  int CP_shot_goalie_react_buffer;
  int CP_good_shot_goalie_react_buffer;
  int CP_better_shot_cyc_diff;

  //add by AI
  float CP_offside_value;
  float CP_dist_to_wide_view;
  Bool CP_use_anti_dribble;
  Bool CP_penalty_mode;
  Bool CP_midfielders_close_to_defense;

  int CP_min_clang_ver;
  int CP_max_clang_ver;
  //from trainer
  float CP_lp_power;
  float CP_lp_angle;
  int CP_last_reciev;

  //Added by Cool Serg
  int CP_team_receive_time;

  float CP_test_pass_angle;
  float CP_test_pass_velocity;
  Bool  CP_test_should_pass;


  /* Formation params */
  char  FP_initial_formation[MAX_FP_LEN];
  char  FP_formation_when_tied[MAX_FP_LEN];
  char  FP_formation_when_losing[MAX_FP_LEN];
  char  FP_formation_when_losing_lots[MAX_FP_LEN];
  char  FP_formation_when_winning[MAX_FP_LEN];
  char  FP_initial_hc_method[MAX_FP_LEN];
  char  FP_initial_mc_method[MAX_FP_LEN];
  int   FP_initial_player_1_pos;
  int   FP_initial_player_2_pos;
  int   FP_initial_player_3_pos;
  int   FP_initial_player_4_pos;
  int   FP_initial_player_5_pos;
  int   FP_initial_player_6_pos;
  int   FP_initial_player_7_pos;
  int   FP_initial_player_8_pos;
  int   FP_initial_player_9_pos;
  int   FP_initial_player_10_pos;
  int   FP_initial_player_11_pos;
  int   FP_goalie_number;

  /* Server params */
  float SP_pitch_length;
  float SP_pitch_width;
  float SP_pitch_margin;
  float SP_penalty_area_length;
  float SP_penalty_area_width;
  float SP_goal_area_length;
  float SP_goal_area_width;
  float SP_penalty_spot_dist;
  float SP_corner_arc_r;
  float SP_free_kick_buffer;
  int   SP_after_goal_wait;
  float SP_feel_distance;
  int   SP_num_lines;
  int   SP_num_markers;
  float SP_unum_far_length;
  float SP_unum_too_far_length;
  float SP_team_far_length;
  float SP_team_too_far_length;

  float SP_version;
  int   SP_team_size;
  int   SP_half;
  char  SP_host[MAX_HOST_LEN];
  float SP_goal_width;       
  float SP_player_size;	
  float SP_player_decay;    	
  float SP_player_rand;	
  float SP_player_weight;	
  float SP_player_speed_max;	
  float SP_stamina_max;	
  float SP_stamina_inc;	
  float SP_recover_dec_thr;	
  float SP_recover_min; 	
  float SP_recover_dec; 	
  float SP_effort_dec_thr;	
  float SP_effort_min;	
  float SP_effort_dec;	
  float SP_effort_inc_thr;	
  float SP_effort_inc;	
  float SP_ball_size;		
  float SP_ball_decay;	
  float SP_ball_rand;		
  float SP_ball_weight;	
  float SP_ball_speed_max;	
  float SP_dash_power_rate;	
  float SP_kick_power_rate;	
  float SP_kickable_margin;	
  float SP_kickable_area;	
  float SP_catch_prob;	
  float SP_catch_area_l;	
  float SP_catch_area_w;	
  float SP_max_power;		
  float SP_min_power;		
  float SP_max_moment;		
  float SP_min_moment;		
  float SP_max_neck_angle;
  float SP_min_neck_angle;
  float SP_max_neck_moment;
  float SP_min_neck_moment;
  float SP_visible_angle;	
  float SP_visible_dist;	
  float SP_audio_cut_dist;	
  float SP_dist_qstep;	
  float SP_land_qstep;	
  float SP_ckmargin;	
  float SP_wind_dir;	
  float SP_wind_force;	
  float SP_wind_rand;	
  Bool SP_wind_none;
  Bool SP_wind_random;
  int SP_half_time;
  int SP_port;
  int SP_coach_port;
  int SP_olcoach_port;
  int SP_simulator_step;
  int SP_send_step;
  int SP_recv_step;
  int SP_say_msg_size;
  int SP_hear_max;
  int SP_hear_inc;
  int SP_hear_decay;
  int SP_catch_ban_cycle;
  Bool SP_coach_mode;
  Bool SP_coach_w_referee_mode;
  int SP_say_coach_cnt_max;	
  int SP_say_coach_msg_size;
  int SP_send_vi_step;
  int SP_look_step;
  float SP_player_accel_max;
  Bool SP_log_times;
  int SP_goalie_max_moves;
  float SP_baccel_max;
  float SP_kick_rand;
  //new messages from trainer
  int SP_clang_win_size;
  int SP_clang_define_win;
  int SP_clang_meta_win;
  int SP_clang_advice_win;
  int SP_clang_info_win;
  int SP_clang_mess_delay;
  int SP_clang_mess_per_cycle;
  //new messages
  float SP_slowness_on_top_for_left_team;
  float SP_slowness_on_top_for_right_team;
  Bool SP_synch_mode;
  int SP_synch_micro_sleep;
  float SP_ball_accel_max;
  int SP_drop_ball_time;

  Bool SP_use_offside;
  Bool SP_forbid_kickoff_offside;
  char SP_logfile[MAX_FILE_LEN];
  char SP_recfile[MAX_FILE_LEN];
  Bool SP_rec_log;
  int  SP_rec_ver;
  char SP_replay[MAX_FILE_LEN];
  Bool SP_verbose;
  Bool SP_send_log;
  float SP_offside_area;
  float SP_inertia_moment;
  int   SP_sense_body_step;
  float SP_offside_kick_margin;
  Bool  SP_record_messages;
  Bool SP_game_logging;
  Bool SP_send_comms;
  Bool SP_text_logging;
  int  SP_game_log_version;
  char SP_text_log_dir[256];
  char SP_game_log_dir[256];
  char SP_text_log_fixed_name[256];
  char SP_game_log_fixed_name[256];
  Bool SP_text_log_fixed;
  Bool SP_game_log_fixed;
  Bool SP_text_log_dated;
  Bool SP_game_log_dated;
  char SP_log_date_format[256];
  int SP_text_log_compression;
  int SP_game_log_compression;

  float SP_recover_init;
  float SP_effort_init;
  float SP_team_actuator_noise;
  float SP_prand_factor_l;
  float SP_prand_factor_r;
  float SP_kick_rand_factor_l;
  float SP_kick_rand_factor_r ;
  float SP_control_radius;
  float SP_control_radius_width;
  float SP_quantize_step_dir;
  float SP_quantize_step_dist_team_l;
  float SP_quantize_step_dist_team_r;
  float SP_quantize_step_dist_l_team_r;
  float SP_quantize_step_dist_l_team_l;
  float SP_quantize_step_dir_team_l;
  float SP_quantize_step_dir_team_r;
  float SP_wind_ang;
	
  float SP_lcm_step;
  int SP_old_coach_hear;
  float SP_slow_down_factor;
  float SP_synch_offset;
  float SP_start_goal_l;
  float SP_start_goal_r;
  float SP_fullstate_l;
  float SP_fullstate_r;
  float SP_profile;
  int SP_point_to_ban;
  int SP_point_to_duration;
  float SP_tackle_dist;
  float SP_tackle_back_dist;
  float SP_tackle_width;
  int SP_tackle_exponent;
  int SP_tackle_cycles;
	
  int SP_clang_rule_win;
  int SP_clang_del_win;
  int SP_freeform_send_period;
  int SP_freeform_wait_period;
  int SP_max_goal_kicks;
  //landmark_file
  Bool SP_back_passes;
  Bool SP_free_kick_faults;
  Bool SP_proper_goal_kicks;
  float SP_stopped_ball_vel;
  float SP_tackle_power_rate;
  char SP_landmark_file[256];
  //AI: new at 9.0 and higher
  int SP_connect_wait;
  int SP_game_over_wait;
  int SP_keepaway_start;
  int SP_kick_off_wait;
  int SP_nr_extra_halfs;
  int SP_nr_normal_halfs;
  int SP_pen_before_setup_wait;
  int SP_pen_setup_wait;
  int SP_pen_ready_wait;
  int SP_pen_taken_wait;
  int SP_pen_nr_kicks;
  int SP_pen_max_extra_kicks;
  char SP_keepaway_log_dir[256];
  char SP_keepaway_log_fixed_name[256];
  char SP_team_l_start[256];
  char SP_team_r_start[256];
  Bool SP_auto_mode;
  Bool SP_keepaway;
  float SP_keepaway_width;
  float SP_keepaway_length;
  Bool SP_keepaway_logging;
  Bool SP_keepaway_log_fixed;
  Bool SP_keepaway_log_dated;
  Bool SP_pen_allow_mult_kicks;
  Bool SP_pen_random_winner;
  Bool SP_penalty_shoot_outs;
  float SP_pen_dist_x;
  char SP_module_dir[256];
  Bool SP_pen_coach_moves_players;
  float SP_pen_max_goalie_dist_x;//AI: parametres for hetro players
  int SP_player_types;
  int SP_pt_max;
  float SP_random_seed;
  int SP_subs_max;//maximum of substitution
  float SP_dash_power_rate_delta_max;
  float SP_dash_power_rate_delta_min;
  float SP_effort_max_delta_factor;
  float SP_effort_min_delta_factor;
  float SP_extra_stamina_delta_max;
  float SP_extra_stamina_delta_min;
  float SP_inertia_moment_delta_factor;
  float SP_kick_rand_delta_factor;
  float SP_kickable_margin_delta_max;
  float SP_kickable_margin_delta_min;
  float SP_new_dash_power_rate_delta_max;
  float SP_new_dash_power_rate_delta_min;
  float SP_new_stamina_inc_max_delta_factor;
  float SP_player_decay_delta_max;
  float SP_player_decay_delta_min;
  float SP_player_size_delta_factor;
  float SP_player_speed_max_delta_max;
  float SP_player_speed_max_delta_min;
  float SP_stamina_inc_max_delta_factor;
	
  HPlayerType* SP_hetro_player_type;

};

#endif
