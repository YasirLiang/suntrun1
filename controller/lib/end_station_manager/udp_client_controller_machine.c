#include "udp_client_controller_machine.h"
#include "upper_computer_common.h"
#include "wait_message.h"
#include "send_pthread.h"
#include "send_common.h"
#include "log_machine.h"

static struct socket_info_s server_fd; // host udp as udp server information
static inflight_plist udp_client_inflight = NULL;

#ifdef __DEBUG__
#define __UDP_CLIENT_CTL_MACH_DEBUG__ // udp client 调试开关
#endif

void init_udp_client_controller_endstation( int fd, struct sockaddr_in *sin )
{
	server_fd.sock_fd = fd;
	memcpy( &server_fd.sock_addr , sin, sizeof(struct sockaddr_in));
	server_fd.sock_len = sizeof(struct sockaddr_in);

	assert( command_send_guard );
	if( command_send_guard == NULL )
		return;
	
	udp_client_inflight = command_send_guard;
}

// used for host and upper computer, only have one this type inflight in the double list
int transmit_udp_client_packet( int fd, uint8_t* frame, uint32_t frame_len, inflight_plist resend_node, bool resend, struct sockaddr_in* sin, bool resp, uint32_t *interval_time )
{
	uint8_t cfc_type = get_host_upper_cmpt_guide_type( frame, UPPER_HOST_CONFERENCE_HEADER_TYPE_OFFSET );
	uint8_t cfc_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	inflight_plist inflight_station = NULL;
	uint8_t dest_mac[6] = {0};
	uint32_t timeout = get_udp_client_timeout_table( cfc_cmd );

	assert( interval_time && sin );
	*interval_time = timeout;
	struct sockaddr_in sin_event;
	memcpy(&sin_event, sin, sizeof(struct sockaddr_in));

	if( (frame_len > TRANSMIT_DATA_BUFFER_SIZE) || (frame_len <= 0) )
	{
		if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                LOGGING_LEVEL_ERROR, 
        		                "[udp packet( size = %d )bigger than frame buf %d or little!]",
			frame_len,TRANSMIT_DATA_BUFFER_SIZE );
        
		return -1;
	}
			
	if( !resp ) // not a response data 
	{
		if( !resend )// data first send
		{
				inflight_station = create_inflight_dblist_new_node( &inflight_station );
				if( NULL == inflight_station )
				{
					if (NULL != gp_log_imp)
				                gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                                        LOGGING_LEVEL_ERROR,
        		                                        "inflight station node create failed!");
					return -1;
				}
				memset(inflight_station, 0, sizeof(inflight_list));
				inflight_station->host_tx.inflight_frame.frame = allot_heap_space( frame_len, &inflight_station->host_tx.inflight_frame.frame );
				if( NULL != inflight_station->host_tx.inflight_frame.frame )
				{
					memset(inflight_station->host_tx.inflight_frame.frame, 0, frame_len );
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
					if( udp_client_inflight != NULL )
						insert_inflight_dblist_trail( udp_client_inflight, inflight_station );
				}
				else
				{
					DEBUG_INFO("Err frame malloc !");
					assert( NULL != inflight_station->host_tx.inflight_frame.frame );
					if( NULL == inflight_station->host_tx.inflight_frame.frame )
						return -1;
				}
		}
		else
		{
			if( resend_node != NULL ) //already search it
			{
				resend_node->host_tx.flags.resend = true;
				resend_node->host_tx.flags.retried++ ;
				inflight_timer_state_avail( timeout, resend_node );
			}
			else
			{
				assert(resend_node != NULL);
				if( resend_node == NULL )
					return -1;
			}
		}
	}

	int send_len = send_udp_frame( fd, frame, frame_len, &sin_event );
	if( send_len < 0)
	{
		if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                LOGGING_LEVEL_ERROR, 
        		                "send udp frame Err!");
        
		assert( send_len >= 0 );
		if( send_len < 0 )
			return -1;
	}
	
	return send_len;
}

