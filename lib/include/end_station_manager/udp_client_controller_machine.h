#ifndef __UDP_CLIENT_CONTROLLER_MACHINE_H__
#define __UDP_CLIENT_CONTROLLER_MACHINE_H__

#include "inflight.h"
#include "upper_computer_data_parser.h"
#include "linked_list_unit.h"
#include "system_packet_tx.h"
#include "avdecc_net.h"

extern struct udp_client upper_udp_client;		    // 上位机的通信信息


int transmit_udp_client_packet( int fd, uint8_t* frame, uint32_t frame_len, inflight_plist guard, bool resend, struct sockaddr_in* sin, bool resp );
void 	udp_client_inflight_station_timeouts( inflight_plist inflight_station, inflight_plist guard );

#endif

