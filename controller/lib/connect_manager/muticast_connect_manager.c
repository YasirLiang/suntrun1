/*
**File:muticast_connect_manager.c
**Author:梁永富
**时间:2016年5月19日
**功能:管理系统中央广播连接表
*/

/********************************************/
//新建系统广播连接机制的管理模块，
//代替原来muticast_connector.c文件中的
//管理机制,
//并新增了数据库的广播配置的
//管理功能
/********************************************/

#include "muticast_connect_manager.h"
#include "system_database.h"
#include "host_timer.h"
#include "output_channel.h"
#include "conference_recieve_unit.h"
#include "linked_list_unit.h"
#include "central_control_transmit_unit.h"
#include "time_handle.h"
#include "log_machine.h"
#include "acmp_controller_machine.h"

enum _enum_muticast_pro_state
{
	MUTICAST_PRO_PRIMED,
	MUTICAST_PRO_HANDING,
	MUTICAST_PRO_IDLE,
};

enum _enum_change_muticast_state
{
	MUTICAST_CHANGE_BEGIN,
	MUTICAST_CHANGE_HANDING,
	MUTICAST_CHANGE_FINISH
};

typedef struct _type_muticast_manager
{
	enum _enum_change_muticast_state mm_cha_state;
	enum _enum_muticast_pro_state mm_pro_state;
	bool running;
	bool muticast_exist;
	Tstr_sysmuti_param mm_sys_flags;
	host_timer mm_errlog_timer;
	struct list_head * ptr_curcfc_recv_model;// tconference_recieve_model
	struct list_head * ptr_muticastor;// T_pccuTModel
	struct list_head *ptr_muticastor_output;// 定义广播者的输出通道 类型为TOutChannel
}Tstr_MMPro;

#ifdef __DEBUG__
#define __MUTICASTOR_MANAGER_DEBUG__
#endif

#ifdef __MUTICASTOR_MANAGER_DEBUG__
#define muticastor_manager_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define muticastor_manager_debug(fmt, args...)
#endif

static Tstr_MMPro gmuticast_manager_pro;
static uint16_t gacmp_sequence_id = 0;

static struct list_head *gpdefault_muticastor = NULL ;// 默认广播者
static struct list_head *gpdefault_muticastor_output = NULL;// 默认广播者下的输出通道

extern solid_pdblist endpoint_list;// 全局1722终端列表

static int muti_cnnt_mngr_unmutic_pro_tmnl_by_selfstate( T_pInChannel_universe ptr_muti_inchn, 
													const uint64_t muticastor_id,
													const uint64_t local_listen_id,
													const bool muticastor_exit,
													const T_pOutChannel muticast_output,
													const bool offline_connect,
													const bool reconnect_self,
													const uint8_t failed_count,
													T_Ptrconference_recieve_model ptr_recv_model);
static int muticast_connect_manger_pro_terminal_by_selfstate( T_pInChannel_universe ptr_Inchn, 
															const uint64_t muticastor_id,
															const uint64_t local_listen_id,
															const bool muticastor_exit,
															const T_pOutChannel muticast_output,
															const bool offline_connect,
															const bool reconnect_self,
															const uint8_t failed_count,
															T_Ptrconference_recieve_model ptr_recv_model);
static int muticast_connect_manger_pro_terminal_by_selfstate( T_pInChannel_universe ptr_Inchn, 
															const uint64_t muticastor_id,
															const uint64_t local_listen_id,
															const bool muticastor_exit,
															const T_pOutChannel muticast_output,
															const bool offline_connect,
															const bool reconnect_self,
															const uint8_t failed_count,
															T_Ptrconference_recieve_model ptr_recv_model);
static int muticast_connect_manger_uphold_muti_list( bool muti_flags );
static int muticast_connect_manger_muticastor_default_change_pro( void );
static int muticast_connect_manger_error_log( void );
static int muticast_connect_manger_not_muticast_pro( void );
static int muticast_connect_manger_muticast_pro( void );

