/*
**central_control_recieve_unit.c
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 中心控制单元接收模块
// ****************************

#include "central_control_recieve_unit.h"
#include "util.h"
#include "acmp_controller_machine.h"

#define CCU_RECV_PROTECT_TIMEOUT (2*1000)

#ifdef __DEBUG__
//#define __CCU_RECV_DEBUG__
#endif

#ifdef __CCU_RECV_DEBUG__
#define ccu_recv_unit_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define ccu_recv_unit_debug(fmt, args...)
#endif

tchannel_allot_pro gchannel_allot_pro;// 全局通道分配处理
TccuRModel gccu_recieve_model_list[CCU_TR_MODEL_MAX_NUM];// 全局中央未连接连接表
uint16_t gccu_acmp_sequeue_id;

observer_t gccu_recv_observer;// 用于更新ucc 接收模块的连接状态

extern solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
extern desc_pdblist descptor_guard;	// 系统中描述符链表哨兵节点


// 通过优先算法查找连接节点
static bool central_control_search_connect_by_arithmetic( T_pInChannel* pp_InChannel )//(unfinish 2016-3-11)
{
	int i = 0;
	bool bret = false;
	T_pInChannel p_lestest_cnnt_inchannel = NULL, p_temp_inchannel = NULL;
	uint32_t cur_time = get_current_time();
	
	if (pp_InChannel == NULL)
	{
		bret = false;
	}
	else
	{
		/*
		**查找算法:
		**		1、没有超出模块的连接能力的，只要找到了都先赋值，
		**			若没有找到则遍历下一个模块
		*/
		for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
		{
			if( gccu_recieve_model_list[i].solid_pnode != NULL )// check solid is right init or whether out  off line
			{
				if( gccu_recieve_model_list[i].solid_pnode->solid.connect_flag == DISCONNECT )
				{// solid out off line
					continue;
				}
			}
			else
			{
				continue;
			}

			if ((cur_time - gccu_recieve_model_list[i].model_last_time) < CCU_RECV_PROTECT_TIMEOUT)
			{
				continue;
			}
			
			if( (gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_OK) ||\
				(gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT))
			{
				if( (gccu_recieve_model_list[i].chanel_connect_num < PER_CCU_CONNECT_MAX_NUM) &&\
					(gccu_recieve_model_list[i].unconnect_channel_head.list.next != gccu_recieve_model_list[i].unconnect_channel_head.list.prev))
				{// 从头结点开始赋值，因为当无可用通道是未连接通道表是没有值的
					/*
					  *寻找空闲超时的，且连接次数最少的
					  */
					gccu_recieve_model_list[i].model_last_time = cur_time;
					p_temp_inchannel  = list_entry( gccu_recieve_model_list[i].unconnect_channel_head.list.next, TInChannel, list );
					if ((NULL != p_temp_inchannel) && (NULL == p_lestest_cnnt_inchannel))
					{
						p_lestest_cnnt_inchannel = p_temp_inchannel;
					}
					else if (NULL != p_temp_inchannel)
					{
						if (p_temp_inchannel->channel_connected_count < p_lestest_cnnt_inchannel->channel_connected_count)
						{
							p_lestest_cnnt_inchannel = p_temp_inchannel;
							p_temp_inchannel = NULL;
						}
					}
					
					//*pp_InChannel = list_entry( gccu_recieve_model_list[i].unconnect_channel_head.list.next, TInChannel, list );
					//bret = true;
					//break;
				}
				else if( (gccu_recieve_model_list[i].chanel_connect_num >= PER_CCU_CONNECT_MAX_NUM ))
				{
					continue;
				}
			}
			else
			{
				continue;
			}
		}
	}

	if (NULL != p_lestest_cnnt_inchannel)
	{
		*pp_InChannel = p_lestest_cnnt_inchannel;
		bret = true;
	}

	return bret;
}

// 找到可用的连接通道，并赋给gchannel_allot_pro.p_current_input_channel (unfinish 2016-3-11)
static bool central_control_found_available_channel( void )//(unfinish 2016-3-11)
{
	bool bret = false;
	
	ccu_recv_unit_debug( " -----elem num = %d, gchannel_allot_pro.cnnt_num = %d ------", \
		gchannel_allot_pro.elem_num, gchannel_allot_pro.cnnt_num );
	if( gchannel_allot_pro.elem_num == 0 )
	{
		gchannel_allot_pro.p_current_input_channel = NULL;
	}
	else if( gchannel_allot_pro.cnnt_num < CHANNEL_MUX_NUM )
	{
		if(central_control_search_connect_by_arithmetic( &gchannel_allot_pro.p_current_input_channel ))
			bret = true;
	}
	else if( gchannel_allot_pro.cnnt_num >= CHANNEL_MUX_NUM )
	{// 若超出了系统的连接数，则断开其中一个连接,连接时间最长的
		bret = false;
	}
	
	return bret;
}


