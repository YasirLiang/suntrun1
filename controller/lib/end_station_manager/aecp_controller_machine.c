#include "aecp_controller_machine.h"
#include "aecp.h"
#include "conference_end_to_host.h"
#include "terminal_command.h"

static uint16_t aecp_seq_id = 0;
static solid_pdblist aecp_solid_guard = NULL;
static desc_pdblist aecp_desc_guard = NULL;
static inflight_plist aecp_inflight_guard = NULL;

void aecp_controller_init( solid_pdblist solid_guard_node, desc_pdblist desc_guard, inflight_plist inflight_guard )
{
	assert( solid_guard_node && desc_guard && inflight_guard);
	aecp_solid_guard = solid_guard_node;
	aecp_desc_guard = desc_guard;
	aecp_inflight_guard = inflight_guard;
}

int transmit_aecp_packet_network( uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, const uint8_t dest_mac[6], bool resp )
{
	uint8_t subtype = jdksavdecc_subtype_data_get_subtype( frame, ZERO_OFFSET_IN_PAYLOAD ); // msg_type in there is sbu
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t timeout = 200;// ��ʱ��ֵ���������޸�
	inflight_plist inflight_station = NULL;
	uint16_t cmd_type = jdksavdecc_aecpdu_aem_get_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
        cmd_type &= 0x7FFF;
	uint8_t conference_cmd = 0;
	uint16_t terminal_address = 0;
		
	if( (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND) &&(cmd_type == JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR) )
		timeout = 750;	// 750 ms timeout (1722.1 timeout is 250ms)
		
	if( msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND )// conference data in this subtype data payload
	{
		conference_cmd = conference_command_type_read( frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET );
		terminal_address = conferenc_terminal_read_address_data( frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET );
	}
	
	if(!resp )	// not a response data 
	{
		if( !resend )	// data first send
		{
			inflight_station = create_inflight_dblist_new_node( &inflight_station );
			if( NULL == inflight_station )
			{
				DEBUG_INFO("inflight station node create failed!");
				return -1;
			}
			
			memset(inflight_station, 0, sizeof(inflight_list));
			inflight_station->host_tx.inflight_frame.frame = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &inflight_station->host_tx.inflight_frame.frame );
			memset(inflight_station->host_tx.inflight_frame.frame, 0, TRANSMIT_DATA_BUFFER_SIZE);
			if( NULL != inflight_station->host_tx.inflight_frame.frame )
			{
				
				memcpy( inflight_station->host_tx.inflight_frame.frame, frame, frame_len);
				inflight_station->host_tx.inflight_frame.inflight_frame_len = frame_len;
				inflight_station->host_tx.inflight_frame.data_type = subtype; 	//Э��aecp acmp adp udpclient udpserver (fb fc fa ac )
				inflight_station->host_tx.inflight_frame.seq_id = aecp_seq_id;	// ��ʼΪ��
				jdksavdecc_aecpdu_common_set_sequence_id( aecp_seq_id++, frame, 0 );
				jdksavdecc_aecpdu_common_set_sequence_id( aecp_seq_id-1, inflight_station->host_tx.inflight_frame.frame, 0 );
				inflight_station->host_tx.inflight_frame.notification_flag = RUNINFLIGHT;
				memcpy( &inflight_station->host_tx.inflight_frame.raw_dest, dest_mac , 6 );
				
				inflight_station->host_tx.command_type = TRANSMIT_TYPE_AECP;
				inflight_station->host_tx.flags.retried = 1;	// meaning send once
				inflight_station->host_tx.flags.resend = false;
				inflight_timer_start(timeout, inflight_station );

				if( msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND ) // �������ݵ�����ʶ�𷽷�(�������Ѿ�������1722Э��㴦������)
				{
					inflight_station->host_tx.inflight_frame.conference_data_recgnize.conference_command = conference_cmd;
					inflight_station->host_tx.inflight_frame.conference_data_recgnize.address = terminal_address;
				}

				// ���½���inflight���������������β��
				insert_inflight_dblist_trail( guard, inflight_station );
			}
			else
			{
				DEBUG_INFO("Err frame malloc !");
				assert( NULL != inflight_station->host_tx.inflight_frame.frame );
			}
		}
		else
		{
			uint16_t seq_id = jdksavdecc_aecpdu_common_get_sequence_id( frame, ZERO_OFFSET_IN_PAYLOAD);
			inflight_station = search_node_inflight_from_dblist( guard, seq_id , subtype);
			
			if( inflight_station != NULL ) // already search it
			{
				inflight_station->host_tx.flags.resend = true;
				inflight_station->host_tx.flags.retried++ ;
				inflight_timer_state_avail( timeout, inflight_station );
			}
			else
			{
				DEBUG_INFO( "nothing to be resend: subtype = 0x%02x, seq_id = %d ", subtype, seq_id );
				assert(inflight_station != NULL);
			}
		}
	}

	// ready to send
	ssize_t send_len = raw_send( &net, dest_mac, frame, frame_len );
	if( send_len < 0 )
	{
		DEBUG_INFO( "Err raw send data!");
		assert( send_len >= 0);
	}
	
	return send_len;
}

