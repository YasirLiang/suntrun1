/*
*file:log_machine.h
*build date:2016-4-6
*
*/

/**
//����:��Ϣ�ɼ�ϵͳ���Դ�ļ�
//
//
*/

#ifndef __LOG_MACHINE_H__
#define __LOG_MACHINE_H__

#include "log.h"// ������־����ͷ�ļ�
#include <semaphore.h>
#include <pthread.h>

typedef struct _type_log_machine
{
	tstrlog_base log;
	pthread_t h_thread;
        sem_t log_waiting;
}tstrlog_imp, *tstrlog_pimp;

extern tstrlog_pimp gp_log_imp;

extern tstrlog_pimp log_machine_create( void (*callback_func)(void *, int32_t, const char *, int32_t), int32_t log_level, void *user_obj );
extern void log_machine_destroy( void );

#endif

