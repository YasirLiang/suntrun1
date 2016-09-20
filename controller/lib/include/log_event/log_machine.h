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
#ifndef __LOG_MACHINE_H__
#define __LOG_MACHINE_H__

#include "log.h"/*include log base include file */
#include <semaphore.h>
#include <pthread.h>

/*! log machine class define */
typedef struct _type_log_machine {
	tstrlog_base log;/*! super log class */
	pthread_t h_thread;/*! log machine owner pthread */
        sem_t log_waiting;/*! log machine object*/
}tstrlog_imp, *tstrlog_pimp;

/*! Extern global object */
extern tstrlog_pimp gp_log_imp;
/*! Create log machine */
extern tstrlog_pimp log_machine_create(void (*callback_func)(void *,
    int32_t, const char *, int32_t), int32_t log_level, void *user_obj);
/*! destroy log machine */
extern void log_machine_destroy(void);

#endif /* __LOG_MACHINE_H__ */