void aecp_inflight_station_timeouts( inflight_plist aecp_sta, inflight_plist hdr )
{
	bool is_retried = false;
	inflight_plist aecp_pstation = NULL;
	uint8_t *frame = NULL;
	uint16_t frame_len = 0;
         
	if( aecp_sta != NULL )
	{
		aecp_pstation = aecp_sta;
		is_retried = is_inflight_cmds_retried( aecp_pstation );
		frame = aecp_pstation->host_tx.inflight_frame.frame;
		frame_len = aecp_pstation->host_tx.inflight_frame.inflight_frame_len;
	}
	else 
	{
		DEBUG_INFO( "noting to be proccessed by aecp timeout" );
		return;
	}

	assert( frame );
	if( is_retried )
	{
		uint64_t dest_id = 0;
            	struct jdksavdecc_eui64 id = jdksavdecc_common_control_header_get_stream_id(frame, 0);
		convert_eui64_to_uint64( id.value, &dest_id);
		uint8_t msg_type = jdksavdecc_common_control_header_get_control_data( frame, 0 );
		uint16_t cmd_type = jdksavdecc_aecpdu_aem_get_command_type(frame, 0 );
           	cmd_type &= 0x7FFF;
		
		if( msg_type != JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND )// host and end deal identify
		{
           		uint16_t desc_type = jdksavdecc_aem_command_read_descriptor_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
            		uint16_t desc_index = jdksavdecc_aem_command_read_descriptor_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
			MSGINFO( "[ COMMAND TIMEOUT: 0x%llx, %s, %s, %d ]", dest_id, get_aem_command_string(cmd_type),get_aem_desc_command_string( desc_type ), desc_index);
		}
		else
		{
			uint8_t cfc_cmd = conference_command_type_read( frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET);
			cfc_cmd &= 0x1f;// �����ڵ���λ
			MSGINFO( "[ UNIQUE CONFERENCE COMMAND TIMEOUT: 0x%llx, %s(0x%02x) ( data len = %d ) ]", dest_id, get_host_and_end_conference_string_value(cfc_cmd), cfc_cmd,cmd_type );
		}

		// free inflight command node in the system
		//DEBUG_INFO( "aecp inflight delect: msg_tyep = %02x, seq_id = %d", aecp_pstation->host_tx.inflight_frame.data_type, aecp_pstation->host_tx.inflight_frame.seq_id);
		release_heap_space( &aecp_pstation->host_tx.inflight_frame.frame );// must release frame space first while need to free inflight node
		delect_inflight_dblist_node( &aecp_pstation );
		
		is_inflight_timeout = true; // ���ó�ʱ
		DEBUG_INFO( "is_inflight_timeout = %d", is_inflight_timeout );
	}
	else
	{
		transmit_aecp_packet_network( frame, frame_len, hdr, true, aecp_pstation->host_tx.inflight_frame.raw_dest.value, false );
	}
}

