/*common.h
**Copyright 2015 YasirLiang, Suntron.
**Author: YasirLiang
**date:2015/10/23
**
**this file about system common queue stype define.
**
**
**
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "jdksavdecc_world.h"

#define SUB_DATA_TYPE_SIZE 256

typedef struct data_element		// ����Ԫ��
{
	uint32_t data_len;
	uint8_t data_buf[SUB_DATA_TYPE_SIZE];
}data_element_type;

typedef struct func_cmd_work_queue 	// �����������������е�Ԫ�صĹ����ڵ�
{
	data_control control;	// ��������
	queue work;			// ��������
}work_queue;

#endif

