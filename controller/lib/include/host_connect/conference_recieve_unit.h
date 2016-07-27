/*
**conferenc_recieve_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// ��������ϵͳ���ӱ�������
// ϵͳ���鵥Ԫ��Ԫ����ģ��
// ****************************

#ifndef __CONFERENCE_RECIEVE_UNIT_H__
#define __CONFERENCE_RECIEVE_UNIT_H__

#include "input_channel.h"
#include "descriptor.h"
#include "entity.h"
#include "global.h"
#include "connector_subject.h"
#include "host_timer.h"

#define CONFERENCE_RECIEVE_UNIT_NAME conference_uint_name
#define CONFERENCE_RECIEVE_UNIT_IN_CHNNEL_MAX_NUM 4// �������ͨ��
#define CONFERENCE_MUTICASTED_INPUT_CHANNEL conference_uint_recieve_uint_input// ����㲥���鵥Ԫ������ͨ��

typedef struct _type_conference_recieve_model
{
	uint64_t listener_id;
	solid_pdblist solid_pnode; // ָ��adp����ڵ�
	desc_pdblist desc_pnode;// ָ��desc����ڵ�
	struct list_head channel_list;// ͨ���ڵ����ڵ�������TInChannel_universe
	struct list_head *p_ccu_muticast_channel;// �������ӵ�Ԫ�Ա�ģ��㲥������ͨ�������ڵ�������TInChannel_universe
	host_timer muticast_query_timer, errlog_timer;// �㲥��ѯ��ʱ��ʱ��, ������־��ʱ��
	bool tark_discut;// �����Ͽ�,��mic �򿪣���ϵͳ�����Ͽ�ʱ����ֵΪtrue
	bool discut_flag;// �Ͽ�����
	bool query_stop; // ��ѯ������־λ
	uint16_t channel_num;// ͨ����
	uint16_t chanel_connect_num;// ģ����������
	struct list_head list;
}tconference_recieve_model, *T_Ptrconference_recieve_model;// ������յ�Ԫ

extern bool conference_recieve_model_found_next( struct list_head * p_cur_model, struct list_head **p_next_model );
extern bool conference_recieve_model_is_right( struct list_head * p_cur_model );
extern void conference_recieve_uinit_proccess_init( void );
int conference_recieve_model_connect_self( uint64_t listern_id );
int conference_recieve_model_discut_self( uint64_t listern_id );
int conference_recieve_model_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id );
void conference_recieve_unit_destroy(void);

#endif
