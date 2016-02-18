/*
**Author:YasirLiang
**File:system_database.c
**Date Create:18-2-2016
**
*/

#include "system_database.h"

static sqlite3* gsystem_db = NULL;
static char gsql[SQL_STRING_LEN] = {'\0'};

/*系统数据库模块的初始化*/
void system_database_init( void )
{	
	gsystem_db = db_open_file( SYSTEM_DB_FILE );

	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_SYSTEM_SET_TABLE, SYSTEM_CONFIG_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_SYSTEM_SET_TABLE );
	
	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_MATRIX_CONTROL_TABLE, MATRIX_CONTROL_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_CONTROL_TABLE );
	
	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_MATRIX_AVSW_TABLE, MATRIX_AVSW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_AVSW_TABLE );
	
	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_MATRIX_VSW_TABLE, MATRIX_VSW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_VSW_TABLE );
	
	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_MATRIX_ASW_TABLE, MATRIX_ASW_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_MATRIX_ASW_TABLE );
	

	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_TMNL_USER_TABLE, TERMINA_USER_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_TMNL_USER_TABLE );

	memset( gsql, 0, SQL_STRING_LEN );
	sprintf( gsql, "create table %s(%s)", SYS_DB_TMNL_CAMERA_PRE_TABLE, CAMERA_PRESET_COLUMN );
	create_database_table( gsystem_db, gsql, strlen( gsql ), SYS_DB_TMNL_CAMERA_PRE_TABLE );
}

void system_database_destroy( void )
{
	db_close( gsystem_db );
}

