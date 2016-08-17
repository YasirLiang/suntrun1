#include "linked_list_unit.h"

// �����СΪsize ����Ϊuint8_t *�Ķѿռ�
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

// �ͷŶѿռ�
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

// malloc �½ڵ�
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

// �ݻ�ָ���ڵ�
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

// �������дݻ�ָ���Ľڵ�
void delect_node_from_endpoint_dblist( solid_pdblist *free_node )
{
	assert( *free_node );
	solid_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_endpoint_node( p );
}

// ��ʼ������,headΪ�ڱ����
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

// �����½ڵ㵽�����β�У�headΪ�ڱ����
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

// ɾ��IDΪentity_id�ն˽ڵ�,headΪ�ڱ����
int delect_entdtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id )
{
	solid_pdblist pnode = head->next;

	if( NULL != pnode )
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( pnode != head )	// �������IDΪentity_id������ڵ�
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

// Ѱ���ն�IDΪendtity_id�������е�λ��,�ն˲����ڷ���NULL,headΪ�ڱ����
solid_pdblist search_endtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id  )
{
	solid_pdblist pnode = head->next;
	
	if( (NULL != pnode) )
	{
		SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id);
		if( (pnode != head) )  // �������IDΪentity_id������ڵ�
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

// ��ȡ����ĳ���
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

// ��ȡ����Ϊindex�Ľڵ�ID��indexΪʵ���������е�����,�������Ǵ�0��ʼ��
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

// ��ȡIDΪentity_id���ն�����
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

// ���ýڵ������״̬,����0��ʾ������ȷ,flags ��ֵ��0��1����ȷ�Ĳ���
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

// �ݻ�������������ͷ���
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
*���ϵĺ������ڲ���ϵͳ���ն˵�������
*���µĺ������ڲ���inflight��������
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

// �ݻ�ָ���ڵ�
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

// ���½ڵ����������
void insert_inflight_dblist_trail( inflight_plist head, inflight_plist new_node )
{
	assert( head && new_node );
	
	new_node->prior = head->prior;
	head->prior->next = new_node;
	new_node->next = head;
	head->prior = new_node;
}

// ��ȡinflight ��������ĳ���
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

// ͨ��seq_id ��Э�����ݵ������ͻ�ȡinflight�����������е�λ�ã�����1722.1Э�����ݵĴ���
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

// �⺯��ֻ������������ֻ����ͬһ��Ӧ�õ�ַ�ն�Ψһ�Ļ��������� 2016-3-2
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

// �Ƿ�ֻ��һ��udp client ���ݽڵ�
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
*���ϵĺ������ڲ���ϵͳ���ն˵������ϵͳ��inflight��������
*���º������ڲ���entity_desc���ļ�����������Ϣ����
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

// ��ʼ���ļ�����������Ϣ����Ϊ˫��ѭ������
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

// ��ʼ���ļ�����������Ϣ����ڵ�
void init_descptor_dblist_node_info( desc_pdblist node )
{
	assert( node );
	memset(node, 0, sizeof(desc_dblist));
}

// �ݻ�ָ���ڵ�
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

// ���½ڵ����������
void insert_descptor_dblist_trail( desc_pdblist head, desc_pdblist new_node )
{
	assert( head && new_node );
	
	new_node->prior = head->prior;
	head->prior->next = new_node;
	new_node->next = head;
	head->prior = new_node;
}

// ��ȡinflight ��������ĳ���
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

// ɾ���ڵ�
void delect_descptor_dblist_node( desc_pdblist *free_node )
{
	assert( *free_node );
	desc_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_descptor_dblist_node( p );
}

// �ݻ�descptor����
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

// Ѱ��IDΪentity_id ����������Ϣ�ڵ�
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

// Ѱ��IDΪentity_id ����������Ϣ�ڵ�
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
*���º������ڲ��������ն˵ĵ���Ϣ����
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

// ��ʼ���ļ�����������Ϣ����Ϊ˫��ѭ������
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

// ��ʼ���ļ�����������Ϣ����ڵ�
void init_terminal_dblist_node_info( tmnl_pdblist node )
{
	assert( node );
	memset(node, 0, sizeof(tmnl_pdblist));
	node->tmnl_dev.tmnl_status.mic_state = 0;
	node->tmnl_dev.tmnl_status.is_rgst = false;
	node->tmnl_dev.address.addr = 0xffff;
	node->tmnl_dev.spk_operate_timp = 0;
}

// �ݻ�ָ���ڵ�
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

// ���½ڵ����������
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

// ��ȡterminal����ĳ���
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

// ɾ���ڵ�
void delect_terminal_dblist_node( tmnl_pdblist *free_node )
{
	assert( *free_node );
	tmnl_pdblist *p = free_node;
	(*p)->next->prior = (*p)->prior;
	(*p)->prior->next = (*p)->next;
	destroy_terminal_dblist_node( p );
}

// Ѱ��IDΪentity_id ����������Ϣ�ڵ�
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

// Ѱ��IDΪentity_id ����������Ϣ�ڵ�
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


// Ѱ�ҵ�ַΪaddress ����������Ϣ�ڵ�
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

// �����Ӧ������,����ͷ���, ɾ���Ľڵ㶼��ͷ����next�ڵ�
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

// �ݻ���Ӧ���������ص�����
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

// ������ȡ���ڵ�2016/1/30
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

// ���ݵ�ַ�Ĵ�С�������򣬴�С���󡣾����Ĳ��Դ˽ӿ���ʱû����ʲô����--2016/1/30
int sort_terminal_dblist_node( tmnl_pdblist guard )
{
	assert( guard );
	if( guard == NULL )
	{
		DEBUG_INFO( "sort terminal dblist err: guard NULL" );
		return -1;
	}

	/*
	**1�����������ҵ����ģ��ŵ�ͷ�����棬����־
	**2����ʣ���������ģ��ŵ��ϴ�����ǰ�棬����־��ֱ��ͷ���
	**3��ǰ����β�ڵ�������
	*/
	tmnl_pdblist key_node = guard->next, remainder_max_node = guard->next;
	tmnl_pdblist next_node = NULL, tmp_node = NULL, loop_node = NULL;
	/**/
	bool found_max_node = false;
	for( tmp_node =  guard->next; (tmp_node != guard) ; tmp_node = tmp_node->next )
	{// �ҵ����ڵ�
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

	// ����remainder_max_node ����β
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
	{// ����ֻʣ��ͷ����ʾ�������
		for( tmp_node =  guard->next; (tmp_node != guard) && (tmp_node != remainder_max_node); tmp_node = tmp_node->next )
		{// �ҵ����ڵ�
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
			// ����key_node 
			key_node->prior->next = key_node->next;
			key_node->next->prior = key_node->prior;

			// �嵽�ϴ��ҵ����ڵ��ǰ��
			key_node->prior = remainder_max_node->prior;
			remainder_max_node->prior->next = key_node;
			key_node->next = remainder_max_node;
			remainder_max_node->prior = key_node;

			// ���±����ҵ������ڵ�
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

