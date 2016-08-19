/*
**Author:YasirLiang
**File:system_database.c
**Date Create:18-2-2016
**
*/

#include "system_database.h"
#include "data_common_pro.h"

static sqlite3* gsystem_db = NULL;
static char gsql[SQL_STRING_LEN] = {'\0'};
smatrix_config_entity gmatrix_record;// 矩阵配置参数
smatrix_input_out_sw gmatrix_io_swich_pro;
smatrix_input_out_sw gmatrix_io_sw_array[SWITCH_CMD_NUM];

/*对数据库的所有表数据记录*/
int system_db_delect_table( char *table_name )//已测试(20-2-2016)
{
	if( table_name != NULL )
	{
		INIT_ZERO( gsql, SQL_STRING_LEN );
		sprintf( gsql, "delete from %s;", table_name );
		return db_excute_sql( gsystem_db, gsql );
	}

	return -1;
}

#ifdef __SYSTEM_DB_DEBUG__
void system_db_table_result_print( int nrow, int ncolumn, char **db_result )//已测试(20-2-2016)
{
	assert( db_result );
	
	int i = 0, j = 0;
	int nindex = ncolumn;
	
	printf( " \n\n\t\t******************Queue database result (nrow %d-col %d)********************\t\t\n", nrow, ncolumn );
	for( i = 0; i < nrow; i++ )
	{
		for( j = 0; j < ncolumn; j++ )
		{
			printf( "%s: %s\n", db_result[j], db_result[nindex] );
			nindex++;
		}
	}
}
#endif

/**=========================开始系统配置数据库表操作======================================**/

/*对数据库的操作:增删查改，事物管理并发，查询延时 已测试(20-2-2016)*/
int system_db_add_configure_system_table( thost_system_set config )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(chman_first,chman_music,auto_close,discuss_mode,speak_limit,apply_limit,chman_limitime,vip_limitime,spk_limtime,temp_close,sign_type,camara_track,current_cmr,menu_language)\
			values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);", \
			SYS_DB_SYSTEM_SET_TABLE,\
			config.chman_first, config.chman_music, config.auto_close, config.discuss_mode,\
			config.speak_limit, config.apply_limit, config.chman_limitime, config.vip_limitime,\
			config.spk_limtime, config.temp_close, config.sign_type, config.camara_track,\
			config.current_cmr, config.menu_language );

	return db_excute_sql( gsystem_db, gsql );
}

// 查表中所有记录(即所有的配置信息)已测试(20-2-2016)
int system_db_queue_configure_system_table( thost_system_set* p_config )
{
	char **db_result = NULL;
	int nrow, ncolumn, i = 0, ret = -1;
	uint8_t flag[sizeof(thost_system_set)] = {0};
	
	if( p_config == NULL )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select* from %s;", SYS_DB_SYSTEM_SET_TABLE );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			for( i = 0; i < sizeof(thost_system_set); i++ )
			{
				flag[i] = atoi( db_result[ncolumn+i]);
			}

			memcpy( p_config, flag, sizeof(thost_system_set));
			ret = 0;
		}
	}
	
	return ret;
}

// 改表中所有记录(即所有的配置信息) 已测试(20-2-2016)
int system_db_update_configure_system_table( thost_system_set config_info )
{
	thost_system_set temp_config;
	if( 0 == system_db_queue_configure_system_table( &temp_config ))// has record?
	{// update new system set
		INIT_ZERO( gsql, SQL_STRING_LEN );
		sprintf( gsql, "update %s set chman_first = %d,chman_music = %d,\
			auto_close = %d,discuss_mode = %d,speak_limit = %d,apply_limit = %d,chman_limitime = %d,vip_limitime = %d,\
			spk_limtime = %d,temp_close = %d,sign_type = %d,camara_track = %d,current_cmr = %d,menu_language = %d;", \
			SYS_DB_SYSTEM_SET_TABLE, 	config_info.chman_first, config_info.chman_music, \
			config_info.auto_close, config_info.discuss_mode,	config_info.speak_limit, config_info.apply_limit, \
			config_info.chman_limitime, config_info.vip_limitime, config_info.spk_limtime, config_info.temp_close,\
			config_info.sign_type, config_info.camara_track, config_info.current_cmr, config_info.menu_language );
		
		db_excute_sql( gsystem_db, gsql );
	}
	else 
	{// insert record
		system_db_add_configure_system_table( config_info );
	}

	return 0;
}
/**=========================结束系统配置数据库表操作======================================**/

