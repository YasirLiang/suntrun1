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
#include "conference_end_to_host.h"
#include "profile_system.h"
#include "terminal_command.h"
#include "terminal_system.h"
#include "upper_computer_pro.h"
#include "camera_pro.h"
#include "time_handle.h"

#include "conference_transmit_unit.h"

FILE* addr_file_fd = NULL; 								// �ն˵�ַ��Ϣ��ȡ�ļ�������
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	// �ն˵�ַ�����б�
terminal_address_list_pro allot_addr_pro;	
tmnl_pdblist dev_terminal_list_guard = NULL; 				// �ն������ͷ��㣬������ȷ�ز�����������ע�����ն�
volatile bool reallot_flag = false; 							// ���·����־
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
tsys_discuss_pro gdisc_flags; 								// ϵͳ���۲���
tchairman_control_in gchm_int_ctl; 						// ��ϯ�廰
volatile ttmnl_register_proccess gregister_tmnl_pro; 			// �ն˱�������
volatile uint8_t speak_limit_time = 0; 						// ����ʱ���� 0��ʾ����ʱ��1-63��ʾ��ʱ1-63����
volatile uint8_t glcd_num = 0; 							// lcd ��ʾ������
volatile uint8_t gled_buf[2] = {0}; 							// �ն�ָʾ��
volatile enum_signstate gtmnl_signstate;					// ǩ����״̬��Ҳ��Ϊ�ն˵�ǩ��״̬
volatile uint8_t gsign_latetime; 							// ��ǩ�ĳ�ʱʱ��
volatile bool gsigned_flag = false;							// ǩ����־
volatile evote_state_pro gvote_flag = NO_VOTE; 						// ͶƱ����
volatile uint16_t gvote_index;								// ͶƱƫ��
volatile bool gfirst_key_flag; 								// ��ΪͶƱ�׼���Ч
volatile tevote_type gvote_mode;							// ͶƱģʽ

type_spktrack gspeaker_track;

tquery_svote  gquery_svote_pro;							// ��ѯǩ��������

void init_terminal_proccess_fd( FILE ** fd )
{
	*fd = Fopen( ADDRESS_FILE, "ab+" );
	if( NULL == *fd )
	{
		DEBUG_ERR( "init terminal addr fd Err!" );
		assert( NULL != *fd );
	}
}

/*==============================================
��ʼ���ն˵�ַ�б�
================================================*/
int init_terminal_address_list_from_file( void )
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
		DEBUG_INFO( "init tmnl_addr_list from address file!need to reallot terminal address\n\t\tPlease send reAllot command by command line!!!");
		reallot_flag = true;
	}

	return ret;
}

void init_terminal_address_list( void )
{
	int i = 0;
	
	// ��ʼ��ȫ�ֱ���tmnl_addr_list
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}
}

/*==============================================
������ʼ���ն˵�ַ�б�
================================================*/


inline void init_terminal_allot_address( void )
{
	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
	reallot_flag = false; // disable reallot
}

inline void init_terminal_device_double_list( void )
{
	// init terminal system double list
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
}

/*
*date:2015-11-6
*/
int init_terminal_discuss_param( void ) 
{
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	gdisc_flags.apply_limit = set_sys.apply_limit;
	gdisc_flags.limit_num = set_sys.speak_limit;
	gdisc_flags.currect_first_index = set_sys.apply_limit;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0; // ��������
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)set_sys.discuss_mode;
	memset( gdisc_flags.speak_addr_list, 0xffff, MAX_LIMIT_SPK_NUM );
	memset( gdisc_flags.apply_addr_list, 0xffff, MAX_LIMIT_APPLY_NUM );

	return 0;
}

void terminal_speak_track_pro_init( void )
{
	int i = 0;

	gspeaker_track.spk_num = 0;
	for( i = 0; i < MAX_SPK_NUM;i++ )
	{
		gspeaker_track.spk_addrlist[i] = 0xffff;
	}
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
		return;
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

void print_out_terminal_addr_infomation( terminal_address_list* p, int num )
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
	int tmnl_count = 0, i = 0;
	uint16_t addr = 0xffff;
	
	terminal_register_init();
	init_terminal_proccess_fd( &addr_file_fd );
	if( NULL == addr_file_fd )
		return;
#if 0	
#ifdef __DEBUG__ // ģ���ն���Ϣ����
	test_interface_terminal_address_list_write_file( &addr_file_fd );
#endif
#endif

	if( NULL != addr_file_fd )
	{
		tmnl_count = init_terminal_address_list_from_file();
		if( tmnl_count != -1)
		{
			DEBUG_INFO( "terminal count num = %d", tmnl_count );

			gregister_tmnl_pro.tmn_total = tmnl_count;// ���浽δע���б�
			for( i = 0; i < tmnl_count; i++ )
			{
				addr = tmnl_addr_list[i].addr;
				if( addr != 0xffff )
					terminal_register_pro_address_list_save( addr, false );
			}

#ifdef __DEBUG__
				int unregister_index = gregister_tmnl_pro.noregister_head;

				printf( "No Register List : (head index = %d)---(trail index = %d)\n\t", unregister_index, gregister_tmnl_pro.noregister_trail );
				for( ; unregister_index <= gregister_tmnl_pro.noregister_trail; unregister_index++ )
					fprintf( stdout, "%04x  ", gregister_tmnl_pro.register_pro_addr_list[unregister_index] );
				fprintf( stdout,"\n" );
#endif
		}		
	}

#ifdef __DEBUG__ // ����ն���Ϣ������
	print_out_terminal_addr_infomation( tmnl_addr_list, tmnl_count );
	if( tmnl_count != -1 )
		DEBUG_INFO( "terminal count num = %d", tmnl_count );
#endif

	init_terminal_allot_address();
	init_terminal_device_double_list();	
	init_terminal_discuss_param();
	terminal_speak_track_pro_init();
	terminal_query_proccess_init();
}

/*�ͷ��ն���Դ2016-1-23*/
void terminal_proccess_system_close( void )
{
	if( addr_file_fd != NULL )
	{
		Fclose( addr_file_fd );// �ر��ļ�������
		if( addr_file_fd != NULL )
			addr_file_fd = NULL;
	}
}
/*********************************************
=ע�ᴦ����غ�����ʼ
**********************************************/
// �ն�ע�ᱣ���ַ����ע���δע���б�,ע:������ע����δע���ն˵��㷨��ͬ
bool terminal_register_pro_address_list_save( uint16_t addr_save, bool is_register_save )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (addr_save != 0xffff))
	{
		if( !is_register_save )
		{
			volatile uint16_t *p_unregister_trail = &p_regist_pro->noregister_trail;
			volatile bool *p_unregister_full = &p_regist_pro->unregister_list_full;
			if(  *p_unregister_trail < p_regist_pro->list_size )
			{
				if( (p_regist_pro->register_pro_addr_list[*p_unregister_trail] == 0xffff) &&\
					(*p_unregister_trail == p_regist_pro->noregister_head ) )
				{// ��ǰ�б���δע��ĵ�ַ
					p_regist_pro->register_pro_addr_list[*p_unregister_trail] = addr_save;//  �ȱ��棬�����б���δע��ĵ�ַ
				}
				else if( p_regist_pro->register_pro_addr_list[*p_unregister_trail] != 0xffff )
				{
					p_regist_pro->register_pro_addr_list[++(*p_unregister_trail)] = addr_save;// ���ƶ�trail��ԭ����trail ��������Ԫ�ص��±�
				}
				
				if( *p_unregister_trail >= p_regist_pro->list_size )
				{
					*p_unregister_full = true;
				}

				return true;
			}
		}
		else
		{
			/*
			**1����δע���б�ͷ��Ԫ�ز���δע���β��
			**2��δע���ͷ����1
			**3������ע���б��β�������豣��ĵ�ַ
			*/ 
			if( terminal_register_pro_address_list_save( p_regist_pro->register_pro_addr_list[p_regist_pro->noregister_head], false ) )
			{
				p_regist_pro->noregister_head++;
				p_regist_pro->register_pro_addr_list[++p_regist_pro->rgsted_trail] = addr_save;
			}
		}
	}

	return false;
}

// ��δע���б���ɾ����ע���ն�,������˵�ַ����ע����б���:register_addr_delect��������ע��ĵ�ַ(��������ʱû�з�������2016/01/26)
bool terminal_delect_unregister_addr( uint16_t register_addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (register_addr_delect != 0xffff))
	{
		// Ѱ��ɾ���Ľڵ�
		int i = 0, delect_index;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->noregister_head;
		volatile uint16_t *p_trail = &p_regist_pro->noregister_trail;
		if( (*p_head > *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail > (SYSTEM_TMNL_MAX_NUM-1)) || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))&&(*p_head != 0)) )
		{
			DEBUG_INFO( "Err delect unregister address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
				*p_head, *p_trail, p_regist_pro->rgsted_trail );
			return false;
		}

		for( i = *p_head; i <= *p_trail; i++ )
		{
			if( p_regist_pro->register_pro_addr_list[i] == register_addr_delect )
			{
				delect_index = i;
				found_dl = true;
				break;
			}
		}

		if( found_dl )
		{
			// ������δע���б��ͷ�����ݽ���
			DEBUG_INFO( "save register addr = %04x ?=( (delect index = %d)list addr = %04x)-(swap addr = %04x)<<====>> %d(head_index)----%d(trail)---%d(rgsted_trail)", \
				register_addr_delect, delect_index,p_regist_pro->register_pro_addr_list[delect_index], p_regist_pro->register_pro_addr_list[*p_head],*p_head, *p_trail, p_regist_pro->rgsted_trail );
			if( *p_head > *p_trail )
				return false;
			else 
			{
				if( swap_valtile_uint16( &p_regist_pro->register_pro_addr_list[*p_head], &p_regist_pro->register_pro_addr_list[delect_index]) )
				{
					/*
					**1: �ƶ���ע���β��δע���ͷ
					**2:�ƶ�δע���ͷ��δע��ı�ͷ����һ��δע��Ԫ��
					*/
					p_regist_pro->rgsted_trail = *p_head;
					if( *p_head == *p_trail )
					{
						*p_trail = ++(*p_head);
					}
					else
					{
						++(*p_head);
					}
					
					return true;
				}
			}
		}
		
	}

	return false;
}

// ��δע���б������δע���ַ(δ���в���2016/01/26)
bool terminal_clear_from_unregister_addr_list( uint16_t unregister_addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (unregister_addr_delect != 0xffff))
	{
		// Ѱ��ɾ���Ľڵ�
		int i = 0, delect_index;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->noregister_head;
		volatile uint16_t *p_trail = &p_regist_pro->noregister_trail;
		if( (*p_head > *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail > (SYSTEM_TMNL_MAX_NUM-1)) ||  ((*p_head !=  (p_regist_pro->rgsted_trail + 1))&&(*p_head != 0)) )
		{
			DEBUG_INFO( "Err delect unregister address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
				*p_head, *p_trail, p_regist_pro->rgsted_trail );
			return false;
		}

		for( i = *p_head; i <= *p_trail; i++ )
		{
			if( p_regist_pro->register_pro_addr_list[i] == unregister_addr_delect )
			{
				delect_index = i;
				found_dl = true;
				break;
			}
		}

		if( found_dl )
		{
			// ������δע���б��ͷ�����ݽ���
			if( swap_valtile_uint16(&p_regist_pro->register_pro_addr_list[*p_trail], &p_regist_pro->register_pro_addr_list[delect_index]) )
			{
				/*
				**1: ֱ�Ӱ�β�ڵ���Ϊ�����õ�ַ0xffff,����βָ����ǰ��һԪ��
				*/
				p_regist_pro->register_pro_addr_list[(*p_trail)--] = 0xffff;
				DEBUG_INFO( "noregister list trail index = %d-trail  emlem value = %d", *p_trail, p_regist_pro->register_pro_addr_list[(*p_trail)] );

				return true;
			}
		}
		
	}

	return false;
}

// ɾ���ն���ע���ַ�����������δע��ĵ�ַ�б��ͷ��(δ���в���2016/01/26)
bool terminal_delect_register_addr( uint16_t addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (addr_delect != 0xffff))
	{
		// Ѱ��ɾ���Ľڵ�
		int i = 0, delect_index = -1;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->rgsted_head;
		volatile uint16_t *p_trail = &p_regist_pro->rgsted_trail;
		if( (*p_head >= *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail >= (SYSTEM_TMNL_MAX_NUM-1)) || (*p_head !=  (p_regist_pro->noregister_head - 1)) )
		{
			DEBUG_INFO( "Err delect register address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
				*p_head, *p_trail, p_regist_pro->rgsted_trail );
			return false;
		}

		for( i = *p_head; i <= *p_trail; i++ )
		{
			if( p_regist_pro->register_pro_addr_list[i] == addr_delect )
			{
				delect_index = i;
				found_dl = true;
				break;
			}
		}

		if( found_dl )
		{
			// ��������ע���б��β�����ݽ���
			if( swap_valtile_uint16(&p_regist_pro->register_pro_addr_list[*p_trail], &p_regist_pro->register_pro_addr_list[delect_index]) )
			{
				/*
				**1: �ƶ�δע���ͷ����ע���β
				**2: �ƶ���ע���β����ע��ı�β����һ����ע��Ԫ��
				*/
				p_regist_pro->noregister_head = *p_trail;
				(*p_trail)--;

				return true;
			}
		}
		
	}

	return false;
}

