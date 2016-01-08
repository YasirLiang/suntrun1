/*
**2015-12-14
**muticast_connector.c
**�����㲥����ش����ļ�
*/

#include "muticast_connector.h"
#include "acmp_controller_machine.h"

muticastor_connect_pro gmuti_connect_pro;
conventioner_cnnt_list_node* pgconventioner_cnnt_list = NULL;// ��ָ��ָ���һ����conventioner_cnnt_list_node���ݿ顣���������������ݣ�Ҳ�������Ӧ������
bool offline_reconnect = false;
uint16_t muticast_connnect_sequence = 0;

// to send the connection command for connecting conventioner and muticastor->�Բ����ߵĽڵ�Ĵ���
int muticast_connector_connect( conventioner_cnnt_list_node* connect_node )
{
	int ret = -1;
	assert( connect_node );

	DEBUG_INFO( " host muticastor connect 0x%llx, ", connect_node->uid );

	if( (gmuti_connect_pro.muticastor.uid != 0) &&\
		(connect_node->uid != 0) && \
		(connect_node->count <= MAX_CVNT_CONNECT_NUM) )
	{
		gmuti_connect_pro.current_listener = connect_node;
		acmp_connect_muticastor_conventioner( gmuti_connect_pro.muticastor.uid, \
			gmuti_connect_pro.muticastor.tarker_index,\
			connect_node->uid, connect_node->listerner_index,\
			muticast_connector_connect_callback, connect_node, 1,\
			muticast_connnect_sequence++ );

		if( !connect_node->connect_flag )
		{
			gmuti_connect_pro.muticastor.connect_num++;
			connect_node->connect_flag = true;
		}

		ret = 0;
	}

	return ret;
}

int muticast_connector_connect_callback( uint64_t tarker_id, conventioner_cnnt_list_node* connect_node, bool success )
{
	int ret = -1;
	assert( connect_node );

	DEBUG_INFO( "muticast connect callback: 0x%016llx-0x%016llx(tarker id: cur tarker)-->success = %d ",\
		tarker_id, gmuti_connect_pro.muticastor.uid, success );

	assert( gmuti_connect_pro.eelem_flags == CVNT_CHECK_WAIT );
	if( !success || (tarker_id == 0) || (gmuti_connect_pro.muticastor.uid == 0) )
	{
		ret = -1;
	}
	else
	{
		if( tarker_id != gmuti_connect_pro.muticastor.uid )
			ret = -1;
		else if( connect_node == gmuti_connect_pro.current_listener ) // connection success?
		{
			if( !connect_node->connect_flag )
			{
				gmuti_connect_pro.muticastor.connect_num++;
				connect_node->connect_flag = true;
			}
			
			connect_node->state = CVNT_ONLINE;
			connect_node->count = 1;
			host_timer_update( CVNT_ONLINE_TIME_OUT*1000, &connect_node->timeout );
			ret = 0;
		}
		else
		{
			ret = -1;
		}
	}

	if( ret == -1 )
	{
		DEBUG_INFO( ">>>>>>>>>>>>>call back ERR !<<<<<<<<<<<");
		if( connect_node->connect_flag )
		{
			if( gmuti_connect_pro.muticastor.connect_num > 0 )
			{
				gmuti_connect_pro.muticastor.connect_num--;
			}
			
			connect_node->connect_flag = false;
		}
		
		connect_node->state = CVNT_OFFLINE;
		connect_node->count++;
		if( connect_node->count > MAX_CVNT_CONNECT_NUM )
		{
			connect_node->state = CVNT_OUT_CONNECT;
			host_timer_update( CVNT_OUTLINE_TIME_OUT*1000, &connect_node->timeout );//  timeouts is setted 1min after timer setted for once
		}
		else
		{
			host_timer_update( CVNT_CONNECT_TIME_OUT*1000, &connect_node->timeout );//  timeouts is setted 1min after timer setted for once
		}
	}

	gmuti_connect_pro.eelem_flags = CVNT_CHECK_IDLE;// set end handle
	
	return 0;
}

