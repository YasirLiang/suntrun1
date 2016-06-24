#include "end_station_prcs.h"
#include "conference_end_to_host.h"
#include "jdksavdecc_aecp_aem.h"
#include "jdksavdecc_util.h"
#include "conference.h"
#include "avdecc_net.h"
#include "entity.h"
#include "linked_list_unit.h"
#include "util.h"
#include "host_controller_debug.h"
#include "aecp_controller_machine.h"
#include "configuration_descptor.h"
#include "terminal_pro.h"
#include "jdksavdecc_pdu.h"
#include "central_control_recieve_unit.h"
#include "conference_transmit_unit.h"
//#include "muticast_connector.h"
#include "conference_recieve_unit.h"
#include "central_control_transmit_unit.h"

void proc_aecp_message_type_vendor_unique_command_conference( const uint8_t *frame, size_t frame_len, int *status )
{
	assert( frame && status );
	uint32_t msg_type;
	struct terminal_deal_frame conference_frame;
	uint16_t connference_len = jdksavdecc_aecpdu_aem_get_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	bool crc_right = false;

	// 协议数据至少12位,包括备份
	if (connference_len < END_TO_HOST_CMD_LEN*2)
	{
		return;
	}
	
	memset(&conference_frame, 0 , sizeof(struct terminal_deal_frame));
	conference_frame.payload_len = connference_len;
	jdksavdecc_aecpdu_aem_read( &conference_frame.aecpdu_aem_header, frame, 0, JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + 12);
	memcpy( conference_frame.payload, frame + CONFERENCE_DATA_IN_CONTROLDATA_OFFSET,  connference_len );
	msg_type = conference_frame.aecpdu_aem_header.aecpdu_header.header.message_type;
	
	// check the crc of the both data backups,if crc is wrong,return directory
	if( check_conferece_deal_data_crc( connference_len/2, conference_frame.payload, ZERO_OFFSET_IN_PAYLOAD))
	{	
		crc_right = true;
	}
	else
	{
		if( check_conferece_deal_data_crc( connference_len/2, conference_frame.payload + connference_len/2, ZERO_OFFSET_IN_PAYLOAD))
			crc_right = true;
		else	
			return;
	}
	
	if( crc_right )
	{
		// if command is response conference command,update aecp inflight command in the controller system
		if( is_terminal_command( (void*)frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET ) )
		{
			if(is_terminal_response(  (void*)frame, CONFERENCE_DATA_IN_CONTROLDATA_OFFSET ) )
			{	
				aecp_update_inflight_for_vendor_unique_message( msg_type, frame, frame_len, status );
			}

			terminal_recv_message_pro( &conference_frame );
		}
	}
}

int proc_rcvd_acmp_resp( uint32_t msg, const uint8_t *frame, size_t frame_len, int *status  )
{
	uint16_t desc_index = 0;
	
	assert( status );
	switch( msg )
	{
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE:
			desc_index = jdksavdecc_acmpdu_get_talker_unique_id( frame, 0 );
			proc_get_tx_state_resp( frame, frame_len, desc_index, status );
		break;
		
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE: // connect resp
			desc_index = jdksavdecc_acmpdu_get_listener_unique_id( frame, 0 );
			proc_connect_rx_resp( frame, frame_len, status );             
		break;
		
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE:
			desc_index = jdksavdecc_acmpdu_get_listener_unique_id(frame, 0);               
			proc_disconnect_rx_resp( frame, frame_len, status );
		break;
		
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE:
			desc_index = jdksavdecc_acmpdu_get_listener_unique_id(frame, 0);                   
			proc_get_rx_state_resp( frame, frame_len, desc_index, status );
		break;
		
		case JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE:
			desc_index = jdksavdecc_acmpdu_get_talker_unique_id(frame, 0);                  
			proc_get_tx_connection_resp( frame, frame_len, status);
		break;
		
		default:
		DEBUG_INFO("NO_MATCH_FOUND:%d, %d, %d, %d, %d,%d ", 0, msg, 0, 0, 0, 0);
		break;
	}

	return 0;
}

