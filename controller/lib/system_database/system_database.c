/*
**Author:YasirLiang
**File:system_database.c
**Date Create:18-2-2016
**
*/

#include "system_database.h"

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

/*系统数据库模块的初始化*/
void system_database_init( void )
{	
	gsystem_db = db_open_file( SYSTEM_DB_FILE );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_SYSTEM_SET_TABLE, SYSTEM_CONFIG_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_SYSTEM_SET_TABLE );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_MATRIX_CONTROL_TABLE, MATRIX_CONTROL_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_CONTROL_TABLE );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_MATRIX_AVSW_TABLE, MATRIX_AVSW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_AVSW_TABLE );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_MATRIX_VSW_TABLE, MATRIX_VSW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_VSW_TABLE );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_MATRIX_ASW_TABLE, MATRIX_ASW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_ASW_TABLE );
	
	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_TMNL_USER_TABLE, TERMINA_USER_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_TMNL_USER_TABLE );

	INIT_ZERO( gsql, SQL_STRING_LEN );
	CREATE_TATBLE_SQL_FORM( gsql, SYS_DB_TMNL_CAMERA_PRE_TABLE, CAMERA_PRESET_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_TMNL_CAMERA_PRE_TABLE );

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
}

void system_database_destroy( void )
{
	db_close( gsystem_db );
}

