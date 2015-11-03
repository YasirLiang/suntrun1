/*endstation_connection.h
**data:2015/11/3
**
**
*/


#ifndef __ENDSTATION_CONNECTION_H__
#define __ENDSTATION_CONNECTION_H__

#include "list.h" // 内核链表
#include "jdksavdecc_world.h"

typedef struct _tconnect_table_data_type	// 系统连接表的数据区数据类型
{
	bool listener_connect_flags; 	// 通道连接标志, true: connected; false:disconnected
	uint16_t listener_index; 		// 通道ID索引
	uint16_t tarker_index;		//  麦克风ID索引
	uint64_t listener_id; 		// 通道ID
	uint64_t tarker_id; 			// 麦克风ID
}cnnt_table_dtype;

typedef struct _tconnect_table_list_node // 连接信息表，双向循环链表
{
	cnnt_table_dtype connect_elem; 	// 连接信息
	struct list_head list; 			
}connect_tbl_dblist, *connect_tbl_pdblist;

#endif

