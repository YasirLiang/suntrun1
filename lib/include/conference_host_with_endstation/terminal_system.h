#ifndef __TERMINAL_SYSTEM_H__
#define __TERMINAL_SYSTEM_H__

#include "jdksavdecc_world.h"

#define TMN_RGST_STATE	0x00000001 	 //Զ�˱��� 1
#define CAMERA_PRESET        (0x00000001<<1) //����ͷԤ�� 2
#define SIGN_STATE          	(0x00000001<<2) //ǩ��״̬ 4
#define DISCUSS_STATE         (0x00000001<<3) //����״̬ 8
#define VOTE_STATE          	(0x00000001<<4) //���״̬ 16
#define GRADE_STATE       	(0x00000001<<5) //����״̬ 32
#define ELECT_STATE         	(0x00000001<<6) //ѡ��״̬ 64 
#define INTERPOSE_STATE     (0x00000001<<7) //��ϯ�廰 128
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

