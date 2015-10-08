#ifndef __TERMINAL_PRO_H__
#define __TERMINAL_PRO_H__

#include "jdksavdecc_world.h"

#define SYSTEM_TMNL_MAX_NUM 450	// 系统中终端最大的数量

typedef struct tterminal_addr_list	// 终端分配地址
{
	uint16_t addr;
	uint8_t  tmn_type;
}terminal_address_list;

typedef struct _tterminal_address_list_pro // 终端分配地址参数
{
	uint16_t addr_start;
	uint16_t index;
	uint16_t renew_flag;
}terminal_address_list_pro;

typedef  struct _tterminal_state	// 终端的状态，一个终端对应一个状态
{
	uint8_t device_type;		// 设备类型
	uint8_t mic_state;			// 麦克风状态
	bool is_rgst;				// 终端报到参数, 在分配地址完成后
	uint8_t sys_state;			// 终端在系统中的状态
	uint8_t sign_state;			// 终端的签到状态
	uint8_t vote_state;			// 投票状态
}terminal_state; 

typedef  struct _tterminal
{
	uint64_t entity_id;			// 实体ID
	uint16_t addr;				// 终端地址
	terminal_state tmnl_status;	// 终端的状态 
}conference_terminal_device; 

typedef  struct tmnl_list_node		// 终端链表节点 
{
	conference_terminal_device tmnl_dev;
	struct tmnl_list_node *next,*prior;
}tmnl_dblist, *tmnl_pdblist; 

#endif

