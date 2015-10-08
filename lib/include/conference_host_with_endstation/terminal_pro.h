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


typedef struct _tterminal_state_set // 终端状态的设置
{
	uint8_t :1;
	uint8_t one_off:1;
	uint8_t sign_stype:1;
	uint8_t auto_close:1;
	uint8_t sys:4;
	
	uint8_t :3;
	uint8_t keydown:5;

	uint8_t :3;
	uint8_t keyup:5;
	
	uint8_t MicClose:1;
	uint8_t :2;
	uint8_t VoteType:5;
}tmnl_state_set;

typedef struct _tterminal_led_show_stype // 设置终端 led 显示方式
{
	uint8_t blink:1;
	uint8_t bright_lv:4;
	uint8_t page_show_state:3;
	uint8_t speed_roll:4;
	uint8_t stop_time:4;
}tmnl_led_state_show_set;

#endif

