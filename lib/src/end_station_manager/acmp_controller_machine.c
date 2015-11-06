#include "acmp_controller_machine.h"
#include "acmp.h"
#include "stream_descriptor.h"
#include "endstation_connection.h"

static struct jdksavdecc_frame acmp_frame;
static struct jdksavdecc_acmpdu acmpdu;
static uint32_t acmp_sequence_id = 0;
static inflight_plist acmp_inflight_guard = NULL;
static solid_pdblist acmp_solid_guard = NULL;
static desc_pdblist acmp_desc_guard = NULL;
struct background_inflight_cmd acmp_connect_state_update;
ttcnn_table_call connet_table_disconnect_call_info;// 断开回调信息
ttcnn_table_call connet_table_connect_call_info; // 连接回调信息

void acmp_endstation_init( inflight_plist guard, solid_pdblist head, desc_pdblist desc_guard )
{
	assert( guard && head && desc_guard );
	acmp_inflight_guard = guard;
	acmp_solid_guard = head;
	acmp_desc_guard = desc_guard;
	acmp_connect_state_update.background_inflight_cmd_type = INPUT_OUTPUT_STREAM_DESCRIPTOR;
	background_inflight_timer_start( (uint32_t)2*1500, &acmp_connect_state_update);// send the comand 2s later
}


void acmp_frame_init( void )
{
        jdksavdecc_frame_init( &acmp_frame );
        memcpy( acmp_frame.src_address.value, net.m_my_mac, 6 );
}


void acmp_frame_init_2( struct jdksavdecc_frame *frame )
{
        jdksavdecc_frame_init( frame );
        memcpy( frame->src_address.value, net.m_my_mac, 6 );
}

void acmp_disconnect_avail( uint8_t output_id[8], uint16_t talker_unique_id, uint8_t input_id[8] , uint16_t listener_unique_id, uint16_t connection_count, uint16_t sequence_id)
{
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	struct jdksavdecc_frame *frame = &acmp_frame;
	
	acmp_frame_init();
	memcpy( talker_entity_id.value, output_id, 8 );
	memcpy( listener_entity_id.value, input_id, 8 );
	acmp_form_msg( &acmp_frame, &acmpdu,  JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND, \
		sequence_id,  talker_entity_id, talker_unique_id, listener_entity_id, listener_unique_id,\
		connection_count );

	system_raw_packet_tx( frame->dest_address.value, frame->payload, frame->length, RUNINFLIGHT, TRANSMIT_TYPE_ACMP, false);
}

void acmp_connect_avail(  uint8_t output_id[8], uint16_t talker_unique_id, uint8_t input_id[8] , uint16_t listener_unique_id, uint16_t connection_count, uint16_t sequence_id )
{
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	struct jdksavdecc_frame *frame = &acmp_frame;
	
	acmp_frame_init();
	memcpy( talker_entity_id.value, output_id, 8 );
	memcpy( listener_entity_id.value, input_id, 8 );
	acmp_form_msg( &acmp_frame, &acmpdu,  JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND, \
		sequence_id,  talker_entity_id, talker_unique_id, listener_entity_id, listener_unique_id,\
		connection_count );

	system_raw_packet_tx( frame->dest_address.value, frame->payload, frame->length, RUNINFLIGHT, TRANSMIT_TYPE_ACMP, false);
}

void acmp_rx_state_avail( uint64_t listener_entity_id, uint16_t listener_unique_id )
{
    struct jdksavdecc_acmpdu acmp_cmd_get_rx_state;
	struct jdksavdecc_frame frame;
	acmp_frame_init_2( &frame );
	
    jdksavdecc_eui64_init(&acmp_cmd_get_rx_state.talker_entity_id);
    jdksavdecc_uint64_write(listener_entity_id, &acmp_cmd_get_rx_state.listener_entity_id, 0, sizeof(uint64_t));
    acmp_cmd_get_rx_state.listener_unique_id = listener_unique_id;
    jdksavdecc_eui48_init(&acmp_cmd_get_rx_state.stream_dest_mac);

	acmp_form_msg( &frame, &acmp_cmd_get_rx_state,  JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_COMMAND, \
		acmp_sequence_id++,  acmp_cmd_get_rx_state.talker_entity_id, 0, acmp_cmd_get_rx_state.listener_entity_id, listener_unique_id,\
		0 );
	system_raw_packet_tx( frame.dest_address.value, frame.payload, frame.length, RUNINFLIGHT, TRANSMIT_TYPE_ACMP, false);

}

