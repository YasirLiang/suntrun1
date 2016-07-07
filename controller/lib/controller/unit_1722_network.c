/*File:unit_1722_network.c
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:与1722终端通信实现文件
  */

#include "unit_1722_network.h"

struct unit_network_1722* unit_1722_network_create(void)
{
	struct unit_network_1722* net_node = NULL;

	net_node =(struct unit_network_1722*)malloc(sizeof(struct unit_network_1722));

	return net_node;
}

int unit_1722_network_init(struct unit_network_1722* network, 
							int (*network_init)(void**), 
							int (*network_send)(void*, int, const void*), 
							int (*network_recv)(void*, int, const void*), 
							int (*network_cleanup)(void**))
{
	int ret = -1;
	
	if (network != NULL && network_init != NULL\
		&& network_send != NULL && network_recv != NULL\
		&& network_cleanup != NULL)
	{
		network->running = 1;
		network->enum_flag = NET_1722_OK;
		network->network_1722.network_init = network_init;
		network->network_1722.network_send = network_send;
		network->network_1722.network_recv = network_recv;
		network->network_1722.network_cleanup = network_cleanup;
	}
	else
	{
		network->running = 0;
		network->enum_flag = NET_1722_NOT_INIT;
	}

	return ret;
}

int unit_1722_network_send(struct unit_network_1722* network, void *frame, int send_len, const void*usr_obj)
{
	int ret = -1;
	if (network != NULL && NULL != frame&&\
		network->running && \
		network->enum_flag == NET_1722_OK)
	{
		ret = network->network_1722.network_send(frame, send_len, usr_obj);
	}

	return ret;
}

int unit_1722_network_recv(struct unit_network_1722* network, void *frame, int recv_len, const void*usr_obj)
{
	int ret = -1;
	if (network != NULL && NULL != frame&&\
		network->running && \
		network->enum_flag == NET_1722_OK)
	{
		ret = network->network_1722.network_recv(frame, recv_len, usr_obj);
	}

	return ret;
}

int unit_1722_network_destroy(struct unit_network_1722** network, void**usr_obj)
{
	int ret = -1;
	if (network != NULL && *network != NULL &&\
		(*network)->running && \
		(*network)->enum_flag == NET_1722_OK)
	{
		(*network)->network_1722.network_cleanup(usr_obj);
		free(*network);
		*network = NULL;
		ret = 0;
	}

	return ret;
}
