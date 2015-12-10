#include "pelco_d.h"

/********************************
**Writer:YasirLiang
**Date:2015-11-25
**Name:pelco_d_cammand_set
**Param:
**	camera_address:camera address
**	d_cmd:pelco_d order
**	speed_lv:level camera speed
**	speed_vertical:vertical camera speed
**	askbuf:format set buf
**Func:set the pelco_d protocol format for input buf
**Return Value:0 is nomal
*********************************/
int pelco_d_cammand_set( uint8_t camera_address,  uint16_t d_cmd, uint8_t speed_lv, uint8_t speed_vertical, pelco_d_format* askbuf )
{
	assert( askbuf );

	askbuf->sync = CAMERA_SYNC_BYTE_HEAD;
	askbuf->bit_id = camera_address;
	askbuf->order = d_cmd;
	askbuf->data_code_1.level_speed = speed_lv;
	askbuf->data_code_2.vertical_speed = speed_vertical;
	askbuf->check_digit = (uint8_t)CHECK_DIGIT_RESULT( camera_address, (uint8_t)(d_cmd&0x00ff), (d_cmd >> 8)&0x00ff, speed_lv, speed_vertical );
	
	return 0;
}

