#ifndef __AVDECC_FUNHDL_H__
#define __AVDECC_FUNHDL_H__

#include <time.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include "upper_computer.h"
#include "avdecc_net.h"
#include "avdecc_main.h"
#include "host_controller_debug.h"
#include "handle_1722_packet_from_raw.h"
#include "util.h"
#include "upper_computer_data_handle.h"
#include "time_event.h"

struct epoll_priv;
typedef int (* handler_fn) (struct epoll_priv * priv);

struct epoll_priv
{
    int fd;
    handler_fn fn;
};

int fn_netif_cb( struct epoll_priv *priv );
int udp_server_fn( struct epoll_priv *priv );
int udp_client_fn( struct epoll_priv *priv );
int prep_evt_desc( int fd,handler_fn fn,struct epoll_priv *priv,struct epoll_event *ev );
int thread_fn( void *pgm );
int pthread_handle_create( pthread_t *h_trd, struct fds *kfds );
//void set_UDP_parameter(struct host_upper_cmpt_frame *frame, struct sockaddr_in *sin, int len );


#endif