// extern function
// 初始化通道节点
int init_central_control_recieve_unit_by_entity_id( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id )
{	
	solid_pdblist solid_node = NULL;
	struct jdksavdecc_string entity_name;// 终端avb名字
	struct jdksavdecc_descriptor_stream stream_input_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_input_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	ccu_recv_unit_debug( "avdecc_read_descriptor_error: stream_input_desc_read error" );
		return -1;
        }

	if( stream_input_desc.descriptor_index > 3 )
	{
        	ccu_recv_unit_debug( "stream_input_desc.descriptor_index = %d out of range:  error",stream_input_desc.descriptor_index);
		return -1;
	}

	assert( endpoint_list && descptor_guard );
	if( (endpoint_list == NULL) || (descptor_guard == NULL) \
		|| (endtity_id == 0) ||desc_node == NULL )
	{
		return -1;
	}

	// 检查是否是中央控制接收模块
	memcpy( &entity_name, &desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
#ifdef CCU_TR_MODEL_RECIEVE_ENABLE
	if( (strcmp( (char*)entity_name.value, CCU_TR_MODEL_NAME) == 0) ||\
		(strcmp( (char*)entity_name.value, CCU_R_MODEL_NAME) == 0) )
	{
		
	}
	else 
	{
		ccu_recv_unit_debug( "not a right recieve model = %s:ringname is %s or %s", \
			(char*)entity_name.value, CCU_TR_MODEL_NAME, CCU_R_MODEL_NAME );
		return -1;
	}
#else 
	if( strcmp( (char*)entity_name.value, CCU_R_MODEL_NAME) != 0 )
	{
		ccu_recv_unit_debug( "not a right recieve model = %s:ringname is %s ", (char*)entity_name.value, CCU_R_MODEL_NAME );
		return -1;
	}
#endif

	solid_node = search_endtity_node_endpoint_dblist( endpoint_list, endtity_id );
	if( solid_node == NULL )
	{
		ccu_recv_unit_debug( "central control not found solid endtity( id = 0x%016llx)", endtity_id );
		return -1;
	}

	/*更新连接状态add in 2016-5-17*/
	//acmp_rx_state_avail( endtity_id, stream_input_desc.descriptor_index );

	/*
	**寻找可用模块
	**1、若模块从未初始化，则选择当前元素
	**2、若模块被初始化了，则判断当前模块是否能继续插入:若某个模块不超出其通道数，则建立
	**3、若无可用模块或某个模块超出其通道数，则直接返回或继续遍历下一个模块
	*/
	int i = 0, insert_index = -1;
	bool isfound = false, init_found = false;
	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		if((gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_OK) &&\
			(gccu_recieve_model_list[i].entity_id == endtity_id))
		{
			insert_index = i;
			isfound = true;
			break;
		}
	}

	if( !isfound )
	{
		for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
		{
			if((gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_UNINIT) &&\
				(gccu_recieve_model_list[i].entity_id == 0))
			{
				insert_index = i;
				init_found = true;
				break;
			}
		}
	}

	if( !isfound && !init_found )
	{
		ccu_recv_unit_debug( " not found model_list!!!" );
		return -1;
	}
	
	if( (isfound && insert_index != -1) || (init_found &&  insert_index != -1 ))
	{
		if( gccu_recieve_model_list[insert_index].channel_num < CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM )
		{
			T_pInChannel new_ch_node = intput_channel_list_node_create();
			assert( new_ch_node );
			if( new_ch_node != NULL )
			{
				INIT_ZERO(new_ch_node, sizeof(TInChannel) );
				input_channel_list_node_init( new_ch_node, endtity_id,  stream_input_desc.descriptor_index );
				input_channel_list_add_trail( new_ch_node, &gccu_recieve_model_list[insert_index].unconnect_channel_head.list );

				new_ch_node->status = INCHANNEL_FREE;
				new_ch_node->pro_status = INCHANNEL_PRO_FINISH;
				if( gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_UNINIT )
				{//  init Node for the first time
					gccu_recieve_model_list[i].entity_id = endtity_id;
					gccu_recieve_model_list[i].solid_pnode = solid_node;
					gccu_recieve_model_list[i].desc_pnode = desc_node;
					gccu_recieve_model_list[i].model_state = CCU_RECIEVE_MODEL_OK;
					gccu_recieve_model_list[i].model_last_time = 0;
					pthread_mutex_init( &gccu_recieve_model_list[insert_index].RModel_mutex, NULL );
				}
				
				if( (++gccu_recieve_model_list[insert_index].channel_num) >= CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM )// number of channel in model 1
				{
					gccu_recieve_model_list[insert_index].model_state = CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT;
				}

				gchannel_allot_pro.elem_num++;
				ccu_recv_unit_debug( "One intput channel add Success....................( ID = 0x%016llx -- input index = %d )", endtity_id, stream_input_desc.descriptor_index );
			}
			else
			{
				ccu_recv_unit_debug( "Create new node failed: no space or other!!!" );
				return -1;
			}
		}
		else
		{
				ccu_recv_unit_debug( " Model input change num is out of range!" );
				return -1;
		}	
	}
	
	return 0;
}

