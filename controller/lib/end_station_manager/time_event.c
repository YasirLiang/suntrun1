#include "time_event.h"
#include "aecp_controller_machine.h"
#include "arcs_common.h"
#include "send_common.h" // °üº¬SEND_DOUBLE_QUEUE_EABLE

bool inflight_list_has_command(void)
{
	bool found = true;
	inflight_plist guard = command_send_guard;
	
	if (guard->next == guard)
		found = false;

	return found;
}

bool inflight_conference_command_exist(void)
{
	inflight_plist guard = command_send_guard;
	inflight_plist inflight_station = NULL;
	bool found = false;
	
	for (inflight_station = guard->next; inflight_station != guard; inflight_station = inflight_station->next)
	{
		uint8_t data_type = inflight_station->host_tx.inflight_frame.data_type;
		if (data_type == JDKSAVDECC_SUBTYPE_AECP)
		{
			if (inflight_station->host_tx.inflight_frame.frame != NULL)
			{
				uint32_t msg_type = jdksavdecc_common_control_header_get_control_data(inflight_station->host_tx.inflight_frame.frame, ZERO_OFFSET_IN_PAYLOAD);
				if (msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND||
					(msg_type == JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE))
				{
					found = true;
					break;
				}
			}
		}		
	}

	return found;
}

void inflight_time_tick( inflight_plist guard )
{
	inflight_plist inflight_station = NULL;
	inflight_plist tmp_inflight = NULL;
	assert( guard );
	if( guard == NULL )
		return;
	
	// check timeout and proccess with match type, only check the list of head node
	for( inflight_station = guard->next; inflight_station != guard; inflight_station = tmp_inflight )
	{
		tmp_inflight = inflight_station->next; // save next node
		uint8_t data_type = inflight_station->host_tx.inflight_frame.data_type;
		if( inflight_timer_timeout(inflight_station) )
		{	
			if( data_type == INFLIGHT_TRANSMIT_TYPE_UDP_CLIENT )
				udp_client_inflight_station_timeouts( inflight_station, guard ); // host and upper cmpt
			else if( data_type == INFLIGHT_TRANSMIT_TYPE_UDP_SERVER )
				udp_server_inflight_station_timeouts( inflight_station, guard ); // host and video controller
			else if( data_type == JDKSAVDECC_SUBTYPE_ACMP )
				acmp_inflight_station_timeouts( inflight_station, guard );
			else if( data_type == JDKSAVDECC_SUBTYPE_AECP )
				aecp_inflight_station_timeouts( inflight_station, guard );
                        else if (data_type == INFLIGHT_TRANSMIT_TYPE_ARCS) {
                            ArcsCommon_packetTimeouts(inflight_station);
                        }
			else
			{
				DEBUG_INFO("Err inflight data type = 0x%02x", data_type );
				release_heap_space( &inflight_station->host_tx.inflight_frame.frame );// must release frame space first while need to free inflight node
				delect_inflight_dblist_node( &inflight_station );
#ifndef SEND_DOUBLE_QUEUE_EABLE
				sem_post( &sem_waiting );
#endif
			}

			break;// if timerout ; return.
		}
	}
}

void time_tick_event( solid_pdblist guard, inflight_plist inflight_guard )
{
	if( guard != NULL );
		adp_entity_time_tick( guard );

	if( inflight_guard == NULL )
		return;
	
	pthread_mutex_lock(&ginflight_pro.mutex);
	inflight_time_tick( inflight_guard );
	pthread_mutex_unlock(&ginflight_pro.mutex);
#if 0
	acmp_binflight_cmd_time_tick();
#endif
}