int aecp_read_desc_init(uint16_t desc_type, uint16_t desc_index, uint64_t target_entity_id)
{
	return aecp_send_read_desc_cmd_with_flag( desc_type, desc_index, target_entity_id);
}

int aecp_send_read_desc_cmd( uint16_t desc_type, uint16_t desc_index, uint64_t target_entity_id)
{
	return aecp_send_read_desc_cmd_with_flag( desc_type, desc_index, target_entity_id);
}

int aecp_send_read_desc_cmd_with_flag( uint16_t desc_type, uint16_t desc_index, uint64_t entity_id)
{
	struct jdksavdecc_frame cmd_frame;
	struct jdksavdecc_aem_command_read_descriptor aem_command_read_desc;
	struct jdksavdecc_eui48 destination_mac;
	struct jdksavdecc_eui64 target_entity_id;
	
	memset(&aem_command_read_desc, 0, sizeof( aem_command_read_desc ));
	memcpy( cmd_frame.src_address.value, net.m_my_mac, 6 );
	convert_entity_id_to_eui48_mac_address( entity_id, destination_mac.value );
	convert_uint64_to_eui64( target_entity_id.value, entity_id);

	desc_pdblist desc_node = search_desc_dblist_node( entity_id, aecp_desc_guard );
	if( desc_node == NULL && desc_type != JDKSAVDECC_DESCRIPTOR_ENTITY )
	{
		DEBUG_INFO( "search descptor node 0x%016llx failed: no such node!", entity_id );
		return -1;
	}
	
	aem_command_read_desc.configuration_index = (desc_type == JDKSAVDECC_DESCRIPTOR_ENTITY || desc_type == JDKSAVDECC_DESCRIPTOR_CONFIGURATION) ?
                                                    0 : desc_node->endpoint_desc.entity_desc.current_configuration;
	if ( aecp_aem_form_read_descriptor_command(
                     &cmd_frame, &aem_command_read_desc, aecp_seq_id, destination_mac, target_entity_id, desc_type, desc_index ) == 0 )
        {
        	system_raw_packet_tx( cmd_frame.dest_address.value, cmd_frame.payload, cmd_frame.length, RUNINFLIGHT, TRANSMIT_TYPE_AECP, false);
		return 0;
        }
	else
	{
		DEBUG_INFO( "form read descriptor failed!" );
        	return -1;
	}

	return 0;
}

int  aecp_update_inflight_for_vendor_unique_message(uint32_t msg_type, const uint8_t *frame, size_t frame_len, int *status )
{
	assert( frame && status );
	struct jdksavdecc_frame jdk_frame;
	memcpy( jdk_frame.payload, frame, frame_len );
	*status = jdksavdecc_common_control_header_get_status( frame, ZERO_OFFSET_IN_PAYLOAD );

	switch( msg_type )
	{
		case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND:
		case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE:  // connferece deal uses
			aecp_state_rcvd_resp( &jdk_frame);
		break;
		default:
			DEBUG_INFO( "LOGGING_LEVEL_ERROR: Invalid message type");
			return -1;
	}

	if( *status ==  JDKSAVDECC_AECP_VENDOR_STATUS_SUCCESS )
		return 0;
	else 
		return -1;
}

int aecp_update_inflight_for_rcvd_resp( uint32_t msg_type, bool u_field, struct jdksavdecc_frame *cmd_frame)
{
	switch (msg_type)
	{
		case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE:
		case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE: // Fallthrough intentional
		if (u_field)
		{
			aecp_state_rcvd_resp( cmd_frame );
		}
		else
		{
			aecp_state_rcvd_resp( cmd_frame);
		}
		break;
		default:
			DEBUG_INFO( "LOGGING_LEVEL_ERROR: Invalid message type");
			return -1;
	}

	return 0;
}

