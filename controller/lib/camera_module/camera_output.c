/*
**camera_output.c
**2015-12-09
**控制摄像头的输出
*/

#include "camera_output.h"

uint8_t gcamera_in[2];

int camera_output_switch( uint8_t input_chn, uint8_t output_chn, bool is_change )
{
	return 0;
}

uint8_t camera_output_get_input( uint8_t out )
{
	if( (out <= 0) ||(out > 2) )
		return 0xff;

	return gcamera_in[out-1];
}

