/* uart.h
**Data: 2015-11-20
**
*/

#ifndef __UART_H__
#define __UART_H__

#include<stdio.h>      /*标准输入输出定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<sys/types.h>   
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<termios.h>    /*PPSIX 终端控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h> 
#include "jdksavdecc_world.h"
#include "host_controller_debug.h"

//宏定义  
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
