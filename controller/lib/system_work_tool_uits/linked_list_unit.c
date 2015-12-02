#include "linked_list_unit.h"

// 分配大小为size 类型为uint8_t *的堆空间
uint8_t* allot_heap_space( int size, uint8_t** pout )
{
	assert( NULL != pout );
	*pout = NULL;
	
	if( size > 0 )
	{
		//DEBUG_INFO( "=======================malloc 1=====================" );
		*pout = (uint8_t *)malloc( size );
		//DEBUG_INFO( "=======================malloc 2=====================" );
		if( NULL != (*pout) )
		{
			//DEBUG_INFO( "=======================malloc success=====================" );
			return (*pout);
		}
		else
		{
			DEBUG_INFO("there is no space for malloc,malloc error!" );
			return NULL;
		}

		//DEBUG_INFO( "=======================malloc 3=====================" );
	}
	else
	{
		DEBUG_INFO("Err size of space!" );
		return NULL;
	}

	//DEBUG_INFO( "=======================malloc 4=====================" );
	return NULL;
}

// 释放堆空间
void release_heap_space( uint8_t **p )
{
	if( NULL != *p )
	{
		free( *p );
		*p = NULL;
	}
	else
	{
		DEBUG_INFO( "NULL release Node!" );
		assert( NULL != *p );
	}
}

// malloc 新节点
solid_pdblist  create_endpoint_new_node( solid_pdblist* node_new )
{
	assert( NULL != node_new );
	
	*node_new = (solid_pdblist)malloc( sizeof(solid_dblist) );
	if( NULL != *node_new )
		return *node_new;
	else
	{
		DEBUG_INFO("msg error: NO space for malloc!");
		return NULL;
	}
}

// 摧毁指定节点
void destroy_endpoint_node( solid_pdblist* node_dstry )
{
	if( NULL != *node_dstry )
	{
		free( *node_dstry );
		*node_dstry = NULL;
	}
	else
	{
		DEBUG_INFO("NULL destroy Node!");
		assert(NULL != *node_dstry);
	}
}

// 初始化链表,head为哨兵结点
solid_pdblist  init_endpoint_dblist( struct entities **head )
{
	if( NULL != head )
	{
		*head = create_endpoint_new_node(head);
		if( NULL != *head )
		{
			(*head)->prior = (*head)->next = *head;
			return *head;
		}
		else
		{
			DEBUG_INFO("msg error: NO space for malloc!");
			return NULL;
		}
	}
	else
	{
		DEBUG_INFO( "location nothing!" );
		return NULL;
	}
}

// 插入新节点到链表结尾中，head为哨兵结点
void insert_endpoint_dblist_trail( solid_pdblist head, solid_pdblist new_node )
{
	if( (NULL != head) && (NULL != new_node) )
	{
		new_node->prior = head->prior;
		head->prior->next = new_node;
		new_node->next = head;
		head->prior = new_node;
	}
	else
	{
		DEBUG_INFO("bad endpoint node for insert!");
		assert((NULL != head) && (NULL != new_node));
	}
}

// 删除ID为entity_id终端节点,head为哨兵结点
int delect_entdtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id )
{
	solid_pdblist pnode = head->next;

	if( NULL != pnode )
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( pnode != head )	// 链表存在ID为entity_id的链表节点
		{
			pnode->next->prior = pnode->prior;
			pnode->prior->next = pnode->next;
			
			destroy_endpoint_node( &pnode );
			if( NULL == pnode )
				return 1;	
			else
			{	
				DEBUG_INFO("free entity_id is 0x%llx List Node free!", entity_id );
				return -1;
			}
		}
		else
		{
			DEBUG_INFO("no entity_id:0x%llx station!", entity_id );
			return 0;
		}
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system!" );
		return -1;
	}
	
	return 0;
}

// 寻找终端ID为endtity_id在链表中的位置,终端不存在返回NULL,head为哨兵结点
solid_pdblist search_endtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id  )
{
	solid_pdblist pnode = head->next;
	
	if( (NULL != pnode) )
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( (pnode != head) )  // 链表存在ID为entity_id的链表节点
		{
			return pnode;
		}
		else
		{
			DEBUG_INFO("no entity_id:0x%lluxstation!", entity_id );
			return NULL;
		}
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system!" );
		return NULL;
	}
	

}

// 获取链表的长度
int get_endpoint_dblist_length( solid_pdblist head )
{
	int i = 0;
	solid_pdblist pnode = head->next;

	if( NULL != pnode)
	{
		for( i = 0; pnode != head; pnode = pnode->next )
			++i;
		
		return i;
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system:this is not a loop double list!" );
		return -1;
	}	
}

