/*
* @file avdecc_funhdl.c
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

/*Including head files------------------------------------------------------*/
#include "avdecc_funhdl.h"
#include "upper_computer.h"
#include "terminal_pro.h"
#include "wait_message.h"
#include "send_common.h"
#include "send_work_queue.h"
#include "func_proccess.h"
#include "profile_system.h"
#include "matrix_output_input.h"
#include "control_matrix_common.h"
#include "controller_machine.h"
#include "raw_network.h"
#include "arcs_extern_port.h"
#include "arcs_common.h"
#include "log_machine.h"

/*$*/
#define INPUT_MSG_LEN	6
/*$*/
static bool system_stop = false;
/*$*/
volatile bool is_inflight_timeout = false;
/*$*/
extern bool acmp_recv_resp_err;
/*$*/
extern volatile bool gsend_pro_idle;
/*$*/
extern unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN];
/*$*/
extern volatile unsigned char gcontrol_sur_msg_len;
/*$*/
extern int gcontrol_sur_fd;
/*$*/
extern volatile bool m_isRunning;

/*$ thread_fn_thread_stop().................................................*/
void thread_fn_thread_stop(void) {
    system_stop = true;
}
/*$ thread_fn_thread_stop().................................................*/
int timer_start_interval(int timerfd) {
    struct itimerspec itimer_new;
    struct itimerspec itimer_old;
    unsigned long ns_per_ms = 1000000;
    unsigned long interval_ms = TIME_PERIOD_25_MILLISECONDS;

    memset(&itimer_new, 0, sizeof(itimer_new));
    memset(&itimer_old, 0, sizeof(itimer_old));
    itimer_new.it_interval.tv_sec = interval_ms / 1000;
    itimer_new.it_interval.tv_nsec = (interval_ms % 1000) * ns_per_ms;
    itimer_new.it_value = itimer_new.it_interval;

    return timerfd_settime(timerfd, 0, &itimer_new, &itimer_old);
}
/*$ fn_timer_cb()...........................................................*/
int fn_timer_cb(struct epoll_priv*priv) {
    int read_len = 0;
    uint64_t timer_exp_count;
    
    read_len = read(priv->fd, &timer_exp_count, sizeof(timer_exp_count));
    
    time_tick_event(endpoint_list, command_send_guard);
    profile_system_file_write_timeouts();

    if (is_inflight_timeout && is_wait_messsage_active_state()) {
        set_wait_message_status(WAIT_TIMEOUT);
        gsend_pro_idle = true;
    }

    is_inflight_timeout = false; 

    return read_len;
}
/*$ fn_netif_cb()...........................................................*/
int fn_netif_cb(struct epoll_priv *priv) {
    ssize_t status = -1;
    uint64_t dest_addr = 0;
    uint64_t default_native_dest = 0;
    uint8_t recv_buf[JDKSAVDECC_FRAME_MAX_PAYLOAD_SIZE] = {0};
    struct jdksavdecc_frame frame;

    status = (ssize_t)controller_machine_1722_network_recv(
                        gp_controller_machine, recv_buf, sizeof(recv_buf));
    if (jdksavdecc_frame_read(&frame, recv_buf, 0, (size_t)status) >= 0) {
        raw_net_1722_user_info *raw_usr_obj;
        int rx_status = -1;
        bool is_notification_id_valid = false;
        uint16_t operation_id = 0;
        bool is_operation_id_valid = false;
        uint8_t subtype;
        
        convert_eui48_to_uint64(frame.dest_address.value, &dest_addr );
        convert_eui48_to_uint64(jdksavdecc_multicast_adp_acmp.value,
            &default_native_dest);
        raw_usr_obj = (raw_net_1722_user_info *)
            gp_controller_machine->unit_1722_net->network_1722_user_obj;
        
        if ((frame.ethertype == JDKSAVDECC_AVTP_ETHERTYPE)
              && ((((uint8_t)(dest_addr >> 40)) & 0x01)
                        ||(dest_addr == raw_usr_obj->mac)))
        {
            subtype = jdksavdecc_common_control_header_get_subtype(
                frame.payload, ZERO_OFFSET_IN_PAYLOAD );
            if (subtype == JDKSAVDECC_SUBTYPE_61883_IIDC) {
            }
            else {
                pthread_mutex_lock(&ginflight_pro.mutex);
                rx_raw_packet_event(frame.dest_address.value,
                frame.src_address.value, &is_notification_id_valid,
                endpoint_list, frame.payload, frame.length, 
                &rx_status, operation_id, is_operation_id_valid);
                pthread_mutex_unlock(&ginflight_pro.mutex);
            }

            if (((0 == rx_status) && is_wait_messsage_active_state())
                 ||(acmp_recv_resp_err && is_wait_messsage_active_state()))
            {
                int msr_status = 0;
                msr_status = set_wait_message_status( rx_status );
                assert( msr_status == 0 );
                gsend_pro_idle = true;
                acmp_recv_resp_err = false;
            }
        }
    }

    return 0;
}
/*$ udp_server_fn().........................................................*/
int udp_server_fn(struct epoll_priv *priv) {
int recv_len = 0;
    struct sockaddr_in sin_in;
    struct host_upper_cmpt_frame recv_frame;

    socklen_t sin_len = sizeof(struct sockaddr_in);
    memset(&recv_frame, 0, sizeof(struct host_upper_cmpt_frame));
    memset(&sin_in, 0, sin_len);

    recv_len = recv_udp_packet(priv->fd, recv_frame.payload,
                sizeof(recv_frame.payload), &sin_in, &sin_len);
    
    if (gregister_tmnl_pro.rgs_state == RGST_IDLE) {
        if (recv_len > 0) {
            upper_computer_common_recv_messsage_save(priv->fd, &sin_in,
                true, sin_len, recv_frame.payload, recv_len);
        }
        else {
            DEBUG_INFO("recv UDP packet len is zero or recv error!");
            is_upper_udp_client_connect = false;
            assert( recv_len >= 0);
        }
    }

    return 0;
}
/*$ udp_client_fn().........................................................*/
int udp_client_fn(struct epoll_priv *priv) {
    return 0;
}
/*$ matrix_control_uart_recv_fn()...........................................*/
int matrix_control_uart_recv_fn(struct epoll_priv *priv) {
    uint16_t recv_len = 0;
    uint8_t recv_buf[2048] = {0};

    recv_len = read(priv->fd, recv_buf, sizeof(recv_buf));
    if(recv_len  > 0) {
         gp_log_imp->log.post_log_msg(&gp_log_imp->log,
                                LOGGING_LEVEL_DEBUG,
                                "[ Recv matrix message %s ]",
                                recv_buf);
        control_matrix_common_recv_message_save(recv_buf, recv_len);
    }

    return 0;
}
/*$ control_surface_recv_fn()...............................................*/
int control_surface_recv_fn(struct epoll_priv *priv) {
    memset(gcontrol_sur_recv_buf, 0, INPUT_MSG_LEN);
    gcontrol_sur_msg_len = read(priv->fd,
        gcontrol_sur_recv_buf, INPUT_MSG_LEN);
    
    if (gregister_tmnl_pro.rgs_state != RGST_IDLE) {
        memset(gcontrol_sur_recv_buf, 0, INPUT_MSG_LEN);
        gcontrol_sur_msg_len = 0;
    }

    return 0;
}
/*$ Arcs_recv_fn()..........................................................*/
int Arcs_recv_fn(struct epoll_priv *priv);
int Arcs_recv_fn(struct epoll_priv *priv) {
    ArcsCommon_recvPacketPro();
    return 0;
}
/*$ prep_evt_desc().........................................................*/
int prep_evt_desc(int fd,handler_fn fn,
    struct epoll_priv *priv,struct epoll_event *ev)
{
    priv->fd = fd;
    priv->fn = fn;
    ev->events = EPOLLIN;
    ev->data.ptr = priv;

