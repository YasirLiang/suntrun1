#include "entity.h"
#include"linked_list_unit.h"
#include "util.h"
#include "adp.h"
#include "adp_controller_machine.h"
#include "jdksavdecc_aem_descriptor.h"
#include "aecp_controller_machine.h"
#include "terminal_common.h" // add date:2016/1/23 ;for register terminal;func interface:terminal_common_create_node_by_adp_discover_can_regist
#include "log_machine.h"
#include "controller_machine.h"

#ifdef __DEBUG__
//#define __ADP_MACHINE_DEBUG__
#endif

void adp_entity_state_avail( solid_pdblist guard,  solid_pdblist exist_node, const struct jdksavdecc_adpdu_common_control_header *adp_hdr )
{
	adp_entity_time_update(  (timetype)(adp_hdr->valid_time *2 *1000 ) , exist_node );// 2s到61s之间(2016-07-08-add double time because of proccessing audio data)
}

inline void adp_entity_post_timeout_msr( solid_pdblist target )
{
	if( !target->solid.connect_flag && target->solid.time.elapsed )
	if (NULL != gp_log_imp)
		gp_log_imp->log.post_log_msg( &gp_log_imp->log, LOGGING_LEVEL_ERROR, " END_STATION DISCONNECT: 0x%llx ", target->solid.entity_id );
}

void adp_entity_time_tick( solid_pdblist guard )
{
	solid_pdblist ptr = NULL;

	if( guard == NULL )
		return;
	
	for( ptr = guard->next; ptr != NULL && ptr != guard; ptr = ptr->next )
	{
		// 如果不超时, 检查超时; 如果超时了,则不检查
		if( !ptr->solid.time.elapsed )
		{
			adp_entity_timeout( ptr );
			
			// 连接超时，设置连接状态为DISCONNECT，并不删除对应实体节点
			if( ptr->solid.time.elapsed )
			{
				ptr->solid.connect_flag = DISCONNECT;
				adp_entity_post_timeout_msr( ptr );

				assert( dev_terminal_list_guard );
				tmnl_pdblist disconnect_node = search_terminal_dblist_entity_id_node(ptr->solid.entity_id, dev_terminal_list_guard );
				if( disconnect_node != NULL )
				{
					disconnect_node->tmnl_dev.tmnl_status.is_rgst = false;
					terminal_delect_register_addr(disconnect_node->tmnl_dev.address.addr);
				}
			}
		}
	}
}

void adp_entity_time_start( timetype timeout, solid_pdblist new_adp_node )
{
	new_adp_node->solid.time.running = true;
	new_adp_node->solid.time.count_time = timeout;
	new_adp_node->solid.time.elapsed = false;	// 超时标志
	new_adp_node->solid.time.start_time = get_current_time();
}

void adp_entity_time_stop(solid_pdblist new_adp_node)
{
	new_adp_node->solid.time.running = false;
	new_adp_node->solid.time.elapsed = false;	// 超时标志
}

void adp_entity_time_update(timetype timeout, solid_pdblist new_adp_node)
{
	new_adp_node->solid.time.running = true;
	new_adp_node->solid.time.count_time = timeout;
	new_adp_node->solid.time.elapsed = false;	// 超时标志
	new_adp_node->solid.time.start_time = get_current_time();
}

void  adp_entity_timeout( solid_pdblist adp_node )
{
	if( adp_node->solid.time.running && !adp_node->solid.time.elapsed )
        {
        	uint32_t elapsed_ms;
            	timetype current_time = get_current_time();
            	elapsed_ms = (uint32_t)(current_time - adp_node->solid.time.start_time);

           	 if(elapsed_ms > adp_node->solid.time.count_time)
            	{
                	adp_node->solid.time.elapsed = true;
            	}
	}
}

int default_send_reboot_cmd(uint64_t entity_entity_id, uint16_t desc_type, uint16_t desc_index)
{
	struct jdksavdecc_frame cmd_frame;
        struct jdksavdecc_aem_command_reboot aem_cmd_reboot;
	struct jdksavdecc_eui48 destination_mac;
	struct jdksavdecc_eui64 target_entity_id;
	
	memset(&aem_cmd_reboot, 0, sizeof( aem_cmd_reboot ));
	memcpy( cmd_frame.src_address.value, net.m_my_mac, 6 );
	convert_entity_id_to_eui48_mac_address( entity_entity_id, destination_mac.value );
	convert_uint64_to_eui64( target_entity_id.value, entity_entity_id);
	
	aem_cmd_reboot.aem_header.command_type = JDKSAVDECC_AEM_COMMAND_REBOOT;
	aem_cmd_reboot.descriptor_type = desc_type;
	aem_cmd_reboot.descriptor_index = desc_index;
	int r = aecp_aem_form_msg(&cmd_frame,
			                       &aem_cmd_reboot,
			                       JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND,
			                       JDKSAVDECC_AEM_COMMAND_REBOOT,
			                       0,
			                       destination_mac,
			                       target_entity_id,
			                       (uint8_t*)&aem_cmd_reboot.descriptor_type,
			                       (size_t)4);
	if (r >= 0)
		system_raw_packet_tx( cmd_frame.dest_address.value, cmd_frame.payload, cmd_frame.length, RUNINFLIGHT, TRANSMIT_TYPE_AECP, false);

	return r;
}