// 用于更新接收模块通道连接的情况
void central_control_recieve_ccu_model_state_update( subject_data_elem connect_info )
{
	const bool cnnt_flag = connect_info.connect_flag;
	T_pInChannel p_temp_chNode = NULL;
	int i = 0;
	
	//ccu_recv_unit_debug( "msg = %d, subtype = %d",  connect_info.ctrl_msg.msg_type, connect_info.ctrl_msg.data_type );
	if( (connect_info.ctrl_msg.data_type != JDKSAVDECC_SUBTYPE_ACMP) ||\
		connect_info.ctrl_msg.msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE ||
		connect_info.ctrl_msg.msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE )
		return ;
	
	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		if( cnnt_flag )// connect success?
		{
			list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].unconnect_channel_head.list, list )
			{
				if( p_temp_chNode->pro_status == INCHANNEL_PRO_HANDLING )
					p_temp_chNode->pro_status = INCHANNEL_PRO_FINISH;
				
				if( (connect_info.listener_id == p_temp_chNode->listener_id) &&\
					(connect_info.listener_index == p_temp_chNode->listener_index))
				{// cut from unconnect list and add to connect list
					__list_del_entry(&p_temp_chNode->list);
					p_temp_chNode->tarker_id = connect_info.tarker_id;
					p_temp_chNode->tarker_index = connect_info.tarker_index;
					p_temp_chNode->status = INCHANNEL_BUSY;
					p_temp_chNode->pro_status = INCHANNEL_PRO_FINISH;
					p_temp_chNode->timetimp = get_current_time();
					p_temp_chNode->channel_connected_count++;
					input_channel_list_add_trail( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list );
					gccu_recieve_model_list[i].chanel_connect_num++;
					gccu_recieve_model_list[i].model_last_time = get_current_time();
					gchannel_allot_pro.cnnt_num++;
					gchannel_allot_pro.pro_eflags = CH_ALLOT_FINISH;

					ccu_recv_unit_debug( "CCU CONNECT update Success!(tarker 0x%016llx:%d-model cnnt num = %d)-(0x%016llx:%d)-(sum cnnt num = %d)", 
						p_temp_chNode->tarker_id,
						p_temp_chNode->tarker_index,
						gccu_recieve_model_list[i].chanel_connect_num,
						connect_info.listener_id, 
						connect_info.listener_index,
						gchannel_allot_pro.cnnt_num);
					return;
				}
			}
		}
		else
		{
			list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
			{
				if( p_temp_chNode->pro_status == INCHANNEL_PRO_HANDLING )
					p_temp_chNode->pro_status = INCHANNEL_PRO_FINISH;
				
				if( (connect_info.listener_id == p_temp_chNode->listener_id) &&\
					(connect_info.listener_index == p_temp_chNode->listener_index))
				{// cut from unconnect list and add to connect list
					__list_del_entry(&p_temp_chNode->list);
					input_channel_list_add_trail( p_temp_chNode, &gccu_recieve_model_list[i].unconnect_channel_head.list );
					p_temp_chNode->status = INCHANNEL_FREE;
					gccu_recieve_model_list[i].chanel_connect_num--;
					gccu_recieve_model_list[i].model_last_time = get_current_time();
					gchannel_allot_pro.cnnt_num--;
					gchannel_allot_pro.pro_eflags = CH_ALLOT_FINISH;

					ccu_recv_unit_debug( "CCU DISCONNECT Success!(tarker 0x%016llx:%d-model cnnt num = %d)-(0x%016llx:%d)-(sum cnnt num = %d)", 
						p_temp_chNode->tarker_id,
						p_temp_chNode->tarker_index,
						gccu_recieve_model_list[i].chanel_connect_num,
						connect_info.listener_id, 
						connect_info.listener_index,
						gchannel_allot_pro.cnnt_num );
					return;
				}
			}
		}
	}
}

