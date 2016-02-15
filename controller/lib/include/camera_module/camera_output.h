/*
**camera_output.h
**2015-12-09
**控制摄像头的输出的相关定义
*/

#ifndef __CAMERA_OUTPUT_H__
#define __CAMERA_OUTPUT_H__

#include "jdksavdecc_world.h"

#define CAMERA_OUTPUT_CHANNAL 2
#define CAMERA_OUT_FULL_VIEW 1
#define CAMERA_OUT_TRACK_VIEW 2

int camera_output_switch( uint8_t input_chn, uint8_t output_chn, bool is_change );
uint8_t camera_output_get_input( uint8_t out );


#endif
