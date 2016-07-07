/*
  *File:controller_machine.h
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:系统控制器头文件
  */

#ifndef __CONTROLLER_MACHINE_H__
#define __CONTROLLER_MACHINE_H__

#include "jdksavdecc_world.h"
#include "unit_1722_network.h"

typedef struct controller_machine
{
	struct unit_network_1722 *unit_1722_net;
	//struct upper_computer_network *upper_net;
	//struct camera_network *camera_control_net;
	//struct matrix_network *matrix_control_net;
	//struct surface_network *surface_control_net;
}controller_machine,*controller_pmachine;

extern controller_pmachine gp_controller_machine;

extern int controller_machine_1722_network_send(const controller_pmachine p_machine, void *frame, int send_len);
extern int controller_machine_1722_network_recv(const controller_pmachine p_machine, void *frame, int recv_len);
extern void controller_machine_init(controller_pmachine p_machine,
							int (*net_1722_init)(void**),
							int (*net_1722_send)(void*, int, const void*),
							int (*net_1722_recv)(void*, int, const void*),
							int (*net_1722_cleanup)(void**));
extern controller_pmachine controller_machine_create(void);
extern void controller_machine_destroy(controller_pmachine *p_machine);

#endif