bool ccu_recv_model_talker_connected( uint64_t  talker_id, uint16_t talker_index )
{
	T_pInChannel p_temp_chNode = NULL;
	int i = 0;
	bool bret = false;
	
	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
		{
			if( (p_temp_chNode->tarker_id == talker_id) &&\
				(p_temp_chNode->tarker_index == talker_index) )
			{
				bret = true;
				break;
			}
		}

		if( bret )
			break;
	}

	return bret;
}

bool ccu_recv_model_talker_connected_listener_id_index( uint64_t  talker_id, uint16_t talker_index, uint64_t *out_listen, uint16_t* listen_index )
{
	T_pInChannel p_temp_chNode = NULL;
	int i = 0;
	bool bret = false;

	if ( out_listen == NULL || listen_index == NULL )
		return false;
	
	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
		{
			if( (p_temp_chNode->tarker_id == talker_id) &&\
				(p_temp_chNode->tarker_index == talker_index) )
			{
				*out_listen = p_temp_chNode->listener_id;
				*listen_index = p_temp_chNode->listener_index;
				bret = true;
				break;
			}
		}

		if( bret )
			break;
	}

	return bret;
}

#define CCU_RECV_M_EV_MAX 20
enum ccu_recv_model_enum_event
{
	TALKER_CONNECT = 1,
	TALKER_DISCONNECT = 2,
};

enum ccu_recv_model_event_elemt_status
{
	elemt_sink_free,
	elemt_sink_busy
};

struct ccu_recv_model_event_elemt
{
	uint64_t talker_id;
	TInChannel *p_input_channel;
	uint16_t pro_times;// 事件处理次数
	uint16_t talker_index;
	enum ccu_recv_model_enum_event event_type;// 事件类型
	enum ccu_recv_model_event_elemt_status elem_status;
};

struct ccu_recv_model_event
{
	struct ccu_recv_model_event_elemt event_elemts[CCU_RECV_M_EV_MAX];
	uint8_t event_counts;
	uint8_t event_cur_index;
};

struct ccu_recv_model_event gccu_recv_model_event_pro;// 中央接收模块事件处理

int ccu_recv_model_event_add( struct ccu_recv_model_event_elemt event )
{
	if (gccu_recv_model_event_pro.event_counts < CCU_RECV_M_EV_MAX)
	{
		int i = 0, ins_index = -1;
		for (i = 0; i < CCU_RECV_M_EV_MAX;++i)
		{
			if (gccu_recv_model_event_pro.event_elemts[i].elem_status == elemt_sink_free)
			{
				ins_index = i;
				break;
			}
		}

		if (ins_index != -1)
		{
			gccu_recv_model_event_pro.event_elemts[ins_index].event_type = event.event_type;
			gccu_recv_model_event_pro.event_elemts[ins_index].elem_status = event.elem_status;
			gccu_recv_model_event_pro.event_elemts[ins_index].pro_times = event.pro_times;
			gccu_recv_model_event_pro.event_elemts[ins_index].p_input_channel = event.p_input_channel;
			gccu_recv_model_event_pro.event_elemts[ins_index].talker_id = event.talker_id;
			gccu_recv_model_event_pro.event_elemts[ins_index].talker_index = event.talker_index;
			++gccu_recv_model_event_pro.event_counts;
			return 0;
		}
		
		return -1;
	}

	return -1;
}