/**=========================开始矩阵配置数据库表操作(对数据库的操作:增删查改，事物管理并发，查询延时 已测试(20-2-2016))======================================**/
int system_db_add_matrix_config_table( smatrix_config_entity entity ) // 增加一条记录
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(matrix_type,version,command_system,password,backlight,serial_on,all_corresponding,lock,bell,close_all)\
			values(%s,%s,%s,%s,%s,%d,%d,%d,%d,%d);", \
			SYS_DB_MATRIX_CONTROL_TABLE,\
			entity.matrix_type, entity.version, entity.command_system, entity.password, entity.backlight,\
			entity.serial_on, entity.all_corresponding, entity.lock, entity.bell, entity.close_all );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_queue_matrix_config_record( smatrix_config_entity *p_entity )// 查询一条记录
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1;
	
	if( p_entity == NULL )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select* from %s;", SYS_DB_MATRIX_CONTROL_TABLE );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			strncpy( p_entity->matrix_type, db_result[ncolumn], 30 );
			strncpy( p_entity->version, db_result[ncolumn+1], 30 );
			strncpy( p_entity->command_system, db_result[ncolumn+2], 30 );
			strncpy( p_entity->password, db_result[ncolumn+3], 8 );
			strncpy( p_entity->backlight, db_result[ncolumn+4], 3 );
			p_entity->serial_on = atoi( db_result[ncolumn+5] );
			p_entity->all_corresponding = atoi( db_result[ncolumn+6] );
			p_entity->lock = atoi( db_result[ncolumn+7] );
			p_entity->bell = atoi( db_result[ncolumn+8] );
			p_entity->close_all = atoi( db_result[ncolumn+9] );
			ret = 0;
		}
	}
	
	return ret;
}

const char* system_db_queue_matrix_matrix_type( void )
{
	return gmatrix_record.matrix_type;
}

const char* system_db_queue_matrix_version( void )
{
	return gmatrix_record.version;
}

const char* system_db_queue_matrix_command_system( void )
{
	return gmatrix_record.command_system;
}

char* system_db_queue_matrix_password( void )
{
	return gmatrix_record.password;
}

const char* system_db_queue_matrix_backlight( void )
{
	return gmatrix_record.backlight;
}

int system_db_queue_matrix_serial_on( void )
{
	return gmatrix_record.serial_on;
}

int system_db_queue_matrix_all_corresponding( void )
{
	return gmatrix_record.all_corresponding;
}

int system_db_queue_matrix_lock( void )
{
	return gmatrix_record.lock;
}

int system_db_queue_matrix_bell( void )
{
	return gmatrix_record.bell;
}

int system_db_queue_matrix_close_all( void )
{
	return gmatrix_record.close_all;
}

int system_db_update_matrix_config_table( smatrix_config_entity config_info )
{
	smatrix_config_entity temp_config_info;
	if( 0 == system_db_queue_matrix_config_record( &temp_config_info ))// has record?
	{// update new system set
		INIT_ZERO( gsql, SQL_STRING_LEN );
		sprintf( gsql, "update %s set matrix_type = %s,version = %s,command_system = %s,password = %s,backlight = %s,\
			serial_on = %d,all_corresponding = %d,lock = %d,bell = %d,close_all = %d;", \
			SYS_DB_MATRIX_CONTROL_TABLE, config_info.matrix_type, config_info.version,\
			config_info.command_system,config_info.password,config_info.backlight,\
			config_info.serial_on,config_info.all_corresponding,config_info.lock,\
			config_info.bell,config_info.close_all );
		
		db_excute_sql( gsystem_db, gsql );
	}
	else 
	{// insert record
		system_db_add_matrix_config_table( config_info );
	}

	return 0;
}

int system_db_update_matrix_type( char *p_str, int str_len )
{	
	if( p_str == NULL || str_len > 30 )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set matrix_type = %s where matrix_type != %s", SYS_DB_MATRIX_CONTROL_TABLE, p_str, p_str );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		memcpy(gmatrix_record.matrix_type, p_str, str_len );
	}

	return 0;
}

int system_db_update_matrix_version( char *p_str, int str_len )
{	
	if( p_str == NULL || str_len > 30 )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set version = %s where version != %s", SYS_DB_MATRIX_CONTROL_TABLE, p_str, p_str );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		memcpy(gmatrix_record.version, p_str, str_len );
	}

	return 0;
}

int system_db_update_matrix_command_system( char *p_str, int str_len )
{
	if( p_str == NULL || str_len > 30 )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set command_system = %s where command_system != %s", SYS_DB_MATRIX_CONTROL_TABLE, p_str, p_str );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		memcpy(gmatrix_record.command_system, p_str, str_len );
	}

	return 0;
}

