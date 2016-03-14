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

tchannel_allot_pro gchannel_allot_pro;// 全局通道分配处理
TccuRModel gccu_recieve_model_list[CCU_TR_MODEL_MAX_NUM];// 全局中央未连接连接表
uint16_t gccu_acmp_sequeue_id;

observer_t gccu_recv_observer;// 用于更新ucc 接收模块的连接状态

extern solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
extern desc_pdblist descptor_guard;	// 系统中描述符链表哨兵节点

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
        	DEBUG_INFO( "avdecc_read_descriptor_error: stream_input_desc_read error" );
		return -1;
        }

	if( stream_input_desc.descriptor_index > 3 )
	{
        	DEBUG_INFO( "stream_input_desc.descriptor_index = %d out of range:  error",stream_input_desc.descriptor_index);
		return -1;
	}

	assert( endpoint_list && descptor_guard );
	if( (endpoint_list == NULL) || (descptor_guard == NULL) \
		|| (endtity_id == 0) ||desc_node == NULL )
	{
		return -1;
	}

	// 检查是否是中央控制接收模块
	memcpy( &entity_name, desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
#ifdef CCU_TR_MODEL_RECIEVE_ENABLE
	if( (strcmp( &entity_name, CCU_TR_MODEL_NAME) == 0) ||\
		(strcmp( &entity_name, CCU_R_MODEL_NAME) == 0) )
	{
		
	}
	else 
	{
		DEBUG_INFO( "not a right recieve model = %s:ringname is %s", \
			&entity_name, CCU_TR_MODEL_NAME, CCU_R_MODEL_NAME );
		return -1;
	}
#else 
	if( strcmp( &entity_name, CCU_R_MODEL_NAME) != 0 )
	{
		DEBUG_INFO( "not a right recieve model = %s:ringname is %s ", &entity_name, CCU_R_MODEL_NAME );
		return -1;
	}
#endif

	solid_node = search_endtity_node_endpoint_dblist( endpoint_list, endtity_id );
	if( solid_node == NULL )
	{
#ifdef __CCU_RECV_DEBUG__
		DEBUG_INFO( "central control not found solid endtity( id = 0x%016llx)", endtity_id );
#endif
		return -1;
	}

	/*
	**寻找可用模块
	**1、若模块从未初始化，则选择当前元素
	**2、若模块被初始化了，则判断当前模块是否能继续插入:若某个模块不超出其通道数，则建立
	**3、若无可用模块或某个模块超出其通道数，则直接返回或继续遍历下一个模块
	*/
	int i = 0, insert_index = -1;
	bool isfound = false;
	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		if( (gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_UNINIT) ||\
			((gccu_recieve_model_list[i].entity_id == endtity_id) && \
			(gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_OK)) )
		{
			insert_index = i;
			isfound = true;
			break;
		}
	}

	if( isfound && insert_index != -1 )
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

				if( gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_UNINIT )
				{//  init Node for the first time
					gccu_recieve_model_list[i].entity_id = endtity_id;
					gccu_recieve_model_list[i].solid_pnode = solid_node;
					gccu_recieve_model_list[i].desc_pnode = desc_node;
					gccu_recieve_model_list[i].model_state = CCU_RECIEVE_MODEL_OK;
					pthread_mutex_init( &gccu_recieve_model_list[insert_index].RModel_mutex, NULL );
				}
				
				if( (++gccu_recieve_model_list[insert_index].channel_num) >= CCU_APIECE_TR_MODEL_CHANNEL_MAX_NUM )// number of channel in model 1
				{
					gccu_recieve_model_list[insert_index].model_state = CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT;
				}

				gchannel_allot_pro.elem_num++;
#ifdef __CCU_RECV_DEBUG__
				DEBUG_INFO( "One intput channel add Success....................( ID = 0x%016llx -- input index = %d )", endtity_id, stream_input_desc.descriptor_index );
