#ifndef __AVDECC_FUNHDL_NATIVE_H__
#define __AVDECC_FUNHDL_NATIVE_H__

#include <pthread.h>
#include "avdecc_main.h"
#include "system_packet_tx.h"
#include "host_controller_debug.h"

int thread_pipe_fn( void *pgm );
int pthread_handle_pipe( pthread_t *p_trd, struct fds *kfds );

#endif