int system_db_update_matrix_password( char *p_str, int str_len )
{
	if( p_str == NULL || str_len != 8 )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set password = %s where password != %s", SYS_DB_MATRIX_CONTROL_TABLE, p_str, p_str );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		memcpy(gmatrix_record.password, p_str, str_len );
	}
	
	return 0;

}

int system_db_update_matrix_backlight( char *p_str, int str_len )
{	
	if( p_str == NULL || str_len != 3 )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set backlight = %s where backlight != %s", SYS_DB_MATRIX_CONTROL_TABLE, p_str, p_str );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		memcpy(gmatrix_record.backlight, p_str, str_len );
	}

	return 0;
}

int system_db_update_matrix_serial_on( int serial_flags )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set serial_on = %d where serial_on != %d", SYS_DB_MATRIX_CONTROL_TABLE, serial_flags, serial_flags );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		gmatrix_record.serial_on = serial_flags;
	}

	return 0;
}

int system_db_update_matrix_all_corresponding( int cor_flags )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set all_corresponding = %d where all_corresponding != %d", SYS_DB_MATRIX_CONTROL_TABLE, cor_flags, cor_flags );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		gmatrix_record.all_corresponding = cor_flags;
	}

	return 0;
}

int system_db_update_matrix_lock( int lock_flag )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set lock = %d where lock != %d", SYS_DB_MATRIX_CONTROL_TABLE, lock_flag, lock_flag );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		gmatrix_record.lock = lock_flag;
	}

	return 0;
}

int system_db_update_matrix_bell( int bell_flag )
{
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set bell = %d where bell != %d", SYS_DB_MATRIX_CONTROL_TABLE, bell_flag, bell_flag );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		gmatrix_record.bell = bell_flag;
	}

	return 0;
}

int system_db_update_matrix_close_all( int close_flag )
{	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set close_all = %d where matrix_type = %d", SYS_DB_MATRIX_CONTROL_TABLE, close_flag, gmatrix_record.close_all );
	if( -1 !=db_excute_sql( gsystem_db, gsql ) )
	{
		gmatrix_record.close_all = close_flag;
	}

	return 0;
}
/**=========================结束矩阵配置数据库表操作======================================**/

