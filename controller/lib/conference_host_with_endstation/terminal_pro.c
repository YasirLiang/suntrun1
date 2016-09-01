/**
*terminal_pro.c
*
*proccess meeting
*****************************************************************************/

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
#include "log_machine.h" /* 系统日志头文件*/
#include "conference_transmit_unit.h"

/* terminal mic time out set macro------------------------------------------*/
#define MIC_SET_TIME_OUT 1000

/* terminal mic num macro---------------------------------------------------*/
#define MIC_ARRAY_NUM 20/*mic数组长度*/

/* terminal mic structure --------------------------------------------------*/
/*${terminal::Terminal_mic} ................................................*/
typedef struct terminal_micLater 
{
    tmnl_pdblist node; /*会议单元节点*/
    uint32_t timeTick; /*超时时间*/
    uint8_t setCount;   /*设置次数*/
    uint8_t micState; /*mic status to set enpointment*/
}Terminal_mic;

/* Terminal micQueue structure ---------------------------------------------*/
/*${Terminal::micQueue } ...................................................*/
typedef struct terminal_micQueue 
{
    uint8_t head; /*head of queue*/
    uint8_t trail; /*trail of queue*/
    uint8_t size;   /*size of queue*/
    Terminal_mic * const pBuf; /*const point to buf of queue */
}Terminal_micQueue;

/* Local objects -----------------------------------------------------------*/
static Terminal_mic l_proMics[MIC_ARRAY_NUM] = {
    {NULL, 0U, 0U}
};

/* Local objects -----------------------------------------------------------*/
static Terminal_micQueue l_micQueue = {
    0U, 0U, MIC_ARRAY_NUM, &l_proMics[0]
};

#ifdef __DEBUG__
#define __TERMINAL_PRO_DEBUG__
#endif

#ifdef __TERMINAL_PRO_DEBUG__
#define terminal_pro_debug(fmt, args...) \
	fprintf(stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n",\
	           __FILE__, __func__, __LINE__, ##args);
#else
#define terminal_pro_debug(fmt, args...)
#endif

/* 终端地址信息读取文件描述符*/
FILE* addr_file_fd = NULL;
/*终端地址分配列表*/
terminal_address_list tmnl_addr_list[SYSTEM_TMNL_MAX_NUM];	
terminal_address_list_pro allot_addr_pro;
/*终端链表表头结点，对其正确地操作，必须先注册完终端*/
tmnl_pdblist dev_terminal_list_guard = NULL; 				
tmnl_pdblist gcur_tmnl_list_node = NULL;
/*重新分配标志*/
volatile bool reallot_flag = false; 							
tmnl_state_set gtmnl_state_opt[TMNL_TYPE_NUM];
/*系统讨论参数*/
tsys_discuss_pro gdisc_flags;
/*主席插话*/
tchairman_control_in gchm_int_ctl;
/* 终端报到处理*/
volatile ttmnl_register_proccess gregister_tmnl_pro; 			
/*发言时长， 0表示无限时；1-63表示限时1-63分钟*/
volatile uint8_t speak_limit_time = 0; 						
/* lcd 显示的屏号*/
volatile uint8_t glcd_num = 0;
/*终端指示灯*/
volatile uint8_t gled_buf[2] = {0};
/*签到的状态，也可为终端的签到状态*/
volatile enum_signstate gtmnl_signstate;
/*补签的超时时间*/
volatile uint8_t gsign_latetime;
/*签到标志*/
volatile bool gsigned_flag = false;
/*  投票处理*/
volatile evote_state_pro gvote_flag = NO_VOTE;
/* 投票偏移*/ 
volatile uint16_t gvote_index;
/* 真为投票首键有效*/
volatile bool gfirst_key_flag;
/* 投票模式*/
volatile tevote_type gvote_mode;							
type_spktrack gspeaker_track;
/*  查询签到表决结果*/
tquery_svote  gquery_svote_pro;

void init_terminal_proccess_fd( FILE ** fd )
{
	*fd = Fopen( ADDRESS_FILE, "rb+" );
	if( NULL == *fd )
	{
		terminal_pro_debug( "open %s Err: not exit!Will create for the address file for the first time!", ADDRESS_FILE );
		terminal_open_addr_file_wt_wb();
	}
}

/*==============================================
初始化终端地址列表
================================================*/
int init_terminal_address_list_from_file( void )
{
	int i = 0;
	int ret = 0;
	
	/* 初始化全局变量tmnl_addr_list*/ 
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}

	/*读终端地址信息到tmnl_addr_list, 若读取失败，则系统需要重新分配终端地址 */ 
	ret = terminal_address_list_read_file( addr_file_fd, tmnl_addr_list );
	if( ret == -1 )
	{
		terminal_pro_debug( "init tmnl_addr_list from address file!need to reallot terminal address\n\t\tPlease send reAllot command by command line!!!");
		reallot_flag = true;
	}

	return ret;
}

void init_terminal_address_list( void )
{
	int i = 0;
	
	/*初始化全局变量tmnl_addr_list */ 
	memset( tmnl_addr_list, 0, sizeof(tmnl_addr_list) );
	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		tmnl_addr_list[i].addr = INIT_ADDRESS;
		tmnl_addr_list[i].tmn_type = TMNL_TYPE_COMMON_RPRST;
	}
}

/*==============================================
结束初始化终端地址列表
================================================*/

inline void init_terminal_allot_address( void )
{
	allot_addr_pro.addr_start = 0;
	allot_addr_pro.index = 0;
	allot_addr_pro.renew_flag= 0;
	reallot_flag = false; /*  disable reallot*/
}

inline void init_terminal_device_double_list( void )
{
	/* init terminal system double list */
	init_terminal_dblist( &dev_terminal_list_guard );
	assert( dev_terminal_list_guard != NULL );
	gcur_tmnl_list_node = dev_terminal_list_guard;
}

/*
*date:2015-11-6
*/
int init_terminal_discuss_param( void ) 
{
	thost_system_set set_sys; /* 系统配置文件的格式*/ 
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	gdisc_flags.apply_limit = set_sys.apply_limit;
	gdisc_flags.limit_num = set_sys.speak_limit;
	gdisc_flags.currect_first_index = set_sys.apply_limit;
	gdisc_flags.apply_num = 0;
	gdisc_flags.speak_limit_num = 0; /*发言人数 */ 
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

#ifdef __DEBUG__  /*  模拟终端信息*/
#define WRITE_ADDR_NUM 10
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

	terminal_register_init();
    
    	init_terminal_device_double_list();	/*初始化链表 */ 

	init_terminal_proccess_fd( &addr_file_fd );
	if( NULL == addr_file_fd )
		return;

	if( NULL != addr_file_fd )
	{
		tmnl_count = init_terminal_address_list_from_file();
		if( tmnl_count != -1)
		{
			terminal_pro_debug( "terminal count num = %d", tmnl_count );
			gregister_tmnl_pro.tmn_total = tmnl_count;/* 保存到未注册列表 */
			
			for( i = 0; i < tmnl_count; i++ )
			{
                                tmnl_pdblist tmnl_list_station = create_terminal_dblist_node( &tmnl_list_station );
                                if (tmnl_list_station != NULL)
                                {
                                        init_terminal_dblist_node_info( tmnl_list_station );
                                        tmnl_list_station->tmnl_dev.entity_id = 0;
                                        tmnl_list_station->tmnl_dev.address.addr = tmnl_addr_list[i].addr;
                                        tmnl_list_station->tmnl_dev.address.tmn_type = tmnl_addr_list[i].tmn_type;
                                        insert_terminal_dblist_trail( dev_terminal_list_guard, tmnl_list_station );
                                        terminal_pro_debug( "create new tmnl list node[ 0x%04x ] Success", tmnl_list_station->tmnl_dev.address.addr );
                                }
			}

                        if( -1 == sort_terminal_dblist_node(dev_terminal_list_guard) )
			{
				terminal_pro_debug( "insert  register node is Err!................" );
			}
		}		
	}

#ifdef __TERMINAL_PRO_DEBUG__ /*输出终端信息的数据 */ 
	print_out_terminal_addr_infomation( tmnl_addr_list, tmnl_count );
	if( tmnl_count != -1 )
		terminal_pro_debug( "terminal count num = %d", tmnl_count );
#endif

	init_terminal_allot_address();
	init_terminal_discuss_param();
	terminal_speak_track_pro_init();
	terminal_query_proccess_init();
}

