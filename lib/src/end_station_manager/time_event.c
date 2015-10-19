#include "time_event.h"
#include "aecp_controller_machine.h"

void inflight_time_tick( inflight_plist guard )
{
	assert( guard );
	inflight_plist inflight_station = guard->next;

	// check timeout and proccess with match type
	for(; inflight_station != guard; inflight_station = inflight_station->next )
	{
		if( inflight_timer_timeout(inflight_station) )
		{	
			DEBUG_LINE();
			uint8_t data_type = inflight_station->host_tx.inflight_frame.data_type;
			if( data_type == INFLIGHT_TRANSMIT_TYPE_UDP_CLIENT )
				udp_client_inflight_station_timeouts( inflight_station, guard ); // host and upper cmpt
			else if( data_type == INFLIGHT_TRANSMIT_TYPE_UDP_SERVER )
				udp_server_inflight_station_timeouts( inflight_station, guard ); // host and video controller
			else if( data_type == JDKSAVDECC_SUBTYPE_ACMP )
				acmp_inflight_station_timeouts( inflight_station, guard );
			else if( data_type == JDKSAVDECC_SUBTYPE_AECP )
				aecp_inflight_station_timeouts( inflight_station, guard );
			else 
				DEBUG_INFO("Err inflight data type");
		}
	}
}

void time_tick_event( solid_pdblist guard, inflight_plist inflight_guard )
{
	uint16_t inflight_list_lenght = (uint16_t)get_inflight_dblist_length( inflight_guard );
	
	if( inflight_list_lenght > 0 ) // proccess while there is a inflight command in the inflight command linked list
	{
		inflight_time_tick( inflight_guard );
		//DEBUG_INFO( "inflight_list_lenght = %d",  inflight_list_lenght);
	}
	adp_entity_time_tick( guard );

	//acmp_binflight_cmd_time_tick();
}

