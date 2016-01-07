#include <signal.h>
#include "avdecc_main.h"
#include "avdecc_funhdl.h"
#include "avdecc_funhdl_native.h"
#include "entity.h"
#include "system.h"
#include "controller_command.h"
#include "send_pthread.h"
#include "check_timer.h"

struct fds net_fd;					// ����ͨ���׽������̼߳�ͨ���׽���
struct raw_context net;				// ԭʼ�׽���
struct udp_server pc_controller_server;	// ������ʾ��Ϣ������ͷ��������ͨ����Ϣ 
solid_pdblist endpoint_list;			// ϵͳ���ն������ڱ��ڵ�
inflight_plist command_send_guard;	// ϵͳ�з��������������������ڱ��ڵ�
desc_pdblist descptor_guard;	// ϵͳ�������������ڱ��ڵ�
struct threads_info threads;

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

void signal_handle_main( int signum )
{
	if( SIGINT == signum )
	{
		system_close( &threads );
		exit(0);
	}
}

int main( int argc, char *argv[] )
{
	struct udp_context udp_net;
	struct sigaction sa;

	sa.sa_handler = signal_handle_main;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = 0;
	if( sigaction( SIGINT, &sa, NULL ) == -1 )// ע��ctl+c ������
	{
		perror( "sigaction: " );
		exit( -1 );
	}
	
	init_system();	// ��ʼ��ϵͳ������ϵͳ�����ն�����,inflight��������
	build_socket( &net_fd, &net, NETWORT_INTERFACE, &udp_net ); // ����raw, udp server, udp client socket
	build_pipe( net_fd.tx_pipe );	// ���������ܵ�

	threads.pthread_nums = 0;
	pthread_t h_thread;
	pthread_handle_create( &h_thread, &net_fd ); // �����������ݴ����߳�
	threads.tid[threads.pthread_nums++] = h_thread;
	pthread_detach( h_thread );
	
#if 0
	pthread_t p_thread;
	pthread_handle_pipe( &p_thread, &net_fd ); // �������ܵ��뷢�����������߳�
	threads.tid[threads.pthread_nums++] = p_thread;
	pthread_detach( p_thread );
#endif

	pthread_t f_thread;	// �����������
	pthread_handle_cmd_func( &f_thread, proccess_func_link_tables );
	threads.tid[threads.pthread_nums++] = f_thread;
	pthread_detach( f_thread );
	
#ifndef __NOT_USE_SEND_QUEUE_PTHREAD__
	pthread_t s_thread; // �����������ݵ��߳�
	pthread_send_network_create( &s_thread );
	threads.tid[threads.pthread_nums++] = s_thread;
	pthread_detach( s_thread );
#endif
	
#ifdef __TEST_DEBUG_CM__
	pthread_t test_cm_thread; // �����߳�
	int rc = 0;
	rc = pthread_create( &test_cm_thread, NULL, (void*)&thread_test_fn, NULL );
	if( rc )
	{
		DEBUG_INFO(" test_cm_thread ERROR; return code from pthread_create() is %d\n", rc);
		assert( rc == 0 );
	}

	threads.tid[threads.pthread_nums++] = test_cm_thread;
	pthread_detach( test_cm_thread );
#endif

	DEBUG_ONINFO("waiting for endpoint for connect!");
	set_system_information( net_fd, &udp_net );
	
	//controller_proccess();

	pthread_exit( NULL );
}

