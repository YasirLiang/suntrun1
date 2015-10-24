#ifndef __FUNC_PROCCESS_H__
#define __FUNC_PROCCESS_H__

#include "common.h"

typedef struct func_cmd_queue_data_type      // 会议命令处理函数队列的数据区 
{
	uint16_t func_index;				// 函数的偏移
	uint16_t func_cmd;					// 函数的命令
	data_element_type meet_msg; 		// 会议系统命令数据消息
}fcqueue_data_elem;

/* 工作节点的链表区(即队列节点);
这里封装在工作节点结构体中的第一个位置，
这样利于对队列节点数据区的存取操作
(在工作节点插入队列时只需把工作节点出强制转换为队列节点插入即可;
而取出节点后强制转换为工作节点即可)*/
typedef struct func_cmd_queue_work_node   // 会议命令处理函数命令队列的工作节点，即包含工作节点数据区与工作节点链表区(即队列节点)
{
	queue_node *next; 			// 工作队列节点的链表区(即队列节点)
	fcqueue_data_elem job_data; // 函数命令工作队列数据区 
}fcqueue_wnode;

typedef struct func_cmd_work_queue // 函数命令队列
{
	data_control control;	// 控制数据
	queue work;			// 工作队列
}fcwqueue;

typedef  struct func_link_items
{
	uint8_t user;					// 所属用户
	uint16_t cmd;					// 会议命令
	enum_func_link func_cmd_link;  	// 会议函数链接命令
	uint16_t func_cmd; 				// 函数命令 
}func_link_items;

typedef struct proccess_func_items
{
	uint32_t permit;				//允许执行的状态位表	
	int (*cmd_proccess)(uint16_t cmd, void *data, uint32_t data_len);
  	enum_func_link func_cmd_link;
}proccess_func_items;

#endif
