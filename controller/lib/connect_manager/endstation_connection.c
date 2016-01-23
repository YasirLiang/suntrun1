/*endstation_connection.c
**data:2015/11/3
**
**
*/

#include "endstation_connection.h"
#include "connect_table_list.h"
#include "descriptor.h"
#include "host_controller_debug.h"
#include "connect_timeout.h"
#include "connect_table_list.h"
#include "util.h"
#include "acmp_controller_machine.h"
#include <pthread.h>

connect_tbl_pdblist cnnt_list_guard = NULL;	// 连接表头结点
static uint16_t ct_acmp_seq_id = 0; 		// 连接表序列号
pthread_mutex_t cnnt_mutex;			// 连接表私有线程锁 

void connect_table_info_init( void )
{
	/*初始化连接双向循环链表*/ 
	pthread_mutex_init( &cnnt_mutex, NULL );
	pthread_mutex_lock( &cnnt_mutex );
	connect_table_double_list_init( &cnnt_list_guard );
	assert( cnnt_list_guard );
	pthread_mutex_unlock( &cnnt_mutex );
}

/* 初始化系统生音通道，必须在系统终端的输入输出流初始化完成后才能调用*/
bool connect_table_get_information( desc_pdblist desc_guard )
{
	desc_pdblist desc_node = desc_guard->next;
	bool ret = false;

	/*终端无描述符*/
	if( desc_node == desc_guard )
	{
		ret = false;
	}

	for( ; desc_node  != desc_guard; desc_node = desc_node->next )
	{
		connect_tbl_pdblist new_node = NULL;
		if( desc_node->endpoint_desc.is_entity_desc_exist && desc_node->endpoint_desc.is_entity_desc_exist\
			&& desc_node->endpoint_desc.is_input_stream_desc_exist\
			&& (desc_node->endpoint_desc.input_stream.num > 0) ) // 通道存在
		{
			// 获取通道0信息
			new_node = connect_table_dblist_node_create( &new_node );
			if( NULL == new_node )
			{
				DEBUG_INFO( "create connect table node Err!" );
				assert( NULL != new_node );
			}

			memset( &new_node->connect_elem.spk_timeout, 0, sizeof(ttspeak_timeouts));
			new_node->connect_elem.listener_id = desc_node->endpoint_desc.entity_id;
			new_node->connect_elem.listener_index = desc_node->endpoint_desc.input_stream.desc[0].descriptor_index;// 存储第一个输入流
			new_node->connect_elem.listener_connect_flags = false;
			new_node->connect_elem.tarker_id = 0;// 未连接初始化
			new_node->connect_elem.tarker_index = 0; // 未连接初始化

			// 插入到连接表结尾
			pthread_mutex_lock( &cnnt_mutex );
			connect_table_double_list_insert_node( new_node, cnnt_list_guard );
			ret = true;
			pthread_mutex_unlock( &cnnt_mutex );
		}
	}

	return ret;
}

// 设置连接表的连接信息
bool connect_table_info_set( desc_pdblist desc_guard, bool is_show_table )
{
	assert( desc_guard );
	desc_pdblist desc_node = desc_guard->next;
	bool ret = false;
	bool connect_listener_exist = false;

	/*终端无描述符*/
	if( desc_node == desc_guard )
	{
		ret = false;
	}

	pthread_mutex_lock( &cnnt_mutex );
	for( ; desc_node != desc_guard; desc_node = desc_node->next )
	{
		connect_tbl_pdblist connect_pnode = NULL;
		list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
		{
			if( connect_pnode->connect_elem.listener_id == desc_node->endpoint_desc.entity_id )
			{
				connect_listener_exist = true;
				break;
			}
		}

		// 一个实体中的所有输入流与所有输出流比较
		uint8_t stream_input_desc_count = desc_node->endpoint_desc.input_stream.num;
		int in_stream_index = 0;
		for( in_stream_index = 0; in_stream_index < stream_input_desc_count; in_stream_index++ )
		{
			if( desc_node->endpoint_desc.input_stream.desc[in_stream_index].connect_num == 0)// 当前实体的输入流连接数为0
			{
				continue;
			}

			// 一个实体中的一个输入流与所有实体中的所有输出流比较, 一个输出流可以连多个输入流，而输入流只能被一个输出流占用
			desc_pdblist out_desc = desc_guard->next;
			for( ; out_desc != desc_guard; out_desc = out_desc->next )
			{
				// 与一个实体中的所有输出流比较
				uint8_t stream_output_desc_count = out_desc->endpoint_desc.output_stream.num;
				int out_stream_index = 0;
				for( out_stream_index = 0; out_stream_index < stream_output_desc_count; out_stream_index++ )
				{
					if( (out_desc->endpoint_desc.output_stream.desc[out_stream_index].connect_num > 0)\
						&& (out_desc->endpoint_desc.output_stream.desc[out_stream_index].stream_id \
						== desc_node->endpoint_desc.input_stream.desc[in_stream_index].stream_id ) )
					{
						if( connect_listener_exist )
						{
							connect_pnode->connect_elem.listener_index = in_stream_index;
							connect_pnode->connect_elem.tarker_id = out_desc->endpoint_desc.entity_id;
							connect_pnode->connect_elem.tarker_index = out_stream_index;
							connect_pnode->connect_elem.listener_connect_flags = true;
							ret = true;
						}
						else
						{
							DEBUG_INFO( "init connect table failed:no such 0x%llx listener! init again!",desc_node->endpoint_desc.entity_id);
						}
					}
				}
			}
		}
	}

	if( is_show_table )
	{
		connect_table_double_list_show_all( cnnt_list_guard );
		connect_table_double_list_show_connected( cnnt_list_guard );
	}

	pthread_mutex_unlock( &cnnt_mutex );

	return ret;
}