/*ע��*/
bool terminal_register( uint16_t address, uint8_t dev_type, tmnl_pdblist p_tmnl_station )
{
	bool bret = false;
	int i = 0;
	
	if( NULL == p_tmnl_station )
	{
#ifdef __DEBUG__
		assert( p_tmnl_station );
#else
		bret = false;
#endif
	}
	else
	{
		if( !p_tmnl_station->tmnl_dev.tmnl_status.is_rgst )
		{
			for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
			{
				if( (address & TMN_ADDR_MASK) == (tmnl_addr_list[i].addr))
				{
					DEBUG_INFO( "register addr = %04x-%04x, index = %d ", address & TMN_ADDR_MASK, tmnl_addr_list[i].addr, i );
					p_tmnl_station->tmnl_dev.tmnl_status.is_rgst = true;
					p_tmnl_station->tmnl_dev.tmnl_status.device_type = dev_type;
				        p_tmnl_station->tmnl_dev.address.addr = address & TMN_ADDR_MASK;
					p_tmnl_station->tmnl_dev.address.tmn_type = tmnl_addr_list[i].tmn_type;

					if( -1 == sort_terminal_dblist_node(dev_terminal_list_guard) )
					{
						DEBUG_INFO( "insert  register node is Err!................" );
					}
					show_terminal_dblist( dev_terminal_list_guard );
					
					// ������ע���ַ����������δע���б��е���Ӧ�ĵ�ַ
					if( terminal_delect_unregister_addr( p_tmnl_station->tmnl_dev.address.addr ) )// �˺����Ѳ��Գɹ�(2016/01/26)
					{
						gregister_tmnl_pro.tmn_rgsted++;
					}
					
					set_terminal_system_state( DISCUSS_STATE, true );
					bret = true;
					break;
				}
			}
		}
	}

	return bret;
}

/******************************************
*Auther:YasirLiang
*Date:2016/1/23
*Name:system_register_terminal_pro
*Func:register terminal proccess
*Paramer:
*		None
*******************************************/
void system_register_terminal_pro( void )
{
	volatile register_state reg_state =  gregister_tmnl_pro.rgs_state;
	if( reallot_flag )
	{// reallot time, can't register
		return;
	}

	if( reg_state == RGST_IDLE )
		return;
	else if( ((reg_state == RGST_WAIT) && over_time_listen(WAIT_TMN_RESTART)) )
	{
		over_time_stop(WAIT_TMN_RESTART);
		uint16_t tmn_total = gregister_tmnl_pro.tmn_total;
		if( ((gregister_tmnl_pro.rgsted_trail - gregister_tmnl_pro.rgsted_head) >= tmn_total) ||\
			(gregister_tmnl_pro.tmn_rgsted >= tmn_total) )
		{
			DEBUG_INFO( "terminal register complet!total = %d", tmn_total );
			gregister_tmnl_pro.rgs_state = RGST_QUERY;
			return;
		}
	
		// ����δע���б�ע��
		if( tmn_total < SYSTEM_TMNL_MAX_NUM )
		{
			uint16_t addr = 0, unregister_list_index = 0xffff;
			tmnl_pdblist register_node = NULL;
			for( unregister_list_index = gregister_tmnl_pro.noregister_head; \
				unregister_list_index <= gregister_tmnl_pro.noregister_trail;\
				unregister_list_index++ )
			{
				DEBUG_INFO( "noregister list index = %d", unregister_list_index );
				if( unregister_list_index < SYSTEM_TMNL_MAX_NUM )
				{	
					addr = gregister_tmnl_pro.register_pro_addr_list[unregister_list_index];

					DEBUG_INFO( "query address %04x----index = %d", addr, unregister_list_index );
					if( addr != 0xffff )
					{
						if( NULL == found_terminal_dblist_node_by_addr(addr) )
						{
							find_func_command_link( MENU_USE, MENU_TERMINAL_SYS_REGISTER, 0, (uint8_t*)&addr, sizeof(uint16_t) );
							over_time_set( QUERY_TMN_GAP, 500 ); // ע�����500ms
						}
						else
						{
							if( !register_node->tmnl_dev.tmnl_status.is_rgst )
							{
								find_func_command_link( MENU_USE, MENU_TERMINAL_SYS_REGISTER, 0, (uint8_t*)&addr, sizeof(uint16_t) );
								over_time_set( QUERY_TMN_GAP, 500 ); // ע�����500ms
							}
						}
					}
				}
				else
				{
					DEBUG_INFO( " Inval unregister_list_index = %d ", unregister_list_index );
					break;
				}
			}
		}
		
		gregister_tmnl_pro.rgs_state = RGST_QUERY;
		over_time_set( TRGST_OTIME_HANDLE, 3000 );
	}
	else if ( reg_state == RGST_QUERY )
	{
		if ( (gregister_tmnl_pro.tmn_rgsted == gregister_tmnl_pro.tmn_total) ||over_time_listen(TRGST_OTIME_HANDLE) )
		{
			DEBUG_INFO( "tmn_rgsted =%d, tmn_total = %d", gregister_tmnl_pro.tmn_rgsted, gregister_tmnl_pro.tmn_total );
			gregister_tmnl_pro.rgs_state = RGST_IDLE;
			over_time_stop( TRGST_OTIME_HANDLE );
#ifdef __DEBUG__
			int unregister_index = gregister_tmnl_pro.noregister_head;

			printf( "No Register List : (head index = %d)---(trail index = %d)\n\t", unregister_index, gregister_tmnl_pro.noregister_trail );
			for( ; unregister_index <= gregister_tmnl_pro.noregister_trail; unregister_index++ )
				fprintf( stdout, "%04x  ", gregister_tmnl_pro.register_pro_addr_list[unregister_index] );
			fprintf( stdout,"\n" );

			int register_index = gregister_tmnl_pro.rgsted_head;

			printf( "Register List : (head index = %d)---(trail index = %d)\n\t", register_index, gregister_tmnl_pro.rgsted_trail );
			for( ; register_index <= gregister_tmnl_pro.rgsted_trail; register_index++ )
			{
				fprintf( stdout, "%04x  ", gregister_tmnl_pro.register_pro_addr_list[register_index] );
			}
			fprintf( stdout,"\n" );
#endif
		}
	}
}

// ��ʼע�ắ��
void terminal_begin_register( void )
{
	gregister_tmnl_pro.rgs_state = RGST_WAIT;
	over_time_set( WAIT_TMN_RESTART, 500 );
}

void terminal_register_init( void )
{
	int i = 0;
	gregister_tmnl_pro.list_size = SYSTEM_TMNL_MAX_NUM;
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
		gregister_tmnl_pro.register_pro_addr_list[i] = 0xffff;

	gregister_tmnl_pro.tmn_rgsted = 0;
	gregister_tmnl_pro.tmn_total = 0;
	gregister_tmnl_pro.noregister_head = 0;
	gregister_tmnl_pro.noregister_trail = 0;
	gregister_tmnl_pro.rgsted_head = 0;
	gregister_tmnl_pro.rgsted_trail = 0;
	gregister_tmnl_pro.register_list_full = false;
	gregister_tmnl_pro.unregister_list_full = false;
	gregister_tmnl_pro.rgs_state = RGST_IDLE;
}
/*********************************************
=ע�ᴦ����غ�������
**********************************************/

void terminal_type_save( uint16_t address, uint8_t tmnl_type, bool is_chman )
{
	int i = 0;

	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( ( address != 0xffff) && (address == (tmnl_addr_list[i].addr)))
		{
			DEBUG_INFO( "terminal(--%04x--) save type = %d ", address, tmnl_type );

			if( (is_chman && ((tmnl_type == TMNL_TYPE_CHM_COMMON) ||(tmnl_type == TMNL_TYPE_CHM_EXCUTE)))\
				|| (!is_chman && ((tmnl_type == TMNL_TYPE_COMMON_RPRST) ||(tmnl_type == TMNL_TYPE_VIP))))
			{
				tmnl_pdblist tmnl_node = found_terminal_dblist_node_by_addr( address );
				if( tmnl_node !=NULL )
				{
					tmnl_node->tmnl_dev.address.tmn_type = tmnl_type;
				}

				tmnl_addr_list[i].tmn_type = tmnl_type;
				break;
			}	
		}
	}
}

void terminal_trasmint_message( uint16_t address, uint8_t *p_data, uint16_t msg_len )
{
	assert( p_data );
	if( p_data == NULL )
		return;
	
	upper_cmpt_terminal_message_report( p_data, msg_len, address );
}

uint16_t find_new_apply_addr( terminal_address_list_pro* p_gallot, terminal_address_list* p_gaddr_list, uint16_t* new_index)
{
	assert( p_gallot && p_gaddr_list && new_index );
	if( (p_gaddr_list == NULL) || (p_gallot == NULL) || (new_index == NULL))
		return 0xffff;
	
	uint16_t current_index = p_gallot->index;
	uint16_t temp_addr = 0;
	uint16_t i = 0;
	
	if( current_index >= SYSTEM_TMNL_MAX_NUM || NULL == new_index)
		return 0xffff;

	if( p_gaddr_list[current_index].addr == 0xffff )
	{
		temp_addr = p_gallot->addr_start + current_index;
		*new_index = current_index;
	}
	else
	{
		i = current_index + 1;
		temp_addr = p_gaddr_list[i].addr;
		do
		{
			i %= SYSTEM_TMNL_MAX_NUM;
			if( p_gaddr_list[i].addr == 0xffff)
			{
				break;
			}

			i++;
		}
		while( i != current_index);

		if( i != current_index)
		{
			temp_addr = p_gallot->addr_start + i;
			*new_index = i;
		}
	}

	DEBUG_INFO( "new addr = %04x",  temp_addr );
	return temp_addr;
}

