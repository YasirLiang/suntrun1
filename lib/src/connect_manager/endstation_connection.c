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
pthread_mutex_t cnnt_mutex;				// 连接表私有线程锁 

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
				for( out_stream_index = 0; out_stream_index < stream_output_desc_count; out_stream_index++)
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
		//DEBUG_INFO( " connect table len = %d", connect_table_double_list_length_get(cnnt_list_guard) );
		connect_table_double_list_show_all( cnnt_list_guard );
		connect_table_double_list_show_connected( cnnt_list_guard );
	}

	pthread_mutex_unlock( &cnnt_mutex );

	return ret;
}

/*断开对应tarker_id的麦克风,暂时不考虑同步的问题(2015/11/4)*/
void connect_table_tarker_disconnect( const uint64_t utarker_id )
{
	connect_tbl_pdblist connect_pnode = NULL;
	uint64_t ulistener_id = 0;
	uint16_t listener_index = 0;
	uint16_t tarker_index = 0;
	bool found_connected = false;
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;

	pthread_mutex_lock( &cnnt_mutex );
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

	if( found_connected )
	{
		convert_uint64_to_eui64( talker_entity_id.value, utarker_id );
		convert_uint64_to_eui64( listener_entity_id.value, ulistener_id );
		acmp_disconnect_avail( talker_entity_id.value, tarker_index, listener_entity_id.value, listener_index, 1, ct_acmp_seq_id++ );

		// 将释放的通道放入连接表的结尾
		connect_pnode->connect_elem.tarker_id = 0;
		connect_pnode->connect_elem.tarker_index = 0;
		connect_pnode->connect_elem.listener_connect_flags = false;
		connect_table_double_list_move_node_to_tail( connect_pnode, cnnt_list_guard );
	}

	pthread_mutex_unlock( &cnnt_mutex );
}

// 连接麦克风 timeouts的单位是分钟
void connect_table_tarker_connect( const uint64_t utarker_id, uint32_t timeouts, bool is_limit_time )
{
	connect_tbl_pdblist connect_pnode = NULL;
	uint64_t ulistener_id = 0;
	uint16_t listener_index = 0;
	uint16_t tarker_index = 0;
	bool found_listener_avail_first = false; // 找到第一个可用的声音通道
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	
	pthread_mutex_lock( &cnnt_mutex );
	list_for_each_entry( connect_pnode, &cnnt_list_guard->list, list )
	{
		if( !connect_pnode->connect_elem.listener_connect_flags &&\
			connect_pnode->connect_elem.tarker_id == 0 ) // 找到可用通道?
		{
			ulistener_id = connect_pnode->connect_elem.listener_id;
			listener_index = connect_pnode->connect_elem.listener_index;
			found_listener_avail_first = true;
			break;
		}
	}

	if( found_listener_avail_first && (connect_pnode!= NULL) ) // connect available
	{
		convert_uint64_to_eui64( talker_entity_id.value, utarker_id );
		convert_uint64_to_eui64( listener_entity_id.value, ulistener_id );
		acmp_connect_avail( talker_entity_id.value, tarker_index, listener_entity_id.value, listener_index, 1, ct_acmp_seq_id++ ); // 这里tarker_index = 0;

		connect_pnode->connect_elem.tarker_id = utarker_id;
		connect_pnode->connect_elem.listener_connect_flags = true;

		if( is_limit_time )
		{
			connect_table_timer_start( 1000* timeouts, connect_pnode );
		}
	}
	
	pthread_mutex_unlock( &cnnt_mutex );

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
	DEBUG_LINE();
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
	pthread_mutex_destroy( &cnnt_mutex );
}


