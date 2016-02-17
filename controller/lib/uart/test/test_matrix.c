#include <pthread.h>
#include "uart.h"

#define MATRIX_CMD_V1TO2 "1V2."
#define MATRIX_CMD_CLOSE_ALL "All$."

pthread_t uart_recv_thread;// 矩阵接收处理线程

void* uart_thread_handle( void* arg )
{
	int fd = *((int*)arg);
	int len = -1;
	uint8_t rcv_buf[UART_BUF_SIZE] = {0}; 

	while( 1 )	//循环读取数据  
	{    
		int fs_sel;
		fd_set fs_read;
		struct timeval time;

		FD_ZERO( &fs_read );
		FD_SET( fd,&fs_read );
		
		time.tv_sec = 10;  
		time.tv_usec = 0;  

		//使用select实现串口的多路通信  
		fs_sel = select( fd+1, &fs_read, NULL, NULL, &time );  
		if( fs_sel && (fs_sel != -1) )  
		{
			memset( rcv_buf, 0, UART_BUF_SIZE );
			len = UART_File_Recv( fd, rcv_buf, UART_BUF_SIZE );
			if( len > 0 )  
			{
#if 1
				int i = 0;
				printf(" len = %d Secial Data :\n\t",len);  
				for( ; i < len; i++)
					printf("%c", rcv_buf[i] );
				printf("\n");
#endif
			}
			else if( len < 0 )
			{
				DEBUG_INFO( "read uart data is Error!return to main function" );
				break;
			}
		}  
		else  
		{  
			DEBUG_MSGINFO( "Select Recv Message TimeOut!");
			continue;
		}
	}    

	pthread_exit( NULL );
}

int main( int argc, char* argv[] )
{
	int fd = -1;//文件描述符  
	int err;//返回调用函数的状态  
	int len;                          
	int i;  
	//uint8_t rcv_buf[UART_BUF_SIZE]; // yasir change    
	//uint8_t send_buf[100] = { 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f, 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f }; // yasir change       
	
	if(argc != 3)  
	{  
		printf("Usage:%s /dev/ttySAC2 0(send data)/1(receive data)\n",argv[0]);  
		return FALSE;  
	}
	
	fd = UART_File_Open( fd,argv[1] );//打开串口，返回文件描述符 
	if( fd == -1 )
	{
		DEBUG_MSGINFO("Open Port Failed!");  
	}
	
	do
	{  
		err = UART_File_Init( fd, 9600, 0, 8, 1, 'N' );
		DEBUG_MSGINFO("Set Port Exactly!");  
	}while( FALSE == err||FALSE == fd ); 

	DEBUG_INFO( "uart file %s init Success:Will create recv phtread!",argv[1] );
	if( pthread_create( &uart_recv_thread, NULL, uart_thread_handle, (void*)&fd ) )
	{
		DEBUG_INFO( "create uart handle thread is failed!Can't recv matrix message!" );
	}

	if( 0 == strcmp( argv[2], "0") )  
	{  
		for( i = 0; i < 10; i++ )  
		{
			if( i % 2 )
			{
				len = UART_File_Send( fd, (void*)MATRIX_CMD_V1TO2, 4 );  
				if( len>0 )
					printf( "%d send data successful\n",i );  
				else  
					printf( "send data failed!\n" );
			}
			else
			{
				len = UART_File_Send( fd, (void*)MATRIX_CMD_CLOSE_ALL, 5 );  
				if( len>0 )
					printf("%d send data successful\n",i);  
				else  
					printf("send data failed!\n");
			}

			sleep(2);  
		}  

		UART_File_Close( fd );
		pthread_cancel( uart_recv_thread );
	}  

	pthread_join( uart_recv_thread, NULL );

	pthread_exit( NULL );
}


