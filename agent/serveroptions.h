/*************************************************************************
 *
 *    DESCRIPTION :    
 *
 *    FILE 	 : serveroptions.h
 *
 *    AUTHOR     : Alexei Kritchoun, Anton Ivanov
 *
 *    $Revision: 2.4 $
 *
 *    $Id: serveroptions.h,v 2.4 2004/03/31 07:35:31 anton Exp $
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

/*(this file is to include into server_param parsing functions)*/
{"version",        		(void *)&SP_version,          V_FLOAT},
{"team_size",        		(void *)&SP_team_size,          V_INT},
{"half",        		(void *)&SP_half,          V_INT},
{"host",             		(void *)&SP_host,               V_STRING},
{"goal_width",   		(void *)&SP_goal_width,         V_FLOAT},
{"player_size",   		(void *)&SP_player_size,	V_FLOAT},
{"player_decay",   	        (void *)&SP_player_decay,    	V_FLOAT},
{"player_rand",   		(void *)&SP_player_rand,	V_FLOAT},
{"player_weight",   	        (void *)&SP_player_weight,	V_FLOAT},
{"player_speed_max",            (void *)&SP_player_speed_max,	V_FLOAT},
{"stamina_max",   		(void *)&SP_stamina_max,	V_FLOAT},
{"stamina_inc_max",   	        (void *)&SP_stamina_inc,	V_FLOAT},
{"recover_dec_thr",   	        (void *)&SP_recover_dec_thr,	V_FLOAT},
{"recover_min",   		(void *)&SP_recover_min, 	V_FLOAT},
{"recover_dec",   		(void *)&SP_recover_dec, 	V_FLOAT},
{"effort_dec_thr",   	        (void *)&SP_effort_dec_thr,	V_FLOAT},
{"effort_min",   		(void *)&SP_effort_min, 	V_FLOAT},
{"effort_dec",   		(void *)&SP_effort_dec, 	V_FLOAT},
{"effort_inc_thr",   	        (void *)&SP_effort_inc_thr,	V_FLOAT},
{"effort_inc",   		(void *)&SP_effort_inc,	        V_FLOAT},
{"ball_size",   		(void *)&SP_ball_size,		V_FLOAT},
{"ball_decay",   		(void *)&SP_ball_decay, 	V_FLOAT},
{"ball_rand",   		(void *)&SP_ball_rand,		V_FLOAT},
{"ball_weight",   		(void *)&SP_ball_weight,	V_FLOAT},
{"ball_speed_max",   	        (void *)&SP_ball_speed_max,	V_FLOAT},
{"dash_power_rate",   	        (void *)&SP_dash_power_rate,	V_FLOAT},
{"kick_power_rate",   	        (void *)&SP_kick_power_rate,	V_FLOAT},
{"kickable_margin",   	        (void *)&SP_kickable_margin,	V_FLOAT},
{"catch_probability",   	(void *)&SP_catch_prob, 	V_FLOAT},
{"catchable_area_l",            (void *)&SP_catch_area_l,	V_FLOAT},
{"catchable_area_w",            (void *)&SP_catch_area_w,	V_FLOAT},
{"maxpower",   		        (void *)&SP_max_power,		V_FLOAT},
{"minpower",   		        (void *)&SP_min_power,		V_FLOAT},
{"maxmoment",   		(void *)&SP_max_moment,		V_FLOAT},
{"minmoment",   		(void *)&SP_min_moment,		V_FLOAT},
{"maxneckang",		(void *)&SP_max_neck_angle,		V_FLOAT},
{"minneckang",		(void *)&SP_min_neck_angle,		V_FLOAT},
{"maxneckmoment",		(void *)&SP_max_neck_moment,		V_FLOAT},
{"minneckmoment",		(void *)&SP_min_neck_moment,		V_FLOAT},
{"visible_angle",   	        (void *)&SP_visible_angle,	V_FLOAT},
{"visible_distance",            (void *)&SP_visible_dist,	V_FLOAT},
{"audio_cut_dist",   	        (void *)&SP_audio_cut_dist,	V_FLOAT},
{"quantize_step",   	        (void *)&SP_dist_qstep, 	V_FLOAT},
{"quantize_step_l",   	        (void *)&SP_land_qstep, 	V_FLOAT},
{"ckick_margin",   	        (void *)&SP_ckmargin,   	V_FLOAT},
{"wind_dir",   		        (void *)&SP_wind_dir,   	V_FLOAT},
{"wind_force",   		(void *)&SP_wind_force,  	V_FLOAT},
{"wind_rand",   		(void *)&SP_wind_rand,  	V_FLOAT},
{"wind_none",   		(void *)&SP_wind_none,  	V_ONOFF},
{"wind_random",   		(void *)&SP_wind_random,	V_ONOFF},
{"half_time",   		(void *)&SP_half_time,  	V_INT},
{"port",   			(void *)&SP_port,       	V_INT},
{"coach_port",   		(void *)&SP_coach_port,  	V_INT},
{"olcoach_port",	        (void *)&SP_olcoach_port,	V_INT},
{"simulator_step",   	        (void *)&SP_simulator_step,	V_INT},
{"send_step",   		(void *)&SP_send_step,  	V_INT},
{"recv_step",   		(void *)&SP_recv_step,  	V_INT},
{"say_msg_size",   	        (void *)&SP_say_msg_size,	V_INT},
{"hear_max",   		        (void *)&SP_hear_max,   	V_INT},
{"hear_inc",   		        (void *)&SP_hear_inc,   	V_INT},
{"hear_decay",   		(void *)&SP_hear_decay,  	V_INT},
{"catch_ban_cycle",             (void *)&SP_catch_ban_cycle,	V_INT},
{"coach",   			(void *)&SP_coach_mode,  	V_ONOFF},
{"coach_w_referee",   	        (void *)&SP_coach_w_referee_mode,V_ONOFF},
{"say_coach_cnt_max",               (void *)&SP_say_coach_cnt_max,V_INT},
{"say_coach_msg_size",          (void *)&SP_say_coach_msg_size,V_INT},
{"send_vi_step",                (void *)&SP_send_vi_step, V_INT},

