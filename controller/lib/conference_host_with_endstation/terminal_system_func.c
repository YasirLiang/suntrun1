/*terminal_system_func.c
**ϵͳ�л���ϵͳ����
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
*****************************************/
void terminal_system_reallot_addr( void )
{
	/* ��ʼ��ϵͳ�����ַ��־ */
	init_terminal_allot_address();

	/*��ʼ����ַ�ڴ��б�*/
	init_terminal_address_list();

	/* ����ն����� ,����ͷ���*/
	terminal_system_dblist_except_free();

	/*���³�ʼ����ǰ�ն˽ڵ�*/
	terminal_pro_init_cur_terminal_node();

	/* �򿪵�ַ�ļ���������ļ����� */
	terminal_open_addr_file_wt_wb();

	/*���ע�ᴦ��*/
	terminal_register_init();

	terminal_reallot_address();
}