int aecp_state_rcvd_resp( struct jdksavdecc_frame *cmd_frame)
{
	assert(cmd_frame);
	return aecp_proc_resp( cmd_frame);
}

int aecp_proc_resp( struct jdksavdecc_frame *cmd_frame)
{
	assert(cmd_frame);
	uint8_t subtype = jdksavdecc_common_control_header_get_subtype( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD );
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD );
	uint16_t seq_id = jdksavdecc_aecpdu_common_get_sequence_id(cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD );
	uint32_t notification_flag = 0;
	inflight_plist inflight_aecp = NULL;
	uint8_t conference_cmd = 0;
	uint16_t terminal_address = 0;

	if( msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND)
	{
		conference_cmd = conference_command_type_read( cmd_frame->payload, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET );
		conference_cmd &=0x1f;// ����λ�ڵͰ�λ
		terminal_address = conferenc_terminal_read_address_data( cmd_frame->payload, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET );
		
		inflight_aecp = search_for_conference_inflight_dblist_node( aecp_inflight_guard, subtype, conference_cmd );
		if( inflight_aecp != NULL )
		{
			if( (inflight_aecp->host_tx.inflight_frame.conference_data_recgnize.address == CONFERENCE_BROADCAST_ADDRESS) || (terminal_address == inflight_aecp->host_tx.inflight_frame.conference_data_recgnize.address) )
			{
				notification_flag = inflight_aecp->host_tx.inflight_frame.notification_flag;
				aecp_callback( notification_flag, cmd_frame->payload );
				release_heap_space( &inflight_aecp->host_tx.inflight_frame.frame);// must release frame space first while need to free inflight node
				delect_inflight_dblist_node( &inflight_aecp );	// delect aecp inflight node
			}
			else
			{
				DEBUG_INFO( " no such right address inflight cmd aecp node:subtype = %02x, conference_cmd = %d terminal_address = %04x[inflight node info: %02x %d %04x]", \
					subtype, conference_cmd, terminal_address, inflight_aecp->host_tx.inflight_frame.data_type,\
					inflight_aecp->host_tx.inflight_frame.conference_data_recgnize.conference_command, inflight_aecp->host_tx.inflight_frame.conference_data_recgnize.address );
			}
		}
		else
		{
			DEBUG_INFO( " no such inflight cmd aecp node:subtype = %02x, conference_cmd = %d terminal_address = %04x", subtype, conference_cmd, terminal_address );
			return -1;
		}
	}
	else
	{
		inflight_aecp = search_node_inflight_from_dblist( aecp_inflight_guard, seq_id, subtype );	// found?
		if( NULL != inflight_aecp )
		{
			notification_flag = inflight_aecp->host_tx.inflight_frame.notification_flag;
			aecp_callback( notification_flag, cmd_frame->payload );
			//DEBUG_INFO( "aecp inflight delect: msg_tyep = %02x, seq_id = %d", inflight_aecp->host_tx.inflight_frame.data_type, inflight_aecp->host_tx.inflight_frame.seq_id);
			release_heap_space( &inflight_aecp->host_tx.inflight_frame.frame);// must release frame space first while need to free inflight node
			delect_inflight_dblist_node( &inflight_aecp );	// delect aecp inflight node
		}
	        else
		{
			DEBUG_INFO( " no such inflight cmd aecp node:subtype = %02x, seq_id = %d", subtype,seq_id);
			return -1;
		}
	}

	return -1;
}