/**=========================开始矩阵切换数据库表操作======================================**/
int system_database_matrix_io_insert( enum matrix_switch_cmd sw_cmd )// 使用前需填充gmatrix_io_swich_pro
{
	int *p = gmatrix_io_swich_pro.output_num;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	if( sw_cmd == AV_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = AV_SWITCH;
		sprintf( gsql, \
			"insert into %s(intput,output1,output2,output3,output4,output5,output6,output7,output8,output9,output10,output11,output12,output13,output14,output15,output16)\
			values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);", SYS_DB_MATRIX_AVSW_TABLE,\
			gmatrix_io_swich_pro.input, p[0], p[1], p[2], p[3], p[4], p[5],\
			p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	}
	else if( sw_cmd == V_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = V_SWITCH;
		sprintf( gsql, \
			"insert into %s(intput,output1,output2,output3,output4,output5,output6,output7,output8,output9,output10,output11,output12,output13,output14,output15,output16)\
			values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);", SYS_DB_MATRIX_VSW_TABLE,\
			gmatrix_io_swich_pro.input, p[0], p[1], p[2], p[3], p[4], p[5],\
			p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	}
	else if( sw_cmd == A_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = A_SWITCH;
		sprintf( gsql, \
			"insert into %s(intput,output1,output2,output3,output4,output5,output6,output7,output8,output9,output10,output11,output12,output13,output14,output15,output16)\
			values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);", SYS_DB_MATRIX_ASW_TABLE,\
			gmatrix_io_swich_pro.input, p[0], p[1], p[2], p[3], p[4], p[5],\
			p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	}
	
	return db_excute_sql( gsystem_db, gsql );
}

int system_database_matrix_io_update( enum matrix_switch_cmd sw_cmd )// 使用前需填充gmatrix_io_swich_pro
{
	int *p_out = gmatrix_io_swich_pro.output_num;
	int ret = 0;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	if( sw_cmd == AV_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = AV_SWITCH;
		sprintf( gsql, "update %s set intput = %d,output1 = %d,output2 = %d,\
			output3 = %d,output4 = %d,output5 = %d,output6 = %d,output7 = %d,\
			output8 = %d,output9 = %d,output10 = %d,output11 = %d,output12 = %d,\
			output13 = %d,output14 = %d,output15 = %d,output16 = %d where intput = %d;", \
			SYS_DB_MATRIX_AVSW_TABLE, gmatrix_io_swich_pro.input, p_out[0],
			p_out[1], p_out[2],p_out[3],p_out[4],p_out[5],p_out[6],p_out[7],p_out[8],\
			p_out[9],p_out[10],p_out[11],p_out[12],p_out[13],p_out[14],p_out[15],\
			gmatrix_io_swich_pro.input);
	}
	else if( sw_cmd == V_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = V_SWITCH;
		sprintf( gsql, "update %s set intput = %d,output1 = %d,output2 = %d,\
			output3 = %d,output4 = %d,output5 = %d,output6 = %d,output7 = %d,\
			output8 = %d,output9 = %d,output10 = %d,output11 = %d,output12 = %d,\
			output13 = %d,output14 = %d,output15 = %d,output16 = %d where intput = %d;", \
			SYS_DB_MATRIX_VSW_TABLE, gmatrix_io_swich_pro.input, p_out[0],
			p_out[1], p_out[2],p_out[3],p_out[4],p_out[5],p_out[6],p_out[7],p_out[8],\
			p_out[9],p_out[10],p_out[11],p_out[12],p_out[13],p_out[14],p_out[15],
			gmatrix_io_swich_pro.input);
	}
	else if( sw_cmd == A_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = A_SWITCH;
		sprintf( gsql, "update %s set intput = %d,output1 = %d,output2 = %d,\
			output3 = %d,output4 = %d,output5 = %d,output6 = %d,output7 = %d,\
			output8 = %d,output9 = %d,output10 = %d,output11 = %d,output12 = %d,\
			output13 = %d,output14 = %d,output15 = %d,output16 = %d where intput = %d;", \
			SYS_DB_MATRIX_ASW_TABLE,gmatrix_io_swich_pro.input, p_out[0],
			p_out[1], p_out[2],p_out[3],p_out[4],p_out[5],p_out[6],p_out[7],p_out[8],\
			p_out[9],p_out[10],p_out[11],p_out[12],p_out[13],p_out[14],p_out[15],\
			gmatrix_io_swich_pro.input);
	}
	else 
		ret = -1;

	if( ret == -1 )
		return -1;

	return db_excute_sql( gsystem_db, gsql );
}

int system_database_matrix_io_queue( enum matrix_switch_cmd sw_cmd )// 查询填充到gmatrix_io_swich
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = 0;
	int *p_out = gmatrix_io_swich_pro.output_num;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	if( sw_cmd == AV_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = AV_SWITCH;
		sprintf( gsql, "select * from %s where intput = %d;", SYS_DB_MATRIX_AVSW_TABLE, gmatrix_io_swich_pro.input );
	}
	else if( sw_cmd == V_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = V_SWITCH;
		sprintf( gsql, "select * from %s where intput = %d;", SYS_DB_MATRIX_VSW_TABLE, gmatrix_io_swich_pro.input );
	}
	else if( sw_cmd == A_SWITCH )
	{
		gmatrix_io_swich_pro.sw_cmd = A_SWITCH;
		sprintf( gsql, "select * from %s where intput = %d;", SYS_DB_MATRIX_ASW_TABLE, gmatrix_io_swich_pro.input );
	}
	else 
		ret = 2;
	
	if( ret == 0 )
		db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	else 
		return ret;

	ret = -1;
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			gmatrix_io_swich_pro.input = atoi( db_result[ncolumn]);
			p_out[0] = atoi( db_result[ncolumn+1]);
			p_out[1] = atoi( db_result[ncolumn+2]);
			p_out[2] = atoi( db_result[ncolumn+3]);
			p_out[3] = atoi( db_result[ncolumn+4]);
			p_out[4] = atoi( db_result[ncolumn+5]);
			p_out[5] = atoi( db_result[ncolumn+6]);
			p_out[6] = atoi( db_result[ncolumn+7]);
			p_out[7] = atoi( db_result[ncolumn+8]);
			p_out[8] = atoi( db_result[ncolumn+9]);
			p_out[9] = atoi( db_result[ncolumn+10]);
			p_out[10] = atoi( db_result[ncolumn+11]);
			p_out[11] = atoi( db_result[ncolumn+12]);
			p_out[12] = atoi( db_result[ncolumn+13]);
			p_out[13] = atoi( db_result[ncolumn+14]);
			p_out[14] = atoi( db_result[ncolumn+15]);
			p_out[15] = atoi( db_result[ncolumn+16]);
			ret = 0;
		}
	}
	
	return ret;
}
/**=========================结束矩阵切换数据库表操作======================================**/