/*释放终端资源2016-1-23*/
void terminal_proccess_system_close( void )
{
	if( addr_file_fd != NULL )
	{
		Fclose( addr_file_fd );/* 关闭文件描述符*/ 
		if( addr_file_fd != NULL )
			addr_file_fd = NULL;
	}
}
/*********************************************
=注册处理相关函数开始
**********************************************/
/* 终端注册保存地址到已注册或未注册列表,注:保存已注册与未注册终端的算法不同*/ 
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
				{/*  当前列表无未注册的地址*/
					p_regist_pro->register_pro_addr_list[*p_unregister_trail] = addr_save;/* 先保存，这是列表无未注册的地址 */ 
				}
				else if( p_regist_pro->register_pro_addr_list[*p_unregister_trail] != 0xffff )
				{
					p_regist_pro->register_pro_addr_list[++(*p_unregister_trail)] = addr_save;/* 先移动trail，原因是trail 代表最后的元素的下标*/ 
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
			**1、将未注册列表头的元素插入未注册的尾部
			**2、未注册的头部加1
			**3、往已注册列表的尾部插入需保存的地址
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

/* 从未注册列表中删除已注册终端,并保存此地址到已注册的列表中:register_addr_delect必须是已注册的地址(经测试暂时没有发现问题2016/01/26)*/ 
bool terminal_delect_unregister_addr( uint16_t register_addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (register_addr_delect != 0xffff))
	{
		/* 寻找删除的节点 */
		int i = 0, delect_index;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->noregister_head;
		volatile uint16_t *p_trail = &p_regist_pro->noregister_trail;
		if( (*p_head > *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail > (SYSTEM_TMNL_MAX_NUM-1)) || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))&&(*p_head != 0)) )
		{
			terminal_pro_debug( "Err delect unregister address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
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
			/* 将其与未注册列表的头的数据交换*/ 
			terminal_pro_debug( "save register addr = %04x ?=( (delect index = %d)list addr = %04x)-(swap addr = %04x)<<====>> %d(head_index)----%d(trail)---%d(rgsted_trail)", \
				register_addr_delect, delect_index,p_regist_pro->register_pro_addr_list[delect_index], p_regist_pro->register_pro_addr_list[*p_head],*p_head, *p_trail, p_regist_pro->rgsted_trail );
			if( *p_head > *p_trail )
				return false;
			else 
			{
				if( swap_valtile_uint16( &p_regist_pro->register_pro_addr_list[*p_head], &p_regist_pro->register_pro_addr_list[delect_index]) )
				{
					/*
					**1: 移动已注册表尾到未注册表头
					**2:移动未注册表头到未注册的表头的下一个未注册元素
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

/* 从未注册列表中清除未注册地址(未进行测试2016/01/26)*/ 
bool terminal_clear_from_unregister_addr_list( uint16_t unregister_addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (unregister_addr_delect != 0xffff))
	{
		/* 寻找删除的节点 */
		int i = 0, delect_index;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->noregister_head;
		volatile uint16_t *p_trail = &p_regist_pro->noregister_trail;
		if( (*p_head > *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail > (SYSTEM_TMNL_MAX_NUM-1)) ||  ((*p_head !=  (p_regist_pro->rgsted_trail + 1))&&(*p_head != 0)) )
		{
			terminal_pro_debug( "Err delect unregister address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
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
			/* 将其与未注册列表的头的数据交换 */
			if( swap_valtile_uint16(&p_regist_pro->register_pro_addr_list[*p_trail], &p_regist_pro->register_pro_addr_list[delect_index]) )
			{
				/*
				**1: 直接把尾节点置为不可用地址0xffff,并将尾指针向前移一元素
				*/
				p_regist_pro->register_pro_addr_list[(*p_trail)--] = 0xffff;
				terminal_pro_debug( "noregister list trail index = %d-trail  emlem value = %d", *p_trail, p_regist_pro->register_pro_addr_list[(*p_trail)] );

				return true;
			}
		}
		
	}

	return false;
}

/*  删除终端已注册地址，并将其放入未注册的地址列表表头中(未进行测试2016/01/26)*/
bool terminal_delect_register_addr( uint16_t addr_delect )
{
	volatile ttmnl_register_proccess *p_regist_pro = &gregister_tmnl_pro;
	if( (p_regist_pro != NULL) && (addr_delect != 0xffff))
	{
		/* 寻找删除的节点*/ 
		int i = 0, delect_index = -1;
		bool found_dl = false;
		volatile uint16_t *p_head = &p_regist_pro->rgsted_head;
		volatile uint16_t *p_trail = &p_regist_pro->rgsted_trail;
		if( (*p_head > *p_trail) ||(*p_head > (SYSTEM_TMNL_MAX_NUM-1))||\
			(*p_trail > (SYSTEM_TMNL_MAX_NUM-1)) || ((*p_head !=  (p_regist_pro->rgsted_trail + 1))&&(*p_head != 0)) )
		{
			terminal_pro_debug( "Err delect register address %d(head_index)----%d(trail)---%d(rgsted_trail)", \
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
			/* 将其与已注册列表的尾的数据交换*/ 
			if( swap_valtile_uint16(&p_regist_pro->register_pro_addr_list[*p_trail], &p_regist_pro->register_pro_addr_list[delect_index]) )
			{
				/*
				**1: 移动未注册表头到已注册表尾
				**2: 移动已注册表尾到已注册的表尾的上一个已注册元素
				*/
				p_regist_pro->noregister_head = *p_trail;
				if( *p_trail > 0 )/* 索引最小为零 */
					(*p_trail)--;

				gregister_tmnl_pro.tmn_rgsted--;

				return true;
			}
		}
		
	}

	return false;
}

/*注册*/
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
					terminal_pro_debug( "register addr = %04x-%04x, index = %d ", address & TMN_ADDR_MASK, tmnl_addr_list[i].addr, i );
					p_tmnl_station->tmnl_dev.tmnl_status.is_rgst = true;
					p_tmnl_station->tmnl_dev.tmnl_status.device_type = dev_type;
				        p_tmnl_station->tmnl_dev.address.addr = address & TMN_ADDR_MASK;
					p_tmnl_station->tmnl_dev.address.tmn_type = tmnl_addr_list[i].tmn_type;

					if( -1 == sort_terminal_dblist_node(dev_terminal_list_guard) )
					{
						terminal_pro_debug( "insert  register node is Err!................" );
					}

					set_terminal_system_state( DISCUSS_STATE, true );
                    
                    			gregister_tmnl_pro.tmn_rgsted++;
					bret = true;
                                        if (NULL != gp_log_imp)
				                gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
				                    LOGGING_LEVEL_DEBUG, 
				                    "[ terminal (0x%016llx-%04x) registed success ]", 
                                                    p_tmnl_station->tmnl_dev.entity_id,
                                                    p_tmnl_station->tmnl_dev.address.addr);

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
extern volatile bool gvregister_recved;/* 单个终端注册完成的标志 */
extern bool inflight_conference_command_exist(void);
extern void menu_first_display(void);
void system_register_terminal_pro( void )
{
	static bool static_reset_flags = true;
	static uint16_t static_norgst_index = 0;
	volatile register_state reg_state = gregister_tmnl_pro.rgs_state;
        bool registing = false;
	
	if( reallot_flag )
	{/*reallot time, can't register */ 
		return;
	}

	if (static_reset_flags)
	{
		static_reset_flags = false;
		over_time_set(WAIT_TMN_RESTART, 10000);
	}

	if (over_time_listen(WAIT_TMN_RESTART) && (RGST_WAIT == reg_state))
	{
	        terminal_query_endstation(0x8000, (uint64_t)0);
		gregister_tmnl_pro.rgs_state = RGST_QUERY;
		over_time_set(TRGST_OTIME_HANDLE, 15000);
	}
	
	if (RGST_QUERY == reg_state)
	{
		uint16_t count_num = 0;
		while(count_num < SYSTEM_TMNL_MAX_NUM)
		{
			uint16_t addr = tmnl_addr_list[static_norgst_index].addr;
			if (addr != 0xffff)
			{
				tmnl_pdblist register_node = found_terminal_dblist_node_by_addr(addr);
				if( NULL == register_node )
				{/*未与1722 id绑定，发送0地址 */ 
				        if (gvregister_recved ||over_time_listen(SIG_TMNL_REGISTER))/* 单个终端注册完成的标志)*/
        	                        {
        					terminal_query_endstation(addr, (uint64_t)0);

                                                gvregister_recved = false;
                                                over_time_set(SIG_TMNL_REGISTER, 200);
            					static_norgst_index++;
            					static_norgst_index %= SYSTEM_TMNL_MAX_NUM;
                                                registing = true;
                                                
                                                if (NULL != gp_log_imp)
				                        gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
                                                                LOGGING_LEVEL_DEBUG, 
                                                                "[Terminal (0x%016llx-%04x) Registing ]", 
                                                                (uint64_t)0,
                                                                addr);
            					break;
                                        }
				}
				else
				{
					if (!register_node->tmnl_dev.tmnl_status.is_rgst)
					{
					        if (gvregister_recved ||over_time_listen(SIG_TMNL_REGISTER))// 单个终端注册完成的标志)
        	                                {
    						    terminal_query_endstation(addr, (uint64_t)0);

                                                    gvregister_recved = false;
                                                    over_time_set(SIG_TMNL_REGISTER, 200);
                				    static_norgst_index++;
                				    static_norgst_index %= SYSTEM_TMNL_MAX_NUM;
                                                    registing = true;
                                                    
                                                    if (NULL != gp_log_imp)
				                        gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
                                                                LOGGING_LEVEL_DEBUG, 
                                                                "[Terminal (0x%016llx-%04x) Registing ]", 
                                                                (uint64_t)0,
                                                                addr);
                				    break;
                                               }
                                        }
				}
			}
			
			static_norgst_index++;
			static_norgst_index %= SYSTEM_TMNL_MAX_NUM;
			count_num++;
		}

		if ((count_num >= SYSTEM_TMNL_MAX_NUM && !registing &&\
                        gregister_tmnl_pro.tmn_total <= gregister_tmnl_pro.tmn_rgsted) ||\
			(over_time_listen(TRGST_OTIME_HANDLE)))
		{
		        DEBUG_INFO( "total = %d, rgsted = %d", gregister_tmnl_pro.tmn_total, gregister_tmnl_pro.tmn_rgsted);
			gregister_tmnl_pro.rgs_state = RGST_IDLE;
			set_terminal_system_state(DISCUSS_STATE, true);
			menu_first_display();
			terminal_start_discuss(false);
			terminal_main_state_send( 0, NULL, 0 );
		}
	}
	else if (RGST_IDLE == reg_state && (over_time_listen(QUEUE_REGISTER_TIMEOUT)))
	{
		uint16_t count_num = 0;
		static uint16_t static_query_index =  0;

		while(count_num < SYSTEM_TMNL_MAX_NUM)
		{
			uint16_t addr = tmnl_addr_list[static_query_index].addr;
			tmnl_pdblist register_node = NULL;
			if (addr != 0xffff)
			{
				register_node = found_terminal_dblist_node_by_addr(addr);
                                if (NULL == register_node) // not found?
                                {// address is not in dev_terminal_list_guard double list, should register again
                                	if (gvregister_recved ||over_time_listen(SIG_TMNL_REGISTER))
                                        {   
                                                terminal_query_endstation(addr, (uint64_t)0);
                                                
                                                gvregister_recved = false;
                                                over_time_set(SIG_TMNL_REGISTER, 100);// 100ms
                                                
                                                static_query_index++;
                				static_query_index %= SYSTEM_TMNL_MAX_NUM;
                                                registing = true;
                                                
                                                if (NULL != gp_log_imp)
                                                        gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
                                                                LOGGING_LEVEL_DEBUG,
                                                                "[Terminal (0x%016llx-%04x) Registing ]", 
                                                                (uint64_t)0,
                                                                addr);
                                                
                                                break;
                                        }
                                }
				else
				{
				        bool regis = false;
                                        solid_pdblist endpoint_node = NULL;
				        if (!register_node->tmnl_dev.tmnl_status.is_rgst)
                                              regis = true;
                        
                                        if (!regis && (0 != register_node->tmnl_dev.entity_id))
                                        {
                                                endpoint_node = search_endtity_node_endpoint_dblist(endpoint_list, register_node->tmnl_dev.entity_id);
                                                if (endpoint_node != NULL)
        					{
        						if (!endpoint_node->solid.connect_flag)
        						{
        							register_node->tmnl_dev.tmnl_status.is_rgst = false;
                                                                regis = true;
        						}
        					}
                                        }

                                        if (regis && (gvregister_recved || over_time_listen(SIG_TMNL_REGISTER)))
                                        {
                                                terminal_query_endstation(addr, (uint64_t)0);
        					gvregister_recved = false;
        					over_time_set(SIG_TMNL_REGISTER, 100);
                    
                                                static_query_index++;
                				static_query_index %= SYSTEM_TMNL_MAX_NUM;
                                                registing = true;
                                                
                                                if (NULL != gp_log_imp)
				                        gp_log_imp->log.post_log_msg(&gp_log_imp->log, 
                                                                LOGGING_LEVEL_DEBUG, 
                                                                "[Terminal (0x%016llx-%04x) Registing ]", 
                                                                (uint64_t)0,
                                                                register_node->tmnl_dev.address.addr);

                                                break;
                                        }
					
				}
			}

			static_query_index++;
			static_query_index %= SYSTEM_TMNL_MAX_NUM;
			count_num++;
		}

		if (count_num >= SYSTEM_TMNL_MAX_NUM && !registing)
		{
			over_time_set(QUEUE_REGISTER_TIMEOUT, 3*1000);// 3S
		}
                else
                        over_time_set(QUEUE_REGISTER_TIMEOUT, 500);
	}
}

// 开始注册函数
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
	gregister_tmnl_pro.rgs_state = RGST_WAIT;
	gregister_tmnl_pro.rgs_query_state = QUERY_RTST_WAIT; // 等待查询注册
	gvregister_recved = true;// 程序开始允许第一个终端注册
	over_time_set(SIG_TMNL_REGISTER, 50);
	over_time_set(QUEUE_REGISTER_TIMEOUT, 10*1000);// 10S
}
/*********************************************
=注册处理相关函数结束
**********************************************/

