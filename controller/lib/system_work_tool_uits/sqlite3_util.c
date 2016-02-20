/*
**Author:YasirLiang
**File:sqlite3_util.c
**Date Create:18-2-2016
**
*/

#include "sqlite3_util.h"
#include "error_util.h"

static char *errmsg;// ȫ�ֱ��ر���

#define DB_GETERR(ret)  do{\
	if(ret != SQLITE_OK){\
		printf("error:%s\n", errmsg);\
		return -1;\
	}\
}while(0)

/**************************************
**Author:YasirLiang
**Name:db_open_file
**Date Create:18-2-2016
**Func:open database
**Param:
**		path_name: database name
**Return Value:���ݿ���ָ��
***************************************/ 
sqlite3 *db_open_file( const char *path_name )
{
	sqlite3 *dbfile;
	int ret;

	ret = sqlite3_open( path_name, &dbfile );
	if ( ret != SQLITE_OK )
	{
		sqlite3_close( dbfile );
		SYS_ERR( "sqlite3_open" );
	}
	
	return dbfile;
}

/**************************************
**Author:YasirLiang
**Name:create_database_table
**Date Create:18-2-2016
**Func:open database
**Param:
**		path_name: database name
**		sql:sql��仺������ָ��
**		sql_len:sql���ĳ���
**		table_name:���ݿ����
**		column_define:���ݿ��¼�Ķ��塣
**Return Value:None
***************************************/ 
void create_database_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char *table_name )
{
	sqlite3_stmt* stmt = NULL;

	assert( p_db && sql && sql_len > 0 && table_name );
	if( (NULL == p_db) || (NULL == sql) ||\
		(sql_len == 0) || (NULL == table_name)\
		|| (db_is_table_exist(p_db, sql, sql_len, table_name)))
	{
		return;
	}
	
	if (sqlite3_prepare_v2( p_db, sql, sql_len, &stmt, NULL) == SQLITE_OK )
	{
		if( sqlite3_step(stmt) != SQLITE_DONE )
		{
			sqlite3_finalize( stmt );
			sqlite3_close( p_db );
			SYS_ERR( "sqlite3_step" );
		}
	}

	sqlite3_finalize( stmt );
}

/**************************************
**Author:YasirLiang
**Name:create_database_table
**Date Create:18-2-2016
**Func:open database
**Param:
**		p_db: database handle pointer
**		sql:sql��仺������ָ��
**		sql_len:sql���ĳ���
**		table_name:���ݿ����
**Return Value:
**		true:database table exist
**		false:database table not exist
***************************************/ 
bool db_is_table_exist( sqlite3 *p_db, char *sql, uint16_t sql_len, const char *table_name )
{
	sqlite3_stmt* stmt3 = NULL;

	assert( p_db && sql && sql_len > 0 && table_name );
	if( (NULL == p_db) || (NULL == sql) ||\
		(sql_len == 0) || (NULL == table_name))
	{
		return false;
	}

	if( sqlite3_prepare_v2( p_db, sql, sql_len,&stmt3,NULL) != SQLITE_OK ) 
	{
		if( stmt3 )
		{
			sqlite3_finalize( stmt3 );
		}
		
		return false;
	}

	if( sqlite3_step(stmt3) == SQLITE_ROW )
	{
		return true;
	}

	return false;
}

int db_get_table( sqlite3 *p_db, char *sql, uint16_t sql_len, char ***db_result, int *nrow, int *ncolumn )
{
	int result;
	
	assert( p_db && sql && sql_len > 0 && nrow && ncolumn && db_result );
	if( (NULL == p_db) || (NULL == sql) || (NULL == db_result) ||\
		(sql_len == 0) || (NULL == nrow) || NULL == ncolumn )
	{
		return -1;
	}
	
	result = sqlite3_get_table( p_db, sql, db_result, nrow, ncolumn, &errmsg );
	DB_GETERR( result );
	
	return 0;
}

/**************************************
**Author:YasirLiang
**Name:create_database_table
**Date Create:18-2-2016
**Func:ִ�����ݿ�sql���
**Param:
**		sql:sql��仺������ָ��
**Return Value:
**		0: Nomal
**		-1:Unnomal
***************************************/ 
int db_excute_sql( sqlite3 *p_db,char * sql )
{
	int result = 0;

	assert( p_db && sql );
	if( p_db == NULL || sql == NULL )
		return -1;
	
	result = sqlite3_exec( p_db, sql, NULL, NULL, &errmsg );
	DB_GETERR(result);

	return 0;
}

int db_close( sqlite3 *p_db )
{
	sqlite3_close( p_db );
	
	return 0;
}
