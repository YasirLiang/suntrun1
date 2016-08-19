#include "acmp_controller_machine.h"
#include "acmp.h"
#include "stream_descriptor.h"
#include "connector_subject.h"
#include "send_common.h" // 包含SEND_DOUBLE_QUEUE_EABLE
#include "log_machine.h"
#include "controller_machine.h"

#ifdef __DEBUG__
#define __ACMP_MACHINE_DEBUG__
#endif

#ifdef __ACMP_MACHINE_DEBUG__
#define acmp_machine_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define acmp_machine_debug(fmt, args...)
#endif

static struct jdksavdecc_frame acmp_frame;
static uint32_t acmp_sequence_id = 0;
static inflight_plist acmp_inflight_guard = NULL;
static solid_pdblist acmp_solid_guard = NULL;
static desc_pdblist acmp_desc_guard = NULL;
struct background_inflight_cmd acmp_connect_state_update;

bool acmp_recv_resp_err = false; // acmp 接收到命令但响应错误参数

void acmp_endstation_init( inflight_plist guard, solid_pdblist head, desc_pdblist desc_guard )
{
	assert( guard && head && desc_guard );
	if( guard == NULL || head == NULL || desc_guard == NULL )
		return;
	
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
	struct jdksavdecc_acmpdu acmpdu_e;
	
	acmp_frame_init();
	memcpy( talker_entity_id.value, output_id, 8 );
	memcpy( listener_entity_id.value, input_id, 8 );

	memset(&acmpdu_e, 0, sizeof(acmpdu_e));
	acmpdu_e.flags = 0;
	acmpdu_e.stream_vlan_id = 0;
	acmp_form_msg( &acmp_frame, &acmpdu_e,  JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_COMMAND, \
		sequence_id,  talker_entity_id, talker_unique_id, listener_entity_id, listener_unique_id,\
		0 );

	system_raw_packet_tx( frame->dest_address.value, frame->payload, frame->length, RUNINFLIGHT, TRANSMIT_TYPE_ACMP, false);
}

void acmp_connect_avail(  uint8_t output_id[8], uint16_t talker_unique_id, uint8_t input_id[8] , uint16_t listener_unique_id, uint16_t connection_count, uint16_t sequence_id )
{
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	struct jdksavdecc_frame *frame = &acmp_frame;
	struct jdksavdecc_acmpdu acmpdu_e;

	acmp_frame_init();
	memcpy( talker_entity_id.value, output_id, 8 );
	memcpy( listener_entity_id.value, input_id, 8 );

	memset(&acmpdu_e, 0, sizeof(acmpdu_e));
	acmpdu_e.flags = 0;
    	acmpdu_e.flags |= (uint16_t)0x2;// fast connect flags
    	acmpdu_e.stream_vlan_id = 0;
	acmp_form_msg( &acmp_frame, &acmpdu_e,  JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND, \
		sequence_id,  talker_entity_id, talker_unique_id, listener_entity_id, listener_unique_id,\
		0);
	
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
		acmp_machine_debug( "there is no descriptor list node : nothing to send!" );
		return -1;
	}

	for( desc_in_out_put = desc_in_out_put->next; desc_in_out_put != acmp_desc_guard; desc_in_out_put = desc_in_out_put->next )
	{
		int i = 0;
		if( (!desc_in_out_put->endpoint_desc.is_output_stream_desc_exist) && (!desc_in_out_put->endpoint_desc.is_input_stream_desc_exist) )
		{
			acmp_machine_debug( "system endstation has no output or input stream desc!you maybe send desc command to read output_stream or input_stream descriptor before completing read DESCRIPTOR_ENTITY" );
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
		acmp_machine_debug( "no such output stream tarker entity ID");
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
		acmp_machine_debug( "no such output stream tarker entity ID");
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
		background_inflight_timer_start( (uint32_t)15*100, &acmp_connect_state_update);// 1s to update after system start
	}
}

