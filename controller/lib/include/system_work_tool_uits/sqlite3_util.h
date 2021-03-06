/*
**Author:YasirLiang
**File:sqlite3_util.h
**Date Create:18-2-2016
**
*/

#ifndef __SQLITE3_UTIL_H__
#define __SQLITE3_UTIL_H__

#include "jdksavdecc_world.h"
#include <sqlite3.h>

#define SQL_STRING_LEN 1024 // sql���ĳ���
#define  FUNC_CREATE_SQLTABLE create_database_table

#define CREATE_TATBLE_SQL_FORM( sql_buf, table_name, column_name ) do{\
		sprintf( sql_buf, "create table %s(%s)", table_name, column_name );\
	}while(0)

#define SQL3_TABLE_CREATE( sql_buf, table_name, column_name, sqlite3_fd_handle ) do{\
	CREATE_TATBLE_SQL_FORM((sql_buf), (table_name), (column_name));\
	FUNC_CREATE_SQLTABLE( (sqlite3_fd_handle), (sql_buf), strlen((sql_buf)), (table_name));\
	}while(0)
	
sqlite3 *db_open_file( const char *path_name );
void create_database_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char *table_name );
bool db_is_table_exist( sqlite3 *p_db, char *sql, uint16_t sql_len, const char *table_name );
int db_get_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char ***db_result, int *nrow, int *ncolumn );
int db_excute_sql( sqlite3 *p_db,char * sql );
int db_close( sqlite3 *p_db );

#endif
