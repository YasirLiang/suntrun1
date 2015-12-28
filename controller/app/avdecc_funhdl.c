#include "avdecc_funhdl.h"
#include "upper_computer.h"
#include "terminal_pro.h"
#include "wait_message.h"
#include "send_pthread.h"
#include "send_work_queue.h"
#include "func_proccess.h"
#include "profile_system.h"
#include "muticast_connector.h"

bool is_inflight_timeout = false;
static bool system_stop = false;	// �߳��˳�ȫ�ֱ���
extern bool acmp_recv_resp_err;	// acmp ���յ������Ӧ�������
extern fcwqueue fcwork_queue;	// ����������Ϣ��������

void thread_fn_thread_stop( void )
{
	system_stop = true;
}

// �����µĳ�ʱʱ��, ����ʼ��ʱ��
int timer_start_interval(int timerfd)
{
	struct itimerspec itimer_new;
        struct itimerspec itimer_old;
        unsigned long ns_per_ms = 1000000;
        unsigned long interval_ms = TIME_PERIOD_25_MILLISECONDS;

        memset(&itimer_new, 0, sizeof(itimer_new));
        memset(&itimer_old, 0, sizeof(itimer_old));

        itimer_new.it_interval.tv_sec = interval_ms / 1000;
        itimer_new.it_interval.tv_nsec = (interval_ms % 1000) * ns_per_ms;
        itimer_new.it_value = itimer_new.it_interval;

	// �����µĳ�ʱʱ�䣬����ʼ��ʱ��
        //return timerfd_settime(timerfd, 0, &itimer_new, &itimer_old);

	return 0;
}

int fn_timer_cb( struct epoll_priv*priv )
{
	int read_len = 0;
	uint64_t timer_exp_count;
	read_len = read( priv->fd, &timer_exp_count, sizeof(timer_exp_count));

	//terminal_mic_speak_limit_time_manager_event();
    	time_tick_event( endpoint_list, command_send_guard );
	profile_system_file_write_timeouts();
	muticast_connector_time_tick();

	if( is_inflight_timeout && is_wait_messsage_active_state() )
	{
		set_wait_message_status( WAIT_TIMEOUT );	
		sem_post( &sem_waiting );
	}
	
	is_inflight_timeout = false; 

	if((is_wait_messsage_active_state()) && (is_send_interval_timer_timeout()))// check uart or resp data timeout
	{
		sem_post( &sem_waiting ); 
	}
	
    	return read_len;
}

int fn_netif_cb( struct epoll_priv *priv )
{
	int fd = priv->fd;
	ssize_t status = -1;
	uint64_t dest_addr = 0;
	uint64_t default_native_dest = 0;
	struct jdksavdecc_frame frame;
	
	status = (ssize_t)conference_host_raw_receive( fd, &frame.ethertype, frame.src_address.value, \
									frame.dest_address.value, frame.payload, sizeof(frame.payload) );
	uint16_t frame_len = ( uint16_t )status;
	frame.length = frame_len;
	
	convert_eui48_to_uint64( frame.dest_address.value, &dest_addr );
	convert_eui48_to_uint64( jdksavdecc_multicast_adp_acmp.value, &default_native_dest );
	if( (status > 0) && ( frame.ethertype == JDKSAVDECC_AVTP_ETHERTYPE ) )
	{
		solid_pdblist list_head = endpoint_list;
	       	int rx_status = -1;
	       	bool is_notification_id_valid = false;// ����û���κ����壬��Ϊϵͳ��û��ʵ�ַ������ݵ�����Ĺ�����ƣ�����Ĺ��������ָ�������������һ�������ն����û������Ĺ��̣����������ݺ���һ���ض���ID����ʶ�������
	        uint16_t operation_id = 0;
	       	bool is_operation_id_valid = false;

		rx_raw_packet_event( frame.dest_address.value, frame.src_address.value, &is_notification_id_valid, list_head, frame.payload, frame_len, &rx_status, operation_id, is_operation_id_valid );

		if( ((rx_status == 0) && is_wait_messsage_active_state()) || (acmp_recv_resp_err && is_wait_messsage_active_state()) )
		{
			int msr_status = 0;
			msr_status = set_wait_message_status( rx_status );
			assert( msr_status == 0 );
			sem_post( &sem_waiting ); 
			acmp_recv_resp_err = false;
		}
	}
	
	return 0;
}

