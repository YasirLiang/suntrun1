#ifndef __UDP_SERVER_CONTROLLER_MACHINE_H__
#define __UDP_SERVER_CONTROLLER_MACHINE_H__

#include "inflight.h"

int transmit_udp_packet_server( int fd, const uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, struct sockaddr_in *sin,const bool resp );
void udp_server_inflight_station_timeouts( inflight_plist inflight_station, inflight_plist guard );


#endif
