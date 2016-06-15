#include "avdecc_funhdl.h"
#include "upper_computer.h"
#include "terminal_pro.h"
#include "wait_message.h"
#include "send_common.h"
#include "send_work_queue.h"
#include "func_proccess.h"
#include "profile_system.h"
//#include "muticast_connector.h" // 注释为了使用muticast_connect_manager.h
#include "matrix_output_input.h"
#include "control_matrix_common.h"
#include "muticast_connect_manager.h"

#ifndef SEND_DOUBLE_QUEUE_EABLE // 没有定义
volatile bool is_inflight_timeout = false;
extern bool acmp_recv_resp_err;	// acmp 接收到命令但响应错误参数
#endif

static bool system_stop = false;	// 线程退出全局变量

void thread_fn_thread_stop( void )
{
	system_stop = true;
}

// 设置新的超时时间, 并开始定时器
int timer_start_interval(int timerfd)
{
	struct itimerspec itimer_new;
        struct itimerspec itimer_old;
        //unsigned long ns_per_ms = 1000000;
        unsigned long ns_per_us = 1000;
        unsigned long interval_ms = TIME_PERIOD_1_MILLISECONDS;

        memset(&itimer_new, 0, sizeof(itimer_new));
        memset(&itimer_old, 0, sizeof(itimer_old));

        itimer_new.it_interval.tv_sec = interval_ms / 1000;
        itimer_new.it_interval.tv_nsec = (interval_ms % 1000) * ns_per_us;// 1微妙
        itimer_new.it_value = itimer_new.it_interval;

	// 设置新的超时时间，并开始计时。
        return timerfd_settime(timerfd, 0, &itimer_new, &itimer_old);
}

int fn_timer_cb( struct epoll_priv*priv )
{
	int read_len = 0;
	uint64_t timer_exp_count;
	read_len = read( priv->fd, &timer_exp_count, sizeof(timer_exp_count));

	//terminal_mic_speak_limit_time_manager_event();
    	time_tick_event( endpoint_list, command_send_guard );
	profile_system_file_write_timeouts();
#if 0
	muticast_connector_time_tick();
#else
	muticast_connect_manger_timeout_event_image();
#endif
	
#ifndef SEND_DOUBLE_QUEUE_EABLE
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
#endif

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
#if 0
		if( (frame.payload[0] != 0xfe) && (frame.payload[0] != 0xfa) && (frame.payload[0] != 0xfc))
		{
			DEBUG_RECV( frame.payload, frame_len, "Begin Raw Recv Data" );
		}
#endif
		solid_pdblist list_head = endpoint_list;
	       	int rx_status = -1;
	       	bool is_notification_id_valid = false;
	        uint16_t operation_id = 0;
	       	bool is_operation_id_valid = false;

		pthread_mutex_lock(&ginflight_pro.mutex);
		rx_raw_packet_event( frame.dest_address.value, frame.src_address.value, &is_notification_id_valid, list_head, frame.payload, frame_len, &rx_status, operation_id, is_operation_id_valid );
		pthread_mutex_unlock(&ginflight_pro.mutex);
		
#ifndef SEND_DOUBLE_QUEUE_EABLE
		if( ((rx_status == 0) && is_wait_messsage_active_state()) || (acmp_recv_resp_err && is_wait_messsage_active_state()) )
		{
			int msr_status = 0;
			msr_status = set_wait_message_status( rx_status );
			assert( msr_status == 0 );
			sem_post( &sem_waiting ); 
			acmp_recv_resp_err = false;
		}
#endif
	}
	
	return 0;
}

// host controller as a server
int udp_server_fn(struct epoll_priv *priv )
{
	struct sockaddr_in sin_in;
	socklen_t sin_len = sizeof( struct sockaddr_in );	// (特别注意)调用者应该在调用之前初始化与struct sockaddr_in相关的缓冲区的大小
	struct host_upper_cmpt_frame recv_frame;
	int recv_len = 0;
	memset( &recv_frame, 0, sizeof( struct host_upper_cmpt_frame ) );
	memset( &sin_in, 0, sin_len );
	
	recv_len = recv_udp_packet( priv->fd, recv_frame.payload, sizeof( recv_frame.payload ), &sin_in, &sin_len );
	if( recv_len > 0)
	{
		// 保存接收数据到缓冲区
		upper_computer_common_recv_messsage_save( priv->fd, &sin_in, true, sin_len, recv_frame.payload, recv_len );
	}
	else
	{
		DEBUG_INFO("recv UDP packet len is zero or recv error!");
		is_upper_udp_client_connect = false;
		assert( recv_len >= 0);
	}

	return 0;
}