/**=========================开始终端信息数据库表操作======================================**/
int system_db_tmnluser_info_insert(  sdb_terminal_user user_info )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(uid, uname, user_type, registered,signed, sys_state, mic_state)\
			values(%d,%s,%d,%d,%d,%d,%d);", SYS_DB_TMNL_USER_TABLE,\
			user_info.uid, user_info.uname, user_info.user_type, user_info.registered,\
			user_info.sign_flag, user_info.sys_state, user_info.mic_state );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_tmnluser_info_queue(  sdb_terminal_user *p_user_info, int uid )
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1;

	if( p_user_info == NULL )
		return 2;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select * from %s where uid = %d;", SYS_DB_TMNL_USER_TABLE, uid );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			p_user_info->uid = atoi( db_result[ncolumn] );
			strncpy( p_user_info->uname, db_result[ncolumn+1], 30 );
			p_user_info->user_type = atoi( db_result[ncolumn+2] );
			p_user_info->registered = atoi( db_result[ncolumn+3] );
			p_user_info->sign_flag = atoi( db_result[ncolumn+4] );
			p_user_info->sys_state = atoi( db_result[ncolumn+5] );
			p_user_info->mic_state = atoi( db_result[ncolumn+6] );
			ret = 0;
		}
	}
	
	return ret;
}

int system_db_tmnluser_info_update(  sdb_terminal_user user_info, int uid )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set uid = %d, uname =%s, user_type = %d, registered = %d,signed = %d, sys_state = %d, mic_state = %d where uid = %d;", \
			SYS_DB_TMNL_USER_TABLE, user_info.uid, user_info.uname, \
			user_info.user_type, user_info.registered,\
			user_info.sign_flag, user_info.sys_state, user_info.mic_state, uid);

	return db_excute_sql( gsystem_db, gsql );
}
/**=========================结束终端信息数据库表操作======================================**/

/**=========================开始终端投票数据库表操作======================================**/
int system_db_tmnlkey_info_insert( enum conference_table_type c_type, sdb_conference_type key_info )
{
	char *p_table = NULL;
	
	if( c_type == CONFERENCE_VOTE )
	{
		p_table = SYS_DB_TMNL_KEY_VOTE_TABLE;
	}
	else if( c_type == CONFERENCE_SELECT )
	{
		p_table = SYS_DB_TMNL_KEY_SELECT_TABLE;
	}
	else if( c_type == CONFERENCE_GRADE )
	{
		p_table = SYS_DB_TMNL_KEY_GRADE_TABLE;
	}
	else 
		return 2;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(uid, key1 , key2 , key3 , key4 , key5 )\
			values(%d,%d,%d,%d,%d,%d);", p_table,key_info.uid,\
			key_info.key[0], key_info.key[1], key_info.key[2],\
			key_info.key[3], key_info.key[4] );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_tmnlkey_info_queue( enum conference_table_type c_type, int uid,sdb_conference_type *p_key_info )
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1, i = 0;
	char *p_table = NULL;

	if( p_key_info == NULL )
		return 2;

	if( c_type == CONFERENCE_VOTE )
	{
		p_table = SYS_DB_TMNL_KEY_VOTE_TABLE;
	}
	else if( c_type == CONFERENCE_SELECT )
	{
		p_table = SYS_DB_TMNL_KEY_SELECT_TABLE;
	}
	else if( c_type == CONFERENCE_GRADE )
	{
		p_table = SYS_DB_TMNL_KEY_GRADE_TABLE;
	}
	else 
		return 2;
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select * from %s where uid = %d;", p_table, uid );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			p_key_info->uid = atoi( db_result[ncolumn] );
			for( i = 0; i < KEY_VOTE_NUM; i++ )
				p_key_info->key[i] = atoi( db_result[ncolumn+1+i] );
			ret = 0;
		}
	}
	
	return ret;
}

int system_db_tmnlkey_info_update( enum conference_table_type c_type, sdb_conference_type key_info )
{
	char *p_table = NULL;
	
	if( c_type == CONFERENCE_VOTE )
	{
		p_table = SYS_DB_TMNL_KEY_VOTE_TABLE;
	}
	else if( c_type == CONFERENCE_SELECT )
	{
		p_table = SYS_DB_TMNL_KEY_SELECT_TABLE;
	}
	else if( c_type == CONFERENCE_GRADE )
	{
		p_table = SYS_DB_TMNL_KEY_GRADE_TABLE;
	}
	else 
		return 2;
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set uid = %d, key1 = %d, key2 = %d, key3 = %d, key4 = %d, key5 = %d where uid = %d;", \
			p_table,key_info.uid,\
			key_info.key[0], key_info.key[1], key_info.key[2],\
			key_info.key[3], key_info.key[4], key_info.uid );

	return db_excute_sql( gsystem_db, gsql );
}
/**=========================结束终端投票数据库表操作======================================**/