// 获取索引为index的节点ID。index为实体在链表中的索引,而索引是从0开始的
int  get_entity_id_endpoint_dblist( solid_pdblist head, int index, uint64_t *entity_id )
{
	uint64_t *p = entity_id;
	solid_pdblist pnode = head->next;
	
	if( NULL != pnode)
	{
		for( ; (pnode != head) && (pnode->solid.entity_index != index ); pnode = pnode->next );
		if( pnode != head )
		{
			*p = pnode->solid.entity_id;
		}
		else
		{
			DEBUG_INFO( "there is no endpoint in system: Double Loop List loop to Head" );
			return 0;
		}
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system:this is not a loop double list!" );
		return -1;
	}

	return 1;
}

// 获取ID为entity_id的终端索引
int  get_entity_index_endpoint_dblist( solid_pdblist head, uint64_t entity_id )
{
	solid_pdblist pnode = head->next;
	
	if( NULL != pnode)
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( pnode != head )
		{
			return pnode->solid.entity_index;
		}
		else
		{
			DEBUG_INFO( "there is no endpoint in system: Double Loop List loop to Head" );
			return 0;
		}
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system:this is not a loop double list!" );
		return -1;
	}

	return 1;
}

// 设置节点的连接状态,返回0表示设置正确,flags 的值有0，1是正确的参数
int set_connect_flag_endpoint_dblist( solid_pdblist head, uint64_t entity_id, int flags)
{
	solid_pdblist pnode = head->next;
	
	if( NULL != pnode)
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( pnode != head )
		{
			if( flags == 0 )
				pnode->solid.connect_flag = DISCONNECT;
			else if(flags == 1)
				pnode->solid.connect_flag = CONNECT;
			else
			{
				DEBUG_INFO( "error connect flag: %d", flags );
				return -1;
			}
		}
		else
		{
			DEBUG_INFO( "there is no endpoint in system: Double Loop List loop to Head" );
			return -1;
		}
	}
	else
	{
		DEBUG_INFO("bad endpoint_dblist for system:this is not a loop double list!" );
		return -1;
	}

	return 0;
}

inline uint32_t get_available_index_endpoint_dblist_node( solid_pdblist target )
{
	return target->solid.available_index;
}

inline const  struct jdksavdecc_eui64* get_entity_model_id_endpoint_dblist_node( solid_pdblist target)
{
	return (&target->solid.entity_model_id);

}

inline void  update_entity_adpdu_endpoint_dblist(struct jdksavdecc_adpdu *util, solid_pdblist target)
{
	target->solid.available_index = util->available_index;
	target->solid.entity_model_id = util->entity_model_id;
}

void endpoint_dblist_show( solid_pdblist head )
{
	solid_pdblist p = head->next;

	while( p != head )
	{
		DEBUG_INFO("[ END_STATION ID: 0x%llx entity_index:%d connect_flag = %d ] ",\
				p->solid.entity_id, p->solid.entity_index, p->solid.connect_flag  );
		p = p->next;
	}
}

/*=====================================
*以上的函数用于操作系统中终端的链表，而
*以下的函数用于操作inflight命令链表
*======================================*/
inflight_plist create_inflight_dblist_new_node( inflight_plist *new_node )
{
	assert( NULL != new_node );

	*new_node = (inflight_plist)malloc( sizeof( inflight_list ) );
	if( NULL != *new_node )
		return *new_node;
	else
	{
		DEBUG_INFO(" inflight node malloc no space! ");
		assert( NULL != new_node );
	}

	return NULL;
}

// 摧毁指定节点
void destroy_inflight_node( inflight_plist *node_dstry )
{
	if( NULL != *node_dstry )
	{
		free( *node_dstry );
		*node_dstry = NULL;
	}
	else
	{
		DEBUG_INFO("NULL destroy Node!");
		assert(NULL != *node_dstry);
	}
}

inflight_plist init_inflight_dblist( inflight_plist *guard )
{
	assert( NULL != guard );

	*guard = create_inflight_dblist_new_node( guard );
	if( NULL != *guard )
	{
		(*guard)->prior = (*guard)->next = *guard;
			return *guard;
	}
	else
	{
		DEBUG_INFO(" init_inflight_dblist failed! ");
		assert( NULL != *guard );
	}

	return NULL;
}

// 将新节点插入链表中
void insert_inflight_dblist_trail( inflight_plist head, inflight_plist new_node )
{
	assert( head && new_node );
	
	new_node->prior = head->prior;
	head->prior->next = new_node;
	new_node->next = head;
	head->prior = new_node;
}

// 获取inflight 命令链表的长度
int get_inflight_dblist_length( inflight_plist head )
{
	assert( NULL != head );
	
	int i = 0;
	inflight_plist pnode = head->next;
	if( NULL != pnode)
	{
		for( ; pnode != head; pnode = pnode->next, ++i );
		return i;
	}
	else
	{
		DEBUG_INFO("bad inflight_dblist for system:this is not a loop double list!" );
		assert( NULL != pnode );
	}

	return 0;
}

