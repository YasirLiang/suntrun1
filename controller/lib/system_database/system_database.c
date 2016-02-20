/*
**Author:YasirLiang
**File:system_database.c
**Date Create:18-2-2016
**
*/

#include "system_database.h"

static sqlite3* gsystem_db = NULL;
static char gsql[SQL_STRING_LEN] = {'\0'};

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

/**=========================开始系统配置数据库表操作======================================**/
#ifdef __SYSTEM_DB_DEBUG__
void system_db_configure_system_table_result_print( int nrow, int ncolumn, char **db_result )//已测试(20-2-2016)
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
			system_db_configure_system_table_result_print( nrow, ncolumn, db_result );
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
			spk_limtime = %d,temp_close = %d,sign_type = %d,camara_track = %d,current_cmr = %d,menu_language = %d", \
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
}

void system_database_destroy( void )
{
	db_close( gsystem_db );
}

