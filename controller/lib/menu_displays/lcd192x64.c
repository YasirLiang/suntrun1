/********************************************************************************
* 文件名：    lcd192x64.c
* 创建人：    SUNTRON 肖云龙
* 版本号：    V1.0
* 创建时间：  2013-01-31
* 说明：      会议主机ACS41xxM-ACS42xxM液晶显示程序代码,液晶型号为：HG1926417G
******************************************************************************
* 
*
* Copyright (c) 2012-2015 GuangZhou SUNTRON electronics co., ltd. All rights reserved.
*********************************************************************************
* 修改
*
* 日期			版本			更改说明
*
*
********************************************************************************/ 
//===============================================================================

/********************************************************************************
  * 包含头文件
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
  * 全局变量
********************************************************************************/
TDispMem gstDispMem;


int g_lcd_gpio_fd;

// ============================================================================

/******************************************************************************
    * 函数名：lcd_wr_cmd
    * 入口参数：ucCmd 要写的命令
    * 出口参数：无
    * 返回值：无
    * 说明：lcd写命令
******************************************************************************/
void lcd_wr_cmd(unsigned char ucCmd) 
{ 
	int i;
 
    // 写命令端口状态切换
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
    * 函数名：lcd_wr_dat
    * 入口参数：dat
    * 出口参数：无
    * 返回值：无
    * 说明：LCD写数据
******************************************************************************/
void lcd_wr_dat(unsigned char ucDat) 
{ 
    int i;
    
    // 写数据端口状态切换
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
    * 函数名：lcd_clr
    * 入口参数：ucSele 清除的屏：0 左半屏 1 右半屏 2 全屏
    * 出口参数：无
    * 返回值：无
    * 说明：lcd显示清屏
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
    * 函数名：lcd_init
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：LCD初始化
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

    lcd_wr_cmd(0xAE); // 关闭显示

    lcd_wr_cmd(0xA0); // 设置DDRAM地址SEG输出对应(ADC = 0)——SEG127 SEG0

    lcd_wr_cmd(0xC8); // 设置COM输出扫描方向(SHL = 1)COM63-COM0 

    lcd_wr_cmd(0x40); // DDRAM显示开始行地址(0) 

    lcd_wr_cmd(0xA6); // 设置显示模式为正常模式 

    lcd_wr_cmd(0xA4); // 设置点显示模式为正常模式 

    lcd_wr_cmd(0xA2); // 设置液晶显示驱动电压偏差比(1/9)

    lcd_wr_cmd(0x2F); // 打开升压电路、电压调节电路和电压输出电路 
    
    lcd_wr_cmd(0x24); // 选择内部电阻率(Rb / Ra) 

    lcd_wr_cmd(0x81); // 设置对比度
    lcd_wr_cmd(0x20); 
    
    lcd_wr_cmd(0xAF); // 打开显示 
	
	lcd_clr(2);
}   
// ============================================================================


/******************************************************************************
    * 函数名：lcd_byte_pos
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：lcd字节定位
******************************************************************************/
void lcd_byte_pos(unsigned char x, unsigned char y)
{
	if(x < 96)//左半屏	   
        LCD_SELE_LEFT(g_lcd_gpio_fd)		
	else if(x < 192) 
    {//右半屏
        x -= 96; 
        LCD_SELE_RIGHT(g_lcd_gpio_fd)
    }
    
//     y <<= 3;
	lcd_wr_cmd(DISP_PAGE_SET + y);//页地址0-7
	lcd_wr_cmd(DISP_COL_H_SET | (x>>4));//列地址0-63
	lcd_wr_cmd(DISP_COL_L_SET | (x&0x0F));//列地址0-63
}
// ============================================================================

/******************************************************************************
    * 函数名：clear_block
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：清空块
******************************************************************************/
void clear_block(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
    unsigned char i,j,ucMask=0;

	if(y1/8 == y2/8)	//清除的内容垂直方向在一个字节中
	{
		for(i=y1%8;i<(y2)%8;i++)
			ucMask |= (1<<i);
		ucMask = ~ucMask;
        for(j = x1;j<=x2;j++)//列循环
        	gstDispMem.ucDispBuf[y1/8][j] &= ucMask;
	}
	else
	{
		if(y1%8)
		{
			ucMask = (0x01<<(y1%8))-1;
			for(j = x1;j<=x2;j++)//列循环
				gstDispMem.ucDispBuf[y1/8][j] &= ucMask;
		}
		for(i=y1/8+(y1%8 ? 1:0);i<y2/8+(((y2+1)%8==0) ? 1:0);i++)
		{
			for(j = x1; j<=x2; j++)//列循环
				gstDispMem.ucDispBuf[i][j] = 0x00;
		}
		if((y2+1)%8)
		{
			ucMask = ~((0x01<<((y2+1)%8))-1);
			for(j = x1;j<=x2;j++)//列循环
				gstDispMem.ucDispBuf[y2/8][j] &= ucMask;
		}
	}
}
// ============================================================================

/******************************************************************************
    * 函数名：update_disp_data
    * 入口参数：左上角及右下角坐标,按点计算
    * 出口参数：无
    * 返回值：无
    * 说明：更新显示数据
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
    * 函数名：send_zf5x8
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：显示5x8字符
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
    * 函数名：send_zf6x10
    * 入口参数：
    * 出口参数：无
    * 返回值：无
    * 说明：显示6x10字符
******************************************************************************/
void
send_zf6x10(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char j,m,ucIdx;
	unsigned char ucBMove;		//不是8整数倍时需要移动的位数
	int unDot;		//当前读出的点
	unsigned char ucMark,ucMark1,ucMark2;		//用了掩模的数
    
	ucBMove = y%8;
	ucMark = (1<<ucBMove)-1;
	ucMark1 = ~((1<<(10-8+ucBMove))-1);
	ucMark2 = 0xfe;
    for(m=0;m<ucLen;m++)	//串长度
    {
        ucIdx = pzfIndex[0]-32;
        
		for(j=0;j<6;j++)	//字符宽(点数)
		{
			unDot = (int)s6x10_code[ucIdx][j+6];
			unDot = (unDot<<8)|(int)s6x10_code[ucIdx][j];
			
			if(ucIsInvert)
				unDot = (~unDot)&0x3ff;		//保持低10bits有效
			
			unDot <<= ucBMove;		//移位后有效点将分布在低2或低3字节（最坏情况是移7位）
			
			gstDispMem.ucDispBuf[y/8][x+j] = (gstDispMem.ucDispBuf[y/8][x+j] & ucMark) | unDot;
			gstDispMem.ucDispBuf[y/8+1][x+j] = (gstDispMem.ucDispBuf[y/8+1][x+j] & ucMark1) | (unDot>>8);
			if(unDot & 0xF0000)	//最坏情况，需要处理3个字节
				gstDispMem.ucDispBuf[y/8+2][x+j] = (gstDispMem.ucDispBuf[y/8+2][x+j] & ucMark2) | (unDot>>16);
		}

		pzfIndex += 1;
        x += 6;
    }    
}
// ============================================================================


/******************************************************************************
    * 函数名：send_zf8x16
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：显示8*16字符
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
    * 函数名：send_hz16x16
    * 入口参数：无
    * 出口参数：无
    * 返回值：无
    * 说明：显示汉字
******************************************************************************/


void 
send_hz16x16(unsigned char x, unsigned char y, unsigned char *pHzIndex, unsigned char ucLen, unsigned char ucIsInvert)
{
	unsigned char i,j,k;
  int m;
   m=0;
   while(m<ucLen*2)
   {
       
       if((pHzIndex[m] & 0x80) == 0) //西文字符
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
     
     k %= GB_16_num; //找不到显示空格
         
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
    ************ 以下是画图函数 ************
============================================================================ */

/******************************************************************************
    * 函数名：draw_pixel
    * 入口参数：x 行(0~191) 
                y 列(0~63)
                IsClear画或清除(0 画 1 清除)
    * 出口参数：无
    * 返回值：无
    * 说明：lcd 画点函数
******************************************************************************/
void 
send_pixel(unsigned char x, unsigned char y, unsigned char IsClear)
{
    unsigned char uctemp_dat;

	if((x>191)|(y>63)) return;
      
    uctemp_dat = 0x01 << (y%8);
    
    if(IsClear == PIXEL_SET)
        gstDispMem.ucDispBuf[y/8][x] |= uctemp_dat;//画点
    else
        gstDispMem.ucDispBuf[y/8][x] &= (~uctemp_dat);//清除点        
}
// ============================================================================

/******************************************************************************
    * 函数名：send_line_h
    * 入口参数：x 行(0~191) 
                y 列(0~63)
                IsClear画或清除(0 画 1 清除)
    * 出口参数：无
    * 返回值：无
    * 说明：lcd 画横线
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
    * 函数名：send_line_v
    * 入口参数：x 行(0~191) 
                y 列(0~63)
                IsClear画或清除(0 画 1 清除)
    * 出口参数：无
    * 返回值：无
    * 说明：lcd 画竖线
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
    * 函数名：send_rectangle
    * 入口参数：x1 y1 左上角坐标  x2 y2 右下角坐标
    * 出口参数：无
    * 返回值：无
    * 说明：lcd 画矩形
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
    * 函数名：disp_ico
    * 入口参数：x
    * 入口参数：y
    * 入口参数：Width
    * 入口参数：Height
    * 入口参数：pIcoData
    * 入口参数：ucIsInvert
    * 出口参数：无
    * 返回值：无
    * 说明：显示16x16图标(y只能为8的整数倍)
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
  * 程序名：    Lcd192x64Init
  * 入口参数：  无
  * 出口参数：  无
  *	返回值：	无
  * 说明：   	液晶初始化
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

		// 清空显示RAM
    memset(gstDispMem.ucDispBuf[0],' ',sizeof(gstDispMem.ucDispBuf));
		
    lcd_init();
		// 打开lcd背光
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
