#include "pipe.h"

static int fd[2] = {-1, -1};

// 判断读管道是否准备好
bool check_pipe_read_ready( int pipe_read_fd )
{
	int ret = 0;
	struct pollfd event;					// 创建一个struct pollfd结构体变量，存放文件描述符、要等待发生的事件

	event.fd = pipe_read_fd; 
	event.events = POLLIN; 				//存放要等待发生的事件,这里表示文件描述符可读
	event.revents = 0;
	
	ret = poll( (struct pollfd *)&event, 1, -1 ); // 监测event，一个对象，等待5000毫秒后超时,-1为无限等待, 0是不阻塞进程
	if( ret<0 ) 						// 判断poll的返回值，负数是出错，0是设定的时间超时，整数表示等待的时间发生
	{
		DEBUG_INFO( "poll error!" );
		assert( ret >= 0 );
	}
	
	if( event.revents & POLLERR ) 			// revents是由内核记录的实际发生的事件，events是进程等待的事件
	{
		   DEBUG_INFO("Device error!");
		   assert( (event.revents & POLLERR) == 0 );
	}
	
	if( event.revents & POLLIN )
		return true;
	else
		return false;
	
}

// 创建无名管道
void build_pipe( int pipe_fd[2] )
{
	int ret = 0;

	ret = pipe( pipe_fd );
	if( ret < 0 )
	{
		DEBUG_INFO( "pipe fd not create" );
		assert( ret >= 0 );
	}

	fd[READ_PIPE_FD] = pipe_fd[READ_PIPE_FD];
	fd[WRITE_PIPE_FD] = pipe_fd[WRITE_PIPE_FD];
}

uint16_t read_pipe_tx( void *buf, uint16_t read_len )
{
	int lenght = 0 ;
	
	lenght = read( fd[READ_PIPE_FD], buf, read_len );
		
	return lenght;
}

int write_pipe_tx(const void *buf, uint16_t buf_len )
{
	int pipe_len = 0;
	
	pipe_len = write( fd[WRITE_PIPE_FD], buf,  buf_len );
	if( pipe_len != -1 )
		return pipe_len;
	else
	{
#ifndef __DEBUG__
		DEBUG_ERR("pipe write ERR:");
		assert( pipe_len != -1 );
#else
		return -1;
#endif
	}
}

