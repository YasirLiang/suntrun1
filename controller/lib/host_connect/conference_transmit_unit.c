/*
**conference_transmit_unit.c
**9-3-2016
**
*/

// ********************************
// file built 2016-3-9
// ��������ϵͳ���ӱ�������
// ���鵥Ԫ����ģ��
// ********************************

#include "conference_transmit_unit.h"
#include "jdksavdecc_aem_descriptor.h"
#include "central_control_recieve_unit.h"
#include "util.h"
#include "terminal_pro.h"
#include "acmp_controller_machine.h"

#ifdef __DEBUG__
//#define __CONFERENCE_TRANSMIT_DEBUG__
#endif

#ifdef __CONFERENCE_TRANSMIT_DEBUG__
#define conference_transmit_unit_debug(fmt, args...) \
	fprintf( stdout,"\033[32m %s-%s-%d:\033[0m "fmt" \r\n", __FILE__, __func__, __LINE__, ##args);
#else
#define conference_transmit_unit_debug(fmt, args...)
#endif

tconference_trans_model gconference_model_guard;// ���鴫�䵥Ԫ����ȫ��ͷ���
uint16_t gconference_tramsmit_acmp_sequeue_id = 0;

observer_t gconference_trans_observer;// ���ڸ��»��鵥Ԫģ�������״̬

void  trans_model_node_insert_to_list( tconference_trans_pmodel p_new_node)
{
	assert( NULL != p_new_node );
	if( NULL == p_new_node )
		return;
	
	list_add_tail(  &p_new_node->list, &gconference_model_guard.list );
}

// ��ʼ�����鵥Ԫ���ͨ����Ϣ
int conference_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const uint64_t endtity_id, const desc_pdblist desc_node )
{
	struct jdksavdecc_string entity_name;// �ն�avb����
	struct jdksavdecc_descriptor_stream stream_output_desc; // Structure containing the stream_output_desc fields
	ssize_t ret = jdksavdecc_descriptor_stream_read( &stream_output_desc, frame, pos, frame_len );
        if (ret < 0)
        {
        	conference_transmit_unit_debug( "avdecc_read_descriptor_error: stream_input_desc_read error" );
		return -1;
        }

	if( stream_output_desc.descriptor_index > CONFERENCE_OUTCHANNEL_MAX_NUM )
	{
        	conference_transmit_unit_debug( "stream_input_desc.descriptor_index = %d out of range:  error",stream_output_desc.descriptor_index);
		return -1;
	}

	// ����Ƿ�Ϊ������ƽ���ģ��
	memcpy( &entity_name, &desc_node->endpoint_desc.entity_name, sizeof(struct jdksavdecc_string));
	if( (strcmp((char*) &entity_name, CCU_TR_MODEL_NAME) == 0) ||\
		(strcmp( (char*)&entity_name, CCU_R_MODEL_NAME) == 0) )
	{
		conference_transmit_unit_debug( "entity not a conference uinit %s", (char*)&entity_name.value);
		return -1;
	}

	/*��������״̬add in 2016-5-17*/
	acmp_tx_state_avail( endtity_id, stream_output_desc.descriptor_index );

	//********* 
	//�����Ƿ����endtity_id ������ڵ�
	tconference_trans_pmodel p_temp_node = NULL;
	bool cfc_node_found = false;
	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( p_temp_node->tarker_id == endtity_id )
		{
			cfc_node_found = true;
			break;
		}
	}

	if( cfc_node_found )
	{
		T_pOutChannel p_node = out_channel_node_create_can_init();
		if( p_node != NULL )
		{
			output_channel_node_init_by_index( p_node, stream_output_desc.descriptor_index );
			INIT_LIST_HEAD( &p_node->input_head.list );// ��ʼ��outstream��Ӧ��inputstream����
			output_channel_insert_node_to_list( &p_temp_node->out_ch.list, p_node );
			
#ifdef __CONFERENCE_TRANSMIT_DEBUG__
			conference_transmit_unit_debug( "out Stream add ONE<<<<<<<<------------------" );

			Input_pChannel Input_pnode = NULL;
			T_pOutChannel p_Out = NULL;
			int in_num = 0, out_num = 0;
			list_for_each_entry( Input_pnode, &p_node->input_head.list, list )
			{
				in_num++;
			}

			list_for_each_entry( p_Out, &p_temp_node->out_ch.list, list )
			{
				out_num++;
			}

			conference_transmit_unit_debug( "out Stream(NUM = %d) has input Stream Num = %d------------------", out_num,in_num );
#endif
		}
	}
	else
	{
		p_temp_node = (tconference_trans_pmodel)malloc( sizeof(tconference_trans_model) );
		if( p_temp_node != NULL )
		{
			p_temp_node->confenrence_node = NULL;
			p_temp_node->tarker_id = endtity_id;
			p_temp_node->model_speak_time.elapsed = false;
			p_temp_node->model_speak_time.running = false;
			INIT_LIST_HEAD( &p_temp_node->out_ch.list );// ��һ��outstream ��ʼ��

			T_pOutChannel p_node = out_channel_node_create_can_init();
			if( p_node != NULL )
			{
				output_channel_node_init_by_index( p_node, stream_output_desc.descriptor_index );
				output_channel_insert_node_to_list( &p_temp_node->out_ch.list, p_node );

#ifdef __CONFERENCE_TRANSMIT_DEBUG__
				conference_transmit_unit_debug( "out Stream add ONE1111111111<<<<<<<<------------------" );

				Input_pChannel Input_pnode = NULL;
				T_pOutChannel p_Out = NULL;
				int in_num = 0, out_num = 0;
				list_for_each_entry( Input_pnode, &p_node->input_head.list, list )
				{
					in_num++;
				}

				list_for_each_entry( p_Out, &p_temp_node->out_ch.list, list )
				{
					out_num++;
				}

				conference_transmit_unit_debug( "out Stream(NUM = %d) has input Stream Num = %d------------------", out_num,in_num );
#endif
			}
			
			trans_model_node_insert_to_list( p_temp_node );
		}
	}

	return 0;
}

