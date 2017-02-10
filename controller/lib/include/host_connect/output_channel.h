/*
**output_channel.h
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// ��������ϵͳ���ӱ�������
// ���ͨ������ģ��
// ****************************

#ifndef __OUTPUT_CHANNEL_H__
#define __OUTPUT_CHANNEL_H__

#include "jdksavdecc_world.h"
#include "list.h"
/* CONNECT INTERVAL */
#define OUTPUT_CHANNEL_OPT_CONNECT_INTERVAL    (2000)
/* DISCONNECT INTERVAL */
#define OUTPUT_CHANNEL_OPT_DISCONNECT_INTERVAL (0)
#define OUTPUT_CHANNEL_OPT_PROTECT_TIME        (1000) // ���ͨ����������ʱ��

typedef struct _type_input_connect_node// ������ӵ�����ڵ�
{
	uint64_t listener_id;
	uint16_t listen_index;
	struct list_head list;
}Input_Channel, *Input_pChannel;// ����������ͨ���ڵ�

typedef struct _type_output_channel// ���������ͨ��
{
	uint16_t tarker_index;
	uint32_t operate_timetimp;// ͨ������ʱ���
	Input_Channel input_head; // ����, Ϊ����������
	struct list_head list;
}TOutChannel, *T_pOutChannel;

bool output_channel_insert_node_to_list( struct list_head *head, T_pOutChannel p_node );
bool output_channel_node_init_by_index( T_pOutChannel p_node, uint16_t tarker_index  );
T_pOutChannel out_channel_node_create_can_init( void );
bool input_connect_node_insert_node_to_list( struct list_head *head, Input_pChannel p_node );
bool input_connect_node_init_by_index( Input_pChannel p_node, uint64_t  listen_id, uint16_t listen_index  );
Input_pChannel input_connect_node_create( void );
bool input_connect_node_delect_node_from_list( struct list_head *head, Input_pChannel *pp_node );

#endif
