#include "uart.h"
#include "pelco_d.h"
#include "en485_send.h"
#include <pthread.h>

// test for camera 
uint8_t gcamera_levelspeed = 25;
uint8_t gcamera_vertspeed = 25;
uint8_t gcamera_addr = 1;
uint8_t gsend_buf[100] = {0};

void camera_test_form(uint16_t d_cmd)
{
	memset( &gsend_buf, 0, sizeof(gsend_buf));
	gsend_buf[0] = CAMERA_SYNC_BYTE_HEAD;
	gsend_buf[1] = gcamera_addr;
	gsend_buf[2] = (uint8_t)(d_cmd&0x00ff);
	gsend_buf[3] = (d_cmd >> 8)&0x00ff;
	gsend_buf[4] = gcamera_levelspeed;
	gsend_buf[5] = gcamera_vertspeed;
	gsend_buf[6] = (uint8_t)CHECK_DIGIT_RESULT( gcamera_addr, (uint8_t)(d_cmd&0x00ff), (d_cmd >> 8)&0x00ff, gcamera_levelspeed, gcamera_vertspeed );

	DEBUG_SEND( &gsend_buf, CAMERA_PELCO_D_DEAL_LEN, "test Cam Data");
}

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


int main( int argc, char**argv )  // 修改过的，用于向串口发送数据或接收串口数据
{  
	int fd = -1;//文件描述符  
	int err;//返回调用函数的状态  
	//int len;                          
	//int i;  
	//uint8_t rcv_buf[UART_BUF_SIZE]; // yasir change    
	//uint8_t send_buf[100] = { 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f, 0xab, 0x04, 0x00, 0x80, 0x00, 0x2f }; // yasir change       

	if(argc != 4)  
	{  
		printf("Usage:%s /dev/ttySn (camera num) (en 485 send 0/1)\n",argv[0]);  
		return FALSE;  
	}

	gcamera_addr = atoi(argv[2]);

	if( atoi(argv[3]) )
	{
		while( !en485_send_init() )
		{
			DEBUG_MSGINFO("eable 485 Pin failed.");  
		}
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

	pthread_t uart_recv_thread;// 矩阵接收处理线程
	DEBUG_INFO( "uart file %s init Success:Will create recv phtread!",argv[1] );
	if( pthread_create( &uart_recv_thread, NULL, uart_thread_handle, (void*)&fd ) )
	{
		DEBUG_INFO( "create uart handle thread is failed!Can't recv matrix message!" );
		exit(1);
	}

 	while( 1 )	//循环发送数据
	{    
		camera_test_form(CAMERA_CTRL_RIGHT);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_LEFT);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_UP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_DOWN);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_ZOOM_TELE);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_ZOOM_WIDE);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_FOCUCS_FAR);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_FOCUCS_NEAR);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_IRIS_OPEN);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		
		camera_test_form(CAMERA_CTRL_IRIS_CLOSE);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
		sleep(2);
		camera_test_form(CAMERA_CTRL_STOP);
		UART_File_Send( fd, gsend_buf, CAMERA_PELCO_D_DEAL_LEN );
	}
		
	UART_File_Close( fd );
	en485_send_mod_cleanup();
	
	pthread_join( uart_recv_thread, NULL );
	pthread_exit( NULL );
}  



