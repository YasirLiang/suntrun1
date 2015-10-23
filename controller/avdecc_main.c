#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"

struct fds net_fd;					// 网络通信套接字与线程间通信套接字
struct raw_context net;				// 原始套接字
struct udp_server pc_controller_server;	// 主机显示信息与摄像头控制器的通信信息 
solid_pdblist endpoint_list;			// 系统中终端链表哨兵节点
inflight_plist command_send_guard;	// 系统中发送网络数据命令链表哨兵节点
desc_pdblist descptor_guard;			// 系统中描述符链表哨兵节点

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

	// 分离主线程与次线程
	pthread_detach( h_thread );
	pthread_detach( p_thread );

	DEBUG_ONINFO("waiting for endpoint for connect!");
	set_system_information( net_fd, &udp_net );
	
	controller_proccess();

	// 主线程退出
	pthread_exit( NULL );
	
	return 0;
}