// to check the connection of conventioner with muticast�������ߵĽڵ�Ĵ���
int muticast_connector_proccess_online( conventioner_cnnt_list_node* connect_node )
{
	int ret = -1;
	assert( connect_node );

	if( (gmuti_connect_pro.muticastor.uid != 0) &&\
		(connect_node->uid != 0) )
	{
		gmuti_connect_pro.current_listener = connect_node;
		acmp_update_muticastor_conventioner( gmuti_connect_pro.muticastor.uid, \
			gmuti_connect_pro.muticastor.tarker_index,\
			connect_node->uid, connect_node->listerner_index, \
			muticast_connector_proccess_online_callback, connect_node,1,\
			muticast_connnect_sequence++ );
		
		ret = 0;
	}

	return ret;
}

int muticast_connector_proccess_online_callback( uint64_t tarker_stream_id, uint64_t listern_stream_id, conventioner_cnnt_list_node* connect_node, bool success )
{
	int ret = -1;
	
	assert( connect_node );
	DEBUG_INFO( "success( %d )-current listener( 0x%016llx-0x%016llx )---- tarker_stream: listern_stream( 0x%016llx-0x%016llx ) ", \
		success, gmuti_connect_pro.current_listener->uid, connect_node->uid, \
		tarker_stream_id, listern_stream_id );
	
	assert( gmuti_connect_pro.eelem_flags == CVNT_CHECK_WAIT );
	if( success )
	{
		if( connect_node == gmuti_connect_pro.current_listener )
		{
			if( tarker_stream_id == 0 || listern_stream_id == 0 )
			{	
				ret = -1;
			}
			else if( tarker_stream_id == listern_stream_id )
			{
				if( !connect_node->connect_flag )
				{
					gmuti_connect_pro.muticastor.connect_num++; // first connect
					connect_node->connect_flag = true;
				}
			
				gmuti_connect_pro.current_listener->state = CVNT_ONLINE;
				connect_node->count = 1; // meaning connect once and connect successfully
				host_timer_update( CVNT_ONLINE_TIME_OUT*1000, &connect_node->timeout );
				ret = 0;
			}	
		}
		else
		{
			ret = -1;
		}
	}
	else
	{
		ret = -1;
	}

	if( ret == -1 )
	{
		DEBUG_INFO( ">>>>>>>>>>>>>call back ERR !<<<<<<<<<<<");
		if( connect_node->connect_flag )
		{
			if( gmuti_connect_pro.muticastor.connect_num > 0 )
			{
				gmuti_connect_pro.muticastor.connect_num--;
			}
			
			connect_node->connect_flag = false;
		}
		
		connect_node->state = CVNT_OFFLINE; // for reflash connect
		connect_node->count = 1;// for reflash connect
		host_timer_update( CVNT_CONNECT_TIME_OUT*1000, &connect_node->timeout );
	}
		
	gmuti_connect_pro.eelem_flags = CVNT_CHECK_IDLE; // set end handle

	return ret;
}

// ������ʧ�ܽڵ�Ĵ���
int muticast_connector_proccess_outline( conventioner_cnnt_list_node* connect_node )
{
	assert( connect_node );
	DEBUG_INFO( " ==================conventioner 0x%llx out of line!====================", connect_node->uid );

	if( connect_node->connect_flag )
	{
		if( gmuti_connect_pro.muticastor.connect_num > 0 )
		{
			gmuti_connect_pro.muticastor.connect_num--;
		}
		
		connect_node->connect_flag = false;
	}
	
	if( (connect_node->count > MAX_CVNT_CONNECT_NUM) && offline_reconnect )
	{
		// this proccess will make node reflash the connection
		connect_node->count = 1; // meaning not connect
		connect_node->state = CVNT_OFFLINE;
		host_timer_update( CVNT_CONNECT_TIME_OUT*1000, &connect_node->timeout );
	}
	else
	{
		host_timer_update( CVNT_OUTLINE_TIME_OUT*1000, &connect_node->timeout );
	}

	gmuti_connect_pro.eelem_flags = CVNT_CHECK_IDLE;// set end handle
	
	return 0;
}

