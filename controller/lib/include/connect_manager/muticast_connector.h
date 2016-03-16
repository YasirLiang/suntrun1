/*
**2015-12-14
**muticast_connector.h
*/

#ifndef __MUTICAST_CONNECTOR_H__
#define __MUTICAST_CONNECTOR_H__

#include "jdksavdecc_world.h"
#include "jdksavdecc_util.h"
#include "terminal_common.h"
#include "host_timer.h"
#include "descriptor.h"
#include "conference_recieve_unit.h"
#include "central_control_recieve_unit.h"


#define MAX_BD_CONNECT_NUM (SYSTEM_TMNL_MAX_NUM) // ���������
#define MAX_CVNT_CONNECT_NUM		5 // �����������ʧ�ܵĴ���
#define CVNT_CONNECT_TIME_OUT  	10
#define CVNT_ONLINE_TIME_OUT  		30
#define CVNT_OUTLINE_TIME_OUT  	60

#define CVNT_MUTICAST_NAME CCU_TR_MODEL_NAME // �����봦����շ���ģ��
#define CVNT_MUTICAST_OUT_CHANNEL 0// ����ģ�����ͨ��Ϊ0
#define CVNT_MUTICAST_IN_CHNNEL CONFERENCE_RECIEVE_UNIT_IN_CHNNEL// ���������յ�Ԫ����ͨ��Ϊ0

typedef enum _econventioner_state
{
	CVNT_OFFLINE,
	CVNT_ONLINE,
	CVNT_OUT_CONNECT,	// δ���ӳɹ�,update �ɹ�ʱ�ɸı�ΪCVNT_ONLINE����countΪ0ʱ����ΪCVNT_OFFLINE
	CVNT_STREAM_ON,
	CVNT_STREAM_OFF
}econventioner_state;

struct host_muticastor	// �����Ĺ㲥��
{
	uint16_t connect_num; 	// �ѹ㲥������
	uint16_t cvntr_total_num;// ���������
	uint16_t current_index;  // ��ǰ�б�����,���ڳ�ʱ���ӱ�Ĵ���
	uint16_t tarker_index;	// ����
	uint64_t uid; 			// ID
	bool muticastor_exsit;	// 
};

typedef struct conventioner_cnnt_list_node	// ����������б�ڵ� 24���ֽ�
{
	uint16_t state;							// ����ߵ�״̬
	bool connect_flag;
	struct host_timer timeout;
	uint16_t count; // ���㿪ʼ������δ�����ϵĴ��� ����������������һ����ֵ �����䲻������,Ҳ��������Ϊ��ʹ����������
	uint16_t listerner_index;
	uint64_t uid;
}conventioner_cnnt_list_node;

typedef enum _emuticast_node_pro
{
	CVNT_CHECK_PRIMED,	// ��ʼ����
	CVNT_CHECK_WAIT,	// ���ڴ���
	CVNT_CHECK_IDLE		// �������
}emuticast_node_pro;

typedef struct conventioner_expend_node		// ��������״̬�Ľṹ
{
	conventioner_cnnt_list_node* current_listener;	// ��ǰ�ı��㲥��
	emuticast_node_pro eelem_flags;		// �����־
	struct host_muticastor muticastor;				// �����Ĺ㲥��
}muticastor_connect_pro;

typedef int (*muticast_offline_callback)( uint64_t tarker_id, conventioner_cnnt_list_node* connect_node, bool success );// �㲥���ӱ���������ص�����ָ��
typedef int (*muticast_online_callback)( uint64_t tarker_stream_id, uint64_t listern_stream_id, conventioner_cnnt_list_node* connect_node, bool success );// �㲥���ӱ���������ն�����Ļص�����ָ��

typedef struct _type_muticast_conventioner_callback
{
	uint64_t tarker_steam_id, listener_stream_id;
	conventioner_cnnt_list_node* p_cvnt_node;
	muticast_offline_callback p_offline_func;
	muticast_online_callback p_online_func;
}muticast_conventioner_callback;

int muticast_connector_connect( conventioner_cnnt_list_node* connect_node );
int muticast_connector_connect_callback( uint64_t tarker_id, conventioner_cnnt_list_node* connect_node, bool success );
int muticast_connector_proccess_online( conventioner_cnnt_list_node* connect_node );
int muticast_connector_proccess_online_callback( uint64_t tarker_stream_id, uint64_t listern_stream_id, conventioner_cnnt_list_node* connect_node, bool success );
int muticast_connector_proccess_outline( conventioner_cnnt_list_node* connect_node );
int muticast_connector_time_tick( void );
int muticast_connector_init( void );
void muticast_connector_destroy( void );
bool muticast_connector_connect_table_set( desc_pdblist desc_guard );

int muticast_connector_connect_table_init_node( const bool is_input_desc, const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node );


#endif