static int muti_cnnt_mngr_unmutic_pro_tmnl_by_selfstate( T_pInChannel_universe ptr_muti_inchn, 
													const uint64_t muticastor_id,
													const uint64_t local_listen_id,
													const bool muticastor_exit,
													const T_pOutChannel muticast_output,
													const bool offline_connect,
													const bool reconnect_self,
													const uint8_t failed_count,
													T_Ptrconference_recieve_model ptr_recv_model )
{
	bool disconnect_flags = true;
	enum input_channel_status in_status;
	struct jdksavdecc_eui64 talker_entity_id, listen_entity_id;
	int ret = -1;
	
	if  ((ptr_muti_inchn == NULL) ||(muticast_output == NULL)\
		|| (ptr_recv_model == NULL))
	{
		return -1;
	}

	if (ptr_muti_inchn->pro_status != INCHANNEL_PRO_FINISH)
	{
		muticastor_manager_debug( "Input Node does not proccess Finish......" );
		return 0;
	}

	/*
	*情况一:已有终端连接上，断开其连接，
	*			并对在线的不再实行更新操作。
	*情况二:已有终端没连接上，则停止定时器。
	*/
	solid_pdblist solid = NULL;
	in_status = ptr_muti_inchn->status;
	switch( in_status )
	{
		case INCHANNEL_UNAVAILABLE:
			break;
		case INCHANNEL_FREE:/* 未连接上*/
			host_timer_stop( &ptr_recv_model->muticast_query_timer );
			ptr_recv_model->query_stop = true;
			break;
		case INCHANNEL_BUSY:/* 被占用了*/
			if ((get_current_time() - muticast_output->operate_timetimp) < OUTPUT_CHANNEL_OPT_PROTECT_TIME)
				disconnect_flags = false;
			
			/* 检查广播者是否在线*/
			if (disconnect_flags)
			{
				solid = search_endtity_node_endpoint_dblist( endpoint_list, ptr_muti_inchn->tarker_id );
				if( solid != NULL && (solid != NULL) &&\
					(solid->solid.connect_flag == DISCONNECT))
				{/* 不检查更新*/
					disconnect_flags = false;
				}	
			}

			if( disconnect_flags )
			{
                                ptr_muti_inchn->pro_status = INCHANNEL_PRO_PRIMED;/* 标示输入节点正在预处理，用于互斥访问改变节点*/

				convert_uint64_to_eui64( talker_entity_id.value, ptr_muti_inchn->tarker_id );
				convert_uint64_to_eui64( listen_entity_id.value, local_listen_id );	
				acmp_disconnect_avail( talker_entity_id.value, 
									ptr_muti_inchn->tarker_index, 
									listen_entity_id.value, 
									ptr_muti_inchn->listener_index, 
									0, 
									gacmp_sequence_id++ );
			}
			break;
		default:
			break;
	}

	return ret;
}