// 通过seq_id 与协议数据的子类型获取inflight命令在链表中的位置，用于1722.1协议数据的处理
inflight_plist search_node_inflight_from_dblist( inflight_plist head, uint16_t  seq_id, uint8_t subtype )
{
	assert( head );
	inflight_plist ptr = head->next;

	SEARCH_INFLIGHT_SEQ_TYPE_NODE( head, ptr, seq_id, subtype );
	if( ptr != head )
		return ptr;
	else
	{
		return NULL;
	}
}

inflight_plist search_for_conference_inflight_dblist_node( inflight_plist head, uint8_t subtype,  uint8_t cfr_cmd )
{
	assert( head );
	inflight_plist ptr = head->next;

	SEARCH_INFLIGHT_CONFERENCE_TYPE_NODE( head, ptr, cfr_cmd, subtype );
	if( ptr != head )
		return ptr;
	else
	{
		return NULL;
	}
}

// 是否只有一个udp client 数据节点
bool is_exist_udp_client_inflight_type_node( inflight_plist head, uint8_t subtype )
{
	assert( head );
	inflight_plist ptr = head->next;

	for( ; ptr != head; ptr = ptr->next )
	{
		if( ptr->host_tx.inflight_frame.data_type == subtype )
			return true;
	}

	if( ptr == head )
		return false;

	return false;
}

void delect_inflight_dblist_node( inflight_plist *free_node )
{
	assert( *free_node );
	inflight_plist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_inflight_node( p );
}

/*=====================================
*以上的函数用于操作系统中终端的链表和系统中inflight命令链表
*以下函数用于操作entity_desc的文件描述符的信息链表
*======================================*/
desc_pdblist create_descptor_dblist_node( desc_pdblist* node )
{
	assert( node );
	*node = (desc_pdblist)malloc( sizeof(desc_dblist) );
	if( *node != NULL )
		return *node;
	else
	{
		DEBUG_INFO( "there is no space for malloc descptor node!" );
		return NULL;
	}

	return NULL;
}

// 初始化文件描述符的信息链表为双向循环链表
void init_descptor_dblist( desc_pdblist *guard )
{
	assert( guard );

	*guard = create_descptor_dblist_node(guard);
	if( NULL != *guard )
	{
		(*guard)->next = (*guard)->prior = *guard;
	}
	else
	{
		DEBUG_INFO( "init descptor info list error!" );
	}
}

// 初始化文件描述符的信息链表节点
void init_descptor_dblist_node_info( desc_pdblist node )
{
	assert( node );
	memset(node, 0, sizeof(desc_dblist));
}

// 摧毁指定节点
void destroy_descptor_dblist_node( desc_pdblist *node_dstry )
{
	if( NULL != *node_dstry )
	{
		free( *node_dstry );
		*node_dstry = NULL;
	}
	else
	{
		DEBUG_INFO("NULL destroy Node!");
		assert(NULL != *node_dstry);
	}
}

// 将新节点插入链表中
void insert_descptor_dblist_trail( desc_pdblist head, desc_pdblist new_node )
{
	assert( head && new_node );
	
	new_node->prior = head->prior;
	head->prior->next = new_node;
	new_node->next = head;
	head->prior = new_node;
}

// 获取inflight 命令链表的长度
int get_descptor_dblist_length( desc_pdblist head )
{
	assert( NULL != head );
	
	int i = 0;
	desc_pdblist pnode = head->next;
	if( NULL != pnode)
	{
		for( ; pnode != head; pnode = pnode->next, ++i );
		return i;
	}
	else
	{
		DEBUG_INFO("bad desc_dblist for system:this is not a loop double list!" );
		assert( NULL != pnode );
	}

	return 0;
}

// 删除节点
void delect_descptor_dblist_node( desc_pdblist *free_node )
{
	assert( *free_node );
	desc_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_descptor_dblist_node( p );
}

// 寻找ID为entity_id 的描述符信息节点
desc_pdblist search_desc_dblist_node( uint64_t entity_id, desc_pdblist guard )
{
	assert( guard );
	desc_pdblist pnode = guard->next;

	SEARCH_FOR_DESCPTOT_LIST_RIGHT_ENTITY_NODE( guard, pnode, entity_id );
	if( pnode != guard )
	{
		return pnode;
	}
	else
	{
		DEBUG_INFO( "no such desc 0x%016llx node",entity_id );
		return NULL;
	}
}

