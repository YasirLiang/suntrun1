#include "terminal_pro.h"

terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// �ն˵�ַ�����б�
terminal_address_list_pro allot_addr_pro;	

tmnl_pdblist dev_terminal_list_guard; // �ն������ͷ���

void init_terminal_address_list( void )
{
	int i = 0;

	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list));
	for( i = 0; i< SYSTEM_TMNL_MAX_NUM; i++)		// ��ַ��ʼ��Ϊ0xff
	{
		tmnl_addr_list[i].addr = 0xff;
	}
}


