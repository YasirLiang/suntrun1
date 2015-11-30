#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"
#include "send_pthread.h"

struct fds net_fd;					// 网络通信套接字与线程间通信套接字
struct raw_context net;				// 原始套接字
struct udp_server pc_controller_server;	// 主机显示信息与摄像头控制器的通信信息 
solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
inflight_plist command_send_guard;	// 系统中发送网络数据命令链表哨兵节点
desc_pdblist descptor_guard;	// 系统中描述符链表哨兵节点

#ifdef __TEST_DEBUG_CM__ 
int thread_test_fn( void*pgm )
{
	uint64_t tar_id = 0;
	
	while( 1 )
	{
		sleep(1);
		terminal_set_mic_status( 1, 0, tar_id );
		sleep(1);
		terminal_set_mic_status( 0, 0, tar_id );
	}
}
#endif

int main( int argc, char *argv[] )
{
	struct udp_context udp_net;

	init_system();	// 初始化系统，包括系统管理终端链表,inflight命令链表
	
	build_socket( &net_fd, &net, NETWORT_INTERFACE, &udp_net ); // 建立raw, udp server, udp client socket
	build_pipe( net_fd.tx_pipe );	// 创建无名管道

	pthread_t h_thread;
	pthread_handle_create( &h_thread, &net_fd ); // 创建接收数据处理线程

	pthread_t p_thread;
	pthread_handle_pipe( &p_thread, &net_fd ); // 创建读管道与发送网络数据线程

	pthread_t f_thread;	// 接收命令处理函数
	pthread_handle_cmd_func( &f_thread, proccess_func_link_tables );

	pthread_t s_thread; // 发送网络数据的线程
	pthread_send_network_create( &s_thread );
	
#ifdef __TEST_DEBUG_CM__
	pthread_t test_cm_thread; // 测试线程
	int rc = 0;
	rc = pthread_create( &test_cm_thread, NULL, (void*)&thread_test_fn, NULL );
	if( rc )
	{
		DEBUG_INFO(" test_cm_thread ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}
	
	pthread_detach( test_cm_thread );
#endif

	// 分离主线程与次线程
	pthread_detach( h_thread );
	pthread_detach( p_thread );
	pthread_detach( f_thread );
	pthread_detach( s_thread );

	DEBUG_ONINFO("waiting for endpoint for connect!");
	set_system_information( net_fd, &udp_net );
	
	controller_proccess();
	while(1);
	
	pthread_exit( NULL );
	
	return 0;
}