/**=========================开始终端预置位数据库表操作======================================**/
int system_db_cmrpre_info_insert( sdb_cmr_preset insert_info, int uid )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(uid, camera_num, camera_preset_point )\
			values(%d,%d,%d);", SYS_DB_TMNL_CAMERA_PRE_TABLE,\
			uid, insert_info.cmr_num, insert_info.preset_point_num );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_cmrpre_info_queue( sdb_cmr_preset *p_insert_info, int uid )
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1;

	if( p_insert_info == NULL )
		return 2;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select * from %s where uid = %d;", SYS_DB_TMNL_CAMERA_PRE_TABLE, uid );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			p_insert_info->uid = atoi( db_result[ncolumn] );
			p_insert_info->cmr_num = atoi( db_result[ncolumn+1] );
			p_insert_info->preset_point_num = atoi( db_result[ncolumn+2] );
			ret = 0;
		}
	}
	
	return ret;
}

int system_db_cmrpre_info_update( sdb_cmr_preset insert_info, int uid )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set uid = %d, camera_num = %d, camera_preset_point = %d where uid = %d;", \
			SYS_DB_TMNL_CAMERA_PRE_TABLE, uid, insert_info.cmr_num, insert_info.preset_point_num, uid );

	return db_excute_sql( gsystem_db, gsql );
}
/**=========================结束终端预置位数据库表操作======================================**/

/**=========================开始avdecc管理数据库表操作======================================**/
int system_db_avdecc_info_insert(  tavdecc_manage discover_info, tavdecc_manage descripor_info, tavdecc_manage device_remove )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(enable_discover,discover_interval, poll_descripor, poll_descripor_interval,automatic_remove,  remove_interval)\
			values(%d,%d,%d,%d,%d,%d);", SYS_DB_AVDECC_SET_TABLE,\
			discover_info.enable, discover_info.query_timer.count_time,\
			descripor_info.enable, descripor_info.query_timer.count_time, \
			device_remove.enable, device_remove.query_timer.count_time );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_avdecc_info_queue(  tavdecc_manage* p_discover_info, tavdecc_manage* p_descripor_info, tavdecc_manage* p_device_remove )
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1;

	if( p_discover_info == NULL || p_descripor_info == NULL || p_device_remove == NULL)
		return 2;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select * from %s;", SYS_DB_AVDECC_SET_TABLE );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			p_discover_info->enable = atoi( db_result[ncolumn] );
			p_discover_info->query_timer.count_time = atoi( db_result[ncolumn+1] );
			p_descripor_info->enable = atoi( db_result[ncolumn+2] );
			p_descripor_info->query_timer.count_time = atoi( db_result[ncolumn+3] );
			p_device_remove->enable = atoi( db_result[ncolumn+4] );
			p_device_remove->query_timer.count_time = atoi( db_result[ncolumn+5] );
			ret = 0;
		}
	}
	
	return ret;
}

int system_db_avdecc_info_update( tavdecc_manage discover_info, tavdecc_manage descripor_info, tavdecc_manage device_remove )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set enable_discover = %d, discover_interval =%d, poll_descripor = %d, poll_descripor_interval = %d,automatic_remove = %d, remove_interval = %d;", \
			SYS_DB_AVDECC_SET_TABLE, 
			discover_info.enable, discover_info.query_timer.count_time,\
			descripor_info.enable, descripor_info.query_timer.count_time, \
			device_remove.enable, device_remove.query_timer.count_time );

	return db_excute_sql( gsystem_db, gsql );
}
/**=========================结束avdecc管理数据库表操作======================================**/

/**=========================开始系统无线遥控地址表操作======================================**/
int system_db_insert_wireless_addr_table( Sdb_wireless_addr_entity wire )
{
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(wireless_addr1,wireless_addr2,wireless_addr3) values(%d,%d,%d);", \
			SYS_DB_WIREADDR_TABLE,\
			wire.addrlist[0], wire.addrlist[1],wire.addrlist[2] );

	return db_excute_sql( gsystem_db, gsql );
}

