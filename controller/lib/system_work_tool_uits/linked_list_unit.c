#include "linked_list_unit.h"

// 分配大小为size 类型为uint8_t *的堆空间
uint8_t* allot_heap_space( int size, uint8_t** pout )
{
	assert( NULL != pout );
	*pout = NULL;
	
	if( size > 0 )
	{
		*pout = (uint8_t *)malloc( size );
		if( NULL != (*pout) )
		{
			return (*pout);
		}
		else
		{
			DEBUG_INFO("there is no space for malloc,malloc error!" );
			return NULL;
		}
	}
	else
	{
		DEBUG_INFO("Err size of space!" );
		return NULL;
	}

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

// 从链表中摧毁指定的节点
void delect_node_from_endpoint_dblist( solid_pdblist *free_node )
{
	assert( *free_node );
	solid_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_endpoint_node( p );
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
				pnode->solid.connect_flag = false;
			else if(flags == 1)
				pnode->solid.connect_flag = true;
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

uint32_t get_available_index_endpoint_dblist_node( solid_pdblist target )
{
	return target->solid.available_index;
}

const  struct jdksavdecc_eui64* get_entity_model_id_endpoint_dblist_node( solid_pdblist target)
{
	return (&target->solid.entity_model_id);
}

void  update_entity_adpdu_endpoint_dblist(struct jdksavdecc_adpdu *util, solid_pdblist target)
{
	target->solid.available_index = util->available_index;
	target->solid.entity_model_id = util->entity_model_id;
}

void endpoint_dblist_show( solid_pdblist head )
{
	assert( head != NULL );
	solid_pdblist p = head->next;

	while( p != head )
	{
		DEBUG_INFO("[ END_STATION ID: 0x%016llx entity_index:%d connect_flag = %d ] ",\
				p->solid.entity_id, p->solid.entity_index, p->solid.connect_flag  );
		p = p->next;
	}
}

// 摧毁整个链表，包括头结点
void destroy_endpoint_dblist( solid_pdblist head )
{
	solid_pdblist tmp_node = NULL;
	solid_pdblist delect_node = NULL;
	
	for( delect_node = head->next;  delect_node != head; delect_node = tmp_node )
	{
		tmp_node = delect_node->next;
		delect_node_from_endpoint_dblist( &delect_node );
	}
#if 0
	if( delect_node == head ) // delect guard
	{
		destroy_endpoint_node( &delect_node );
	}
	else
	{
		DEBUG_INFO( "Err endpoint dblist delect!" );
	}
#endif
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

// 这函数只适用于链表中只存在同一个应用地址终端唯一的会议命令中 2016-3-2
inflight_plist search_for_conference_inflight_dblist_node( inflight_plist head, uint8_t subtype,  uint8_t cfr_cmd, uint16_t addr )
{
	assert( head );
	inflight_plist ptr = head->next;

	SEARCH_INFLIGHT_CONFERENCE_TYPE_NODE( head, ptr, cfr_cmd, subtype, addr );
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

void destroy_inflight_dblist( inflight_plist guard )
{
	assert( guard );

	inflight_plist tmp_node = NULL;
	inflight_plist de_node = NULL;

	for( de_node = guard->next; de_node != guard; de_node = tmp_node )
	{
		tmp_node = de_node->next;
		delect_inflight_dblist_node( &de_node );
	}
#if 0
	if( de_node == guard ) // delect guard
	{
		destroy_inflight_node( &de_node );
	}
	else
	{
		DEBUG_INFO( "Err inflight dblist delect!" );
	}
#endif
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

// 摧毁descptor链表
void destroy_descptor_dblist( desc_pdblist  head )
{
	assert( head );
	desc_pdblist tmp_node = NULL;
	desc_pdblist delect_node = NULL;
	
	for( delect_node = head->next;  delect_node != head; delect_node = tmp_node )
	{
		tmp_node = delect_node->next;
		delect_descptor_dblist_node( &delect_node );
	}

#if 0
	if( delect_node == head ) // delect guard
	{
		destroy_descptor_dblist_node( &delect_node );
	}
	else
	{
		DEBUG_INFO( "Err descptor dblist delect!" );
	}
#endif
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
		//DEBUG_INFO( "no such desc 0x%016llx node",entity_id );
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
	node->tmnl_dev.tmnl_status.mic_state = 0;
	node->tmnl_dev.tmnl_status.is_rgst = false;
	node->tmnl_dev.address.addr = 0xffff;
	node->tmnl_dev.spk_operate_timp = 0;
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
	if( head == NULL || (new_node == NULL))
		return;
	
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

// 寻找ID为entity_id 的描述符信息节点
tmnl_pdblist search_terminal_dblist_address_id_node( const uint16_t addr, tmnl_pdblist guard )
{
	tmnl_pdblist pnode = NULL;

	assert (NULL != guard);
	if (guard == NULL ||addr == 0xffff)
		return NULL;
	
	for (pnode = guard->next; pnode != guard; pnode = pnode->next)
		if (pnode->tmnl_dev.address.addr == addr)
			return pnode;

	return NULL;
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
	tmnl_pdblist p_free = NULL;
	tmnl_pdblist tmp_node = NULL;
	
	for( p_free = guard->next; p_free != guard; p_free = tmp_node )
	{
		tmp_node = p_free->next;
		delect_terminal_dblist_node( &p_free );
	}
	
	if( p_free == guard )
	{	
		return guard;
	}
	
	return NULL;
}

// 摧毁相应会议管理相关的链表
tmnl_pdblist destroy_terminal_dblist( tmnl_pdblist guard )
{
	assert( guard );
	tmnl_pdblist p_free = guard->next;
	tmnl_pdblist tmp_node = NULL;
	
	for( p_free = guard->next; p_free != guard; p_free = tmp_node )
	{
		tmp_node = p_free->next;
		delect_terminal_dblist_node( &p_free );
	}
	
	if( p_free == guard )
	{	
		destroy_terminal_dblist_node( &p_free );
	}

	return p_free;
}

// 从链表取出节点2016/1/30
tmnl_pdblist move_node_from_terminal_dblist( tmnl_pdblist get_node )
{
	assert( get_node );
	if( get_node == NULL )
	{
		return NULL;
	}
	
	get_node->next->prior = get_node->prior;
	get_node->prior->next = get_node->next;

	return get_node;
}

// 根据地址的大小进行排序，从小到大。经过的测试此接口暂时没发现什么问题--2016/1/30
int sort_terminal_dblist_node( tmnl_pdblist guard )
{
	assert( guard );
	if( guard == NULL )
	{
		DEBUG_INFO( "sort terminal dblist err: guard NULL" );
		return -1;
	}

	/*
	**1、从链表中找到最大的，放到头结点后面，并标志
	**2、从剩余中找最大的，放到上次最大的前面，并标志。直到头结点
	**3、前提是尾节点是最大的
	*/
	tmnl_pdblist key_node = guard->next, remainder_max_node = guard->next;
	tmnl_pdblist next_node = NULL, tmp_node = NULL, loop_node = NULL;
	/**/
	bool found_max_node = false;
	for( tmp_node =  guard->next; (tmp_node != guard) ; tmp_node = tmp_node->next )
	{// 找到最大节点
		next_node = tmp_node->next;
		if( (next_node != guard)  )
		{
			if( tmp_node->tmnl_dev.address.addr > next_node->tmnl_dev.address.addr )
			{
				remainder_max_node = tmp_node;
			}
			else
			{
				remainder_max_node = next_node;
			}

			found_max_node = true;
		}
	}

	// 剪切remainder_max_node 到表尾
	if( found_max_node )
	{
		remainder_max_node->prior->next = remainder_max_node->next;
		remainder_max_node->next->prior = remainder_max_node->prior;
		remainder_max_node->prior = guard->prior;
		guard->prior->next = remainder_max_node;
		remainder_max_node->next = guard;
		guard->prior = remainder_max_node;
	}
	
	found_max_node = false;
	for( loop_node = guard->next; (loop_node != guard) &&  (remainder_max_node != loop_node ) && (remainder_max_node->prior != loop_node );\
		loop_node = guard->next )
	{// 排序只剩表头即表示排序完成
		for( tmp_node =  guard->next; (tmp_node != guard) && (tmp_node != remainder_max_node); tmp_node = tmp_node->next )
		{// 找到最大节点
			next_node = tmp_node->next;
			if( (next_node != guard) && (next_node != remainder_max_node) )
			{
				if( tmp_node->tmnl_dev.address.addr > next_node->tmnl_dev.address.addr )
				{
					key_node = tmp_node;
				}
				else
				{
					key_node = next_node;
				}

				found_max_node = true;
			}
		}

		if( found_max_node )
		{
			// 剪切key_node 
			key_node->prior->next = key_node->next;
			key_node->next->prior = key_node->prior;

			// 插到上次找到最大节点的前面
			key_node->prior = remainder_max_node->prior;
			remainder_max_node->prior->next = key_node;
			key_node->next = remainder_max_node;
			remainder_max_node->prior = key_node;

			// 更新本次找到的最大节点
			remainder_max_node = key_node;
			found_max_node = false;
		}
	}

	return 0;
}

//2016/1/30
int show_terminal_dblist( tmnl_pdblist guard )
{
	assert( guard );
	if( guard == NULL )
		return -1;

	tmnl_pdblist loop_node = NULL;
	
	printf( "\n-----------------------------------terminal double list table----------------------------------\n\t" );
	for( loop_node = guard->next; loop_node != guard; loop_node = loop_node->next )
	{
		printf( "0x%04x\t", loop_node->tmnl_dev.address.addr );
	}

	printf("\n-----------------------------------------------------------------------------------------------\n");

	return 0;
}

/*===============================================================
*					end terminal double list
*================================================================*/

