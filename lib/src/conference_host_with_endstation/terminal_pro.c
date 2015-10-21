/**
*terminal_pro.c
*
*proccess meeting
*/

#include "terminal_pro.h"
#include "linked_list_unit.h"
#include "host_controller_debug.h"
#include "conference_host_to_end.h"
#include "system_packet_tx.h"
#include "inflight.h"
#include "aecp_controller_machine.h"
#include "file_util.h"

FILE* addr_file_fd = NULL; 		// �ն˵�ַ��Ϣ��ȡ�ļ�������
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// �ն˵�ַ�����б�
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; // �ն������ͷ���
bool reallot_flag = false; // ���·����־


void init_terminal_proccess_fd( FILE ** fd )
{
	*fd = Fopen( ADDRESS_FILE, "ab+" );
	if( NULL == *fd )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}
}

/***
* ��ʼ���ն˵�ַ�б�
*/
int init_terminal_address_list( void )
{
	int i = 0;
	int ret = 0;
	
	// ��ʼ��ȫ�ֱ���tmnl_addr_list
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}

	// ���ն˵�ַ��Ϣ��tmnl_addr_list, ����ȡʧ�ܣ���ϵͳ��Ҫ���·����ն˵�ַ
	ret = terminal_address_list_read_file( addr_file_fd, tmnl_addr_list );
	if( ret == -1 )
	{
		DEBUG_INFO( "init tmnl_addr_list from address file!need to reallot terminal address");
		reallot_flag = true;
	}

	return ret;
}

inline void init_terminal_allot_address( void )
{
	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
}

inline void init_terminal_device_double_list()
{
	// init terminal system double list
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
}

#ifdef __DEBUG__  // ģ���ն���Ϣ
#define WRITE_ADDR_NUM 10
// ���Խӿڵĸ�ʽ:test_interface_ + ʵ�ʻ��õ��Ľӿ�
void 	test_interface_terminal_address_list_write_file( FILE** fd )
{
	Fclose( *fd );
	*fd = Fopen( ADDRESS_FILE, "wb+" );
	if( *fd == NULL )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}

	// write info
	int i = 0;
	for( ; i < WRITE_ADDR_NUM; i++ )
	{
		terminal_address_list tmp_addr;
		tmp_addr.addr = i;
		tmp_addr.tmn_type = 0;
		terminal_address_list_write_file( *fd, &tmp_addr, 1 );
	}
}

void print_out_terminal_addr_infomation( terminal_address_list* p, int num)
{
	int i = 0;

	printf( "Addr Info:\n" );
	for( ; i < num; i++ )
	{
		printf( "[ (addr-type)-> (%d -%d) ]\n", p[i].addr, p[i].tmn_type );
	}
}

#endif

void init_terminal_proccess_system( void )
{
	int tmnl_count = 0;
	init_terminal_proccess_fd(  &addr_file_fd );
	if( NULL == addr_file_fd )
		return;
	
#ifdef __DEBUG__ // ģ���ն���Ϣ����
	test_interface_terminal_address_list_write_file( &addr_file_fd );
#endif


	if( NULL != addr_file_fd )
	{
		tmnl_count = init_terminal_address_list();
		if( tmnl_count != -1)
			DEBUG_INFO( "terminal count num = %d", tmnl_count );
	}
	
#ifdef __DEBUG__ // ���ģ���ն���Ϣ������
	print_out_terminal_addr_infomation( tmnl_addr_list, tmnl_count );
#endif

	init_terminal_allot_address();
	init_terminal_device_double_list();
	
}
