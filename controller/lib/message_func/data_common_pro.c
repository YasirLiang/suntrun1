/*
*file:data_common_pro.c
*date:2016-05-20
*Author:YasirLiang
*/

////////////////////////////

#include "data_common_pro.h"

// 后期可以加入消息队列
void run_db_callback_func( enum enum_db_call_table_owner usr, uint32_t link, void *p_data )
{
	const struct str_db_callback *p = &gdb_callback_table[0];
	
	while( p->owner != ERROR_OWNER )
	{
		if( (p->owner == usr )&&\
			p->func_link == link )
		{
			if( p->func_proccess != NULL )
				p->func_proccess( p_data );
			break;
		}
	}
}