// ��ʼ�����鴫�䵥Ԫ�Ļ��鵥Ԫ�ڵ�
int conference_transmit_unit_init_conference_node( const tmnl_pdblist p_tmnl_node, const uint64_t tarker_id )
{
	tconference_trans_pmodel p_temp_node = NULL;
	bool found = false;
	
	assert( NULL != p_tmnl_node );
	if( NULL == p_tmnl_node )
	{
		return -1;
	}

	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( p_temp_node->tarker_id == tarker_id )
		{
			if( p_temp_node->confenrence_node == NULL )
				p_temp_node->confenrence_node = p_tmnl_node;
			
			found = true;
			break;
		}
	}

	return (found?0:-1);
}

bool trans_model_unit_is_connected( uint64_t tarker_id )
{
	return ccu_recv_model_talker_connected( tarker_id, CONFERENCE_OUTPUT_INDEX );
}

int trans_model_unit_connect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node )// return -1; means that there is no ccu reciever model 
{
	if( 0 == conference_transmit_unit_init_conference_node( p_tmnl_node, tarker_id ) )
		return ccu_recv_model_talk( tarker_id, CONFERENCE_OUTPUT_INDEX );

	return -1;
}

int trans_model_unit_disconnect( uint64_t tarker_id, const tmnl_pdblist p_tmnl_node ) // return -1 means talker not connect
{
	if( 0 == conference_transmit_unit_init_conference_node( p_tmnl_node, tarker_id ) )
		return ccu_recv_model_untalk( tarker_id, CONFERENCE_OUTPUT_INDEX );

	return -1;
}

/*�Ͽ�����ʱ����Ļ����ն�,ֻ������ϯvip��pptģʽ��ʹ�÷���0:����*/
int trans_model_unit_disconnect_longest_connect( void )
{
	tconference_trans_pmodel p_temp_node = NULL, longest = NULL;
	timetype curtime = get_current_time();
	int ret = -1;
	
	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( !p_temp_node->model_speak_time.running )
			continue;

		if (longest == NULL)
		{
			longest = p_temp_node;
		}
		else
		{
			if ( (curtime - longest->model_speak_time.start_time) < \
				(curtime - p_temp_node->model_speak_time.start_time))
			longest = p_temp_node;
		}
	}

	if (NULL != longest)
	{// found
		ret = ccu_recv_model_untalk( longest->tarker_id, CONFERENCE_OUTPUT_INDEX );
	}

	return ret;
}

