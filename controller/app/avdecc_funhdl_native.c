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
#include "terminal_pro.h"
#include "muticast_connect_manager.h"
#include "global.h"
#include <pthread.h>
#include "func_proccess.h"
#include "avdecc_funhdl_native.h"
#include "arcs_common.h" /*$ for arcs interface */
#include "central_control_recieve_unit.h"
#include "conference_transmit_unit.h"
#include "queue_com.h"

//#define KEY_ACT_QUEUE_ENABLE
#ifdef KEY_ACT_QUEUE_ENABLE
struct keyData {
    uint16_t func_index;
    uint16_t func_cmd;
    uint32_t data_len;
    TUserTimer timer;
    uint8_t func_data[SUB_DATA_TYPE_SIZE];
};
#define KEY_ACT_QSIZE 32
static uint32_t l_keyQueueBuf[KEY_ACT_QSIZE] = {0};
static TComQueue l_keyActQueue = {
    0U, 0U, 0U, KEY_ACT_QSIZE, l_keyQueueBuf
};
#endif

static uint32_t l_sysTick;

/*Macro INPUT_MSG_LEN for control surface message len-----------------------*/
#define INPUT_MSG_LEN	6
/*Macro SYS_BUF_RECV_COUNT buffer counts------------------------------------*/
#define SYS_BUF_RECV_COUNT 3
/*Extern function delaration------------------------------------------------*/
extern void input_recv_pro(unsigned char *p_buf, unsigned recv_len);
/*Extern Global varialable delaration-----------------------------------------*/
extern unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN]; 
/*Extern Global varialable delaration-----------------------------------------*/
extern volatile unsigned char gcontrol_sur_msg_len;
extern volatile bool m_isRunning;

/*$ function declaration----------------------------------------------------*/
void log_link(bool upFlag);
void avdecc_check_link(void);

#ifdef KEY_ACT_QUEUE_ENABLE
void keyActProcess(void);

/*$ keyActProcess().........................................................*/
void keyActProcess(void) {
    static uint32_t l_lastTick;
    if ((l_sysTick - l_lastTick) > 100U) { /* 100 ms*/
        uint32_t addr;
        struct keyData *ptr;
        
        if (QueueCom_popFiFo(&l_keyActQueue, &addr)) {
            ptr = (struct keyData *)addr;
            if (ptr != NULL) {
                if (!userTimerTimeout(&ptr->timer)) {
                    proccess_func_link_tables[ptr->func_index].cmd_proccess(
                        ptr->func_cmd, ptr->func_data, ptr->data_len);

                    l_lastTick = l_sysTick;
                }

                free(ptr);
                ptr = NULL;
            }
        }
    }
}
#endif

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
            pthread_mutex_unlock(&p_func_wq->control.mutex);
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
            pthread_mutex_unlock(&p_func_wq->control.mutex);
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
        pthread_mutex_unlock(&p_func_wq->control.mutex);
        
#ifdef KEY_ACT_QUEUE_ENABLE        
        if (func_index == FUNC_TMNL_KEY_ACTION) {
            struct keyData * ptr;
            ptr = (struct keyData *)malloc(sizeof(struct keyData));
            if (ptr != NULL) {
                ptr->func_index = func_index;
                ptr->func_cmd = func_cmd;
                ptr->data_len = data_len;
                memcpy(ptr->func_data, func_data, data_len);
                userTimerStart(200U, &ptr->timer);

                if (!QueueCom_postFiFo(&l_keyActQueue, (void *)ptr)) {
                    free(ptr);
                }
            }

            continue;
        }
#endif

        /* run function command */
        p_func_items[func_index].cmd_proccess(func_cmd,
            func_data, data_len);
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
    while(m_isRunning) {
        unsigned long us_per_ms = 1000U;
        unsigned long interval_ms = 1U;
        struct timeval tempval;

        tempval.tv_sec = interval_ms / 1000U;
        tempval.tv_usec = (interval_ms % 1000U) * us_per_ms;
        select(0, NULL, NULL, NULL, &tempval); /*wait for a time*/

        l_sysTick++;

        if (static_buf_num >= SYS_BUF_RECV_COUNT) {
            static_buf_num = 0;
        }
        switch ((static_buf_num++) % SYS_BUF_RECV_COUNT) {
            case 0: /* switch to buffer 1 for proccessing recieve data */
            case 1: /* switch to buffer 2 for proccessing recieve data */
            case 2: {  /* arcs */
                /* matrix process */
                control_matrix_common_recv_message_pro();
                
                if (gregister_tmnl_pro.rgs_state == RGST_IDLE) {
                    pthread_mutex_lock(&ginflight_pro.mutex);
                    /* proccessing the data from upper computer */
                    upper_computer_recv_message_get_pro();
                    pthread_mutex_unlock(&ginflight_pro.mutex);
                    
                    /* for arcs process */
                    ArcsCommon_process();
                }
                
                break;
            }
            default: {/* will never come this case */
                break;
            }
        }

        /* stop pthread? */
        if (!m_isRunning) {
            break;
        }
        
        /* proccess termianl point register */
        system_register_terminal_pro();
        if (gregister_tmnl_pro.rgs_state == RGST_IDLE) {
            /* process terminal of not signing success */
            terminal_sign_in_pro();
            /* process voting */
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
#ifndef DISABLE_MUTI
            /* muticast connect table manager */
            muticast_connect_manger_timeout_event_image();
#endif
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
        
        Terminal_comPro(l_sysTick);
        
        terminal_over_time_firstapply_pro();

        /* check link */
        avdecc_check_link();
        
#ifdef KEY_ACT_QUEUE_ENABLE
        /* process key active */
        keyActProcess();
#endif /* KEY_ACT_QUEUE_ENABLE */
    }	
    /* exit pthread */
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
/*$ */
void log_link(bool upFlag) {
    char buf[2048] = {0};
    
    FILE* ffd = Fopen("linkSta.log", "ab+");
    if (ffd != NULL) {
        time_t tem = time(NULL);
        struct tm *t = (struct tm*)localtime(&tem);/* get local time */
        
        /* first reset buffer */
        memset(buf, 0, sizeof(buf));
        /* format new line information */
        sprintf(buf, "[%d-%d-%d %d:%d:%d LOG]  %s\n",
                t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour,
                t->tm_min, t->tm_sec,
                upFlag ? "[ Link is Up ]" : "[ Link is Down]");
        
        fputs(buf, ffd);/* write to file */
        Fflush(ffd); /* flush buffer to file */
    }
    
    if (ffd != NULL) {
        Fclose(ffd);
    }
}
/*$ */
extern int check_ifrPhyLinkStatus(char *nicName);
void avdecc_check_link(void) {
    static bool l_up = 0; /* true are up */ 
    static TUserTimer l_tm = {
        (bool)1, (bool)0, 1000, (Timestamp)0
    }; /* begin timer */

    if (userTimerTimeout(&l_tm)) {
        int flag = check_ifrPhyLinkStatus(NETWORT_INTERFACE);
        if (flag == 0) {
            /* change to up */
            if (!l_up) {
                l_up = (bool)1;
                log_link(l_up);
            }
        }
        else {
            if (l_up) {
                /* change to down */
                l_up = (bool)0;
                log_link(l_up);
            }
        }

        /* update timer */
        userTimerStart(1000, &l_tm);
    }
}

