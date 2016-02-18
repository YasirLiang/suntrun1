/*
**Author:YasirLiang
**File:system_database.h
**Date Create:18-2-2016
**
*/

#ifndef __SYSTEM_DATABASE_H__
#define __SYSTEM_DATABASE_H__

#include "sqlite3_util.h"

#define SYSTEM_DB_FILE "system.db" // 系统配置参数数据库

#define SYS_DB_SYSTEM_SET_TABLE "config_system" // 见profile_system.h的thost_system_set结构
#define SYSTEM_CONFIG_COLUMN   "chman_first int,chman_music int,auto_close int,discuss_mode int,speak_limit int,apply_limit int,chman_limitime int,vip_limitime int,spk_limtime int,temp_close int,sign_type int,camara_track int,current_cmr int,menu_language int"

#define SYS_DB_MATRIX_CONTROL_TABLE "system_matrix" // 系统控制矩阵的参数
#define MATRIX_CONTROL_COLUMN   "matrix_type varchar(30),version varchar(30),command_system varchar(30),password varchar(8),backlight int,serial_on int,all_corresponding int,lock int,bell int,close_all int"
#define SYS_DB_MATRIX_AVSW_TABLE "matrix_av_switch" // 矩阵切换输入对应的输出(av), 非零代表已切换到相应的输出
#define MATRIX_AVSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"
#define SYS_DB_MATRIX_VSW_TABLE "matrix_video_switch"// 矩阵切换输入对应的输出(v), 非零代表已切换到相应的输出
#define MATRIX_VSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"
#define SYS_DB_MATRIX_ASW_TABLE "matrix_audio_switch"// 矩阵切换输入对应的输出(a), 非零代表已切换到相应的输出
#define MATRIX_ASW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"


#define SYS_DB_TMNL_USER_TABLE "tmnl_user" // 终端信息
#define TERMINA_USER_COLUMN   "uid int, uname varchar(30) primary key not null, user_type int"
#define SYS_DB_TMNL_CAMERA_PRE_TABLE "camera_preset" // 终端的预置位信息
#define CAMERA_PRESET_COLUMN   "uid int, uname varchar(30) primary key not null, camera_num int, camera_preset_point int"

void system_database_init( void );
void system_database_destroy( void );

#endif