/*==================================================
					��ʼ�ն������
====================================================*/
/***************************************************
**Writer:YasirLiang
**Date: 2015/10/26
**Name:terminal_func_allot_address
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference allot command data sended by terminal
******************************************************/
int terminal_func_allot_address( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg; 
	struct endstation_to_host_special spe_msg;
	terminal_address_list* p_addr_list = tmnl_addr_list;
	terminal_address_list_pro* p_allot = &allot_addr_pro;
	uint16_t new_addr = 0;
	uint16_t new_index = 0;
	uint8_t data_buf[DATAMAXLENGTH] = {0};
	uint16_t send_data_lng = 0;
	
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	
	// �����ַ����
	if( msg.cchdr.command_control & COMMAND_TMN_REPLY )
	{
		DEBUG_LINE();
		send_data_lng = 0;
		if( p_addr_list[p_allot->index].addr != 0xffff)
		{
			p_allot->renew_flag = 1;

			DEBUG_INFO( "man type = 0x%02x ", msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN );
			if( msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN )
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_CHM_EXCUTE;
				DEBUG_INFO( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}
			else
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_COMMON_RPRST;
				DEBUG_INFO( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}

			terminal_address_list tmp_addr;
			tmp_addr.addr = p_addr_list[p_allot->index].addr;
			tmp_addr.tmn_type = p_addr_list[p_allot->index].tmn_type;
			if( 1 == terminal_address_list_write_file( addr_file_fd, &tmp_addr, 1 ) )
			{// �������棬��ʼע���ն�,�������ն�����û��ע��Ľڵ�
				gregister_tmnl_pro.tmn_total++;
				if (terminal_register_pro_address_list_save( tmp_addr.addr, false ) )
				{
					terminal_begin_register();
				}
				else
				{
					DEBUG_INFO( "allot address register Not begin!new allot address = --0x%04x--", tmp_addr.addr );
				}
			}
		}
	}
	else
	{
		if( (msg.data == ADDRESS_ALREADY_ALLOT) && (p_allot->renew_flag) && (p_addr_list[p_allot->index].addr != 0xffff))
		{
			new_addr = p_addr_list[p_allot->index].addr;
		}
		else
		{
			new_addr = find_new_apply_addr( p_allot, p_addr_list, &new_index );
			p_allot->index = new_index;
			p_allot->renew_flag = 0;
			p_addr_list[p_allot->index].addr = new_addr;
		}

		data_buf[0] = (uint8_t)((new_addr & 0x00ff) >> 0);// ���ֽ���ǰ
		data_buf[1] = (uint8_t )((new_addr & 0xff00) >> 8);
		send_data_lng = sizeof(uint16_t);
	}

	host_reply_terminal( ALLOCATION, msg.cchdr.address, data_buf, send_data_lng );

	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_key_action
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference key_action command data sended by terminal
******************************************************/
int terminal_func_key_action( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	tmnl_pdblist cam_node = NULL;
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	uint8_t key_num = KEY_ACTION_KEY_NUM( msg.data );
	uint8_t key_value = KEY_ACTION_KEY_VALUE( msg.data );
	uint8_t tmnl_state = KEY_ACTION_STATE_VALUE( msg.data );
	uint8_t sys_state = get_sys_state();
	DEBUG_INFO( "key_num = %d, key_value = %d, tmnl_state = %d, sys_state = %d", key_num, key_value, tmnl_state, sys_state );

	if( (msg.cchdr.command_control & COMMAND_TMN_REPLY) == COMMAND_TMN_REPLY )
	{
		DEBUG_INFO( "key action command not valid!" );
		return -1;
	}
	
	switch( sys_state )
	{
		case SIGN_STATE:
		case VOTE_STATE:
		case GRADE_STATE:
		case ELECT_STATE:
			terminal_vote( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_key_speak( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case DISCUSS_STATE:
			terminal_key_discuccess( addr, key_num, key_value, tmnl_state, msg.data );
			terminal_chairman_apply_reply( msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN,addr, key_num, key_value, tmnl_state, msg.data);
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case INTERPOSE_STATE:
			terminal_key_action_chman_interpose( addr, key_num, key_value, tmnl_state, msg.data );
			break;
		case CAMERA_PRESET:
			cam_node = found_terminal_dblist_node_by_addr( addr );
			if( cam_node != NULL )
			{
				terminal_key_action_host_common_reply( msg.data, cam_node );
				terminal_key_preset( 0, addr, tmnl_state, key_num, key_value );
			}
			break;
		default:
			break;
	}
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_chairman_control
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference chairman control command data sended by terminal
******************************************************/
int terminal_func_chairman_control( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	uint8_t sign_value = 0;
	uint8_t chair_opt = msg.data&CHAIRMAN_CONTROL_MEET_MASK;
	uint8_t sign_flag = 0;
	tmnl_pdblist query_tmp = NULL;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		DEBUG_INFO( "not found chairman conntrol address!" );
		return -1;
	}

	if(  chair_opt != CHM_BEGIN_VOTE )
	{
		terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
	}

	switch( chair_opt )
	{
		case CHM_BEGIN_SIGN:
			gset_sys.sign_type = KEY_SIGN_IN;
			terminal_chman_control_start_sign_in( KEY_SIGN_IN, 10 );
			break;
		case CHM_END_SIGN:
			terminal_end_sign( 0, NULL, 0 );
			break;
		case CHM_BEGIN_VOTE:
			if( gtmnl_signstate ) // ϵͳ�Ѿ�ǩ��
			{
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
				terminal_chman_control_begin_vote( VOTE_MODE, false, &sign_flag );// ĩ�ΰ�����Ч
			}
			else
			{
				sign_value = 1; // δǩ��
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
			}
			break;
		case CHM_END_VOTE:
			terminal_end_vote( 0, NULL, 0 );

			// �����ն˵�״̬
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0; 
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST] );
			terminal_state_all_copy_from_common();

			// ����lcd
			terminal_lcd_display_num_send( BRDCST_ALL, LCD_OPTION_CLEAR, VOTE_INTERFACE );
			// ����led��
			terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
			fterminal_led_set_send( BRDCST_ALL );

			// ���ͱ�����
			if( msg.data&0x10 )	// �㲥������
			{
				terminal_broadcast_end_vote_result( BRDCST_ALL );
			}
			else
			{
				terminal_broadcast_end_vote_result( addr );
			}
			break;
		case CHM_SUSPEND_VOTE:
			terminal_pause_vote( 0, NULL, 0 );
			break;
		case CHM_RECOVER_VOTE:
			terminal_regain_vote( 0, NULL, 0 );
			break;
		case CHM_RETURN_DISCUSS:
			set_terminal_system_state( DISCUSS_STATE, true );
			terminal_start_discuss( false );
			break;
		case CHM_CLOSE_ALL_MIC:// �ر�������ͨ�����
			assert( dev_terminal_list_guard );
			if( dev_terminal_list_guard == NULL )
				return -1;
			
			for( query_tmp = dev_terminal_list_guard->next; query_tmp != dev_terminal_list_guard; query_tmp = query_tmp->next )
			{
				if( (query_tmp->tmnl_dev.address.addr != 0xffff) && \
					query_tmp->tmnl_dev.tmnl_status.is_rgst && \
					query_tmp->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST &&\
					(query_tmp->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) )
				{
					query_tmp->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
					connect_table_tarker_disconnect( query_tmp->tmnl_dev.entity_id, query_tmp, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
					trans_model_unit_disconnect( query_tmp->tmnl_dev.entity_id );
#endif
				}
			}
			
			terminal_mic_state_set( MIC_COLSE_STATUS, BRDCST_MEM,BRDCST_1722_ALL, false, NULL );
			cmpt_miscrophone_status_list();
			gdisc_flags.speak_limit_num = 0;
			gdisc_flags.apply_num = 0;
			gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
			terminal_main_state_send( 0, NULL, 0 );
			break;
		default:
			break;
	}

	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_send_main_state
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference send main state command data sended by terminal
******************************************************/
int terminal_func_send_main_state( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_main_state_send( 0, NULL, 0 );
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_func_cmd_event
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces conference special event command data sended by terminal
******************************************************/
int terminal_func_cmd_event( uint16_t cmd, void *data, uint32_t data_len )
{
	struct endstation_to_host msg;
	struct endstation_to_host_special spe_msg;
	conference_end_to_host_frame_read( data, &msg, &spe_msg, 0, sizeof(msg) );
	uint16_t addr = msg.cchdr.address & TMN_ADDR_MASK;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	/*reply termianl*/
	if( msg.cchdr.command_control & COMMAND_TMN_REPLY )
	{
		return -1;
	}

	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		return -1;
	}

	terminal_endstation_special_event_reply( tmp->tmnl_dev.entity_id, addr);
	
	uint8_t special_event = msg.data;
	if( special_event == SIGN_IN_SPECIAL_EVENT ) // �ն�ǩ��
	{
		terminal_sign_in_special_event( tmp );
		thost_sys_state sys_state = get_terminal_system_state();
		if( DISCUSS_STATE == sys_state.host_state )
		{
			uint8_t dis_mode = set_sys.discuss_mode;
			
			if( APPLY_MODE == dis_mode  && (tmp->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
			{
				terminal_chairman_apply_type_set( addr );
			}
			else
			{
				terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			}
		}
	}
	
	return 0;
}

/***************************************************
**Writer:YasirLiang
**Date: 2015/10/29
**Name:terminal_mic_auto_close
**Garam:
**		cmd: func cmd
**		data: proccess recv data
**		data_len: recv data length
**Func: procces mic_auto_close system set command
******************************************************/
int terminal_mic_auto_close( uint16_t cmd, void *data, uint32_t data_len )
{
	uint8_t auto_close = 0;
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;
	int i = 0;
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	/* �����ն˵���˷�״̬*/
	auto_close = set_sys.auto_close;
	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[i].auto_close = auto_close?1:0;
		gtmnl_state_opt[i].MicClose = MIC_CLOSE;
	}
	
	/*�ر�������˷磬������Ҫһ�����ƣ���ͨ�������������˷����û���(��ʱδʵ��10/29), ʹ�����ӱ����ϵͳ����˷������״̬����ʱδ����ͬ��������(11/4)*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
				connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id );
#endif
			
		}
	}

	/*��������״̬*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/*��������״̬, */
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len )
{
	tmnl_pdblist p_tmnl_list = NULL;	
	tmnl_main_state_send host_main_state;
	uint8_t spk_num = 0;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	host_main_state.unit = gregister_tmnl_pro.tmn_total;
	host_main_state.camera_follow = set_sys.camara_track ? 1 : 0;
	host_main_state.chm_first = set_sys.temp_close ? 1 : 0;
	host_main_state.conference_stype = (set_sys.discuss_mode&0x0f); // low 4bit
	host_main_state.limit = set_sys.speak_limit; 		// ������������
	host_main_state.apply_set = set_sys.apply_limit;	// ������������

	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;	
	
	for( p_tmnl_list = dev_terminal_list_guard->next;p_tmnl_list != dev_terminal_list_guard; p_tmnl_list = p_tmnl_list->next )
	{
		if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && (p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
			spk_num++;
	}
	
	host_main_state.spk_num = spk_num; // ��ǰ��������
	host_main_state.apply = gdisc_flags.apply_num;

	terminal_host_send_state( BRDCST_1722_ALL, host_main_state ); // target id is 0

	return 0;
}

/*�ն˷�����ʾ���ţ����Լ�������11/2*/
int terminal_lcd_display_num_send( uint16_t addr, uint8_t display_opt, uint8_t display_num )
{
	tmnl_send_end_lcd_display lcd_dis;
	lcd_dis.opt = display_opt;
	lcd_dis.num = display_num;

	if( display_opt == LCD_OPTION_DISPLAY )
	{
		glcd_num = display_num;
	}
	
	terminal_send_end_lcd_display( 0, addr, lcd_dis );
	
	return 0;
}

/*�ݶ�ͶƱ*/
int terminal_pause_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE );

	return 0;
}

/*����ͶƱ*/
int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE );

#ifdef __MIND_UPPER_CMPT_SIGN_RESULT__
	gvote_flag = VOTE_SET;
	over_time_set( MIND_UPPER_CMPT_SIGN_RESULT, 500 );// �����ϱ��ն�ǩ������ĳ�ʼ��ʱʱ��
#endif

	return 0;
}

int terminal_system_discuss_mode_set( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data && dev_terminal_list_guard );
	if( data == NULL || dev_terminal_list_guard == NULL )
		return -1;
	
	uint8_t dis_mode = *((uint8_t*)data);
	tmnl_pdblist tmnl_node = dev_terminal_list_guard->next;

	terminal_speak_track_pro_init();
	gdisc_flags.edis_mode = (ttmnl_discuss_mode)dis_mode;
	gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0;

	/*�ر�������˷�*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		// 2.������˷�tarker��״̬,�ϱ���˷�״̬, ������Ӧ�ն˵���˷�״̬(�����������ն�Э��)
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) )
		{
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
			connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id );
#endif
		}
	}

	/*��������״̬*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

int terminal_speak_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )// �������д�����(11/4)
{
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	uint8_t spk_limt_num = *((uint8_t*)data);
	uint8_t dis_mode = set_sys.discuss_mode;

	gdisc_flags.limit_num = spk_limt_num;

	return 0;
}

int terminal_apply_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )
{
	thost_system_set set_sys;
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	uint8_t apply_limt_num = *((uint8_t*)data);
	gdisc_flags.apply_limit = apply_limt_num;

	return 0;
}

/* �ն˷�����ʱ�Ŀ���,�����ն˲��ܿ�����˷�Ĺر���򿪣�
����ϵͳ�������ǻ���Э�����ݵķ��ͣ���������ն˵ķ���ʱ��;������ʱ����ϵͳ�ĳ�ʱ���ƽ��й���,
���ֻ�����ն�����ʱ�������ӱ���Ӧ�ĳ�ʱʱ�伴��*/
int terminal_limit_speak_time_set( uint16_t cmd, void *data, uint32_t data_len )
{
	tmnl_limit_spk_time spk_time;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ

	//DEBUG_INFO( "SIZE OF SYS SET = %d", sizeof(thost_system_set) );
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	spk_time.limit_time = set_sys.spk_limtime;
	speak_limit_time = (uint8_t)spk_time.limit_time;
	
	if( !set_sys.spk_limtime ) // ����ʱ
	{
		terminal_limit_spk_time( 0, BRDCST_ALL, spk_time );
	}
	else
	{
		uint16_t nolimit_addr = 0;
		uint16_t limit_addr = BRDCST_MEM; // ��ʱ��������ͨ������Ч
		tmnl_limit_spk_time nolimit_spk_time;
		
		if( set_sys.vip_limitime ) // vip ��ʱ
		{
			limit_addr |= BRDCST_VIP;
		}
		else
		{
			nolimit_addr |= BRDCST_VIP;
		}
		
		if( set_sys.chman_limitime ) // ��ϯ��ʱ
		{
			limit_addr |= BRDCST_CHM |BRDCST_EXE;
		}
		else
		{
			nolimit_addr |= BRDCST_CHM |BRDCST_EXE;
		}

		if( nolimit_addr )
		{
			nolimit_spk_time.limit_time = 0;
			terminal_limit_spk_time( BRDCST_1722_ALL, nolimit_addr, nolimit_spk_time );
		}

		terminal_limit_spk_time( BRDCST_1722_ALL, limit_addr, spk_time );
	}

	return 0;
}

int terminal_end_sign( uint16_t cmd, void *data, uint32_t data_len )
{
	set_terminal_system_state( DISCUSS_STATE, true );
	gtmnl_signstate = SIGN_IN_BE_LATE;

	// �������۵�״̬
	terminal_start_discuss( false );
	// ��ʼ��ǩ// ���ó�ʱǩ��
	over_time_set( SIGN_IN_LATE_HANDLE, gsign_latetime * 60 * 1000 );
	gquery_svote_pro.running = false;

	return 0;
}

int terminal_end_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	gvote_flag = NO_VOTE;// ����ͶƱ

	// ����ͶƱ����Ĳ�ѯ
	gquery_svote_pro.index = 0;
	gquery_svote_pro.running = false;
	host_timer_stop( &gquery_svote_pro.query_timer );

	return 0;
}

