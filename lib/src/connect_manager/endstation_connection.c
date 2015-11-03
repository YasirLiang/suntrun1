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
	/*��ʼ������˫��ѭ������*/ 
	connect_table_double_list_init( &cnnt_list_guard );
	assert( cnnt_list_guard );
}

/* ��ʼ��ϵͳ����ͨ����������ϵͳ�ն˵������������ʼ����ɺ���ܵ���*/
bool connect_table_get_information( desc_pdblist desc_guard )
{
	desc_pdblist desc_node = desc_guard->next;
	bool ret = false;

	/*�ն���������*/
	if( desc_node == desc_guard )
	{
		ret = false;
	}

	for( ; desc_node  != desc_guard; desc_node = desc_node->next )
	{
		connect_tbl_pdblist new_node = NULL;
		if( desc_node->endpoint_desc.is_entity_desc_exist && desc_node->endpoint_desc.is_entity_desc_exist\
			&& desc_node->endpoint_desc.is_input_stream_desc_exist\
			&& (desc_node->endpoint_desc.input_stream.num > 0) ) // ͨ������
		{
			// ��ȡͨ��0��Ϣ
			new_node = connect_table_dblist_node_create( &new_node );
			if( NULL == new_node )
			{
				DEBUG_INFO( "create connect table node Err!" );
				assert( NULL != new_node );
			}

			new_node->connect_elem.listener_id = desc_node->endpoint_desc.entity_id;
			new_node->connect_elem.listener_index = desc_node->endpoint_desc.input_stream.desc[0].descriptor_index;// �洢��һ��������
			new_node->connect_elem.listener_connect_flags = false;
			new_node->connect_elem.tarker_id = 0;// δ���ӳ�ʼ��
			new_node->connect_elem.tarker_index = 0; // δ���ӳ�ʼ��

			// ���뵽���ӱ��β
			connect_table_double_list_insert_node( new_node, cnnt_list_guard );
			ret = true;
		}
	}
}

// �������ӱ��������Ϣ
bool connect_table_info_set( desc_pdblist desc_guard, bool is_show_table )
{
	assert( desc_guard );
	desc_pdblist desc_node = desc_guard->next;
	bool ret = false;
	bool connect_listener_exist = false;

	/*�ն���������*/
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

		
		// һ��ʵ���е�����������������������Ƚ�
		uint8_t stream_input_desc_count = desc_node->endpoint_desc.input_stream.num;
		int in_stream_index = 0;
		for( in_stream_index = 0; in_stream_index < stream_input_desc_count; in_stream_index++ )
		{
			if( desc_node->endpoint_desc.input_stream.desc[in_stream_index].connect_num == 0)// ��ǰʵ���������������Ϊ0
			{
				continue;
			}

			// һ��ʵ���е�һ��������������ʵ���е�����������Ƚ�, һ��������������������������������ֻ�ܱ�һ�������ռ��
			desc_pdblist out_desc = desc_guard->next;
			for( ; out_desc != desc_guard; out_desc = out_desc->next )
			{
				// ��һ��ʵ���е�����������Ƚ�
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

