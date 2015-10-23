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

typedef struct data_element		// 数据元素
{
	uint32_t data_len;
	uint8_t data_buf[SUB_DATA_TYPE_SIZE];
}data_element_type;

typedef struct func_cmd_work_queue 	// 会议命令处理函数命令队列的元素的工作节点
{
	data_control control;	// 控制数据
	queue work;			// 工作队列
}work_queue;

#endif

