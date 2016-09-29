/*
* @file
* @brief system init and destroy
* @ingroup system
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-04
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/

/*Including files-----------------------------------------------------------*/
#include "system.h"
#include "host_controller_debug.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "terminal_pro.h"
#include "udp_client_controller_machine.h"
#include "message_queue.h"
#include "profile_system.h"
#include "send_common.h"
#include "send_work_queue.h"
#include "stream_descriptor.h"
#include "terminal_system.h"
#include "profile_system.h"
#include "muticast_connect_manager.h"
#include "check_timer.h"
#include "camera_pro.h"
#include "camera_common.h"
#include "matrix_output_input.h"/*Matrix output*/
#include "control_matrix_common.h"
#include "system_database.h"
#include "conference_transmit_unit.h"
#include "central_control_recieve_unit.h"
#include "connector_subject.h"
#include "avdecc_manage.h"
#include "log_machine.h"
#include "en485_send.h"
#include "global.h"
#include "lcd192x64.h"
#include "menu_f.h"
#include "central_control_transmit_unit.h"
#include "system_1722_recv_handle.h"
#include "controller_machine.h"

/*Global Objects declaration------------------------------------------------*/
extern int gcontrol_sur_fd;/* menu display control fd */
extern FILE *glog_file_fd;/* system log file fd*/
/*extern function declaration-----------------------------------------------*/
extern int lcd192x64_close(void);

/*$ system init before working..............................................*/
void init_system(void) {
    /* system endpoint double link list init */
    endpoint_list = init_endpoint_dblist(&endpoint_list);
    assert(endpoint_list != NULL);
    /* system inflight double link list init */
    command_send_guard = init_inflight_dblist(&command_send_guard);
    assert(command_send_guard != NULL);
    /* system descptor double link list init */
    init_descptor_dblist(&descptor_guard);
    assert(descptor_guard != NULL);
    /* acmp model init */
    acmp_endstation_init(command_send_guard, endpoint_list, descptor_guard);
    /* aecp model init */
    aecp_controller_init(endpoint_list, descptor_guard, command_send_guard);
    
    /* system profile init */
    init_profile_system_file();
    /* system state init */
    init_terminal_system_state();
    /* terminal proccess init */
    init_terminal_proccess_system();
    /* function work queue init */
    init_func_command_work_queue();
    /* upper computer model init */
    upper_computer_common_init();
    /* system subjector model init */
    init_connector_subjector();
    /* conference transmit unit proccess init */
    conference_transmit_model_init();
    /* conference recieve unit proccess init */
    central_control_recieve_uinit_init_list();
    /* CCU recieve unit proccess init */
    conference_recieve_uinit_proccess_init();
    /* CCU transmit unit proccess init */
    central_control_transmit_unit_model_pro_init();
    /* sem wait init*/
    init_sem_wait_can();
    /* sending working queue */
    init_network_send_queue();
    /* 1722 recv data handle proccesss */
    system_1722_recv_handle_init();
    /* extern data interval time */
    send_interval_init();
    /* inflight command args init */
    inflight_proccess_init();
    /* matrix output init */
    matrix_output_init();
    /* matrix model init */
    control_matrix_common_init();
    /* camera common control init */
    camera_common_control_init();
    /* camera preset file init */
    camera_pro_init();
    /* system database init */
    system_database_init();
    /* avdecc manage init */
    avdecc_manage_init();
    /* muticator model init */
    muticast_muticast_connect_manger_init();
    /* enable 485 port sending */
    en485_send_init();
#ifdef __ARM_BACK_TRACE__
    int fd = -1;
    int ret ;
    fd = UART_File_Open(fd,UART4);/* menu display control port is UART4 */
    if (fd == -1) {
        printf("Open Port Failed!\n");  
    }

    if ((UART_File_Init(fd, 9600, 0, 8, 1, 'N') != -1)
          && (fd != -1))/* menu display control port init */
    {
        gcontrol_sur_fd = fd;
    }

    ret = lcd192x64_init();/* port of lcd init */
    if (ret) {
        printf("lcd192x64_init fail\n");
        exit(1);
    }

    MenuInit(); /* init menu */
#endif
}
/*$set_system_information...................................................*/
void set_system_information(struct fds net_fd, 
                                                struct udp_context* p_udp_net)
{
    struct jdksavdecc_eui64 zero;
    bzero(&zero, sizeof(struct jdksavdecc_eui64));
    assert(p_udp_net);
    /* init udp client */
    init_udp_client_controller_endstation(net_fd.udp_server_fd,
                                               &p_udp_net->udp_srv.sock_addr);
    sleep(3);/* wait for net up */

    /* found all endpoints */
    adp_entity_avail(zero, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER);
}
/*$system_close.............................................................*/
void system_close(struct threads_info *p_threads) {
    int can_num = p_threads->pthread_nums;/* system pthread num */
    int i = 0, ret;
    /* exit muticast proccessing */
    muticast_muticast_connect_manger_pro_stop();
    /* en485 model clear*/
    en485_send_mod_cleanup();
    /* lcd ports close */
    lcd192x64_close();
     /* system controller machine */
    controller_machine_destroy(&gp_controller_machine);
    /* 1722 handle recv model destroy */
    system_1722_recv_handle_destroy();
    /* CCU recieve unit destroy */
    central_control_recieve_uinit_destroy();
    /* CCU transmit unit destroy */
    central_control_transmit_unit_model_destroy();
    /* conference transmit unit destroy */
    conference_transmit_unit_destroy();
    /* conference recieve unit destroy */
    conference_recieve_unit_destroy();
    /* kill system ptheads */
    for (i = 0; i < can_num; i++) {
        ret = pthread_kill(p_threads->tid[i], SIGQUIT);
        if (ret != 0) {
            if (errno == ESRCH) {
                DEBUG_INFO("An invalid signal was specified: tid[%d] ", i);
            }
            else if (errno == EINVAL) {
                DEBUG_INFO("no such tid[%d] thread to quit ", i);
            }
            else {
                /* no need */
            }
        }
    }

    /* endpiont double link list */
    destroy_endpoint_dblist(endpoint_list);
    if (endpoint_list != NULL) {
        free( endpoint_list );
        endpoint_list = NULL;
    }
    /* inflght command list destroy */
    destroy_inflight_dblist(command_send_guard);
    if (command_send_guard != NULL) {
        free( command_send_guard );
        command_send_guard = NULL;
    }

    /* 1722 descption */
    destroy_descptor_dblist(descptor_guard);
    if (descptor_guard != NULL) {
        free( descptor_guard );
        descptor_guard = NULL;
    }

    /* termianl double link list */
    terminal_system_dblist_destroy();

    /* queues of work pthread and sending pthread */
    destroy_func_command_work_queue();
    destroy_network_send_work_queue();

    /* save system profile */
    profile_system_close();// 保存配置文件的信息
    /* camera information destroy */
    camera_pro_system_close();
    /* camera control surface destroy */
    camera_common_control_destroy();
    /* matrix model destroy */
    matrix_control_destroy();
    /* close system database */
    system_database_destroy();
    /* terminal proccess destroy */
    terminal_proccess_system_close();
    /* log machine destroy */
    log_machine_destroy();
    /* clear all system fd */
    if (NULL != glog_file_fd) {
        Fclose(glog_file_fd);
    }
    close(net_fd.raw_fd);/* raw socket fd */
    close(net_fd.udp_client_fd);/* as client udp socket fd */
    close(net_fd.udp_server_fd);/* as server udp socket fd */
}

