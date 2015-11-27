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
#include "terminal_common.h"

#define CHANNEL_MUX_NUM 4 // 最大的通道数

#ifdef __DEBUG__
#ifndef __DEBUG_CONNECT_TABLE__
#define __DEBUG_CONNECT_TABLE__
#endif
#endif

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

typedef int (*pc_connect_table_callback_func)( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id ); // 连接连接表命令回调函数
typedef int (*pdis_connect_table_callback_func)( connect_tbl_pdblist p_cnnt_node ); // 断开连接表命令回调函数
typedef void (*p_mic_state_set_callback)( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node );
typedef int (*p_main_state_send_callback)( uint16_t cmd, void *data, uint32_t data_len );

typedef struct __ttconnnect_table_callback // 用于回调函数
{
	uint32_t limit_speak_time;		// 发言限时时间，若为0表示无限时
	uint64_t tarker_id;				// 麦克风ID
	connect_tbl_pdblist p_cnnt_node; // 指向连接表的节点
	pc_connect_table_callback_func pc_callback; // 连接回调
	pdis_connect_table_callback_func pdis_callback;// 断开回调
}ttcnn_table_call;

typedef struct _ttdisconnect_connect_mic_set_callback
{
	uint8_t mic_state;
	bool mic_state_set;
	tmnl_pdblist connect_node; // 不能改变的参数
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

