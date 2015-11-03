/*endstation_connection.h
**data:2015/11/3
**
**
*/


#ifndef __ENDSTATION_CONNECTION_H__
#define __ENDSTATION_CONNECTION_H__

#include "list.h" // �ں�����
#include "jdksavdecc_world.h"

typedef struct _tconnect_table_data_type	// ϵͳ���ӱ����������������
{
	bool listener_connect_flags; 	// ͨ�����ӱ�־, true: connected; false:disconnected
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

#endif