void terminal_type_save( uint16_t address, uint8_t tmnl_type, bool is_chman )
{
	int i = 0;

	for( i = 0; i < SYSTEM_TMNL_MAX_NUM; i++ )
	{
		if( ( address != 0xffff) && (address == (tmnl_addr_list[i].addr)))
		{
			terminal_pro_debug( "terminal(--%04x--) save type = %d ", address, tmnl_type );

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

	terminal_pro_debug( "new addr = %04x",  temp_addr );
	return temp_addr;
}

/*==================================================
					开始终端命令函数
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
	
	// 保存地址数据
	if( msg.cchdr.command_control & COMMAND_TMN_REPLY )
	{
		send_data_lng = 0;
		if( p_addr_list[p_allot->index].addr != 0xffff)
		{
			p_allot->renew_flag = 1;

			terminal_pro_debug( "man type = 0x%02x ", msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN );
			if( msg.cchdr.command_control & COMMAND_TMN_CHAIRMAN )
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_CHM_EXCUTE;
				terminal_pro_debug( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}
			else
			{
				p_addr_list[p_allot->index].tmn_type = TMNL_TYPE_COMMON_RPRST;
				terminal_pro_debug( "tmn type = %d ", p_addr_list[p_allot->index].tmn_type );
			}

			terminal_address_list tmp_addr;
			tmp_addr.addr = p_addr_list[p_allot->index].addr;
			tmp_addr.tmn_type = p_addr_list[p_allot->index].tmn_type;
			if( 1 == terminal_address_list_write_file( addr_file_fd, &tmp_addr, 1 ) )
			{// 正常保存，则开始注册终端,用于有终端链表但没有注册的节点
				gregister_tmnl_pro.tmn_total++;
#if 0
				if (terminal_register_pro_address_list_save( tmp_addr.addr, false ) )
				{
					//terminal_begin_register();
				}
				else
				{
					terminal_pro_debug( "allot address register Not begin!new allot address = --0x%04x--", tmp_addr.addr );
				}
#endif
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

		data_buf[0] = (uint8_t)((new_addr & 0x00ff) >> 0);// 低字节在前
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
	terminal_pro_debug( "key_num = %d, key_value = %d, tmnl_state = %d, sys_state = %d", key_num, key_value, tmnl_state, sys_state );

	if( (msg.cchdr.command_control & COMMAND_TMN_REPLY) == COMMAND_TMN_REPLY )
	{
		terminal_pro_debug( "key action command not valid!" );
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
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
	if( tmp == NULL )
	{
		terminal_pro_debug( "not found chairman conntrol address!" );
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
			if( gtmnl_signstate ) // 系统已经签到
			{
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
				terminal_chman_control_begin_vote( VOTE_MODE, false, &sign_flag );// 末次按键有效
			}
			else
			{
				sign_value = 1; // 未签到
				terminal_chairman_control_meeting( tmp->tmnl_dev.entity_id, addr, sign_value );
			}
			break;
		case CHM_END_VOTE:
			terminal_end_vote( 0, NULL, 0 );

			// 设置终端的状态
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0; 
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keyup = 0;
			terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST] );
			terminal_state_all_copy_from_common();

			// 设置lcd
			terminal_lcd_display_num_send( BRDCST_ALL, LCD_OPTION_CLEAR, VOTE_INTERFACE );
			// 设置led灯
			terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
			terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
			fterminal_led_set_send( BRDCST_ALL );

			// 发送表决结果
			terminal_broadcast_end_vote_result( msg.data&0x10?BRDCST_ALL: addr);
			
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
		case CHM_CLOSE_ALL_MIC:// 关闭所有普通代表机
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
					trans_model_unit_disconnect( query_tmp->tmnl_dev.entity_id, query_tmp );
					terminal_speak_track(query_tmp->tmnl_dev.address.addr, false );
				}
			}
			
			//terminal_mic_state_set( MIC_COLSE_STATUS, BRDCST_MEM,BRDCST_1722_ALL, false, NULL );
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
	thost_system_set set_sys; // 系统配置文件的格式
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
	if( special_event == SIGN_IN_SPECIAL_EVENT ) // 终端签到
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
	
	/* 设置终端的麦克风状态*/
	auto_close = set_sys.auto_close;
	for( i = 0; i < TMNL_TYPE_NUM; i++)
	{
		gtmnl_state_opt[i].auto_close = auto_close?1:0;
		gtmnl_state_opt[i].MicClose = MIC_CLOSE;
	}
	
	/*关闭所有麦克风，这里需要一个机制，即通道分配机制与麦克风设置机制(这时未实现10/29), 使用连接表管理系统的麦克风的连接状态，暂时未考虑同步的问题(11/4)*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && trans_model_unit_is_connected(tmnl_node->tmnl_dev.entity_id))
		{
			if (0 ==trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node ))
				terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );			
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/*主机发送状态, */
int terminal_main_state_send( uint16_t cmd, void *data, uint32_t data_len )
{
	tmnl_pdblist p_tmnl_list = NULL;	
	tmnl_main_state_send host_main_state;
	uint8_t spk_num = 0;
	uint8_t spk_common_num = 0;
	uint8_t apply_num = 0;
	thost_system_set set_sys; // 系统配置文件的格式

	terminal_pro_debug("conference_stype = %d", host_main_state.conference_stype);
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;	

	for( p_tmnl_list = dev_terminal_list_guard->next;p_tmnl_list != dev_terminal_list_guard; p_tmnl_list = p_tmnl_list->next )
	{
		if( p_tmnl_list->tmnl_dev.address.addr != 0xffff && (p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
			spk_num++;

		// 计算非主席发言人数
		if ( p_tmnl_list->tmnl_dev.address.addr != 0xffff && \
			p_tmnl_list->tmnl_dev.tmnl_status.is_rgst &&\
			p_tmnl_list->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST\
			&&(p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS))
		{
			spk_common_num++;
		}

		// 计算非主席发言人数
		if ( p_tmnl_list->tmnl_dev.address.addr != 0xffff && \
			p_tmnl_list->tmnl_dev.tmnl_status.is_rgst &&\
			(p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_FIRST_APPLY_STATUS||\
			p_tmnl_list->tmnl_dev.tmnl_status.mic_state == MIC_OTHER_APPLY_STATUS))
		{
			apply_num++;
		}

		// 计算申请人数
	}

#if 0
	if (gdisc_flags.speak_limit_num != spk_common_num)
		gdisc_flags.speak_limit_num = spk_common_num;

	if (gdisc_flags.apply_num != apply_num)
		gdisc_flags.apply_num = apply_num;
#endif
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	host_main_state.unit = gregister_tmnl_pro.tmn_total;
	host_main_state.camera_follow = set_sys.camara_track ? 1 : 0;
	host_main_state.chm_first = set_sys.temp_close ? 1 : 0;
	host_main_state.conference_stype = (set_sys.discuss_mode&0x07); // low 3bit change intime 2016-06-28
	host_main_state.limit = set_sys.speak_limit; 		// 讲话人数上限
	host_main_state.apply_set = set_sys.apply_limit;	// 申请人数上限
	host_main_state.spk_num = spk_num; // 当前讲话人数
	host_main_state.apply = gdisc_flags.apply_num;

	terminal_host_send_state( BRDCST_1722_ALL, host_main_state ); // target id is 0

	return 0;
}

/*终端发送显示屏号，可以继续完善11/2*/
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

/*暂定投票*/
int terminal_pause_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_SUSPEND_VOTE );

	return 0;
}

/*重新投票*/
int terminal_regain_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_RECOVER_VOTE );

#ifdef __MIND_UPPER_CMPT_SIGN_RESULT__
	gvote_flag = VOTE_SET;
	over_time_set( MIND_UPPER_CMPT_SIGN_RESULT, 500 );// 设置上报终端签到情况的初始超时时间
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

	if (APPLY_MODE == dis_mode)
	{
		terminal_chairman_apply_type_set( BRDCST_EXE );
	}
	else
	{
		terminal_chairman_apply_type_clear(BRDCST_EXE);
	}

	/*关闭所有麦克风*/
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		// 2.设置麦克风tarker的状态,上报麦克风状态, 设置相应终端的麦克风状态(会议主机与终端协议)
		if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && trans_model_unit_is_connected(tmnl_node->tmnl_dev.entity_id))
		{
			if (0 ==trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node))
				terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );
		}
	}

	/*发送主机状态*/
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

