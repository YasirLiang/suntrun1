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

#include "connector_subject.h"
#include "output_channel.h"
#include "terminal_common.h"
#include "descriptor.h"
#include "host_timer.h"
#include "global.h"

#define CONFERENCE_OUTPUT_INDEX conference_uint_transmit_uint_ouput// ����Ϊ��ͨ�����
#define CONFERENCE_OUTCHANNEL_MAX_NUM 8// ����ģ������������
#define CONFERENCE_PER_MODEL_OUT_MAX_NUM 2// ����ͬʱ�������������

#define CONFERENCE_TRANSMIT_UNIT_NAME conference_uint_name

typedef struct _confenrence_trans_model
{
	uint64_t tarker_id;
	tmnl_pdblist confenrence_node;
	host_timer model_speak_time;// �ǻ����ն��ѷ���ʱ��
	uint32_t unit_protect_timetimp;// ʵ�������ʼʱ���
	TOutChannel out_ch; // ���ͨ������Ϊ���������
	struct list_head list; 
}tconference_trans_model, *tconference_trans_pmodel;// ���鴫��ģ��

// *************************************************
void  trans_model_node_insert_to_list( tconference_trans_pmodel p_new_node);
int conference_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node );
int trans_model_unit_connect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node );// return -1; means that there is no ccu reciever model 
int trans_model_unit_disconnect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node ); // return -1 means talker not connect
void trans_model_unit_update( subject_data_elem connect_info );// ���´���ģ�������״̬, ������֪ͨ����ϵͳЭ�����Ϣ
void conference_transmit_model_init( void );
int conference_transmit_model_node_destroy( uint64_t tarker_id );
int trans_model_unit_disconnect_longest_connect( void );
bool trans_model_unit_is_connected( uint64_t tarker_id );
bool trans_model_unit_reconnect_disconnect_tarker( uint64_t tarker_id, bool connect_or_disconnect_flag );
void conference_transmit_unit_destroy(void);
int trans_model_unit_disconnect_longest_connect_re_id_cfcnode(uint64_t *id, tmnl_pdblist* pp_confenrence_node);
uint8_t trans_model_unit_get_system_input_num(void);

// *************************************************//

#endif
