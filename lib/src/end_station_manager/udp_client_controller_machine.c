#include "udp_client_controller_machine.h"

static struct udp_server server_fd; // host udp as udp server information

void init_udp_client_controller_endstation( int fd, struct sockaddr_in *sin )
{
	server_fd.s_fd = fd;
	memcpy( &server_fd.srvaddr, sin, sizeof(struct sockaddr_in));
	server_fd.srvlen = sizeof(struct sockaddr_in);
}

// used for host and upper computer
int transmit_udp_client_packet( int fd, uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, struct sockaddr_in* sin, bool resp )
{
	uint8_t cfc_type = get_host_upper_cmpt_guide_type( frame, UPPER_HOST_CONFERENCE_HEADER_TYPE_OFFSET );
	uint8_t cfc_cmd = get_host_upper_cmpt_command_type( frame, UPPER_HOST_CONFERENCE_COMMAND_TYPE_OFFSET );
	uint32_t timeout = get_udp_client_timeout_table( cfc_cmd );
	inflight_plist inflight_station = NULL;
	uint8_t dest_mac[6] = {0};
	
	assert( sin );
	struct sockaddr_in sin_event;
	memcpy(&sin_event, sin, sizeof(struct sockaddr_in));
	
	if( !resp ) // not a response data 
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
				inflight_station->host_tx.inflight_frame.inflight_frame_len = frame_len;
				memcpy( inflight_station->host_tx.inflight_frame.frame, frame, frame_len );
				inflight_station->host_tx.inflight_frame.data_type = cfc_type; //协议类型(ac)
				inflight_station->host_tx.inflight_frame.seq_id = cfc_cmd;	// 协议命令
				inflight_station->host_tx.inflight_frame.notification_flag = RUNINFLIGHT;
				memcpy( inflight_station->host_tx.inflight_frame.raw_dest.value, dest_mac , 6 );
				memcpy( &inflight_station->host_tx.inflight_frame.sin_in, &sin_event , sizeof(struct sockaddr_in) );
				
				inflight_station->host_tx.command_type = TRANSMIT_TYPE_UDP_CLT;
				inflight_station->host_tx.flags.retried = 1;	// meaning send once
				inflight_station->host_tx.flags.resend = false;
				inflight_timer_start( timeout, inflight_station );

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
			inflight_station = search_node_inflight_from_dblist( guard, cfc_cmd, cfc_type );
			if( inflight_station != NULL ) //already search it
			{
				inflight_station->host_tx.flags.resend = true;
				inflight_station->host_tx.flags.retried++ ;
				inflight_timer_state_avail( timeout, inflight_station );
			}
			else
			{
				DEBUG_INFO( "udp server nothing to be resend!" );
				assert(inflight_station != NULL);
			}
		}
	}

	int send_len = send_udp_frame( fd, frame, frame_len, &sin_event );
	if( send_len < 0)
	{
		DEBUG_ERR("send udp frame Err!");
		assert( send_len >= 0 );
	}
	
	return send_len;
}

/************************************************************************
*Writer:	YasirLiang
*Data:	2015/10/16
*Name:	udp_client_inflight_station_timeouts
*Func: proccess udp client inflight command timeout
*state:if data of sending to udp client by host is responsable data, this function will never run 
*/
void 	udp_client_inflight_station_timeouts( inflight_plist inflight_station, inflight_plist guard )
{
	bool is_retried = false;
	inflight_plist udp_client_pstation = NULL;
	uint8_t *frame = NULL;
	uint16_t frame_len = 0;
         
	if( inflight_station != NULL )
	{
		udp_client_pstation = inflight_station;
		is_retried = is_inflight_cmds_retried( udp_client_pstation );
		frame = udp_client_pstation->host_tx.inflight_frame.frame;
		frame_len = udp_client_pstation->host_tx.inflight_frame.inflight_frame_len;
	}
	else 
	{
		DEBUG_INFO( "noting to be proccessed by udp client timeout" );
		return;
	}

	assert( frame && inflight_station != NULL );
	if( is_retried )
	{
		uint8_t upper_cmpt_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD + 2);
        	uint8_t  upper_cmpt_deal_type = get_host_upper_cmpt_deal_type( frame, ZERO_OFFSET_IN_PAYLOAD );
			
		DEBUG_INFO( " [ COMMAND TIMEOUT: %s, %s, %s, %d ]", 
					upper_cmpt_cmd_value_to_string_name( upper_cmpt_cmd ),
					upper_cmpt_cmd_value_to_string_name( udp_client_pstation->host_tx.inflight_frame.seq_id ),
					"NULL",
					upper_cmpt_deal_type );

		// free inflight command node in the system
		release_heap_space( &udp_client_pstation->host_tx.inflight_frame.frame);
		delect_inflight_dblist_node( &udp_client_pstation );
	}
	else
	{
		DEBUG_INFO( " udp client information resended " );
		// udp data sending is not response
		transmit_udp_client_packet( server_fd.s_fd, frame, frame_len, guard, true, &udp_client_pstation->host_tx.inflight_frame.sin_in, false );
	}
}

