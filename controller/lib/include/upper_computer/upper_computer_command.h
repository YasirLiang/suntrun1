#ifndef __UPPER_COMPUTER_COMMAND_H__
#define __UPPER_COMPUTER_COMMAND_H__

#include "enum.h"
#include "jdksavdecc_world.h"

int send_upper_computer_command( uint8_t deal_type, uint8_t command, const void *data, uint16_t data_len );

#endif

