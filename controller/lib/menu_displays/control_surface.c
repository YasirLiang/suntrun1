#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h> 
#include <unistd.h> 
#include <time.h>
#include <semaphore.h>

#include "lcd192x64.h"
#include "menu_f.h"
#include "uart.h"
#include "wireless.h"

/*input message len of Macro------------------------------------------------*/
#define INPUT_MSG_LEN	6
/*command's index in the input message's Macro------------------------------*/
#define INPUT_MSG_CMD_INDEX	3

/*Global Objects------------------------------------------------------------*/
unsigned char gcontrol_sur_recv_buf[INPUT_MSG_LEN];               /*recv buf*/
volatile unsigned char gcontrol_sur_msg_len = 0;          /*recv message len*/
int gcontrol_sur_fd = -1;                      /*fd of control data recieved*/
sem_t gsem_surface;                           /*sem of surface control*/

/*$CrcChk...................................................................*/
unsigned char CrcChk(unsigned char *ccp) {
    unsigned char aa;
    unsigned char bb;
    aa=*ccp;

    for (bb=0; bb<4; bb++) {
        ccp++;
        aa+=*ccp;
    }
    aa=~aa;
    return(aa);
}
/*$knob_pross...............................................................*/
void knob_pross(unsigned char cmd) {
    if (cmd&1) {
        MenuScroll(gsnCurMGrp, 1);
    }
    else if (cmd&(1<<1)) {
        MenuScroll(gsnCurMGrp, -1);
    }

    if (cmd&(1<<2)) {
        ItemSelected(gsnCurMGrp);
    }
}

/****************************************************************************/
/**
* @description
* This function used for menu display is writed by HuangYuqing
* @param[in] p_buf pointer control data buf
*                    recv_len control data len 
*/
void input_recv_pro(unsigned char *p_buf, unsigned recv_len) {
    if (recv_len<INPUT_MSG_LEN) {
        return;
    }
    if (p_buf[5] == CrcChk(p_buf)) {
        if (p_buf[INPUT_MSG_CMD_INDEX] == 0xAA) {
            knob_pross(p_buf[INPUT_MSG_CMD_INDEX+1]);
        }
        else {
            wireless_pross(p_buf);
        }
    }
}

/****************************************************************************/
/**
* @description
* This function used for menu display is writed by HuangYuqing
* But it's not used in this system because of there another way to control.
* @param[in] arg pthread args
*/
void *thread_control_surface(void *arg) {
    int ret;
    int fd = -1;
    int err;

    ret = lcd192x64_init();/*init surface port*/
    if (ret) {
        printf("lcd192x64_init fail\n");
        exit(1);
    }

    MenuInit(); /*init menu*/

    fd = UART_File_Open(fd,UART4);/*control port is uart4*/
    if (fd == -1) {
        printf("Open Port Failed!\n");
    }

    do {  
        err = UART_File_Init(fd, 9600, 0, 8, 1, 'N');
        printf("Set Port Exactly!\n");
    }while( FALSE == err||FALSE == fd );

    gcontrol_sur_fd = fd;
    sem_init(&gsem_surface, 0, 0);

    while(1) {
        sem_wait(&gsem_surface);
        input_recv_pro(gcontrol_sur_recv_buf, gcontrol_sur_msg_len);
    }
}