void acmp_tx_state_avail( uint64_t tarker_entity_id, uint16_t tarker_unique_id )
{
	struct jdksavdecc_acmpdu acmp_cmd_get_tx_state;
	struct jdksavdecc_frame frame;
	acmp_frame_init_2( &frame );
	
        jdksavdecc_eui64_init( &acmp_cmd_get_tx_state.listener_entity_id );
        jdksavdecc_uint64_write( tarker_entity_id, &acmp_cmd_get_tx_state.talker_entity_id, 0, sizeof(uint64_t) );
        acmp_cmd_get_tx_state.talker_unique_id = tarker_unique_id;
        jdksavdecc_eui48_init(&acmp_cmd_get_tx_state.stream_dest_mac);

	acmp_form_msg( &frame, &acmp_cmd_get_tx_state,  JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_COMMAND, \
		acmp_sequence_id++,  acmp_cmd_get_tx_state.talker_entity_id, tarker_unique_id, acmp_cmd_get_tx_state.listener_entity_id, 0,\
		0 );
	system_raw_packet_tx( frame.dest_address.value, frame.payload, frame.length, RUNINFLIGHT, TRANSMIT_TYPE_ACMP, false);
}

// update system's endstation connections status by sending rx_state and tx_state command to the network
int acmp_update_endstation_connections_networks( void )
{
	desc_pdblist desc_in_out_put = acmp_desc_guard;
	if( desc_in_out_put->next == desc_in_out_put )
	{
		DEBUG_INFO( "there is no descriptor list node : nothing to send!" );
		return -1;
	}

	for( desc_in_out_put = desc_in_out_put->next; desc_in_out_put != acmp_desc_guard; desc_in_out_put = desc_in_out_put->next )
	{
		int i = 0;
		if( (!desc_in_out_put->endpoint_desc.is_output_stream_desc_exist) && (!desc_in_out_put->endpoint_desc.is_input_stream_desc_exist) )
		{
			DEBUG_INFO( "system endstation has no output or input stream desc!you maybe send desc command to read output_stream or input_stream descriptor before completing read DESCRIPTOR_ENTITY" );
			continue;
		}

		uint8_t entity_input_stream_num = desc_in_out_put->endpoint_desc.input_stream.num;
		for( i = 0; i < entity_input_stream_num; i++ )
		{
			acmp_rx_state_avail( desc_in_out_put->endpoint_desc.entity_id, desc_in_out_put->endpoint_desc.input_stream.desc[i].descriptor_index);
		}

		uint8_t entity_output_stream_num = desc_in_out_put->endpoint_desc.output_stream.num;
		for( i = 0; i < entity_output_stream_num; i++ )
		{
			acmp_tx_state_avail(desc_in_out_put->endpoint_desc.entity_id, desc_in_out_put->endpoint_desc.output_stream.desc[i].descriptor_index );
		}
	}

	return 0;
}

void acmp_update_input_stream_descriptor( uint16_t desc_index, struct jdksavdecc_acmpdu *acmpdu_get_rx_state_resp )
{
	uint64_t uint64_stream_id = convert_eui64_to_uint64_return( acmpdu_get_rx_state_resp->header.stream_id.value );
	uint64_t uint64_listener_entity_id = convert_eui64_to_uint64_return( acmpdu_get_rx_state_resp->listener_entity_id.value );
	
	desc_pdblist input_desc = search_desc_dblist_node( uint64_listener_entity_id, acmp_desc_guard);
	if( input_desc == NULL )
	{
		DEBUG_INFO( "no such output stream tarker entity ID");
		return;
	}

	// get the importance connect count and stream id
	input_desc->endpoint_desc.input_stream.desc[desc_index].connect_num = acmpdu_get_rx_state_resp->connection_count;
	input_desc->endpoint_desc.input_stream.desc[desc_index].stream_id = uint64_stream_id;
}


