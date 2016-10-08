/*
* @file
* @brief avdecc funhdl native
* @ingroup avdecc funhdl native
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-21
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
#include "avdecc_main.h"
#include "system_packet_tx.h"
#include "host_controller_debug.h"
#include "message_queue.h"
#include "send_work_queue.h"
#include "system_packet_tx.h"
#include "send_common.h"
#include "control_matrix_common.h"
#include "time_handle.h"
#include "avdecc_manage.h"/* discover and remove terminal */
#include "send_common.h" /* Including SEND_DOUBLE_QUEUE_EABLE*/
#include "camera_pro.h"
#include "system_1722_recv_handle.h"
#include "terminal_pro.h"
#include "muticast_connect_manager.h"
#include "global.h"
#include <pthread.h>
#include "func_proccess.h"
#include "avdecc_funhdl_native.h"

/*Macro INPUT_MSG_LEN for control surface message len-----------------------*/
#define INPUT_MSG_LEN	6
/*Macro SYS_BUF_RECV_COUNT buffer counts------------------------------------*/
#define SYS_BUF_RECV_COUNT 2
/*Extern function delaration------------------------------------------------*/
extern void input_recv_pro(unsigned char *p_buf, unsigned recv_len);
/*Extern Global varialable delaration-----------------------------------------*/
extern unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN]; 
/*Extern Global varialable delaration-----------------------------------------*/
extern volatile unsigned char gcontrol_sur_msg_len;

