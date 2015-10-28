/*terminal_system_func.c
**系统中会议系统功能
**
**
*/	

#include "terminal_system_func.h"
#include "terminal_pro.h"
#include "terminal_command.h"

/***************************************
**Writer:YasirLiang
**Date: 2015/10/27
**Name:
**Garam:
**		none
**Func: system reallot all terminal address  
*****************************************/
tmnl_pdblist terminal_system_reallot_addr( void )
{
	/* 初始化系统分配地址标志 */
	init_terminal_allot_address();

	/*初始化地址内存列表*/
	init_terminal_address_list();

	/* 清除终端链表 ,除了头结点*/
	terminal_system_dblist_except_free();

	/* 打开地址文件，并清除文件内容 */
	terminal_open_addr_file_wt_wb();

	terminal_reallot_address();
}

