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

//�궨��  
#define FALSE  -1  
#define TRUE   0

#define UART_BUF_SIZE 1024

int UART0_Send( int fd, uint8_t *send_buf, int data_len );
int UART0_Recv( int fd, uint8_t *rcv_buf, int data_len ); 
int UART0_Init( int fd, int speed, int flow_ctrl, int databits, int stopbits,int parity );
int UART0_Set( int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity );  
int UART0_Open( int fd, char* port );  
void UART0_Close(int fd);


#endif