/***********************************************************************
*writer:YasirLiang
*change data:2015-11-11
*chang context:
*		添加用于回调函数用的终端链表节点参数，添加设置麦克风状态的回调函数	的参数，发送主机状态的回调函数参数
*连接麦克风 timeouts的单位是分钟, 连接表发送连接命令可以同过异步回调更新tarker在连接表里的状态
//断开对应tarker_id的麦克风,暂时不考虑同步的问题(2015/11/4)
*************************************************************************/ 
void connect_table_tarker_disconnect( const uint64_t utarker_id, 
									tmnl_pdblist connect_node, 
									bool mic_report, 
									uint8_t mic_status, 
									p_mic_state_set_callback p_mic_call, 
									p_main_state_send_callback p_main_send_call )
{
	connect_tbl_pdblist connect_pnode = NULL;
	uint64_t ulistener_id = 0;
	uint16_t listener_index = 0;
	uint16_t tarker_index = 0;
	bool found_connected = false;
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;

	pthread_mutex_lock( &cnnt_mutex );
	
	connect_table_double_list_show_all( cnnt_list_guard );
	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{
		if( connect_pnode->connect_elem.listener_connect_flags &&\
			connect_pnode->connect_elem.tarker_id == utarker_id )
		{
			ulistener_id = connect_pnode->connect_elem.listener_id;
			listener_index = connect_pnode->connect_elem.listener_index;
			tarker_index = connect_pnode->connect_elem.tarker_index;
			found_connected = true;
			break;
		}
	}

	DEBUG_INFO( "found connected = %d, listener_entity_id = 0x%016llx", found_connected, ulistener_id );
	assert( connect_node );
	assert( p_mic_call );
	assert( p_main_send_call );
	if( found_connected )
	{
		ttcnn_table_call call_elem;
		call_elem.limit_speak_time = 0;
		call_elem.p_cnnt_node = connect_pnode;
		call_elem.tarker_id = utarker_id;
		convert_uint64_to_eui64( talker_entity_id.value, utarker_id );
		convert_uint64_to_eui64( listener_entity_id.value, ulistener_id );

		tdisconnect_connect_mic_main_set mic_main_set; // mic status callback information
		mic_main_set.connect_node = connect_node;
		mic_main_set.mic_state = mic_status;
		mic_main_set.mic_state_set = mic_report;
		mic_main_set.p_mian_state_send = p_main_send_call;
		mic_main_set.p_mic_set_callback = p_mic_call;
		
		acmp_disconnect_connect_table( talker_entity_id.value, tarker_index, \
			listener_entity_id.value, listener_index, 1, ct_acmp_seq_id++, &call_elem , connect_table_disconnect_callback, &mic_main_set );
	}
	else // 直接回调麦克风状态设置函数与发送主机状态的回调函数
	{
		p_mic_call( mic_status, connect_node->tmnl_dev.address.addr, connect_node->tmnl_dev.entity_id, mic_report, connect_node );// close mic
		p_main_send_call( 0, NULL, 0 );
	}

	pthread_mutex_unlock( &cnnt_mutex );
}

int connect_table_disconnect_callback( connect_tbl_pdblist p_cnnt_node )
{
	if( NULL != p_cnnt_node )
	{
		p_cnnt_node->connect_elem.tarker_id = 0;
		p_cnnt_node->connect_elem.tarker_index = 0;
		p_cnnt_node->connect_elem.listener_connect_flags = false;
		connect_table_double_list_move_node_to_tail( p_cnnt_node, cnnt_list_guard );
		
#ifdef __DEBUG_CONNECT_TABLE__
		connect_table_double_list_show_all( cnnt_list_guard );
#endif
	}
	else
	{	
		return -1;
	}
	
	return 0;
}

