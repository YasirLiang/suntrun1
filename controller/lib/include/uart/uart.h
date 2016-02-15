/* uart.h
**Data: 2015-11-20
**
*/

#ifndef __UART_H__
#define __UART_H__

#include<stdio.h>      /*��׼�����������*/  
#include<stdlib.h>     /*��׼�����ⶨ��*/  
#include<unistd.h>     /*Unix ��׼��������*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*�ļ����ƶ���*/  
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/  
#include<errno.h>      /*����Ŷ���*/  
#include<string.h> 
#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

// ���ڶ˿ڶ���
#define UART0 "/dev/ttySAC0"
#define UART1 "/dev/ttySAC1"
#define UART2 "/dev/ttySAC2"
#define UART3 "/dev/ttySAC3"
#define UART4 "/dev/ttySAC4"
#define UART5 "/dev/ttySAC5"
#define UART6 "/dev/ttySAC6"
#define UART7 "/dev/ttySAC7"

//�궨��  
#define FALSE  -1  
#define TRUE   0

#define UART_BUF_SIZE 1024

int UART_File_Send( int fd, uint8_t *send_buf, int data_len );
int UART_File_Recv( int fd, uint8_t *rcv_buf, int data_len ); 
int UART_File_Init( int fd, int speed, int flow_ctrl, int databits, int stopbits,int parity );
int UART_File_Set( int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity );  
int UART_File_Open( int fd, char* port );  
void UART_File_Close(int fd);


#endif