int termianal_music_enable( uint16_t cmd, void *data, uint32_t data_len )
{
	bool music_en = false;
	if( data_len != sizeof(uint8_t));
		return -1;

	music_en = *((uint8_t*)data)?true:false;
	gset_sys.chman_music = music_en;
}

int termianal_chairman_prior_set( uint16_t cmd, void *data, uint32_t data_len )
{
	bool prior_en = false;
	if( data_len != sizeof(uint8_t));
		return -1;

	prior_en = *((uint8_t*)data)?true:false;
	gset_sys.chman_first = prior_en;
}

/******************************************
*Auther:YasirLiang
*Date:2016/1/23
*Name:terminal_system_register
*Func:register terminal
*Paramer:
*		cmd;data;data_len
*******************************************/
int terminal_system_register( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data );
	if( data != NULL )
	{
		uint16_t address = *((uint16_t*)data);
		terminal_query_endstation( address, BRDCST_1722_ALL );
		
		return 0;
	}
	
	return -1;
}

/*==================================================
					�����ն������
====================================================*/


/*===================================================
{@�ն˴�������
=====================================================*/

int terminal_socroll_synch(void )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_LED_DISPLAY_ROLL_SYNC );

	return 0;
}

void terminal_remove_unregitster( void ) // ����û������ն˵�ַ�ļ��Լ��ڴ��ն��б�����Ӧ������
{
	tmnl_pdblist p_node = dev_terminal_list_guard->next;

	for( ; p_node != dev_terminal_list_guard; p_node = p_node->next )
	{
		if(  p_node->tmnl_dev.address.addr == 0xffff || !p_node->tmnl_dev.tmnl_status.is_rgst )
		{
			delect_terminal_dblist_node( &p_node );
		}
	}
}

// ���ϱ����Ҳ�����ϯ�廰���ű�����˷�״̬
void terminal_mic_state_set( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node )
{
	assert( tmnl_node );
	DEBUG_INFO( "===========mic state = %d ============",  mic_status );

	if( (tmnl_node == NULL) && !(addr & BROADCAST_FLAG) )
	{
		DEBUG_INFO( "nothing to send to set mic status!");
		return;
	}

	terminal_set_mic_status( mic_status, addr, tarker_id );
	upper_cmpt_report_mic_state( mic_status, tmnl_node->tmnl_dev.address.addr );
	if( (is_report_cmpt && (mic_status != MIC_CHM_INTERPOSE_STATUS) && tmnl_node != NULL) )
	{
		tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
	}
}

/*********************************************************
*writer:YasirLiang
*Date:2015/11/4
*Name:terminal_mic_speak_limit_time_manager_event
*Param:
*	none
*Retern Value:
*	-1,Err;
*	0,nomal.
*state:����ϵͳ�ն˵ķ���ʱ������ʱ�Ͽ����ӣ�
*	��λ�Ƿ���;��ʱ�������ۿ�ʼ; �ն���ʱ���Ե�ϵͳ��ʱ������ƵĽӿں���
***********************************************************/ 
int terminal_mic_speak_limit_time_manager_event( void )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	uint8_t system_state = get_sys_state();
	if(  system_state != DISCUSS_STATE/* || p_node == dev_terminal_list_guard */ )
	{
		return -1;
	}
	
	/*�鿴ϵͳ���ӱ�, ����鳬ʱ*/
#ifdef ENABLE_CONNECT_TABLE
	connect_table_timeouts_image();
#else
#endif

	return 0;
}

/* �����ն˿�ʼ���۵�״̬*/
int terminal_start_discuss( bool mic_flag )
{
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	if( !mic_flag ) // �ر�������˷�
	{	
		/*�ر�������˷�*/
		for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
		{
			//DEBUG_INFO( "=======>>>mic node id = %016llx, mic state = %d<<<=========", tmnl_node->tmnl_dev.entity_id, tmnl_node->tmnl_dev.tmnl_status.mic_state );
			if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) ) // �Ͽ�ע�����ӵ��ն�
			{
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id );
#endif
			}
		}
	}
	
	int i = 0;
	for( i = 0; i < TMNL_TYPE_NUM; i++ )
	{
		gtmnl_state_opt[i].auto_close = set_sys.auto_close?1:0;
		gtmnl_state_opt[i].keydown = 0;
		gtmnl_state_opt[i].keyup = 0;
		gtmnl_state_opt[i].MicClose = mic_flag?1:0;
		gtmnl_state_opt[i].sys = TMNL_SYS_STA_DISC; // ����ģʽ
	} 

	/* �����ն�״̬*/
	if( APPLY_MODE == set_sys.discuss_mode )
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// �����ն����������ն˵�״̬
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, LCD_OPTION_CLEAR, glcd_num );// ����lcd��ʾ����
		terminal_chairman_apply_type_set( BRDCST_EXE );
		
	}
	else
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE,gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// �����ն����������ն˵�״̬
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE, LCD_OPTION_CLEAR, glcd_num );// ����lcd��ʾ����
	}
	
	/*�����ն�ָʾ��*/
	terminal_led_set_save( BRDCST_ALL, TLED_KEY1, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY5, TLED_OFF );
	fterminal_led_set_send( BRDCST_ALL );
	
	terminal_main_state_send( 0, NULL, 0 );
	
	return 0;
}

/*�������������ն˵�״̬*/
void terminal_state_set_base_type( uint16_t addr, tmnl_state_set tmnl_state )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_state_set( tmnl_state, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_state_set( tmnl_state, addr, tmp->tmnl_dev.entity_id );
	}
	
}

/*��ϯ���������ն�״̬����*/
void terminal_chairman_apply_type_set( uint16_t addr )
{
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0x0e; // 2 3 4��������Ч
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

void terminal_chairman_apply_type_clear( uint16_t addr )
{
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0; 
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

/*����ledָʾ�Ƶ�״̬*/
bool terminal_led_set_save( uint16_t addr, uint8_t led_id, uint8_t  led_state )
{
	uint8_t byte_num,shifting;

	if( 0xFFFF == addr )
	{
		return false;
	}
	
	if(led_id>=TLED_MAX_NUM)
	{
		return false;
	}

	byte_num = led_id/4;
	shifting = (led_id%4)*2;
	gled_buf[byte_num]&=(~(0x03<<shifting));
	gled_buf[byte_num]|=(led_state<<shifting);

	return true;
}

/*����ledָʾ�Ƶ�״̬*/ 
void fterminal_led_set_send( uint16_t addr )
{
	ttmnl_led_lamp led_lamp;
	led_lamp.data_low = gled_buf[0];
	led_lamp.data_high = gled_buf[1];

	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_set_indicator_lamp( led_lamp, addr, BRDCST_1722_ALL );
	}
	else
	{
		terminal_set_indicator_lamp( led_lamp, addr, tmp->tmnl_dev.entity_id  );
	}
}

/*������λ������˷�Ĳ�������(����ر�)*/
int terminal_upper_computer_speak_proccess( tcmpt_data_mic_switch mic_flag )
{
	uint16_t addr = (((uint16_t)(mic_flag.addr.low_addr << 0)) | ((uint16_t)(mic_flag.addr.high_addr << 0)));
	uint8_t mic_state_set = mic_flag.switch_flag;
	ttmnl_discuss_mode dis_mode = gdisc_flags.edis_mode;
	tmnl_pdblist speak_node = NULL;
	uint8_t limit_time = 0;
	bool found_node = false;
	bool read_success = false;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	read_success = true;
	limit_time = set_sys.spk_limtime;
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	DEBUG_INFO( " speak addr = %04x, discuccess mode = %d", addr, dis_mode );
	for( speak_node = dev_terminal_list_guard->next; speak_node != dev_terminal_list_guard; speak_node = speak_node->next )
	{
		if( speak_node->tmnl_dev.address.addr == addr && speak_node->tmnl_dev.tmnl_status.is_rgst )
		{
			found_node = true;
			break;
		}
	}

	if( read_success && found_node )
	{
		if( mic_state_set &&  dis_mode != APPLY_MODE )
		{
			//found_connect_table_available_connect_node( speak_node->tmnl_dev.entity_id );
		}
		
		if( dis_mode == PPT_MODE ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
		{
			if( mic_state_set )
			{
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			}
			else
			{
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
			}
		}
		else
		{
			switch( dis_mode )
			{
				case FREE_MODE:
				{
					terminal_free_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case LIMIT_MODE:
				{
					terminal_limit_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case FIFO_MODE:
				{
					terminal_fifo_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				case APPLY_MODE:
				{
					terminal_apply_disccuss_mode_cmpt_pro( mic_state_set, limit_time, speak_node );
					break;
				}
				default:
				{
					DEBUG_INFO( " out of discuss mode bound!" );
					break;
				}
			}
		}
	}
	else if( !found_node)
	{
		return -1;
	}
	
	return 0;
}

//  file STSTEM_SET_STUTUS_PROFILE must be close,before use this function
bool terminal_read_profile_file( thost_system_set *set_sys )
{
	FILE* fd = NULL;
	thost_system_set tmp_set_sys;
	int ret = false;
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // ֻ����������
	if( NULL == fd )
	{
		DEBUG_INFO( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return false;
	}

	ret = true;
	memset( &tmp_set_sys, 0, sizeof( thost_system_set));
	if( profile_system_file_read( fd, &tmp_set_sys ) == -1)
	{
		DEBUG_INFO( "Read profile system Err!" );
		ret = false;
	}

	if( ret )
	{
		assert( set_sys );
		if( set_sys != NULL )
		{
			memcpy( set_sys, &tmp_set_sys, sizeof(thost_system_set));
		}
		else
		{
			ret = false;
		}	
	}
			
	Fclose( fd );
	return ret;
}

void terminal_free_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return;
	}
	
	if( mic_flag ) // ����˷�
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
		}
	}
	else
	{
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // ��λ���뷢��
	
	if( speak_node == NULL )
	{
		return ret;
	}

	if( mic_flag ) // ����˷�
	{
		if( speak_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num ) // ����˷�
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			ret = true;
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // ���뷢��
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else
	{
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );

			if( gdisc_flags.apply_num > 0 && current_addr == addr )// ����һ������Ϊ��λ����״̬
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else if( cc_state == MIC_OPEN_STATUS )
		{
			if( gdisc_flags.speak_limit_num > 0 )
			{
				gdisc_flags.speak_limit_num--;
			}

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// ��������,����ʼ��һ�������ն˵ķ���
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// ������һ�����뻰Ͳ
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
#ifdef ENABLE_CONNECT_TABLE
						connect_table_tarker_connect( first_speak->tmnl_dev.entity_id, limit_time, first_speak, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
						trans_model_unit_connect( first_speak->tmnl_dev.entity_id );
#endif

						if( gdisc_flags.apply_num > 0 ) // ������λ���뷢���ն�
						{
							gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
							first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
							if( first_apply != NULL )
							{
								terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
							}
							else
							{
								DEBUG_INFO( " no such tmnl dblist node!");
							}
						}
						else
						{
							gdisc_flags.currect_first_index = gdisc_flags.apply_num;
						}
					}
					else
					{
						DEBUG_INFO( " no such tmnl dblist node!");
					}
				}
				else
				{
					gdisc_flags.currect_first_index = 0;
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}

	return ret;
}

bool terminal_fifo_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	if( speak_node == NULL )
	{
		DEBUG_INFO( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	if( mic_flag ) // �򿪻�Ͳ
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			ret = true;
		}
		else if( speak_limit_num < gdisc_flags.limit_num )
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			gdisc_flags.speak_addr_list[speak_limit_num] = addr;
			ret = true;
		}
		else // �����������ڻ������������
		{
			if( gdisc_flags.speak_addr_list[0] != 0xffff ) // �Ƚ��ȳ�
			{
				tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
				if( first_speak != NULL )
				{
#ifdef ENABLE_CONNECT_TABLE
					connect_table_tarker_disconnect( first_speak->tmnl_dev.entity_id, first_speak, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
					trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id );
#endif
				}
				else
				{
					DEBUG_INFO( "fifo not found tmnl list node!");
				}
				
				addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// ��λ����ɾ��

				uint8_t speak_limit_num1 = gdisc_flags.speak_limit_num;
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
				gdisc_flags.speak_addr_list[speak_limit_num1] = speak_node->tmnl_dev.address.addr;
				ret = true;
			}
		}
	}
	else
	{
		addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
		ret = true;
	}

	return ret;
}

bool terminal_apply_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( mic_flag ) // ���뷢��,�ӵ�ַ�������б�
	{
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else // ȡ�����뷢��
	{
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// ����һ������Ϊ��λ����״̬
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
		}
		
		ret = true;
	}

	return ret;
}

bool addr_queue_delete_by_index( uint16_t *addr_queue, uint8_t *queue_len, uint8_t index )
{
	uint8_t i;
	
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	if((index >= *queue_len)||(0==*queue_len))
	{
		return false;
	}

	for(i = index; i<(*queue_len-1); i++)
	{
		*(addr_queue+i)=*(addr_queue+i+1);
	}
	
	*(addr_queue+*queue_len-1)=0xFFFF;
	(*queue_len)--;

	return true;
}

bool addr_queue_delect_by_value( uint16_t *addr_queue, uint8_t *queue_len, uint16_t value)
{
	uint16_t   i;
	uint16_t  index;
	if((NULL==addr_queue)||(NULL==queue_len))
	{
		return false;
	}
	
	if( 0 == *queue_len )
	{
		return false;
	}
	
	for(i=0; i<*queue_len; i++)
	{
		if(*(addr_queue+i)==value)
		{
			break;
		}
	}
	
	index = i;
	if(index<*queue_len)
	{
		for(i=index; i<(*queue_len-1); i++)
		{
			*(addr_queue+i)=*(addr_queue+i+1);
		}
		
		*(addr_queue+*queue_len-1)=0xFFFF;
		(*queue_len)--;
		
		return true;
	}
	
	return false;
}

bool addr_queue_find_by_value( uint16_t *addr_queue, uint8_t queue_len, uint16_t value, uint8_t *index)
{
	uint8_t i;
	
	if((NULL==addr_queue)||(0==queue_len))
	{
		return false;
	}
	
	for(i=0; i<queue_len; i++)
	{
		if(value == *(addr_queue+i))
		{
			if(index != NULL)
			{
				*index = i;
			}
			
			return true;
		}
	}
	
	return false;
}

tmnl_pdblist found_terminal_dblist_node_by_addr( uint16_t addr )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return NULL;
	
	tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
	for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
	{
		if( tmp_node->tmnl_dev.address.addr == addr )
			return tmp_node;
	}

	return NULL;
}

