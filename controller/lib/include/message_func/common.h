/*common.h
**Copyright 2015 YasirLiang, Suntron.
**Author: YasirLiang
**date:2015/10/23
**
**this file about system common queue stype define.
**
**
**
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "jdksavdecc_world.h"
#include "control_data.h"
#include "circular_link_queue.h"
#include "host_controller_debug.h"


#define SUB_DATA_TYPE_SIZE 512

typedef struct data_element		// Êý¾ÝÔªËØ
{
	uint32_t data_len;
	uint8_t data_buf[SUB_DATA_TYPE_SIZE];
}data_element_type;

#endif

