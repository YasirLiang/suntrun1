#ifndef __CONTOL_DATA_H__
#define __CONTOL_DATA_H__

typedef struct data_control
{
	pthread_mutex_t mutex; // �������
	pthread_mutex_t cond;	// ��������������˯��
	int active;				// �����̴߳����ݽṹ�Ƿ�
}data_control;

#endif