/*************************************
*Writer:YasirLiang
*Date:2015-12-15
*Name:muticast_connector_time_tick
*Param:��
*Return value:-1 ����
*Function:ϵͳά���㲥���ӱ�
**************************************/
int muticast_connector_time_tick( void )
{
	conventioner_cnnt_list_node* p_cvnt_node = pgconventioner_cnnt_list;
	conventioner_cnnt_list_node* p_cur_node = NULL;
	uint16_t *p_current_index = NULL;
	uint16_t conventor_state;
	emuticast_node_pro muti_flags = gmuti_connect_pro.eelem_flags;
	uint16_t total_num = gmuti_connect_pro.muticastor.cvntr_total_num;

	if( (p_cvnt_node == NULL) || \
		(total_num == 0)\
		||(muti_flags != CVNT_CHECK_IDLE) )
	{
		return -1;
	}

	/**
	 * ��鵱ǰָ�����Ч�ԣ������ܳ�������ķ�Χ 
	 */
	p_current_index = &gmuti_connect_pro.muticastor.current_index;
	if( ((*p_current_index + 1) > MAX_BD_CONNECT_NUM ) ||\
		(*p_current_index > (total_num-1)) )// err?( the most *p_current_index is (total_num -1 or MAX_BD_CONNECT_NUM-1))
	{
		DEBUG_INFO( "total_num (%d)<-->(%d) current_index", total_num,*p_current_index );
		*p_current_index = 0;
		return -1;
	}

	/**
	 * ֻ������ǰ�ĳ�ʱ�ڵ�;
	 * ����ǰ�ڵ㳬ʱ��������ɺ����¿�ʼ��ʱ�����ѵ�ǰָ��ָ����һ��Ԫ��;
	 * ������ʱ���򷵻غ���
	 */
	p_cur_node = &p_cvnt_node[*p_current_index];
	if( NULL == p_cur_node )
	{
		return -1;
	}

	gmuti_connect_pro.current_listener = p_cur_node;
	if( host_timer_timeout(&(p_cur_node->timeout )) )
	{
		DEBUG_INFO( "muticast table current index = %d", *p_current_index );
		conventor_state = p_cur_node->state;
		switch( conventor_state )
		{
			case CVNT_ONLINE:
				gmuti_connect_pro.eelem_flags = CVNT_CHECK_PRIMED;
				muticast_connector_proccess_online( p_cur_node );
				gmuti_connect_pro.eelem_flags = CVNT_CHECK_WAIT;// proccess one node waiting
				break;
			case CVNT_OFFLINE:
				gmuti_connect_pro.eelem_flags = CVNT_CHECK_PRIMED;
				muticast_connector_connect( p_cur_node );
				gmuti_connect_pro.eelem_flags = CVNT_CHECK_WAIT;// proccess one node waiting
				break;
			case CVNT_OUT_CONNECT: 
				gmuti_connect_pro.eelem_flags = CVNT_CHECK_PRIMED;
				muticast_connector_proccess_outline( p_cur_node );
				break;
			default:
				DEBUG_INFO( "conventioner state is wrong:Please check the right conventioner state!" );
				break;
		}
	}

	/*move to next muticast node or begin the head*/
	if( (*p_current_index >= (total_num - 1 )) || \
		((*p_current_index + 1) >= MAX_BD_CONNECT_NUM) )// last index,make zero at the begining
	{
		*p_current_index = 0;
	}
	else
	{
		(*p_current_index)++; // �����ǽ������Լ�
	}

	return 0;
}

