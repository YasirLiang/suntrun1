#include "time_event.h"
#include "aecp_controller_machine.h"

void inflight_time_tick( inflight_plist guard )
{
	assert( guard );
	inflight_plist inflight_station = NULL, tmp_inflight = NULL;
	
	// check timeout and proccess with match type, only check the list of head node
	for( inflight_station = guard->next; inflight_station != guard; inflight_station = tmp_inflight )
	{
		tmp_inflight = inflight_station->next; // save next node
		uint8_t data_type = inflight_station->host_tx.inflight_frame.data_type;
		
		//DEBUG_INFO( "inflight data type = %d", data_type );
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
			else
			{
				DEBUG_INFO("Err inflight data type = 0x%02x", data_type );
				release_heap_space( &inflight_station->host_tx.inflight_frame.frame );// must release frame space first while need to free inflight node
				delect_inflight_dblist_node( &inflight_station );
			}
		}/*
		else
		{
			break; // only check the head
		}*/
	}
}

void time_tick_event( solid_pdblist guard, inflight_plist inflight_guard )
{
	adp_entity_time_tick( guard );
	inflight_time_tick( inflight_guard );

	//acmp_binflight_cmd_time_tick();
}

