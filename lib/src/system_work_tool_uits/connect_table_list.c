/*connect_table_list.c
**manager system connect table
**
**
*/

#include "connect_table_list.h"
#include "host_controller_debug.h"

// 创建连接表的节点
connect_tbl_pdblist connect_table_dblist_node_create( connect_tbl_pdblist* new_node )
{
	assert( new_node );
	
	*new_node = (connect_tbl_pdblist)malloc(sizeof(connect_tbl_dblist));
	if( NULL == *new_node )
	{
		DEBUG_INFO( "there is no space for malloc!" );
		assert( NULL != *new_node );
	}

	return *new_node;
}

// 初始化连接表
void connect_table_double_list_init( connect_tbl_pdblist* guard_node )
{
	assert( guard_node );

	*guard_node = connect_table_dblist_node_create( guard_node );
	if( NULL == guard_node )
	{
		DEBUG_INFO( "init connect_table_double_list_init Err!" );
		assert( NULL != *guard_node );
	}

	(*guard_node)->list.next = (*guard_node)->list.prev = &((*guard_node)->list);
}

// 插入新结到连接表中,插入到链表的最后
void connect_table_double_list_insert_node( connect_tbl_pdblist new_node, connect_tbl_pdblist guard )
{
	assert( new_node && guard );
	list_add_tail( &new_node->list, &guard->list );
}

// 将相应的节点剪切后放到链表的结尾
void connect_table_double_list_move_node_to_tail( connect_tbl_pdblist move_node, connect_tbl_pdblist guard )
{
	assert( move_node && guard );
	list_move_tail( &move_node->list, &guard->list );
}

// 显示连接表
void connect_table_double_list_show( connect_tbl_pdblist guard )
{
	assert( guard );
	connect_tbl_pdblist connect_pnode = NULL;
	
	MSGINFO( "\n------------------------------connnect table---------------------------\n");
	list_for_each_entry( connect_pnode, &guard->list, list )
	{
		if( connect_pnode->connect_elem.listener_connect_flags )
		{
			MSGINFO( "\t0x%llx(%d)-->0x%llx(%d)\n",\
				connect_pnode->connect_elem.tarker_id, connect_pnode->connect_elem.tarker_index,\
				connect_pnode->connect_elem.listener_id, connect_pnode->connect_elem.listener_index );
		}
	}
}

