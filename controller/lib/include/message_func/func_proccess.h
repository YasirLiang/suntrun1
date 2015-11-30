/*func_proccess.h
**
**
**
**
**
*/


#ifndef __FUNC_PROCCESS_H__
#define __FUNC_PROCCESS_H__

#include "common.h"
#include "enum.h"
#include <semaphore.h>

#define MAX_FUNC_MSG_LEN 512

typedef struct func_message_head
{
	uint16_t func_index;				// ������ƫ��
	uint16_t func_cmd;					// ����������
}func_message_head;

typedef struct func_cmd_queue_data_type      // ��������������е������� 
{
	func_message_head func_msg_head;
	data_element_type meet_msg; 		// ����ϵͳ����������Ϣ
}fcqueue_data_elem;

/* �����ڵ��������(�����нڵ�);
�����װ�ڹ����ڵ�ṹ���еĵ�һ��λ�ã�
�������ڶԶ��нڵ��������Ĵ�ȡ����
(�ڹ����ڵ�������ʱֻ��ѹ����ڵ��ǿ��ת��Ϊ���нڵ���뼴��;
��ȡ���ڵ��ǿ��ת��Ϊ�����ڵ㼴��)*/
typedef struct func_cmd_queue_work_node   // �����������������еĹ����ڵ㣬�����������ڵ��������빤���ڵ�������(�����нڵ�)
{
	queue_node *next; 			// �������нڵ��������(�����нڵ�)
	fcqueue_data_elem job_data; // ������������������� 
}fcqueue_wnode, *p_fcqueue_wnode;

typedef struct func_cmd_work_queue // �����������
{
	data_control control;	// ��������
	queue work;			// ��������
}fcwqueue;

typedef  struct func_link_items
{
	uint8_t user;					// �����û�
	uint16_t cmd;					// ��������
	enum_func_link func_cmd_link;  	// ���麯����������
	uint16_t func_cmd; 				// �������� 
}func_link_items;

typedef struct proccess_func_items
{
	uint32_t permit;				//����ִ�е�״̬λ��	
	int (*cmd_proccess)( uint16_t cmd, void *data, uint32_t data_len );
  	enum_func_link func_cmd_link;
}proccess_func_items;

extern fcwqueue fcwork_queue;		// ����������Ϣ��������

extern uint8_t get_sys_state( void );
extern void init_func_command_work_queue( void );
extern int func_command_find_and_run( proccess_func_items * func_tables, fcwqueue*  p_func_wq );
extern bool find_func_command_link( uint8_t user, uint16_t cfc_cmd, uint16_t func_cmd, const uint8_t *pdata, const uint16_t data_len );
extern bool use_dis_set( uint8_t  user, bool set );


#endif
