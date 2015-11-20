#ifndef __TERMINAL_SYSTEM_H__
#define __TERMINAL_SYSTEM_H__

#include "jdksavdecc_world.h"

#define TMN_RGST_STATE	0x00000001 	 //Ô¶¶Ë±¨µ½ 1
#define CAMERA_PRESET        (0x00000001<<1) //ÉãÏñÍ·Ô¤ÖÃ 2
#define SIGN_STATE          	(0x00000001<<2) //Ç©µ½×´Ì¬ 4
#define DISCUSS_STATE         (0x00000001<<3) //ÌÖÂÛ×´Ì¬ 8
#define VOTE_STATE          	(0x00000001<<4) //±í¾ö×´Ì¬ 16
#define GRADE_STATE       	(0x00000001<<5) //ÆÀ·Ö×´Ì¬ 32
#define ELECT_STATE         	(0x00000001<<6) //Ñ¡¾Ù×´Ì¬ 64 
#define INTERPOSE_STATE     (0x00000001<<7) //Ö÷Ï¯²å»° 128
#define HOST_STA_NUM  8

typedef struct _thost_system_state
{
	uint8_t host_state;
}thost_sys_state;

void init_terminal_system_state( void );
thost_sys_state get_terminal_system_state( void );
bool set_terminal_system_state( uint8_t state_value, bool is_pre );
uint8_t get_sys_state( void );

#endif

