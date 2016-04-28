/**
*file:en485_send.h
*build date:27-4-2016
*writer:YasirLiang
*/

//******************
//使能底板485发送数据
//
//******************

#ifndef __EN485_SEND_H__
#define __EN485_SEND_H__

#include "jdksavdecc_world.h"

extern bool en485_send_init( void );// 使能485发送端，成功返回真，否则返回假
extern void en485_send_mod_cleanup( void );

#endif