int terminal_speak_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )// 处理函数有待完善(11/4)
{
	thost_system_set set_sys;
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	while (set_sys.speak_limit > gdisc_flags.limit_num && \
		set_sys.discuss_mode == LIMIT_MODE && \
		gdisc_flags.apply_num > 0)
	{
		uint16_t current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if (addr_queue_delete_by_index (gdisc_flags.apply_addr_list, 
			&gdisc_flags.apply_num, gdisc_flags.currect_first_index))// 开启下一个申请话筒
		{
			tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr(current_addr);
			if (first_speak != NULL)
			{
				if (!trans_model_unit_is_connected(first_speak->tmnl_dev.entity_id))
				{
					if (0 == trans_model_unit_connect( first_speak->tmnl_dev.entity_id, first_speak ))
					{// connect success
						gdisc_flags.speak_limit_num++;
						terminal_apply_list_first_speak(first_speak);
					}
				}
			}
			else
			{
				terminal_pro_debug( " no such tmnl dblist node!");
			}
		}
		else
		{
			gdisc_flags.currect_first_index = 0;
		}
		
		gdisc_flags.limit_num++;
	}
	
	gdisc_flags.limit_num = set_sys.speak_limit;
	
	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

int terminal_apply_limit_num_set( uint16_t cmd, void *data, uint32_t data_len )
{
	thost_system_set set_sys;
	uint8_t apply_limt_num = *((uint8_t*)data);
	
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));

	/*更新数据库成功后设置*/
	gdisc_flags.apply_limit = apply_limt_num;
	gset_sys.apply_limit = apply_limt_num;

	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/* 终端发言限时的控制,由于终端不能控制麦克风的关闭与打开，
所以系统不仅仅是会议协议数据的发送，还需管理终端的发言时长;而发言时长由系统的超时机制进行管理,
因此只需在终端连接时设置连接表相应的超时时间即可*/
int terminal_limit_speak_time_set( uint16_t cmd, void *data, uint32_t data_len )
{
	tmnl_limit_spk_time spk_time;
	thost_system_set set_sys; // 系统配置文件的格式

	//terminal_pro_debug( "SIZE OF SYS SET = %d", sizeof(thost_system_set) );
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	spk_time.limit_time = set_sys.spk_limtime;
	speak_limit_time = (uint8_t)spk_time.limit_time;
	
	if( !set_sys.spk_limtime ) // 无限时
	{
		terminal_limit_spk_time( 0, BRDCST_ALL, spk_time );
	}
	else
	{
		uint16_t nolimit_addr = 0;
		uint16_t limit_addr = BRDCST_MEM; // 限时对所有普通代表有效
		tmnl_limit_spk_time nolimit_spk_time;
		
		if( set_sys.vip_limitime ) // vip 限时
		{
			limit_addr |= BRDCST_VIP;
		}
		else
		{
			nolimit_addr |= BRDCST_VIP;
		}
		
		if( set_sys.chman_limitime ) // 主席限时
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

	// 设置讨论的状态
	terminal_start_discuss( false );
	// 开始补签// 设置超时签到
	over_time_set( SIGN_IN_LATE_HANDLE, gsign_latetime * 60 * 1000 );
	gquery_svote_pro.running = false;

	return 0;
}

int terminal_end_vote( uint16_t cmd, void *data, uint32_t data_len )
{
	gvote_flag = NO_VOTE;// 结束投票

	// 结束投票结果的查询
	gquery_svote_pro.index = 0;
	gquery_svote_pro.running = false;
	host_timer_stop( &gquery_svote_pro.query_timer );

	return 0;
}

int termianal_music_enable( uint16_t cmd, void *data, uint32_t data_len )
{
	if( data_len != sizeof(uint8_t) || data == NULL)
		return -1;

	uint8_t value = *((uint8_t*)data);
	if (gset_sys.chman_music != value)
		gset_sys.chman_music = value;
	
	return 0;
}

int termianal_chairman_prior_set( uint16_t cmd, void *data, uint32_t data_len )
{
	if( data_len != sizeof(uint8_t) || data == NULL)
		return -1;

	uint8_t value = *((uint8_t*)data);
	if (gset_sys.chman_first != value)
		gset_sys.chman_first = value;
	return 0;
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

/******************************************
*Auther:YasirLiang
*Date:2016/3/18
*Name:termianal_temp_close_set
*Func:SET temp close flags
*Paramer:
*		cmd;data;data_len
*******************************************/
int termianal_temp_close_set( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data != NULL );
	if( data_len != sizeof(uint8_t) || data == NULL )
		return -1;
	
	uint8_t value = *((uint8_t*)data);
	if (gset_sys.temp_close != value)
		gset_sys.temp_close = value;
	
	return 0;
}

/******************************************
*Auther:YasirLiang
*Date:2016/3/18
*Name:termianal_camera_track_set
*Func:SET camera track flags
*Paramer:
*		cmd;data;data_len
*******************************************/
int termianal_camera_track_set( uint16_t cmd, void *data, uint32_t data_len )
{
	assert( data != NULL );
	if( data_len != sizeof(uint8_t) || data == NULL )
		return -1;

	uint8_t value = *((uint8_t*)data);
	if (gset_sys.camara_track != value)
		gset_sys.camara_track = value;
	terminal_main_state_send( 0, NULL, 0 );
	
	return 0;
}

/*==================================================
					结束终端命令函数
====================================================*/


/*===================================================
{@终端处理流程
=====================================================*/

int terminal_socroll_synch(void )
{
	terminal_option_endpoint( BRDCST_1722_ALL, CONFERENCE_BROADCAST_ADDRESS, OPT_TMNL_LED_DISPLAY_ROLL_SYNC );

	return 0;
}

void terminal_remove_unregitster( void ) // 这里没有清除终端地址文件以及内存终端列表里相应的内容
{
	tmnl_pdblist p_loop_node = dev_terminal_list_guard->next;
	tmnl_pdblist p_tmp_node = NULL;

#ifdef __DEBUG__
	show_terminal_dblist(dev_terminal_list_guard);
#endif

	for( ; p_loop_node != dev_terminal_list_guard; p_loop_node = p_tmp_node )
	{
		uint64_t id = p_loop_node->tmnl_dev.entity_id;
		p_tmp_node = p_loop_node->next;
		if(  p_loop_node->tmnl_dev.address.addr == 0xffff || !p_loop_node->tmnl_dev.tmnl_status.is_rgst )
		{
			delect_terminal_dblist_node( &p_loop_node );
			conference_transmit_model_node_destroy( id );
		}
	}

#ifdef __DEBUG__
	show_terminal_dblist(dev_terminal_list_guard);
#endif
}

// 需上报，且不是主席插话，才保存麦克风状态
void terminal_mic_state_set( uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node )
{
	assert( tmnl_node );
	terminal_pro_debug( "===========mic state = %d ============",  mic_status );

	if( (tmnl_node == NULL) && !(addr & BROADCAST_FLAG) )
	{
		terminal_pro_debug( "nothing to send to set mic status!");
		return;
	}

	terminal_set_mic_status( mic_status, addr, tarker_id );
	if( tmnl_node != NULL )
	{
		if(tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE &&\
			mic_status ==MIC_CHM_INTERPOSE_STATUS)
			upper_cmpt_report_mic_state( MIC_OPEN_STATUS, tmnl_node->tmnl_dev.address.addr );
		else
			upper_cmpt_report_mic_state( mic_status, tmnl_node->tmnl_dev.address.addr );
		
		if( is_report_cmpt && (mic_status != MIC_CHM_INTERPOSE_STATUS) )
		{
			tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
		}
	}
}

void	terminal_mic_state_set_send_terminal( bool send_tmnl,uint8_t mic_status, uint16_t addr, uint64_t tarker_id, bool is_report_cmpt, tmnl_pdblist tmnl_node )
{
        assert( tmnl_node );
	terminal_pro_debug( "===========mic state = %d ============",  mic_status );

	if( (tmnl_node == NULL) && !(addr & BROADCAST_FLAG) )
	{
		terminal_pro_debug( "nothing to send to set mic status!");
		return;
	}

        if (send_tmnl)
	    terminal_set_mic_status( mic_status, addr, tarker_id );
        
	if( tmnl_node != NULL )
	{
		if(tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE &&\
			mic_status ==MIC_CHM_INTERPOSE_STATUS)
			upper_cmpt_report_mic_state( MIC_OPEN_STATUS, tmnl_node->tmnl_dev.address.addr );
		else
			upper_cmpt_report_mic_state( mic_status, tmnl_node->tmnl_dev.address.addr );
		
		if( is_report_cmpt && (mic_status != MIC_CHM_INTERPOSE_STATUS) )
		{
			tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
		}
	}       
}

/*********************************************************
*writer:YasirLiang
*Date:2016/4/26
*Name:terminal_speak_limit_timeout_set
*Param:
*	p_tmnl_node:set timeout terminal node
*Retern Value:
*	None
*state:根据系统设置设置超时时间
***********************************************************/ 
int terminal_speak_limit_timeout_set( tmnl_pdblist p_tmnl_node )
{
	uint8_t spk_limit_time = gset_sys.speak_limit;
	bool vip_time_limit = gset_sys.vip_limitime?true:false;
	bool chm_time_limit = gset_sys.chman_limitime?true:false;
	int ret = -1;
	
	if( (p_tmnl_node != NULL) && (!spk_limit_time) )// 发言限时?
	{
		if( p_tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST	)// 普通代表?
		{
			host_timer_start( spk_limit_time*60*1000, &p_tmnl_node->tmnl_dev.spk_timeout );//  单位是分钟
		}
		else if( p_tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP)
		{
			if( vip_time_limit )
				host_timer_start( spk_limit_time*60*1000, &p_tmnl_node->tmnl_dev.spk_timeout );//  单位是分钟
		}
		else if( (p_tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(p_tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE) )
		{
			if( chm_time_limit )
				host_timer_start( spk_limit_time*60*1000, &p_tmnl_node->tmnl_dev.spk_timeout );//  单位是分钟
		}

		ret = 0;
	}

	return ret;
}

/*${Terminal::isMicQueueEmpty}..............................................*/
/*${Terminal::isMicQueueEmpty}*/
static bool Terminal_isMicQueueEmpty(Terminal_micQueue * const queue) {
    assert(queue != NULL);
    return queue->head == queue->trail;
}
/*${Terminal::isMicQueueFull}...............................................*/
/*${Terminal::isMicQueueFull}*/
static bool Terminal_isMicQueueFull(Terminal_micQueue * const queue) {
    assert(queue != NULL);
    return (queue->head+1) % queue->size == queue->trail;
}
/*${Terminal::postMicFiFo}..................................................*/
/*${Terminal::postMicFiFo}*/
static bool Terminal_postMicFiFo(Terminal_micQueue * const queue, 
                                     Terminal_mic node)
{
    assert(queue != NULL);
    if (!Terminal_isMicQueueFull(queue)) {
        queue->pBuf[queue->trail] = node;
        queue->trail = (queue->trail + 1U) % queue->size;
        return true;
    }
    return false;
}
/*${Terminal::popMicFiFo}..................................................*/
/*${Terminal::popMicFiFo}*/
static bool Terminal_popMicFiFo(Terminal_micQueue* const queue,
                                     Terminal_mic *node) 
{
    assert(queue != NULL);
    if (!Terminal_isMicQueueEmpty(queue)) {
        *node = queue->pBuf[queue->head];
        queue->head = (queue->head + 1U) % queue->size;
        return true;
    }
    return false;
}
/*${Terminal::micCallbackPro}...............................................*/
/*${Terminal::micCallbackPro}*/
void Terminal_micCallbackPro(void){
    static Terminal_mic l_setNode;
    static bool l_finish = true;
    tmnl_pdblist p;
    uint32_t curTime;

    if (l_finish) {/*finish proccess the head*/
        Terminal_popMicFiFo(&l_micQueue, &l_setNode);
    }

    curTime = get_current_time();
    p = l_setNode.node;
    if ((p != NULL)
          && ((curTime - l_setNode.timeTick) > MIC_SET_TIME_OUT))
    {
        terminal_set_mic_status(l_setNode.micState,
                            p->tmnl_dev.address.addr, p->tmnl_dev.entity_id);
        upper_cmpt_report_mic_state(l_setNode.micState, 
                            p->tmnl_dev.address.addr);
        terminal_main_state_send(0, NULL, 0);
        /*send twice before finishing pro main state send coding*/
        terminal_main_state_send(0, NULL, 0);
        l_setNode.node = NULL;
        l_finish = true;
    }
    else if (p != NULL) {
        l_finish = false;
    }
    else {
        /*no node in queue,do noting*/
    }
}

/*********************************************************
*writer:YasirLiang
*Date:2016/3/16
*Name:terminal_mic_status_set_callback
*Param:
*	connect_flag:mic connect flag
*	p_tmnl_node:callback terminal node
*Retern Value:
*	None
*state:打开或关闭终端mic的回调函数:设置mic的状态;若成功连接，设置发言的
*超时时间
***********************************************************/ 
void terminal_mic_status_set_callback( bool connect_flag, tmnl_pdblist p_tmnl_node )
{
        uint8_t mic_status;
        Terminal_mic micNode;
        
	assert( p_tmnl_node );
	if( p_tmnl_node == NULL )
		return;

        mic_status = connect_flag?MIC_OPEN_STATUS:MIC_COLSE_STATUS;
	if( (get_sys_state() == INTERPOSE_STATE) && p_tmnl_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE)
	{
                /*interpose state should not save mic state if endpiot
                  *is excute chairman.
                  */
        }
	else if((get_sys_state() == INTERPOSE_STATE) && p_tmnl_node->tmnl_dev.address.tmn_type != TMNL_TYPE_CHM_EXCUTE)
	{
                /*interpose state should will save mic state if endpiont
                  *is common and temp close is set.
                  */
                if (!gset_sys.temp_close)
                {
                       p_tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
                }
	}
	else if((get_sys_state() != INTERPOSE_STATE))
	{
		p_tmnl_node->tmnl_dev.tmnl_status.mic_state = mic_status;
	}
        else
        {
                /*will never came this else*/
        }

	/**
	*change data: 26-4-2016
	*1、根据系统的设置，设置超时时间
	*2、此时修改tmnl_pdblist的结构体，
	*	增加超时发言元素
	*/
	if( connect_flag )
		terminal_speak_limit_timeout_set( p_tmnl_node );

	if( !connect_flag && p_tmnl_node != NULL )// 停止计时
		host_timer_stop( &p_tmnl_node->tmnl_dev.spk_timeout );

        upper_cmpt_report_mic_state(mic_status, 
                            p_tmnl_node->tmnl_dev.address.addr);/*report*/

        micNode.micState = mic_status;
        micNode.node = p_tmnl_node;
        micNode.setCount = 1U;
        micNode.timeTick = get_current_time();
        Terminal_postMicFiFo(&l_micQueue, micNode);
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
*state:管理系统终端的发言时长，到时断开连接，
*	单位是分钟;超时检查从讨论开始; 终端限时发言的系统超时管理机制的接口函数
*Change Date:26-4-2016
*Context:与函数terminal_mic_status_set_callback(设置超时时间)共同实现发言的超时机制
*注:在协议下的处理方式是不用调用此函数，见函数terminal_key_discuccess的协议
*注释部分(// 见协议命令bit3-bit0=1000:关闭麦克风)
***********************************************************/ 
int terminal_mic_speak_limit_time_manager_event( void )
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;

	/*查看会讨单元链表，并检查超时*/
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
	{
		if( (tmnl_node != NULL) && host_timer_timeout(&tmnl_node->tmnl_dev.spk_timeout) )
		{// timer out
			trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node );
			terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );
		}
	}

	return 0;
}

/* 设置终端开始讨论的状态*/
int terminal_start_discuss( bool mic_flag )
{
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	tmnl_pdblist  tmnl_node = dev_terminal_list_guard->next;
	if( !mic_flag ) // 关闭所有麦克风
	{	
		/*关闭所有麦克风*/
		for( ; tmnl_node != dev_terminal_list_guard; tmnl_node = tmnl_node->next )
		{
			if( tmnl_node->tmnl_dev.tmnl_status.is_rgst && (tmnl_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS) ) // 断开注册连接的终端
			{
				if (0 == trans_model_unit_disconnect( tmnl_node->tmnl_dev.entity_id, tmnl_node ))
					terminal_speak_track(tmnl_node->tmnl_dev.address.addr, false );
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
		gtmnl_state_opt[i].sys = TMNL_SYS_STA_DISC; // 讨论模式
	} 

	/* 设置终端状态*/
	if( APPLY_MODE == set_sys.discuss_mode )
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
		terminal_chairman_apply_type_set( BRDCST_EXE );
	}
	else
	{
		terminal_state_set_base_type( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE,gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);	// 根据终端类型设置终端的状态
		terminal_lcd_display_num_send( BRDCST_MEM |BRDCST_VIP|BRDCST_CHM|BRDCST_EXE, LCD_OPTION_CLEAR, glcd_num );// 发送lcd显示屏号
	}

	/*设置终端指示灯*/
	terminal_led_set_save( BRDCST_ALL, TLED_KEY1, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY2, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY3, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY4, TLED_OFF );
	terminal_led_set_save( BRDCST_ALL, TLED_KEY5, TLED_OFF );
	fterminal_led_set_send( BRDCST_ALL );

	terminal_main_state_send( 0, NULL, 0 );

	return 0;
}

/*依据类型设置终端的状态*/
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

/*主席申请类型终端状态设置*/
void terminal_chairman_apply_type_set( uint16_t addr )
{
	if( addr == 0xffff )
		return;
	
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0x0e; // 2 3 4键按下有效
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_DISPLAY, CHM_APPROVE_APPLY_INTERFACE );
}

void terminal_chairman_apply_type_clear( uint16_t addr )
{
	if( addr == 0xffff )
		return;
	
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keydown = 0; 
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].keyup = 0;
	gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE].sys = TMNL_SYS_STA_DISC;

	terminal_state_set_base_type( addr, gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE] );
	terminal_lcd_display_num_send( addr, LCD_OPTION_CLEAR, CHM_APPROVE_APPLY_INTERFACE );
}

/*保存led指示灯的状态*/
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

/*发送led指示灯的状态*/ 
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

