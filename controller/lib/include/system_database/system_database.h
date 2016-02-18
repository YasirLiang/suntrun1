/*
**Author:YasirLiang
**File:system_database.h
**Date Create:18-2-2016
**
*/

#ifndef __SYSTEM_DATABASE_H__
#define __SYSTEM_DATABASE_H__

#include "sqlite3_util.h"

#define SYSTEM_DB_FILE "system.db" // ϵͳ���ò������ݿ�

#define SYS_DB_SYSTEM_SET_TABLE "config_system" // ��profile_system.h��thost_system_set�ṹ
#define SYSTEM_CONFIG_COLUMN   "chman_first int,chman_music int,auto_close int,discuss_mode int,speak_limit int,apply_limit int,chman_limitime int,vip_limitime int,spk_limtime int,temp_close int,sign_type int,camara_track int,current_cmr int,menu_language int"

#define SYS_DB_MATRIX_CONTROL_TABLE "system_matrix" // ϵͳ���ƾ���Ĳ���
#define MATRIX_CONTROL_COLUMN   "matrix_type varchar(30),version varchar(30),command_system varchar(30),password varchar(8),backlight int,serial_on int,all_corresponding int,lock int,bell int,close_all int"
#define SYS_DB_MATRIX_AVSW_TABLE "matrix_av_switch" // �����л������Ӧ�����(av), ����������л�����Ӧ�����
#define MATRIX_AVSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"
#define SYS_DB_MATRIX_VSW_TABLE "matrix_video_switch"// �����л������Ӧ�����(v), ����������л�����Ӧ�����
#define MATRIX_VSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"
#define SYS_DB_MATRIX_ASW_TABLE "matrix_audio_switch"// �����л������Ӧ�����(a), ����������л�����Ӧ�����
#define MATRIX_ASW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"


#define SYS_DB_TMNL_USER_TABLE "tmnl_user" // �ն���Ϣ
#define TERMINA_USER_COLUMN   "uid int, uname varchar(30) primary key not null, user_type int"
#define SYS_DB_TMNL_CAMERA_PRE_TABLE "camera_preset" // �ն˵�Ԥ��λ��Ϣ
#define CAMERA_PRESET_COLUMN   "uid int, uname varchar(30) primary key not null, camera_num int, camera_preset_point int"

void system_database_init( void );
void system_database_destroy( void );

#endif