int system_db_query_wireless_addr_table( Sdb_wireless_addr_entity *p_wire )
{
	char **db_result = NULL;
	int nrow, ncolumn, i = 0, ret = -1;

	if( p_wire == NULL )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select* from %s;", SYS_DB_WIREADDR_TABLE );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			for( i = 0; i < sizeof(Sdb_wireless_addr_entity); i++ )
			{
				p_wire->addrlist[i] = atoi( db_result[ncolumn+i]);
			}

			ret = 0;
		}
	}
	
	return ret;
}

// 改表中所有记录
int system_db_update_wireless_addr_table( Sdb_wireless_addr_entity wire )
{
	Sdb_wireless_addr_entity temp_wire;
	if( 0 == system_db_query_wireless_addr_table( &temp_wire ))// has record?
	{// update new system set
		INIT_ZERO( gsql, SQL_STRING_LEN );
		sprintf( gsql, "update %s set wireless_addr1 = %d,wireless_addr2 = %d,\
			wireless_addr3 = %d;", \
			SYS_DB_WIREADDR_TABLE, wire.addrlist[0], wire.addrlist[1],wire.addrlist[2] );
		
		db_excute_sql( gsystem_db, gsql );
	}
	else 
	{// insert record
		system_db_insert_wireless_addr_table( wire );
	}

	return 0;
}
/**=========================结束系统无线遥控地址表操作======================================**/

/**=========================开始广播表配置操作======================================**/
int system_db_insert_muticast_table( Tstr_sysmuti_param sys_muti )
{
	int ret = -1;
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, \
			"insert into %s(muti_flag, en_default_muti, reconnect_self, offline_connect,\
			reconnect_timeout, failed_connect_count , discut_self, log_err , \
			log_discut, log_none_muticast, log_timeout, query_timeout ) \
			values( %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d );", SYS_DB_MUTICAST_TABLE,\
			sys_muti.muti_flag, sys_muti.en_default_muti, sys_muti.reconnect_self, sys_muti.offline_connect,\
			sys_muti.reconnect_timeout , sys_muti.failed_connect_count , sys_muti.discut_self , sys_muti.log_err , \
			sys_muti.log_discut , sys_muti.log_none_muticast, sys_muti.log_timeout , sys_muti.query_timeout );

	// insert record
	ret = db_excute_sql( gsystem_db, gsql );

	if( ret == 0 )
	{
		// 更新全局数据
		run_db_callback_func( MUTICASTOR, MUTICASTOR_GET_HOST_MUTICAST, &sys_muti );
	}

	return ret;
}

int system_db_query_muticast_table( Tstr_sysmuti_param *p_sys_muti )
{
	char **db_result = NULL;
	int nrow, ncolumn, ret = -1;

	if( p_sys_muti == NULL )
		return -1;

	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "select* from %s;", SYS_DB_MUTICAST_TABLE );
	db_get_table( gsystem_db, gsql, strlen(gsql), &db_result, &nrow, &ncolumn );
	if( db_result != NULL )
	{
		if( nrow != 0 && ncolumn != 0 )
		{
#ifdef __SYSTEM_DB_DEBUG__
			system_db_table_result_print( nrow, ncolumn, db_result );
#endif
			p_sys_muti->muti_flag =  atoi( db_result[ncolumn]);
			p_sys_muti->en_default_muti = atoi( db_result[ncolumn+1]);
			p_sys_muti->reconnect_self = atoi( db_result[ncolumn+2]);
			p_sys_muti->offline_connect = atoi( db_result[ncolumn+3]);
			p_sys_muti->reconnect_timeout = atoi( db_result[ncolumn+4]);
			p_sys_muti->failed_connect_count = atoi( db_result[ncolumn+5]);
			p_sys_muti->discut_self = atoi( db_result[ncolumn+6]);
			p_sys_muti->log_err = atoi( db_result[ncolumn+7]);
			p_sys_muti->log_discut = atoi( db_result[ncolumn+8]);
			p_sys_muti->log_none_muticast = atoi( db_result[ncolumn+9]);
			p_sys_muti->log_timeout = atoi( db_result[ncolumn+10]);
			p_sys_muti->query_timeout = atoi( db_result[ncolumn+11]);
			ret = 0;
		}
	}
	
	return ret;
}

