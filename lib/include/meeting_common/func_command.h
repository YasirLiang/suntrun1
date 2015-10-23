#ifndef __FUNC_COMMAND_H__
#define __FUNC_COMMAND_H__

#include "common.h"

typedef struct func_cmd_queue_wnode   // 会议命令处理函数命令队列的元素的工作节点
{
	uint16_t func_index;				// 函数的偏移
	uint16_t func_cmd;					// 函数的命令
	data_element_type meet_msg; 		// 会议系统命令数据消息
	
}fcqueue_wnode;


#endif
