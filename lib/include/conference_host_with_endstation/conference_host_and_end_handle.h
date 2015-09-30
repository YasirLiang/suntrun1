#ifndef __CONFERENCE_HOST_AND_END_HANDLE_H__
#define __CONFERENCE_HOST_AND_END_HANDLE_H__

#include "jdksavdecc_world.h"
#include "conference.h"

void handle_end_to_host_command_type( uint8_t order, const void* packet );
void handle_end_to_host_normal_packet( const struct endstation_to_host* nml_pacet );
void handle_end_to_host_special_packet(const struct endstation_to_host_special* spl_paceket );
int handle_end_to_host_packet_deal_func(int param, const struct endstation_to_host* normal_pacet, const struct endstation_to_host_special* special_paceket );

#endif




