/*
**conference_transmit_unit.h
**9-3-2016
**
*/

// ********************************
// file built 2016-3-9
// ��������ϵͳ���ӱ�������
// ���鵥Ԫ����ģ��
// ********************************

#ifndef __CONFERENCE_TRANSMIT_UNIT_H__
#define __CONFERENCE_TRANSMIT_UNIT_H__

#define CONFERENCE_OUTPUT_INDEX 0// ����Ϊ��ͨ�����
#define CONFERENCE_OUTCHANNEL_MAX_NUM 8// ����ģ������������
#define CONFERENCE_PER_MODEL_OUT_MAX_NUM 2// ����ͬʱ�������������

typedef struct _confenrence_trans_model
{
	uint64_t tarker_id;
	tmnl_pdblist confenrence_node;
	host_timer model_speak_time;
	TOutChannel out_ch; // ���ͨ������Ϊ���������
	struct list_head list; 
}tconference_trans_model, *tconference_trans_pmodel;// ���鴫��ģ��

#endif