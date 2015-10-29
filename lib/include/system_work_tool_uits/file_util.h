#ifndef __FILE_UTIL_H__
#define __FILE_UTIL_H__

#include "jdksavdecc_world.h"

#define FILE_MAX_READ_BUF 1024

int read_addr_file_terminal_num( FILE* fd, uint16_t* tmnl_num );
int read_addr_file_terminal_crc( FILE* fd, uint16_t* tmnl_crc );
int Fseek( FILE* fd, long offset, int from_where );
int Fwrite( FILE* fd, void* write_buf, size_t size_member, size_t write_counts );
int  Fread( FILE* fd, void *read_buf, size_t size_member, size_t read_counts );
int Fwrite( FILE* fd, void* write_buf, size_t size_member, size_t write_counts );
FILE * Fopen( const char *file_name, const char* mode );
void Fflush( FILE* fd );
int Fclose( FILE *fd );

#endif