void central_control_recieve_unit_event_pro(void)
{
	if (gccu_recv_model_event_pro.event_counts > 0)
	{
		int i = 0, j = 0;
		bool found_entity = false;
		uint32_t cur_time = get_current_time();
		bool time_out = false;
		
		for (i = 0; i < CCU_RECV_M_EV_MAX;++i)
		{
			bool no_pro = true;
			if (gccu_recv_model_event_pro.event_elemts[i].p_input_channel != NULL &&
				(gccu_recv_model_event_pro.event_elemts[i].elem_status == elemt_sink_busy))
				no_pro = false;

			if (!no_pro)
			{
				for( j = 0; j < CCU_TR_MODEL_MAX_NUM; j++ )
				{
					if (gccu_recieve_model_list[j].entity_id ==\
						gccu_recv_model_event_pro.event_elemts[i].p_input_channel->listener_id)
					{
						found_entity = true;
						no_pro = false;
						break;
					}
				}
			}

			if (!found_entity)
			{
				gccu_recv_model_event_pro.event_elemts[i].p_input_channel = NULL;
				gccu_recv_model_event_pro.event_elemts[i].event_type = 0;
				gccu_recv_model_event_pro.event_elemts[i].elem_status = elemt_sink_free;
			}
			else
			{
				if (cur_time - gccu_recieve_model_list[j].model_last_time > CCU_RECV_PROTECT_TIMEOUT)
				{
					time_out = true;
				}
			}

			if (!no_pro)
			{
				if (gccu_recv_model_event_pro.event_elemts[i].pro_times <= 0 ||\
					(gccu_recv_model_event_pro.event_elemts[i].elem_status != elemt_sink_busy))
				{
					no_pro = true;
				}
			}

			if (!no_pro && time_out)
			{
				struct jdksavdecc_eui64 talker_entity_id;
				struct jdksavdecc_eui64 listener_entity_id;
				uint64_t taker_id = gccu_recv_model_event_pro.event_elemts[i].talker_id;
				uint16_t taker_index = gccu_recv_model_event_pro.event_elemts[i].talker_index;
				uint64_t listen_id = gccu_recv_model_event_pro.event_elemts[i].p_input_channel->listener_id;
				uint16_t listen_index = gccu_recv_model_event_pro.event_elemts[i].p_input_channel->listener_index;
				convert_uint64_to_eui64( talker_entity_id.value, taker_id);
				convert_uint64_to_eui64( listener_entity_id.value, listen_id );

				if (gccu_recv_model_event_pro.event_elemts[i].event_type == TALKER_CONNECT)
				{
					gccu_recv_model_event_pro.event_elemts[i].p_input_channel->pro_status = INCHANNEL_PRO_PRIMED;
					acmp_connect_avail( talker_entity_id.value, 
						taker_index, 
						listener_entity_id.value, 
						listen_index, 
						1, ++gccu_acmp_sequeue_id );
					gccu_recv_model_event_pro.event_elemts[i].p_input_channel->pro_status = INCHANNEL_PRO_HANDLING;
				}
				else if ((gccu_recv_model_event_pro.event_elemts[i].event_type == TALKER_DISCONNECT))
				{
					conference_recieve_model_connect_self( taker_id );// 重连本机
				
					gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_PRIMED;
					acmp_disconnect_avail( talker_entity_id.value, 
								taker_index, 
								listener_entity_id.value, 
								listen_index, 
								1, ++gccu_acmp_sequeue_id );
					gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_HANDLING;
				}

				gccu_recieve_model_list[j].model_last_time = cur_time;
				gccu_recv_model_event_pro.event_elemts[i].pro_times--;
				gccu_recv_model_event_pro.event_counts = (gccu_recv_model_event_pro.event_counts-1)%CCU_RECV_M_EV_MAX;
				gccu_recv_model_event_pro.event_elemts[i].p_input_channel = NULL;
				gccu_recv_model_event_pro.event_elemts[i].event_type = 0;
				gccu_recv_model_event_pro.event_elemts[i].elem_status = elemt_sink_free;

				if (gccu_recv_model_event_pro.event_counts ==0 )
				{
					break;
				}
			}
		}
	}
}

