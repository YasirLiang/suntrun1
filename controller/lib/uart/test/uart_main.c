#include "uart.h"

int main( int argc, char**argv )  // 修改过的，用于向串口发送数据或接收串口数据
{  
	int fd = -1;//文件描述符  
	int err;//返回调用函数的状态  
	int len;                          
	int i;  
	uint8_t rcv_buf[UART_BUF_SIZE]; // yasir change    
	uint8_t send_buf[100] = { 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f, 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f }; // yasir change       
	
	if(argc != 3)  
	{  
		printf("Usage:%s /dev/ttySn 0(send data)/1(receive data)\n",argv[0]);  
		return FALSE;  
	}
	
	fd = UART0_Open( fd,argv[1] );//打开串口，返回文件描述符 
	if( fd == -1 )
	{
		DEBUG_MSGINFO("Open Port Failed!");  
	}
	
	do
	{  
		err = UART0_Init( fd, 9600, 0, 8, 1, 'N' );
		DEBUG_MSGINFO("Set Port Exactly!");  
	}while( FALSE == err||FALSE == fd );  

	if( 0== strcmp( argv[2], "0") )  
	{  
		for( i=0; i<10; i++ )  
		{  
			len	= UART0_Send(fd,send_buf,12);  
			if(len>0)
				printf("%d send data successful\n",i);  
			else  
				printf("send data failed!\n");

			sleep(2);  
		}  

		UART0_Close(fd);               
	}  
	else  
	{  

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
			if( fs_sel )  
			{  
				len = UART0_Recv( fd, rcv_buf, UART_BUF_SIZE );
				if(len>0)  
				{
#if 0
					int i = 0;
					printf(" len = %d Secial Data :\n\t",len);  
					for( ; i < len; i++)
						printf("%02x ", rcv_buf[i] );
					printf("\n");
#endif
				}
			}  
			else  
			{  
				DEBUG_MSGINFO( "Select Recv Message TimeOut!");
				continue;
			}
		}
		
		UART0_Close( fd ); 
	}              
	
	return 0;
}  