void terminal_select_apply( uint16_t addr ) // ʹѡ�������������λ������
{
	tmnl_pdblist apply_first = NULL; 
	uint8_t apply_index = MAX_LIMIT_APPLY_NUM;
	int i = 0;
	
	if( apply_first == NULL )
	{
		return;
	}
	
	if( addr != gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]) // ������λ����
	{
		apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
		terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );

		for( i = 0; i < gdisc_flags.apply_num; i++ )
		{
			if( addr == gdisc_flags.apply_addr_list[i])
			{
				apply_index = i;
				break;
			}
		}

		if( apply_index < gdisc_flags.apply_num )
		{
			apply_first = found_terminal_dblist_node_by_addr( addr );
			terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
		}
	}
}

bool terminal_examine_apply( enum_apply_pro apply_value )// be tested in 02-3-2016,passed
{
	uint16_t addr = 0;
	tmnl_pdblist apply_first = NULL;
	bool ret = false;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	if((gdisc_flags.edis_mode != APPLY_MODE) && (gdisc_flags.edis_mode != LIMIT_MODE))
	{
		return false;
	}

	switch( apply_value )
	{
		case REFUSE_APPLY:// be tested in 02-3-2016,passed
			addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
			if( addr_queue_delect_by_value(gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr) )
			{// delect success
				apply_first = found_terminal_dblist_node_by_addr( addr );
				if( apply_first != NULL )
				{
					//DEBUG_INFO( "set apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
					terminal_mic_state_set( MIC_COLSE_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					if( gdisc_flags.apply_num > 0 )
					{
						gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
						apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
						if( apply_first != NULL )
						{
							//DEBUG_INFO( "set FIRST apply addr = 0x%04x", apply_first->tmnl_dev.address.addr );
							terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
						}
					}
					else 
					{
						gdisc_flags.currect_first_index = MAX_LIMIT_APPLY_NUM;
					}
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			else
			{
				DEBUG_INFO( "delect apply add = 0x%04x failed: no such address", addr );
			}
			break;
		case NEXT_APPLY: // be tested in 02-3-2016,passed
			if( gdisc_flags.apply_num > 1 )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr );
				if( apply_first != NULL )
				{
					//DEBUG_INFO( "set apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
					terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );

					gdisc_flags.currect_first_index++;
					gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
					addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
					apply_first = found_terminal_dblist_node_by_addr( addr );
					if( apply_first != NULL )
					{
						//DEBUG_INFO( "set FIRST apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
						terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					}
					else
					{
						DEBUG_INFO( "no found first apply node!" );
					}
				}
				else
				{
					DEBUG_INFO( "no found first apply node!" );
				}

				//terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		case APPROVE_APPLY:// be tested in 02-3-2016,passed
			if( gdisc_flags.currect_first_index < gdisc_flags.apply_num )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr ); // �򿪵�һ���������˷�
				if( apply_first != NULL )
				{
#ifdef ENABLE_CONNECT_TABLE
					connect_table_tarker_connect( apply_first->tmnl_dev.entity_id, set_sys.spk_limtime, apply_first, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
					trans_model_unit_connect( apply_first->tmnl_dev.entity_id );
#endif
				}

				addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index );
				if( gdisc_flags.apply_num > 0 )
				{
					gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
					apply_first = found_terminal_dblist_node_by_addr(gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
					if( apply_first != NULL )
					{
						terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					}
				}
				else
				{
					gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		default:
			break;
	}

	return ret;
}

void terminal_type_set( tcmpt_data_meeting_authority tmnl_type )
{
	uint16_t addr = ((uint16_t)tmnl_type.addr.high_addr << 8)|((uint16_t)tmnl_type.addr.low_addr << 0 );
	tmnl_pdblist tmnl_node = NULL;
	
	DEBUG_INFO( "terminal type set addr = 0x%04x ", addr );
	tmnl_node = found_terminal_dblist_node_by_addr( addr );
	if( tmnl_node == NULL )
	{
		DEBUG_INFO( "no such type addr found!");
		return;
	}
	
	if( (tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON ||\
		tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE) &&\
		(tmnl_type.identity == TMNL_TYPE_CHM_COMMON ||\
		tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE) )
	{
		tmnl_node->tmnl_dev.address.tmn_type = tmnl_type.identity;
		terminal_chairman_excute_set( tmnl_node->tmnl_dev.address.addr,(tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE)?true:false);
		if( tmnl_type.identity == TMNL_TYPE_CHM_EXCUTE )
		{
			terminal_chairman_apply_type_set( addr );
		}
		else
		{
			terminal_chairman_apply_type_clear( addr );
		}
	}
	else if(tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST ||\
		tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP)
	{
		if( tmnl_type.identity == TMNL_TYPE_VIP )
		{
			tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_VIP;
		}
		else
		{
			tmnl_node->tmnl_dev.address.tmn_type = TMNL_TYPE_COMMON_RPRST;
		}

		terminal_vip_type_set( tmnl_node->tmnl_dev.address.addr, (tmnl_type.identity == TMNL_TYPE_VIP)?true:false );
	}

	// ���浽��ַ�ļ�address.dat
	int i = 0;
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( tmnl_addr_list[i].addr == addr )
		{
			tmnl_addr_list[i].tmn_type = tmnl_type.identity;
			break;
		}
	}
	
	terminal_type_save_to_address_profile( addr, tmnl_node->tmnl_dev.address.tmn_type );
}

void terminal_chairman_excute_set( uint16_t addr, bool is_set_excute )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr, \
			is_set_excute ? OPT_TMNL_SET_EXCUTE_CHM : OPT_TMNL_CANCEL_EXCUTE_CHM );
	}
}

void terminal_vip_type_set( uint16_t addr, bool is_set_vip )
{
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
	else
	{
		terminal_option_endpoint( tmp->tmnl_dev.entity_id, addr,\
			is_set_vip ? OPT_TMNL_SET_VIP : OPT_TMNL_CANCEL_VIP );
	}
}

int terminal_type_save_to_address_profile( uint16_t addr, uint16_t tmnl_type )
{
	FILE *fd = NULL;
	terminal_address_list addr_list[SYSTEM_TMNL_MAX_NUM];
	int i = 0;
	int index = 0;
	uint16_t type_tmp;

	fd = Fopen( ADDRESS_FILE, "ab+" ); // 
	if( fd == NULL )
	{
		DEBUG_ERR( "addr file open Err: %s", ADDRESS_FILE );
		return -1;
	}

	memset( addr_list, 0xff, sizeof(terminal_address_list)*SYSTEM_TMNL_MAX_NUM );
	if( terminal_address_list_read_file( fd, addr_list ) == -1 ) // ��ȡʧ��
	{
		DEBUG_INFO( "type save read file Err!");
		return -1;
	}

	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( addr_list[i].addr == addr )
		{
			type_tmp = addr_list[i].tmn_type;
			addr_list[i].tmn_type = tmnl_type;
			index = i;
			break;
		}
	}

	if( i >= SYSTEM_TMNL_MAX_NUM )
	{
		DEBUG_INFO( "not found addr in the address profile !" );
		return -1;
	}

	if( Fseek( fd, 2, SEEK_SET ) == -1 )
	{
		return -1;
	}

	// ��д����
	uint16_t crc;
	Fread( fd, &crc, sizeof(uint16_t), 1 );
	crc -= type_tmp; // ��ԭ��������
	crc += tmnl_type; // ��У��
	
	if( Fseek( fd, 2, SEEK_SET ) == -1 ) // �����ƫ��
	{
		return -1;
	}
	Fwrite( fd, &crc, sizeof(uint16_t), 1 );
	
	// д����
	if( Fseek( fd, index*sizeof(terminal_address_list) + 4/*�ļ�ͷ��С*/, SEEK_SET ) == -1 )
	{
		return -1;
	}

	if( Fwrite( fd, &tmnl_type, sizeof( uint16_t), 1 ) != 1)
	{
		DEBUG_INFO( "write terminal type Err!" );
		return -1;
	}

	Fclose( fd );
	return 0;
}

void terminal_send_upper_message( uint8_t *data_msg, uint16_t addr, uint16_t msg_len )
{
	assert( data_msg );
	if( data_msg == NULL )
	{
		return;
	}

	if( msg_len > MAX_UPPER_MSG_LEN )
	{
		return;
	}

	//DEBUG_INFO( "=====>>>>>>>>>>>> messsag target addr = %04x<<<<<<<<<<<<<<==========", addr );
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_transmit_upper_cmpt_message( BRDCST_1722_ALL, addr, data_msg, msg_len );
	}
	else
	{
		terminal_transmit_upper_cmpt_message( tmp->tmnl_dev.entity_id, addr, data_msg, msg_len );
	}
	
}

void terminal_tablet_stands_manager( tcmpt_table_card *table_card, uint16_t addr, uint16_t contex_len )// ���ƹ���
{
	assert( table_card );
	if( table_card == NULL )
		return;
	
	uint8_t card_flag = table_card->msg_type;
	tmnl_led_state_show_set card_opt;

	if( card_flag == 0 )
	{
		terminal_socroll_synch();
	}
	else if( card_flag == 1 )// ����led��ʾ��ʽ
	{
		memcpy( &card_opt, table_card->msg_buf, sizeof(uint16_t));

		tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
		if( tmp == NULL )
		{
			terminal_set_led_play_stype( BRDCST_1722_ALL, addr, card_opt );// ����led��ʾ��ʽ
		}
		else
		{
			terminal_set_led_play_stype( tmp->tmnl_dev.entity_id, addr, card_opt );
		}
	}
}

// ��ʼǩ��
void terminal_start_sign_in( tcmpt_begin_sign sign_flag )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	uint8_t sign_type = sign_flag.sign_type;
	uint8_t timeouts = sign_flag.retroactive_timeouts;
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1�忨��0����
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
	
	// ���ò�ѯǩ��ͶƱ���(2016-1-27���)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

// ��ϯ�����ն�ǩ��
void terminal_chman_control_start_sign_in( uint8_t sign_type, uint8_t timeouts )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int i = 0;

	set_terminal_system_state( SIGN_STATE, true );
	gtmnl_signstate = SIGN_IN_ON_TIME;
	gsign_latetime = timeouts;
	gsigned_flag = true;

	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && tmp->tmnl_dev.tmnl_status.is_rgst )
		{
			tmp->tmnl_dev.tmnl_status.sign_state = TMNL_NO_SIGN_IN;
		}
	}

	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SIGN;
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1�忨��0����
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);

	// ���ò�ѯǩ��ͶƱ���(2016-1-28���)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

void terminal_begin_vote( tcmp_vote_start vote_start_flag,  uint8_t* sign_flag )
{
	uint8_t vote_type = vote_start_flag.vote_type;
	tmnl_pdblist tmp = NULL;

	assert( sign_flag );
	if( sign_flag == NULL )
		return;
	
	*sign_flag = gsigned_flag ? 0 : 1;// ��Ӧ0������;��Ӧ1�쳣 
	gfirst_key_flag = vote_start_flag.key_effective?true:false;
	
	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	for( tmp = dev_terminal_list_guard->next ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst && (tmp->tmnl_dev.address.addr != 0xffff))
		{
			if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// ��ǩ��
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
			}
			else
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // δǩ������ͶƱ
			}
		}
	}

	terminal_vote_state_set( BRDCST_ALL );

	// ���ò�ѯǩ��ͶƱ��� (2016-1-27)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
	
