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

#define SQL_STRING_LEN 1024 // sqlÓï¾äµÄ³¤¶È

sqlite3 *db_open_file( const char *path_name );
void create_database_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char *table_name );
bool db_is_table_exist( sqlite3 *p_db, char *sql, uint16_t sql_len, const char *table_name );
int db_get_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char ***db_result, int *nrow, int *ncolumn );
int db_excute_sql( sqlite3 *p_db,char * sql );
int db_close( sqlite3 *p_db );

#endif