static int muticast_connect_manger_pro_terminal_by_selfstate( T_pInChannel_universe ptr_Inchn, 
															const uint64_t muticastor_id,
															const uint64_t local_listen_id,
															const bool muticastor_exit,
															const T_pOutChannel muticast_output,
															const bool offline_connect,
															const bool reconnect_self,
															const uint8_t failed_count,
															T_Ptrconference_recieve_model ptr_recv_model )
{
	enum input_channel_status in_status;
	int ret = -1;
	struct jdksavdecc_eui64 talker_entity_id, listen_entity_id;
	bool model_tark_discut = false;
	bool connect_flags = true;
	bool update_flags = true;
	
	if ((ptr_Inchn == NULL) || (muticast_output == NULL)\
		|| (ptr_recv_model == NULL))
	{
		return -1;
	}

	if (ptr_Inchn->pro_status != INCHANNEL_PRO_FINISH)
	{
		muticastor_manager_debug( "0x%016llx - %d Input Node does not proccess Finish......", \
			local_listen_id, ptr_Inchn->listener_index );
		
		return 0;
	}

	in_status = ptr_Inchn->status;
	switch( in_status )
	{
		case INCHANNEL_UNAVAILABLE:
			break;
		case INCHANNEL_FREE:/* 未连接上*/
			model_tark_discut = ptr_recv_model->tark_discut;

			if ((get_current_time() - muticast_output->operate_timetimp) < OUTPUT_CHANNEL_OPT_PROTECT_TIME)
				connect_flags = false;
			
			if( connect_flags && ((model_tark_discut && !reconnect_self)\
				||(!offline_connect && (ptr_Inchn->connect_failed_count > failed_count))))
			{/* 本机断开不重连或掉线不重连*/
				connect_flags = false;
			}

			if( connect_flags && muticastor_exit )
			{
			        ptr_Inchn->pro_status = INCHANNEL_PRO_PRIMED;
			        convert_uint64_to_eui64( talker_entity_id.value, muticastor_id );
			        convert_uint64_to_eui64( listen_entity_id.value, local_listen_id );
				acmp_connect_avail( talker_entity_id.value,
							muticast_output->tarker_index, 
							listen_entity_id.value, 
							ptr_Inchn->listener_index,
							0, 
							gacmp_sequence_id++ );
			}
			break;
		case INCHANNEL_BUSY:/* 被占用了*/
			if( (muticastor_id == ptr_Inchn->tarker_id) &&\
				muticast_output->tarker_index == ptr_Inchn->tarker_index )
			{
				/* 检查是否在线*/
				if( ptr_recv_model != NULL && \
					(ptr_recv_model->solid_pnode != NULL) &&\
					(ptr_recv_model->solid_pnode->solid.connect_flag == DISCONNECT))
				{/* 不检查更新*/
					update_flags = false;
				}

				if( update_flags )
				{
				        /*特别注意此语句必须在acmp_rx_state_avail 或acmp_connect_avail之前执行
                                          *因为调用系统的发送函数会导致此运行的线程被抢占，
                                          *若数据处理函数的线程与此函数被调用的线程不同，则该线程被抢占后可能会导致其
                                          *ptr_Inchn->pro_status状态的设置推后,其他关于此状态INCHANNEL_PRO_PRIMED的设置的同理
                                          */
				        ptr_Inchn->pro_status = INCHANNEL_PRO_PRIMED;/* 标示输入节点正在预处理，用于互斥访问改变节点*/

				        if (ptr_Inchn->connect_count_interval)
                                        {

        					acmp_rx_state_avail( local_listen_id, ptr_Inchn->listener_index );
                                        }
                                        else
                                        {

                                                convert_uint64_to_eui64( talker_entity_id.value, muticastor_id );
                			        convert_uint64_to_eui64( listen_entity_id.value, local_listen_id );
                				acmp_connect_avail( talker_entity_id.value,
                        							muticast_output->tarker_index, 
                        							listen_entity_id.value, 
                        							ptr_Inchn->listener_index,
                        							0, 
                        							gacmp_sequence_id++ );

                                        }
				}
			}
			else
			{/* 不是默认广播者*/
				muticastor_manager_debug( "listener id = 0x%016llx, current tarker id = 0x%016llx -%d,	default id =  0x%016llx-%d",
					local_listen_id, ptr_Inchn->tarker_id, ptr_Inchn->tarker_index,muticastor_id, muticast_output->tarker_index );
				if (0 != ptr_Inchn->tarker_id)
				{
					if ((get_current_time() - muticast_output->operate_timetimp) > OUTPUT_CHANNEL_OPT_PROTECT_TIME)
					{
					        ptr_Inchn->pro_status = INCHANNEL_PRO_PRIMED;

						convert_uint64_to_eui64( talker_entity_id.value, ptr_Inchn->tarker_id );
						convert_uint64_to_eui64( listen_entity_id.value, local_listen_id );	
						acmp_disconnect_avail( talker_entity_id.value, 
							ptr_Inchn->tarker_index, 
							listen_entity_id.value, 
							ptr_Inchn->listener_index, 
							0, 
							gacmp_sequence_id++ );
					}
				}
				else
				{/* update the right  listener's talker */
					muticastor_manager_debug(" update the right  listener's talker");
                                        ptr_Inchn->pro_status = INCHANNEL_PRO_PRIMED;
					acmp_rx_state_avail( local_listen_id, ptr_Inchn->listener_index );
				}
			}
			break;
		default:
			break;
	}

	return ret;
}