// ͨ��ϵͳ���������������ù㲥���ӱ�����Ϣ����Ҫ�ǰ���һ���Ĺ���(��������ʱ���ģ�δ����ȷ��(2015-12-15))���ù㲥�ߺ�����߶�Ӧ������Ϣ
bool muticast_connector_connect_table_set( desc_pdblist desc_guard )
{
	bool ret = false;
	desc_pdblist desc_node = desc_guard->next;
	if( desc_node == desc_guard )
	{
		DEBUG_INFO( "there is noting to set muticast connector table!" );
		ret = false;
	}

	assert( NULL != pgconventioner_cnnt_list );
	conventioner_cnnt_list_node* list_node =  pgconventioner_cnnt_list;
	uint16_t* p_num = &gmuti_connect_pro.muticastor.cvntr_total_num;
	for( ; desc_node != desc_guard; desc_node = desc_node->next )
	{
		/**
		 * ע��:��������ʱ���ù㲥��������ߵģ����滹�����ݸ������Ϣ���й㲥���ӱ������á�
		 * ������descͷ��Ϊ�㲥��,������������ն���Ϊ�����
		 * 1�����ù㲥��
		 * 2�����ñ��㲥��
		 */
		if( !gmuti_connect_pro.muticastor.muticastor_exsit )
		{ // set muticastor
			if( desc_node->endpoint_desc.is_entity_desc_exist && desc_node->endpoint_desc.is_entity_desc_exist &&\
				desc_node->endpoint_desc.is_output_stream_desc_exist &&\
				(desc_node->endpoint_desc.output_stream.num > 0) ) // ͨ������
			{
				gmuti_connect_pro.muticastor.uid = desc_node->endpoint_desc.entity_id;
				gmuti_connect_pro.muticastor.tarker_index = desc_node->endpoint_desc.output_stream.desc[0].descriptor_index;// �洢��һ�������
				gmuti_connect_pro.muticastor.muticastor_exsit = true;
				DEBUG_INFO( "muticastor enstation is 0x%016llx ", gmuti_connect_pro.muticastor.uid );
			}
			else
			{
				DEBUG_INFO( "set the muticast is failed! check the desc output stream is right?" );
				return false;
			}
		}
		else
		{// set connectedor
			if( desc_node->endpoint_desc.is_entity_desc_exist && desc_node->endpoint_desc.is_entity_desc_exist &&\
				desc_node->endpoint_desc.is_input_stream_desc_exist &&\
				(desc_node->endpoint_desc.input_stream.num > 0) ) // ͨ������
			{
				// ����ͨ��0��Ϣ
				if( *p_num < MAX_BD_CONNECT_NUM )
				{
					list_node[*p_num].uid = desc_node->endpoint_desc.entity_id;
					list_node[*p_num].listerner_index = desc_node->endpoint_desc.input_stream.desc[0].descriptor_index;// �洢��һ��������
					list_node[*p_num].state = CVNT_OFFLINE;
					list_node[*p_num].count = 1;
					list_node[*p_num].connect_flag = false;
					host_timer_start( 200, &(list_node[*p_num].timeout));// set 200ms time for the first time

					(*p_num)++;
					DEBUG_INFO( "list conventioner number is  %d ", *p_num );
					ret = true;	
				}
				else
				{
					DEBUG_INFO( " muticast connect table is full!can't be add to list again!" );
					ret = false;
				}
			}	
		}
	}
	
	return ret;
}

/*ע:����û�г�ʼ�����õ�tarker.����ռ�, ���󷵻�-1��������Ҫ����ִֹͣ�в��˳�*/
int muticast_connector_init( void )
{
	int i = 0;
	memset( &gmuti_connect_pro, 0, sizeof(muticastor_connect_pro));
	gmuti_connect_pro.eelem_flags = CVNT_CHECK_IDLE;// check stop
	
	pgconventioner_cnnt_list = (conventioner_cnnt_list_node*)malloc( sizeof(conventioner_cnnt_list_node) * MAX_BD_CONNECT_NUM );
	if( NULL == pgconventioner_cnnt_list )
	{
		DEBUG_INFO( "init conventioner is failed: Must to check!" );
		return -1;
	}

	for( i = 0; i < MAX_BD_CONNECT_NUM; i++ )
	{
		// ֹͣ���ж�ʱ��
		pgconventioner_cnnt_list[i].count = 1;
		pgconventioner_cnnt_list[i].listerner_index = 0;
		pgconventioner_cnnt_list[i].state = CVNT_OFFLINE;
		pgconventioner_cnnt_list[i].uid = 0;
		host_timer_stop( &(pgconventioner_cnnt_list[i].timeout) );
	}
	
	gmuti_connect_pro.current_listener = pgconventioner_cnnt_list;

	return 0;
}

void muticast_connector_destroy( void )
{
	gmuti_connect_pro.current_listener = NULL;
	if( pgconventioner_cnnt_list != NULL )
	{
		free( pgconventioner_cnnt_list );
		pgconventioner_cnnt_list = NULL;
	}
}