int ccu_recv_model_talk( uint64_t  talker_id, uint16_t talker_index )
{
#if 1
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	int ret = -1;
	
	if( central_control_found_available_channel() )
	{
		assert( gchannel_allot_pro.p_current_input_channel != NULL );
		if( gchannel_allot_pro.p_current_input_channel != NULL )
		{
			if( gchannel_allot_pro.p_current_input_channel->status == INCHANNEL_FREE )
			{
				conference_recieve_model_discut_self( talker_id );// 断开本机
				
				gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_PRIMED;
				convert_uint64_to_eui64( talker_entity_id.value, talker_id );
				convert_uint64_to_eui64( listener_entity_id.value, gchannel_allot_pro.p_current_input_channel->listener_id );
				acmp_connect_avail( talker_entity_id.value, 
								talker_index, 
								listener_entity_id.value, 
								gchannel_allot_pro.p_current_input_channel->listener_index, 
								3, ++gccu_acmp_sequeue_id );
				gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_HANDLING;
				ret = 0;
			}
		}
	}
	else 
	{
		ccu_recv_unit_debug( " has not a valid input channel to talk to !" );
	}

	return ret;
#else
	int ret = -1;
	
	if( central_control_found_available_channel() )
	{
		assert( gchannel_allot_pro.p_current_input_channel != NULL );
		if( gchannel_allot_pro.p_current_input_channel != NULL )
		{
			if( gchannel_allot_pro.p_current_input_channel->status == INCHANNEL_FREE )
			{
				struct ccu_recv_model_event_elemt event;
				event.event_type = TALKER_CONNECT;
				event.elem_status = elemt_sink_busy;
				event.pro_times = 1;
				event.p_input_channel = gchannel_allot_pro.p_current_input_channel;
				event.talker_id = talker_id;
				event.talker_index = talker_index;
				ccu_recv_model_event_add(event);
				if ( 0 == ccu_recv_model_event_add(event))
					ret = 0;
			}
		}
	}
	else 
	{
		ccu_recv_unit_debug( " has not a valid input channel to talk to !" );
	}

	return ret;
#endif
}

int ccu_recv_model_untalk( const uint64_t  talker_id, const uint16_t talker_index )
{
#if 1
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	T_pInChannel p_temp_chNode = NULL;
	int i = 0;

	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
		{
			if( (p_temp_chNode->tarker_id == talker_id) &&\
				(p_temp_chNode->tarker_index == talker_index) )
			{
				conference_recieve_model_connect_self( talker_id );// 重连本机
				
				gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_PRIMED;
				convert_uint64_to_eui64( talker_entity_id.value, talker_id );
				convert_uint64_to_eui64( listener_entity_id.value, p_temp_chNode->listener_id );
				acmp_disconnect_avail( talker_entity_id.value, 
							talker_index, 
							listener_entity_id.value, 
							p_temp_chNode->listener_index, 
							3, ++gccu_acmp_sequeue_id );
				gchannel_allot_pro.p_current_input_channel->pro_status = INCHANNEL_PRO_HANDLING;

				return 0;
			}
		}
	}

	return -1;
#else
	T_pInChannel p_temp_chNode = NULL;
	int i = 0;

	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
		{
			if( (p_temp_chNode->tarker_id == talker_id) &&\
				(p_temp_chNode->tarker_index == talker_index) )
			{
				struct ccu_recv_model_event_elemt event;
				event.event_type = TALKER_DISCONNECT;
				event.elem_status = elemt_sink_busy;
				event.pro_times = 1;
				event.p_input_channel = p_temp_chNode;
				event.talker_id = talker_id;
				event.talker_index = talker_index;
				//ccu_recv_model_event_add(event);
				if ( 0 == ccu_recv_model_event_add(event))
					return 0;

				return -1;
			}
		}
	}

	return -1;
#endif
}

void central_control_recieve_uinit_init_list( void )
{
	int i = 0;
	
	INIT_ZERO( &gchannel_allot_pro, sizeof(gchannel_allot_pro));
	INIT_ZERO( gccu_recieve_model_list, sizeof(gccu_recieve_model_list));
	INIT_ZERO(&gccu_recv_model_event_pro, sizeof(gccu_recv_model_event_pro));

	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		INIT_LIST_HEAD( &gccu_recieve_model_list[i].connect_channel_head.list );
		INIT_LIST_HEAD( &gccu_recieve_model_list[i].unconnect_channel_head.list );
	}

	gccu_recv_model_event_pro.event_cur_index = 0;
	gccu_recv_model_event_pro.event_counts = 0;
	for ( i = 0; i < CCU_RECV_M_EV_MAX; i++)
	{
		gccu_recv_model_event_pro.event_elemts[i].p_input_channel = NULL;
		gccu_recv_model_event_pro.event_elemts[i].event_type = 0;
		gccu_recv_model_event_pro.event_elemts[i].elem_status = elemt_sink_free;
		gccu_recv_model_event_pro.event_elemts[i].pro_times = 0;
		gccu_recv_model_event_pro.event_elemts[i].talker_id = 0;
		gccu_recv_model_event_pro.event_elemts[i].talker_index = 0xffff;
	}

	// 初始化观察者
	init_observer( &gccu_recv_observer, central_control_recieve_ccu_model_state_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gccu_recv_observer );
}