static int muticast_connect_manager_inout_channel_entry_get( struct list_head *p_recv_model, 
													struct list_head *p_ccut_model,
													struct list_head *p_ccut_ouput,
													T_pInChannel_universe* pptr_Inchn,
													T_pccuTModel* pptr_muticastor,
													T_pOutChannel* ppOutChannel )
{	
	assert((p_recv_model != NULL) && (p_ccut_model != NULL) &&\
		(p_ccut_ouput != NULL));
	if( (p_recv_model == NULL) || (p_ccut_model == NULL)||\
		(p_ccut_ouput == NULL))
		return -1;

	assert((pptr_Inchn != NULL) && (pptr_muticastor != NULL)&&\
		(ppOutChannel != NULL));
	if ((pptr_Inchn == NULL) || (pptr_muticastor == NULL) ||\
		ppOutChannel == NULL)
		return -1;

	*pptr_Inchn = list_entry( p_recv_model, TInChannel_universe, list );
	*pptr_muticastor = list_entry( p_ccut_model, TccuTModel, list );
	*ppOutChannel = list_entry( p_ccut_ouput, TOutChannel, list );

	return 0;
}

static int muticast_connect_manger_uphold_muti_list( bool muti_flags )
{
	int ret = -1;
	T_Ptrconference_recieve_model ptr_recv_model = NULL;
	uint32_t query_timeout = (uint32_t)gmuticast_manager_pro.mm_sys_flags.query_timeout*1000;	

	ptr_recv_model = list_entry( gmuticast_manager_pro.ptr_curcfc_recv_model, tconference_recieve_model, list );
	if( ptr_recv_model != NULL )
	{	
		if( ptr_recv_model->query_stop )
		{
			if( muti_flags )
			{/* start query */
				ptr_recv_model->query_stop = false;
				host_timer_start( query_timeout, &ptr_recv_model->muticast_query_timer );
			}
		}
		else if( !host_timer_timeout( &ptr_recv_model->muticast_query_timer ) )
		{
		        if (host_timer_is_stop(&ptr_recv_model->muticast_query_timer))
                        {
                                if (ptr_recv_model->solid_pnode != NULL && \
                                            ptr_recv_model->solid_pnode->solid.connect_flag == CONNECT)// reconnect?
                                {
                                        host_timer_start( query_timeout, &ptr_recv_model->muticast_query_timer );
                                }
                        }
                
			ret = -1;
		}
		else/* not end query update and timeout?*/
		{
			T_pInChannel_universe ptr_muti_Inchn = NULL;
			T_pccuTModel ptr_muticastor = NULL;
			T_pOutChannel pOutChannel = NULL;
			struct list_head *p_recv_muti_model = ptr_recv_model->p_ccu_muticast_channel;
			struct list_head *p_ccut_mo = gmuticast_manager_pro.ptr_muticastor;
			struct list_head *p_ccut_out = gmuticast_manager_pro.ptr_muticastor_output;

			if ((p_recv_muti_model != NULL) && (p_ccut_mo != NULL)
                                && (p_ccut_out != NULL))
			{
				muticast_connect_manager_inout_channel_entry_get( p_recv_muti_model,
													p_ccut_mo,
													p_ccut_out, 
													&ptr_muti_Inchn, 
													&ptr_muticastor, 
													&pOutChannel );
				assert( ptr_muti_Inchn  && ptr_muticastor && pOutChannel );
				if ((ptr_muti_Inchn != NULL) && (ptr_muticastor != NULL)
                                        && (pOutChannel != NULL))
				{
					if( muti_flags )
					{
						ret = muticast_connect_manger_pro_terminal_by_selfstate( ptr_muti_Inchn, 
							ptr_muticastor->tarker_id,
							ptr_recv_model->listener_id,
							gmuticast_manager_pro.muticast_exist, 
							pOutChannel,
							gmuticast_manager_pro.mm_sys_flags.offline_connect,
							gmuticast_manager_pro.mm_sys_flags.reconnect_self,
							gmuticast_manager_pro.mm_sys_flags.failed_connect_count,
							ptr_recv_model );

						host_timer_update( query_timeout, &ptr_recv_model->muticast_query_timer );
						ret = 0;
					}
					else
					{
						ret = muti_cnnt_mngr_unmutic_pro_tmnl_by_selfstate( ptr_muti_Inchn, 
							ptr_muticastor->tarker_id,
							ptr_recv_model->listener_id,
							gmuticast_manager_pro.muticast_exist, 
							pOutChannel,
							gmuticast_manager_pro.mm_sys_flags.offline_connect,
							gmuticast_manager_pro.mm_sys_flags.reconnect_self,
							gmuticast_manager_pro.mm_sys_flags.failed_connect_count,
							ptr_recv_model );
					}
				}
				else 
				{
					ret = -1;
				}
			}
			else 
				ret = -1;
		}
	}

	return ret;
}

