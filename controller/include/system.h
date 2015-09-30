#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "linked_list_unit.h"
#include "entity.h"
#include "inflight.h"

extern solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
extern inflight_plist command_send_guard;	// 系统中发送网络数据命令链表哨兵节点

extern void init_system( void );			// 初始化系统

#endif