#endif
			}
			else
			{
#ifdef __CCU_RECV_DEBUG__
				DEBUG_INFO( "Create new node failed: no space or other!!!" );
#endif
				return -1;
			}
		}
		else
		{
#ifdef __CCU_RECV_DEBUG__
				DEBUG_INFO( " Model input change num is out of range!" );
#endif
				return -1;
		}	
	}
	
	return 0;
}

// 通过优先算法查找连接节点
static bool central_control_search_connect_by_arithmetic( T_pInChannel* pp_InChannel )//(unfinish 2016-3-11)
{
	int i = 0;
	bool bret = false;
	
	if( pp_InChannel == NULL )
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
			if( (gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_OK) ||\
				(gccu_recieve_model_list[i].model_state == CCU_RECIEVE_MODEL_ALL_CHANNEL_INIT))
			{
				if( (gccu_recieve_model_list[i].chanel_connect_num < PER_CCU_CONNECT_MAX_NUM) &&\
					(gccu_recieve_model_list[i].unconnect_channel_head.list.next != gccu_recieve_model_list[i].unconnect_channel_head.list.prev))
				{// 从头结点开始赋值，因为当无可用通道是未连接通道表是没有值的
					*pp_InChannel = gccu_recieve_model_list[i].unconnect_channel_head.list.next;
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

	return bret;
}

// 找到可用的连接通道，并赋给gchannel_allot_pro.p_current_input_channel (unfinish 2016-3-11)
static bool central_control_found_available_channel( void )//(unfinish 2016-3-11)
{
	bool bret = false;
	int i = 0;
	T_pInChannel p_temp_chNode = NULL, p_next_node = NULL;
	T_pInChannel p_longest_cnntNode = NULL;
	
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

	if( !bret )
	{
		for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
		{
			if( gccu_recieve_model_list[i].chanel_connect_num < PER_CCU_CONNECT_MAX_NUM )
				continue;
		
			list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
			{
				timetype current_time = get_current_time();
				assert( p_temp_chNode );
				p_next_node = p_temp_chNode->list.next;
				if( p_next_node != gccu_recieve_model_list[i].connect_channel_head )
				{
					if( (p_longest_cnntNode == NULL ) )
					{
						if( (current_time-p_next_node->timetimp) > (current_time-p_temp_chNode->timetimp) )
						{
							p_longest_cnntNode = p_next_node;
						}
						else
							p_longest_cnntNode = p_temp_chNode;
					}
					else 
					{
						if((current_time-p_temp_chNode->timetimp) > (current_time-p_longest_cnntNode->timetimp))
						{
							p_longest_cnntNode = p_temp_chNode;
						}

						if((current_time-p_next_node->timetimp) > (current_time-p_longest_cnntNode->timetimp))
						{
							p_longest_cnntNode = p_next_node;
						}
					}
				}
				else
				{// seach next model connect channel
					if( p_longest_cnntNode == NULL )
					{
						p_longest_cnntNode = p_temp_chNode;
					}
					else 
					{
						if((current_time-p_temp_chNode->timetimp) > (current_time-p_longest_cnntNode->timetimp))
						{
							p_longest_cnntNode = p_temp_chNode;
						}
					}
					
					break;
				}
			}
		}

		if( p_longest_cnntNode != NULL )
		{// send a disconnect cmd
			gchannel_allot_pro.p_current_input_channel = p_longest_cnntNode;

			struct jdksavdecc_eui64 talker_entity_id;
			struct jdksavdecc_eui64 listener_entity_id;
			convert_uint64_to_eui64( talker_entity_id.value, p_longest_cnntNode->tarker_id );
			convert_uint64_to_eui64( listener_entity_id.value, p_longest_cnntNode->listener_id );
			
			gchannel_allot_pro.pro_eflags = CH_ALLOT_PRIMED;
			acmp_disconnect_avail( talker_entity_id.value, p_longest_cnntNode, listener_entity_id.value, p_longest_cnntNode, 1, ++gccu_acmp_sequeue_id );
			gchannel_allot_pro.pro_eflags = CH_ALLOT_HANDLING;
			gchannel_allot_pro.pro_stype = CH_DISCONNECT;
			bret = true;
		}
	}

	// has searched woring :1\disconnect. 2\delect from connect list 3\add to unconnect list
	return bret;
}

// 用于更新接收模块通道连接的情况
void central_control_recieve_ccu_model_state_update( subject_data_elem connect_info )
{
	const bool cnnt_flag = connect_info.connect_flag;
	T_pInChannel p_temp_chNode = NULL;
	struct list_head  *p_channellist = NULL;
	int i = 0;

	for( i = 0; i < CCU_TR_MODEL_MAX_NUM; i++ )
	{
		if( cnnt_flag )// connect success?
		{
			list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].unconnect_channel_head.list, list )
			{
				if( (connect_info.listener_id == p_temp_chNode->listener_id) &&\
					(connect_info.listener_index == p_temp_chNode->listener_index))
				{// cut from unconnect list and add to connect list
					__list_del_entry(&p_temp_chNode->list);
					input_channel_list_add_trail( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head );
					p_temp_chNode->tarker_id = connect_info->tarker_id;
					p_temp_chNode->tarker_index = connect_info->tarker_index;
					p_temp_chNode->status = INCHANNEL_BUSY;
					p_temp_chNode->timetimp = get_current_time();
					gccu_recieve_model_list[i].chanel_connect_num++;
					gchannel_allot_pro.cnnt_num++;
					gchannel_allot_pro.pro_eflags = CH_ALLOT_FINISH;
					DEBUG_INFO( "CCU RECV model update.......Success!" );
					return;
				}
			}
		}
		else
		{
			list_for_each_entry( p_temp_chNode, &gccu_recieve_model_list[i].connect_channel_head.list, list )
			{
				if( (connect_info.listener_id == p_temp_chNode->listener_id) &&\
					(connect_info.listener_index == p_temp_chNode->listener_index))
				{// cut from unconnect list and add to connect list
					__list_del_entry(&p_temp_chNode->list);
					input_channel_list_add_trail( p_temp_chNode, &gccu_recieve_model_list[i].unconnect_channel_head );
					p_temp_chNode->status = INCHANNEL_FREE;
					gccu_recieve_model_list[i].chanel_connect_num--;
					gchannel_allot_pro.cnnt_num--;
					gchannel_allot_pro.pro_eflags = CH_ALLOT_FINISH;
					DEBUG_INFO( "CCU RECV model update.......Success!" );
					return;
				}
			}
		}
	}
}