static int muticast_connect_manger_muticastor_default_change_pro( void )
{
	if( (gmuticast_manager_pro.mm_cha_state == MUTICAST_CHANGE_BEGIN) )
	{
		if( (gmuticast_manager_pro.ptr_muticastor != gpdefault_muticastor) &&
			(gmuticast_manager_pro.ptr_muticastor_output != gpdefault_muticastor_output))
		{
			gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_HANDING;
		}
		else 
		{
			over_time_stop( CHANGE_MUTICASTOR_TIMEOUT_INDEX );
			gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_FINISH;
		}
	}
	else if( (gmuticast_manager_pro.mm_cha_state == MUTICAST_CHANGE_HANDING) &&
		(gpdefault_muticastor != NULL) && (gpdefault_muticastor_output != NULL))
	{
		if( over_time_listen( CHANGE_MUTICASTOR_TIMEOUT_INDEX ) )
		{
			bool change_default = true;
			
			if( gmuticast_manager_pro.mm_sys_flags.en_default_muti && \
					gmuticast_manager_pro.muticast_exist )
			{/* 不能改变*/
				change_default = false;
			}
			
			if( change_default && \
				(gpdefault_muticastor != gmuticast_manager_pro.ptr_muticastor) &&\
				(gpdefault_muticastor_output != gmuticast_manager_pro.ptr_muticastor))
			{
				gmuticast_manager_pro.ptr_muticastor = gpdefault_muticastor;
				gmuticast_manager_pro.ptr_muticastor_output = gpdefault_muticastor_output;
				gmuticast_manager_pro.muticast_exist = true;
			}

			over_time_stop( CHANGE_MUTICASTOR_TIMEOUT_INDEX );
			gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_FINISH;
		}
	}
        else
        {
                /*gpdefault_muticastor or gpdefault_muticastor_output NULL value or 
                    system muticastor finish being changed */
        }

	return 0;
}

