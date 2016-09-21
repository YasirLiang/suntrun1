/*
* @file
* @brief log machine
* @ingroup log machine
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2016-09-20
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
#include "log_machine.h"
#include "enum.h"
#include "host_controller_debug.h"
/*Local Objects-------------------------------------------------------------*/
static bool l_logRun;
/*Golbal Objects------------------------------------------------------------*/
tstrlog_pimp gp_log_imp = NULL;
/*$ log_machine_post_event..................................................*/
void log_machine_post_event(void) {
    assert(gp_log_imp != NULL);
    if (gp_log_imp != NULL) {
        sem_post(&gp_log_imp->log_waiting);/* post message */
    }
}
/*$ log_machine_thread......................................................*/
void *log_machine_thread(void* param) {
    uint32_t write_index, read_index;/* write and read index */
    int32_t level, m;/* log level and stamp ms */
    char *pMsg; /* pointer to message buffer. */
    
    while (l_logRun) {
        sem_wait(&gp_log_imp->log_waiting);
        write_index = gp_log_imp->log.write_index;
        read_index = gp_log_imp->log.read_index;
        level = gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].level;
        pMsg = gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].msg;
        m = gp_log_imp->log.log_buf[read_index % LOG_BUF_COUNT].time_stamp_ms;
        if ((write_index- read_index) > 0) {
            gp_log_imp->log.callback_func(gp_log_imp->log.user_obj,
                                                              level, pMsg, m);
            gp_log_imp->log.read_index++;
            pMsg = NULL; /*reset pMsg pointer */
        }
        else {
            break;
        }
    }
    return NULL;
}
/*$ log_machine_create......................................................*/
tstrlog_pimp log_machine_create(void (*callback_func)(void *,
           int32_t, const char *, int32_t), int32_t log_level, void *user_obj)
{
    gp_log_imp = malloc(sizeof(tstrlog_imp));
    if (gp_log_imp == NULL) {
        printf("malloc failed: No space for tstrlog_imp malloc\n");
    }
    else {
        gp_log_imp->log.log_init = log_init;
        gp_log_imp->log.log_init(&gp_log_imp->log);/* initial log machine */
        gp_log_imp->log.set_log_level = set_log_level;
        gp_log_imp->log.post_log_msg = post_log_msg;
        gp_log_imp->log.post_log_event = log_machine_post_event;
        gp_log_imp->log.set_log_callback = set_log_callback;
        gp_log_imp->log.missed_log_event_count = missed_log_event_count;
        gp_log_imp->log.callback_func = callback_func;
        gp_log_imp->log.log_level = log_level;
        gp_log_imp->log.user_obj = user_obj;

        sem_init(&gp_log_imp->log_waiting, 0, 0);
        l_logRun = true;/* make log run flag */

        int rc;
        rc = pthread_create(&gp_log_imp->h_thread, NULL,
                                                    log_machine_thread, NULL);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            if (NULL != gp_log_imp) {
                free(gp_log_imp);
                gp_log_imp = NULL;
            }
        }
    }
    return gp_log_imp;
}
/*$ log_machine_destroy.....................................................*/
void log_machine_destroy(void) {/* destroy log machine and its pthread */
    if (NULL != gp_log_imp) {
        l_logRun = false;
        gp_log_imp->log.post_log_event();
        free(gp_log_imp);
        gp_log_imp = NULL;
    }
}