#ifdef __MIND_UPPER_CMPT_SIGN_RESULT__
	over_time_set( MIND_UPPER_CMPT_SIGN_RESULT, 500 );// �����ϱ��ն�ǩ������ĳ�ʼ��ʱʱ��
#endif
}

void terminal_chman_control_begin_vote(  uint8_t vote_type, bool key_effective, uint8_t* sign_flag )
{
	tmnl_pdblist tmp = NULL;
	
	assert( sign_flag );
	if( sign_flag == NULL )
		return;
	
	gfirst_key_flag = key_effective; // true = �״ΰ�����Ч��
	*sign_flag = gsigned_flag; 

	gvote_mode = (tevote_type)vote_type;
	if( vote_type ==  VOTE_MODE )
	{
		set_terminal_system_state( VOTE_STATE, true );
	}
	else if( vote_type ==  GRADE_MODE )
	{
		set_terminal_system_state( GRADE_STATE, true );
	}
	else
	{
		set_terminal_system_state( ELECT_STATE, true );
	}

	gvote_flag = VOTE_SET;
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
	{
		return;
	}
	
	for( tmp = dev_terminal_list_guard->next; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.tmnl_status.is_rgst || tmp->tmnl_dev.address.addr )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// ��ǩ��
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
		}
		else
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // δǩ������ͶƱ
		}
	}

	terminal_vote_state_set( BRDCST_ALL );
}

void terminal_vote_state_set( uint16_t addr )
{
	tevote_type vote_type = gvote_mode;
	if( addr == 0xffff )
		return;

	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].one_off = gfirst_key_flag ? 1 : 0;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].VoteType = vote_type;
	gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].MicClose = 0;
	switch( vote_type )
	{
		case VOTE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0e; // 2 3 4 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, VOTE_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case GRADE_MODE:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_GRADE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, GRADE_1_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		case SLCT_2_1:
		case	SLCT_2_2:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x03;// 1 2 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_2_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_3_1:
		case SLCT_3_2:
		case SLCT_3_3:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x07;// 1 2 3 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_3_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_OFF );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_4_1:
		case SLCT_4_2:
		case SLCT_4_3:
		case SLCT_4_4:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0f; // 1 2 3 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_SELECT;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_4_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
			fterminal_led_set_send( addr );
			break;
		case SLCT_5_1:
		case SLCT_5_2:
		case SLCT_5_3:
		case SLCT_5_4:
		case SLCT_5_5:
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 ��
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sys = TMNL_SYS_STA_VOTE;
			terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
			terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, SLCT_LV_5_INTERFACE );
			terminal_led_set_save( addr, TLED_KEY1, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY2, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY3, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY4, TLED_ON );
			terminal_led_set_save( addr, TLED_KEY5, TLED_ON );
			fterminal_led_set_send( addr );
			break;
		default:
			DEBUG_INFO( "out of bround : vote of type!");
			break;
	}

	terminal_state_all_copy_from_common();
}

void terminal_sign_in_special_event( tmnl_pdblist sign_node ) // �ն������¼�-ǩ��
{
	assert( sign_node );
	if( sign_node == NULL )
		return;
	
	if( gtmnl_signstate == SIGN_IN_ON_TIME )// ����ǩ����־
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
	}
	else if( (gtmnl_signstate == SIGN_IN_BE_LATE) && (sign_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN) )
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
	}
	
	// ����ͶƱʹ��
	termianl_vote_enable_func_handle( sign_node );
	// �ϱ�ǩ�����
	DEBUG_INFO( "sign state = %d", sign_node->tmnl_dev.tmnl_status.sign_state );
	upper_cmpt_report_sign_in_state( sign_node->tmnl_dev.tmnl_status.sign_state, sign_node->tmnl_dev.address.addr );

	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = dev_terminal_list_guard->next;
	int sign_num = 0;
	for( ; tmp != dev_terminal_list_guard; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			tmp->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN )
		{
			break;
		}
		
		sign_num++;
	}

	DEBUG_INFO( "sign num = %d", sign_num );
	if( sign_num >= SYSTEM_TMNL_MAX_NUM )// all sign in
	{
		gtmnl_signstate = SIGN_IN_OVER;
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_SIGN );
	}
}

void termianl_vote_enable_func_handle( tmnl_pdblist sign_node )
{
	assert( sign_node );
	if( sign_node == NULL )
		return;
	
	sign_node->tmnl_dev.tmnl_status.vote_state |= TVOTE_EN; // TVOTE_SET_FLAG ->TVOTE_EN ->TWAIT_VOTE_FLAG(ͶƱ״̬����)
}

void terminal_state_all_copy_from_common( void )
{
	memcpy( &gtmnl_state_opt[TMNL_TYPE_VIP], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_COMMON], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
}

void terminal_broadcast_end_vote_result( uint16_t addr ) // �����ն˵�2 3 4��ͳ�ƽ��
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = NULL, head_list = dev_terminal_list_guard;
	uint16_t vote_total = 0, neg = 0, abs = 0, aff = 0;
	tmnl_vote_result vote_rslt;

	for( tmp = head_list; tmp != head_list; tmp = tmp->next )
	{
		if( (tmp->tmnl_dev.address.addr == 0xffff) || (!tmp->tmnl_dev.tmnl_status.is_rgst) )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )
		{
			vote_total++;
		}

		if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY2_ENABLE )// 2������ ,����
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY3_ENABLE )// 3������ ,��Ȩ
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY4_ENABLE )// 4������ ,�޳�
		{
			neg++;
		}
	}

	vote_rslt.total = vote_total;
	vote_rslt.neg = neg;
	vote_rslt.abs = abs;
	vote_rslt.aff = aff;

	if( (tmp = found_terminal_dblist_node_by_addr( addr )) != NULL )
	{
		terminal_send_vote_result( tmp->tmnl_dev.entity_id, addr, vote_rslt );
	}
	else
	{
		terminal_send_vote_result( BRDCST_1722_ALL, addr, vote_rslt );
	}
}

void terminal_vote( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp_node = NULL, tmp_head = dev_terminal_list_guard;
	if( gvote_flag == NO_VOTE || ( key_num > 5 ) ) // ��Э��(2.	�ն˰����ı�ţ������1~5�����Լ�6����ϯ���ȼ�7)
	{
		DEBUG_INFO( "system not ready to vote or key num out of vote key num!" );
		return;
	}

	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such tmp_node: addr = %04x", addr );
		return;
	}

	// ����keyֵ
	if( terminal_key_action_value_judge_can_save( key_num,  tmp_node ) ) 
	{// ������Ӧ2
		terminal_key_action_host_special_num2_reply( recvdata, 0, 0, 0, VOID_VOTE_INTERFACE ,tmp_node );
	}
	else
	{
		terminal_key_action_host_common_reply( recvdata, tmp_node );
	}

	upper_cmpt_vote_situation_report( tmp_node->tmnl_dev.tmnl_status.vote_state, tmp_node->tmnl_dev.address.addr );

	// ����Ƿ�����ͶƱ���
	tmnl_pdblist tmp = tmp_head;
	int vote_num = 0;
	for( ; tmp != tmp_head; tmp = tmp->next )
	{
		if( tmp->tmnl_dev.address.addr != 0xffff && \
			tmp->tmnl_dev.tmnl_status.is_rgst && \
			(tmp->tmnl_dev.tmnl_status.mic_state & TWAIT_VOTE_FLAG ) )
		{
			break;
		}
		
		vote_num++;
	}

	DEBUG_INFO( "vote num = %d", vote_num );
	if( vote_num >= SYSTEM_TMNL_MAX_NUM )
	{
		terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_VOTE );
	}
}

bool terminal_key_action_value_judge_can_save( uint8_t key_num,  tmnl_pdblist vote_node )
{
	bool ret = false;
	uint8_t *p_vote_state = NULL;
	
	assert( vote_node );
	if( NULL == p_vote_state )
		return false;
	
	p_vote_state = &vote_node->tmnl_dev.tmnl_status.vote_state;
	assert( p_vote_state );
	if( NULL == p_vote_state )
		return false;

	if( !gfirst_key_flag )	// last key effective 
	{
		*p_vote_state &= (~TVOTE_KEY_MARK);
		*p_vote_state |= (1<< ( key_num -1));
		return ret;
	}

	switch(gvote_mode)
	{
		case VOTE_MODE:
		case GRADE_MODE:
		case SLCT_2_1:
		case SLCT_3_1:
		case SLCT_4_1:
		case SLCT_5_1:
			*p_vote_state |= (1<< ( key_num -1));
			*p_vote_state &= (~TWAIT_VOTE_FLAG);// ���ý�����־
			ret = true;
			break;
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 2 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// ���ý�����־
				ret = true;
			}
			break;
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 3 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// ���ý�����־
				ret = true;
			}
			break;
		case SLCT_4_4:
		case SLCT_5_4:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 4 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// ���ý�����־
				ret = true;
			}
			break;
		case SLCT_5_5:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 5 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// ���ý�����־
				ret = true;
			}
			break;
		default:
			break;
	}

	return ret;
}

void terminal_key_action_host_special_num2_reply( const uint8_t recvdata, uint8_t key_down, uint8_t key_up, uint16_t key_led, uint8_t lcd_num, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_special2_reply reply_data;
	reply_data.recv_data = recvdata;
	reply_data.reply_num = REPLY_SPECAIL_NUM2;
	reply_data.key_down = key_down & TVOTE_KEY_MARK;
	reply_data.key_up = key_up & TVOTE_KEY_MARK;
	reply_data.sys = recvdata & KEY_ACTION_TMN_STATE_MASK;
	reply_data.key_led = key_led & 0x03ff;// ��ʮλ
	reply_data.lcd_num = lcd_num;
	data_len = SPECIAL2_REPLY_KEY_AC_DATA_LEN;

	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, NULL, &reply_data );
}

void terminal_key_action_host_special_num1_reply( const uint8_t recvdata, uint8_t mic_state, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_special1_reply reply_data;
	reply_data.mic_state = mic_state;
	reply_data.reply_num = REPLY_SPECAIL_NUM1;
	reply_data.recv_data = recvdata;
	data_len = SPECIAL1_REPLY_KEY_AC_DATA_LEN;
	
	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, NULL, &reply_data, NULL );
}

void terminal_key_action_host_common_reply( const uint8_t recvdata, tmnl_pdblist node )
{
	uint8_t data_len;
	tka_common_reply common_data;
	common_data.recv_data = recvdata;
	data_len = COMMON_REPLY_KEY_AC_DATA_LEN;

	assert( node );
	if( node == NULL )
		return;
	
	terminal_key_action_host_reply( node->tmnl_dev.entity_id, node->tmnl_dev.address.addr, data_len, &common_data, NULL, NULL );
}

void terminal_key_speak( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	tmnl_pdblist tmp_node = NULL;
	
	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such tmp_node: addr = %04x", addr );
		return;
	}

	if( key_num == KEY6_SPEAK )
	{
		uint8_t mic_state;
		if( key_value )
		{
			// no limit time
			mic_state = MIC_OPEN_STATUS;
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( tmp_node->tmnl_dev.entity_id, 0, tmp_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( tmp_node->tmnl_dev.entity_id );
#endif
		}
		else
		{
			mic_state = MIC_COLSE_STATUS;
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_disconnect( tmp_node->tmnl_dev.entity_id, tmp_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( tmp_node->tmnl_dev.entity_id );
#endif
		}

		terminal_key_action_host_special_num1_reply( recvdata, mic_state, tmp_node );
	}
}

void terminal_key_action_chman_interpose( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	if( key_num != KEY7_CHAIRMAN_FIRST )
	{
		DEBUG_INFO( "not valid chairman interpose value key " );
		return;
	}

	tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		DEBUG_INFO( "no such 0x%04x addr chairman!", addr );
		return;
	}

	if( key_value )
	{
		terminal_chairman_interpose( addr, true, tmp_node, recvdata );
	}
	else
	{
		terminal_chairman_interpose( addr, false, tmp_node, recvdata );
	}
}