void trans_model_unit_update( subject_data_elem connect_info )// ���´���ģ�������״̬, ������֪ͨ����ϵͳЭ�����Ϣ
{
	const bool cnnt_flag = connect_info.connect_flag;
	tconference_trans_pmodel p_temp_node = NULL;

	if( connect_info.ctrl_msg.msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE ||
		connect_info.ctrl_msg.msg_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_TX_STATE_RESPONSE )
		return ;

	if( !cnnt_flag )// connferenc Mic close
	{
		list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
		{
			if( p_temp_node->tarker_id == connect_info.tarker_id )
			{
				T_pOutChannel p_Outnode = NULL;
				
				if( NULL != p_temp_node->confenrence_node)
					terminal_mic_status_set_callback( false, p_temp_node->confenrence_node );
				
				list_for_each_entry(p_Outnode, &p_temp_node->out_ch.list, list)
				{
					if( p_Outnode->tarker_index == connect_info.tarker_index )
					{
						if (p_Outnode->tarker_index == CONFERENCE_OUTPUT_INDEX)
						{// ֹͣ���Լ�ʱ
							host_timer_stop(&p_temp_node->model_speak_time);
						}
						
						Input_pChannel Input_pnode = NULL;
						list_for_each_entry( Input_pnode, &p_Outnode->input_head.list, list )
						{
							if(  Input_pnode->listener_id == connect_info.listener_id&& \
								(Input_pnode->listen_index== connect_info.listener_index))// found?
							{
								__list_del_entry(&Input_pnode->list);// delect connect input node
								if( Input_pnode != NULL )
								{
									free(Input_pnode);
									Input_pnode = NULL;	
								}
								conference_transmit_unit_debug( "conference unit tranmist  model update.......Success!(tarker :index)(0x%016llx:%d)--(listen :index)(0x%016llx:%d)",\
										connect_info.tarker_id, connect_info.tarker_index, connect_info.listener_id, connect_info.listener_index );
								return;
							}
						}
					}
				}
			}
		}
	}
	else// connferenc Mic open
	{
		list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
		{
			if( p_temp_node->tarker_id == connect_info.tarker_id )
			{
				T_pOutChannel p_Outnode = NULL;				
				list_for_each_entry(p_Outnode, &p_temp_node->out_ch.list, list)
				{
					if( p_Outnode->tarker_index == connect_info.tarker_index )
					{
						if (p_Outnode->tarker_index == CONFERENCE_OUTPUT_INDEX)
						{// ��ʼ���Լ�ʱ
							host_timer_start( 1000,&p_temp_node->model_speak_time);
						}
						
						Input_pChannel Input_pnode = NULL;
						Input_pnode = input_connect_node_create();
						if( Input_pnode == NULL )
						{
							conference_transmit_unit_debug( "connect info not save Success!!!" );
							return;
						}

						input_connect_node_init_by_index( Input_pnode, connect_info.listener_id, connect_info.listener_index );
						input_connect_node_insert_node_to_list( &p_Outnode->input_head.list, Input_pnode );

						if( NULL != p_temp_node->confenrence_node)
							terminal_mic_status_set_callback( true, p_temp_node->confenrence_node );
						
						conference_transmit_unit_debug( "conference unit tranmist  model update.......Success!(tarker :index)(0x%016llx:%d)-- (listen :index)(0x%016llx:%d)",\
							connect_info.tarker_id, connect_info.tarker_index, connect_info.listener_id, connect_info.listener_index );
						return;
					}
				}

				break;// if found and updata failed, return, not search again;
			}
		}
	}
	
}

int conference_transmit_model_node_destroy( uint64_t tarker_id )
{
	tconference_trans_pmodel p_temp_node = NULL;
	
	list_for_each_entry( p_temp_node, &gconference_model_guard.list, list )
	{
		if( p_temp_node->tarker_id == tarker_id )
		{
			conference_transmit_unit_debug( "destroy ID = 0x%016llx", tarker_id );
			if( p_temp_node->confenrence_node != NULL )
				p_temp_node->confenrence_node = NULL;

			T_pOutChannel p_Outnode = NULL, p_TmpOut = NULL;
			list_for_each_entry(p_Outnode, &p_temp_node->out_ch.list, list)
			{// �ͷ����е�source��ÿ�δ�ͷ��ʼ�ͷ�
				p_TmpOut = &p_temp_node->out_ch;// ��ʱ����ͷ
				Input_pChannel Input_pnode = NULL, p_TmpIn = NULL;
				list_for_each_entry( Input_pnode, &p_Outnode->input_head.list, list )
				{// �Ͽ����ͷ�source ��Ӧ���е�sink����, ÿ�δ�ͷ��ʼ�ͷ�
					struct jdksavdecc_eui64 talker_entity_id;
					struct jdksavdecc_eui64 listener_entity_id;
					convert_uint64_to_eui64( talker_entity_id.value, tarker_id );
					convert_uint64_to_eui64( listener_entity_id.value, Input_pnode->listener_id );
					acmp_disconnect_avail( talker_entity_id.value, 
								p_Outnode->tarker_index, 
								listener_entity_id.value, 
								Input_pnode->listen_index, 
								1, ++gconference_tramsmit_acmp_sequeue_id );
					
					p_TmpIn = &p_Outnode->input_head;
					__list_del_entry(&Input_pnode->list);// delect connect input node
					if( Input_pnode != NULL )
					{
						free(Input_pnode);
						Input_pnode = p_TmpIn;	
					}
				}

				__list_del_entry(&p_Outnode->list);
				if( p_Outnode != NULL )
				{
					free(p_Outnode);
					p_Outnode = p_TmpOut;
				}
			}

			// �ͷ����node
			__list_del_entry(&p_temp_node->list);
			if( p_temp_node != NULL )
			{
				free(p_temp_node);
				p_temp_node = NULL;	
			}
			
			return 0;
		}
	}

	return -1;
}

void conference_transmit_model_init( void )
{
	INIT_LIST_HEAD( &gconference_model_guard.list );

	// ��ʼ���۲���
	init_observer( &gconference_trans_observer, trans_model_unit_update );
	// ����۲��ߵ����۲���
	attach_observer( &gconnector_subjector, &gconference_trans_observer );
}

