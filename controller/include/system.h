#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "linked_list_unit.h"
#include "entity.h"
#include "inflight.h"
#include "avdecc_main.h"

extern solid_pdblist endpoint_list;			// ϵͳ���ն������ڱ��ڵ�
extern inflight_plist command_send_guard;	// ϵͳ�з��������������������ڱ��ڵ�

extern void init_system( void );			// ��ʼ��ϵͳ
void set_system_information( struct fds net_fd, struct udp_context* p_udp_net );

#endif