/***********************************************************************
*writer:YasirLiang
*change data:2015-11-11
*chang context:
*		添加用于回调函数用的终端链表节点参数，添加设置麦克风状态的回调函数	的参数，发送主机状态的回调函数参数
*func:用于连接通道，并使用异步回调的方式设置麦克风的状态
*连接麦克风 timeouts的单位是分钟, 连接表发送连接命令可以同过异步回调更新tarker在连接表里的状态
*************************************************************************/ 
void connect_table_tarker_connect( const uint64_t utarker_id, 
									uint32_t timeouts, 
									tmnl_pdblist connect_node, 
									bool mic_report, 
									uint8_t mic_status,
									p_mic_state_set_callback p_mic_call, 
									p_main_state_send_callback p_main_send_call )
{
	connect_tbl_pdblist connect_pnode = NULL;
	uint64_t ulistener_id = 0;
	uint16_t listener_index = 0;
	uint16_t tarker_index = 0;
	bool found_listener_avail_first = false; // 找到第一个可用的声音通道
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	bool found_tarker_connected = false; // 是否已经连接
	
	pthread_mutex_lock( &cnnt_mutex );
	
	connect_table_double_list_show_all( cnnt_list_guard );
	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{
		if( (connect_pnode->connect_elem.listener_connect_flags) && (connect_pnode->connect_elem.tarker_id == utarker_id) )
		{
			found_tarker_connected = true;// 已连接
			break;
		}
		
		if( (!connect_pnode->connect_elem.listener_connect_flags &&\
			connect_pnode->connect_elem.tarker_id == 0 &&\
			connect_pnode->connect_elem.listener_id != utarker_id) ) // 找到可用通道? 且不是自身
		{
			ulistener_id = connect_pnode->connect_elem.listener_id;
			listener_index = connect_pnode->connect_elem.listener_index;
			found_tarker_connected = false;
			found_listener_avail_first = true;
			break;
		}
	}

	DEBUG_INFO( "found found_listener_avail_first = %d, listener_entity_id = 0x%016llx, tarker_entity_id = 0x%016llx", found_listener_avail_first, ulistener_id, utarker_id );
	assert( connect_node );
	assert( p_mic_call );
	assert( p_main_send_call );

	if( !found_tarker_connected )// not connected?
	{
		if( found_listener_avail_first && (connect_pnode != NULL) ) // connect available
		{
			ttcnn_table_call call_elem;
			call_elem.limit_speak_time = timeouts;
			call_elem.p_cnnt_node = connect_pnode;
			call_elem.tarker_id = utarker_id;
			call_elem.pc_callback = connect_table_connect_callback;

			tdisconnect_connect_mic_main_set mic_main_set; // mic status callback information
			mic_main_set.connect_node = connect_node;
			mic_main_set.mic_state = mic_status;
			mic_main_set.mic_state_set = mic_report;
			mic_main_set.p_mian_state_send = p_main_send_call;
			mic_main_set.p_mic_set_callback = p_mic_call;
			
			convert_uint64_to_eui64( talker_entity_id.value, utarker_id );
			convert_uint64_to_eui64( listener_entity_id.value, ulistener_id );
			acmp_connect_connect_table( talker_entity_id.value, tarker_index, \
				listener_entity_id.value, listener_index, 1, ct_acmp_seq_id++, &call_elem, connect_table_connect_callback, &mic_main_set );
		}
		else // 直接回调麦克风状态设置函数与发送主机状态的回调函数
		{
			p_mic_call( !mic_status, connect_node->tmnl_dev.address.addr, connect_node->tmnl_dev.entity_id, mic_report, connect_node );// close mic status
			p_main_send_call( 0, NULL, 0 );
		}
	}
	else
	{
		p_mic_call( mic_status, connect_node->tmnl_dev.address.addr, connect_node->tmnl_dev.entity_id, mic_report, connect_node );// set mic open status
		p_main_send_call( 0, NULL, 0 );
	}
	
	pthread_mutex_unlock( &cnnt_mutex );
}

int connect_table_connect_callback( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id )
{
	DEBUG_INFO( "timeout = %d, is_limit_time = %d, 0x%016llx", timeouts, is_limit_time, utarker_id );
	if( NULL != p_cnnt_node )
	{
		p_cnnt_node->connect_elem.tarker_id = utarker_id;
		p_cnnt_node->connect_elem.listener_connect_flags = true;

		if( is_limit_time )
		{
			connect_table_timer_start( 1000* timeouts, p_cnnt_node );
		}
		else
		{
			connect_table_timer_stop( p_cnnt_node );
		}

#ifdef __DEBUG_CONNECT_TABLE__
		connect_table_double_list_show_all( cnnt_list_guard );
#endif

	}
	else
	{
		return -1;
	}

	return 0;
}

