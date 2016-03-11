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
		return ;
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
				INIT_ZERO(new_ch_node, sizeof( TInChannel));
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
				gchannel_allot_pro.p_current_input_channel = new_ch_node;// 当前可用通道指针是最新连接点
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
static bool central_control_search_connect_by_arithmetic( TccuRModel **pp_current_RModel )//(unfinish 2016-3-11)
{
	bool bret = false;
	TccuRModel * temp_model = NULL;
	
	if( pp_current_RModel == NULL )
	{
		bret = false;
	}
	else
	{
		/*
		**查找算法:
		**		1、找到没有一个连接的模块，优先赋值
		**		2、若都是一个连接，则随机赋值；
		**		3、连接最多的只有最后才赋值
		**
		*/
	}

	return bret;
}

// 找到可用的连接通道，并赋给gchannel_allot_pro.p_current_input_channel (unfinish 2016-3-11)
static bool central_control_found_available_channel( void )//(unfinish 2016-3-11)
{
	bool bret = false;
	
	if( gchannel_allot_pro.elem_num == 0 )
	{
		gchannel_allot_pro.p_current_input_channel = NULL;
	}
	else if( gchannel_allot_pro.cnnt_num < CHANNEL_MUX_NUM )
	{
			if(!central_control_search_connect_by_arithmetic( &gchannel_allot_pro.p_current_input_channel ))
				return false;
	}
	else if( gchannel_allot_pro.cnnt_num >= CHANNEL_MUX_NUM )
	{
		
	}
	
	return bret;
}

int central_control_recieve_ccu_model_state_update()
{
	
}

void central_control_recieve_uinit_init_list( void )
{
	INIT_ZERO( &gchannel_allot_pro, sizeof(gchannel_allot_pro));
	INIT_ZERO( gccu_recieve_model_list, sizeof(gccu_recieve_model_list));
	INIT_LIST_HEAD( &gccu_recieve_model_list.connect_channel_head.list );
	INIT_LIST_HEAD( &gccu_recieve_model_list.unconnect_channel_head.list );
}