{"look_step",                       (void *)&SP_look_step,V_INT},
{"use_offside",   		(void *)&SP_use_offside,	V_ONOFF},
{"forbid_kick_off_offside",     (void *)&SP_forbid_kickoff_offside, V_ONOFF},
{"log_file",		        (void *)&SP_logfile,		V_STRING},
{"record",			(void *)&SP_recfile,		V_STRING},
{"record_log",		        (void *)&SP_rec_log,		V_ONOFF},
{"record_version",	        (void *)&SP_rec_ver,		V_INT},
{"send_log",	        (void *)&SP_send_log,		V_ONOFF},
{"replay",			(void *)&SP_replay,		V_STRING},
{"verbose",                     (void *)&SP_verbose,		V_ONOFF},
{"offside_active_area_size",    (void *)&SP_offside_area,	V_FLOAT},
{"inertia_moment",      	(void *)&SP_inertia_moment,     V_FLOAT},
{"sense_body_step",      	(void *)&SP_sense_body_step,     V_INT},
{"offside_kick_margin",     (void *)&SP_offside_kick_margin,   V_FLOAT},
{"record_messages",         (void *)&SP_record_messages,     V_ONOFF},
{"clang_win_size",          (void *)&SP_clang_win_size,     V_INT},
{"clang_define_win",        (void*)&SP_clang_define_win,    V_INT},
{"clang_meta_win",          (void*)&SP_clang_meta_win,      V_INT},
{"clang_advice_win",        (void*)&SP_clang_advice_win,    V_INT},
{"clang_info_win",          (void*)&SP_clang_info_win,      V_INT},
{"clang_mess_delay",        (void*)&SP_clang_mess_delay,    V_INT},
{"clang_mess_per_cycle",    (void*)&SP_clang_mess_per_cycle,V_INT},
{"player_accel_max",        (void*)&SP_player_accel_max,    V_FLOAT},
{"log_times",               (void*)&SP_log_times,           V_ONOFF},
{"goalie_max_moves",        (void*)&SP_goalie_max_moves,    V_INT},
{"baccel_max",              (void*)&SP_baccel_max,          V_FLOAT},
{"kick_rand",               (void*)&SP_kick_rand,           V_FLOAT},
{"slowness_on_top_for_left_team", (void*)&SP_slowness_on_top_for_left_team, V_FLOAT},
{"slowness_on_top_for_right_team", (void*)&SP_slowness_on_top_for_right_team, V_FLOAT},
{"synch_mode", 		(void*)&SP_synch_mode, 		V_ONOFF},
{"synch_micro_sleep",	(void*)&SP_synch_micro_sleep,	V_INT},
{"ball_accel_max", 		(void*)&SP_ball_accel_max, 	V_FLOAT},
{"drop_ball_time", 		(void*)&SP_drop_ball_time, 	V_INT},
{"game_logging",		(void*)&SP_game_logging,	V_ONOFF},
{"send_comms",		(void*)&SP_send_comms,		V_ONOFF},
{"text_logging",		(void*)&SP_text_logging,	V_ONOFF},
{"game_log_version",	(void*)&SP_game_log_version,	V_INT},
{"text_log_dir",		(void*)&SP_text_log_dir,	V_STRING},
{"game_log_dir", 		(void*)&SP_game_log_dir,	V_STRING},
{"text_log_fixed_name",	(void*)&SP_text_log_fixed_name,	V_STRING},
{"game_log_fixed_name", 	(void*)&SP_game_log_fixed_name,	V_STRING},
{"text_log_fixed", 		(void*)&SP_text_log_fixed,	V_ONOFF},
{"game_log_fixed",		(void*)&SP_game_log_fixed,	V_ONOFF},
{"text_log_dated",		(void*)&SP_text_log_dated,	V_ONOFF},
{"game_log_dated",		(void*)&SP_game_log_dated,	V_ONOFF},
{"log_date_format",		(void*)&SP_log_date_format,	V_STRING},
{"text_log_compression",	(void*)&SP_text_log_compression,V_INT},
{"game_log_compression",	(void*)&SP_game_log_compression,V_INT},
{"recover_init",		(void*)&SP_recover_init	, V_FLOAT},
{"effort_init",			(void*)&SP_effort_init	, V_FLOAT},
{"team_actuator_noise",	(void*)&SP_team_actuator_noise, V_FLOAT},
{"prand_factor_l",		(void*)&SP_prand_factor_l	,V_FLOAT },
{"prand_factor_r",		(void*)&SP_prand_factor_r	,V_FLOAT },
{"kick_rand_factor_l",	(void*)&SP_kick_rand_factor_l	,V_FLOAT },
{"kick_rand_factor_r",	(void*)&SP_kick_rand_factor_r	,V_FLOAT },
{"control_radius",		(void*)&SP_control_radius	,V_FLOAT },
{"control_radius_width",(void*)&SP_control_radius_width	,V_FLOAT },
{"quantize_step_dir",	(void*)&SP_quantize_step_dir	,V_FLOAT },
{"quantize_step_dist_team_l",(void*)&SP_quantize_step_dist_team_l,V_FLOAT },
{"quantize_step_dist_team_r",(void*)&SP_quantize_step_dist_team_r,V_FLOAT },
{"quantize_step_dist_l_team_l",	(void*)&SP_quantize_step_dist_l_team_l	,V_FLOAT },
{"quantize_step_dist_l_team_r",	(void*)&SP_quantize_step_dist_l_team_r	,V_FLOAT },
{"quantize_step_dir_team_l",	(void*)&SP_quantize_step_dir_team_l	,V_FLOAT },
{"quantize_step_dir_team_r",	(void*)&SP_quantize_step_dir_team_r	,V_FLOAT },
{"wind_ang",			(void*)&SP_wind_ang	,V_FLOAT },
{"kickable_area",		(void*)&SP_kickable_area,V_FLOAT },
{"lcm_step",			(void*)&SP_lcm_step,V_FLOAT },
{"old_coach_hear",		(void*)&SP_old_coach_hear	,V_INT },
{"slow_down_factor",	(void*)&SP_slow_down_factor,V_FLOAT },
{"synch_offset",	(void*)&SP_synch_offset,V_FLOAT },
{"start_goal_l",	(void*)&SP_start_goal_l,V_FLOAT },
{"start_goal_r",	(void*)&SP_start_goal_r,V_FLOAT },
{"fullstate_l",		(void*)&SP_fullstate_l,V_FLOAT },
{"fullstate_r",		(void*)&SP_fullstate_r,V_FLOAT },
{"profile",			(void*)&SP_profile,V_FLOAT },
{"point_to_ban",	(void*)&SP_point_to_ban,V_FLOAT },
{"point_to_duration",	(void*)&SP_point_to_duration,V_INT },
{"tackle_dist",		(void*)&SP_tackle_dist,V_FLOAT },
{"tackle_back_dist",(void*)&SP_tackle_back_dist,V_FLOAT },
{"tackle_width",	(void*)&SP_tackle_width,V_FLOAT },
{"tackle_exponent",	(void*)&SP_tackle_exponent,V_INT },
{"tackle_cycles",	(void*)&SP_tackle_cycles,V_INT },
{"clang_rule_win",(void*)&SP_clang_rule_win,V_INT},
{"clang_del_win",(void*)&SP_clang_del_win,V_INT},
{"freeform_send_period",(void*)&SP_freeform_send_period,V_INT},
{"freeform_wait_period", (void*)&SP_freeform_wait_period,V_INT},
{"max_goal_kicks",(void*)&SP_max_goal_kicks,V_INT},
{"back_passes",(void*)&SP_back_passes,V_ONOFF},
{"free_kick_faults",(void*)&SP_free_kick_faults,V_ONOFF},
{"proper_goal_kicks",(void*)&SP_proper_goal_kicks,V_ONOFF},
{"stopped_ball_vel",(void*)&SP_stopped_ball_vel,V_FLOAT},
{"tackle_power_rate",(void*)&SP_tackle_power_rate,V_FLOAT},
{"landmark_file",(void*)&SP_landmark_file,V_STRING},
{"connect_wait",(void*)&SP_connect_wait,V_INT},
{"game_over_wait",(void*)&SP_game_over_wait,V_INT},
{"keepaway_start",(void*)&SP_keepaway_start,V_INT},
{"kick_off_wait",(void*)&SP_kick_off_wait,V_INT},
{"nr_extra_halfs",(void*)&SP_nr_extra_halfs,V_INT},
{"nr_normal_halfs",(void*)&SP_nr_normal_halfs,V_INT},
{"pen_before_setup_wait",(void*)&SP_pen_before_setup_wait,V_INT},
{"pen_setup_wait",(void*)&SP_pen_setup_wait,V_INT},
{"pen_ready_wait",(void*)&SP_pen_ready_wait,V_INT},
{"pen_taken_wait",(void*)&SP_pen_taken_wait,V_INT},
{"pen_nr_kicks",(void*)&SP_pen_nr_kicks,V_INT},
{"pen_max_extra_kicks",(void*)&SP_pen_max_extra_kicks,V_INT},
{"keepaway_log_dir",(void*)&SP_keepaway_log_dir,V_STRING},
{"keepaway_log_fixed_name",(void*)&SP_keepaway_log_fixed_name,V_STRING},
{"team_l_start",(void*)&SP_team_l_start,V_STRING},
{"team_r_start",(void*)&SP_team_r_start,V_STRING},
{"auto_mode",(void*)&SP_auto_mode,V_ONOFF},
{"keepaway",(void*)&SP_keepaway,V_ONOFF},
{"keepaway_logging",(void*)&SP_keepaway_logging,V_ONOFF},
{"keepaway_log_fixed",(void*)&SP_keepaway_log_fixed,V_ONOFF},
{"keepaway_log_dated",(void*)&SP_keepaway_log_dated,V_ONOFF},
{"pen_allow_mult_kicks",(void*)&SP_pen_allow_mult_kicks,V_ONOFF},
{"pen_random_winner",(void*)&SP_pen_random_winner,V_ONOFF},
{"penalty_shoot_outs",(void*)&SP_penalty_shoot_outs,V_ONOFF},
{"keepaway_width",(void*)&SP_keepaway_width,V_FLOAT},
{"keepaway_length",(void*)&SP_keepaway_length,V_FLOAT},
{"pen_dist_x",(void*)&SP_pen_dist_x,V_FLOAT},
{"pen_max_goalie_dist_x",(void*)&SP_pen_max_goalie_dist_x,V_FLOAT},
{"module_dir",(void*)&SP_module_dir,V_STRING},
{"pen_coach_moves_players",(void*)&SP_pen_coach_moves_players,V_ONOFF},