    return 0;
}
/*$ thread_fn().............................................................*/
int thread_fn(void *pgm) {
    struct fds *fn_fds = (struct fds *)pgm;
    int epollfd;
    struct epoll_event ev, epoll_evt[POLL_COUNT];
    struct epoll_priv fd_fns[POLL_COUNT];

    /* create epoll handle */
    epollfd = epoll_create(POLL_COUNT);

    /* register function handing */
    prep_evt_desc(timerfd_create(CLOCK_MONOTONIC, 0),
                &fn_timer_cb, &fd_fns[0],  &ev);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[0].fd, &ev );

    prep_evt_desc(fn_fds->raw_fd, &fn_netif_cb, &fd_fns[1], &ev);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[1].fd, &ev );

    prep_evt_desc( fn_fds->udp_server_fd, &udp_server_fn, &fd_fns[2], &ev);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[2].fd, &ev );

    prep_evt_desc(fn_fds->udp_client_fd, &udp_client_fn, &fd_fns[3], &ev);
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[3].fd, &ev);

    if( gmatrix_file_set_success) {
        prep_evt_desc(gmatrix_output_file,
            &matrix_control_uart_recv_fn, &fd_fns[4], &ev);
        epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[4].fd, &ev);
    }
    else {
        printf("init matrix uart recv thread handle failed!\n");
    }

    if (gcontrol_sur_fd > 0) {
        prep_evt_desc(gcontrol_sur_fd,
            &control_surface_recv_fn, &fd_fns[5], &ev);
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[5].fd, &ev);
    }
    else {
        printf("init gcontrol_sur_fd recv thread handle failed!\n");
    }
    if (EP0_readFd > 0) {
        prep_evt_desc(EP0_readFd,
        &Arcs_recv_fn, &fd_fns[6], &ev);
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[6].fd, &ev);
    }
    else {
        printf("init ARCS extern recv function failed!\n");
    }

    fcntl(fd_fns[0].fd, F_SETFL, O_NONBLOCK);
    timer_start_interval(fd_fns[0].fd);

    do {
        int i, res;

        struct epoll_priv *priv;
        res = epoll_wait(epollfd, epoll_evt, POLL_COUNT, -1);

        /* exit on error */
        if (-1 == res) {
            /* Interrupted by system call? */
            if (errno == EINTR) {  
                continue;  
            }

            DEBUG_ERR("epoll_wait error: ");
            return -errno;
        }

        if (system_stop || !m_isRunning) {
            printf("thread_fn return success......\n" );
            return 0;
        }

        for (i = 0; i < res; i++) {
            priv = (struct epoll_priv *)epoll_evt[i].data.ptr;
            if (priv->fn(priv) < 0) {
                return -1;
            }
        }
    }while (m_isRunning);

    return 0;
}
/*$ pthread_handle_create().................................................*/
int pthread_handle_create(pthread_t *h_trd, struct fds *kfds) {
    int rc;

    rc = pthread_create(h_trd, NULL, (void*)&thread_fn, kfds);
    if (rc) {
        printf("return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    return 0;
}

