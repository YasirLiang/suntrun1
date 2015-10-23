#ifndef __FUNC_COMMAND_H__
#define __FUNC_COMMAND_H__

#include "common.h"

typedef struct func_cmd_queue_data_type      // ��������������е������� 
{
	uint16_t func_index;				// ������ƫ��
	uint16_t func_cmd;				// ����������
	data_element_type meet_msg; 		// ����ϵͳ����������Ϣ
}fcqueue_data_elem;

typedef struct func_cmd_queue_work_node   // �����������������еĹ����ڵ㣬�����������ڵ��������빤���ڵ�������(�����нڵ�)
{
/* �����ڵ��������(�����нڵ�);
�����װ�ڹ����ڵ�ṹ���еĵ�һ��λ�ã�
�������ڶԶ��нڵ��������Ĵ�ȡ����
(�ڹ����ڵ�������ʱֻ��ѹ����ڵ��ǿ��ת��Ϊ���нڵ���뼴��;
��ȡ���ڵ��ǿ��ת��Ϊ�����ڵ㼴��)*/
	queue_node *next; 			// �������нڵ��������(�����нڵ�)
	fcqueue_data_elem job_data; // ������������������� 
}fcqueue_wnode;

typedef struct func_cmd_work_queue // �����������
{
	data_control control;	// ��������
	queue work;			// ��������
}fcwqueue;

#endif
