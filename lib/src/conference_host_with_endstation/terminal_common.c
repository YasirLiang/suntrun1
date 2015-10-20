#include "terminal_common.h"

void init_terminal_proccess_fd( FILE ** fd )
{
	*fd = Fopen( ADDRESS_FILE );
	if( NULL == *fd )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}
}

int terminal_address_list_read_file( FILE* fd,  terminal_address_list* ptmnl_addr )
{
	assert( fd && ptmnl_addr );

	// read the num of terminal and the crc
	uint16_t tmnl_num = 0;
	if( read_addr_file_terminal_num( fd, &tmnl_num ) < 0)
	{
		DEBUG_INFO( "read addr flie err: end of file or read error" );
		return -1;
	}
	uint16_t tmnl_num = 0;
	if( read_addr_file_terminal_crc( fd, &tmnl_num ) < 0)
	{
		DEBUG_INFO( "read addr flie err: end of file or read error" );
		return -1;
	}
	

	while(  )
}