// host controller as a server
int udp_server_fn(struct epoll_priv *priv )
{
	struct sockaddr_in sin_in;
	socklen_t sin_len = sizeof( struct sockaddr_in );	// (�ر�ע��)������Ӧ���ڵ���֮ǰ��ʼ����struct sockaddr_in��صĻ������Ĵ�С
	struct host_upper_cmpt_frame recv_frame;
	int recv_len = 0;
	memset( &recv_frame, 0, sizeof( struct host_upper_cmpt_frame ) );
	memset( &sin_in, 0, sin_len );
	
	recv_len = recv_udp_packet( priv->fd, recv_frame.payload, sizeof( recv_frame.payload ), &sin_in, &sin_len );
	if( recv_len > 0)
	{
		// ����ȫ�ֲ���
		upper_udp_client.c_fd = priv->fd;
		memcpy( &upper_udp_client.cltaddr, &sin_in, sizeof(struct sockaddr_in) );
		upper_udp_client.cltlen = sin_len;
		is_upper_udp_client_connect = true;
		recv_frame.payload_len = recv_len;
		int rx_status = -1;

		// ������յ���λ�����͹��������ݰ�
		handle_upper_computer_conference_data( &recv_frame, &rx_status );
				
		if( rx_status && is_wait_messsage_active_state() )
		{
			set_wait_message_status( 0 );
			sem_post( &sem_waiting );
		}
	}
	else
	{
		DEBUG_INFO("recv UDP packet len is zero or recv error!");
		assert( recv_len >= 0);
	}

	return 0;
}

// host controller as a client
int udp_client_fn(struct epoll_priv *priv )
{
	return 0;
}

int prep_evt_desc(int fd,handler_fn fn,struct epoll_priv *priv,struct epoll_event *ev)
{
	priv->fd = fd;
	priv->fn = fn;
	ev->events = EPOLLIN;	// ��ʾ��Ӧ���ļ����������Զ��������Զ�SOCKET�����رգ�
	ev->data.ptr = priv;		// �ڴ�����¼��У� ����ע���˴���������������fd
	
	return 0;
}

int thread_fn(void *pgm)
{
	struct fds *fn_fds = (struct fds *)pgm;

	int epollfd;
	struct epoll_event ev, epoll_evt[POLL_COUNT]; // struct epoll_event ������ sys/epoll.h
	struct epoll_priv fd_fns[POLL_COUNT];

	//����һ��epoll�ľ����
	epollfd = epoll_create( POLL_COUNT );

	//ע���µ�fd�봦������ev��
	//prep_evt_desc( timerfd_create( CLOCK_MONOTONIC, 0 ), &fn_timer_cb, &fd_fns[0],  &ev );
	//epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[0].fd, &ev );

	prep_evt_desc( fn_fds->raw_fd, &fn_netif_cb, &fd_fns[0], &ev );
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[0].fd, &ev );

	prep_evt_desc( fn_fds->udp_server_fd, &udp_server_fn, &fd_fns[1],&ev );
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[1].fd, &ev );

	//prep_evt_desc( fn_fds->udp_client_fd, &udp_client_fn, &fd_fns[1], &ev );
	//epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[2].fd, &ev );

	//fcntl( fd_fns[0].fd, F_SETFL, O_NONBLOCK );
	//timer_start_interval( fd_fns[0].fd );

	//��ʼѭ��
	do
	{
		int i, res;

		struct epoll_priv *priv;
		res = epoll_wait( epollfd, epoll_evt, POLL_COUNT, -1 );

		/* exit on error */
		if (-1 == res)
		{
			if (errno == EINTR)  // EINTR meaning by Interrupted system call
         		{  
             			continue;  
          		}
			
			DEBUG_ERR("epoll_wait error : ");
			return -errno;
		}
 
		if( system_stop ) //  stop the thread?
		{
			DEBUG_INFO( "thread_fn return success......" );
			return 0;
		}
		
		//��������׼���õ��ļ�������������
		for (i = 0; i < res; i++)
		{
			priv = (struct epoll_priv *)epoll_evt[i].data.ptr;//������ǿ��ת��Ϊ struct epoll_priv *����
			if (priv->fn(priv) < 0) //������ǰ�Ѿ�ע��õĺ���ָ��
				return -1;
		}
	}
	while (1);

	return 0;
}

//ϵͳ����ʱ�㿪ʼ��ִ��
int pthread_handle_create( pthread_t *h_trd, struct fds *kfds )
{
	int rc;

	//���������߳�
	rc = pthread_create(h_trd, NULL, (void*)&thread_fn, kfds);
	if ( rc )
	{
		DEBUG_ERR("return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	return 0;
}

