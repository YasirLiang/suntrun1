/*terminal_system_func.c
**系统中会议系统功能
**
**
*/	

#include "terminal_system_func.h"
#include "terminal_command.h"

/***************************************
**Writer:YasirLiang
**Date: 2015/10/27
**Name:
**Garam:
**		none
**Func: system reallot all terminal address  
**注意:若应用于多线程环境，必须为地址内存列表,
**		终端链表\进行互斥锁操作，(2016-07-16注)
*****************************************/

extern void conference_transmit_unit_cleanup_conference_node(void);
void terminal_system_reallot_addr( void )
{
	/* 初始化系统分配地址标志 */
	init_terminal_allot_address();

	/*初始化地址内存列表*/
	init_terminal_address_list();

	/*必须置所有被引用终端节点指针为空，在 清除终端链表之前*/
	conference_transmit_unit_cleanup_conference_node();

	/* 清除终端链表 ,除了头结点*/
	//terminal_system_dblist_except_free();

        /* 清除除了target_id的终端链表 节点信息*/
        terminal_system_clear_node_info_expect_target_id();

	/*重新初始化当前终端节点*/
	terminal_pro_init_cur_terminal_node();

	/* 打开地址文件，并清除文件内容 */
	terminal_open_addr_file_wt_wb();

	/*清除注册处理*/
	//terminal_register_init();

	terminal_reallot_address();
}

