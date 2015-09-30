#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"

struct fds net_fd;					// ����ͨ���׽������̼߳�ͨ���׽���
struct udp_client upper_udp_client;		// ��λ����ͨ����Ϣ
struct raw_context net;				// ԭʼ�׽���
struct udp_server pc_controller_server;	// ������ʾ��Ϣ������ͷ��������ͨ����Ϣ 
solid_pdblist endpoint_list;			// ϵͳ���ն������ڱ��ڵ�
inflight_plist command_send_guard;	// ϵͳ�з��������������������ڱ��ڵ�
desc_pdblist descptor_guard;			// ϵͳ�������������ڱ��ڵ�


int main( int argc, char *argv[] )
{
	struct udp_context udp_net;

	// ��ʼ��ϵͳ������ϵͳ�����ն�����,inflight��������
	init_system();
	
	// ����raw, udp server, udp client socket
	build_socket( &net_fd, &net, NETWORT_INTERFACE, &udp_net );
	// ���������ܵ�
	build_pipe( net_fd.tx_pipe );

	// �����������ݴ����߳�
	pthread_t h_thread;
	pthread_handle_create( &h_thread, &net_fd );

	// �����ܵ����߳�
	pthread_t p_thread;
	pthread_handle_pipe( &p_thread, &net_fd );

	DEBUG_ONINFO("waiting for endpoint for connect!");
	get_system_information();

	controller_proccess();
	
	pthread_detach( h_thread );
	pthread_detach( p_thread );
	pthread_exit( NULL );
	
	return 0;
}