int aecp_callback( uint32_t notification_flag, uint8_t *frame)
{
        uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
        uint16_t cmd_type = jdksavdecc_aecpdu_aem_get_command_type(frame, ZERO_OFFSET_IN_PAYLOAD);
        cmd_type &= 0x7FFF;
        uint32_t status = jdksavdecc_common_control_header_get_status(frame, ZERO_OFFSET_IN_PAYLOAD);
        uint16_t desc_type = 0;
        uint16_t desc_index = 0;

	if( (msg_type !=  JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND) && (msg_type !=  JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE))
	{
	        switch(cmd_type)
	        {
		        case JDKSAVDECC_AEM_COMMAND_ACQUIRE_ENTITY:
		            desc_type = jdksavdecc_aem_command_acquire_entity_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_acquire_entity_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_LOCK_ENTITY:
		            desc_type = jdksavdecc_aem_command_lock_entity_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_lock_entity_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_ENTITY_AVAILABLE:
		            break;

		        case JDKSAVDECC_AEM_COMMAND_CONTROLLER_AVAILABLE:
		            break;

		        case JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR:
		            desc_type = jdksavdecc_aem_command_read_descriptor_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_read_descriptor_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_SET_STREAM_FORMAT:
		            desc_type = jdksavdecc_aem_command_set_stream_format_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_set_stream_format_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_GET_STREAM_FORMAT:
		            desc_type = jdksavdecc_aem_command_get_stream_format_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_stream_format_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_SET_STREAM_INFO:
		            desc_type = jdksavdecc_aem_command_set_stream_info_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_set_stream_info_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_GET_STREAM_INFO:
		            desc_type = jdksavdecc_aem_command_get_stream_info_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_stream_info_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_SET_NAME:
		            desc_type = jdksavdecc_aem_command_set_name_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_set_name_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_GET_NAME:
		            desc_type = jdksavdecc_aem_command_get_name_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_name_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_SET_SAMPLING_RATE:
		            desc_type = jdksavdecc_aem_command_set_sampling_rate_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_set_sampling_rate_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_GET_SAMPLING_RATE:
		            desc_type = jdksavdecc_aem_command_get_sampling_rate_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_sampling_rate_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_SET_CLOCK_SOURCE:
		            desc_type = jdksavdecc_aem_command_set_clock_source_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_set_clock_source_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_GET_CLOCK_SOURCE:
		            desc_type = jdksavdecc_aem_command_get_clock_source_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_clock_source_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_START_STREAMING:
		            desc_type = jdksavdecc_aem_command_start_streaming_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_start_streaming_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;

		        case JDKSAVDECC_AEM_COMMAND_STOP_STREAMING:
		            desc_type = jdksavdecc_aem_command_stop_streaming_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_stop_streaming_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;
		        
		        case JDKSAVDECC_AEM_COMMAND_GET_COUNTERS:
		            desc_type = jdksavdecc_aem_command_get_counters_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		            desc_index = jdksavdecc_aem_command_get_counters_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
		            break;
		        
		        case JDKSAVDECC_AEM_COMMAND_REGISTER_UNSOLICITED_NOTIFICATION:
		            break;

		        default:
		           DEBUG_INFO("LOGGING_LEVEL_DEBUG:NO_MATCH_FOUND for %s", aem_cmd_value_to_name(cmd_type));
		            break;
	        }
	}
	
        struct jdksavdecc_eui64 id = jdksavdecc_common_control_header_get_stream_id(frame, ZERO_OFFSET_IN_PAYLOAD);
        if((notification_flag == CMD_WITH_NOTIFICATION) &&
            ((msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE) ||
            (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE)))
        {
        	DEBUG_ONINFO("[ RESPONSE_RECEIVED, 0x%016llx, %d, %d, %d, %d ]",
						jdksavdecc_uint64_get(&id, 0),
						cmd_type,
						desc_type,
	                                        desc_index,
	                                        status);

            if(status != AEM_STATUS_SUCCESS)
            {
                DEBUG_INFO( "LOGGING_LEVEL_ERROR,RESPONSE_RECEIVED, 0x%016llx, %s, %s, %d, %d, %s",
                                          jdksavdecc_uint64_get(&id, 0),
                                          aem_cmd_value_to_name(cmd_type),
                                          aem_desc_value_to_name(desc_type),
                                          desc_index,
                                          jdksavdecc_aecpdu_common_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD),
                                          aem_cmd_status_value_to_name(status));
            }
        }
        else if(((notification_flag == CMD_WITH_NOTIFICATION) || (notification_flag == CMD_WITHOUT_NOTIFICATION)) &&
                ((msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND) || (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_COMMAND)))
        {
                DEBUG_ONINFO( "[ LOGGING_LEVEL_DEBUG:COMMAND_SENT, 0x%016llx, %s, %s, %d, %d ]",
                                      jdksavdecc_uint64_get(&id, 0),
                                      aem_cmd_value_to_name(cmd_type),
                                      aem_desc_value_to_name(desc_type),
                                      desc_index,
                                      jdksavdecc_aecpdu_common_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD));
        }
	else  if((notification_flag == CMD_WITH_NOTIFICATION) &&
            ((msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND) ||
            (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE)))
        {
        	uint8_t addr[2] ={0};
        	uint16_t conference_data_len = jdksavdecc_aecpdu_aem_get_command_type(frame, ZERO_OFFSET_IN_PAYLOAD);
		uint8_t conference_cmd = conference_command_type_read( frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET);
		addr[0] = get_conference_guide_type(frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET + 2 );
		addr[1] = get_conference_guide_type(frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET + 3 );
		uint8_t data_len = get_conference_guide_type(frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET + 4 );
		conference_cmd &= 0x1f;// ����λ
		
		DEBUG_ONINFO("[ UNIQUE CONFENENCE COMMAND, 0x%016llx, %s( %02x ), 0x%02x%02x, %d(data_len), %d(all len), (status = %s) ]",
						jdksavdecc_uint64_get(&id, 0),
						get_host_and_end_conference_string_value(conference_cmd),
						conference_cmd,
						addr[1],
						addr[0],
						data_len,
						conference_data_len,
	                                        aecp_vendor_unique_status_value_to_name(status));

           	if( status != JDKSAVDECC_AECP_VENDOR_STATUS_SUCCESS )
            	{
                	DEBUG_INFO("[ UNIQUE CONFENENCE COMMAND ERR, 0x%016llx, %s( %02x ), 0x%02x%02x, %d, %d, (status = %s) ]",
						jdksavdecc_uint64_get(&id, 0),
						get_host_and_end_conference_string_value(conference_cmd),
						conference_cmd ,
						addr[0],
						addr[1],
						data_len,
						conference_data_len,
	                                        aecp_vendor_unique_status_value_to_name(status));
            	}
        }
        else if((notification_flag == CMD_WITHOUT_NOTIFICATION) &&
                ((msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE) ||
                (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE)))
        {
            if(status == AEM_STATUS_SUCCESS)
            {
                    DEBUG_ONINFO( "[ LOGGING_LEVEL_DEBUG: RESPONSE_RECEIVED, 0x%llx, %s, %s, %d, %d, %s ]",
                                          jdksavdecc_uint64_get(&id, 0),
                                          aem_cmd_value_to_name(cmd_type),
                                          aem_desc_value_to_name(desc_type),
                                          desc_index,
                                          jdksavdecc_aecpdu_common_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD),
                                          aem_cmd_status_value_to_name(status));
            }
            else
            {
                    DEBUG_INFO( "[ LOGGING_LEVEL_ERROR:RESPONSE_RECEIVED, 0x%llx, %s, %s, %d, %d, %s ]",
                                          jdksavdecc_uint64_get(&id, 0),
                                          aem_cmd_value_to_name(cmd_type),
                                          aem_desc_value_to_name(desc_type),
                                          desc_index,
                                          jdksavdecc_aecpdu_common_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD),
                                          aem_cmd_status_value_to_name(status));
            }
        }
		
	return 0;
}