void acmp_update_output_stream_descriptor( uint16_t desc_index, struct jdksavdecc_acmpdu *acmpdu_get_rx_state_resp )
{
	uint64_t uint64_stream_id = convert_eui64_to_uint64_return( acmpdu_get_rx_state_resp->header.stream_id.value );
	uint64_t uint64_tarker_entity_id = convert_eui64_to_uint64_return( acmpdu_get_rx_state_resp->talker_entity_id.value );
	
	desc_pdblist input_desc = search_desc_dblist_node( uint64_tarker_entity_id, acmp_desc_guard);
	if( input_desc == NULL )
	{
		DEBUG_INFO( "no such output stream tarker entity ID");
		return;
	}

	// get the importance connect count and stream id
	input_desc->endpoint_desc.output_stream.desc[desc_index].connect_num = acmpdu_get_rx_state_resp->connection_count;
	input_desc->endpoint_desc.output_stream.desc[desc_index].stream_id = uint64_stream_id;
}

// background send to rx_state and tx_state to network
void acmp_binflight_cmd_time_tick( void )
{
	if( background_inflight_timeout( &acmp_connect_state_update) )
	{
		background_read_descriptor_input_output_stream();
		background_inflight_timer_start( (uint32_t)2*2000, &acmp_connect_state_update);// 1s to update after system start
	}
}