void proc_rcvd_aem_resp( const uint8_t *frame, size_t frame_len, int *status )
{
        uint16_t cmd_type;
        uint16_t desc_type;
        uint16_t desc_index;
        cmd_type = jdksavdecc_aecpdu_aem_get_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
        cmd_type &= 0x7FFF;

        switch(cmd_type)
        {
            	case JDKSAVDECC_AEM_COMMAND_ACQUIRE_ENTITY:// 1
		{
            	    desc_type = jdksavdecc_aem_command_acquire_entity_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
            	    desc_index = jdksavdecc_aem_command_acquire_entity_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_ENTITY)// 1
                    {
			   //proc_acquire_entity_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)// 2
                    {
                        
                    	    //proc_acquire_entity_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)// 3
                    {
                        
                            //proc_acquire_entity_resp(notification_id, frame, frame_len, status);
                       
                    }
                    else
                    {
                        DEBUG_INFO( "LOGGING_LEVEL_ERROR: Descriptor type %d is not implemented.", desc_type);
                    }
		}

                break;
				
            case JDKSAVDECC_AEM_COMMAND_LOCK_ENTITY:// 2
                {
                    desc_type = jdksavdecc_aem_command_lock_entity_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_lock_entity_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_ENTITY)// 1
                    {
                            //proc_lock_entity_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT) // 2
                    {
                            //proc_lock_entity_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT) // 3
                    {
                            //proc_lock_entity_resp(notification_id, frame, frame_len, status);
                    }
                    else
                    {
                        DEBUG_INFO( "LOGGING_LEVEL_ERROR: Descriptor type %d is not implemented.", desc_type);
                    }
                }

                break;

            case JDKSAVDECC_AEM_COMMAND_ENTITY_AVAILABLE: // 3
                //proc_entity_avail_resp( frame, frame_len, status);
                break;

            case JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR: // 4
                proc_read_desc_resp( frame, frame_len, status);
                break;

            case JDKSAVDECC_AEM_COMMAND_SET_STREAM_FORMAT: // 5
                {
                    desc_type = jdksavdecc_aem_command_set_stream_format_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_set_stream_format_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                    {
                           // proc_set_stream_format_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                    {
                          //  proc_set_stream_format_resp(notification_id, frame, frame_len, status);
                    }
                }

                break;

            case JDKSAVDECC_AEM_COMMAND_GET_STREAM_FORMAT: // 6
                {
                    desc_type = jdksavdecc_aem_command_get_stream_format_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_get_stream_format_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                    {
                           // proc_get_stream_format_resp(notification_id, frame, frame_len, status);
                      
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                    {
                           // proc_get_stream_format_resp(notification_id, frame, frame_len, status);
                    }
                }

                break;

            case JDKSAVDECC_AEM_COMMAND_SET_STREAM_INFO: // 7
                desc_type = jdksavdecc_aem_command_set_stream_info_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                desc_index = jdksavdecc_aem_command_set_stream_info_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
                if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                {
                        //proc_set_stream_info_resp(notification_id, frame, frame_len, status);
                }
                else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                {
                         //proc_set_stream_info_resp(notification_id, frame, frame_len, status);
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_GET_STREAM_INFO: // 8
                desc_type = jdksavdecc_aem_command_get_stream_info_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                desc_index = jdksavdecc_aem_command_get_stream_info_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                {
                        //proc_get_stream_info_resp(notification_id, frame, frame_len, status);
                }
                else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                {
                        //proc_get_stream_info_resp(notification_id, frame, frame_len, status);
                }

                break;

            case JDKSAVDECC_AEM_COMMAND_SET_NAME: // 9
                desc_type = jdksavdecc_aem_command_set_name_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                desc_index = jdksavdecc_aem_command_set_name_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
                DEBUG_INFO("LOGGING_LEVEL_ERROR:Need to implement SET_NAME command.");

                break;

            case JDKSAVDECC_AEM_COMMAND_GET_NAME: // 10
                desc_type = jdksavdecc_aem_command_get_name_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                desc_index = jdksavdecc_aem_command_get_name_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
                DEBUG_INFO("LOGGING_LEVEL_ERROR:Need to implement GET_NAME command.");

                break;

            case JDKSAVDECC_AEM_COMMAND_SET_SAMPLING_RATE:// 11 
                {
                    desc_type = jdksavdecc_aem_command_set_sampling_rate_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_set_sampling_rate_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_AUDIO_UNIT)
                    {
                       
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_GET_SAMPLING_RATE: // 12
                {
                    desc_type = jdksavdecc_aem_command_get_sampling_rate_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_get_sampling_rate_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_AUDIO_UNIT)
                    {
                     
                    }
                }
                break;
            
            case JDKSAVDECC_AEM_COMMAND_GET_COUNTERS: // 13
                {
                    desc_type = jdksavdecc_aem_command_get_counters_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_get_counters_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
                
                    if(desc_type == JDKSAVDECC_DESCRIPTOR_AVB_INTERFACE)
                    {
                
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_CLOCK_DOMAIN)
                    {
                        
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                    {
                       
                    }
                    else
                    {
                        DEBUG_INFO("LOGGING_LEVEL_ERROR:Descriptor type %d is not implemented.", desc_type);
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_SET_CLOCK_SOURCE: // 14
                {
                    desc_type = jdksavdecc_aem_command_set_clock_source_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_set_clock_source_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD); 
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_GET_CLOCK_SOURCE: // 15
                {
                    desc_type = jdksavdecc_aem_command_get_clock_source_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_get_clock_source_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_START_STREAMING: // 16
                {
                    desc_type = jdksavdecc_aem_command_start_streaming_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_start_streaming_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                    {
                       //proc_start_streaming_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                    {
                       //proc_start_streaming_resp(notification_id, frame, frame_len, status);
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_STOP_STREAMING: // 17
                {
                    desc_type = jdksavdecc_aem_command_stop_streaming_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_stop_streaming_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_INPUT)
                    {
                       //proc_stop_streaming_resp(notification_id, frame, frame_len, status);
                    }
                    else if(desc_type == JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT)
                    {
			//proc_stop_streaming_resp(notification_id, frame, frame_len, status);
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_REBOOT: // 18
                {
                    desc_type = jdksavdecc_aem_command_reboot_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_reboot_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_ENTITY)
                    {
                       //proc_reboot_resp(notification_id, frame, frame_len, status); 
                    }
                    else
                    {
                        DEBUG_INFO("LOGGING_LEVEL_ERROR:Descriptor type %d is not valid.", desc_type);
                    }
                }

                break;

            case JDKSAVDECC_AEM_COMMAND_START_OPERATION: // 19
                {
                    desc_type = jdksavdecc_aem_command_start_operation_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_start_operation_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_MEMORY_OBJECT)
                    {
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_OPERATION_STATUS: // 20
                {
                    desc_type = jdksavdecc_aem_command_operation_status_response_get_descriptor_type(frame, ZERO_OFFSET_IN_PAYLOAD);
                    desc_index = jdksavdecc_aem_command_operation_status_response_get_descriptor_index(frame, ZERO_OFFSET_IN_PAYLOAD);

                    if(desc_type == JDKSAVDECC_DESCRIPTOR_MEMORY_OBJECT)
                    {
                    }
                }
                break;

            case JDKSAVDECC_AEM_COMMAND_SET_CONTROL: // 21
              //  proc_set_control_resp(notification_id, frame, frame_len, status);
                break;

            default:
                DEBUG_INFO( "NO_MATCH_FOUND:%d, cmd_type(%d), %d,%d, %d, %d", 0, cmd_type, 0, 0, 0, 0);
                break;
        }
}

void proc_rcvd_aecp_aa_resp( const uint8_t *frame, size_t frame_len, int *status )
{
	
}

int proc_read_desc_resp( const uint8_t *frame, size_t frame_len, int *status)
{
        const int read_desc_offset = ZERO_OFFSET_IN_PAYLOAD+ JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR_RESPONSE_LEN;
        struct jdksavdecc_frame cmd_frame;
        struct jdksavdecc_aem_command_read_descriptor_response aem_cmd_read_desc_resp;
	struct jdksavdecc_descriptor_entity entity_desc; // Structure containing the entity_desc fields
        ssize_t aem_cmd_read_desc_resp_returned;
        uint32_t msg_type;
        bool u_field;
        uint16_t desc_type;
	struct jdksavdecc_eui64  _target_entity_id = jdksavdecc_common_control_header_get_stream_id( frame, 0 );
	uint64_t target_entity_id = convert_eui64_to_uint64_return(_target_entity_id.value);
		
	desc_pdblist  descptor_info = NULL;	
	bool found_descptor_endstation = false;
	descptor_info = search_desc_dblist_node(target_entity_id, descptor_guard );
	if( NULL != descptor_info )
	{
		found_descptor_endstation = true;
	}
	
        memset(&aem_cmd_read_desc_resp,0,sizeof(aem_cmd_read_desc_resp));
        memcpy(cmd_frame.payload, frame, frame_len);
        aem_cmd_read_desc_resp_returned = jdksavdecc_aem_command_read_descriptor_response_read(&aem_cmd_read_desc_resp,
                                                                                               frame,
                                                                                               ZERO_OFFSET_IN_PAYLOAD,
                                                                                               frame_len);
        if(aem_cmd_read_desc_resp_returned < 0)
        {
            DEBUG_INFO("LOGGING_LEVEL_ERROR:aem_cmd_read_desc_res_read error");
            return -1;
        }
        msg_type = aem_cmd_read_desc_resp.aem_header.aecpdu_header.header.message_type;
        *status = aem_cmd_read_desc_resp.aem_header.aecpdu_header.header.status;
        u_field = aem_cmd_read_desc_resp.aem_header.command_type >> 15 & 0x01; // u_field = the msb of the uint16_t command_type
        desc_type = jdksavdecc_uint16_get(frame, ZERO_OFFSET_IN_PAYLOAD+ JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR_RESPONSE_OFFSET_DESCRIPTOR);

	if( desc_type == JDKSAVDECC_DESCRIPTOR_ENTITY)
	{
		memset(&entity_desc, 0, sizeof(entity_desc));
		ssize_t desc_entity_read_returned = jdksavdecc_descriptor_entity_read( &entity_desc, frame, read_desc_offset, frame_len );
		if( desc_entity_read_returned < 0 )
		{
			DEBUG_INFO("LOGGING_LEVEL_ERROR:aem_cmd_read_desc_res_read error");
            		return -1;
		}
	}

       aecp_update_inflight_for_rcvd_resp( msg_type, u_field, &cmd_frame );

        bool store_descriptor = false;
        if( *status == AEM_STATUS_SUCCESS )
        {
            switch( desc_type )
            {
                case JDKSAVDECC_DESCRIPTOR_ENTITY:
                    store_descriptor = true;
                    break;

                case JDKSAVDECC_DESCRIPTOR_CONFIGURATION:
                    if( found_descptor_endstation && descptor_info->endpoint_desc.conf_desc.descriptor_counts_count == 0)
                    {
                        store_descriptor = true;
                    }
                    break;

                default:
                    if( found_descptor_endstation && descptor_info->endpoint_desc.conf_desc.descriptor_counts_count >= 1)
                    {
                        store_descriptor = true;
                    }
                    break;
            }
        }

        if (store_descriptor)
        {
        	switch (desc_type)
                {
                    case JDKSAVDECC_DESCRIPTOR_ENTITY:
                        if ( !found_descptor_endstation )	// create descptor double list node to store the descptor information
                        {
                        	descptor_info = create_descptor_dblist_node( &descptor_info );
				if( NULL == descptor_info )
				{
					DEBUG_INFO( "create new descptor endstation failed!system init descptor information failed, you must be attention for it!" );
					assert( NULL != descptor_info );
				}
				init_descptor_dblist_node_info( descptor_info );
				
				// init entity descptor information
				if( target_entity_id != 0)
				{
					descptor_info->endpoint_desc.entity_id = target_entity_id;
					descptor_info->endpoint_desc.entity_desc.configurations_count = entity_desc.configurations_count;
					descptor_info->endpoint_desc.entity_desc.current_configuration = entity_desc.current_configuration;
 					descptor_info->endpoint_desc.entity_desc.descriptor_type = entity_desc.descriptor_type;
					descptor_info->endpoint_desc.entity_desc.descriptor_index = entity_desc.descriptor_index;
					descptor_info->endpoint_desc.is_entity_desc_exist = true;
					memcpy(&descptor_info->endpoint_desc.entity_name,  &entity_desc.entity_name, sizeof(struct jdksavdecc_string ));
					memcpy(&descptor_info->endpoint_desc.firmware_version,  &entity_desc.firmware_version, sizeof(struct jdksavdecc_string ));
				}
				
				insert_descptor_dblist_trail( descptor_guard, descptor_info );
                        }
                        break;

                    case JDKSAVDECC_DESCRIPTOR_CONFIGURATION:
			if( found_descptor_endstation )
			{
                        	store_config_desc( frame, read_desc_offset, frame_len, descptor_info);
			}
                        break;

                    case JDKSAVDECC_DESCRIPTOR_AUDIO_UNIT:
                        //store_audio_unit_desc(this, frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_STREAM_INPUT:
			if( found_descptor_endstation/* && descptor_info->endpoint_desc.is_input_stream_desc_exist*/)
			{
                        	store_stream_input_desc( frame, read_desc_offset, frame_len, descptor_info );
				init_central_control_recieve_unit_by_entity_id( frame, read_desc_offset, frame_len, descptor_info, target_entity_id );
#if 0
				muticast_connector_connect_table_init_node( true, frame, read_desc_offset, frame_len, target_entity_id, descptor_info );
#else
				conference_recieve_model_init( frame, read_desc_offset, frame_len, descptor_info, target_entity_id );
#endif
			}
                        break;

                    case JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT:
			if( found_descptor_endstation /*&& !descptor_info->endpoint_desc.is_output_stream_desc_exist*/)
			{
                        	store_stream_output_desc( frame, read_desc_offset, frame_len, descptor_info );
				conference_transmit_unit_init( frame, read_desc_offset, frame_len, target_entity_id, descptor_info );
#if 0
				muticast_connector_connect_table_init_node( false, frame, read_desc_offset, frame_len, target_entity_id, descptor_info );
#else
				central_control_transmit_unit_init( frame, read_desc_offset, frame_len, descptor_info, target_entity_id );
#endif
			}
                        break;

                    case JDKSAVDECC_DESCRIPTOR_JACK_INPUT:
                        //store_jack_input_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_JACK_OUTPUT:
                        //store_jack_output_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_AVB_INTERFACE:
                        //store_avb_interface_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_CLOCK_SOURCE:
                        //store_clock_source_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_MEMORY_OBJECT:
                        //store_memory_object_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_LOCALE:
                        //store_locale_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_STRINGS:
                        //store_strings_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_STREAM_PORT_INPUT:
                        //store_stream_port_input_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_STREAM_PORT_OUTPUT:
                        //store_stream_port_output_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_AUDIO_CLUSTER:
                        //store_audio_cluster_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_AUDIO_MAP:
                        //store_audio_map_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_CLOCK_DOMAIN:
                        //store_clock_domain_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_CONTROL:
                        //store_control_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_EXTERNAL_PORT_INPUT:
                        //store_external_port_input_desc( frame, read_desc_offset, frame_len);
                        break;

                    case JDKSAVDECC_DESCRIPTOR_EXTERNAL_PORT_OUTPUT:
                        //store_external_port_output_desc( frame, read_desc_offset, frame_len);
                        break;

                    default:
                        DEBUG_INFO("LOGGING_LEVEL_ERROR:Descriptor %s is not yet implemented in avdecc-host.", aem_desc_value_to_name(desc_type));
                        break;
                }
		
		background_read_deduce_next( descptor_info, desc_type, (void *)frame, read_desc_offset); 
	}
	if ((found_descptor_endstation) && (descptor_info->endpoint_desc.is_configuration_desc_exist))
	{
		// all read descriptor completed, 这里可以优化，做法是如avdecc-lib的处理，这里暂时不模仿了
		//if (m_backbround_read_inflight.empty() && m_backbround_read_pending.empty())
		//{
		 //       DEBUG_ONINFO("[ END_STATION_READ_COMPLETED %016llx, %d, %d, %d, %d, NULL ]", end_station_entity_id, 0, 0, 0, 0, NULL);
		//}
	}

        return 0;
}

/** This function looks inside rx'd descriptors and deducees which descriptors need to be read in the background next.
 *  There are two lists that are maintained for reading descriptors. The m_backbround_read_pending list where descriptors
 *  are queued before being sent and the m_backbround_read_inflight list the contains read requests that are on "the wire".
 */
void background_read_deduce_next( desc_pdblist cd, uint16_t desc_type, void *frame, ssize_t read_desc_offset)
{
	uint16_t total_num_of_desc = 0;
	uint16_t desc_index;
	uint64_t target_id = cd->endpoint_desc.entity_id;
	
	bool have_index = desc_index_from_frame(desc_type, frame, read_desc_offset, &desc_index);
	if (!have_index)
		desc_index = 0;
	
	//DEBUG_INFO("[background_read_deduce_next target id: 0x%016llx (desc_tyep = %d), con_is_exist = %d ]", target_id, desc_type, cd->endpoint_desc.is_configuration_desc_exist);
	if ((desc_type != JDKSAVDECC_DESCRIPTOR_ENTITY) && (!cd->endpoint_desc.is_configuration_desc_exist))
	{
		DEBUG_INFO("LOGGING_LEVEL_ERROR:Invalid configuration_descriptor passed to background_read_deduce_next()");
		return;
	}

	switch (desc_type)
	{
		case JDKSAVDECC_DESCRIPTOR_ENTITY:
			if( target_id != 0)
				queue_background_read_request(target_id, JDKSAVDECC_DESCRIPTOR_CONFIGURATION, 0, 1);
			break;

		case JDKSAVDECC_DESCRIPTOR_CONFIGURATION:
			total_num_of_desc = cd->endpoint_desc.conf_desc.descriptor_counts_count;
			int j = 0;
			for ( j = 0; j < total_num_of_desc; j++)
			{
				if( target_id != 0)
					queue_background_read_request( target_id, cd->endpoint_desc.conf_desc.desc_type[j], 0, cd->endpoint_desc.conf_desc.desc_count[j]);
			}
			break;

		// 注:可以在后面继续添加其它的DESCRIPTOR type
		default:
			//DEBUG_INFO( "wrong descriptor type we background_read_deduce_next " );
			break;
	}
}

void queue_background_read_request(uint64_t target_id, uint16_t desc_type, uint16_t desc_base_index, uint16_t desc_count )
{
	int i = 0;
	for ( i = 0; i < desc_count; i++)
	{
		aecp_read_desc_init( desc_type, desc_base_index + i, target_id);
	}
}

bool desc_index_from_frame(uint16_t desc_type, void *frame, ssize_t read_desc_offset, uint16_t *desc_index)
{
    switch (desc_type)
    {
        case JDKSAVDECC_DESCRIPTOR_ENTITY:
            *desc_index = 0;
            break;

        case JDKSAVDECC_DESCRIPTOR_CONFIGURATION:
            *desc_index = jdksavdecc_descriptor_configuration_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_AUDIO_UNIT:
            *desc_index = jdksavdecc_descriptor_audio_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_STREAM_INPUT:
        case JDKSAVDECC_DESCRIPTOR_STREAM_OUTPUT:
            *desc_index = jdksavdecc_descriptor_stream_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_JACK_INPUT:
        case JDKSAVDECC_DESCRIPTOR_JACK_OUTPUT:
            *desc_index = jdksavdecc_descriptor_jack_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_AVB_INTERFACE:
            *desc_index = jdksavdecc_descriptor_avb_interface_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_CLOCK_SOURCE:
            *desc_index = jdksavdecc_descriptor_clock_source_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_MEMORY_OBJECT:
            *desc_index = jdksavdecc_descriptor_memory_object_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_LOCALE:
            *desc_index = jdksavdecc_descriptor_locale_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_STRINGS:
            *desc_index = jdksavdecc_descriptor_strings_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_STREAM_PORT_INPUT:
        case JDKSAVDECC_DESCRIPTOR_STREAM_PORT_OUTPUT:
            *desc_index = jdksavdecc_descriptor_stream_port_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_EXTERNAL_PORT_INPUT:
        case JDKSAVDECC_DESCRIPTOR_EXTERNAL_PORT_OUTPUT:
            *desc_index = jdksavdecc_descriptor_external_port_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_AUDIO_CLUSTER:
            *desc_index = jdksavdecc_descriptor_audio_cluster_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_AUDIO_MAP:
            *desc_index = jdksavdecc_descriptor_audio_map_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_CLOCK_DOMAIN:
            *desc_index = jdksavdecc_descriptor_clock_domain_get_descriptor_index(frame, read_desc_offset);
            break;

        case JDKSAVDECC_DESCRIPTOR_CONTROL:
            *desc_index = jdksavdecc_descriptor_control_get_descriptor_index(frame, read_desc_offset);
            break;

        default:
            return false;
    }
    return true;
}


