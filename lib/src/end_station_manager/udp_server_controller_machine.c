#include "udp_server_controller_machine.h"

int transmit_udp_packet_server( int fd, const uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, struct sockaddr_in *sin,const bool resp  )
{
	return 0;
}

void udp_server_inflight_station_timeouts( inflight_plist inflight_station, inflight_plist guard )
{
	
}

void udp_server_inflight_time_tick()
{

}


