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

// 串口端口定义
#define UART0 "/dev/ttySAC0"
#define UART1 "/dev/ttySAC1"
#define UART2 "/dev/ttySAC2"
#define UART3 "/dev/ttySAC3"
#define UART4 "/dev/ttySAC4"
#define UART5 "/dev/ttySAC5"
#define UART6 "/dev/ttySAC6"
#define UART7 "/dev/ttySAC7"

//宏定义  
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
