#ifndef __MENU_F_H__
#define __MENU_F_H__

#include "lcd192x64.h"

extern TDispMem gstDispMem;


//�˵�������
#define MT_STATE	  0xFF
#define MT_MENU		  1
#define MT_OPERATE	2
#define MT_YESNO	  3
#define MT_ONOFF	  4
#define MT_SWITCH	  5
#define MT_VAR_MENU 6
//��Ļ����
#define ST_CMN    1 //��ͨ�˵�
#define ST_VALUE  2 //��ֵ����
#define ST_LR     3 //���Ҽ�ͷ
#define ST_UD     4 //���¼�ͷ
#define ST_MODE   5 //ģʽѡ��
#define ST_TMN_SEL  6 //�ն�ѡ��

#define CHINESE		0
#define ENGLISH		1

//ÿ���˵�����������Ϊ3��Ӣ��Ϊ4��
//#define MI_C_LINES 		3
//#define MI_E_LINES 		4
#define MI_LINES 		3		//Ӣ�Ĳ���6*10���󣬲˵���ͳһΪ3��
	
//�˵���ȣ�����Ϊ14��Ӣ��31��
#define MI_C_LENS 		14
#define MI_E_LENS 		31		//Ӣ�Ĳ˵��ã����������������ʾ�������ַ�

//����������
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

//������Դ�궨��
#define TMN_USE   (1<<0)
#define MENU_USE  (1<<1)
#define SYS_USE   (1<<2)
#define CPT_USE   (1<<3)


typedef  struct 
{
  int ucMode;      //ģʽ 0 ����ģʽ  1 ����ģʽ  2  �Ƚ��ȳ�  3 ����ģʽ
  int ucTempClose;  //��ʱ�رչ��� 0 �ر�  1 ��
  int ucCmHint;    //��ϯ��ʾ���� 0 �ر�  1 ��
  int ucAutoClose; //�Զ��رչ���  0 �ر�  1 ��
  int ucNumLimit;  //����������
  int ucCurCmr;     //��ǰ����ͷ
  int ucCmrTrack;   //�������
  int ucApplyLimit; //������������
  int ucLang;
}TMenuSet;



typedef struct Menu{
	char Text[20];
	short Type;		//��������
	short Link;		//��Ҫ���Ӳ˵���������
	void (*fpMenuDo)(unsigned int value);
//    unsigned char x;
//    unsigned char y;
	signed char Index;	//�������ڵ�ǰ�˵��е����
}TMenu;

typedef struct MGrpData{
//	char Title[16];
	char *pTitle;
	TMenu *FirstItem;
  unsigned char Type;
 	unsigned char Count;		//��ͨ�˵���ʾ�˵������������ı�ʾ���ص�����
	short Level;		//��ǰ�˵��鼶�����˵�Ϊ1���¼�Ϊ2....
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
//���˵����±꣨����gMG���飩
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