/*$ command function thread proccess........................................*/
int thread_func_fn(void * pgm) {
    /* pointer to function working queue */
    fcwqueue* p_func_wq = &fcwork_queue;
    assert( pgm != NULL && p_func_wq != NULL);
    /* command funtion table pointer */
    proccess_func_items *p_func_items = (proccess_func_items *)pgm;
    /* queue node pointer of  command queue */
    fcqueue_wnode *p_msg_wnode = NULL;

    /* The thread will be waked up by other threads which save queue
    node to command working queue.When the thread wake up by
    recieving pthread cond signal, this thread will run the command
    function according to the command function index from queue node
    */
    while (1) {
        /* lock queue before operating first */
        pthread_mutex_lock(&p_func_wq->control.mutex);
        while ((p_func_wq->control.active)
                    && (p_func_wq->work.head == NULL))
        {
            /* thread sleep until recieving pthread cond signal */
            pthread_cond_wait(&p_func_wq->control.cond,
                                                   &p_func_wq->control.mutex);
        }

        if (!p_func_wq->control.active) {
            /* unlock queue because the queue not active */
            pthread_mutex_unlock( &p_func_wq->control.mutex);
            break;
        }
        /* get queue node from queue */
        p_msg_wnode = func_command_work_queue_messag_get(p_func_wq);
        if (p_func_wq->work.head == NULL) {
            if (p_func_wq->work.trail != NULL) {
                /*while queue is empty,
                make sure the queue trail not to be used again*/
                p_func_wq->work.trail = NULL;
            }
        }
        if (NULL == p_msg_wnode) {
            /* before continue must unlock the mutex */
            pthread_mutex_unlock(&p_func_wq->control.mutex);
            continue;/*get wrong node from the queue*/
        }

        /* get func command queue message from work node */
        uint16_t func_index = p_msg_wnode->job_data.func_msg_head.func_index;
        uint16_t func_cmd = p_msg_wnode->job_data.func_msg_head.func_cmd;
        uint32_t data_len = p_msg_wnode->job_data.meet_msg.data_len;
        uint8_t func_data[SUB_DATA_TYPE_SIZE] = {0};/* reset buffer to zero */
        if (data_len > SUB_DATA_TYPE_SIZE) {
            if (NULL != p_msg_wnode) {
                free(p_msg_wnode);/*free space*/
                p_msg_wnode = NULL;/*make NULL*/
            }
            /* before continue must unlock the mutex */
            pthread_mutex_unlock( &p_func_wq->control.mutex);
            continue;/* get data length */
        }
        /* copy function data */
        memcpy(func_data, p_msg_wnode->job_data.meet_msg.data_buf, data_len);
        if (NULL != p_msg_wnode) {
            free( p_msg_wnode );
            p_msg_wnode = NULL;
        }
        /* before run command  must unlock the mutex for other thread can save
            comand function node to this queue in the meantime */
        pthread_mutex_unlock( &p_func_wq->control.mutex );
        /* run function command */
        p_func_items[func_index].cmd_proccess(func_cmd, func_data, data_len);
    }

    return 0;
}
/*$ for create the thread of command function...............................*/
int pthread_handle_cmd_func(pthread_t *pid,
                                           const proccess_func_items *p_items)
{
    int rc = 0;   /* pthread_create function return value */
    assert(NULL != pid); /* make sure pid pointer is available */
    rc = pthread_create(pid, NULL, (void*)&thread_func_fn, (void*)p_items);
    if (rc) {
        DEBUG_INFO("thread Create failed: rc = %d", rc);
        assert(rc == 0);
    }
    return 0;
}
/*$ thread funtion..........................................................*/
int pthread_recv_data_fn(void *pgm) {
    static int static_buf_num = 0;/*for change buffer*/
    while(1) {
        unsigned long us_per_ms = 1000;
        unsigned long interval_ms = 0;
        struct timeval tempval;

        tempval.tv_sec = interval_ms/1000;  
        tempval.tv_usec = (interval_ms%1000)*us_per_ms;
        select( 0, NULL, NULL, NULL, &tempval );/*wait for a time*/

        if (static_buf_num >= SYS_BUF_RECV_COUNT) {
            static_buf_num = 0;
        }

        switch ((static_buf_num++) % SYS_BUF_RECV_COUNT) {
            case 0: {/* switch to buffer 1 for proccessing recieve data */
                if (gregister_tmnl_pro.rgs_state == RGST_IDLE) {
                    pthread_mutex_lock(&ginflight_pro.mutex);
                    /* proccessing the data from upper computer */
                    upper_computer_recv_message_get_pro();
                    pthread_mutex_unlock(&ginflight_pro.mutex);
                }
                break;
            }
            case 1: {/* switch to buffer 2 for proccessing recieve data */
                control_matrix_common_recv_message_pro();
                break;
            }
            default: {/* will never come this case */
                break;
            }
        }

        /* proccess termianl point register */
        system_register_terminal_pro();
        if (gregister_tmnl_pro.rgs_state == RGST_IDLE) {
            /* proccess terminal of not signing success */
            terminal_sign_in_pro();
            /* proccess voting */
            terminal_vote_proccess();
            /* query the result of sign and vote */
            terminal_query_sign_vote_pro();
/* enable arm version when __ARM_BACK_TRACE__ is defined */
#ifdef __ARM_BACK_TRACE__
            /*proccessing the menu display according to the data recieving from*/
            if (gcontrol_sur_msg_len > 0) {
                input_recv_pro(gcontrol_sur_recv_buf, gcontrol_sur_msg_len);
                gcontrol_sur_msg_len = 0;
            }
#endif
            /* muticast connect table manager */
            muticast_connect_manger_timeout_event_image();
        }
        
        /* avdecc discover terminal managing */
        avdecc_manage_discover_proccess();
        /* check send queue is empty? */
        send_common_check_squeue();
        /* camera proccessing */
        camera_pro();
        /* speaking timeout proccessing */
        terminal_over_time_speak_pro();
        /* microphone state set later */
        terminal_after_time_mic_state_pro();
        /*mic state set callback proccessing */
        Terminal_micCallbackPro();
    }	
    return 0;
}
/*$ create the thread for proccessing recieve data..........................*/
int pthread_proccess_recv_data_create(pthread_t *pid, void * pgm) {
    int rc = 0; /* pthread_create function return value */
    assert(NULL != pid);/* make sure pid pointer is available */
    rc = pthread_create(pid, NULL, (void*)&pthread_recv_data_fn, NULL);
    if(rc) {
        DEBUG_INFO("recv thread Create failed: rc = %d", rc);
        assert(rc == 0);
    }
    return 0;
}