/*处理上位机对麦克风的操作命令(打开与关闭)*/
int terminal_upper_computer_speak_proccess( tcmpt_data_mic_switch mic_flag )
{
	uint16_t addr = (((uint16_t)(mic_flag.addr.low_addr << 0)) | ((uint16_t)(mic_flag.addr.high_addr << 0)));
	uint8_t mic_state_set = mic_flag.switch_flag;
	ttmnl_discuss_mode dis_mode = gdisc_flags.edis_mode;
	tmnl_pdblist speak_node = NULL;
	uint8_t limit_time = 0;
	bool found_node = false;
	bool read_success = false;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	
	read_success = true;
	limit_time = set_sys.spk_limtime;
	
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return -1;
	
	terminal_pro_debug( " speak addr = %04x, discuccess mode = %d", addr, dis_mode );
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
		if( dis_mode == PPT_MODE ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
		{
			if( mic_state_set )
			{
				if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
				{// already connected
					if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
						speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;					
				}
				else
				{
					/*主席在发言上限里*/
					uint16_t speak_num = terminal_speak_num_count();
					int ret_cnnt = -1;
					if (speak_num < gdisc_flags.limit_num)
					{
						ret_cnnt = trans_model_unit_connect(speak_node->tmnl_dev.entity_id, speak_node);
					}
					
					if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt == 0))
					{
						terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					}
					else if ((speak_num < gdisc_flags.limit_num) ||ret_cnnt != -2)
					{//has timeout for operation transmit ouput channel
						/*
						  *1\断开连接时间最长的
						  *2\超时后连接
						  */
						tmnl_pdblist disconnect_lnode = NULL;
						uint64_t id = 0;
						int ret = -1;
						bool if_common = false;

						ret = trans_model_unit_disconnect_longest_connect_re_id_cfcnode(&id, &disconnect_lnode);
						if (0 == ret)
						{
							if (disconnect_lnode != NULL && \
								disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
							else if (disconnect_lnode == NULL && id != 0)
							{
								disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
								if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
								{
									if_common = true;
								}
							}

							if (if_common)
							{
								if (dis_mode == FIFO_MODE)
									addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
								else if (dis_mode == LIMIT_MODE)
									gdisc_flags.speak_limit_num--;
							}

							terminal_over_time_speak_node_set(speak_node);
							if (NULL != disconnect_lnode)
								DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
									disconnect_lnode->tmnl_dev.address.tmn_type, 
									disconnect_lnode->tmnl_dev.address.addr, 
									gdisc_flags.speak_limit_num);
						}
					}
					else if (speak_num >= gdisc_flags.limit_num)
					{
						/*
						  *1\断开连接时间最长的
						  *2\超时后连接
						  */
						tmnl_pdblist disconnect_lnode = NULL;
						uint64_t id = 0;
						int ret = -1;
						bool if_common = false;

						ret = trans_model_unit_disconnect_longest_connect_re_id_cfcnode(&id, &disconnect_lnode);
						if (0 == ret)
						{
							if (disconnect_lnode != NULL && \
								disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
							else if (disconnect_lnode == NULL && id != 0)
							{
								disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
								if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
								{
									if_common = true;
								}
							}

							if (if_common)
							{
								if (dis_mode == FIFO_MODE)
									addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
								else if (dis_mode == LIMIT_MODE)
									gdisc_flags.speak_limit_num--;
							}

							terminal_over_time_speak_node_set(speak_node);
							if (NULL != disconnect_lnode)
								DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
									disconnect_lnode->tmnl_dev.address.tmn_type, 
									disconnect_lnode->tmnl_dev.address.addr, 
									gdisc_flags.speak_limit_num);
						}
					}
				}
			}
			else
			{
				if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
				{
					if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
					{
						terminal_speak_track(speak_node->tmnl_dev.address.addr, false );

						/*主席\vip控制限制模式下的普通终端的mic状态*/
						if (dis_mode == LIMIT_MODE)
						{
							terminal_chman_vip_control_common_mic();
						}
					}
				}
			}
		}
		else
		{
			switch( dis_mode )
			{
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
					terminal_pro_debug( " out of discuss mode bound!" );
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
	
	fd = Fopen( STSTEM_SET_STUTUS_PROFILE, "rb" ); // 只读读出数据
	if( NULL == fd )
	{
		terminal_pro_debug( "mian state send ->open files %s Err!",  STSTEM_SET_STUTUS_PROFILE );
		return false;
	}

	ret = true;
	memset( &tmp_set_sys, 0, sizeof( thost_system_set));
	if( profile_system_file_read( fd, &tmp_set_sys ) == -1)
	{
		terminal_pro_debug( "Read profile system Err!" );
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
	
	if( mic_flag ) // 打开麦克风
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			if( -1 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
					terminal_mic_status_set_callback( false, speak_node );
			
			terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
		}
	}
	else
	{
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
		terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
	}

	terminal_main_state_send( 0, NULL, 0 );
}

bool terminal_limit_disccuss_mode_cmpt_pro( uint8_t mic_flag, uint8_t limit_time, tmnl_pdblist speak_node )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	uint16_t current_addr = 0;
	uint8_t cc_state = 0;
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	
	if( speak_node == NULL )
	{
		return ret;
	}

	if( mic_flag ) // 打开麦克风
	{
		uint16_t speak_num = terminal_speak_num_count();
		if( speak_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
		{
			ret = true;
		}
		else if( gdisc_flags.speak_limit_num < gdisc_flags.limit_num \
			&& speak_num < gdisc_flags.limit_num) // 打开麦克风
		{
			if(0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_limit_num++;
				ret = true;
			}
		}
		else if( gdisc_flags.apply_num < gdisc_flags.apply_limit ) // 申请发言
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
		if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
		{
			terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
		}
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );
			terminal_mic_state_set(MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
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

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
						if (0 == trans_model_unit_connect( first_speak->tmnl_dev.entity_id, first_speak ))
						{// connect success
							gdisc_flags.speak_limit_num++;
							terminal_apply_list_first_speak(first_speak);
						}
						else
						{
							/*
							  *当前的mic断开成功后会留出一个通道
							  */
							terminal_over_time_speak_node_set(first_speak);
						}
					}
					else
					{
						terminal_pro_debug( " no such tmnl dblist node!");
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
	int dis_ret = -1;
	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	uint16_t speak_num = terminal_speak_num_count();

	if( speak_node == NULL )
	{
		terminal_pro_debug( " NULL speak node!" );
		return false;
	}

	if( mic_flag ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				ret = true;
			}
			else
			{
				if (0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
				{
					terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					ret = true;
				}
			}
		}
		else if( speak_limit_num < gdisc_flags.limit_num \
			&& speak_num < gdisc_flags.limit_num)
		{
			if ( 0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_addr_list[speak_limit_num] = addr;
				gdisc_flags.speak_limit_num++;
				ret = true;
			}			
		}
		else // 发言人数大于或等于限制人数
		{
			if (!trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
					if( first_speak != NULL )
					{
						if (trans_model_unit_is_connected(first_speak->tmnl_dev.entity_id))
						{
							dis_ret = trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id, first_speak );
							if (0 == dis_ret)
							{
								terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
								addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
								terminal_over_time_speak_node_set(speak_node);
							}
						}
					}
					else
					{
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
			}
			else
			{
				if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
			}
		}
	}
	else
	{
		if ( 0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
		{
			addr_queue_delect_by_value( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr );
			terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
			ret = true;
		}
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

	if( mic_flag ) // 申请发言,加地址入申请列表
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
	else // 取消申请发言
	{
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{
			terminal_mic_state_set( MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );// 上报mic状态
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
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
		{// 关闭话筒
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
				terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
			}
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

tmnl_pdblist found_terminal_dblist_node_by_endtity_id(const uint64_t tarker_id)
{
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return NULL;
	
	tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
	for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
	{
		if (tmp_node->tmnl_dev.entity_id == tarker_id)
			return tmp_node;
	}

	return NULL;
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

void terminal_select_apply( uint16_t addr ) // 使选择的申请人是首位申请人
{
	tmnl_pdblist apply_first = NULL; 
	uint8_t apply_index = MAX_LIMIT_APPLY_NUM;
	int i = 0;
	
	if( addr != gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]) // 不是首位申请
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

typedef struct TMicStateSetPro
{
	bool running;
	tmnl_pdblist micNode;
        bool sendUpperCmpt;
}TMicStateSetPro;

TMicStateSetPro gmicSetPro;
void terminal_after_time_mic_state_node_set( tmnl_pdblist speak_node, uint8_t mic_status, bool send_upper_cmpt)
{
	if ((!gmicSetPro.running) && (NULL != speak_node) )
	{
		gmicSetPro.micNode = speak_node;
                gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state = mic_status;
		gmicSetPro.running = true; 
                gmicSetPro.sendUpperCmpt = send_upper_cmpt;
		over_time_set( MIC_SET_AFTER, 200);
	}
}

void terminal_after_time_mic_state_pro(void)
{
        if ((gmicSetPro.running) && over_time_listen(MIC_SET_AFTER))
        {
        	if( gmicSetPro.micNode != NULL )
        	{
        	        terminal_set_mic_status( gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state,
                        gmicSetPro.micNode->tmnl_dev.address.addr, 
                        gmicSetPro.micNode->tmnl_dev.entity_id);
        	}

                if (gmicSetPro.sendUpperCmpt)
                {
                        upper_cmpt_report_mic_state( gmicSetPro.micNode->tmnl_dev.tmnl_status.mic_state,
                                                                gmicSetPro.micNode->tmnl_dev.address.addr);
                }
                
                gmicSetPro.running = false;
                over_time_stop(MIC_SET_AFTER);
        }
}

bool terminal_examine_apply( enum_apply_pro apply_value )// be tested in 02-3-2016,passed
{
	uint16_t addr = 0;
	tmnl_pdblist apply_first = NULL;
	bool ret = false;
	thost_system_set set_sys; // 系统配置文件的格式
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
					//terminal_pro_debug( "set apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
					terminal_mic_state_set( MIC_COLSE_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
					if( gdisc_flags.apply_num > 0 )
					{
						gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
						apply_first = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
						if( apply_first != NULL )
						{
							//terminal_pro_debug( "set FIRST apply addr = 0x%04x", apply_first->tmnl_dev.address.addr );
							//terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
                                                        terminal_after_time_mic_state_node_set(apply_first, MIC_FIRST_APPLY_STATUS, true);
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
				terminal_pro_debug( "delect apply add = 0x%04x failed: no such address", addr );
			}
			break;
		case NEXT_APPLY: // be tested in 02-3-2016,passed
			if( gdisc_flags.apply_num > 1 )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr );
				if( apply_first != NULL )
				{
					//terminal_pro_debug( "set apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
					terminal_mic_state_set( MIC_OTHER_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );

					gdisc_flags.currect_first_index++;
					gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
					addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
					apply_first = found_terminal_dblist_node_by_addr( addr );
					if( apply_first != NULL )
					{
						//terminal_pro_debug( "set FIRST apply addr = 0x%04x----0x%04x", apply_first->tmnl_dev.address.addr, addr );
						//terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
                                                terminal_after_time_mic_state_node_set(apply_first, MIC_FIRST_APPLY_STATUS, true);
                                        }
					else
					{
						terminal_pro_debug( "no found first apply node!" );
					}
				}
				else
				{
					terminal_pro_debug( "no found first apply node!" );
				}

				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			break;
		case APPROVE_APPLY:// be tested in 02-3-2016,passed
			if( gdisc_flags.currect_first_index < gdisc_flags.apply_num )
			{
				addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
				apply_first = found_terminal_dblist_node_by_addr( addr ); // 打开第一个申请的麦克风
				if( apply_first != NULL )
				{
					if (0 == trans_model_unit_connect( apply_first->tmnl_dev.entity_id, apply_first ))
					{// 连接成功
						terminal_speak_track(apply_first->tmnl_dev.address.addr, true );
						addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index );
						if( gdisc_flags.apply_num > 0 )
						{
							gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
							apply_first = found_terminal_dblist_node_by_addr(gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
							if( apply_first != NULL )
							{
								//terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, apply_first->tmnl_dev.address.addr, apply_first->tmnl_dev.entity_id, true, apply_first );
								terminal_after_time_mic_state_node_set(apply_first, MIC_FIRST_APPLY_STATUS, true);

							}
						}
						else
						{
							gdisc_flags.currect_first_index = gdisc_flags.apply_limit;
						}

						terminal_main_state_send( 0, NULL, 0 );
						ret = true;
					}
				}
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
	
	terminal_pro_debug( "terminal type set addr = 0x%04x ", addr );
	tmnl_node = found_terminal_dblist_node_by_addr( addr );
	if( tmnl_node == NULL )
	{
		terminal_pro_debug( "no such type addr found!");
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

	// 保存到地址文件address.dat
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
	if( terminal_address_list_read_file( fd, addr_list ) == -1 ) // 读取失败
	{
		terminal_pro_debug( "type save read file Err!");
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
		terminal_pro_debug( "not found addr in the address profile !" );
		return -1;
	}

	if( Fseek( fd, 2, SEEK_SET ) == -1 )
	{
		return -1;
	}

	// 读写检验
	uint16_t crc;
	Fread( fd, &crc, sizeof(uint16_t), 1 );
	crc -= type_tmp; // 减原来的类型
	crc += tmnl_type; // 新校验
	
	if( Fseek( fd, 2, SEEK_SET ) == -1 ) // 检验的偏移
	{
		return -1;
	}
	Fwrite( fd, &crc, sizeof(uint16_t), 1 );
	
	// 写数据
	if( Fseek( fd, index*sizeof(terminal_address_list) + 4/*文件头大小*/, SEEK_SET ) == -1 )
	{
		return -1;
	}

	if( Fwrite( fd, &tmnl_type, sizeof( uint16_t), 1 ) != 1)
	{
		terminal_pro_debug( "write terminal type Err!" );
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

	//terminal_pro_debug( "=====>>>>>>>>>>>> messsag target addr = %04x<<<<<<<<<<<<<<==========", addr );
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

void terminal_tablet_stands_manager( tcmpt_table_card *table_card, uint16_t addr, uint16_t contex_len )// 桌牌管理
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
	else if( card_flag == 1 )// 设置led显示方式
	{
		memcpy( &card_opt, table_card->msg_buf, sizeof(uint16_t));

		tmnl_pdblist tmp = found_terminal_dblist_node_by_addr( addr );
		if( tmp == NULL )
		{
			terminal_set_led_play_stype( BRDCST_1722_ALL, addr, card_opt );// 设置led显示方式
		}
		else
		{
			terminal_set_led_play_stype( tmp->tmnl_dev.entity_id, addr, card_opt );
		}
	}
}

// 开始签到
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
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);
	
	// 设置查询签到投票结果(2016-1-27添加)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
}

// 主席控制终端签到
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
		gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].sign_stype = sign_type? CARD_SIGN_IN : KEY_SIGN_IN; // 1插卡；0按键
	}

	terminal_state_set_base_type( BRDCST_ALL, gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST]);

	// 设置查询签到投票结果(2016-1-28添加)
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
	
	*sign_flag = gsigned_flag ? 0 : 1;// 响应0，正常;响应1异常 
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
			if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
			}
			else
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
			}
		}
	}

	terminal_vote_state_set( BRDCST_ALL );

	// 设置查询签到投票结果 (2016-1-27)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
	
#ifdef __MIND_UPPER_CMPT_SIGN_RESULT__
	over_time_set( MIND_UPPER_CMPT_SIGN_RESULT, 500 );// 设置上报终端签到情况的初始超时时间
#endif
}

void terminal_chman_control_begin_vote(  uint8_t vote_type, bool key_effective, uint8_t* sign_flag )
{// yasir tested in 2016-4-11
	tmnl_pdblist tmp = NULL;
	
	assert( sign_flag );
	if( sign_flag == NULL )
		return;
	
	gfirst_key_flag = key_effective; // true = 首次按键有效；
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
#if 0 // yasir change in 2016-4-11
		if( tmp->tmnl_dev.tmnl_status.is_rgst || tmp->tmnl_dev.address.addr )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
		}
		else
		{
			tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
		}
#else
		if( tmp->tmnl_dev.tmnl_status.is_rgst && (tmp->tmnl_dev.address.addr != 0xffff))
		{
			if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )// 已签到
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TWAIT_VOTE_FLAG;
			}
			else
			{
				tmp->tmnl_dev.tmnl_status.vote_state = TVOTE_SET_FLAG; // 未签到不能投票
			}
		}
