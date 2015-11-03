/*endstation_connection.c
**data:2015/11/3
**
**
*/

#include "endstation_connection.h"
#include "connect_table_list.h"
#include "descriptor.h"
#include "host_controller_debug.h"

connect_tbl_pdblist cnnt_list_guard = NULL;

void connect_table_info_init( void )
{
	/*初始化连接双向循环链表*/ 
	connect_table_double_list_init( &cnnt_list_guard );
	assert( cnnt_list_guard );
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

			new_node->connect_elem.listener_id = desc_node->endpoint_desc.entity_id;
			new_node->connect_elem.listener_index = desc_node->endpoint_desc.input_stream.desc[0].descriptor_index;// 存储第一个输入流
			new_node->connect_elem.listener_connect_flags = false;
			new_node->connect_elem.tarker_id = 0;// 未连接初始化
			new_node->connect_elem.tarker_index = 0; // 未连接初始化

			// 插入到连接表结尾
			connect_table_double_list_insert_node( new_node, cnnt_list_guard );
			ret = true;
		}
	}
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
		connect_table_double_list_show( cnnt_list_guard );
	}
}

