/*
* @file
* @brief sending external data
* @ingroup system send pthread
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
#include "send_pthread.h"
#include <time.h>
#include "time_handle.h"
#include "send_common.h"

/*sending data invteval Macro-----------------------------------------------*/
#define SEND_INTERVAL_TIMEOUT 5
/*Lacal Objects-------------------------------------------------------------*/
static uint8_t send_frame[TRANSMIT_DATA_BUFFER_SIZE];
/*Static function declaration-----------------------------------------------*/
static int thread_send_func( void *pgm );
/*Global Objecsts-----------------------------------------------------------*/
volatile bool gsend_pro_idle = true; /*stop flags of sending*/
/*Extern funcion declaraction-----------------------------------------------*/
extern bool inflight_list_has_command(void);

/*$thread_send_func.........................................................*/
static int thread_send_func(void *pgm) {
    (void)pgm;/*avoid waning when being compiled*/
    sdpwqueue *p_send_wq = &net_send_queue;
    assert(p_send_wq);
    over_time_set(SYSTEM_SQUEUE_SEND_INTERVAL, 
                    SEND_INTERVAL_TIMEOUT); /*set to send interval*/

    while (1) {
        uint8_t *frame;/*point to frame buf*/
        bool is_resp_data;/*respond flags of send data*/
        uint8_t data_type;/*type of sending data*/
        uint32_t msg_type;/*message type of frame data*/
        uint8_t dest_raw[6];/*message sending destination*/
        bool notification_flag;/*notification flag*/
        p_sdpqueue_wnode p;/*queue node*/
        uint16_t send_frame_len;/*frame len of sending data*/
        struct sockaddr_in udp_sin;/*udp sending destination contex*/
        uint32_t resp_interval_time;/*responding interval time*/
        bool write_empty, read_empty; /* queue empty flag writing or reading*/
        
        if (!over_time_listen(SYSTEM_SQUEUE_SEND_INTERVAL)) {
            continue;
        }

        write_empty = is_queue_empty(&gwrite_send_queue.work);
        read_empty = is_queue_empty(&p_send_wq->work);

        /*
          *swap queue when queue for writing is not empty and
          *for reading is empty
          */ 
        if (read_empty && !write_empty) {
            pthread_mutex_lock(&p_send_wq->control.mutex);
            swap_sdpqueue(p_send_wq, &gwrite_send_queue);
            pthread_mutex_unlock(&p_send_wq->control.mutex);
        }

        if (read_empty && write_empty) {/*All queue empty?*/
            pthread_mutex_lock(&p_send_wq->control.mutex);
            pthread_cond_wait(&p_send_wq->control.cond, 
                    &p_send_wq->control.mutex);/*wait for cond sign*/
            pthread_mutex_unlock(&p_send_wq->control.mutex);
            continue;
        }

        /*for getting queue data*/
        p = send_queue_message_get(p_send_wq);
        if (p_send_wq->work.head == NULL) {
            /*while queue is empty,
              *make sure the queue trail not to be used again*/
            if (p_send_wq->work.trail != NULL) {
                p_send_wq->work.trail = NULL;
            }
        }

        if (NULL == p)
        {
            DEBUG_INFO("No send queue message: ERROR!");
            continue;
        }

        send_frame_len = p->job_data.frame_len;
        data_type = p->job_data.data_type;
        notification_flag = p->job_data.notification_flag;
        is_resp_data = p->job_data.resp;
        frame = p->job_data.frame;
        msg_type = jdksavdecc_common_control_header_get_control_data(frame,
                                                      ZERO_OFFSET_IN_PAYLOAD);
        if (send_frame_len > TRANSMIT_DATA_BUFFER_SIZE) {
            if (frame != NULL) {
                free(frame);
                frame = NULL;
            }

            if (NULL != p) {
                free(p); /*free queue node*/
                p = NULL;
            }

            continue;
        }

        memset(send_frame, 0, sizeof(send_frame));
        memcpy(send_frame, frame, send_frame_len);
        memcpy(dest_raw, p->job_data.raw_dest, 6);
        memcpy(&udp_sin, &p->job_data.udp_sin, sizeof(struct sockaddr_in));
        if (frame != NULL) {
            free (frame);
            p->job_data.frame = NULL;
        }

        if (NULL != p) {
            free(p); /*free queue node*/
            p = NULL;
        }

        /* ready to sending data*/
        pthread_mutex_lock(&ginflight_pro.mutex);
        tx_packet_event(data_type, notification_flag, send_frame,
                                send_frame_len, &net_fd, command_send_guard,
                                dest_raw, &udp_sin, is_resp_data,
                                &resp_interval_time);
        pthread_mutex_unlock(&ginflight_pro.mutex);

        if (is_resp_data 
            && (msg_type !=\
                   JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND))
        {/*checking send status*/
            over_time_set(SYSTEM_SQUEUE_SEND_INTERVAL, SEND_INTERVAL_TIMEOUT);
        }
        else
        {
            if ((data_type == TRANSMIT_TYPE_AECP)
                && (msg_type == \
                       JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND))
            {
                int status = 0;
                status = set_wait_message_primed_state();
                assert(status == 0);
                status = set_wait_message_active_state();
                assert(status == 0);
                gsend_pro_idle = false;

                over_time_set(SYSTEM_SQUEUE_SEND_INTERVAL, 200);
                while (!gsend_pro_idle && !is_resp_data) {
                    if (!inflight_list_has_command()) {
                        break;
                    }

                    if (over_time_listen(SYSTEM_SQUEUE_SEND_INTERVAL)) {
                        break;
                    }

                    continue;
                }

                status = set_wait_message_idle_state();
                assert(status == 0);
                over_time_set(SYSTEM_SQUEUE_SEND_INTERVAL, 10);
            }
            else {
                over_time_set(SYSTEM_SQUEUE_SEND_INTERVAL,
                                SEND_INTERVAL_TIMEOUT);
            }
        }
    }

    return 0;
}

/*$pthread_send_network_create..............................................*/
int pthread_send_network_create(pthread_t *send_pid) {
        int rc = 0;
        assert( send_pid );
	rc = pthread_create(send_pid, NULL, (void*)&thread_send_func, NULL);
	if(rc != 0) {
		DEBUG_INFO("send pthread create Failed: %d\n", rc);
		assert(rc == 0);
	}
	return 0;
}

