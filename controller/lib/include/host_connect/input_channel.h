/*
**input_channel.h
**11-3-2016
**
*/

// ****************************
// file built 2016-3-11
// ��������ϵͳ���ӱ�������
// ����ͨ������ģ��
// ****************************

#ifndef __INPUT_CHANNEL_H__
#define __INPUT_CHANNEL_H__

#include "jdksavdecc_world.h"
#include "list.h"
#include "host_time.h"

enum input_channel_status// ����ͨ����״̬
{
	INCHANNEL_UNAVAILABLE = 0, // ������
	INCHANNEL_FREE,// ����
	INCHANNEL_BUSY,// ��ռ��
};

enum input_channel_pro_status
{
	INCHANNEL_PRO_PRIMED = 0,// Ԥ����
	INCHANNEL_PRO_HANDLING,// ���ڴ���
	INCHANNEL_PRO_FINISH// �ڵ��ѱ��������
};

typedef struct _type_input_channel// ���������ͨ��
{
	uint64_t listener_id; 		// ͨ��ID
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// ͨ��ID����
	timetype timetimp;// ����ʱʱ���
	enum input_channel_status status;// ͨ��������״̬
	enum input_channel_pro_status pro_status; // ͨ���ڵ㴦���״̬
	struct list_head list;			// �������������ӱ���������
}TInChannel,*T_pInChannel;

// **************************************************
int input_channel_list_add_trail( T_pInChannel p_Inputnode, struct list_head *list );
T_pInChannel intput_channel_list_node_create( void );
bool input_channel_list_node_init( T_pInChannel p_Inputnode, uint64_t channel_id, uint16_t channel_index );
int  get_input_channel_list_double_list_length( struct list_head *list_gurad );

// **************************************************//

#endif
