#ifndef __AVDECC_FUNHDL_NATIVE_H__
#define __AVDECC_FUNHDL_NATIVE_H__

#include <pthread.h>
#include "avdecc_main.h"
#include "system_packet_tx.h"
#include "host_controller_debug.h"

int thread_pipe_fn( void *pgm );
int pthread_handle_pipe( pthread_t *p_trd, struct fds *kfds );
int pthread_handle_cmd_func( pthread_t *pid, const proccess_func_items *p_items );
int thread_func_fn( void * pgm );
int pthread_recv_data_fn( void *pgm );
int pthread_proccess_recv_data_create( pthread_t *pid, void * pgm );

#endif

