#ifndef __TERMINAL_SYSTEM_H__
#define __TERMINAL_SYSTEM_H__

#include "jdksavdecc_world.h"

#define TMN_RGST_STATE	0x00000001 	 //Ô¶¶Ë±¨µ½
#define CAMERA_PRESET        (0x00000001<<1) //ÉãÏñÍ·Ô¤ÖÃ
#define SIGN_STATE          	(0x00000001<<2) //Ç©µ½×´Ì¬
#define DISCUSS_STATE         (0x00000001<<3) //ÌÖÂÛ×´Ì¬
#define VOTE_STATE          	(0x00000001<<4) //±í¾ö×´Ì¬
#define GRADE_STATE       	(0x00000001<<5) //ÆÀ·Ö×´Ì¬
#define ELECT_STATE         	(0x00000001<<6) //Ñ¡¾Ù×´Ì¬
#define INTERPOSE_STATE     (0x00000001<<7) //Ö÷Ï¯²å»°
#define HOST_STA_NUM  8

typedef struct _thost_system_state
{
	uint8_t host_state;
}thost_sys_state;

thost_sys_state get_terminal_system_state( void );
bool set_terminal_system_state( uint8_t state_value, bool is_pre );

#endif