#endif
	}

	terminal_vote_state_set( BRDCST_ALL );

	// 设置查询签到投票结果 (2016-4-8)
	gquery_svote_pro.running = true;
	gquery_svote_pro.index = 0;
	host_timer_start( 500, &gquery_svote_pro.query_timer );
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0e; // 2 3 4 键
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x03;// 1 2 键
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x07;// 1 2 3 键
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x0f; // 1 2 3 键
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
			gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST].keydown = 0x1f; // 1 2 3 4 5 键
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
			terminal_pro_debug( "out of bround : vote of type!");
			break;
	}

	terminal_state_all_copy_from_common();
}

void terminal_sign_in_special_event( tmnl_pdblist sign_node ) // 终端特殊事件-签到
{
	assert( sign_node );
	if( sign_node == NULL )
		return;
	
	if( gtmnl_signstate == SIGN_IN_ON_TIME )// 设置签到标志
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = TMNL_SIGN_ON_TIME;
	}
	else if( (gtmnl_signstate == SIGN_IN_BE_LATE) && (sign_node->tmnl_dev.tmnl_status.sign_state == TMNL_NO_SIGN_IN) )
	{
		sign_node->tmnl_dev.tmnl_status.sign_state = SIGN_IN_BE_LATE;
	}
	
	// 设置投票使能
	termianl_vote_enable_func_handle( sign_node );
	// 上报签到情况
	terminal_pro_debug( "sign state = %d", sign_node->tmnl_dev.tmnl_status.sign_state );
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

	terminal_pro_debug( "sign num = %d", sign_num );
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
	
	sign_node->tmnl_dev.tmnl_status.vote_state |= TVOTE_EN; // TVOTE_SET_FLAG ->TVOTE_EN ->TWAIT_VOTE_FLAG(投票状态流程)
}

void terminal_state_all_copy_from_common( void )
{
	memcpy( &gtmnl_state_opt[TMNL_TYPE_VIP], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_COMMON], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
	memcpy( &gtmnl_state_opt[TMNL_TYPE_CHM_EXCUTE], &gtmnl_state_opt[TMNL_TYPE_COMMON_RPRST], sizeof(tmnl_state_set));
}

void terminal_broadcast_end_vote_result( uint16_t addr ) // 根据终端的2 3 4键统计结果
{// yasir tested in 2016-4-11
	assert( dev_terminal_list_guard );
	if( dev_terminal_list_guard == NULL )
		return;
	
	tmnl_pdblist tmp = NULL, head_list = dev_terminal_list_guard;
	uint16_t vote_total = 0, neg = 0, abs = 0, aff = 0;
	tmnl_vote_result vote_rslt;

	for( tmp = head_list->next; tmp != head_list; tmp = tmp->next )
	{
		if( (tmp->tmnl_dev.address.addr == 0xffff) || (!tmp->tmnl_dev.tmnl_status.is_rgst) )
		{
			continue;
		}

		if( tmp->tmnl_dev.tmnl_status.sign_state != TMNL_NO_SIGN_IN )
		{
			vote_total++;
		}

		if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY2_ENABLE )// 2键按下 ,反对
		{
			neg++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY3_ENABLE )// 3键按下 ,弃权
		{
			abs++;
		}
		else if( (tmp->tmnl_dev.tmnl_status.vote_state & TVOTE_KEY_MARK) == TVOTE_KEY4_ENABLE )// 4键按下 ,赞成
		{
			aff++;
		}
	}

	vote_rslt.total = vote_total;
	vote_rslt.neg = neg;
	vote_rslt.abs = abs;
	vote_rslt.aff = aff;

	gp_log_imp->log.post_log_msg( &gp_log_imp->log, LOGGING_LEVEL_INFO, "------Vote Info------\nvote_total = %d	neg = %d	abs = %d	aff = %d\n", 
								vote_total, 
								neg,
								abs,
								aff );
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
	if( gvote_flag == NO_VOTE || ( key_num > 5 ) ) // 见协议(2.	终端按键的编号：表决键1~5，发言键6，主席优先键7)
	{
		terminal_pro_debug( "system not ready to vote or key num out of vote key num!" );
		return;
	}

	tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		terminal_pro_debug( "no such tmp_node: addr = %04x", addr );
		return;
	}

	// 保存key值
	if( terminal_key_action_value_judge_can_save( key_num,  tmp_node ) ) 
	{// 特殊响应2
		terminal_key_action_host_special_num2_reply( recvdata, 0, 0, 0, VOID_VOTE_INTERFACE ,tmp_node );
	}
	else
	{
		terminal_key_action_host_common_reply( recvdata, tmp_node );
	}

	upper_cmpt_vote_situation_report( tmp_node->tmnl_dev.tmnl_status.vote_state, tmp_node->tmnl_dev.address.addr );

	// 检查是否所有投票完成
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

	terminal_pro_debug( "vote num = %d", vote_num );
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
			*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
			ret = true;
			break;
		case SLCT_2_2:
		case SLCT_3_2:
		case SLCT_4_2:
		case SLCT_5_2:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 2 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_3_3:
		case SLCT_4_3:
		case SLCT_5_3:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 3 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_4_4:
		case SLCT_5_4:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 4 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
				ret = true;
			}
			break;
		case SLCT_5_5:
			*p_vote_state |= (1<< ( key_num -1));
			if(COUNT_KEY_DOWN_NUM( *p_vote_state & TVOTE_KEY_MARK ) >= 5 )
			{
				*p_vote_state &= (~TWAIT_VOTE_FLAG);// 设置结束标志
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
	reply_data.key_led = key_led & 0x03ff;// 低十位
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
		terminal_pro_debug( "no such tmp_node: addr = %04x", addr );
		return;
	}

	if( key_num == KEY6_SPEAK )
	{
	        uint32_t cur_time = get_current_time();
	        if ((cur_time - tmp_node->tmnl_dev.spk_operate_timp) < SPK_KEY_OPT_TIME)
        	{
        		DEBUG_INFO("speaking key press not time out, do not do that fast");
        		terminal_key_action_host_special_num1_reply( recvdata, tmp_node->tmnl_dev.tmnl_status.mic_state, tmp_node );
        		tmp_node->tmnl_dev.spk_operate_timp = cur_time;
                        return;
        	}
            
		if( key_value )
		{
			// no limit time
			bool speak_track = true;
			if (!trans_model_unit_is_connected(tmp_node->tmnl_dev.entity_id))
			{
				if (0 !=trans_model_unit_connect(tmp_node->tmnl_dev.entity_id, tmp_node))
				{
					terminal_key_action_host_special_num1_reply( recvdata, MIC_COLSE_STATUS, tmp_node );
					speak_track = false;
				}
				else
					terminal_key_action_host_special_num1_reply( recvdata, MIC_OPEN_STATUS, tmp_node );
			}
			else
				terminal_key_action_host_special_num1_reply( recvdata, MIC_OPEN_STATUS, tmp_node );

			if (speak_track)
				terminal_speak_track(tmp_node->tmnl_dev.address.addr, true);
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recvdata, MIC_COLSE_STATUS, tmp_node );
			if (0 == trans_model_unit_disconnect(tmp_node->tmnl_dev.entity_id, tmp_node))// disconnect success!
				terminal_speak_track(tmp_node->tmnl_dev.address.addr, false );
		}
	}
}

void terminal_key_action_chman_interpose( uint16_t addr, uint8_t key_num, uint8_t key_value, uint8_t tmnl_state, const uint8_t recvdata )
{
	if( key_num != KEY7_CHAIRMAN_FIRST )
	{
		terminal_pro_debug( "not valid chairman interpose value key " );
		return;
	}

	tmnl_pdblist tmp_node = found_terminal_dblist_node_by_addr( addr );
	if( tmp_node == NULL )
	{
		terminal_pro_debug( "no such 0x%04x addr chairman!", addr );
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
	tcmpt_data_mic_status mic_list[SYSTEM_TMNL_MAX_NUM]; // 100-临时发言总人数
	uint16_t report_mic_num = 0;
	thost_system_set set_sys; // 系统配置文件的格式
	int dis_ret = -1;
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

	terminal_pro_debug( "system mode = %d", get_sys_state());
	if( (get_sys_state() != INTERPOSE_STATE) && key_down )
	{
		bool tmp_close = false; // temp close
		
		set_terminal_system_state( INTERPOSE_STATE, true );
		terminal_pro_debug( "system mode = %d", get_sys_state());
		gchm_int_ctl.is_int = true;
		gchm_int_ctl.chmaddr = addr;
		tmp_close = (set_sys.temp_close != 0)?true:false; 

		/**
		 *2015-12-11
		 *打开主席mic,不保存状态
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS )
		{
			dis_ret = trans_model_unit_connect( chman_node->tmnl_dev.entity_id, chman_node );
			terminal_speak_track(chman_node->tmnl_dev.address.addr, true );
			terminal_key_action_host_special_num1_reply( recvdata,(dis_ret != -1)?MIC_CHM_INTERPOSE_STATUS:MIC_COLSE_STATUS, chman_node );// 设置主席mic状态
			if( dis_ret != -1 )
			{
				terminal_mic_state_set( MIC_CHM_INTERPOSE_STATUS, BRDCST_ALL, BRDCST_1722_ALL, true, chman_node );
			}
		}
		else 
		{
			terminal_key_action_host_special_num1_reply( recvdata, MIC_CHM_INTERPOSE_STATUS, chman_node );// 设置主席mic状态
		}

		assert( dev_terminal_list_guard );
		if( dev_terminal_list_guard == NULL )
			return;
		
		tmnl_pdblist tmp_node = dev_terminal_list_guard->next;
		for( ; tmp_node != dev_terminal_list_guard; tmp_node = tmp_node->next )
		{
			if( tmp_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST )
			{// 关闭所有普通代表机
				if( tmp_node->tmnl_dev.tmnl_status.mic_state != MIC_COLSE_STATUS )
				{
					trans_model_unit_disconnect( tmp_node->tmnl_dev.entity_id, tmp_node );
					terminal_speak_track(tmp_node->tmnl_dev.address.addr, false );
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

		// 上报mic状态
		cmpt_miscrophone_status_list_from_set( mic_list, report_mic_num );
		
		gdisc_flags.apply_num = 0;
		gdisc_flags.speak_limit_num = 0;		
	}
	else if( !key_down )
	{
		set_terminal_system_state( INTERPOSE_STATE, false );
		terminal_pro_debug( "system mode = %d Mic state = %d", get_sys_state(), chman_node->tmnl_dev.tmnl_status.mic_state );
		terminal_key_action_host_special_num1_reply( recvdata, chman_node->tmnl_dev.tmnl_status.mic_state, chman_node );// 设置主席mic状态

		/**
		 *2015-12-11
		 *若主席mic的上一个状态是打开的状态不去管它
		 *否则断开其mic，而不重新设置保存，若断开成功在此时主席的mic状态是close状态
		 * 因为在非插入状态，mic的状态会被成功的回调保存(见terminal_mic_status_set_callback)
		 */
		if( chman_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS )
		{
			dis_ret = trans_model_unit_disconnect( chman_node->tmnl_dev.entity_id, chman_node );
			terminal_speak_track(chman_node->tmnl_dev.address.addr, false );
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
				trans_model_unit_connect( end_node->tmnl_dev.entity_id, chman_node );
				terminal_speak_track(end_node->tmnl_dev.address.addr, true );
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
		terminal_pro_debug( "not found discuccess termianl key!" );
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
	else if( key_num == 0 && key_value )
	{// 见协议命令bit3-bit0=1000:关闭麦克风
		terminal_key_speak_proccess( dis_node, false, recv_msg );
	}

	return 0;
}

void terminal_chman_vip_control_common_mic(void)
{
	if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
	{
		uint16_t current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if (addr_queue_delete_by_index(gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
		{
			tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
			if( first_speak != NULL )
			{
				terminal_over_time_speak_node_set(first_speak);
			}
		}
		else
		{
			gdisc_flags.currect_first_index = 0;
		}
	}
}

uint16_t terminal_speak_num_count(void)
{
	assert(dev_terminal_list_guard);
	if( dev_terminal_list_guard == NULL )
		return 0;

	tmnl_pdblist end_node = dev_terminal_list_guard->next;
	uint16_t speak_num = 0;
	for ( ; end_node != dev_terminal_list_guard; end_node = end_node->next )
	{
		if (end_node->tmnl_dev.tmnl_status.mic_state == MIC_OPEN_STATUS)
		{
			speak_num++;
		}
	}

	return speak_num;
}

bool terminal_key_speak_proccess( tmnl_pdblist dis_node, bool key_down, uint8_t recv_msg )
{
	uint8_t dis_mode = gdisc_flags.edis_mode;
	thost_system_set set_sys; // 系统配置文件的格式
	memcpy( &set_sys, &gset_sys, sizeof(thost_system_set));
	uint32_t cur_time = get_current_time();

	if( dis_node == NULL )
		return false;

	if( !dis_node->tmnl_dev.tmnl_status.is_rgst )
		return false;
	
	if ((cur_time - dis_node->tmnl_dev.spk_operate_timp) < SPK_KEY_OPT_TIME)
	{
		DEBUG_INFO("speaking key press not time out, do not do that fast");
		terminal_key_action_host_special_num1_reply( recv_msg, dis_node->tmnl_dev.tmnl_status.mic_state, dis_node );
		dis_node->tmnl_dev.spk_operate_timp = cur_time;
		return false;
	}
	
	dis_node->tmnl_dev.spk_operate_timp = cur_time;
	terminal_pro_debug( "dis mode = %d ", dis_mode );
	DEBUG_INFO("spking num = %d apply num = %d=====================", gdisc_flags.speak_limit_num, gdisc_flags.apply_num);
	if( dis_mode == PPT_MODE ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP) ||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON)||\
			(dis_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
	{	
		if(key_down)
		{
			if (trans_model_unit_is_connected(dis_node->tmnl_dev.entity_id))
			{// already connected
				if (dis_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					dis_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
				
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
			}
			else
			{
				/*主席在发言上限里*/
				uint16_t speak_num = terminal_speak_num_count();
				int ret_cnnt = -1;
				if (speak_num < gdisc_flags.limit_num)
				{
					ret_cnnt = trans_model_unit_connect(dis_node->tmnl_dev.entity_id, dis_node);
				}
				
				if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt == 0))
				{
#if 0
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
#elif 0
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, dis_node );
#endif
                                        terminal_speak_track(dis_node->tmnl_dev.address.addr, true );
				}
				else if ((speak_num < gdisc_flags.limit_num) && (ret_cnnt != -2))
				{//has timeout for operation transmit ouput channel
#if 0
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#else
                                        terminal_key_action_host_special_num1_reply(recv_msg, MIC_OTHER_APPLY_STATUS, dis_node);  
#endif

					/*
					  *1\断开连接时间最长的
					  *2\超时后连接
					  */
					tmnl_pdblist disconnect_lnode = NULL;
					uint64_t id = 0;
					int ret = -1;
					bool if_common = false;

					ret = trans_model_unit_disconnect_longest_connect_re_id_cfcnode(&id, &disconnect_lnode);
					if (0 == ret)
					{
						if (disconnect_lnode != NULL && \
							disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
						{
							if_common = true;
						}
						else if (disconnect_lnode == NULL && id != 0)
						{
							disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
							if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
						}

						if (if_common)
						{
							if (dis_mode == FIFO_MODE)
								addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
							else if (dis_mode == LIMIT_MODE)
								gdisc_flags.speak_limit_num--;
						}

						terminal_over_time_speak_node_set(dis_node);
						if (NULL != disconnect_lnode)
							DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
								disconnect_lnode->tmnl_dev.address.tmn_type, 
								disconnect_lnode->tmnl_dev.address.addr, 
								gdisc_flags.speak_limit_num);
					}
				}
				else if (speak_num >= gdisc_flags.limit_num)
				{
#if 0				
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#else
                                        terminal_key_action_host_special_num1_reply(recv_msg, MIC_OTHER_APPLY_STATUS, dis_node);  
#endif
					/*
					  *1\断开连接时间最长的
					  *2\超时后连接
					  */
					tmnl_pdblist disconnect_lnode = NULL;
					uint64_t id = 0;
					int ret = -1;
					bool if_common = false;

					ret = trans_model_unit_disconnect_longest_connect_re_id_cfcnode(&id, &disconnect_lnode);
					if (0 == ret)
					{
						if (disconnect_lnode != NULL && \
							disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
						{
							if_common = true;
						}
						else if (disconnect_lnode == NULL && id != 0)
						{
							disconnect_lnode = found_terminal_dblist_node_by_endtity_id(id);
							if (disconnect_lnode != NULL && disconnect_lnode->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
							{
								if_common = true;
							}
						}

						if (if_common)
						{
							if (dis_mode == FIFO_MODE)
								addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, disconnect_lnode->tmnl_dev.address.addr);
							else if (dis_mode == LIMIT_MODE)
								gdisc_flags.speak_limit_num--;
						}

						terminal_over_time_speak_node_set(dis_node);
						if (NULL != disconnect_lnode)
							DEBUG_INFO("tmnl type = %d disconnect add = 0x%02x  spking num = %d====================1", 
								disconnect_lnode->tmnl_dev.address.tmn_type, 
								disconnect_lnode->tmnl_dev.address.addr, 
								gdisc_flags.speak_limit_num);
					}
				}
			}
		}
		else
		{
			if (trans_model_unit_is_connected(dis_node->tmnl_dev.entity_id))
			{
				if (0 == trans_model_unit_disconnect( dis_node->tmnl_dev.entity_id, dis_node ))
				{
#if 0
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
#else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
#endif
                                        terminal_speak_track(dis_node->tmnl_dev.address.addr, false );

					/*主席\vip控制限制模式下的普通终端的mic状态*/
					if (dis_mode == LIMIT_MODE)
					{
						terminal_chman_vip_control_common_mic();
					}
				}
				else
				{
					terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, dis_node );
				}
			}
			else
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, dis_node );
			}
		}
	}
	else
	{
		switch( dis_mode )
		{
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
				terminal_pro_debug( " out of discuss mode bound!" );
				break;
			}
		}
	}
	
	return true;
}

