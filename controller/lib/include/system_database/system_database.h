/*
**Author:YasirLiang
**File:system_database.h
**Date Create:18-2-2016
**
*/

#ifndef __SYSTEM_DATABASE_H__
#define __SYSTEM_DATABASE_H__

#include "sqlite3_util.h"
#include "profile_system.h"
#include "matrix_output_input.h"
#include "avdecc_manage.h"

#ifdef __DEBUG__
#define __SYSTEM_DB_DEBUG__
#endif

#define SYSTEM_DB_FILE "system.db" // 系统配置参数数据库

#define SYS_DB_SYSTEM_SET_TABLE "config_system" // 见profile_system.h的thost_system_set结构
#define SYSTEM_CONFIG_COLUMN   "chman_first int,chman_music int,auto_close int,discuss_mode int,speak_limit int,apply_limit int,chman_limitime int,vip_limitime int,spk_limtime int,temp_close int,sign_type int,camara_track int,current_cmr int,menu_language int"

#define SYS_DB_MATRIX_CONTROL_TABLE "system_matrix" // 系统控制矩阵的参数
#define MATRIX_CONTROL_COLUMN   "matrix_type varchar(30),version varchar(30),command_system varchar(30),password varchar(8),backlight varchar(3),serial_on int,all_corresponding int,lock int,bell int,close_all int"

#define SYS_DB_MATRIX_AVSW_TABLE "matrix_av_switch" // 矩阵切换输入对应的输出(av), 非零代表已切换到相应的输出
#define MATRIX_AVSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"

#define SYS_DB_MATRIX_VSW_TABLE "matrix_video_switch"// 矩阵切换输入对应的输出(v), 非零代表已切换到相应的输出
#define MATRIX_VSW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"

#define SYS_DB_MATRIX_ASW_TABLE "matrix_audio_switch"// 矩阵切换输入对应的输出(a), 非零代表已切换到相应的输出
#define MATRIX_ASW_COLUMN   "intput int,output1 int,output2 int,output3 int,output4 int,output5 int,output6 int,output7 int,output8 int,output9 int,output10 int,output11 int,output12 int,output13 int,output14 int,output15 int,output16 int"

#define SYS_DB_TMNL_USER_TABLE "tmnl_user" // 终端信息 
#define TERMINA_USER_COLUMN   "uid int, uname varchar(30) primary key not null, user_type int, registered int,signed int, sys_state int, mic_state int"

#define SYS_DB_TMNL_KEY_VOTE_TABLE "conference_vote" // 终端投票按键键值表
#define SYS_DB_TMNL_KEY_SELECT_TABLE "conference_select" // 终端选举按键键值表
#define SYS_DB_TMNL_KEY_GRADE_TABLE "conference_grade" // 终端评分按键键值表
#define TERMINA_KEY_COLUMN   "uid int, key1 int, key2 int, key3 int, key4 int, key5 int"

#define SYS_DB_TMNL_CAMERA_PRE_TABLE "camera_preset" // 终端的预置位信息
#define CAMERA_PRESET_COLUMN   "uid int, camera_num int, camera_preset_point int"

#define SYS_DB_AVDECC_SET_TABLE "avdecc_set" // 设置avdecc设置参数
#define SYSTEM_AVDECC_SET_COLUMN   "enable_discover int,discover_interval int, poll_descripor int, poll_descripor_interval int,automatic_remove int, remove_interval int"// 时间的单位是ms

typedef struct type_struct_database_handle
{
	void *data_elem;// 操作数据类型
	void *handle_set;// 操作集合
}Sdb_handle;

typedef struct type_matrix_database
{
	char matrix_type[31];// 矩阵型号第31个放"\0"
	char version[31];// 矩阵的软件版本第31个放"\0"
	char command_system[31];// 指令系统第31个放"\0"
	char password[9];// 键盘锁密码 第九个放"\0"
	char backlight[4];// 000组成背光时间第四个放"\0"
	int serial_on;// 是否打开串口信息返回
	int all_corresponding;// 是否一一对应
	int lock;// 是否上锁
	int bell;// 是否打开蜂鸣器
	int close_all;// 是否全部关闭输出
}smatrix_config_entity;

enum matrix_switch_cmd
{
	AV_SWITCH,
	V_SWITCH,
	A_SWITCH,
	SWITCH_CMD_NUM
};

typedef struct type_matrix_db_output
{
	enum matrix_switch_cmd sw_cmd;
	int input;// 输入
	int output_num[MATRIX_OUTPUT_NUM];// 输出 ，非零代表输入已切换到相应的输出
}smatrix_input_out_sw;// 输入切换到对应输出通道

typedef struct database_terminal_user
{
	int uid;
	char uname[31];// 第31个放"\0"
	int user_type;
	int registered;
	int sign_flag;
	int sys_state;
	int mic_state;
}sdb_terminal_user;

enum conference_table_type
{
	CONFERENCE_VOTE,
	CONFERENCE_SELECT,
	CONFERENCE_GRADE,
	CONFERENCE_TYPE_NUM
};