static solid_pdblist gstatic_node = NULL;
int adp_1722_unit_reboot_pro(void)
{
	static uint32_t  last_time = 0;
	uint32_t current_time = get_current_time();
	static solid_pdblist static_node = NULL;
	solid_pdblist tmp_node = endpoint_list->next;

	if (static_node == NULL && gstatic_node != NULL)
		static_node = gstatic_node->next;
	
	if (static_node != endpoint_list && static_node->solid.reboot_times == 0)
	{
		if ((current_time - last_time) > (uint32_t)3000)
		{
			default_send_reboot_cmd(static_node->solid.entity_id, JDKSAVDECC_DESCRIPTOR_ENTITY, 0);
			static_node->solid.reboot_times = 1;
			static_node = static_node->next;
			last_time = current_time;
		}

		if (static_node == gstatic_node)
			return 0;
	}

	return -1;
}

solid_pdblist adp_proccess_new_entity( solid_pdblist guard, solid_pdblist* new_entity, const struct jdksavdecc_adpdu *src_du, const int list_len )
{
	const struct jdksavdecc_adpdu *adp_du = src_du;
	uint64_t entity_entity_id = 0; 

	if (gstatic_node == NULL)
		gstatic_node = guard;
	
	convert_eui64_to_uint64( adp_du->header.entity_id.value, &entity_entity_id );
		
	*new_entity = create_endpoint_new_node( new_entity );
	if( NULL != *new_entity )
	{
		solid_pdblist success_node = *new_entity;
		memset(success_node, 0, sizeof(solid_dblist));
		(*success_node).solid.reboot_times = 0;
		(*success_node).solid.entity_id = entity_entity_id;
		adp_entity_time_start( (timetype)adp_du->header.valid_time * 2 * 1000, success_node);// 2s到61s之间(2016-07-08-add double time because of proccessing audio data)
		(*success_node).solid.connect_flag = CONNECT;
		(*success_node).solid.entity_index = list_len; 		//索引从零开始 
		(*success_node).solid.available_index = adp_du->available_index;
		memcpy(&(*success_node).solid.entity_model_id, &adp_du->entity_model_id, sizeof(struct jdksavdecc_eui64));
		insert_endpoint_dblist_trail(guard,success_node);
		
#ifdef __ADP_MACHINE_DEBUG__
		DEBUG_INFO( "list lenght = %d", list_len + 1 );// 新创建节点插入到链表前链表长度为原来的长度
		endpoint_dblist_show( guard );
#endif

#ifdef __ADP_NOT_REBOOT_ENTITY__
		// 重启终端
		default_send_reboot_cmd(entity_entity_id, JDKSAVDECC_DESCRIPTOR_ENTITY, 0);
#else
		aecp_read_desc_init( JDKSAVDECC_DESCRIPTOR_ENTITY, 0, entity_entity_id );
#endif
		return success_node;
	}
	else
	{
#ifdef __ADP_MACHINE_DEBUG__
		DEBUG_INFO( "no place for new entity!" );
#endif
		return NULL;
	}

	return NULL;
}

// not add to inflight cmd
ssize_t transmit_adp_packet_to_net( uint8_t* frame,  uint32_t frame_len, inflight_plist resend_node, bool resend, const uint8_t dest_mac[6], bool resp, uint32_t *interval_time )
{
	assert( interval_time );
	*interval_time = 0;

	// ready to send
	uint8_t tx_frame[2048] = {0};
	uint8_t ethertype[2] = {0x22, 0xf0};
	int send_len = frame_len + ETHER_HDR_SIZE;

	if (send_len <= 2048)
	{
		if (dest_mac)
		{
			memcpy(tx_frame+0, dest_mac, 6);
		}
		else
		{
			memcpy(tx_frame+0, net.m_default_dest_mac, 6);
		}

		memcpy(tx_frame+6, net.m_my_mac, 6);
		memcpy(tx_frame+12, ethertype, 2);
		memcpy(tx_frame + ETHER_HDR_SIZE, frame, frame_len);
		
		controller_machine_1722_network_send(gp_controller_machine, tx_frame, send_len);
	}
	else
		send_len = -1;
	
	return (ssize_t)send_len;
}

void adp_entity_avail( struct jdksavdecc_eui64 discover_entity_id, uint16_t msg_type )
{
	struct jdksavdecc_adpdu adpdu;
	struct jdksavdecc_frame frame;
	bzero( &adpdu, sizeof( adpdu ) );
	jdksavdecc_frame_init( &frame );
        memcpy( frame.src_address.value, net.m_my_mac, 6 );

	if ( adp_form_msg( &frame, &adpdu, msg_type, discover_entity_id ) == 0 )
	{
		system_raw_packet_tx( frame.dest_address.value, frame.payload, frame.length, RUNINFLIGHT, TRANSMIT_TYPE_ADP,  true );// 暂时不需要响应数据
	}
	else
	{
		DEBUG_INFO( "adp form msg failed!");
	}
}


