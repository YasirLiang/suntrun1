/*terminal_system_func.c
**ϵͳ�л���ϵͳ����
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
	/* ��ʼ��ϵͳ�����ַ��־ */
	init_terminal_allot_address();

	/*��ʼ����ַ�ڴ��б�*/
	init_terminal_address_list();

	/* ����ն����� ,����ͷ���*/
	terminal_system_dblist_except_free();

	/* �򿪵�ַ�ļ���������ļ����� */
	terminal_open_addr_file_wt_wb();

	terminal_reallot_address();
}

