#include "file_util.h"
#include "host_controller_debug.h"

int read_addr_file_terminal_num( FILE* fd, uint16_t* tmnl_num )
{
	int count_num = 0;
	count_num = Fread( fd, tmnl_num, sizeof(uint16_t), 1 );

	return count_num;
}

int read_addr_file_terminal_crc( FILE* fd, uint16_t* tmnl_crc )
{
	int count_num = 0;

	if( !feof( fd ) )
	{
		count_num = Fread( fd, tmnl_crc, sizeof(uint16_t), 1 );
		return count_num;
	}

	return -1;
}

int Fseek( FILE* fd, long offset, int from_where )
{
	return fseek( fd, offset, from_where );
}

int  Fread( FILE* fd, void *read_buf, size_t size_member, size_t read_counts )
{
	int count_num = 0;
	count_num = fread( read_buf, size_member, read_counts, fd );

	return count_num; // < 0 is error
}

int Fwrite( FILE* fd, void* write_buf, size_t size_member, size_t write_counts )
{
	int write_num = 0;
	write_num = fwrite( write_buf, size_member, write_counts, fd );

	return write_num;
}

FILE * Fopen( const char *file_name, const char* mode )
{
	assert( file_name );
	FILE * fd = NULL;
	
	fd = fopen( file_name, mode );
	if( NULL == fd )
	{
		DEBUG_ERR( "open %s address Err", file_name );
	}
	
	DEBUG_INFO( "fopen ok" );
	return fd;
}

void Fflush( FILE* fd )
{
	fflush( fd );
}

int Fclose( FILE *fd )
{
	fclose( fd );

	return 0;
}


