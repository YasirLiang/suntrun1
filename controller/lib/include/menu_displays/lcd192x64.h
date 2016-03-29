#ifndef __LCD192X64_H__
#define __LCD192X64_H__


//命令
#define DISP_ON             0xAF
#define DISP_OFF            0xAE
#define DISP_START_LINE_SET 0x40//0~63
#define DISP_PAGE_SET       0xB0//0~7
#define DISP_COL_H_SET      0x10//高4位
#define DISP_COL_L_SET      0x00//低4位

//引脚
#define LCD_LED 1
#define LCD_RST 2
#define LCD_CSB 3
#define LCD_CSA 4
#define LCD_A0 	5
#define LCD_SCL 6
#define LCD_SI	7
#define LCD_EN	8
#define LCD_RW	9


#define LCD_SET_RST(x,a)      ioctl(x,a,LCD_RST)//0有效

#define WR_CMD(x)				ioctl(x,0,LCD_A0);ioctl(x,1,LCD_EN);ioctl(x,0,LCD_RW) // RS=0,EN=1,RW=0      
#define WR_DATA(x)			ioctl(x,1,LCD_A0);ioctl(x,1,LCD_EN);ioctl(x,0,LCD_RW) // RS=1,EN=1,RW=0        
#define RD_STATE(x)			ioctl(x,0,LCD_A0);ioctl(x,0,LCD_EN);ioctl(x,1,LCD_RW) // RS=0,EN=0,RW=1        
#define RD_DATA(x)      ioctl(x,1,LCD_A0);ioctl(x,0,LCD_EN);ioctl(x,1,LCD_RW) // RS=1,EN=0,RW=1 
#define WR_RD_OK(x)			ioctl(x,0,LCD_A0);ioctl(x,0,LCD_EN);ioctl(x,0,LCD_RW) // RS=0,EN=0,RW=0 

#define CLOCK_DOWN(x)		ioctl(x,0,LCD_SCL)
#define CLOCK_UP(x)			ioctl(x,1,LCD_SCL)
#define SEND_DAT_1(x)		ioctl(x,1,LCD_SI)
#define SEND_DAT_0(x)		ioctl(x,0,LCD_SI)

#define LCD_SELE_LEFT(x)	{ioctl(x,1,LCD_CSA);ioctl(x,0,LCD_CSB);}
#define LCD_SELE_RIGHT(x)	{ioctl(x,0,LCD_CSA);ioctl(x,1,LCD_CSB);}
#define LCD_SELE_ALL(x)		{ioctl(x,0,LCD_CSA);ioctl(x,0,LCD_CSB);}
#define LCD_SELE_NONE(x)	{ioctl(x,1,LCD_CSA);ioctl(x,1,LCD_CSB);}

#define PIXEL_SET	0
#define PIXEL_CLR	1

#define PAGE_NUM    8
#define COL_SUM    96*2

//显示缓存结构
typedef struct stDispMem{
    unsigned char ucX1;
    unsigned char ucX2;
    unsigned char ucY1;
    unsigned char ucY2;
    unsigned char ucDispBuf[PAGE_NUM+1][COL_SUM+1];
}TDispMem;

typedef enum
{
	FALSE=0,
	TRUE,
}Bool;

void clear_block(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void send_zf5x8(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
void send_zf6x10(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
void send_zf8x16(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
void send_hz16x16(unsigned char x, unsigned char y, unsigned char *pHzIndex, unsigned char ucLen, unsigned char ucIsInvert);
void send_pixel(unsigned char x, unsigned char y, unsigned char IsClear);
void send_line_h(unsigned char x, unsigned char y,  unsigned char ucLen,unsigned char IsClear);
void send_line_v(unsigned char x, unsigned char y, unsigned char ucLen,unsigned char IsClear);
void send_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void disp_ico(unsigned char x, unsigned char y, unsigned char Width, unsigned char Height, const unsigned char *pIcoData, unsigned char ucIsInvert);
void update_disp_data(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
int lcd192x64_init(void);

#endif //__LCD192X64_H__
