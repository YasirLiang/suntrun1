/*endstation_connection.h
**data:2015/11/3
**
**
*/


#ifndef __ENDSTATION_CONNECTION_H__
#define __ENDSTATION_CONNECTION_H__

#include "list.h" // �ں�����
#include "jdksavdecc_world.h"
#include "descriptor.h"
#include "terminal_common.h"

#define CHANNEL_MUX_NUM 4 // ����ͨ����

#ifdef __DEBUG__
#ifndef __DEBUG_CONNECT_TABLE__
#define __DEBUG_CONNECT_TABLE__
#endif
#endif

typedef struct _tterminal_speak_timeouts
{
	bool is_limit;					// ��ʱ�趨
	bool running;					// ����
	bool elapsed;					// ��ʱ��־
	uint32_t start_time;			// ��ʼʱ��
	uint32_t count_time;			// ��ʱʱ��
}ttspeak_timeouts;

typedef struct _tconnect_table_data_type	// ϵͳ���ӱ����������������
{
	bool listener_connect_flags; 	// ͨ�����ӱ�־, true: connected; false:disconnected
	ttspeak_timeouts spk_timeout;	// ���ӵĳ�ʱ����
	uint16_t listener_index; 		// ͨ��ID����
	uint16_t tarker_index;		//  ��˷�ID����
	uint64_t listener_id; 		// ͨ��ID
	uint64_t tarker_id; 			// ��˷�ID
}cnnt_table_dtype;

typedef struct _tconnect_table_list_node // ������Ϣ��˫��ѭ������
{
	cnnt_table_dtype connect_elem; 	// ������Ϣ
	struct list_head list; 			
}connect_tbl_dblist, *connect_tbl_pdblist;

typedef int (*pc_connect_table_callback_func)( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id ); // �������ӱ�����ص�����
typedef int (*pdis_connect_table_callback_func)( connect_tbl_pdblist p_cnnt_node ); // �Ͽ����ӱ�����ص�����
typedef void (*p_mic_state_set_callback)( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node );
typedef int (*p_main_state_send_callback)( uint16_t cmd, void *data, uint32_t data_len );

typedef struct __ttconnnect_table_callback // ���ڻص�����
{
	uint32_t limit_speak_time;		// ������ʱʱ�䣬��Ϊ0��ʾ����ʱ
	uint64_t tarker_id;				// ��˷�ID
	connect_tbl_pdblist p_cnnt_node; // ָ�����ӱ�Ľڵ�
	pc_connect_table_callback_func pc_callback; // ���ӻص�
	pdis_connect_table_callback_func pdis_callback;// �Ͽ��ص�
}ttcnn_table_call;

typedef struct _ttdisconnect_connect_mic_set_callback
{
	uint8_t mic_state;
	bool mic_state_set;
	tmnl_pdblist connect_node; // ���ܸı�Ĳ���
	p_mic_state_set_callback p_mic_set_callback;
	p_main_state_send_callback p_mian_state_send;
}tdisconnect_connect_mic_main_set;


void connect_table_info_init( void );
bool connect_table_get_information( desc_pdblist desc_guard );
bool connect_table_info_set( desc_pdblist desc_guard, bool is_show_table );
int connect_table_timeouts_image( void );
void connect_table_destroy( void );
int connect_table_disconnect_callback( connect_tbl_pdblist p_cnnt_node );
int connect_table_connect_callback( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id );
connect_tbl_pdblist found_connect_table_available_connect_node( const uint64_t utarker_id );
void connect_table_tarker_connect( const uint64_t utarker_id, 
									uint32_t timeouts, 
									tmnl_pdblist connect_node, 
									bool mic_report, 
									uint8_t mic_status,
									p_mic_state_set_callback p_mic_call, 
									p_main_state_send_callback p_main_send_call );
void connect_table_tarker_disconnect( const uint64_t utarker_id, 
									tmnl_pdblist connect_node, 
									bool mic_report, 
									uint8_t mic_status, 
									p_mic_state_set_callback p_mic_call, 
									p_main_state_send_callback p_main_send_call );

#endif