void ccu_recv_model_talk( uint64_t  talker_id, uint16_t talker_index )
{
	struct jdksavdecc_eui64 talker_entity_id;
	struct jdksavdecc_eui64 listener_entity_id;
	
	if( central_control_found_available_channel() )
	{
		assert( gchannel_allot_pro.p_current_input_channel != NULL );
		if( gchannel_allot_pro.p_current_input_channel != NULL )
		{
			convert_uint64_to_eui64( talker_entity_id.value, talker_id );
			convert_uint64_to_eui64( listener_entity_id.value, gchannel_allot_pro.p_current_input_channel->listener_id );
			acmp_connect_avail( talker_entity_id.value, 
							talker_index, 
							listener_entity_id, 
							gchannel_allot_pro.p_current_input_channel->listener_index, 
							1, ++gccu_acmp_sequeue_id );
		}
	}
	else 
	{
#ifdef __CCU_RECV_DEBUG__
		DEBUG_INFO( " has not a valid input channel to talk to !" );
#endif
	}
}

void central_control_recieve_uinit_init_list( void )
{
	INIT_ZERO( &gchannel_allot_pro, sizeof(gchannel_allot_pro));
	INIT_ZERO( gccu_recieve_model_list, sizeof(gccu_recieve_model_list));
	INIT_LIST_HEAD( &gccu_recieve_model_list.connect_channel_head.list );
	INIT_LIST_HEAD( &gccu_recieve_model_list.unconnect_channel_head.list );

	// 初始化观察者
	init_observer( &gccu_recv_observer, central_control_recieve_ccu_model_state_update );
	// 加入观察者到被观察者
	attach_observer( &gconnector_subjector, &gccu_recv_observer );
}

