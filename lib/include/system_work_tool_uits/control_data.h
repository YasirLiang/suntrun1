#ifndef __CONTOL_DATA_H__
#define __CONTOL_DATA_H__

#include "jdksavdecc_world.h"
#include <pthread.h>

typedef struct data_control
{
	pthread_mutex_t mutex; // �������
	pthread_cond_t cond;	// ��������������˯��
	int active;				// �����̴߳����ݽṹ�Ƿ�
}data_control;

bool controll_activate( data_control* p_controll );
bool controll_deactivate( data_control* p_controll );
bool controll_init( data_control *p_controll );
bool controll_destroy( data_control *p_controll );


#endif

