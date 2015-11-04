/*endstation_connection.h
**data:2015/11/3
**
**
*/


#ifndef __ENDSTATION_CONNECTION_H__
#define __ENDSTATION_CONNECTION_H__

#include "list.h" // 内核链表
#include "jdksavdecc_world.h"
#include "descriptor.h"

typedef struct _tterminal_speak_timeouts
{
	bool is_limit;					// 超时设定
	bool running;					// 运行
	bool elapsed;					// 超时标志
	uint32_t start_time;			// 开始时间
	uint32_t count_time;			// 超时时间
}ttspeak_timeouts;

typedef struct _tconnect_table_data_type	// 系统连接表的数据区数据类型
{
	bool listener_connect_flags; 	// 通道连接标志, true: connected; false:disconnected
	ttspeak_timeouts spk_timeout;	// 连接的超时属性
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

void connect_table_info_init( void );
bool connect_table_get_information( desc_pdblist desc_guard );
bool connect_table_info_set( desc_pdblist desc_guard, bool is_show_table );
void connect_table_tarker_disconnect( const uint64_t utarker_id );
void connect_table_tarker_connect( const uint64_t utarker_id, uint32_t timeouts, bool is_limit_time );
int connect_table_timeouts_image( void );
void connect_table_destroy( void );


#endif