ssize_t transmit_acmp_packet_network( uint8_t* frame, uint16_t frame_len, inflight_plist guard, bool resend ,const uint8_t dest_mac[6], bool resp )
{
	uint8_t sub_type = jdksavdecc_subtype_data_get_subtype(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t timeout = get_acmp_timeout( msg_type );
	inflight_plist inflight_station = NULL;

	if(!resp )// not a response data
	{
		if( !resend )// data first send
		{
			inflight_station = create_inflight_dblist_new_node( &inflight_station );
			if( NULL == inflight_station )
			{
				DEBUG_INFO("inflight station node create failed!");
				return -1;
			}
			memset(inflight_station, 0, sizeof(inflight_list));
			
			inflight_station->host_tx.inflight_frame.frame = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &inflight_station->host_tx.inflight_frame.frame );
			if( NULL != inflight_station->host_tx.inflight_frame.frame )
			{
				memcpy( inflight_station->host_tx.inflight_frame.frame, frame, frame_len);
				inflight_station->host_tx.inflight_frame.inflight_frame_len = frame_len;
				inflight_station->host_tx.inflight_frame.data_type = sub_type; //协议
				inflight_station->host_tx.inflight_frame.seq_id = seq_id;
				memcpy(&inflight_station->host_tx.inflight_frame.raw_dest, dest_mac , 6 );
				if( (msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE ) || ( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE ))		
					inflight_station->host_tx.inflight_frame.notification_flag = CMD_WITHOUT_NOTIFICATION;
				else
					inflight_station->host_tx.inflight_frame.notification_flag = CMD_WITH_NOTIFICATION;
				
				inflight_station->host_tx.command_type = TRANSMIT_TYPE_ACMP;
				inflight_station->host_tx.flags.retried = 1;	// meaning send once
				inflight_station->host_tx.flags.resend = false;
				inflight_timer_start(timeout, inflight_station );

				// 将新建的inflight命令结点插入链表结尾中
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
			inflight_station = search_node_inflight_from_dblist( guard, seq_id, sub_type);
			if( inflight_station != NULL ) //already search it
			{
				inflight_station->host_tx.flags.resend = true;
				inflight_station->host_tx.flags.retried++ ;
				inflight_timer_state_avail( timeout, inflight_station );
			}
			else
			{
				DEBUG_INFO( "nothing to be resend!" );
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

void acmp_inflight_station_timeouts( inflight_plist  acmp_sta, inflight_plist hdr )
{
	bool is_retried = false;
	inflight_plist acmp_pstation = NULL;
	uint8_t *frame = NULL;
	uint16_t frame_len = 0;
         
	if( acmp_sta != NULL )
	{
		acmp_pstation = acmp_sta;
		is_retried = is_inflight_cmds_retried( acmp_pstation );
		frame = acmp_pstation->host_tx.inflight_frame.frame;
		frame_len = acmp_pstation->host_tx.inflight_frame.inflight_frame_len;
	}
	else 
	{
		DEBUG_INFO( "noting to be proccessed by aecp timeout" );
		return;
	}

	assert( frame && acmp_sta != NULL );
	if( is_retried )
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_listener_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
        	uint64_t end_station_entity_id = jdksavdecc_uint64_get( &_end_station_entity_id, 0 );
        	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data( frame, 0 );
			
		DEBUG_INFO( " [ COMMAND TIMEOUT: 0x%016llx, %s, %s,%d ]", 
					end_station_entity_id,
					acmp_cmd_value_to_name(msg_type),
					"NULL",
					 acmp_pstation->host_tx.inflight_frame.seq_id);

		// free inflight command node in the system
		release_heap_space( &acmp_pstation->host_tx.inflight_frame.frame);
		delect_inflight_dblist_node( &acmp_pstation );

		is_inflight_timeout = true; // 设置超时
	}
	else
	{
		DEBUG_INFO( "acmp resended " );
		transmit_acmp_packet_network( frame, frame_len, hdr, true, acmp_pstation->host_tx.inflight_frame.raw_dest.value, false);
		is_inflight_timeout = false; // 不是超时
	}
}

int acmp_proc_state_resp( struct jdksavdecc_frame *cmd_frame )
{
	uint8_t subtype = jdksavdecc_common_control_header_get_subtype( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD );
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t notification_flag = 0;
	inflight_plist inflight_est = NULL;

	inflight_est = search_node_inflight_from_dblist( acmp_inflight_guard, seq_id, subtype );	// found?
	if( NULL != inflight_est )
	{
		notification_flag = inflight_est->host_tx.inflight_frame.notification_flag;
		acmp_callback( notification_flag, cmd_frame->payload);
		release_heap_space( &inflight_est->host_tx.inflight_frame.frame);// it must delect
		delect_inflight_dblist_node( &inflight_est );	// delect acmp inflight node must delect date frame
	}
	else
	{
		DEBUG_INFO( "no such inflight cmd acmp node!");
		return -1;
	}
	
	return 0;
}

int acmp_callback(  uint32_t notification_flag, uint8_t *frame)
{
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t status = jdksavdecc_common_control_header_get_status(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint64_t end_station_entity_id;

	if((notification_flag == CMD_WITH_NOTIFICATION) &&
	((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE)))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_talker_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);

		DEBUG_ONINFO( " [ RESPONSE_RECEIVED: %d 0x%016llx (talker), %d, %d, %d, %s ]",
						RESPONSE_RECEIVED,
						end_station_entity_id,
						(uint16_t)msg_type + CMD_LOOKUP, 
						0, 
						0, 
						acmp_cmd_status_value_to_name(status));

		if(status != ACMP_STATUS_SUCCESS)
		{
			DEBUG_INFO( "LOGGING_LEVEL_ERROR: RESPONSE_RECEIVED, 0x%016llx (talker), %s, %s, %s, %s, %d",
						end_station_entity_id,
						acmp_cmd_value_to_name(msg_type),
						"NULL",
						"NULL", 
						acmp_cmd_status_value_to_name(status),
						seq_id);
		}
	}
	else if((notification_flag == CMD_WITH_NOTIFICATION) &&
	((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE)))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_listener_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
		struct jdksavdecc_eui64 _tarker_id = jdksavdecc_acmpdu_get_talker_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
		uint64_t tarker_id = jdksavdecc_uint64_get( &_tarker_id, 0 );
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);
		DEBUG_ONINFO( " [ RESPONSE_RECEIVED: %d 0x%016llx (listener)-0x%016llx(tarker), %d, %d, %d, %s ]",
						RESPONSE_RECEIVED,
						end_station_entity_id,
						tarker_id,
						(uint16_t)msg_type + CMD_LOOKUP, 
						0, 
						0, 
						acmp_cmd_status_value_to_name(status));

		if( (status == ACMP_STATUS_SUCCESS) &&\
			((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE) || \
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE)))// udpate system descriptor connect list 
		{
			if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE )
			{
				assert( connet_table_connect_call_info.p_cnnt_node && connet_table_connect_call_info.pc_callback );
				connet_table_connect_call_info.pc_callback( connet_table_disconnect_call_info.p_cnnt_node,\
					connet_table_disconnect_call_info.limit_speak_time, connet_table_disconnect_call_info.limit_speak_time?true:false,\
					connet_table_disconnect_call_info.tarker_id );
			}
			else if( msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE )
			{
				assert( connet_table_disconnect_call_info.p_cnnt_node && connet_table_disconnect_call_info.pdis_callback );
				connet_table_disconnect_call_info.pdis_callback( connet_table_disconnect_call_info.p_cnnt_node );
			}
			
			acmp_update_endstation_connections_networks();
		}
		if( status != ACMP_STATUS_SUCCESS )
		{
			DEBUG_INFO("LOGGING_LEVEL_ERROR: RESPONSE_RECEIVED, 0x%016llx (listener), %s, %s, %s, %s, %d",
							end_station_entity_id,
							acmp_cmd_value_to_name(msg_type),
							"NULL",
							"NULL", 
							acmp_cmd_status_value_to_name(status),
							seq_id);
		}
	}
	else if((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_talker_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);
		DEBUG_ONINFO("LOGGING_LEVEL_DEBUG: RESPONSE_RECEIVED, 0x%016llx (talker), %s, %s, %s, %s, %d",
								end_station_entity_id,
								acmp_cmd_value_to_name(msg_type),
								"NULL",
								"NULL",  
								acmp_cmd_status_value_to_name(status),
								seq_id);
	}
	else
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_listener_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);
		DEBUG_INFO( "LOGGING_LEVEL_DEBUG:COMMAND_SENT, 0x%016llx listener), %s, %s, %s, %s, %d",
								end_station_entity_id,
								acmp_cmd_value_to_name(msg_type),
								"NULL",
								"NULL",
								acmp_cmd_status_value_to_name(status),
								seq_id);
	}

	return 0;
}

