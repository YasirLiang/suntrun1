/* uart.c
**Data: 2015-11-20
**
*/

#include "uart.h"

/******************************************************************* 
*���ƣ�UART0_Open 
*���ܣ��򿪴��ڲ����ش����豸�ļ����� 
*��ڲ�����
* 		fd: �ļ�������     
* 		port: ���ں�(ttyS0,ttyS1) 
*���ڲ�����   
		��ȷ����Ϊ1�����󷵻�Ϊ0 
*******************************************************************/  
int UART_File_Open( int fd, char* port )  
{  
	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY );  
	if( FALSE == fd )  
	{  
		DEBUG_ERR( "Can't Open Serial Port:" );  
		return( FALSE );  
	} 

	//�ָ�����Ϊ����״̬                                 
	if( fcntl( fd, F_SETFL, 0 ) < 0 )  
	{  
		DEBUG_MSGINFO("fcntl failed!");  
		return( FALSE );  
	}       
	else  
	{  
		DEBUG_MSGINFO(" fcntl = %d ", fcntl( fd, F_SETFL, 0 ) );  
	} 

	//�����Ƿ�Ϊ�ն��豸      
	if( 0 == isatty(STDIN_FILENO) )  
	{  
		DEBUG_MSGINFO("standard input is not a terminal device!");  
		return( FALSE );  
	}  

	return fd;  
}

/******************************************************************* 
*���ƣ� UART0_Close 
* ���ܣ��رմ��ڲ����ش����豸�ļ����� 
* ��ڲ�����
* 			fd :�ļ�������
			port :���ں�(ttyS0,ttyS1,ttyS2) 
* ���ڲ����� void 
*******************************************************************/  
void UART_File_Close(int fd)  
{  
	close( fd );  
}  

/******************************************************************* 
* ���ƣ� UART0_Set 
* ���ܣ� ���ô�������λ��ֹͣλ��Ч��λ 
* ��ڲ�����fd: �����ļ������� 
*			 speed: �����ٶ� 
*			 flow_ctrl: ���������� 
*			 databits: ����λ ȡֵΪ 7 ����8 
*			 stopbits: ֹͣλ ȡֵΪ 1 ����2 
*			 parity: Ч������ ȡֵΪN,E,O,,S 
*���ڲ�������ȷ����Ϊ1�����󷵻�Ϊ0 
*******************************************************************/  
int UART_File_Set( int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity )  
{  
	int i;  
	int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300 };  
	int name_arr[] = { 115200, 19200, 9600, 4800, 2400, 1200, 300 };  
	struct termios options;  

	/*tcgetattr(fd,&options)�õ���fdָ��������ز������������Ǳ�����options,�ú��������Բ��������Ƿ���ȷ���ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1. 
	*/  
	if( tcgetattr( fd, &options ) != 0 )  
	{  
		perror("SetupSerial	1");      
		return(FALSE);   
	}  

	//���ô������벨���ʺ����������  
	for( i=0; i < sizeof(speed_arr)/sizeof(int); i++ )  
	{
		if( speed == name_arr[i] )  
		{               
			cfsetispeed(&options,speed_arr[i]);   
			cfsetospeed(&options,speed_arr[i]);    
		}  
	}       

	options.c_cflag |= CLOCAL; //�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���  
	options.c_cflag |= CREAD;  //�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������  

	//��������������  
	switch( flow_ctrl )  
	{  
		case 0://��ʹ��������  
			options.c_cflag &= ~CRTSCTS;  
			break;
		case 1://ʹ��Ӳ��������  
			options.c_cflag |= CRTSCTS;  
			break;  
		case 2://ʹ�����������  
			options.c_cflag |= IXON | IXOFF | IXANY;  
			break;  
	}  

	//��������λ  
	//����������־λ  
	options.c_cflag &= ~CSIZE;  
	switch(databits)  
	{    
		case 5:
			options.c_cflag |= CS5;  
			break;  
		case 6:options.c_cflag |= CS6;  
			break;  
		case 7:   
			options.c_cflag |= CS7;  
			break;  
		case 8:      
			options.c_cflag |= CS8;  
			break;    
		default:     
			DEBUG_MSGINFO( "Unsupported data size" );  
			return(FALSE);   
	}

	//����ֹͣλ   
	switch(stopbits)  
	{    
		case 1:     
			options.c_cflag &= ~CSTOPB;
			break;   
		case 2:
			options.c_cflag |= CSTOPB;
			break;  
		default:     
			DEBUG_MSGINFO( "Unsupported stop bits" );   
			return(FALSE);  
	}  

	//����У��λ  
	switch(parity)  
	{    
		case'n':
		case'N'://����żУ��λ��  
			options.c_cflag &= ~PARENB;   
			options.c_iflag &= ~INPCK;      
			break;   
		case'o':    
		case'O'://����Ϊ��У��      
			options.c_cflag |= ( PARODD|PARENB );   
			options.c_iflag |= INPCK;               
			break;   
		case'e':   
		case'E'://����ΪżУ��    
			options.c_cflag |= PARENB;         
			options.c_cflag &= ~PARODD;         
			options.c_iflag |= INPCK;        
			break;  
		case's':  
		case'S'://����Ϊ�ո�   
			options.c_cflag &= ~PARENB;  
			options.c_cflag &= ~CSTOPB;  
			break;   
		default:    
			DEBUG_MSGINFO( "Unsupported parity" );      
			return(FALSE);   
	} 

	//�޸����ģʽ��ԭʼ������� // yasirֻ�Ǵ������� 
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~( ICANON|ECHO|ECHOE|ISIG );//�Ҽӵ�  
	options.c_lflag &= ~(ISIG|ICANON);  

	//���õȴ�ʱ�����С�����ַ�  
	options.c_cc[VTIME] = 1;

	/*
	   ��ȡһ���ַ��ȴ�1*(1/10)s
	*/   
	options.c_cc[VMIN] = 1;

	/*
	   ��ȡ�ַ������ٸ���Ϊ1
	   */  
	//�����������������������ݣ����ǲ��ٶ�ȡˢ���յ������ݵ��ǲ���  
	tcflush( fd, TCIFLUSH );  

	//��������(���޸ĺ��termios�������õ������У�  
	if( tcsetattr( fd, TCSANOW, &options ) != 0 )    
	{   
		perror("com set error!\n");
		return(FALSE);   
	}  

	 return(TRUE);   
}  