// 寻找ID为entity_id 的描述符信息节点
desc_pdblist search_desc_dblist_node_no_printf_info( uint64_t entity_id, desc_pdblist guard )
{
	assert( guard );
	desc_pdblist pnode = guard->next;

	SEARCH_FOR_DESCPTOT_LIST_RIGHT_ENTITY_NODE( guard, pnode, entity_id );
	if( pnode != guard )
	{
		return pnode;
	}
	else
	{
		return NULL;
	}
}

int get_desc_dblist_length( desc_pdblist head )
{
	assert( NULL != head );
	
	int i = 0;
	desc_pdblist pnode = head->next;
	if( NULL != pnode)
	{
		for( ; pnode != head; pnode = pnode->next, ++i );
		return i;
	}
	else
	{
		DEBUG_INFO("bad desc_dblist for system:this is not a loop double list!" );
		assert( NULL != pnode );
	}

	return 0;
}

/*=====================================
*
*以下函数用于操作会议终端的的信息链表
*======================================*/
tmnl_pdblist create_terminal_dblist_node( tmnl_pdblist* node )
{
	assert( node );
	*node = (tmnl_pdblist)malloc( sizeof(tmnl_dblist) );
	if( *node != NULL )
		return *node;
	else
	{
		DEBUG_INFO( "there is no space for malloc descptor node!" );
		return NULL;
	}

	return NULL;
}

// 初始化文件描述符的信息链表为双向循环链表
void init_terminal_dblist( tmnl_pdblist *guard )
{
	assert( guard );

	*guard = create_terminal_dblist_node(guard);
	if( NULL != *guard )
	{
		(*guard)->next = (*guard)->prior = *guard;
	}
	else
	{
		DEBUG_INFO( "init terminal info list error!" );
	}
}

// 初始化文件描述符的信息链表节点
void init_terminal_dblist_node_info( tmnl_pdblist node )
{
	assert( node );
	memset(node, 0, sizeof(tmnl_pdblist));
	node->tmnl_dev.tmnl_status.is_rgst = false;
	node->tmnl_dev.address.addr = 0xffff;
}

// 摧毁指定节点
void destroy_terminal_dblist_node( tmnl_pdblist* node_dstry )
{
	if( NULL != *node_dstry )
	{
		free( *node_dstry );
		*node_dstry = NULL;
	}
	else
	{
		DEBUG_INFO("NULL destroy Node!");
		assert(NULL != *node_dstry);
	}
}

// 将新节点插入链表中
void insert_terminal_dblist_trail( tmnl_pdblist head, tmnl_pdblist new_node )
{
	assert( head && new_node );
	
	new_node->prior = head->prior;
	head->prior->next = new_node;
	new_node->next = head;
	head->prior = new_node;
}

// 获取terminal链表的长度
int get_terminal_dblist_length( tmnl_pdblist head )
{
	assert( NULL != head );
	
	int i = 0;
	tmnl_pdblist pnode = head->next;
	if( NULL != pnode)
	{
		for( ; pnode != head; pnode = pnode->next, ++i );
		return i;
	}
	else
	{
		DEBUG_INFO("bad desc_dblist for system:this is not a loop double list!" );
		assert( NULL != pnode );
	}

	return 0;
}

// 删除节点
void delect_terminal_dblist_node( tmnl_pdblist *free_node )
{
	assert( *free_node );
	tmnl_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_terminal_dblist_node( p );
}

// 寻找ID为entity_id 的描述符信息节点
tmnl_pdblist search_terminal_dblist_entity_id_node( uint64_t entity_id, tmnl_pdblist guard )
{
	assert( guard );
	tmnl_pdblist pnode = guard->next;

	SEARCH_FOR_TERMINAL_LIST_RIGHT_ENTITY_NODE( guard, pnode, entity_id );
	if( pnode != guard )
	{
		return pnode;
	}
	else
	{
		return NULL;
	}
}

// 寻找地址为address 的描述符信息节点
tmnl_pdblist search_terminal_dblist_address_node( uint16_t  address, tmnl_pdblist guard )
{
	assert( guard );
	tmnl_pdblist pnode = guard->next;

	SEARCH_FOR_TERMINAL_LIST_RIGHT_ADDRESS_NODE( guard, pnode, address );
	if( pnode != guard )
	{
		return pnode;
	}
	else
	{
		DEBUG_INFO( "no such terminal address %d node",address );
		return NULL;
	}
}

// 清除相应的链表,除了头结点, 删除的节点都是头结点的next节点
tmnl_pdblist terminal_dblist_except_free( tmnl_pdblist guard )
{
	assert( guard );
	tmnl_pdblist p_free = guard->next;
	
	for( ; (p_free != guard); p_free = guard->next )
	{
		delect_terminal_dblist_node( &p_free );
	}
	
	if( p_free == guard )
	{	
		return guard;
	}
	
	return NULL;
}

/*===============================================================
*					end terminal double list
*================================================================*/

