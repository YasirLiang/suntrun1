#ifndef __FUNC_COMMAND_H__
#define __FUNC_COMMAND_H__

#include "common.h"

typedef struct func_cmd_queue_wnode   // �����������������е�Ԫ�صĹ����ڵ�
{
	uint16_t func_index;				// ������ƫ��
	uint16_t func_cmd;					// ����������
	data_element_type meet_msg; 		// ����ϵͳ����������Ϣ
	
}fcqueue_wnode;


#endif
