/********************************************************************************
* �ļ�����    lcd192x64.c
* �����ˣ�    SUNTRON Ф����
* �汾�ţ�    V1.0
* ����ʱ�䣺  2013-01-31
* ˵����      ��������ACS41xxM-ACS42xxMҺ����ʾ�������,Һ���ͺ�Ϊ��HG1926417G
******************************************************************************
* 
*
* Copyright (c) 2012-2015 GuangZhou SUNTRON electronics co., ltd. All rights reserved.
*********************************************************************************
* �޸�
*
* ����			�汾			����˵��
*
*
********************************************************************************/ 
//===============================================================================

/********************************************************************************
  * ����ͷ�ļ�
********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <sys/ioctl.h>

#include "matrix.h"
#include "lcd192x64.h"


//===============================================================================

/********************************************************************************
  * ȫ�ֱ���
********************************************************************************/
TDispMem gstDispMem;


int g_lcd_gpio_fd;

// ============================================================================

/******************************************************************************
    * ��������lcd_wr_cmd
    * ��ڲ�����ucCmd Ҫд������
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcdд����
******************************************************************************/
void lcd_wr_cmd(unsigned char ucCmd) 
{ 
	int i;
 
    // д����˿�״̬�л�
    WR_CMD(g_lcd_gpio_fd);
    
    for(i=0;i<8;i++)
    {
        CLOCK_DOWN(g_lcd_gpio_fd);
        if(ucCmd&0x80) 
            SEND_DAT_1(g_lcd_gpio_fd);
        else
            SEND_DAT_0(g_lcd_gpio_fd);

        CLOCK_UP(g_lcd_gpio_fd);
        ucCmd<<=1;
    }
    
    WR_RD_OK(g_lcd_gpio_fd);
} 
// ============================================================================

/******************************************************************************
    * ��������lcd_wr_dat
    * ��ڲ�����dat
    * ���ڲ�������
    * ����ֵ����
    * ˵����LCDд����
******************************************************************************/
void lcd_wr_dat(unsigned char ucDat) 
{ 
    int i;
    
    // д���ݶ˿�״̬�л�
    WR_DATA(g_lcd_gpio_fd);
    
    for(i=0;i<8;i++)
    {
        CLOCK_DOWN(g_lcd_gpio_fd);
        if(ucDat&0x80) 
            SEND_DAT_1(g_lcd_gpio_fd);
        else
            SEND_DAT_0(g_lcd_gpio_fd);

        CLOCK_UP(g_lcd_gpio_fd);
        ucDat<<=1;
    }
    
    WR_RD_OK(g_lcd_gpio_fd);
} 
// ============================================================================


// ============================================================================


// ============================================================================

/******************************************************************************
    * ��������lcd_clr
    * ��ڲ�����ucSele ���������0 ����� 1 �Ұ��� 2 ȫ��
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd��ʾ����
******************************************************************************/
void lcd_clr(unsigned char ucSele)
{
	unsigned char i, j;

	if(ucSele == 0)		
        LCD_SELE_LEFT(g_lcd_gpio_fd)
	else if(ucSele == 1)	
        LCD_SELE_RIGHT(g_lcd_gpio_fd)
	else if(ucSele == 2)	
        LCD_SELE_ALL(g_lcd_gpio_fd)
    
	for(j = 0; j < 8; j++)
	{
		lcd_wr_cmd(DISP_PAGE_SET + j);
		lcd_wr_cmd(DISP_COL_H_SET);
		lcd_wr_cmd(DISP_COL_L_SET);
        
		for(i = 0; i < 96; i++) 
            lcd_wr_dat(0x00);
	}
}
// ============================================================================