int terminal_chairman_apply_reply(uint8_t tmnl_type, uint16_t addr,
                                                                     uint8_t key_num, uint8_t key_value, 
                                                                     uint8_t tmnl_state, uint8_t recv_msg)
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

	if ((tmnl_state == TMNL_SYS_STA_VOTE)
              || (tmnl_state == TMNL_SYS_STA_GRADE)
              || (tmnl_state == TMNL_SYS_STA_SELECT))
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

void terminal_free_disccuss_mode_pro(bool key_down, uint8_t limit_time,
                                                                             tmnl_pdblist speak_node,
                                                                             uint8_t recv_msg)
{
	assert( speak_node );
	if( speak_node == NULL )
		return;
	
	if( key_down )
	{
		if( gdisc_flags.speak_limit_num < FREE_MODE_SPEAK_MAX )
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
			terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
		}
	}
	else
	{
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node );
		terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
	}

	terminal_main_state_send( 0, NULL, 0 );
}

void terminal_apply_list_first_speak( tmnl_pdblist const first_speak )
{
	if( first_speak == NULL )
		return;
	
	terminal_speak_track(first_speak->tmnl_dev.address.addr, true );

	//terminal_mic_state_set(MIC_OPEN_STATUS, first_speak->tmnl_dev.address.addr, first_speak->tmnl_dev.entity_id, true, first_speak);
	if( gdisc_flags.apply_num > 0 ) // 设置首位申请发言终端
	{
		tmnl_pdblist first_apply = NULL;
		gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
		first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
		if( first_apply != NULL )
		{
			terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
		}
		else
		{
			terminal_pro_debug( " no such tmnl dblist node!");
		}
	}
	else
	{
		gdisc_flags.currect_first_index = gdisc_flags.apply_num;
	}
}

