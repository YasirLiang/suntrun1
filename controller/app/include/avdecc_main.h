/*
* @file
* @brief main function
* @ingroup main function
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-22
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
#ifndef __AVDECC_MAIN_H__
#define __AVDECC_MAIN_H__

/*Including files-----------------------------------------------------------*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "raw.h"
#include "host_controller_debug.h"
#include "pipe.h"
#include "entity.h"
#include "inflight.h"
#include "descriptor.h"
#include "func_proccess.h"
#include "data.h"

/*! Macro network interface name */
#define NETWORT_INTERFACE "eth0"
/*! Macro threads mux num */
#define THREADS_MUX_NUM 16
/*! enum useful value */
enum useful_enums {
    PIPE_RD = 0,    /*! pipe read fd index */
    PIPE_WR = 1,   /*! pipe write fd index */
    POLL_COUNT = 6,        /*! poll number */
    TIME_PERIOD_1_MILLISECONDS = 1,
    TIME_PERIOD_25_MILLISECONDS = 25
};
/*! fds of extern ports set */ 
struct fds {
    int raw_fd;        /*! raw socket fd*/
    int udp_server_fd;/*! udp server fd */
    int udp_client_fd;/*! udp client fd */
    int tx_pipe[2];            /*! pipe */
};
/*! socket_info_s */ 
struct socket_info_s {
    int sock_fd;                        /*! socket fd*/
    struct sockaddr_in sock_addr; /*! socket address */
    socklen_t sock_len;            /*! socket length */
};
/*! udp_context */ 
struct udp_context {
    struct socket_info_s udp_srv, udp_clt;
};
/*! threads_info */ 
struct threads_info {
    pthread_t tid[THREADS_MUX_NUM];
    int pthread_nums;
};
/*! Extern global net_fd varialable declaration*/
extern struct fds net_fd;
/*! Extern global udp client fd varialable declaration*/
extern struct socket_info_s upper_udp_client;
/*! Extern global pc_controller_server variable declaration*/
extern struct socket_info_s pc_controller_server;
/*! Extern system terminal double list guard node declaration*/
extern solid_pdblist endpoint_list;
/*! Extern inflight command double list guard node declaration*/
extern inflight_plist command_send_guard;
/*! Extern raw context varialable declaration */
extern struct raw_context net;
/*! Extern 1722.1 terminal descptor double list guard varialable declaration*/
extern desc_pdblist descptor_guard;
/*! Extern the varialable of table command function delaration*/
const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS];

#endif /*__AVDECC_MAIN_H__*/

