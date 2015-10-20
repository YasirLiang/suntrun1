#include "file_util.h"

int read_addr_file_terminal_num( FILE* fd, uint16_t* tmnl_num )
{
	int cont_num = 0;
	
	if( !feof( fd ) )
	{
		cont_num = Fread( fd, tmnl_num, sizeof(uint16_t), 1 );
		return cont_num;
	}
	
	return -1;
}

int  Fread( FILE* fd, void *read_buf, size_t size_member, size_t read_counts )
{
	int count_num = 0;
	count_num = fread( read_buf, size_member, read_counts, fd );

	return count_num; // < 0 is error
}

FILE * Fopen( const char *file_name )
{
	assert( file_name );
	FILE * fd = NULL;
	
	fd = fopen( file_name, ab+ );
	if( NULL == fd )
	{
		DEBUG_ERR( "open %s address Err", file_name );
		assert( NULL != fd );
	}
	
	return fd;
}