// 改表中所有记录
int system_db_update_muticast_table( Tstr_sysmuti_param sys_muti )
{
	int ret = -1;
	
	// update new system set
	INIT_ZERO( gsql, SQL_STRING_LEN );
	sprintf( gsql, "update %s set muti_flag = %d, en_default_muti = %d, reconnect_self = %d, offline_connect = %d,\
		reconnect_timeout = %d, failed_connect_count = %d, discut_self = %d, log_err = %d, \
		log_discut = %d, log_none_muticast = %d, log_timeout = %d, query_timeout = %d;", SYS_DB_MUTICAST_TABLE, \
		sys_muti.muti_flag, sys_muti.en_default_muti, sys_muti.reconnect_self, sys_muti.offline_connect,\
		sys_muti.reconnect_timeout , sys_muti.failed_connect_count , sys_muti.discut_self , sys_muti.log_err , \
		sys_muti.log_discut , sys_muti.log_none_muticast, sys_muti.log_timeout , sys_muti.query_timeout );
	
	ret = db_excute_sql( gsystem_db, gsql );

	if( ret == 0 )
	{
		// 更新全局数据
		run_db_callback_func( MUTICASTOR, MUTICASTOR_GET_HOST_MUTICAST, &sys_muti );
	}

	return ret;
}
/**=========================结束广播表配置操作======================================**/

/*系统数据库模块的初始化*/
void system_database_init( void )
{	
	gsystem_db = db_open_file( SYSTEM_DB_FILE );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_SYSTEM_SET_TABLE, SYSTEM_CONFIG_COLUMN, gsystem_db );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_MATRIX_CONTROL_TABLE, MATRIX_CONTROL_COLUMN, gsystem_db );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_MATRIX_AVSW_TABLE, MATRIX_AVSW_COLUMN, gsystem_db );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_MATRIX_VSW_TABLE, MATRIX_VSW_COLUMN, gsystem_db );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_MATRIX_ASW_TABLE, MATRIX_ASW_COLUMN, gsystem_db );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_TMNL_USER_TABLE, TERMINA_USER_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_TMNL_CAMERA_PRE_TABLE, CAMERA_PRESET_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_TMNL_KEY_VOTE_TABLE, TERMINA_KEY_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_TMNL_KEY_SELECT_TABLE, TERMINA_KEY_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_TMNL_KEY_GRADE_TABLE, TERMINA_KEY_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_AVDECC_SET_TABLE, SYSTEM_AVDECC_SET_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_WIREADDR_TABLE, SYSTEM_WIREADDR_COLUMN, gsystem_db );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	SQL3_TABLE_CREATE( gsql, SYS_DB_MUTICAST_TABLE, SYSTEM_MUTICAST_COLUMN, gsystem_db );

	INIT_ZERO( &gmatrix_record, sizeof(gmatrix_record));
	if( -1 == system_db_queue_matrix_config_record( &gmatrix_record ))
	{
		MSGINFO( " init matrix system configuration failed" );
	}

	INIT_ZERO( gmatrix_io_sw_array, sizeof(gmatrix_io_sw_array));	
	INIT_ZERO( &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));
	if( -1 == system_database_matrix_io_queue(AV_SWITCH) )
	{
		MSGINFO( " init matrix AVSW output input failed" );
	}
	else
		memcpy( &gmatrix_io_sw_array[AV_SWITCH], &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));
	
	INIT_ZERO( &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));
	if( -1 == system_database_matrix_io_queue(V_SWITCH) )
	{
		MSGINFO( " init matrix VSW output input failed" );
	}
	else
		memcpy( &gmatrix_io_sw_array[V_SWITCH], &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));

	INIT_ZERO( &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));
	if( -1 == system_database_matrix_io_queue(A_SWITCH) )
	{
		MSGINFO( " init matrix ASW output input failed" );
	}
	else
		memcpy( &gmatrix_io_sw_array[A_SWITCH], &gmatrix_io_swich_pro, sizeof(gmatrix_io_swich_pro));

	Tstr_sysmuti_param sys_muti;
	if( -1 == system_db_query_muticast_table(&sys_muti) ) 
	{
		// 第一次初始化化默认数据
		sys_muti.muti_flag = true; 
		sys_muti.en_default_muti = false;
		sys_muti.reconnect_self = true; // true 解析:本机断开重连本机;false 解析:本机不重连且断开本机才能断开本机
		sys_muti.offline_connect = true;
		sys_muti.reconnect_timeout = 10;// 10s
		sys_muti.failed_connect_count = 10;// 10次
		sys_muti.discut_self = false;// 本机发言时不断开本机
		sys_muti.log_err = true;
		sys_muti.log_discut = true;
		sys_muti.log_none_muticast = true;
		sys_muti.log_timeout = 30;// 30s
		sys_muti.query_timeout = 10;// 10s
		
		system_db_insert_muticast_table( sys_muti );
	}
	else
	{// 更新全局数据
		run_db_callback_func( MUTICASTOR, MUTICASTOR_GET_HOST_MUTICAST, &sys_muti );
	}
}

void system_database_destroy( void )
{
	db_close( gsystem_db );
}

