#ifndef __CONTOL_DATA_H__
#define __CONTOL_DATA_H__

#include <pthread.h>
#include "jdksavdecc_world.h"

typedef struct data_control
{
	pthread_mutex_t mutex; // �������
	pthread_cond_t cond;	// ��������������˯��
	int active;				// �����̴߳����ݽṹ�Ƿ�
}data_control;

#endif

