#include "system.h"
#include "host_controller_debug.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "terminal_pro.h"
#include "udp_client_controller_machine.h"
#include "message_queue.h"
#include "profile_system.h"
#include "send_pthread.h"
#include "send_work_queue.h"
#include "stream_descriptor.h"
#include "terminal_system.h"
#include "profile_system.h"
#include "camera_pro.h"
#include "camera_common.h"
#include "muticast_connector.h"
#include "check_timer.h"

void init_system( void )
{
	endpoint_list = init_endpoint_dblist( &endpoint_list );
	assert( endpoint_list );
	command_send_guard = init_inflight_dblist( &command_send_guard );
	assert( command_send_guard );
	init_descptor_dblist( &descptor_guard );
	assert( descptor_guard );

	acmp_endstation_init( command_send_guard, endpoint_list, descptor_guard );
	aecp_controller_init(endpoint_list, descptor_guard,command_send_guard );
	DEBUG_ONINFO("[ SUCCESS: ENDPOINT AND INFLIGHT list init. ]");

	init_profile_system_file();
	init_terminal_system_state();// 初始化系统状态
	init_terminal_proccess_system();
	init_func_command_work_queue();
	init_sem_wait_can();
	upper_computer_common_init();
	
	connect_table_info_init();/*初始化连接表*/ 
	if( -1 == muticast_connector_init())// 初始广播表
	{
		DEBUG_INFO( "init mitucast conventionr list failed" );
		exit( -1 );
	}
	
	init_sem_tx_can();
	init_network_send_queue();
	send_interval_init();// 发送间隔

	camera_common_control_init(); // 串口初始化
	camera_pro_init();// 初始化预置点配置文件

	DEBUG_INFO( "quue node size = %d ", sizeof(queue_node) );
	DEBUG_INFO( "quue size = %d ", sizeof(queue) );
	
#ifdef __DEBUG__
#ifdef __TEST_QUEUE__
	fcqueue_data_elem queue_data_elem;
	memset( &queue_data_elem, 0, sizeof(fcqueue_data_elem));
	
	pthread_mutex_lock( &fcwork_queue.control.mutex );
	
	int i = 0;
	for( i = 0; i < 100; i++ ) // 目前测试的最大数是100000
	{
		queue_data_elem.func_msg_head.func_index = i;
		queue_data_elem.func_msg_head.func_cmd = i + 2;
		func_command_work_queue_messag_save( &queue_data_elem, &fcwork_queue );
	}
		
	pthread_mutex_unlock( &fcwork_queue.control.mutex );
#endif
#endif

#ifdef __DEBUG__
#ifdef __TEST_SEND_QUEUE__

	pthread_mutex_lock( &net_send_queue.control.mutex );

	tx_data tnt;
	memset( &tnt, 0, sizeof(tx_data));
	int i = 0;
	for(  i = 0; i < 100000; i++ )// 目前测试的最大数(同时存在)是100000,就frame总大小200M;在虚拟机上的测试，内存够大，无问题
	{
		uint8_t *tran_buf = NULL;
		tran_buf = allot_heap_space( TRANSMIT_DATA_BUFFER_SIZE, &tran_buf );
		if( tran_buf == NULL )
		{
			DEBUG_INFO( "Err allot_heap_space!" );
			exit(1);
		}
		tnt.frame_len = i;
		tnt.frame = tran_buf;
		send_work_queue_message_save( &tnt );
	}
	
	pthread_mutex_unlock( &net_send_queue.control.mutex );
#endif
#endif
}

void set_system_information( struct fds net_fd, struct udp_context* p_udp_net )
{
	assert( p_udp_net );
	struct jdksavdecc_eui64 zero;
	bzero( &zero, sizeof(struct jdksavdecc_eui64));

	init_udp_client_controller_endstation( net_fd.udp_server_fd,  &p_udp_net->udp_srv.sock_addr );
	
	// found all endpoints
	adp_entity_avail( zero, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER );
	sleep(2);
	
	/*获取系统的终端连接信息*/ 
	connect_table_get_information( descptor_guard );
	background_read_descriptor_input_output_stream();
	sleep(2);
	
	/* 设置连接表*/
	connect_table_info_set( descptor_guard, true );
	sleep(1);
	
	// 设置广播连接表的信息
	muticast_connector_connect_table_set( descptor_guard );
	
	// 注册会议终端, 维持5s
}

void system_close( struct threads_info *p_threads )
{	
	int can_num = p_threads->pthread_nums;
	int i = 0, ret;

	// 退出线程
	sem_post( &sem_waiting );
	for( i = 0; i < can_num; i++ )
	{
		ret = pthread_kill( p_threads->tid[i], SIGQUIT );
		if( ret != 0 )
		{
			if( errno == ESRCH )
			{
				DEBUG_INFO( "An invalid signal was specified: tid[%d] ", i );
			}
			else if( errno == EINVAL )
			{
				DEBUG_INFO( "no such tid[%d] thread to quit ", i );
			}
		}
	}

	// 退出检查定时器线程
#if 0
	check_timer_destroy();
#endif

	// 释放所有系统链表
	destroy_endpoint_dblist( endpoint_list );
	destroy_inflight_dblist( command_send_guard );
	destroy_descptor_dblist( descptor_guard );
	terminal_system_dblist_destroy();
	
	// 释放系统队列资源
	destroy_func_command_work_queue();
#if 1
	destroy_network_send_work_queue();
#endif

	// 释放连接表资源
	connect_table_destroy();
	// 释放广播表资源
	muticast_connector_destroy();

	// 保存配置文件的信息
	if( -1 == profile_system_file_write_gb_param( profile_file_fd, &gset_sys ) )
	{
		DEBUG_INFO( "write system profile Err! " );
	}

	camera_pro_system_close();// 摄像头相关的资源释放
	camera_common_control_destroy(); // 串口资源释放

	// 释放所有文件描述符
	close( net_fd.raw_fd );
	close( net_fd.tx_pipe[PIPE_RD] );
	close( net_fd.tx_pipe[PIPE_WR]);
	close( net_fd.udp_client_fd );
	close( net_fd.udp_server_fd );
	Fclose( profile_file_fd );
}


