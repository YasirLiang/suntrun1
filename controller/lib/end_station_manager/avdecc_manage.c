/*
**avdecc_manage.c
**2016-3-3
**
**
*/



#include "avdecc_manage.h"
#include "system_database.h"
#include "adp_controller_machine.h"

tavdecc_manage gadp_discover_pro; // 发现终端处理
tavdecc_manage gread_descripor_pro;// 读终端信息描述符处理
tavdecc_manage gremove_device_pro;// 自动移除终端处理

void avdecc_manage_insert_database( void )
{
	system_db_avdecc_info_insert( gadp_discover_pro, gread_descripor_pro, gremove_device_pro );
}

void avdecc_manage_discover_proccess( void )
{
	if( !gadp_discover_pro.running )
		return;
	
	if( gadp_discover_pro.enable && host_timer_timeout( &gadp_discover_pro.query_timer ) )
	{
		struct jdksavdecc_eui64 zero;
		bzero( &zero, sizeof(struct jdksavdecc_eui64));

		// found all endpoints
		adp_entity_avail( zero, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER );
	}
}

void avdecc_manage_read_desc_proccess( void )
{
	
}

void avdecc_manage_remove_device_proccess( void )
{
	
}

void avdecc_manage_init( void )
{
	gadp_discover_pro.running = true;
}
