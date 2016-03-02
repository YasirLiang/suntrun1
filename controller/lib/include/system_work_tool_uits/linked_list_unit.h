#ifndef __LINKED_LIST_UNIT_H__
#define __LINKED_LIST_UNIT_H__

#include <stddef.h>
#include "jdksavdecc_world.h"
#include "jdksavdecc_util.h"
#include "host_controller_debug.h"
#include "entity.h"
#include "inflight.h"
#include "descriptor.h"
#include "terminal_pro.h"

#define HEAP_RAW_RECV_BUF_SIZE 2048	// 原始套接字接收缓冲区的大小

/*{@*/
// 寻找ID为entity_id的终端节点,head是链表的哨兵
#define SEARCH_ENTITY_ID_ENDPOINT_NODE( head, pnode, entity_id )\
	for( ; (pnode != head) && (pnode->solid.entity_id != entity_id ); pnode = pnode->next )
		
uint32_t get_available_index_endpoint_dblist_node( solid_pdblist target );
const struct jdksavdecc_eui64* get_entity_model_id_endpoint_dblist_node( solid_pdblist target);
void  update_entity_adpdu_endpoint_dblist(struct jdksavdecc_adpdu *util, solid_pdblist target);
uint8_t *allot_heap_space( int size, uint8_t ** pout);
void release_heap_space( uint8_t **p );
solid_pdblist  create_endpoint_new_node( solid_pdblist* node_new );
void destroy_endpoint_node( solid_pdblist* node_dstry );
solid_pdblist  init_endpoint_dblist( struct entities **head );
void insert_endpoint_dblist_trail( solid_pdblist head, solid_pdblist new_node );
int delect_entdtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id );
solid_pdblist search_endtity_node_endpoint_dblist( solid_pdblist head, const uint64_t entity_id  );
int get_endpoint_dblist_length( solid_pdblist head );
int  get_entity_id_endpoint_dblist( solid_pdblist head, int index, uint64_t *entity_id );
int  get_entity_index_endpoint_dblist( solid_pdblist head, uint64_t entity_id );
int set_connect_flag_endpoint_dblist( solid_pdblist head, uint64_t entity_id, int flags);
void endpoint_dblist_show( const solid_pdblist head );
void destroy_endpoint_dblist( solid_pdblist head );
void delect_node_from_endpoint_dblist( solid_pdblist *free_node );

/*@}*/


/*=====================================
*以上的函数用于操作系统中终端的链表，而
*以下的函数用于操作inflight命令链表
*======================================*/
#define SEARCH_INFLIGHT_SEQ_TYPE_NODE( head, pnode, sequeue, subtype )\
	for( ; (pnode != head)&&(pnode != NULL); pnode = pnode->next )\
		if((pnode->host_tx.inflight_frame.data_type == subtype) && (pnode->host_tx.inflight_frame.seq_id == sequeue))break
#define SEARCH_INFLIGHT_CONFERENCE_TYPE_NODE( head, pnode, cmd, subtype, addr )\
	for( ; (pnode != head)&&(pnode != NULL); pnode = pnode->next )\
		if((addr == pnode->host_tx.inflight_frame.conference_data_recgnize.address)&&(pnode->host_tx.inflight_frame.data_type == subtype) && (pnode->host_tx.inflight_frame.conference_data_recgnize.conference_command == cmd))break

inflight_plist create_inflight_dblist_new_node( inflight_plist *new_node );
void destroy_inflight_node( inflight_plist *node_dstry );
inflight_plist init_inflight_dblist( inflight_plist *guard );
void insert_inflight_dblist_trail( inflight_plist head, inflight_plist new_node );
int get_inflight_dblist_length( inflight_plist head );
inflight_plist search_node_inflight_from_dblist(inflight_plist head, uint16_t  seq_id, uint8_t subtype );
void delect_inflight_dblist_node( inflight_plist *free_node );
inflight_plist search_for_conference_inflight_dblist_node( inflight_plist head, uint8_t subtype,  uint8_t cfr_cmd, uint16_t addr );
bool is_exist_udp_client_inflight_type_node( inflight_plist head, uint8_t subtype );
void destroy_inflight_dblist( inflight_plist guard );


/*{@以下函数用于操作会议终端的的信息链表*/
#define SEARCH_FOR_DESCPTOT_LIST_RIGHT_ENTITY_NODE( head, pnode, entity_id )\
	for( ; pnode != head; pnode = pnode->next )\
		if( pnode->endpoint_desc.entity_id == entity_id)break
			
desc_pdblist search_desc_dblist_node( uint64_t entity_id, desc_pdblist guard );
desc_pdblist search_desc_dblist_node_no_printf_info( uint64_t entity_id, desc_pdblist guard );
int get_desc_dblist_length( desc_pdblist head );
void delect_descptor_dblist_node( desc_pdblist *free_node );
int get_descptor_dblist_length( desc_pdblist head );
void insert_descptor_dblist_trail( desc_pdblist head, desc_pdblist new_node );
void destroy_descptor_dblist_node( desc_pdblist *node_dstry );
void init_descptor_dblist_node_info( desc_pdblist node );
void init_descptor_dblist( desc_pdblist *guard );
desc_pdblist create_descptor_dblist_node( desc_pdblist* node );
void destroy_descptor_dblist( desc_pdblist  head );
/*@}*/

/*=====================================
*
*以下函数用于操作会议终端的的信息链表
*======================================*/
/*{@*/
#define SEARCH_FOR_TERMINAL_LIST_RIGHT_ENTITY_NODE(head, pnode, entity_id )\
		for( ; pnode != head; pnode = pnode->next )\
			if( pnode->tmnl_dev.entity_id == entity_id)break
#define SEARCH_FOR_TERMINAL_LIST_RIGHT_ADDRESS_NODE( guard, pnode, address )\
		for( ; pnode != guard; pnode = pnode->next )\
			if( pnode->tmnl_dev.address.addr == address)break
			
tmnl_pdblist create_terminal_dblist_node( tmnl_pdblist* node );
void init_terminal_dblist( tmnl_pdblist *guard );
void init_terminal_dblist_node_info( tmnl_pdblist node );
void destroy_terminal_dblist_node( tmnl_pdblist* node_dstry );
void insert_terminal_dblist_trail( tmnl_pdblist head, tmnl_pdblist new_node );
int get_terminal_dblist_length( tmnl_pdblist head );
void delect_terminal_dblist_node( tmnl_pdblist *free_node );
tmnl_pdblist search_terminal_dblist_address_node( uint16_t  address, tmnl_pdblist guard );
tmnl_pdblist search_terminal_dblist_entity_id_node( uint64_t entity_id, tmnl_pdblist guard ); 
tmnl_pdblist terminal_dblist_except_free( tmnl_pdblist guard );
tmnl_pdblist destroy_terminal_dblist( tmnl_pdblist guard );
tmnl_pdblist move_node_from_terminal_dblist( tmnl_pdblist get_node );
int sort_terminal_dblist_node( tmnl_pdblist guard );
int show_terminal_dblist( tmnl_pdblist guard );

/*@}*/

#endif