static int muticast_connect_manger_error_log( void )
{
	uint32_t log_timeout = gmuticast_manager_pro.mm_sys_flags.log_timeout*1000;// s
	uint16_t failed_connect_count = gmuticast_manager_pro.mm_sys_flags.failed_connect_count;
	T_Ptrconference_recieve_model ptr_curcfc_recv_model = list_entry( gmuticast_manager_pro.ptr_curcfc_recv_model, tconference_recieve_model, list);
	
	if( !gmuticast_manager_pro.mm_sys_flags.log_err )
	{
		return -1;
	}
	
	if( !gmuticast_manager_pro.muticast_exist &&\
		gmuticast_manager_pro.mm_sys_flags.log_none_muticast )
	{
		if( host_timer_timeout( &gmuticast_manager_pro.mm_errlog_timer) )
		{
			if( gp_log_imp != NULL )
    				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
								LOGGING_LEVEL_NOTICE, 
								"System Muticastor Not exit or not init:Please Check!" );

			host_timer_update( log_timeout, &gmuticast_manager_pro.mm_errlog_timer );
		}
	}
	
	if ((ptr_curcfc_recv_model != NULL) &&\
		gmuticast_manager_pro.mm_sys_flags.log_discut )
	{
		if( !host_timer_timeout( &ptr_curcfc_recv_model->errlog_timer ) )
		{
			return -1;
		}
		
		struct list_head *ptr_tmp_model = ptr_curcfc_recv_model->p_ccu_muticast_channel;
		T_pInChannel_universe ptr_Inchn = NULL;
		if( NULL !=  ptr_tmp_model )
		{
			ptr_Inchn = list_entry( ptr_tmp_model, TInChannel_universe, list );
		}

		if( (((ptr_curcfc_recv_model->solid_pnode != NULL)&&(ptr_curcfc_recv_model->solid_pnode->solid.connect_flag == DISCONNECT))||\
		     ((ptr_Inchn != NULL) && (ptr_Inchn->connect_failed_count > failed_connect_count))) &&\
			(gp_log_imp != NULL) )
		{
    				gp_log_imp->log.post_log_msg( &gp_log_imp->log, 
								LOGGING_LEVEL_ERROR, 
								"[ Terminal (0x%016llx) - %d(Failed cont(Max-already): %d-%d) Out of Line:Please Check!]", 
								ptr_curcfc_recv_model->listener_id,
								ptr_Inchn->listener_index,
								failed_connect_count,
								ptr_Inchn->connect_failed_count );

			host_timer_update( log_timeout, &ptr_curcfc_recv_model->errlog_timer );
                        host_timer_stop( &ptr_curcfc_recv_model->muticast_query_timer );
		}
	}

	return 0;
}

static int muticast_connect_manger_not_muticast_pro( void )
{
	int ret = -1;

	/* 当改变广播者时，不能改变当前的广播者*/
	if( gmuticast_manager_pro.mm_cha_state == MUTICAST_CHANGE_BEGIN )
	{
		over_time_stop( CHANGE_MUTICASTOR_TIMEOUT_INDEX );
		gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_FINISH;
	}

	ret = muticast_connect_manger_uphold_muti_list( false );

	return ret;
}

static int muticast_connect_manger_muticast_pro( void )
{
	int ret = -1;

	muticast_connect_manger_error_log();
	muticast_connect_manger_muticastor_default_change_pro();

	if( gmuticast_manager_pro.mm_cha_state == MUTICAST_CHANGE_FINISH )
		ret = muticast_connect_manger_uphold_muti_list( true );

	return ret;
}

int muticast_connect_manger_timeout_event_image( void )
{
	int ret = 0;

	if (gmuticast_manager_pro.running)
	{
		struct list_head *tmp_recv_model = gmuticast_manager_pro.ptr_curcfc_recv_model;

		if( conference_recieve_model_is_right(tmp_recv_model) )
		{/* 有用的节点*/
			if( gmuticast_manager_pro.mm_sys_flags.muti_flag )
			{
				ret = muticast_connect_manger_muticast_pro();
			}
			else
			{
				ret = muticast_connect_manger_not_muticast_pro();
			}
		}

		/* 移动被广播者的指针,指向下一个*/
		conference_recieve_model_found_next( tmp_recv_model,  &gmuticast_manager_pro.ptr_curcfc_recv_model );
	}
	
	return ret;
}

/* 系统改变广播者函数*/
int muticast_connect_manger_chdefault_outmuticastor( struct list_head *p_muti, struct list_head *p_muit_out )
{
	assert( p_muti != NULL && p_muit_out != NULL);
	if( p_muti == NULL || p_muit_out == NULL )
		return -1;

	if (((gmuticast_manager_pro.muticast_exist && over_time_listen(CHANGE_MUTICASTOR_INTERVAL))
		||(!gmuticast_manager_pro.muticast_exist)) 
		&& (gmuticast_manager_pro.mm_cha_state == MUTICAST_CHANGE_FINISH))
	{
		gpdefault_muticastor = p_muti;
		gpdefault_muticastor_output = p_muit_out;
		gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_BEGIN;
		over_time_set( CHANGE_MUTICASTOR_TIMEOUT_INDEX, 500 );// 500ms timeout
		over_time_set( CHANGE_MUTICASTOR_INTERVAL, 15*1000 );// 15s timeout 
	}

	return 0;
}

