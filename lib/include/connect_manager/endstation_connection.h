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

void connect_table_info_init( void );
bool connect_table_get_information( desc_pdblist desc_guard );
bool connect_table_info_set( desc_pdblist desc_guard, bool is_show_table );
void connect_table_tarker_disconnect( const uint64_t utarker_id );
void connect_table_tarker_connect( const uint64_t utarker_id, uint32_t timeouts, bool is_limit_time );
int connect_table_timeouts_image( void );
void connect_table_destroy( void );


#endif