// 已测试(2016-3-16)
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
	tmnl_pdblist first_apply = NULL; // 首位申请发言
	int  dis_ret = -1;

	terminal_pro_debug("speak_limit_num = %d limit_num = %d apply_num = %d apply_limit = %d", gdisc_flags.speak_limit_num,
				gdisc_flags.limit_num, gdisc_flags.apply_num, gdisc_flags.apply_limit);
	if( key_down ) // 打开麦克风
	{
		if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
		{
			terminal_key_action_host_special_num1_reply(recv_msg, MIC_OPEN_STATUS, speak_node);
		}
		else
		{
			uint16_t speak_num = terminal_speak_num_count();
			if (gdisc_flags.speak_limit_num < gdisc_flags.limit_num \
				&& speak_num < gdisc_flags.limit_num) // 打开麦克风
			{
				dis_ret = trans_model_unit_connect(speak_node->tmnl_dev.entity_id, speak_node);
#if 0
                                terminal_key_action_host_special_num1_reply(recv_msg, (dis_ret == 0)?MIC_OPEN_STATUS:MIC_COLSE_STATUS, speak_node);
#elif 0
                                terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
#else
                                terminal_key_action_host_special_num1_reply(recv_msg, MIC_OTHER_APPLY_STATUS, speak_node);
#endif
                                if (dis_ret == 0)
				{
					gdisc_flags.speak_limit_num++;
					terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
					ret = true;
				}
				else if (gdisc_flags.apply_num < gdisc_flags.apply_limit && (dis_ret != -2)) // 申请发言,且操作超时
				{//has timeout for operation transmit ouput channel
					uint8_t state = MIC_OTHER_APPLY_STATUS;
					if (0 == gdisc_flags.apply_num)
					{
						gdisc_flags.currect_first_index = 0;
						state = MIC_FIRST_APPLY_STATUS;
					}
					gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
					gdisc_flags.apply_num++;

					terminal_pro_debug("apply_num = %d", gdisc_flags.apply_num);
					terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
					terminal_mic_state_set(state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
					terminal_main_state_send( 0, NULL, 0 );
					ret = true;
				}					
			}
			else if (gdisc_flags.apply_num < gdisc_flags.apply_limit) // 申请发言
			{
				uint8_t state = MIC_OTHER_APPLY_STATUS;
				if (0 == gdisc_flags.apply_num)
				{
					gdisc_flags.currect_first_index = 0;
					state = MIC_FIRST_APPLY_STATUS;
				}
				gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
				gdisc_flags.apply_num++;

				terminal_pro_debug("apply_num = %d", gdisc_flags.apply_num);
				terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
				terminal_mic_state_set(state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
				terminal_main_state_send( 0, NULL, 0 );
				ret = true;
			}
			else
				terminal_key_action_host_special_num1_reply(recv_msg, MIC_COLSE_STATUS, speak_node);
		}
	}
	else
        {
#if 0
		terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#endif
                if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
		{
#if 0		
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#else
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
#endif
                        terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
		}
                else
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );                    
		
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		cc_state = speak_node->tmnl_dev.tmnl_status.mic_state;
		if( cc_state == MIC_FIRST_APPLY_STATUS || cc_state == MIC_OTHER_APPLY_STATUS )
		{
			addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr );
			terminal_mic_state_set(MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node);
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
			{
				gdisc_flags.currect_first_index %= gdisc_flags.apply_num;
				first_apply = found_terminal_dblist_node_by_addr( gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index]);
				if( first_apply != NULL )
				{
					terminal_mic_state_set( MIC_FIRST_APPLY_STATUS, first_apply->tmnl_dev.address.addr, first_apply->tmnl_dev.entity_id, true, first_apply );
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

			if(gdisc_flags.speak_limit_num < gdisc_flags.limit_num && gdisc_flags.apply_num > 0 )// 结束发言,并开始下一个申请终端的发言
			{
				if( addr_queue_delete_by_index( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, gdisc_flags.currect_first_index) )// 开启下一个申请话筒
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( current_addr );
					if( first_speak != NULL )
					{
						if (0 == trans_model_unit_connect( first_speak->tmnl_dev.entity_id, first_speak ))
						{// connect success
							gdisc_flags.speak_limit_num++;
							terminal_apply_list_first_speak(first_speak);
						}
						else
						{
							/*
							  *当前的mic断开成功后会留出一个通道
							  */
							terminal_over_time_speak_node_set(first_speak);
						}
					}
					else
					{
						terminal_pro_debug( " no such tmnl dblist node!");
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

// 已测试(2016-3-16)
bool terminal_fifo_disccuss_mode_pro( bool key_down, uint8_t limit_time,tmnl_pdblist speak_node, uint8_t recv_msg )
{
	bool ret = false;
	uint16_t addr = speak_node->tmnl_dev.address.addr;
	int dis_ret = -1;

	assert( speak_node );
	if( speak_node == NULL )
	{
		terminal_pro_debug( " NULL speak node!" );
		return false;
	}

	uint8_t speak_limit_num = gdisc_flags.speak_limit_num;
	uint16_t speak_num = terminal_speak_num_count();
	terminal_pro_debug( " speak NUM = %d limit speak NUM = %d", speak_limit_num, gdisc_flags.limit_num );
	if( key_down ) // 打开话筒
	{
		if( addr_queue_find_by_value( gdisc_flags.speak_addr_list, speak_limit_num, addr, NULL))
		{
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
				ret = true;
			}
			else
			{
				dis_ret = trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
				ret = (dis_ret == 0)? true:false;
				if (ret)
					terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
#if 0
				terminal_key_action_host_special_num1_reply( recv_msg, ret?MIC_OPEN_STATUS:MIC_COLSE_STATUS, speak_node );
#elif 0
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );      
#else
                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
#endif
                        }
		}
		else if( speak_num < gdisc_flags.limit_num && speak_limit_num < gdisc_flags.limit_num)
		{
			speak_node->tmnl_dev.tmnl_status.mic_state = MIC_COLSE_STATUS;
			dis_ret = trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node );
#if 0
                        terminal_key_action_host_special_num1_reply( recv_msg, (dis_ret == 0)?MIC_OPEN_STATUS:MIC_COLSE_STATUS, speak_node );
#elif 0
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#else
		        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
#endif
                        if (dis_ret == 0)
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
				gdisc_flags.speak_addr_list[speak_limit_num] = addr;
				gdisc_flags.speak_limit_num++;
				ret = true;
			}
			else if (dis_ret != -2)//  has no input channel to connect, delect address from fifo speaking list and opt not timeout
			{//has timeout for operation transmit ouput channel
				terminal_pro_debug( "=============connect Failed,Will disconnect first speak================" );
				if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr(gdisc_flags.speak_addr_list[0]);
					if( first_speak != NULL )
					{
						dis_ret = trans_model_unit_disconnect (first_speak->tmnl_dev.entity_id, first_speak);
						if (0 == dis_ret)
						{
							terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
							addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
							terminal_over_time_speak_node_set(speak_node);
						}
					}
					else
					{
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
			}
		}
		else // 发言人数大于或等于限制人数
		{
			if (!trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if( gdisc_flags.speak_addr_list[0] != 0xffff ) // 先进先出
				{
					tmnl_pdblist first_speak = found_terminal_dblist_node_by_addr( gdisc_flags.speak_addr_list[0] );
					if( first_speak != NULL )
					{
						if (trans_model_unit_is_connected(first_speak->tmnl_dev.entity_id))
						{
							dis_ret = trans_model_unit_disconnect( first_speak->tmnl_dev.entity_id, first_speak );
							if (0 == dis_ret)
							{
#if 0
                                                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#else
                                                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_OTHER_APPLY_STATUS, speak_node );
#endif
								terminal_speak_track(first_speak->tmnl_dev.address.addr, false );
								addr_queue_delete_by_index( gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, 0 );// 首位发言删除
								terminal_over_time_speak_node_set(speak_node);
							}
                                                        else
                                                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
						}
                                                else
                                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
					}
					else
					{
					        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
						terminal_pro_debug( "fifo not found tmnl list node!");
					}			
				}
                                else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );

			}
			else
			{
				if (speak_node->tmnl_dev.tmnl_status.mic_state != MIC_OPEN_STATUS)
					speak_node->tmnl_dev.tmnl_status.mic_state = MIC_OPEN_STATUS;
				
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			}
		}
	}
	else
	{
		if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
		{
			if (0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
#if 0
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
#else
                        	terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
#endif
                                addr_queue_delect_by_value(gdisc_flags.speak_addr_list, &gdisc_flags.speak_limit_num, speak_node->tmnl_dev.address.addr);
				terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
				ret = true;
			}
			else
			{
				terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
			}
		}
		else
		{
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
		}
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

	if( key_down ) /* 申请发言,加地址入申请列表*/
	{
		uint8_t state = MIC_OTHER_APPLY_STATUS;
		if(  gdisc_flags.apply_num < gdisc_flags.apply_limit )
		{
			if( 0 == gdisc_flags.apply_num )
			{
				gdisc_flags.currect_first_index = 0;
				state = MIC_FIRST_APPLY_STATUS;
			}
			
			gdisc_flags.apply_addr_list[gdisc_flags.apply_num] = speak_node->tmnl_dev.address.addr;
			gdisc_flags.apply_num++;

			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
			terminal_mic_state_set_send_terminal( false, state, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );
			terminal_main_state_send( 0, NULL, 0 );
			ret = true;
		}
		else
		{
			state = MIC_COLSE_STATUS;
			terminal_key_action_host_special_num1_reply( recv_msg, state, speak_node );
		}
	}
	else /* 取消申请发言*/
	{
		current_addr = gdisc_flags.apply_addr_list[gdisc_flags.currect_first_index];
		if(addr_queue_delect_by_value( gdisc_flags.apply_addr_list, &gdisc_flags.apply_num, addr ))
		{/* terminal apply */
			terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
			terminal_mic_state_set_send_terminal( false, MIC_COLSE_STATUS, speak_node->tmnl_dev.address.addr, speak_node->tmnl_dev.entity_id, true, speak_node );// 上报mic状态
			if( gdisc_flags.apply_num > 0 && current_addr == addr )// 置下一个申请为首位申请状态
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
		{/* terminal speaking */
			if (trans_model_unit_is_connected(speak_node->tmnl_dev.entity_id))
			{
				if ( 0 == trans_model_unit_disconnect( speak_node->tmnl_dev.entity_id, speak_node ))
                                {
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_OPEN_STATUS, speak_node );
                                        terminal_speak_track(speak_node->tmnl_dev.address.addr, false );
                                }
                                else
                                        terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );

			}
                        else
                                terminal_key_action_host_special_num1_reply( recv_msg, MIC_COLSE_STATUS, speak_node );
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

int terminal_speak_track( uint16_t addr, bool track_en )/* 摄像跟踪接口*/
{
	uint16_t temp;
	uint16_t i;
	uint16_t index;
	uint8_t cmr_track;

	cmr_track = gset_sys.camara_track;
 	if( !cmr_track )
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
*==开始投票处理-->处理未签到的终端的投票
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
				{/* 等待投票条件TVOTE_EN成立(即签到成功) */
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
				tmp->tmnl_dev.tmnl_status.vote_state |= (TWAIT_VOTE_FLAG);// 设置成可查询状态
				gvote_index++;
				gvote_index %= SYSTEM_TMNL_MAX_NUM;
			}
		}
		else
		{/* 查看系统是否投票完成*/
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
*==结束投票处理
*/

/*************************************************************
*==开始查询处理
*/
/*************************************************************
*Date:2016/1/27
*Name:terminal_query_vote_ask
*功能:保存终端签到、表决结果
*Param:
*	address :terminal application address
*	vote_state:终端签到表决按键结果
*Return:None
**************************************************************/
void terminal_query_vote_ask( uint16_t address, uint8_t vote_state )
{
	tmnl_pdblist vote_node = found_terminal_dblist_node_by_addr( address );
	if( NULL == vote_node )
	{
		terminal_pro_debug( "no such address 0x%04x node ", address );
		return;
	}

	uint8_t sys_state = get_sys_state();
	if( (SIGN_STATE == sys_state ) && (vote_state & 0x80) )/* sign complet? */
	{                                                                                           
		if( gtmnl_signstate == SIGN_IN_ON_TIME )            /* 设置签到标志*/
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
			terminal_pro_debug( "max key num = %d-------vote num = %d", key_num, vote_num );
			if ( vote_num >= key_num )
			{/* 投票完成，相应的终端停止查询投票结果*/
				vote_node->tmnl_dev.tmnl_status.vote_state &= (~TWAIT_VOTE_FLAG);
			}
#else
			if ( vote_num >= gvote_mode )
			{// 投票完成，相应的终端停止查询投票结果
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

void terminal_over_time_speak_node_set( tmnl_pdblist speak_node )
{
	if ((!gdisc_flags.over_speak.running) && (NULL != speak_node) )
	{
		gdisc_flags.over_speak.speak_node = speak_node;
		gdisc_flags.over_speak.running = true;
		over_time_set( DISCUSS_MODE_SPEAK_AFTER, 500);
	}
}

void terminal_over_time_speak_pro(void)
{
	if (gdisc_flags.over_speak.running && over_time_listen(DISCUSS_MODE_SPEAK_AFTER))
	{
		tmnl_pdblist speak_node = gdisc_flags.over_speak.speak_node;
		if (NULL != speak_node && gdisc_flags.edis_mode == FIFO_MODE)
		{
			if (trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ) == 0)
			{
				if (speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
				{
					gdisc_flags.speak_addr_list[gdisc_flags.speak_limit_num] = speak_node->tmnl_dev.address.addr;
					gdisc_flags.speak_limit_num++;
				}
				
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
			}
		}
		else if (NULL != speak_node && gdisc_flags.edis_mode == LIMIT_MODE)
		{
			if (0 == trans_model_unit_connect(speak_node->tmnl_dev.entity_id, speak_node))
			{/* connect success */
				if (speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_COMMON_RPRST)
				{
					gdisc_flags.speak_limit_num++;
				}

				terminal_apply_list_first_speak(speak_node);
			}
		}
		else if (NULL != speak_node && \
			(gdisc_flags.edis_mode == PPT_MODE ||\
			speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_VIP ||\
			speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_COMMON ||\
			speak_node->tmnl_dev.address.tmn_type == TMNL_TYPE_CHM_EXCUTE))
		{
			if( 0 == trans_model_unit_connect( speak_node->tmnl_dev.entity_id, speak_node ))
			{
				terminal_speak_track(speak_node->tmnl_dev.address.addr, true );
			}
		}
		
		gdisc_flags.over_speak.running = false;
		gdisc_flags.over_speak.speak_node = NULL;
		over_time_stop(DISCUSS_MODE_SPEAK_AFTER);
	}
}

/* 主机查询签到投票结果*/
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
		terminal_pro_debug( "out of system terminal list bank!" );
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
*==结束查询处理
*/

/*************************************************************
*==开始签到处理
*/
void terminal_sign_in_pro( void )
{
	uint8_t sign_type;
	int i = 0;

	if( (gtmnl_signstate == SIGN_IN_BE_LATE)  )
	{
		if(over_time_listen( SIGN_IN_LATE_HANDLE ))
		{
			gtmnl_signstate = SIGN_IN_OVER;
			sign_type = gset_sys.sign_type;
			if( sign_type )
			{
				terminal_pro_debug( "over time sign in sign type is card sign in " );
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
*==结束签到处理
*/

/*===================================================
终端处理流程@}
=====================================================*/

/*==================================================
	start reallot address
====================================================*/
/* 清除终端链表*/
tmnl_pdblist terminal_system_dblist_except_free( void )
{
	tmnl_pdblist p_node = NULL;
	
	p_node = terminal_dblist_except_free( dev_terminal_list_guard );
	if( p_node == dev_terminal_list_guard )
                gcur_tmnl_list_node = dev_terminal_list_guard;

	return p_node;
}

/* 清除除了target_id 终端链表节点*/
void terminal_system_clear_node_info_expect_target_id( void )
{
	tmnl_pdblist p_node = NULL;
	
	for (p_node = dev_terminal_list_guard->next;\
            p_node != dev_terminal_list_guard; \
            p_node = p_node->next)
	{/* clear info expect target_id */
	    p_node->tmnl_dev.address.addr = 0xffff;
            p_node->tmnl_dev.address.tmn_type = 0xffff;
            p_node->tmnl_dev.spk_operate_timp = 0;
            host_timer_stop(&p_node->tmnl_dev.spk_timeout);
            memset(&p_node->tmnl_dev.tmnl_status, 0, sizeof(terminal_state));
	}
}

void terminal_open_addr_file_wt_wb( void )
{
	if( addr_file_fd != NULL )
		Fclose( addr_file_fd );// 先关闭
		
	addr_file_fd = Fopen( ADDRESS_FILE, "wb+");
	if( addr_file_fd == NULL )
	{
		DEBUG_ERR( "terminal_open_addr_file_wt_wb open fd  Err!" );
		assert( NULL != addr_file_fd );
		if( NULL == addr_file_fd )
			return;
	}	
}

/* 摧毁终端链表*/
void terminal_system_dblist_destroy( void )
{
	tmnl_pdblist p_node = destroy_terminal_dblist( dev_terminal_list_guard );
	if( NULL != p_node )
	{
		terminal_pro_debug( "destroy terminal double list not success!" );
	}
}
	
/*===================================================
end reallot address
=====================================================*/

uint16_t terminal_pro_get_address( int get_flags, uint16_t addr_cur )
{
	uint16_t addr = 0xffff;
	static int person = 0;/* 1 链表最后一个；-1链表前一个可用节点*/

	assert( gcur_tmnl_list_node );
	if( gcur_tmnl_list_node != NULL )
	{
		if( get_flags == 1 )
		{
			if( gcur_tmnl_list_node->next != dev_terminal_list_guard )
			{
				if( gcur_tmnl_list_node->next->tmnl_dev.address.addr != 0xffff &&\
					gcur_tmnl_list_node->next->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最后一个已注册终端(有效节点) */
					addr = gcur_tmnl_list_node->next->tmnl_dev.address.addr;
					gcur_tmnl_list_node = gcur_tmnl_list_node->next;
				}
				else
					person = 1;
			}
			else 
			{
				person = 1;
			}
		}
		else if( get_flags == -1 )
		{
			if( person && gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&
				 gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst )
			{/* 最后一个可用节点*/
				addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				person = 0;
			}
			else if( gcur_tmnl_list_node->prior != dev_terminal_list_guard && gcur_tmnl_list_node->prior->tmnl_dev.address.addr != 0xffff )
			{
				if(  gcur_tmnl_list_node->prior->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最前一个已注册终端*/
					addr = gcur_tmnl_list_node->prior->tmnl_dev.address.addr;
					gcur_tmnl_list_node = gcur_tmnl_list_node->prior;
				}
			}
			else if(gcur_tmnl_list_node->prior != dev_terminal_list_guard && gcur_tmnl_list_node->prior->tmnl_dev.address.addr == 0xffff )
			{/* 不可用节点0xffff */
				gcur_tmnl_list_node = gcur_tmnl_list_node->prior;
				if( gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&
				 	gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst)
				{/* 最后一个可用节点*/
					addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				}
			}
			else if( gcur_tmnl_list_node->prior == dev_terminal_list_guard )
			{
				if( gcur_tmnl_list_node->tmnl_dev.address.addr != 0xffff &&\
					gcur_tmnl_list_node->tmnl_dev.tmnl_status.is_rgst )
				{/* gcur_tmnl_list_node 只移到最前一个已注册终端*/
					addr = gcur_tmnl_list_node->tmnl_dev.address.addr;
				}
			}
		}
	}

	return addr;
}

void terminal_pro_init_cur_terminal_node( void )
{
	gcur_tmnl_list_node = dev_terminal_list_guard;
}

