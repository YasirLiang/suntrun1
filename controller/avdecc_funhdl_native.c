#include "avdecc_funhdl_native.h"

int thread_pipe_fn( void *pgm )
{
	struct fds *kfds = ( struct fds* )pgm;
	struct fds thr_fds;
	memcpy( &thr_fds, kfds, sizeof(struct fds));

	while(1)
	{
		if( check_pipe_read_ready( thr_fds.tx_pipe[PIPE_RD]) )
		{
			tx_data tnt;
			int result = read_pipe_tx( &tnt, sizeof(tx_data) );
			DEBUG_INFO( "RECV pipe data: [ frame_len = %d ]", tnt.frame_len );
			
			if( result > 0 )
			{
				tx_packet_event( tnt.data_type, 
							     tnt.notification_flag, 
							     tnt.frame, 
							     tnt.frame_len, 
							     &thr_fds,
							     command_send_guard,
							     tnt.raw_dest.value,
							     &tnt.udp_sin,
							     tnt.resp );
				release_heap_space( &tnt.frame ); // free heap space mallo by write pipe thread
				assert( tnt.frame == NULL );		// free successfully and result is NULL? 
			}
			else 
			{
				assert( tnt.frame && result >= 0 );
			}
		}
		else
		{
			DEBUG_INFO( "read pipe is not ready!" );
			continue;
		}
	}

	return 0;
}

int pthread_handle_pipe( pthread_t *p_trd, struct fds *kfds )
{
	int rc = 0;
	
	rc = pthread_create( p_trd, NULL, ( void* )&thread_pipe_fn, kfds );
	if ( rc )
	{
		DEBUG_INFO(" Pthread_Handle_Pipe ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	return 0;
}


