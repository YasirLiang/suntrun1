/*
**avdecc_manage.h
**2016-3-3
**
**
*/

#ifndef __AVDECC_MANAGE_H__
#define __AVDECC_MANAGE_H__

#include "host_timer.h"

typedef struct type_avdecc_manage
{
	bool running;
	bool enable;// ʹ�ܴ����־
	host_timer query_timer;// ��ѯ��ʱ��
}tavdecc_manage;

void avdecc_manage_insert_database( void );
void avdecc_manage_discover_proccess( void );
void avdecc_manage_read_desc_proccess( void );
void avdecc_manage_remove_device_proccess( void );
void avdecc_manage_init( void );

#endif
