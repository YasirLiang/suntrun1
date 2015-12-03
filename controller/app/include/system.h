#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "linked_list_unit.h"
#include "entity.h"
#include "inflight.h"
#include "avdecc_main.h"

extern void init_system( void );			// ��ʼ��ϵͳ
extern void set_system_information( struct fds net_fd, struct udp_context* p_udp_net );
void system_close( struct threads_info *p_threads );

#endif

