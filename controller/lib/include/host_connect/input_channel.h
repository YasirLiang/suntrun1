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

enum input_channel_status// ����ͨ����״̬
{
	INCHANNEL_UNAVAILABLE = 0, // ������
	INCHANNEL_FREE,// ����
	INCHANNEL_BUSY,// ��ռ��
};

typedef struct _type_input_channel// ���������ͨ��
{
	uint64_t listener_id; 		// ͨ��ID
	uint64_t tarker_id;
	uint16_t tarker_index;
	uint16_t listener_index; 		// ͨ��ID����
	timetype timetimp;// ����ʱʱ���
	enum input_channel_status status;// ͨ����״̬
	struct list_head list;			// �������������ӱ���������
}TInChannel,*T_pInChannel;

#endif