/**
 *这个函数可以修改作为有效通道的寻找函数接口，
 *参数可能有会议的模式mode，通道的数量等等(2015-12-11)
 */
connect_tbl_pdblist found_connect_table_available_connect_node( const uint64_t utarker_id )
{
	int occupy_channal_num = 0;
	connect_tbl_pdblist connect_pnode = NULL;
	bool found_listener_avail_first = false; // 找到第一个可用的声音通道
	
	pthread_mutex_lock( &cnnt_mutex );
	
	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{
		if( !connect_pnode->connect_elem.listener_connect_flags &&\
			connect_pnode->connect_elem.tarker_id == 0 &&\
			connect_pnode->connect_elem.listener_id != utarker_id ) // 找到可用通道?, 不能连接自己
		{
			found_listener_avail_first = true;
			break;
		}
		else
		{
			occupy_channal_num++;
		}
	}

	pthread_mutex_unlock( &cnnt_mutex );

	if( (occupy_channal_num > CHANNEL_MUX_NUM) || !found_listener_avail_first ) // if not found
	{  
		connect_pnode = list_entry( cnnt_list_guard->list.next, connect_tbl_dblist, list );// 链表第一个结点
		if( connect_pnode->connect_elem.listener_connect_flags && connect_pnode->connect_elem.tarker_id != 0)
		{
			//connect_table_tarker_disconnect( connect_pnode->connect_elem.tarker_id );
		}
	}
	
	return connect_pnode;
}

/*****************************************************
*writer:YasirLiang
*Date:2015/11/4
*Name:terminal_mic_speak_limit_time_manager( 连接表超时管理函数 )
*Param:
*	none
*Retern Value:
*	-1,Err;
*	0,nomal.
*state:轮询连接表，若有节点连接超时，关闭麦克风；
******************************************************/ 
int connect_table_timeouts_image( void )
{
	connect_tbl_pdblist connect_pnode = NULL;
	uint64_t ulistener_id = 0;
	uint16_t listener_index = 0;
	uint64_t utarker_id = 0;
	uint16_t tarker_index = 0;
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	
	pthread_mutex_lock( &cnnt_mutex );
	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{
		if( connect_table_timer_timeout( connect_pnode ) )
		{
			ulistener_id = connect_pnode->connect_elem.listener_id;
			listener_index = connect_pnode->connect_elem.listener_index;
			utarker_id = connect_pnode->connect_elem.tarker_id;
			tarker_index = connect_pnode->connect_elem.tarker_index;

			convert_uint64_to_eui64( talker_entity_id.value, utarker_id );
			convert_uint64_to_eui64( listener_entity_id.value, ulistener_id );
			acmp_disconnect_avail( talker_entity_id.value, tarker_index, listener_entity_id.value, listener_index, 1, ct_acmp_seq_id++ );// disconnect

			connect_table_timer_stop( connect_pnode );// 定时器停止

			// 将释放的通道放入连接表的结尾
			connect_pnode->connect_elem.tarker_id = 0;
			connect_pnode->connect_elem.tarker_index = 0;
			connect_pnode->connect_elem.listener_connect_flags = false;
			connect_table_double_list_move_node_to_tail( connect_pnode, cnnt_list_guard );
		}
	}
		
	pthread_mutex_unlock( &cnnt_mutex );
	
	return 0;
}

void connect_table_destroy( void )
{
	connect_tbl_pdblist connect_pnode = NULL;
	pthread_mutex_lock( &cnnt_mutex );
	/*	list_for_each( &connect_pnode->list, &cnnt_list_guard->list )
	{
		list_entry( ,list );
	}

	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{DEBUG_LINE();
		// delect node
		if( (connect_pnode != NULL) && (connect_pnode != cnnt_list_guard) )
		{DEBUG_LINE();
			list_del( &connect_pnode->list );
			free( connect_pnode );
			connect_pnode = NULL;
		}
	}
DEBUG_LINE();*/
	if( cnnt_list_guard->list.prev == cnnt_list_guard->list.next )
	{DEBUG_LINE();
		free( cnnt_list_guard );
		cnnt_list_guard = NULL;
	}
	else
		DEBUG_INFO( "free list Failed!or is bad list!" );
	DEBUG_LINE();
	pthread_mutex_unlock( &cnnt_mutex );
	pthread_mutex_destroy( &cnnt_mutex );
}