/******************************************************************************
    * ��������lcd_init
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵����LCD��ʼ��
******************************************************************************/
void lcd_init(void) 
{ 
    LCD_SET_RST(g_lcd_gpio_fd,0);
    usleep(100*300);	
    LCD_SET_RST(g_lcd_gpio_fd,1);
    usleep(250*300); 

    LCD_SELE_ALL(g_lcd_gpio_fd);
    lcd_wr_cmd(0xE2); 
    usleep(100*300); 

    lcd_wr_cmd(0xAE); // �ر���ʾ

    lcd_wr_cmd(0xA0); // ����DDRAM��ַSEG�����Ӧ(ADC = 0)����SEG127 SEG0

    lcd_wr_cmd(0xC8); // ����COM���ɨ�跽��(SHL = 1)COM63-COM0 

    lcd_wr_cmd(0x40); // DDRAM��ʾ��ʼ�е�ַ(0) 

    lcd_wr_cmd(0xA6); // ������ʾģʽΪ����ģʽ 

    lcd_wr_cmd(0xA4); // ���õ���ʾģʽΪ����ģʽ 

    lcd_wr_cmd(0xA2); // ����Һ����ʾ������ѹƫ���(1/9)

    lcd_wr_cmd(0x2F); // ����ѹ��·����ѹ���ڵ�·�͵�ѹ�����· 
    
    lcd_wr_cmd(0x24); // ѡ���ڲ�������(Rb / Ra) 

    lcd_wr_cmd(0x81); // ���öԱȶ�
    lcd_wr_cmd(0x20); 
    
    lcd_wr_cmd(0xAF); // ����ʾ 
	
	lcd_clr(2);
}   
// ============================================================================


/******************************************************************************
    * ��������lcd_byte_pos
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd�ֽڶ�λ
******************************************************************************/
void lcd_byte_pos(unsigned char x, unsigned char y)
{
	if(x < 96)//�����	   
        LCD_SELE_LEFT(g_lcd_gpio_fd)		
	else if(x < 192) 
    {//�Ұ���
        x -= 96; 
        LCD_SELE_RIGHT(g_lcd_gpio_fd)
    }
    
//     y <<= 3;
	lcd_wr_cmd(DISP_PAGE_SET + y);//ҳ��ַ0-7
	lcd_wr_cmd(DISP_COL_H_SET | (x>>4));//�е�ַ0-63
	lcd_wr_cmd(DISP_COL_L_SET | (x&0x0F));//�е�ַ0-63
}
// ============================================================================

/******************************************************************************
    * ��������clear_block
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵������տ�
******************************************************************************/
void clear_block(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
    unsigned char i,j,ucMask=0;

	if(y1/8 == y2/8)	//��������ݴ�ֱ������һ���ֽ���
	{
		for(i=y1%8;i<(y2)%8;i++)
			ucMask |= (1<<i);
		ucMask = ~ucMask;
        for(j = x1;j<=x2;j++)//��ѭ��
        	gstDispMem.ucDispBuf[y1/8][j] &= ucMask;
	}
	else
	{
		if(y1%8)
		{
			ucMask = (0x01<<(y1%8))-1;
			for(j = x1;j<=x2;j++)//��ѭ��
				gstDispMem.ucDispBuf[y1/8][j] &= ucMask;
		}
		for(i=y1/8+(y1%8 ? 1:0);i<y2/8+(((y2+1)%8==0) ? 1:0);i++)
		{
			for(j = x1; j<=x2; j++)//��ѭ��
				gstDispMem.ucDispBuf[i][j] = 0x00;
		}
		if((y2+1)%8)
		{
			ucMask = ~((0x01<<((y2+1)%8))-1);
			for(j = x1;j<=x2;j++)//��ѭ��
				gstDispMem.ucDispBuf[y2/8][j] &= ucMask;
		}
	}
}
// ============================================================================

/******************************************************************************
    * ��������update_disp_data
    * ��ڲ��������ϽǼ����½�����,�������
    * ���ڲ�������
    * ����ֵ����
    * ˵����������ʾ����
******************************************************************************/
void update_disp_data(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
    unsigned char i,j;

    for(i=y1/8;i<=y2/8;i++)
    {
        for(j=x1;j<=x2;j++)
        {
            lcd_byte_pos(j, i);
            lcd_wr_dat(gstDispMem.ucDispBuf[i][j]);
        }
    }
}
// ============================================================================



/******************************************************************************
    * ��������send_zf5x8
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ5x8�ַ�
******************************************************************************/
void send_zf5x8(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char j,m,ucIdx;
	
    for(m=0;m<ucLen;m++)
    {
        ucIdx = pzfIndex[0]-32;
        for(j=0;j<5;j++)
        {
            if(ucIsInvert)
                gstDispMem.ucDispBuf[y/8][x+j] = ~(unsigned char)s5x8_code[ucIdx][j];
            else
                gstDispMem.ucDispBuf[y/8][x+j] = (unsigned char)s5x8_code[ucIdx][j];
        }
        
        pzfIndex += 1;
        x += 5;
    }      
}

