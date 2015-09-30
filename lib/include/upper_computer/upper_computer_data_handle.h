#ifndef __UPPER_COMPUTER_DATA_HANDLE_H__
#define __UPPER_COMPUTER_DATA_HANDLE_H__

#include "jdksavdecc_world.h"
#include "upper_computer.h"
#include "upper_computer_command_excute.h"
#include "upper_computer_data_parser.h"

int handle_command_from_udp_client(uint8_t type_cmd, uint16_t discuss_param_data_len, const void*discuss_param_data );
int handle_pack_event( struct host_upper_cmpt *cnfrnc_pack);
int handle_upper_computer_conference_data( struct host_upper_cmpt_frame * pframe );

#endif