ssize_t transmit_acmp_packet_network( uint8_t* frame, uint16_t frame_len, inflight_plist resend_node, bool resend ,const uint8_t dest_mac[6], bool resp, uint32_t *interval_time )
{
	uint8_t sub_type = jdksavdecc_subtype_data_get_subtype(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD);
	inflight_plist inflight_station = NULL;
	uint32_t timeout = get_acmp_timeout( msg_type );
	assert( interval_time );
	*interval_time = timeout;

	if( (frame_len > TRANSMIT_DATA_BUFFER_SIZE) || (frame_len <= 0) )
	{
		acmp_machine_debug( "udp packet( size = %d )bigger than frame buf %d or little!",
			frame_len,TRANSMIT_DATA_BUFFER_SIZE );
		return -1;
	}

	if(!resp )// not a response data
	{
		if( !resend )// data first send
		{
			inflight_station = create_inflight_dblist_new_node( &inflight_station );
			if( NULL == inflight_station )
			{
				acmp_machine_debug("inflight station node create failed!");
				return -1;
			}
			memset(inflight_station, 0, sizeof(inflight_list));
			
			inflight_station->host_tx.inflight_frame.frame = allot_heap_space( frame_len, &inflight_station->host_tx.inflight_frame.frame );
			if( NULL != inflight_station->host_tx.inflight_frame.frame )
			{
				memset(inflight_station->host_tx.inflight_frame.frame, 0, frame_len );
				inflight_station->host_tx.inflight_frame.inflight_frame_len = frame_len;
				memcpy( inflight_station->host_tx.inflight_frame.frame, frame, frame_len);
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
				if( acmp_inflight_guard != NULL )
					insert_inflight_dblist_trail( acmp_inflight_guard, inflight_station );
			}
			else
			{
				acmp_machine_debug("Err frame malloc !");
				assert( NULL != inflight_station->host_tx.inflight_frame.frame );
				if( NULL == inflight_station->host_tx.inflight_frame.frame )
					return -1;
			}
		}
		else
		{
			acmp_machine_debug( " acmp seq id = %d, subtype = %d", seq_id, sub_type );
			if( resend_node != NULL ) //already search it
			{
				resend_node->host_tx.flags.resend = true;
				resend_node->host_tx.flags.retried++ ;
				inflight_timer_state_avail( timeout, resend_node );
			}
			else
			{
				acmp_machine_debug( "nothing to be resend!" );
				assert(resend_node != NULL);
				if( resend_node == NULL )
					return -1;
			}
		}
	}
	
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

void acmp_inflight_station_timeouts( inflight_plist  acmp_sta, inflight_plist hdr )
{
	bool is_retried = false;
	inflight_plist acmp_pstation = NULL;
	uint8_t *frame = NULL;
	uint16_t frame_len = 0;
	uint32_t interval_time = 0;

	if( acmp_sta != NULL )
	{
		acmp_pstation = acmp_sta;
		is_retried = is_inflight_cmds_retried( acmp_pstation );
		frame = acmp_pstation->host_tx.inflight_frame.frame;
		frame_len = acmp_pstation->host_tx.inflight_frame.inflight_frame_len;
	}
	else 
	{
		acmp_machine_debug( "noting to be proccessed by aecp timeout" );
		return;
	}

	assert( frame && acmp_sta != NULL );
	if( is_retried )
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_listener_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
		struct jdksavdecc_eui64 _end_station_tarker_id = jdksavdecc_acmpdu_get_talker_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
        	uint64_t end_station_entity_id = jdksavdecc_uint64_get( &_end_station_entity_id, 0 );
		uint64_t tarker_id = jdksavdecc_uint64_get( &_end_station_tarker_id, 0 );
        	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data( frame, 0 );

		if (NULL != gp_log_imp)
				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
				        LOGGING_LEVEL_ERROR, " [ COMMAND TIMEOUT: 0x%016llx, %s, %s,%d ]",
					end_station_entity_id,
					acmp_cmd_value_to_name(msg_type),
					"NULL",
					 acmp_pstation->host_tx.inflight_frame.seq_id);

		struct jdksavdecc_acmpdu node_acmpdu;
		jdksavdecc_acmpdu_read( &node_acmpdu, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len );
		subject_data_elem elem;
		
		elem.connect_flag = false;
		elem.listener_id = end_station_entity_id;
		elem.listener_index = node_acmpdu.listener_unique_id;
		elem.tarker_id = tarker_id;
		elem.tarker_index = node_acmpdu.talker_unique_id;
		elem.ctrl_msg.data_type = JDKSAVDECC_SUBTYPE_ACMP;
		elem.ctrl_msg.msg_type = msg_type;
		elem.ctrl_msg.msg_resp_status = -1;// -1 means timeout status.
		elem.data_frame = NULL;// set data frame
		elem.data_frame_len = 0;
		set_subject_data( elem, &gconnector_subjector );
		notify_observer( &gconnector_subjector );
		
		// free inflight command node in the system
		release_heap_space( &acmp_pstation->host_tx.inflight_frame.frame);
		delect_inflight_dblist_node( &acmp_pstation );

		is_inflight_timeout = true; // 设置超时
		acmp_machine_debug( "is_inflight_timeout = %d", is_inflight_timeout );
	}
	else
	{
		if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
			                        LOGGING_LEVEL_DEBUG,  
			                        "acmp Date resend" );
		transmit_acmp_packet_network( frame, frame_len, acmp_pstation, true, acmp_pstation->host_tx.inflight_frame.raw_dest.value, false, &interval_time );
	}
}