void terminal_chairman_interpose( uint16_t addr, bool key_down, tmnl_pdblist chman_node, const uint8_t recvdata )
{
	tcmpt_data_mic_status mic_list[SYSTEM_TMNL_MAX_NUM]; // 100-��ʱ����������
	uint16_t report_mic_num = 0;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	assert( chman_node );
	if( chman_node == NULL )
		return;
	
	if( (key_down && gchm_int_ctl.is_int) ||\
		((!key_down) && (!gchm_int_ctl.is_int)) ||\
		((!key_down) && (gchm_int_ctl.chmaddr != addr)))
	{
		terminal_key_action_host_common_reply( recvdata, chman_node );
		return;
	}

	if( (get_sys_state() != INTERPOSE_STATE) && key_down )
	{
		bool tmp_close = false; // temp close
		
		set_terminal_system_state( INTERPOSE_STATE, true );
		gchm_int_ctl.is_int = true;
		gchm_int_ctl.chmaddr = addr;
		tmp_close = (set_sys.temp_close != 0)?true:false; 

		terminal_key_action_host_special_num1_reply( recvdata, MIC_CHM_INTERPOSE_STATUS, chman_node );// ������ϯmic״̬
		terminal_mic_state_set( MIC_CHM_INTERPOSE_STATUS, BRDCST_ALL, BRDCST_1722_ALL, true, chman_node );

		/**
		 *2015-12-11
		 *����ϯmic,������״̬
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS )
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( chman_node->tmnl_dev.entity_id,\
				0, chman_node, false, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send ); // ���ﲻ������mic open��״̬2015-12-11
#else
			trans_model_unit_connect( chman_node->tmnl_dev.entity_id );
#endif
		}

		assert( dev_terminal_list_guard );
		if( dev_terminal_list_guard == NULL )
			return;
		
		tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
		for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
		{
			if( tmp_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST )
			{// �ر�������ͨ�����
				if( tmp_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
				{
#ifdef ENABLE_CONNECT_TABLE
					connect_table_tarker_disconnect( tmp_node->tmnl_dev.entity_id, tmp_node, !tmp_close, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
					trans_model_unit_disconnect( tmp_node->tmnl_dev.entity_id );
#endif
					if( report_mic_num <= SYSTEM_TMNL_MAX_NUM )
					{
						mic_list[report_mic_num].addr.low_addr = (uint8_t)((tmp_node->tmnl_dev.address.addr&0x00ff) >> 0);
						mic_list[report_mic_num].addr.high_addr = (uint8_t)((tmp_node->tmnl_dev.address.addr&0xff00) >> 0);
						mic_list[report_mic_num].switch_flag = MIC_COLSE_STATUS;
						report_mic_num++;
					}
				}
			}
		}

		// �ϱ�mic״̬
		cmpt_miscrophone_status_list_from_set( mic_list, report_mic_num );
		
		gdisc_flags.apply_num = 0;
		gdisc_flags.speak_limit_num = 0;		
	}
	else if( !key_down )
	{
		set_terminal_system_state( INTERPOSE_STATE, false );
		terminal_key_action_host_special_num1_reply( recvdata, chman_node->tmnl_dev.tmnl_status.mic_state, chman_node );

		/**
		 *2015-12-11
		 *����ϯmic����һ��״̬�Ǵ򿪵�״̬��ȥ������
		 *����Ͽ���mic�������������ñ���
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS )
		{
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_disconnect( chman_node->tmnl_dev.entity_id, chman_node, false, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( chman_node->tmnl_dev.entity_id );
#endif
		}
		
		assert(dev_terminal_list_guard);
		if( dev_terminal_list_guard == NULL )
			return;
		
		tmnl_pdblist end_node = dev_terminal_list_guard->next;
		for( ;end_node != dev_terminal_list_guard; end_node = end_node->next )
		{
			if( (end_node->tmnl_dev.address.addr != 0xffff)&&\
				(end_node->tmnl_dev.tmnl_status.is_rgst) &&\
				(end_node->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS) &&\
				((end_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST) ||(end_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP)))
			{
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( end_node->tmnl_dev.entity_id, set_sys.spk_limtime, end_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( end_node->tmnl_dev.entity_id );
#endif
			}
		}

		int i = 0;
		for( i = 0; i < gdisc_flags.apply_num; i++ )
		{
			uint16_t addr_apply = gdisc_flags.apply_addr_list[i];
			tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr(addr_apply);
			if( tmp_node == NULL )
			{
				continue;
			}
			
			if( i == gdisc_flags.currect_first_index )
			{
				terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, addr_apply, tmp_node->tmnl_dev.entity_id, true, tmp_node );
			}
			else
			{
				terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, addr_apply, tmp_node->tmnl_dev.entity_id, true, tmp_node );
			}
		}

		gchm_int_ctl.is_int = false;
		gchm_int_ctl.chmaddr = 0xffff;
	}
}

int terminal_key_discuccess( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg )
{
	tmnl_pdblist dis_node;
	
	dis_node = found_terminal_dblist_node_by_addr( addr );
	if( dis_node == NULL )
	{
		DEBUG_INFO( "not found discuccess termianl key!" );
		return -1;
	}

	if( key_num == KEY6_SPEAK )
	{
		if( key_value )
		{
			terminal_key_speak_proccess( dis_node, true, recv_msg );
		}
		else
		{
			terminal_key_speak_proccess( dis_node, false, recv_msg );
		}
	}

	return 0;
}

bool terminal_key_speak_proccess( tmnl_pdblist dis_node, bool key_down, uint8_t recv_msg )
{
	uint8_t dis_mode = gdisc_flags.edis_mode;
	thost_system_set set_sys; // ϵͳ�����ļ��ĸ�ʽ
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	if( dis_node == NULL )
		return false;

	if( !dis_node->tmnl_dev.tmnl_status.is_rgst )
		return false;
	
	DEBUG_INFO( "dis mode = %d ", dis_mode );
	if( dis_mode == PPT_MODE ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
	{
		if( key_down )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( dis_node->tmnl_dev.entity_id, 0, dis_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( dis_node->tmnl_dev.entity_id );
#endif
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_disconnect( dis_node->tmnl_dev.entity_id, dis_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( dis_node->tmnl_dev.entity_id );
#endif
		}
	}
	else
	{
		switch( dis_mode )
		{
			case FREE_MODE:
			{
				terminal_free_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case LIMIT_MODE:
			{
				terminal_limit_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case FIFO_MODE:
			{
				terminal_fifo_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			case APPLY_MODE:
			{
				terminal_apply_disccuss_mode_pro( key_down, set_sys.spk_limtime, dis_node, recv_msg );
				break;
			}
			default:
			{
				DEBUG_INFO( " out of discuss mode bound!" );
				break;
			}
		}
	}
	
	return true;
}

int terminal_chairman_apply_reply( uint8_t tmnl_type, uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, uint8_t recv_msg )
{
	tmnl_pdblist apply_node;
	
	apply_node = found_terminal_dblist_node_by_addr( addr );
	if( apply_node == NULL )
	{
		return -1;
	}
	
	if( tmnl_type != COMMAND_TMN_CHAIRMAN )
	{
		return -1;
	}

	if( tmnl_state == TMNL_SYS_STA_VOTE || tmnl_state == TMNL_SYS_STA_GRADE || tmnl_state == TMNL_SYS_STA_SELECT )
	{
		return -1;
	}

	enum_apply_pro apply_flag = REFUSE_APPLY;
	if( (key_num == KEY2_VOTE) && key_value )
	{
		apply_flag = REFUSE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY3_VOTE) && key_value )
	{
		apply_flag = NEXT_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}
	else if( (key_num == KEY4_VOTE) && key_value )
	{
		apply_flag = APPROVE_APPLY;
		terminal_key_action_host_common_reply( recv_msg, apply_node );
		terminal_examine_apply( apply_flag );
	}

	return 0;
}

void terminal_free_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );
	if( speak_node == NULL )
		return;
	
	if( key_down )
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, 0, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );	
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	bool ret = false;
	
	assert( speak_node );
	if( speak_node == NULL )
	{
		return ret;
	}
		
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // ��λ���뷢��
	if( key_down ) // ����˷�
	{
		if( speak_node->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			ret = true;
		}
		else if(speak_node->tmnl_dev.tmnl_status.mic_state == MIC_OTHER_APPLY_STATUS)
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
			ret = true;
		}
		else if(speak_node->tmnl_dev.tmnl_status.mic_state == MIC_FIRST_APPLY_STATUS)
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_FIRST_APPLY_STATUS, speak_node );
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num ) // ����˷�
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			ret = true;
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // ���뷢��
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );

			if( gdisc_flags.apply_num > 0 && current_addr == addr )// ����һ������Ϊ��λ����״̬
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else if( cc_state == MIC_OPEN_STATUS )
		{
			if( gdisc_flags.speak_limit_num > 0 )
			{
				gdisc_flags.speak_limit_num--;
			}

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// ��������,����ʼ��һ�������ն˵ķ���
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// ������һ�����뻰Ͳ
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
#ifdef ENABLE_CONNECT_TABLE
						connect_table_tarker_connect( first_speak->tmnl_dev.entity_id, limit_time, first_speak, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
						trans_model_unit_connect( first_speak->tmnl_dev.entity_id );
#endif

						if( gdisc_flags.apply_num > 0 ) // ������λ���뷢���ն�
						{
							gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
							first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
							if( first_apply != NULL )
							{
								terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
							}
							else
							{
								DEBUG_INFO( " no such tmnl dblist node!");
							}
						}
						else
						{
							gdisc_flags.currect_first_index = gdisc_flags.apply_num;
						}
					}
					else
					{
						DEBUG_INFO( " no such tmnl dblist node!");
					}
				}
				else
				{
					gdisc_flags.currect_first_index = 0;
				}
			}

			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}

	return ret;
	
}

bool terminal_fifo_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;

	assert( speak_node );
	if( speak_node == NULL )
	{
		DEBUG_INFO( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	if( key_down ) // �򿪻�Ͳ
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			ret = true;
		}
		else if( speak_limit_num < gdisc_flags.limit_num )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
			gdisc_flags.speak_addr_list[speak_limit_num] = addr;
			ret = true;
		}
		else // �����������ڻ������������
		{
			if( gdisc_flags.speak_addr_list[0] != 0xffff ) // �Ƚ��ȳ�
			{
				tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
				if( first_speak != NULL )
				{
#ifdef ENABLE_CONNECT_TABLE
					connect_table_tarker_disconnect( first_speak->tmnl_dev.entity_id, first_speak, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
					trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id );
#endif
				}
				else
				{
					DEBUG_INFO( "fifo not found tmnl list node!");
				}
				
				addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// ��λ����ɾ��

				uint8_t speak_limit_num1 = gdisc_flags.speak_limit_num;
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
				connect_table_tarker_connect( speak_node->tmnl_dev.entity_id, limit_time, speak_node, true, MIC_OPEN_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
				trans_model_unit_connect( speak_node->tmnl_dev.entity_id );
#endif
				gdisc_flags.speak_addr_list[speak_limit_num1] = speak_node->tmnl_dev.address.addr;
				ret = true;
			}
		}
	}
	else
	{
		addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
		connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
		ret = true;
	}

	return ret;
}

bool terminal_apply_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	assert( speak_node );
	if( speak_node == NULL )
	{
		return false;
	}

	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	tmnl_pdblist first_apply = NULL;

	if( key_down ) // ���뷢��,�ӵ�ַ�������б�
	{
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			uint8_t state = MIC_OTHER_APPLY_STATUS;
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			terminal_mic_state_set( state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
	}
	else // ȡ�����뷢��
	{
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{// terminal apply
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );// ȡ����ǰ�����뷢��
			terminal_mic_state_set( MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );// �ϱ�mic״̬
			//DEBUG_INFO( "gdisc_flags.apply_num =%d  current_addr = 0x%04x addr = 0x%04x", gdisc_flags.apply_num, current_addr, addr );
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// ����һ������Ϊ��λ����״̬
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index] );
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
				}
			}
			
			terminal_main_state_send( 0, NULL, 0 );
		}	
		else
		{// terminal speaking
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#ifdef ENABLE_CONNECT_TABLE
			connect_table_tarker_disconnect( speak_node->tmnl_dev.entity_id, speak_node, true, MIC_COLSE_STATUS, terminal_mic_state_set, terminal_main_state_send );
#else
			trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id );
#endif
			// �������� camera strack 2016-3-2
		}
		
		ret = true;
	}

	return ret;
}

void terminal_key_preset( uint8_t tmnl_type, uint16_t tmnl_addr, uint8_t tmnl_state, uint8_t key_num, uint8_t key_value )
{
	uint16_t addr;
	if( key_value )
	{
		addr = tmnl_addr;
		find_func_command_link( SYSTEM_USE, SYS_PRESET_ADDR, 0, (uint8_t*)&addr, sizeof(uint16_t) );
	}
}

int terminal_speak_track( uint16_t addr, bool track_en )// ������ٽӿ�
{
	uint16_t temp;
	uint16_t i;
	uint16_t index;
	uint8_t cmr_track;

	cmr_track = gset_sys.camara_track;
 	if( cmr_track )
	{
		return -1;
	}
	
	for( i = 0; i <gspeaker_track.spk_num; i++)
	{
		if( addr == gspeaker_track.spk_addrlist[i])
		{
			break;
		}
	}
	
	index = i;
	if( index < gspeaker_track.spk_num )
	{
		if( gspeaker_track.spk_num > 1)
		{
			for( i = index; i < (gspeaker_track.spk_num-1); i++)
			{
				gspeaker_track.spk_addrlist[i] = gspeaker_track.spk_addrlist[i+1];
			}
			
			gspeaker_track.spk_num--;
		}
		else if( gspeaker_track.spk_num == 1 )
		{
			gspeaker_track.spk_addrlist[0] = 0xFFFF;
			gspeaker_track.spk_num = 0;
		}
	}
	
	if( track_en )
	{
		if( gspeaker_track.spk_num < MAX_SPK_NUM )
		{
			gspeaker_track.spk_addrlist[gspeaker_track.spk_num] = addr;
			gspeaker_track.spk_num++;
			temp = addr;
			find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
		}
	}
	else
	{
		if( 0 == gspeaker_track.spk_num )
		{
			temp = FULL_VIEW_ADDR;
		}
		else
		{
			temp = gspeaker_track.spk_addrlist[gspeaker_track.spk_num-1];
		}
		
		find_func_command_link( SYSTEM_USE, SYS_GET_PRESET, 0, (uint8_t*)&temp,sizeof(uint16_t) );
	}

	return 0;
}

/*************************************************************
*==��ʼͶƱ����-->����δǩ�����ն˵�ͶƱ
*/
void terminal_vote_proccess( void )
{
	if( gvote_flag == VOTE_SET )
	{
		uint16_t index = gvote_index;
		uint16_t addr = 0xffff;
		tmnl_pdblist tmp = NULL;
		bool waiting_query = false;
		
		do
		{
			addr = tmnl_addr_list[index].addr;
			if( addr != 0xffff )
			{
				tmp = found_terminal_dblist_node_by_addr( addr );
				if( (tmp != NULL) && (tmp->tmnl_dev.address.addr != 0xffff)\
					&& ( tmp->tmnl_dev.tmnl_status.is_rgst) &&\
					( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_SET_FLAG ) &&\
					( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_EN ))
				{// �ȴ�ͶƱ����TVOTE_EN����(��ǩ���ɹ�) ,
					waiting_query = true;
					break;
				}
			}

			index++;
			index %= SYSTEM_TMNL_MAX_NUM;
		}while( index != gvote_index );

		if( waiting_query )
		{
			if( tmp != NULL )
			{
				tmp->tmnl_dev.tmnl_status.vote_state &= (~TVOTE_SET_FLAG);
				tmp->tmnl_dev.tmnl_status.vote_state |= (TWAIT_VOTE_FLAG);// ���óɿɲ�ѯ״̬
				gvote_index++;
				gvote_index %= SYSTEM_TMNL_MAX_NUM;
			}
		}
		else
		{// �鿴ϵͳ�Ƿ�ͶƱ���
			int i = 0;
			for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
			{
				addr = tmnl_addr_list[i].addr;
				if( addr != 0xffff )
				{
					tmp = found_terminal_dblist_node_by_addr( addr );
					if( (tmp != NULL) && (tmp->tmnl_dev.address.addr != 0xffff)\
						&& (tmp->tmnl_dev.tmnl_status.is_rgst) &&\
						( tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_SET_FLAG))
					{
						break;
					}
				}
			}

			if( i >= SYSTEM_TMNL_MAX_NUM )
			{
				gvote_flag = VOTE_SET_OVER;
				gvote_index = 0;
			}
		}
	}
}

/*************************************************************
*==����ͶƱ����
*/

/*************************************************************
*==��ʼ��ѯ����
*/
/*************************************************************
*Date:2016/1/27
*Name:terminal_query_vote_ask
*����:�����ն�ǩ����������
*Param:
*	address :terminal application address
*	vote_state:�ն�ǩ������������
*Return:None
**************************************************************/
void terminal_query_vote_ask( uint16_t address, uint8_t vote_state )
{
	tmnl_pdblist vote_node = found_terminal_dblist_node_by_addr( address );
	if( NULL == vote_node )
	{
		DEBUG_INFO( "no such address 0x%04x node ", address );
		return;
	}

	uint8_t sys_state = get_sys_state();
	if( (SIGN_STATE == sys_state ) && (vote_state & 0x80) )
	{// sign complet
		if( gtmnl_signstate == SIGN_IN_ON_TIME )// ����ǩ����־
		{
			vote_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
		}
		else if( gtmnl_signstate == SIGN_IN_BE_LATE && (vote_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN) )
		{
			vote_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
		}
		
		upper_cmpt_report_sign_in_state( vote_node->tmnl_dev.tmnl_status.sign_state, vote_node->tmnl_dev.address.addr );
	}
	else if ( ((VOTE_STATE == sys_state) || (GRADE_STATE == sys_state) \
		||(ELECT_STATE == sys_state)) && ((vote_state & TVOTE_KEY_MARK )))
	{
		if ( gfirst_key_flag )
		{
			int i = 0, vote_num = 0;
			for ( i = 0; i < 5; i++ )
			{
				if( vote_node->tmnl_dev.tmnl_status.vote_state& (1<<i) )
				{
					vote_num++;
				}
			}
#if 1
			uint8_t key_num = 0;
			terminal_vote_mode_max_key_num( &key_num, gvote_mode );
			DEBUG_INFO( "max key num = %d-------vote num = %d", key_num, vote_num );
			if ( vote_num >= key_num )
			{// ͶƱ��ɣ���Ӧ���ն�ֹͣ��ѯͶƱ���
				vote_node->tmnl_dev.tmnl_status.vote_state &= (~TWAIT_VOTE_FLAG);
			}
#else
			if ( vote_num >= gvote_mode )
			{// ͶƱ��ɣ���Ӧ���ն�ֹͣ��ѯͶƱ���
				vote_node->tmnl_dev.tmnl_status.vote_state &= (~TWAIT_VOTE_FLAG);
			}
#endif
		}
		
		vote_node->tmnl_dev.tmnl_status.vote_state &= (~TVOTE_KEY_MARK);
		vote_node->tmnl_dev.tmnl_status.vote_state |= (vote_state & TVOTE_KEY_MARK);

		upper_cmpt_vote_situation_report( vote_node->tmnl_dev.tmnl_status.vote_state, vote_node->tmnl_dev.address.addr );
	}
}

void terminal_vote_mode_max_key_num( uint8_t *key_num, tevote_type vote_mode  )
{
	assert( key_num );
	if( key_num == NULL )
		return;
	
	switch( vote_mode )
	{
		case VOTE_MODE:
		case GRADE_MODE:
		case SLCT_2_1:
		case SLCT_3_1:
		case SLCT_4_1:
		case SLCT_5_1:
		{
			*key_num = 1;
			break;
		}
		
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
		{
			*key_num = 2;
			break;
		}
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
		{
			*key_num = 3;
			break;
		}
		case SLCT_4_4:
		case SLCT_5_4:
		{
			*key_num = 4;
			break;
		}
		case SLCT_5_5:
		{
			*key_num = 5;
			break;
		}
		default:
		{
			*key_num = 0;
			break;
		}
	}
}

// ������ѯǩ��ͶƱ���
void terminal_query_sign_vote_pro( void )
{
	bool sending = false;
	uint16_t index;
	uint8_t sys_state = get_sys_state();
	tmnl_pdblist tmp_node = NULL;
	uint16_t addr = 0xffff;

	index = gquery_svote_pro.index;
	if( (index > (SYSTEM_TMNL_MAX_NUM - 1)) && (index < 0 ))
	{
		DEBUG_INFO( "out of system terminal list bank!" );
		return;
	}
	
	if ( (gquery_svote_pro.running) && host_timer_timeout(&gquery_svote_pro.query_timer))
	{
		host_timer_update( 60, &gquery_svote_pro.query_timer );
		if( sys_state == SIGN_STATE )
		{
			do
			{
				addr = tmnl_addr_list[index].addr;
				if( addr != 0xffff )
				{
					tmp_node = found_terminal_dblist_node_by_addr( addr );
					if( (tmp_node != NULL) && (tmp_node->tmnl_dev.address.addr != 0xffff) &&\
						(tmp_node->tmnl_dev.tmnl_status.is_rgst) &&\
						(tmp_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN ) )
					{
						terminal_query_vote_sign_result( tmp_node->tmnl_dev.entity_id, addr );
						sending = true;
						break;
					}
				}

				index++;
				index %= SYSTEM_TMNL_MAX_NUM;
			}while( index != gquery_svote_pro.index );

			if( sending )
			{
				gquery_svote_pro.index = (index + 1)%SYSTEM_TMNL_MAX_NUM;
			}
			else
			{
				gquery_svote_pro.running = false;
			}
		}
		else if( (sys_state == VOTE_STATE ) || (sys_state == GRADE_STATE) ||(ELECT_STATE))
		{
			do
			{
				addr = tmnl_addr_list[index].addr;
				if( addr != 0xffff )
				{
					tmp_node = found_terminal_dblist_node_by_addr( addr );
					if( (tmp_node != NULL) && (tmp_node->tmnl_dev.address.addr != 0xffff) &&\
						(tmp_node->tmnl_dev.tmnl_status.is_rgst) &&\
						(tmp_node->tmnl_dev.tmnl_status.vote_state & TWAIT_VOTE_FLAG ) )
					{
						terminal_query_vote_sign_result( tmp_node->tmnl_dev.entity_id, addr );
						sending = true;
						break;
					}
				}

				index++;
				index %= SYSTEM_TMNL_MAX_NUM;
			}while( index != gquery_svote_pro.index );

			if( sending )
			{
				gquery_svote_pro.index = (index + 1)%SYSTEM_TMNL_MAX_NUM;
			}
			else
			{
				terminal_option_endpoint( BRDCST_1722_ALL, BRDCST_EXE, OPT_TMNL_ALL_VOTE );
				gquery_svote_pro.running = false;
			}
		}
	}
}

void terminal_query_proccess_init( void )
{
	gquery_svote_pro.index = 0;
	gquery_svote_pro.running = false;
	host_timer_stop(&gquery_svote_pro.query_timer );
}

/*************************************************************
*==������ѯ����
*/

/*************************************************************
*==��ʼǩ������
*/

// ��ǩ����
//#define __MIND_UPPER_CMPT_SIGN_RESULT__
void terminal_sign_in_pro( void )
{
	uint8_t sign_type;
	int i = 0;

#ifdef __MIND_UPPER_CMPT_SIGN_RESULT__
	/*
	* 1��Ϊ�˽�������������ϱ�ǩ��״̬����λ������ͣ
	*����ǩ�������⡣2016/1/29
	*2�����ﱻע���ˣ����õĸ���ԭ���Ǿ������Բ����κ����á����
	*�޸����ϱ�ͶƱ����ĳ�ʱʱ���50ms��Ϊ150ms
	*/
	static int report_num = 0;
	if( gvote_flag != NO_VOTE )
	{// ͶƱʱ��
		if( over_time_listen( MIND_UPPER_CMPT_SIGN_RESULT ))
		{
			over_time_set( MIND_UPPER_CMPT_SIGN_RESULT, 500 );
			proccess_upper_cmpt_sign_state_list();
			if( (++report_num) == 5 )
			{
				report_num = 0;
				over_time_stop( MIND_UPPER_CMPT_SIGN_RESULT );
			}
		}
	}
#endif

	if( (gtmnl_signstate == SIGN_IN_BE_LATE)  )
	{
		if(over_time_listen( SIGN_IN_LATE_HANDLE ))
		{
			gtmnl_signstate = SIGN_IN_OVER;
			sign_type = gset_sys.sign_type;
			if( sign_type )
			{
				DEBUG_INFO( "over time sign in sign type is card sign in " );
				return;
			}
			
			for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++)
			{
				uint16_t addr = tmnl_addr_list[i].addr;
				if( addr == 0xffff )
					continue;
				else if( addr != 0xffff )
				{
					tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
					if( (tmp != NULL) && ( tmp->tmnl_dev.address.addr != 0xffff) &&\
						(tmp->tmnl_dev.tmnl_status.is_rgst) && \
							(tmp->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN ))
					{
						terminal_led_set_save( addr, TLED_KEY5, TLED_OFF );
						fterminal_led_set_send( addr );
					}
				}
			}

			over_time_stop( SIGN_IN_LATE_HANDLE );
		}
	}
}

/*************************************************************
*==����ǩ������
*/

/*===================================================
�ն˴�������@}
=====================================================*/

/*==================================================
	start reallot address
====================================================*/
//����ն�����
tmnl_pdblist terminal_system_dblist_except_free( void )
{
	tmnl_pdblist p_node = NULL;
	
	p_node = terminal_dblist_except_free( dev_terminal_list_guard );

	return p_node;
}

void terminal_open_addr_file_wt_wb( void )
{
	if( addr_file_fd != NULL )
		Fclose( addr_file_fd );// �ȹر�
		
	addr_file_fd = Fopen( ADDRESS_FILE, "wb+");
	if( addr_file_fd == NULL )
	{
		DEBUG_ERR( "terminal_open_addr_file_wt_wb open fd  Err!" );
		assert( NULL != addr_file_fd );
		if( NULL == addr_file_fd )
			return;
	}	
}

// �ݻ��ն�����
void terminal_system_dblist_destroy( void )
{
	tmnl_pdblist p_node = destroy_terminal_dblist( dev_terminal_list_guard );
	if( NULL != p_node )
	{
		DEBUG_INFO( "destroy terminal double list not success!" );
	}
}
	
/*===================================================
end reallot address
=====================================================*/