/******************************************************************************
    * ��������send_zf6x10
    * ��ڲ�����
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ6x10�ַ�
******************************************************************************/
void
send_zf6x10(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char j,m,ucIdx;
	unsigned char ucBMove;		//����8������ʱ��Ҫ�ƶ���λ��
	int unDot;		//��ǰ�����ĵ�
	unsigned char ucMark,ucMark1,ucMark2;		//������ģ����
    
	ucBMove = y%8;
	ucMark = (1<<ucBMove)-1;
	ucMark1 = ~((1<<(10-8+ucBMove))-1);
	ucMark2 = 0xfe;
    for(m=0;m<ucLen;m++)	//������
    {
        ucIdx = pzfIndex[0]-32;
        
		for(j=0;j<6;j++)	//�ַ���(����)
		{
			unDot = (int)s6x10_code[ucIdx][j+6];
			unDot = (unDot<<8)|(int)s6x10_code[ucIdx][j];
			
			if(ucIsInvert)
				unDot = (~unDot)&0x3ff;		//���ֵ�10bits��Ч
			
			unDot <<= ucBMove;		//��λ����Ч�㽫�ֲ��ڵ�2���3�ֽڣ���������7λ��
			
			gstDispMem.ucDispBuf[y/8][x+j] = (gstDispMem.ucDispBuf[y/8][x+j] & ucMark) | unDot;
			gstDispMem.ucDispBuf[y/8+1][x+j] = (gstDispMem.ucDispBuf[y/8+1][x+j] & ucMark1) | (unDot>>8);
			if(unDot & 0xF0000)	//��������Ҫ����3���ֽ�
				gstDispMem.ucDispBuf[y/8+2][x+j] = (gstDispMem.ucDispBuf[y/8+2][x+j] & ucMark2) | (unDot>>16);
		}

		pzfIndex += 1;
        x += 6;
    }    
}
// ============================================================================


/******************************************************************************
    * ��������send_zf8x16
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ8*16�ַ�
******************************************************************************/

void
send_zf8x16(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char i,j,m,ucIdx;
	
    for(m=0;m<ucLen;m++)
    {
        ucIdx = pzfIndex[m]-32;
        
        for(i=0;i<2;i++)
        {
            for(j=0;j<8;j++)
            {
                if(ucIsInvert)
                    gstDispMem.ucDispBuf[y/8+i][x+j] = ~(unsigned char)s8x16_code[ucIdx][i*8+j];
                else
                    gstDispMem.ucDispBuf[y/8+i][x+j] = (unsigned char)s8x16_code[ucIdx][i*8+j];
            }
        }
        x += 8;
    }      
}

// ============================================================================

/******************************************************************************
    * ��������send_hz16x16
    * ��ڲ�������
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ����
******************************************************************************/


void 
send_hz16x16(unsigned char x, unsigned char y, unsigned char *pHzIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char i,j,k;
  int m;
   m=0;
   while(m<ucLen*2)
   {
       
       if((pHzIndex[m] & 0x80) == 0) //�����ַ�
       {
         send_zf8x16(x,y,pHzIndex+m,1,ucIsInvert);
         x+=8;
         m++;
         continue;
       }
         for(k = 0; k < GB_16_num; k++)
         {
             if((GB_16[k].Index[0] == *(pHzIndex+m)) && (GB_16[k].Index[1] == *(pHzIndex+m+1))) 
                 break;
         }
     
     k %= GB_16_num; //�Ҳ�����ʾ�ո�
         
         for(i=0;i<2;i++)
         {
             for(j=0;j<16;j++)
             {
                 if(ucIsInvert)
                   gstDispMem.ucDispBuf[y/8+i][x+j] = ~GB_16[k].Msk[i*16+j];
                 else
                   gstDispMem.ucDispBuf[y/8+i][x+j] = GB_16[k].Msk[i*16+j];
             }
         }
         
         m += 2;
         x += 16;
     }    
       
}

// ============================================================================

/* ============================================================================
    ************ �����ǻ�ͼ���� ************
============================================================================ */

