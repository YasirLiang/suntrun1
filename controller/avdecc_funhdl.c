#include "avdecc_funhdl.h"
#include "upper_computer.h"

void set_UDP_parameter(struct host_upper_cmpt_frame *frame, struct sockaddr_in *sin, int len)
{
	frame->payload_len = len;
	strcpy( (char*)frame->dest_address, (char*)inet_ntoa( sin->sin_addr ) );
	frame->dest_port = ntohs( sin->sin_port );

	DEBUG_INFO("dest address = %s:%d", frame->dest_address, frame->dest_port);
}

// 设置新的超时时间, 并开始定时器
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

	// 设置新的超时时间，并开始计时。
        return timerfd_settime(timerfd, 0, &itimer_new, &itimer_old);
}

int fn_timer_cb(struct epoll_priv*priv)
{
	uint64_t timer_exp_count;
    	read(priv->fd, &timer_exp_count, sizeof(timer_exp_count));

    	time_tick_event( endpoint_list, command_send_guard);
	
    	return 0;
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
	       	bool is_notification_id_valid = false;// 这里没有任何意义，因为系统中没有实现发送数据的命令的管理机制，这里的管理机制是指管理机制主机的一个管理终端与用户交互的过程，即发送数据后有一个特定的ID来标识其命令的
	        uint16_t operation_id = 0;
	       	bool is_operation_id_valid = false;

		rx_raw_packet_event( frame.dest_address.value, frame.src_address.value, &is_notification_id_valid, list_head, frame.payload, frame_len, &rx_status, operation_id, is_operation_id_valid );
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
	memset( &recv_frame, -1, sizeof( struct host_upper_cmpt_frame ) );
	memset( &sin_in, 0, sin_len );

	recv_len = recv_udp_packet( priv->fd, recv_frame.payload, sizeof( recv_frame.payload ), &sin_in, &sin_len );
	if( recv_len > 0)
	{
		// 设置全局参数
		upper_udp_client.c_fd = priv->fd;
		memcpy( &upper_udp_client.cltaddr, &sin_in, sizeof(struct sockaddr_in) );
		upper_udp_client.cltlen = sin_len;
		set_UDP_parameter( &recv_frame, &sin_in, recv_len );

		// 处理接收的上位机发送过来的数据包
		handle_upper_computer_conference_data( &recv_frame );
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
	DEBUG_LINE();
	return 0;
}

int prep_evt_desc(int fd,handler_fn fn,struct epoll_priv *priv,struct epoll_event *ev)
{
	priv->fd = fd;
	priv->fn = fn;
	ev->events = EPOLLIN;	// 表示对应的文件描述符可以读（包括对端SOCKET正常关闭）
	ev->data.ptr = priv;		// 在处理的事件中， 这里注册了处理函数。还包含了fd
	
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
	prep_evt_desc(timerfd_create(CLOCK_MONOTONIC, 0), &fn_timer_cb, &fd_fns[0],  &ev);
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[0].fd, &ev);

	prep_evt_desc(fn_fds->raw_fd, &fn_netif_cb, &fd_fns[1], &ev);
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[1].fd, &ev);

	prep_evt_desc(fn_fds->udp_server_fd, &udp_server_fn, &fd_fns[2],
			&ev);
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[2].fd, &ev);

	prep_evt_desc(fn_fds->udp_server_fd, &udp_client_fn, &fd_fns[3],
			&ev);
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_fns[3].fd, &ev);

	fcntl(fd_fns[0].fd, F_SETFL, O_NONBLOCK);
	timer_start_interval(fd_fns[0].fd);

	//开始循环
	do
	{
		int i, res;

		struct epoll_priv *priv;
		res = epoll_wait(epollfd, epoll_evt, POLL_COUNT, -1);

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
		
		//监听到的准备好的文件描述符的数量
		for (i = 0; i < res; i++)
		{
			priv = (struct epoll_priv *)epoll_evt[i].data.ptr;//空类型强制转换为 struct epoll_priv *类型
			if (priv->fn(priv) < 0) //运行先前已经注册好的函数指针
				return -1;
		}
	}
	while (1);

	return 0;
}

//系统启动时便开始了执行
int pthread_handle_create( pthread_t *h_trd, struct fds *kfds )
{
	int rc;

	//创建接收线程
	rc = pthread_create(h_trd, NULL, (void*)&thread_fn, kfds);
	if ( rc )
	{
		DEBUG_ERR("return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	return 0;
}