/* 广播配置数据表回调函数*/
int muticast_connect_manger_database_update( void* p_muti_param_tmp )
{
	Tstr_sysmuti_param*p_mm_sys_flags = &gmuticast_manager_pro.mm_sys_flags;
	Tstr_sysmuti_param*p_muti_param = (Tstr_sysmuti_param*)p_muti_param_tmp;

	assert( p_muti_param != NULL && p_mm_sys_flags != NULL );
	if( p_muti_param == NULL || p_mm_sys_flags == NULL )
		return -1;
	
	if( p_mm_sys_flags->muti_flag != p_muti_param->muti_flag )
		p_mm_sys_flags->muti_flag = p_muti_param->muti_flag?true:false;

	if( p_mm_sys_flags->en_default_muti != p_muti_param->en_default_muti )
		p_mm_sys_flags->en_default_muti = p_muti_param->en_default_muti?true:false;
	
	if( p_mm_sys_flags->reconnect_self != p_muti_param->reconnect_self )
		p_mm_sys_flags->reconnect_self = p_muti_param->reconnect_self?true:false;
	
	if( p_mm_sys_flags->offline_connect != p_muti_param->offline_connect )
		p_mm_sys_flags->offline_connect = p_muti_param->offline_connect?true:false;
	
	if( p_mm_sys_flags->reconnect_timeout != p_muti_param->reconnect_timeout )
		p_mm_sys_flags->reconnect_timeout = p_muti_param->reconnect_timeout;
	
	if( p_mm_sys_flags->failed_connect_count != p_muti_param->failed_connect_count )
		p_mm_sys_flags->failed_connect_count = p_muti_param->failed_connect_count;

	if( p_mm_sys_flags->discut_self != p_muti_param->discut_self )
	{
		p_mm_sys_flags->discut_self = p_muti_param->discut_self?true:false;
		muticastor_manager_debug( "======>>>>discut flags = %d", p_muti_param->discut_self );
	}

	if( p_mm_sys_flags->log_err != p_muti_param->log_err )
		p_mm_sys_flags->log_err = p_muti_param->log_err?true:false;
	
	if( p_mm_sys_flags->log_discut != p_muti_param->log_discut )
		p_mm_sys_flags->log_discut = p_muti_param->log_discut?true:false;
	
	if( p_mm_sys_flags->log_none_muticast != p_muti_param->log_none_muticast )
		p_mm_sys_flags->log_none_muticast = p_muti_param->log_none_muticast?true:false;
	
	if( p_mm_sys_flags->log_timeout != p_muti_param->log_timeout )
		p_mm_sys_flags->log_timeout = p_muti_param->log_timeout;
	
	if( p_mm_sys_flags->query_timeout != p_muti_param->query_timeout )
		p_mm_sys_flags->query_timeout = p_muti_param->query_timeout;

	return 0;
}

bool muticast_muticast_connect_manger_get_discut_self_flag( void )
{
	return gmuticast_manager_pro.mm_sys_flags.discut_self;
}

void muticast_muticast_connect_manger_init( void )
{
	gmuticast_manager_pro.running = true;
	gmuticast_manager_pro.mm_cha_state = MUTICAST_CHANGE_FINISH;
	host_timer_start( (uint32_t)gmuticast_manager_pro.mm_sys_flags.log_timeout*1000,\
					&gmuticast_manager_pro.mm_errlog_timer );

}

void muticast_muticast_connect_manger_pro_stop(void)
{
	gmuticast_manager_pro.running = false;
}

void muticast_muticast_connect_manger_pro_start(void)
{
	gmuticast_manager_pro.running = true;
}

