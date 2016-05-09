#ifndef __MENU_F_H__
#define __MENU_F_H__

#include "lcd192x64.h"

extern TDispMem gstDispMem;


//菜单项类型
#define MT_STATE	  0xFF
#define MT_MENU		  1
#define MT_OPERATE	2
#define MT_YESNO	  3
#define MT_ONOFF	  4
#define MT_SWITCH	  5
#define MT_VAR_MENU 6
//屏幕类型
#define ST_CMN    1 //普通菜单
#define ST_VALUE  2 //数值设置
#define ST_LR     3 //左右箭头
#define ST_UD     4 //上下箭头
#define ST_MODE   5 //模式选择
#define ST_TMN_SEL  6 //终端选择

#define CHINESE		0
#define ENGLISH		1

//每屏菜单项数（中文为3，英文为4）
//#define MI_C_LINES 		3
//#define MI_E_LINES 		4
#define MI_LINES 		3		//英文采用6*10点阵，菜单项统一为3行
	
//菜单项长度（中文为14，英文31）
#define MI_C_LENS 		14
#define MI_E_LENS 		31		//英文菜单用６ｘ１０点阵，最多显示３１个字符

//滚动条方向
#define SB_VERT		1
#define SB_HORZ		2

#define MD_NORMAL	0
#define MD_REVERSE	1
#define MD_NULL	    2

#define PPT_MODE		0
#define	LIMIT_MODE	1
#define	FIFO_MODE 	2
#define	APPLY_MODE	3


#define FULL_VIEW_ADDR  0x1000
#define BACKUP_FULL_VIEW_ADDR	0x0FFF

//命令来源宏定义
#define TMN_USE   (1<<0)
#define MENU_USE  (1<<1)
#define SYS_USE   (1<<2)
#define CPT_USE   (1<<3)


typedef  struct 
{
  int ucMode;      //模式 0 自由模式  1 限制模式  2  先进先出  3 申请模式
  int ucTempClose;  //暂时关闭功能 0 关闭  1 打开
  int ucCmHint;    //主席提示功能 0 关闭  1 打开
  int ucAutoClose; //自动关闭功能  0 关闭  1 打开
  int ucNumLimit;  //开启量限制
  int ucCurCmr;     //当前摄像头
  int ucCmrTrack;   //摄像跟踪
  int ucApplyLimit; //申请人数上限
  int ucLang;
}TMenuSet;



typedef struct Menu{
	char Text[20];
	short Type;		//本项类型
	short Link;		//需要链接菜单组的组序号
	void (*fpMenuDo)(unsigned int value);
//    unsigned char x;
//    unsigned char y;
	signed char Index;	//本对象在当前菜单中的序号
}TMenu;

typedef struct MGrpData{
//	char Title[16];
	char *pTitle;
	TMenu *FirstItem;
  unsigned char Type;
 	unsigned char Count;		//普通菜单表示菜单项数，其它的表示返回的链接
	short Level;		//当前菜单组级别，主菜单为1，下级为2....
	union {
		unsigned int BitState;	
		struct {
			short Min;	
			short Max;	
		}mm;
	}ss;
 	short Position;
  int Value;
}TMGrpData;

typedef  struct
{
  unsigned char   GrpNum;
  unsigned char   LineNum;
  unsigned char   ValueIndex;
}TMenuValue;
//各菜单组下标（按照gMG数组）
#define MGS_NONE	-1	
typedef enum
{
  MGS_MAIN=0,
  MGS_M11,		
  MGS_M12,	
  MGS_M13,			
  MGS_M21,		
  MGS_CCM,   
  MGS_ONUM,  
  MGS_ANUM,
  MGS_LR,    
  MGS_UD,    
  MGS_FOCUS, 
  MGS_ZOOM,  
  MGS_APERTURE, 
  MGS_MODE,  
  MGS_TMN_SEL,
  MGS_STATE, 
  MGS_NUM
}EMgs;

typedef enum
{
  MGS_MAIN_E=0,
  MGS_M11_E,		
  MGS_M12_E,	
  MGS_M13_E,		
  MGS_M21_E,		
  MGS_CCM_E,   
  MGS_ONUM_E,
  MGS_ANUM_E,
  MGS_LR_E,    
  MGS_UD_E,    
  MGS_FOCUS_E, 
  MGS_ZOOM_E,  
  MGS_APERTURE_E, 
  MGS_MODE_E,
  MGS_TMN_SEL_E,
  MGS_STATE_E, 
  MGS_NUM_E
}EMgs_E;



extern short gsnCurMGrp;

extern void clear_block(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
extern void send_zf5x8(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
extern void send_zf6x10(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
extern void send_zf8x16(unsigned char x, unsigned char y, unsigned char *pzfIndex, unsigned char ucLen, unsigned char ucIsInvert);
extern void send_hz16x16(unsigned char x, unsigned char y, unsigned char *pHzIndex, unsigned char ucLen, unsigned char ucIsInvert);
extern void send_pixel(unsigned char x, unsigned char y, unsigned char IsClear);
extern void send_line_h(unsigned char x, unsigned char y,  unsigned char ucLen,unsigned char IsClear);
extern void send_line_v(unsigned char x, unsigned char y, unsigned char ucLen,unsigned char IsClear);
extern void send_rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
extern void send_v_scrollBar(unsigned char x, unsigned char y, unsigned char ucHeight);
extern void send_h_StepBar(unsigned char x, unsigned char y, unsigned char ucWidth,unsigned char BarWidth);

Bool CtrlMenuSw(short snMGSeq);

void DisplayOneState(int Line, int Value);

void GetMenuValue(unsigned char GrpNum,unsigned char LineNum,unsigned int *pValue);

void SaveMenuValue(unsigned char GrpNum,unsigned char LineNum,unsigned int Value);

void ShowSelTmn(unsigned short Addr);

void MenuInit(void);

short MenuScroll(short snMGSeq,int nInc);// yasir add in 2016-3-29
void ItemSelected(short snMGSeq);// yasir add in 2016-3-29
#endif //__MENU_F_H__