/************************************************************************
*Writer:	YasirLiang
*Data:	2015/10/16
*Name:	udp_client_inflight_station_timeouts
*Func: proccess udp client inflight command timeout
*state:if data of sending to udp client by host is responsable data, this function will never run 
**************************************************************************/
void 	udp_client_inflight_station_timeouts( inflight_plist inflight_station, inflight_plist guard )
{
	bool is_retried = false;
	uint8_t *frame = NULL;
	uint16_t frame_len = 0;
	uint32_t interval_time = 0;
         
	if( inflight_station != NULL )
	{
		is_retried = is_inflight_cmds_retried( inflight_station );
		frame = inflight_station->host_tx.inflight_frame.frame;
		frame_len = inflight_station->host_tx.inflight_frame.inflight_frame_len;
	}
	else 
	{
		return;
	}
	
	assert( frame && inflight_station != NULL );
	if( frame == NULL || inflight_station == NULL )
		return;
	
	if( is_retried )
	{
		uint8_t upper_cmpt_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
        	uint8_t upper_cmpt_deal_type = get_host_upper_cmpt_deal_type( frame, ZERO_OFFSET_IN_PAYLOAD );
		uint16_t data_len = get_host_upper_cmpt_data_len( frame, ZERO_OFFSET_IN_PAYLOAD );

            if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                LOGGING_LEVEL_ERROR,
        		                "[ UPPER COMMAND TIMEOUT: %s, %s, %s, %d (data len = %d )]", 
					upper_cmpt_cmd_value_to_string_name( upper_cmpt_cmd ),
					upper_cmpt_cmd_value_to_string_name( inflight_station->host_tx.inflight_frame.seq_id ),
					"NULL",
					upper_cmpt_deal_type, data_len );

		/* free inflight command node in the system */
		release_heap_space( &inflight_station->host_tx.inflight_frame.frame );
		delect_inflight_dblist_node( &inflight_station );
		is_inflight_timeout = true; /* 设置超时*/	
	}
	else
	{
		if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                LOGGING_LEVEL_DEBUG,
        		                " udp client information resended " );
        
		transmit_udp_client_packet( server_fd.sock_fd, frame, frame_len, inflight_station, true, &inflight_station->host_tx.inflight_frame.sin_in, false, &interval_time );
	}
}

int udp_client_update_inflight_comand( uint8_t *frame, int frame_len )
{
	assert( frame != NULL );
	return udp_client_proc_resp( frame, frame_len );
}

int udp_client_proc_resp( uint8_t *frame, int frame_len  )
{
	assert( frame != NULL );
	uint8_t subtype = get_host_upper_cmpt_guide_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	uint8_t upper_cmpt_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
        uint8_t upper_cmpt_deal_type = get_host_upper_cmpt_deal_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	inflight_plist inflight_udp_client = NULL;
	bool is_found_inflight_stations = false;

	if( udp_client_inflight == NULL )
		return -1;

	if( (subtype == UPPER_COMPUTER_DATA_LOADER) && (upper_cmpt_deal_type & CMPT_MSG_TYPE_RESPONSE) )// 是响应报文
	{
		inflight_udp_client = search_node_inflight_from_dblist( udp_client_inflight, upper_cmpt_cmd, subtype ); // found? 
		if( inflight_udp_client != NULL )
		{
			is_found_inflight_stations = true;
		}
	}

	if( is_found_inflight_stations )
	{
		assert( inflight_udp_client->host_tx.inflight_frame.frame );
		udp_client_callback( CMD_WITH_NOTIFICATION, frame );
		release_heap_space( &inflight_udp_client->host_tx.inflight_frame.frame ); // must release frame space first while need to free inflight node
		delect_inflight_dblist_node( &inflight_udp_client );	// delect aecp inflight node
	}
	else
	{
		return -1;
	}

	return 0;
}

void udp_client_callback( uint32_t notification_flag, uint8_t *frame )
{
	assert( frame );
        if (frame == NULL)
            return;
        
	uint8_t upper_cmpt_cmd = get_host_upper_cmpt_command_type( frame, ZERO_OFFSET_IN_PAYLOAD );
        uint8_t upper_cmpt_deal_type = get_host_upper_cmpt_deal_type( frame, ZERO_OFFSET_IN_PAYLOAD );
	uint16_t data_len = get_host_upper_cmpt_data_len( frame, ZERO_OFFSET_IN_PAYLOAD ); 
	
	if( notification_flag == CMD_WITH_NOTIFICATION )
        {
        	if (NULL != gp_log_imp)
			gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
        		                LOGGING_LEVEL_DEBUG,
        		                "[ UDP CLIENT RESPONSE: %s, %d, %02x(type) ( data len = %d )]",  
        		                upper_cmpt_cmd_value_to_string_name( upper_cmpt_cmd ),
        		                0, 
        		                upper_cmpt_deal_type, 
        		                data_len );
        }
}

