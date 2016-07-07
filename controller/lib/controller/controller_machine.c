/*
  *File:controller_machine.c
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:系统控制器
  */

#include "controller_machine.h"

controller_pmachine gp_controller_machine = NULL;

/*{@camera_control network*/
/////////////////////////////////////
/////////////////////////////////////
int controller_machine_init_camera_control_network(void)
{
	int ret = -1;
	
	return ret;
}
/////////////////////////////////////
/////////////////////////////////////
/*camera_control network@}*/

/*{@upper_computer*/
/////////////////////////////////////
/////////////////////////////////////
int controller_machine_init_upper_computer_network(void)
{
	int ret = -1;
	
	return ret;
}
/////////////////////////////////////
/////////////////////////////////////
/*upper_computer@}*/

/*{@matrix_control network*/
/////////////////////////////////////
/////////////////////////////////////
int controller_machine_init_matrix_control_network(void)
{
	int ret = -1;
	
	return ret;
}
/////////////////////////////////////
/////////////////////////////////////
/*matrix_control network@}*/

/*{@surface_control network*/
/////////////////////////////////////
/////////////////////////////////////
int controller_machine_init_surface_control_network(void)
{
	int ret = -1;
	
	return ret;
}
/////////////////////////////////////
/////////////////////////////////////
/*surface_control network@}*/

/*{@1722 network*/
/////////////////////////////////////
/////////////////////////////////////
static void controller_machine_init_1722_network(controller_pmachine p_machine,
							int (*net_1722_init)(void**),
							int (*net_1722_send)(void*, int, const void*),
							int (*net_1722_recv)(void*, int, const void*),
							int (*net_1722_cleanup)(void**))
{
	struct unit_network_1722* net_1722_node = unit_1722_network_create();
	if (NULL != net_1722_node)
	{
		unit_1722_network_init(net_1722_node, 
							net_1722_init, 
							net_1722_send, 
							net_1722_recv,
							net_1722_cleanup);

	}

	p_machine->unit_1722_net = net_1722_node;
	if (p_machine->unit_1722_net != NULL)
	{
		p_machine->unit_1722_net->network_1722.network_init(&p_machine->unit_1722_net->network_1722_user_obj);
	}
}

int controller_machine_1722_network_send(const controller_pmachine p_machine, void *frame, int send_len)
{
	int ret = -1;
	if (p_machine != NULL)
	{
		ret = unit_1722_network_send(p_machine->unit_1722_net, frame, send_len, p_machine->unit_1722_net->network_1722_user_obj);
	}

	return ret;
}

int controller_machine_1722_network_recv(const controller_pmachine p_machine, void *frame, int recv_len)
{
	int ret = -1;
	if (p_machine != NULL)
	{
		ret = unit_1722_network_recv(p_machine->unit_1722_net, frame, recv_len, p_machine->unit_1722_net->network_1722_user_obj);
	}

	return ret;
}

static void controller_machine_1722_network_destroy(controller_pmachine p_machine)
{
	if (p_machine != NULL)
	{
		unit_1722_network_destroy(&p_machine->unit_1722_net, &p_machine->unit_1722_net->network_1722_user_obj);
	}
}

/////////////////////////////////////
/////////////////////////////////////
/*1722 network@}*/

/*初始化p_machine的各个参数,后期可以继续增加参数*/ 
void controller_machine_init(controller_pmachine p_machine,
							int (*net_1722_init)(void**),
							int (*net_1722_send)(void*, int, const void*),
							int (*net_1722_recv)(void*, int, const void*),
							int (*net_1722_cleanup)(void**))
{
	if (NULL != p_machine)
	{
		if (NULL != net_1722_init && \
			NULL != net_1722_send && \
			NULL != net_1722_recv && \
			NULL != net_1722_cleanup)
		{
			controller_machine_init_1722_network(p_machine,net_1722_init, net_1722_send, net_1722_recv, net_1722_cleanup);
		}
	}
}

controller_pmachine controller_machine_create(void)
{
	controller_pmachine p_machine = (controller_pmachine)malloc(sizeof(controller_machine));

	return p_machine;
}

void controller_machine_destroy(controller_pmachine*p_machine)
{
	if (p_machine != NULL &&\
		*p_machine != NULL)
	{
		controller_machine_1722_network_destroy(*p_machine);
	}
}