/******************************************************************************
    * ��������draw_pixel
    * ��ڲ�����x ��(0~191) 
                y ��(0~63)
                IsClear�������(0 �� 1 ���)
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd ���㺯��
******************************************************************************/
void 
send_pixel(unsigned char x, unsigned char y, unsigned char IsClear)
{
    unsigned char uctemp_dat;

	if((x>191)|(y>63)) return;
      
    uctemp_dat = 0x01 << (y%8);
    
    if(IsClear == PIXEL_SET)
        gstDispMem.ucDispBuf[y/8][x] |= uctemp_dat;//����
    else
        gstDispMem.ucDispBuf[y/8][x] &= (~uctemp_dat);//�����        
}
// ============================================================================

/******************************************************************************
    * ��������send_line_h
    * ��ڲ�����x ��(0~191) 
                y ��(0~63)
                IsClear�������(0 �� 1 ���)
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd ������
******************************************************************************/
void 
send_line_h(unsigned char x, unsigned char y,  unsigned char ucLen,unsigned char IsClear)
{
    unsigned char i;
    
    for(i=x;i<x+ucLen;i++)
       send_pixel(i,y,IsClear);
        
}
// ============================================================================

/******************************************************************************
    * ��������send_line_v
    * ��ڲ�����x ��(0~191) 
                y ��(0~63)
                IsClear�������(0 �� 1 ���)
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd ������
******************************************************************************/
void 
send_line_v(unsigned char x, unsigned char y, unsigned char ucLen,unsigned char IsClear)
{
    unsigned char i;
    
    for(i=y;i<y+ucLen;i++)
       send_pixel(x,i,IsClear);
}
// ============================================================================

/******************************************************************************
    * ��������send_rectangle
    * ��ڲ�����x1 y1 ���Ͻ�����  x2 y2 ���½�����
    * ���ڲ�������
    * ����ֵ����
    * ˵����lcd ������
******************************************************************************/
void send_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
     send_line_h(x1,y1,x2-x1+1,PIXEL_SET);
     send_line_h(x1,y2,x2-x1+1,PIXEL_SET);
     send_line_v(x1,y1,y2-y1+1,PIXEL_SET);
     send_line_v(x2,y1,y2-y1+1,PIXEL_SET);
}
// ============================================================================

/******************************************************************************
    * ��������disp_ico
    * ��ڲ�����x
    * ��ڲ�����y
    * ��ڲ�����Width
    * ��ڲ�����Height
    * ��ڲ�����pIcoData
    * ��ڲ�����ucIsInvert
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ16x16ͼ��(yֻ��Ϊ8��������)
******************************************************************************/
void disp_ico(unsigned char x, unsigned char y, unsigned char Width, unsigned char Height,const unsigned char *pIcoData, unsigned char ucIsInvert)
{
    int i, j;
    
    for(i=0;i<Height/8;i++)
    {
        for(j=0;j<Width;j++)
        {
            if(ucIsInvert)
                gstDispMem.ucDispBuf[y/8+i][x+j] = ~pIcoData[i*Width+j];
            else
                gstDispMem.ucDispBuf[y/8+i][x+j] = pIcoData[i*Width+j];
        }
    }
}
// ============================================================================


/********************************************************************************
  * ��������    Lcd192x64Init
  * ��ڲ�����  ��
  * ���ڲ�����  ��
  *	����ֵ��	��
  * ˵����   	Һ����ʼ��
********************************************************************************/
int lcd192x64_init(void)
{
		int i;
		
		g_lcd_gpio_fd = open("/dev/Lcd192x64", O_RDWR | O_NDELAY); 			 
		
		if (g_lcd_gpio_fd < 0) 	 
		{  
				printf("open(/dev/Lcd192x64) failed.\n"); 		 
				return (-1);  
		} 	 

		// �����ʾRAM
    memset(gstDispMem.ucDispBuf[0],' ',sizeof(gstDispMem.ucDispBuf));
		
    lcd_init();
		// ��lcd����
		ioctl(g_lcd_gpio_fd,1,LCD_LED);
    
    clear_block(0,0,192,64);
    for(i=0; i<PAGE_NUM; i++)
    {
      memcpy(gstDispMem.ucDispBuf[i],&Logo192x64[i*192],192);
    }
    update_disp_data(0,0,192,64);
		return 0;
}

int lcd192x64_close( void )// yasirLiang add in 2016/05/16
{
	if( g_lcd_gpio_fd > 0 )
		close( g_lcd_gpio_fd );

	return 0;
}

//===============================================================================
