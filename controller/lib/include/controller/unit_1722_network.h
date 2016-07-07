/*File:unit_1722_network.h
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:与1722终端通信接口文件
  */

#ifndef __UNIT_1722_NETWORK_H__
#define __UNIT_1722_NETWORK_H__

#include "jdksavdecc_world.h"
#include "network_extern.h"

enum network_1722_flags
{
	NET_1722_NOT_INIT,
	NET_1722_OK,
};

struct unit_network_1722
{
	struct network_extern network_1722;
	void *network_1722_user_obj;// 初始化1722发送接收数据接口后的用户信息
	int running;// 0:not running; 1:runing
	enum network_1722_flags enum_flag;
};

extern struct unit_network_1722* unit_1722_network_create(void);
extern int unit_1722_network_init(struct unit_network_1722* network, 
							int (*network_init)(void**), 
							int (*network_send)(void*, int, const void*), 
							int (*network_recv)(void*, int, const void*), 
							int (*network_cleanup)(void**));
extern int unit_1722_network_send(struct unit_network_1722* network, void *frame, int send_len, const void*usr_obj);
extern int unit_1722_network_recv(struct unit_network_1722* network, void *frame, int recv_len, const void*usr_obj);
extern int unit_1722_network_destroy(struct unit_network_1722** network, void**usr_obj);

 #endif
