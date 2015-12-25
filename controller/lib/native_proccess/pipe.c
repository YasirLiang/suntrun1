#include "pipe.h"

static int fd[2] = {-1, -1};

// �ж϶��ܵ��Ƿ�׼����
bool check_pipe_read_ready( int pipe_read_fd )
{
	int ret = 0;
	struct pollfd event;					// ����һ��struct pollfd�ṹ�����������ļ���������Ҫ�ȴ��������¼�

	event.fd = pipe_read_fd; 
	event.events = POLLIN; 				//���Ҫ�ȴ��������¼�,�����ʾ�ļ��������ɶ�
	event.revents = 0;
	
	ret = poll( (struct pollfd *)&event, 1, -1 ); // ���event��һ�����󣬵ȴ�5000�����ʱ,-1Ϊ���޵ȴ�, 0�ǲ���������
	if( ret<0 ) 						// �ж�poll�ķ���ֵ�������ǳ���0���趨��ʱ�䳬ʱ��������ʾ�ȴ���ʱ�䷢��
	{
		DEBUG_INFO( "poll error!" );
		assert( ret >= 0 );
	}
	
	if( event.revents & POLLERR ) 			// revents�����ں˼�¼��ʵ�ʷ������¼���events�ǽ��̵ȴ����¼�
	{
		   DEBUG_INFO("Device error!");
		   assert( (event.revents & POLLERR) == 0 );
	}
	
	if( event.revents & POLLIN )
		return true;
	else
		return false;
	
}

// ���������ܵ�
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