// host controller as a client
int udp_client_fn(struct epoll_priv *priv )
{
	return 0;
}

// matrix control recv mssage
int matrix_control_uart_recv_fn( struct epoll_priv *priv )
{
	uint16_t recv_len = 0;
	uint8_t recv_buf[2048] = {0};
	
	recv_len = read( priv->fd, recv_buf, sizeof(recv_buf));
	if( recv_len  > 0 )
	{
		// save message to ring buf
		DEBUG_INFO( " Recv matrix msg %s", recv_buf );
		control_matrix_common_recv_message_save( recv_buf, recv_len );
	}
	
	return 0;
}

#define INPUT_MSG_LEN	6
extern unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN]; 
extern volatile unsigned char gcontrol_sur_msg_len; 
extern int gcontrol_sur_fd;
//extern sem_t gsem_surface;
int control_surface_recv_fn( struct epoll_priv *priv )
{
	memset( gcontrol_sur_recv_buf, 0, INPUT_MSG_LEN );
	gcontrol_sur_msg_len = read( priv->fd, gcontrol_sur_recv_buf, INPUT_MSG_LEN );
	if( gcontrol_sur_msg_len  > 0 )
	{
#ifdef __DEBUG__
		printf("uart recv: ");
		int i;
		for(i=0; i<gcontrol_sur_msg_len; i++)
		{
			printf("0x%x ",gcontrol_sur_recv_buf[i]);
		}
		printf("\n");
#endif
		//sem_post( &gsem_surface );
	}
	else
	{
#ifdef __DEBUG__
		perror("uart recv failed:" );
		printf("\n");
#endif
	}
	
	return 0;
}

int prep_evt_desc(int fd,handler_fn fn,struct epoll_priv *priv,struct epoll_event *ev)
{
	priv->fd = fd;
	priv->fn = fn;
	ev->events = EPOLLIN;	// 可读
	ev->data.ptr = priv;		//注册处理函数
	
	return 0;
}

int thread_fn(void *pgm)
{
	struct fds *fn_fds = (struct fds *)pgm;
	int epollfd;
	struct epoll_event ev, epoll_evt[POLL_COUNT]; // struct epoll_event 定义于 sys/epoll.h
	struct epoll_priv fd_fns[POLL_COUNT];

	//创建一个epoll的句柄。
	epollfd = epoll_create( POLL_COUNT );

	//注册新的fd与处理函数到ev中
	prep_evt_desc( timerfd_create( CLOCK_MONOTONIC, 0 ), &fn_timer_cb, &fd_fns[0],  &ev );
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[0].fd, &ev );

	prep_evt_desc( fn_fds->raw_fd, &fn_netif_cb, &fd_fns[1], &ev );
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[1].fd, &ev );

	prep_evt_desc( fn_fds->udp_server_fd, &udp_server_fn, &fd_fns[2], &ev );
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[2].fd, &ev );

	prep_evt_desc( fn_fds->udp_client_fd, &udp_client_fn, &fd_fns[3], &ev );
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[3].fd, &ev );

	if( gmatrix_file_set_success)
	{// matrix uart recv module
		prep_evt_desc( gmatrix_output_file, &matrix_control_uart_recv_fn, &fd_fns[4], &ev );
		epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[4].fd, &ev );
	}
	else 
		DEBUG_INFO( "init matrix uart recv thread handle failed!" );

	if( gcontrol_sur_fd > 0 )
	{
		prep_evt_desc( gcontrol_sur_fd, &control_surface_recv_fn, &fd_fns[5], &ev );
		epoll_ctl( epollfd, EPOLL_CTL_ADD, fd_fns[5].fd, &ev );

	}
	else 
		DEBUG_INFO( "init gcontrol_sur_fd recv thread handle failed!" );

	fcntl( fd_fns[0].fd, F_SETFL, O_NONBLOCK );
	timer_start_interval( fd_fns[0].fd );

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
		
		for (i = 0; i < res; i++)
		{
			priv = (struct epoll_priv *)epoll_evt[i].data.ptr;
			if (priv->fn(priv) < 0) 
				return -1;
		}
	}
	while (1);

	return 0;
}

int pthread_handle_create( pthread_t *h_trd, struct fds *kfds )
{
	int rc;

	rc = pthread_create(h_trd, NULL, (void*)&thread_fn, kfds);
	if ( rc )
	{
		DEBUG_ERR("return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	return 0;
}

