/*
**out_channel.h
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

typedef struct _type_input_channel
{
	uint64_t listener_id;
	uint16_t listen_index;
	struct list_head list;
}Input_Channel;

typedef struct _type_output_channel// ���������ͨ��
{
	uint16_t tarker_index;
	Input_Channel input_head; // ����, Ϊ����������
	struct list_head list;
}TOutChannel, *T_pOutChannel;

#endif
