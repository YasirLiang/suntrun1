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

enum output_channel_status// ���ͨ����״̬
{
	OUTCHANNEL_UNAVAILABLE,// ������
	OUTCHANNEL_OFFLINE,// ������
	OUTCHANNEL_ONLINE = 0, // ����������
};

typedef struct _type_output_channel// ���������ͨ��
{
	uint64_t tarker_id; 		// ͨ��ID
	uint16_t tarker_index; 	// ͨ��ID����
	enum output_channel_status status;// ͨ����״̬
	timetype timetimp;			// ����ʱʱ���
	struct list_head list;			// �������������ӱ���������
}TOutChannel;


#endif
