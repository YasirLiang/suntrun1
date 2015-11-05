#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"
#include "send_pthread.h"

struct fds net_fd;					// ����ͨ���׽������̼߳�ͨ���׽���
struct raw_context net;				// ԭʼ�׽���
struct udp_server pc_controller_server;	// ������ʾ��Ϣ������ͷ��������ͨ����Ϣ 
solid_pdblist endpoint_list;			// ϵͳ���ն������ڱ��ڵ�
inflight_plist command_send_guard;	// ϵͳ�з��������������������ڱ��ڵ�
desc_pdblist descptor_guard;			// ϵͳ�������������ڱ��ڵ�

int main( int argc, char *argv[] )
{
	struct udp_context udp_net;

	init_system();	// ��ʼ��ϵͳ������ϵͳ�����ն�����,inflight��������
	
	build_socket( &net_fd, &net, NETWORT_INTERFACE, &udp_net ); // ����raw, udp server, udp client socket
	build_pipe( net_fd.tx_pipe );	// ���������ܵ�

	pthread_t h_thread;
	pthread_handle_create( &h_thread, &net_fd ); // �����������ݴ����߳�

	pthread_t p_thread;
	pthread_handle_pipe( &p_thread, &net_fd ); // �������ܵ��뷢�����������߳�

	pthread_t f_thread;	// �����������
	pthread_handle_cmd_func( &f_thread, proccess_func_link_tables );

	pthread_t s_thread; // �����������ݵ��߳�
	pthread_send_network_create( &s_thread );

	// �������߳�����߳�
	pthread_detach( h_thread );
	pthread_detach( p_thread );
	pthread_detach( f_thread );
	pthread_detach( s_thread );

	DEBUG_ONINFO("waiting for endpoint for connect!");
	set_system_information( net_fd, &udp_net );
	
	controller_proccess();
	
	pthread_exit( NULL );
	
	return 0;
}