/******************************************************************* 
*����:UART0_Init() 
*���ܣ����ڳ�ʼ�� 
*��ڲ�����fd:�ļ�������    
*		  speed: �����ٶ� 
*		  flow_ctrl: ���������� 
*		  databits: ����λ ȡֵΪ7����8 
*		  stopbits: ֹͣλ ȡֵΪ1����2 
*		  parity: Ч������ ȡֵΪN,E,O,,S 
* ���ڲ�������ȷ����Ϊ0�����󷵻�Ϊ-1
*********************************************************************/  
int UART_File_Init( int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity )
{  
	int err;  

	if( ( err = UART_File_Set( fd, speed, flow_ctrl, databits, stopbits, parity ) ) == FALSE )  //���ô�������֡��ʽ  
	{                                                           
		return FALSE;  
	}  
	else  
	{  
		return TRUE;  
	}  
}  

/******************************************************************* 
*���ƣ�UART0_Recv 
*���ܣ����մ������� 
*��ڲ�����
*		fd:�ļ�������     
*		rcv_buf:���մ��������ݴ���rcv_buf�������� 
*		 data_len:һ֡���ݵĳ��� 
*���ڲ�������ȷ����Ϊ1�����󷵻�Ϊ0 
*******************************************************************/  
int UART_File_Recv( int fd, uint8_t *rcv_buf, int data_len )  
{  
	int len;

	len = read( fd, rcv_buf, data_len );
	
	return len; 
} 

/******************************************************************** 
*���ƣ�UART0_Send 
*���ܣ��������� 
*��ڲ�����fd:�ļ�������     
*			send_buf:��Ŵ��ڷ������� 
*			data_len:һ֡���ݵĸ��� 
*���ڲ�������ȷ����Ϊ1�����󷵻�Ϊ0 
*******************************************************************/  
int UART_File_Send( int fd, uint8_t *send_buf, int data_len )  
{  
	int len = 0;  
	len = write( fd, send_buf, data_len );  
	if( len == data_len )  
	{  
		return len;  
	}       
	else     
	{  
		tcflush( fd,TCOFLUSH );  
		return ( FALSE );  
	}  
}