int acmp_proc_state_resp( struct jdksavdecc_frame *cmd_frame )
{
	uint8_t subtype = jdksavdecc_common_control_header_get_subtype( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD );
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id( cmd_frame->payload, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t notification_flag = 0;
	inflight_plist inflight_est = NULL;

	if( acmp_inflight_guard == NULL )
		return -1;

	inflight_est = search_node_inflight_from_dblist( acmp_inflight_guard, seq_id, subtype );	// found?
	if( NULL != inflight_est )
	{

		notification_flag = inflight_est->host_tx.inflight_frame.notification_flag;
		acmp_callback( notification_flag, cmd_frame->payload, cmd_frame->length );
		release_heap_space( &inflight_est->host_tx.inflight_frame.frame );// it must delect
		delect_inflight_dblist_node( &inflight_est );	// delect acmp inflight node must delect date frame
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int acmp_callback(  uint32_t notification_flag, uint8_t *frame, uint16_t frame_len )
{
	uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint16_t seq_id = jdksavdecc_acmpdu_get_sequence_id(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint32_t status = jdksavdecc_common_control_header_get_status(frame, ZERO_OFFSET_IN_PAYLOAD);
	uint64_t end_station_entity_id;
	struct jdksavdecc_eui64 stream_entity_station = jdksavdecc_common_control_header_get_stream_id( frame, ZERO_OFFSET_IN_PAYLOAD );
	uint64_t end_stream_id = jdksavdecc_uint64_get(&stream_entity_station, 0);
	subject_data_elem elem;

	// acmp info
	struct jdksavdecc_acmpdu node_acmpdu;
	jdksavdecc_acmpdu_read( &node_acmpdu, frame, ZERO_OFFSET_IN_PAYLOAD, frame_len );
	
	if((notification_flag == CMD_WITH_NOTIFICATION) &&
	((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE)))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_talker_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		struct jdksavdecc_eui64 listener_entity = jdksavdecc_acmpdu_get_listener_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);
		uint64_t listener_id = jdksavdecc_uint64_get(&listener_entity, 0);

                if (status == ACMP_STATUS_SUCCESS)
                {
                        if (NULL != gp_log_imp)
			        gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
			                        LOGGING_LEVEL_DEBUG, 
			                        " [ RESPONSE_RECEIVED: %d 0x%016llx (talker), %d, %d, %d, %s ]",
						RESPONSE_RECEIVED,
						end_station_entity_id,
						(uint16_t)msg_type + CMD_LOOKUP, 
						0, 
						0, 
						acmp_cmd_status_value_to_name(status));
                }
                else
		{
			if (NULL != gp_log_imp)
				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
				                LOGGING_LEVEL_ERROR, 
				                "[ RESPONSE_RECEIVED, 0x%016llx (talker), %s, %s, %s, %s, %d ]",
						end_station_entity_id,
						acmp_cmd_value_to_name(msg_type),
						"NULL",
						"NULL", 
						acmp_cmd_status_value_to_name(status),
						seq_id );
			
			acmp_recv_resp_err = true;
		}
		
		// add notification of get_tx_state command to obserber
		if (msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE)
		{
			subject_data_elem elem;
			elem.listener_id = listener_id;
			elem.listener_index = node_acmpdu.listener_unique_id;
			elem.tarker_id = end_stream_id;
			elem.tarker_index = node_acmpdu.talker_unique_id;
			elem.ctrl_msg.data_type = JDKSAVDECC_SUBTYPE_ACMP;
			elem.ctrl_msg.msg_type = msg_type;
			elem.ctrl_msg.msg_resp_status = status;// -1 means timeout status.
			elem.data_frame = NULL;// set data frame
			elem.data_frame_len = 0;
			set_subject_data( elem, &gconnector_subjector );
			notify_observer( &gconnector_subjector );
		}
	}
	else if((notification_flag == CMD_WITH_NOTIFICATION) &&
	((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE)))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_listener_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
		struct jdksavdecc_eui64 _tarker_id = jdksavdecc_acmpdu_get_talker_entity_id( frame, ZERO_OFFSET_IN_PAYLOAD );
		uint64_t tarker_id = jdksavdecc_uint64_get( &_tarker_id, 0 );
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);

		if( (status == ACMP_STATUS_SUCCESS) &&\
			((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE) || \
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE)))// udpate system descriptor connect list 
		{
		      if (NULL != gp_log_imp)
				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
				                LOGGING_LEVEL_DEBUG,
				                " [ RESPONSE_RECEIVED: %d 0x%016llx (listener)-0x%016llx(tarker), %d, %d, %d, %s ]",
						RESPONSE_RECEIVED,
						end_station_entity_id,
						tarker_id,
						(uint16_t)msg_type + CMD_LOOKUP, 
						0, 
						0, 
						acmp_cmd_status_value_to_name(status));

			elem.connect_flag = (msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE)?true:false;
			elem.listener_id = end_station_entity_id;
			elem.listener_index = node_acmpdu.listener_unique_id;
			elem.tarker_id = tarker_id;
			elem.tarker_index = node_acmpdu.talker_unique_id;
			elem.ctrl_msg.data_type = JDKSAVDECC_SUBTYPE_ACMP;
			elem.ctrl_msg.msg_type = msg_type;
			elem.ctrl_msg.msg_resp_status = status;
			elem.data_frame = NULL;// set data frame
			elem.data_frame_len = 0;
			set_subject_data( elem, &gconnector_subjector );
			notify_observer( &gconnector_subjector );
		}
		else if( (status == ACMP_STATUS_SUCCESS) && \
			(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE))
		{
			if (NULL != gp_log_imp)
				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
				                LOGGING_LEVEL_DEBUG, 
				                "[ RESPONSE_RECEIVED: %d 0x%016llx (listener), %d, %d, %d, %s ]",
						RESPONSE_RECEIVED,
						end_station_entity_id,
						(uint16_t)msg_type + CMD_LOOKUP, 
						0, 
						0, 
						acmp_cmd_status_value_to_name(status));

			elem.listener_id = end_station_entity_id;
			elem.listener_index = node_acmpdu.listener_unique_id;
			elem.tarker_id = tarker_id;
			elem.tarker_index = node_acmpdu.talker_unique_id;
			elem.ctrl_msg.data_type = JDKSAVDECC_SUBTYPE_ACMP;
			elem.ctrl_msg.msg_type = msg_type;
			elem.ctrl_msg.msg_resp_status = status;
			elem.data_frame = NULL;// set data frame
			elem.data_frame_len = 0;
			set_subject_data( elem, &gconnector_subjector );
			notify_observer( &gconnector_subjector );
		}
		else if( status != ACMP_STATUS_SUCCESS )
		{
			if (NULL != gp_log_imp)
				gp_log_imp->log.post_log_msg( &gp_log_imp->log,
				                        LOGGING_LEVEL_ERROR, 
				                        "[ 0x%016llx (listener), %s, %s, %s, %s, %d ]",
							end_station_entity_id,
							acmp_cmd_value_to_name(msg_type),
							"NULL",
							"NULL", 
							acmp_cmd_status_value_to_name(status),
							seq_id);

			elem.connect_flag = false;
			elem.listener_id = end_station_entity_id;
			elem.listener_index = node_acmpdu.listener_unique_id;
			elem.tarker_id = tarker_id;
			elem.tarker_index = node_acmpdu.talker_unique_id;
			elem.ctrl_msg.data_type = JDKSAVDECC_SUBTYPE_ACMP;
			elem.ctrl_msg.msg_type = msg_type;
			elem.ctrl_msg.msg_resp_status = status;
			elem.data_frame = NULL;// set data frame
			elem.data_frame_len = 0;
			set_subject_data( elem, &gconnector_subjector );
			notify_observer( &gconnector_subjector );
			
			acmp_recv_resp_err = true;
		}
	}
	else if((msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE) ||
	(msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_CONNECTION_RESPONSE))
	{
		struct jdksavdecc_eui64 _end_station_entity_id = jdksavdecc_acmpdu_get_talker_entity_id(frame, ZERO_OFFSET_IN_PAYLOAD);
		end_station_entity_id = jdksavdecc_uint64_get(&_end_station_entity_id, 0);
		gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
				                LOGGING_LEVEL_DEBUG, 
				                "[RESPONSE_RECEIVED, 0x%016llx (talker), %s, %s, %s, %s, %d]",
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
		if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
			                        LOGGING_LEVEL_DEBUG, 
			                        "[ COMMAND_SENT, 0x%016llx listener, %s, %s, %s, %s, %d ]",
						end_station_entity_id,
						acmp_cmd_value_to_name(msg_type),
						"NULL",
						"NULL",
						acmp_cmd_status_value_to_name(status),
						seq_id );
	}

	return 0;
}

