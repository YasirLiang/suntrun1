#include "system.h"
#include "host_controller_debug.h"
#include "acmp_controller_machine.h"
#include "adp_controller_machine.h"
#include "aecp_controller_machine.h"
#include "terminal_pro.h"
#include "udp_client_controller_machine.h"
#include "message_queue.h"
#include "profile_system.h"
#include "send_common.h"
#include "send_work_queue.h"
#include "stream_descriptor.h"
#include "terminal_system.h"
#include "profile_system.h"
#include "camera_pro.h"
#include "camera_common.h"
//#include "muticast_connector.h"
#include "check_timer.h"
#include "matrix_output_input.h"// 矩阵输出
#include "control_matrix_common.h"
#include "system_database.h"
#include "conference_transmit_unit.h"
#include "central_control_recieve_unit.h"
#include "connector_subject.h"
#include "avdecc_manage.h"
#include "log_machine.h"
#include "en485_send.h"

#include "lcd192x64.h"// lcd 界面显示
#include "menu_f.h"

#include "central_control_transmit_unit.h"
#include "system_1722_recv_handle.h"
#include "controller_machine.h"

extern void muticast_muticast_connect_manger_init( void );
extern int gcontrol_sur_fd;
extern sem_t gsem_surface;

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
	upper_computer_common_init();

	init_connector_subjector();// 初始化系统的被观察者
	conference_transmit_model_init();// 初始化系统会议单元传输模块
	central_control_recieve_uinit_init_list();// 初始化中央接收模块
	conference_recieve_uinit_proccess_init();// 初始化会议接收模块
	central_control_transmit_unit_model_pro_init();// 初始化中央传输单元模块处理
	
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
	connect_table_info_init();/*初始化连接表*/
#endif

	init_sem_tx_can();
	init_sem_wait_can();
	init_network_send_queue();
	system_1722_recv_handle_init();// 初始化接收处理
	send_interval_init();// 发送间隔

	inflight_proccess_init();// 初始化inflight 处理参数
	matrix_output_init();// 初始化矩阵输出端口
	control_matrix_common_init();

	camera_common_control_init(); // 串口初始化
	camera_pro_init();// 初始化预置点配置文件

	system_database_init();// 打开数据库

	avdecc_manage_init();// 初始化avdecc 管理 

	en485_send_init(); // 使能发送485端数据
	muticast_muticast_connect_manger_init();

	/*
	  * 初始化化界面控制数据串口传输
	  * 文件描述符
	  */
	int fd = -1;
	fd = UART_File_Open(fd,UART4);//打开串口，返回文件描述符 
	if( fd == -1 )
	{
		printf("Open Port Failed!\n");  
	}

	if( (UART_File_Init( fd, 9600, 0, 8, 1, 'N' ) != -1) && fd != -1 )
	{
		gcontrol_sur_fd = fd;
		//sem_init( &gsem_surface, 0, 0 );
	}

	/*
	  * 初始化化界面显示
	  *
	  */
	int ret ;
	ret = lcd192x64_init();
	if (ret)
	{
		printf("lcd192x64_init fail\n");
		exit(1);
	}

	MenuInit();

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
	//sleep(2);
	
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
	/*获取系统的终端连接信息*/ 
	connect_table_get_information( descptor_guard );
#endif

#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
	/* 设置连接表*/
	connect_table_info_set( descptor_guard, true );
#endif

	// 开始注册会议终端
	terminal_begin_register();
}

extern FILE *glog_file_fd;
extern int lcd192x64_close( void );// yasirLiang add in 2016/05/16
void system_close( struct threads_info *p_threads )
{	
	int can_num = p_threads->pthread_nums;
	int i = 0, ret;

	en485_send_mod_cleanup();// 释放使能发送485端数据文件
	lcd192x64_close();// yasirLiang add in 2016/05/16
	controller_machine_destroy(&gp_controller_machine);// 摧毁控制器
	system_1722_recv_handle_destroy();
	// 退出线程
#ifndef SEND_DOUBLE_QUEUE_EABLE
	sem_post( &sem_waiting );
#endif

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

	// 释放所有系统链表
	destroy_endpoint_dblist( endpoint_list );
	if( endpoint_list != NULL )
	{
		free( endpoint_list );
		endpoint_list = NULL;
	}

	destroy_inflight_dblist( command_send_guard );
	if( command_send_guard != NULL )
	{
		free( command_send_guard );
		command_send_guard = NULL;
	}

	destroy_descptor_dblist( descptor_guard );
	if( descptor_guard != NULL )
	{
		free( descptor_guard );
		descptor_guard = NULL;
	}

	terminal_system_dblist_destroy();
	
	// 释放系统队列资源
	destroy_func_command_work_queue();
	destroy_network_send_work_queue();
	
#ifdef ENABLE_CONNECT_TABLE// endstation_connection.h
	connect_table_destroy();// 释放连接表资源
#endif

#if 0
	muticast_connector_destroy();// 释放广播表资源
#else
#endif
	profile_system_close();// 保存配置文件的信息
	camera_pro_system_close();// 摄像头相关的资源释放
	camera_common_control_destroy(); // 串口资源释放
	matrix_control_destroy();
	// 关闭数据库
	system_database_destroy();
	// 释放系统相关的资源
	terminal_proccess_system_close();
	log_machine_destroy();
	// 释放所有文件描述符
	if( NULL != glog_file_fd)
		Fclose( glog_file_fd );
	close( net_fd.raw_fd );
	close( net_fd.tx_pipe[PIPE_RD] );
	close( net_fd.tx_pipe[PIPE_WR]);
	close( net_fd.udp_client_fd );
	close( net_fd.udp_server_fd );
}


