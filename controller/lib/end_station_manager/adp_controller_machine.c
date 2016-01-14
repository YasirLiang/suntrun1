#include "entity.h"
#include"linked_list_unit.h"
#include "util.h"
#include "adp.h"
#include "adp_controller_machine.h"
#include "jdksavdecc_aem_descriptor.h"
#include "aecp_controller_machine.h"

void adp_entity_state_avail( solid_pdblist guard,  solid_pdblist exist_node, const struct jdksavdecc_adpdu_common_control_header *adp_hdr )
{
	adp_entity_time_update(  (timetype)(adp_hdr->valid_time *2 *1000) , exist_node );// 2s��61s֮��
}

inline void adp_entity_post_timeout_msr( solid_pdblist target )
{
	if( !target->solid.connect_flag && target->solid.time.elapsed )
		DEBUG_ONINFO( "[ END_STATION DISCONNECT: 0x%llx ]", target->solid.entity_id );
}

void adp_entity_time_tick( solid_pdblist guard )
{
	solid_pdblist ptr = NULL;

	if( guard == NULL )
		return;
	
	for( ptr = guard->next; ptr != NULL && ptr != guard; ptr = ptr->next )
	{
		// �������ʱ, ��鳬ʱ; �����ʱ��,�򲻼��
		if( !ptr->solid.time.elapsed )
		{
			adp_entity_timeout( ptr );
			
			// ���ӳ�ʱ����������״̬ΪDISCONNECT������ɾ����Ӧʵ��ڵ�
			if( ptr->solid.time.elapsed )
			{
				ptr->solid.connect_flag = DISCONNECT;
				adp_entity_post_timeout_msr( ptr );
			}
		}
	}
}

void adp_entity_time_start( timetype timeout, solid_pdblist new_adp_node )
{
	new_adp_node->solid.time.running = true;
	new_adp_node->solid.time.count_time = timeout;
	new_adp_node->solid.time.elapsed = false;	// ��ʱ��־
	new_adp_node->solid.time.start_time = get_current_time();
}

void adp_entity_time_stop(solid_pdblist new_adp_node)
{
	new_adp_node->solid.time.running = false;
	new_adp_node->solid.time.elapsed = false;	// ��ʱ��־
}

void adp_entity_time_update(timetype timeout, solid_pdblist new_adp_node)
{
		new_adp_node->solid.time.running = true;
		new_adp_node->solid.time.count_time = timeout;
		new_adp_node->solid.time.elapsed = false;	// ��ʱ��־
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

solid_pdblist adp_proccess_new_entity( solid_pdblist guard, solid_pdblist* new_entity, const struct jdksavdecc_adpdu *src_du, const int list_len )
{
	const struct jdksavdecc_adpdu *adp_du = src_du;
	uint64_t entity_entity_id = 0; 
	
	convert_eui64_to_uint64( adp_du->header.entity_id.value, &entity_entity_id );
		
	*new_entity = create_endpoint_new_node( new_entity );
	if( NULL != *new_entity )
	{
		solid_pdblist success_node = *new_entity;
		memset(success_node, 0, sizeof(solid_dblist));
		(*success_node).solid.entity_id = entity_entity_id;
		adp_entity_time_start( (timetype)adp_du->header.valid_time * 2 * 1000, success_node);// 2s��61s֮��
		(*success_node).solid.connect_flag = CONNECT;
		(*success_node).solid.entity_index = list_len; 		//�������㿪ʼ 
		(*success_node).solid.available_index = adp_du->available_index;
		memcpy(&(*success_node).solid.entity_model_id, &adp_du->entity_model_id, sizeof(struct jdksavdecc_eui64));
		insert_endpoint_dblist_trail(guard,success_node);
	
		DEBUG_INFO( "list lenght = %d", list_len + 1 );// �´����ڵ���뵽����ǰ������Ϊԭ���ĳ���
		endpoint_dblist_show( guard );
		aecp_read_desc_init( JDKSAVDECC_DESCRIPTOR_ENTITY, 0, entity_entity_id );
		
		return success_node;
	}
	else
	{
		DEBUG_INFO( "no place for new entity!" );
		return NULL;
	}

	return NULL;
}

// not add to inflight cmd
ssize_t transmit_adp_packet_to_net( uint8_t* frame,  uint32_t frame_len, inflight_plist guard, bool resend, const uint8_t dest_mac[6], bool resp, uint32_t *interval_time )
{
	assert( interval_time );
	*interval_time = 0;
	
	// ready to send
	ssize_t send_len = raw_send( &net, dest_mac, frame, frame_len );
	if( send_len < 0 )
	{
		DEBUG_INFO( "Err raw send data!");
		assert( send_len >= 0);
	}
	
	return send_len;
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
		system_raw_packet_tx( frame.dest_address.value, frame.payload, frame.length, RUNINFLIGHT, TRANSMIT_TYPE_ADP,  true );// ��ʱ����Ҫ��Ӧ����
	}
	else
	{
		DEBUG_INFO( "adp form msg failed!");
	}
}


