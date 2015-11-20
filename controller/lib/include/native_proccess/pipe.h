#ifndef __PIPE_H__
#define __PIPE_H__

#include <unistd.h>		// pipe
#include <poll.h>		// poll
#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

enum _pipe
{
	READ_PIPE_FD,
	WRITE_PIPE_FD
};

void build_pipe( int pipe_fd[2] );
bool check_pipe_read_ready( int pipe_read_fd );
uint16_t read_pipe_tx( void *buf, uint16_t read_len );
uint16_t write_pipe_tx(const void *buf, uint16_t buf_len );

#endif