#define KEY_VOTE_NUM 5
typedef struct database_conference_type
{
	int uid;
	int key[KEY_VOTE_NUM];
}sdb_conference_type;

typedef struct database_cmr_preset
{
	int uid;
	int cmr_num;
	int preset_point_num;
}sdb_cmr_preset;

extern smatrix_config_entity gmatrix_record;// 矩阵配置参数
extern smatrix_input_out_sw gmatrix_io_swich_pro;

int system_db_delect_table( char *table_name );
void system_db_table_result_print( int nrow, int ncolumn, char **db_result );

/**=========================开始系统配置数据库表操作======================================**/
int system_db_update_configure_system_table( thost_system_set config_info );
int system_db_queue_configure_system_table( thost_system_set* p_config );
int system_db_add_configure_system_table( thost_system_set config );
/**=========================结束系统配置数据库表操作======================================**/

/**=========================开始矩阵配置数据库表操作(对数据库的操作:增删查改，事物管理并发，查询延时 已测试(20-2-2016))======================================**/
int system_db_add_matrix_config_table( smatrix_config_entity entity ); // 增加一条记录
int system_db_queue_matrix_config_record( smatrix_config_entity *p_entity );// 查询一条记录
const char* system_db_queue_matrix_matrix_type( void );
const char* system_db_queue_matrix_version( void );
const char* system_db_queue_matrix_command_system( void );
char* system_db_queue_matrix_password( void );
const char* system_db_queue_matrix_backlight( void );
int system_db_queue_matrix_serial_on( void );
int system_db_queue_matrix_all_corresponding( void );
int system_db_queue_matrix_lock( void );
int system_db_queue_matrix_bell( void );
int system_db_queue_matrix_close_all( void );
int system_db_update_matrix_config_table( smatrix_config_entity config_info );
int system_db_update_matrix_type( char *p_str, int str_len );
int system_db_update_matrix_version( char *p_str, int str_len );
int system_db_update_matrix_command_system( char *p_str, int str_len );
int system_db_update_matrix_password( char *p_str, int str_len );
int system_db_update_matrix_backlight( char *p_str, int str_len );
int system_db_update_matrix_serial_on( int serial_flags );
int system_db_update_matrix_all_corresponding( int cor_flags );
int system_db_update_matrix_lock( int lock_flag );
int system_db_update_matrix_bell( int bell_flag );
int system_db_update_matrix_close_all( int close_flag );

/**=========================结束矩阵配置数据库表操作(对数据库的操作:增删查改，事物管理并发，查询延时 已测试(20-2-2016))======================================**/

/**=========================开始矩阵切换数据库表操作======================================**/
int system_database_matrix_io_insert( enum matrix_switch_cmd sw_cmd );// 使用前需填充gmatrix_io_swich
int system_database_matrix_io_update( enum matrix_switch_cmd sw_cmd );// 使用前需填充gmatrix_io_swich
int system_database_matrix_io_queue( enum matrix_switch_cmd sw_cmd );
/**=========================结束矩阵切换数据库表操作======================================**/

/**=========================开始终端信息数据库表操作======================================**/
int system_db_tmnluser_info_insert(  sdb_terminal_user user_info );
int system_db_tmnluser_info_queue(  sdb_terminal_user *p_user_info, int uid );
int system_db_tmnluser_info_update(  sdb_terminal_user user_info, int uid );
/**=========================结束终端信息数据库表操作======================================**/

/**=========================开始终端投票数据库表操作======================================**/
int system_db_tmnlkey_info_insert( enum conference_table_type c_type, sdb_conference_type key_info );
int system_db_tmnlkey_info_queue( enum conference_table_type c_type, int uid,sdb_conference_type *p_key_info );
int system_db_tmnlkey_info_update( enum conference_table_type c_type, sdb_conference_type key_info );
/**=========================结束终端投票数据库表操作======================================**/

/**=========================开始终端预置位数据库表操作======================================**/
int system_db_cmrpre_info_insert( sdb_cmr_preset insert_info, int uid );
int system_db_cmrpre_info_queue( sdb_cmr_preset *p_insert_info, int uid );
int system_db_cmrpre_info_update( sdb_cmr_preset insert_info, int uid );
/**=========================结束终端预置位数据库表操作======================================**/

/**=========================开始avdecc管理数据库表操作======================================**/
int system_db_avdecc_info_insert(  tavdecc_manage discover_info, tavdecc_manage descripor_info, tavdecc_manage device_remove );
int system_db_avdecc_info_queue(  tavdecc_manage* p_discover_info, tavdecc_manage* p_descripor_info, tavdecc_manage* p_device_remove );
int system_db_avdecc_info_update( tavdecc_manage discover_info, tavdecc_manage descripor_info, tavdecc_manage device_remove );
/**=========================结束avdecc管理数据库表操作======================================**/

void system_database_init( void );
void system_database_destroy( void );

#endif
