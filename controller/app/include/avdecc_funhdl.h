/*
* @file avdecc_funhdl.h
* @brief
* @ingroup
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2017-03-07
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __AVDECC_FUNHDL_H__
#define __AVDECC_FUNHDL_H__
/*Including head files------------------------------------------------------*/
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

/*$ \*/
struct epoll_priv;

/*$ \*/
typedef int (* handler_fn) (struct epoll_priv * priv);

/*$ \*/
struct epoll_priv {
    int fd;         /*! epoll register fd */
    handler_fn fn;  /*! handler function */
};

/*$ \*/
int fn_netif_cb(struct epoll_priv *priv);

/*$ \*/
int udp_server_fn(struct epoll_priv *priv);

/*$ \*/
int udp_client_fn(struct epoll_priv *priv);

/*$ \*/
int prep_evt_desc(int fd,handler_fn fn,
    struct epoll_priv *priv,struct epoll_event *ev);

/*$ \*/
int thread_fn(void *pgm);

/*$ \*/
int pthread_handle_create(pthread_t *h_trd, struct fds *kfds);

#endif /* __AVDECC_FUNHDL_H__ */