/**********************************************
*Date:2015-11-6
*功能:
*	用于连接表连接麦克风，回调在接收响应的acmp响应数据调用
*
***********************************************/
int acmp_disconnect_connect_table( uint8_t tarker_value[8], 
									uint16_t tarker_index, 
									uint8_t listener_value[8], 
									uint16_t listener_index, 
									uint16_t cnnt_count, 
									uint16_t sequence_id, 
									ttcnn_table_call *discnnt_callback_save , 
									int (*disconnect_callback_func)( connect_tbl_pdblist p_cnnt_node ) )
{
	assert( disconnect_callback_func && discnnt_callback_save );
	if( discnnt_callback_save == NULL )
	{
		return -1;
	}
	else
	{
		connet_table_disconnect_call_info.limit_speak_time = discnnt_callback_save->limit_speak_time;
		connet_table_disconnect_call_info.p_cnnt_node = discnnt_callback_save->p_cnnt_node;
		connet_table_disconnect_call_info.tarker_id = discnnt_callback_save->tarker_id;
		connet_table_disconnect_call_info.pdis_callback = disconnect_callback_func;
		connet_table_disconnect_call_info.pc_callback = NULL;
		acmp_disconnect_avail( tarker_value, tarker_index, listener_value, listener_index, cnnt_count, sequence_id );
	}

	return 0;
}

/**********************************************
*Date:2015-11-6
*功能:
*	用于连接表断开麦克风，回调在接收响应的acmp响应数据调用
*
***********************************************/
int acmp_connect_connect_table( uint8_t tarker_value[8], 
									uint16_t tarker_index, 
									uint8_t listener_value[8], 
									uint16_t listener_index, 
									uint16_t cnnt_count, 
									uint16_t sequence_id, 
									ttcnn_table_call *cnnt_callback_save , 
									int (*connect_callback)( connect_tbl_pdblist p_cnnt_node, uint32_t timeouts, bool is_limit_time, uint64_t utarker_id ) )
{
	assert( connect_callback && cnnt_callback_save );
	if( cnnt_callback_save == NULL )
	{
		return -1;
	}
	else
	{
		connet_table_connect_call_info.limit_speak_time = cnnt_callback_save->limit_speak_time;
		connet_table_connect_call_info.p_cnnt_node = cnnt_callback_save->p_cnnt_node;
		connet_table_connect_call_info.tarker_id = cnnt_callback_save->tarker_id;
		connet_table_connect_call_info.pc_callback = connect_callback;
		connet_table_connect_call_info.pdis_callback = NULL;
		acmp_connect_avail( tarker_value, tarker_index, listener_value, listener_index, cnnt_count, sequence_id );
	}
	
	return 0;
}


