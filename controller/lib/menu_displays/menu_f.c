#include <stddef.h>
#include <stdio.h> 
#include <string.h>
#include "lcd192x64.h"
#include "ico_data.h"
#include "menu_exe.h"
#include "menu_f.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	Ŀǰÿ��˵��������32��
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


// ��Ҫ��ʾ�� 4 ��ϵͳ״̬
#define STD_MODE	0x01
#define STD_CAM		0x02
#define STD_LIMIT	0x04
#define STD_APPLY	0x08


//���±�����ѡ��ʱ�ı䣬�ڸ�λʱ�������ж�ȡ
//int MI_LINES =	3;
int MI_LENS =	14;


const TMenu gMState[]= //״̬��ҳ       ����       ����       
{
    {"ģʽ��",		    MT_STATE , MGS_NONE,   NULL,0},
    {"��ʱ�رգ�",		MT_STATE , MGS_NONE,   NULL,1},
    {"��ϯ��ʾ��",		MT_STATE , MGS_NONE,   NULL,2},
    {"�Զ��رգ�",		MT_STATE , MGS_NONE,   NULL,3},
    {"����ͷ��",		MT_STATE , MGS_NONE,   NULL,4},
    {"��������",		MT_STATE , MGS_NONE,   NULL,5}
};
const TMenu gMState_E[]= //״̬��ҳ
{
    {"Mode:",		    MT_STATE , MGS_NONE,   NULL,0},
    {"Temporary Close:",	MT_STATE , MGS_NONE,   NULL,1},
    {"Chairman Alarm:",	MT_STATE , MGS_NONE,   NULL,2},
    {"Auto Close:",		MT_STATE , MGS_NONE,   NULL,3},
    {"Camera:",		    MT_STATE , MGS_NONE,   NULL,4},
    {"Open quantity:",	MT_STATE , MGS_NONE,   NULL,5}
};
const TMenu gMMain[]=  	//���˵�        ����        ����        
{
    {"�����趨",		MT_VAR_MENU,	MGS_M11,    				NULL,0},
    {"��˷�ID�趨",	MT_OPERATE,	MGS_M13,    				NULL,1},
    {"����ͷ��λ",		MT_MENU,	MGS_M12,    EnterCmrPreset,		 2},
    {"To English",		MT_SWITCH , MGS_NONE,   				NULL,3},
//    {"����״̬ҳ",		MT_STATE ,  MGS_STATE,  				NULL,4}
};
const TMenu gMMain_E[]= //���˵�        ����        ����        
{
    {"Meet Setting",	MT_VAR_MENU,	MGS_M11_E,  	NULL,0},
    {"Micphone ID",	    MT_OPERATE,	MGS_M13_E,  	NULL,1},
    {"Camera Setting",	MT_MENU,	MGS_M12_E,  	EnterCmrPreset,2},
    {"To Chinese",		MT_SWITCH,  MGS_NONE,   	NULL,3},
//    {"Return status page",	MT_STATE,  MGS_STATE,   NULL,4}
};

const TMenu gM11[]=  	//��ģʽ�趨    ����        ����        
{
		{"��ģʽ�趨",		MT_OPERATE,	MGS_MODE,   ModeSet,      0},
		{"��ʱ�ر�",		  MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
		{"��ϯ��ʾ",		  MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"�������",      MT_ONOFF,   MGS_NONE,   CmrTrackSet,  3},
		{"�Զ��ر�",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet, 4},
		{"����������",		MT_OPERATE,	MGS_ONUM,   SpkLmtSet,    5},
		{"��������",		  MT_OPERATE,	MGS_ANUM,   ApplyLmtSet,  6},
		{"����",			    MT_MENU,	  MGS_MAIN,   NULL,         7}
};

const TMenu gM11_E[]=  	//��ģʽ�趨    ����        ����     
{
		{"Mode Setting",		MT_OPERATE,   MGS_MODE_E,   ModeSet,0},
		{"Temporary Close",	MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
		{"Chairman Alarm",	MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"Cemara Track",      MT_ONOFF, MGS_NONE,   CmrTrackSet,3}, 
		{"Auto Close",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet,4},
		{"Max Num of Mic(ON)",	MT_OPERATE,	MGS_ONUM_E, SpkLmtSet,5},
		{"Max Num of Apply",    MT_OPERATE,	MGS_ANUM_E, ApplyLmtSet,6},
		{"Return",			    MT_MENU,	  MGS_MAIN_E, NULL,7}
};
          

const TMenu gM14[]=   //��ģʽ�趨    ����        ����        
{
    {"��ģʽ�趨",    MT_OPERATE, MGS_MODE,   ModeSet,      0},
    {"��ʱ�ر�",      MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
    {"��ϯ��ʾ",      MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"�������",      MT_ONOFF,   MGS_NONE,   CmrTrackSet,  3}, 
    {"�Զ��ر�",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet, 4},
    {"��������",      MT_OPERATE, MGS_ANUM,   ApplyLmtSet,  5},
    {"����",          MT_MENU,    MGS_MAIN,   NULL,         6}
};

const TMenu gM14_E[]=   //��ģʽ�趨    ����        ����     
{
    {"Mode Setting",    MT_OPERATE,   MGS_MODE_E,   ModeSet,0},
    {"Temporary Close", MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
    {"Chairman Alarm",  MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"Cemara Track",      MT_ONOFF, MGS_NONE,   CmrTrackSet,3},  
    {"Auto Close",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet,4},
    {"Max Num of Apply",    MT_OPERATE, MGS_ANUM_E, ApplyLmtSet,5},
    {"Return",          MT_MENU,    MGS_MAIN_E, NULL,6}
};

const TMenu gM15[]=   //��ģʽ�趨    ����        ����        
{
    {"��ģʽ�趨",    MT_OPERATE, MGS_MODE,   ModeSet,      0},
    {"��ʱ�ر�",      MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
    {"��ϯ��ʾ",      MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"�������",      MT_ONOFF,   MGS_NONE,   CmrTrackSet,  3},
    {"�Զ��ر�",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet, 4},
    {"����",          MT_MENU,    MGS_MAIN,   NULL,         5}
};

const TMenu gM15_E[]=   //��ģʽ�趨    ����        ����     
{
    {"Mode Setting",    MT_OPERATE,   MGS_MODE_E,   ModeSet,0},
    {"Temporary Close", MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
    {"Chairman Alarm",  MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"Cemara Track",      MT_ONOFF, MGS_NONE,   CmrTrackSet,3}, 
    {"Auto Close",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet,4},
    {"Return",          MT_MENU,    MGS_MAIN_E, NULL,5}
};

const TMenu gM16[]=  	//��ģʽ�趨    ����        ����        
{
		{"��ģʽ�趨",		MT_OPERATE,	MGS_MODE,   ModeSet,      0},
		{"��ʱ�ر�",		  MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
		{"��ϯ��ʾ",		  MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"�������",      MT_ONOFF,   MGS_NONE,   CmrTrackSet,  3},
    {"�Զ��ر�",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet, 4},
		{"����������",		MT_OPERATE,	MGS_ONUM,   SpkLmtSet,    5},
		{"����",			    MT_MENU,	  MGS_MAIN,   NULL,         6}
};

const TMenu gM16_E[]=  	//��ģʽ�趨    ����        ����     
{
		{"Mode Setting",		MT_OPERATE,   MGS_MODE_E,   ModeSet,0},
		{"Temporary Close",	MT_YESNO,   MGS_NONE,   ChmPriorEnSet,1},
		{"Chairman Alarm",	MT_ONOFF,   MGS_NONE,   ChmMusicEnSet,2},
    {"Cemara Track",      MT_ONOFF, MGS_NONE,   CmrTrackSet,3},  
    {"Auto Close",		  MT_ONOFF,   MGS_NONE,   AutoOffEnSet,4},
		{"Max Num of Mic(ON)",	MT_OPERATE,	MGS_ONUM_E, SpkLmtSet,5},
		{"Return",			    MT_MENU,	  MGS_MAIN_E, NULL,6}
};

const TMenu gM12[]=  	//����ͷ��λ    ����        ����     
{
    {"��ǰ��̨����",	MT_MENU,	MGS_M21,    NULL,0},
    {"ң��������",	  MT_ONOFF,	MGS_NONE,   RCtrlAlign,1},
    {"Ԥ��λ�洢",	    MT_MENU,   MGS_NONE,   SavePreset,2},
    {"�л�����ͷ",		MT_OPERATE,    MGS_CCM,   SwitchCamera,3},
    {"���Ԥ��λ",		MT_MENU,   MGS_NONE,   ClearPreset,4},
    {"Ԥ���ն�ѡ��",  MT_OPERATE,MGS_TMN_SEL,PresetTmnSel,5},
    {"����",			MT_MENU,	MGS_MAIN,   EscCmrPreset,6}
};

const TMenu gM12_E[]=  	//����ͷ��λ        ����        ����      
{
    {"Pan Tilt Control",	MT_MENU,	MGS_M21_E,  NULL,0},
    {"RCtrl Match Code",  MT_ONOFF, MGS_NONE,   RCtrlAlign,1},
    {"Save presetting",	    MT_MENU,   MGS_NONE,   SavePreset,2},
    {"Switch Camera",		MT_OPERATE,    MGS_CCM_E,   SwitchCamera,3},
    {"Clear presetting",	MT_MENU,   MGS_NONE,   ClearPreset,4},
    {"Preset Tmn Select",  MT_OPERATE,MGS_TMN_SEL_E,PresetTmnSel,5}, 
    {"Return",			    MT_MENU,	MGS_MAIN_E, NULL,6}
};

const TMenu gM13[]=  	//��˷�ID����      ����        ����      
{
    {"��������",		MT_MENU,     MGS_NONE,TerminalReAllotAddr,0},
    {"��������",		MT_MENU,     MGS_NONE,TerminalAddAllotAddr,1},
    {"�������",		MT_MENU,     MGS_NONE,TerminalAllotAddrOver,2},
    {"����",			MT_MENU,    MGS_MAIN,    NULL,3}
};

const TMenu gM13_E[]=  	//��˷�ID����   ����        ����       
{
    {"Reset",		MT_MENU,     MGS_NONE,   TerminalReAllotAddr,0},
    {"Add",		    MT_MENU,     MGS_NONE,   TerminalAddAllotAddr,1},
    {"Set Over",		MT_MENU,     MGS_NONE,TerminalAllotAddrOver,2},
    {"Return",			MT_MENU,    MGS_MAIN_E,  NULL,3}
};


const TMenu gM21[]=  	//������̨      ����        ����      
{
    {"����",			MT_OPERATE,     MGS_LR,   CameraLR,0},
    {"����",			MT_OPERATE,     MGS_UD,   CameraUD,1},
    {"�۽�",		MT_OPERATE,     MGS_FOCUS,   CameraFouce,2},
    {"����",			MT_OPERATE,     MGS_ZOOM,   CameraZoom,3},
    {"��Ȧ",		MT_OPERATE,     MGS_APERTURE, CameraAperture,4},
    {"����",			MT_MENU,	MGS_M12,    NULL,5}
};

const TMenu gM21_E[]=  	//������̨      ����        ����     
{
    {"Left-Right",		MT_OPERATE,     MGS_LR,   CameraLR,0},
    {"Up-Down",			MT_OPERATE,     MGS_UD,   CameraUD,1},
    {"Focus",		    MT_OPERATE,     MGS_FOCUS,   CameraFouce,2},
    {"Zoom IN-OUT",		MT_OPERATE,     MGS_ZOOM,   CameraZoom,3},
    {"Aperture",		MT_OPERATE,     MGS_APERTURE,   CameraAperture,4},
    {"Return",			MT_MENU,	MGS_M12_E,  NULL,5}
};


const char gMGTitle[][24]=
{
    "��V3.004���˵�",
		 "�ڻ����趨",
    "�ڶ�λ����ͷ  ",
    "���趨��˷�ID",
    "  �ۿ�����̨  ",
    "�۵�ǰ����ͷ ",
    "�ۿ���������  ",
    "����������    ",
    "������",
    "������",
    "�ܾ۽�",
    "������",
    "�ܹ�Ȧ",
		 "����ģʽ�趨",
		 "��Ԥ���ն�ѡ��"
};
const char gMGTitle_E[][24]=
{
    "     (1) Main Menu     ",
    "   (2) Meet Setting    ",
    "   (2) Camera Setting  ",
    "(2) Micphone ID Setting",
    "   (3) Pan Tilt Drive  ",
    "   (3) current Camera  ",
    "(3) Max Num of Mic(ON) ",
    "(3) Max Num of Apply   ",
    "(4) Left-Right",
    "(4) Up-Down",
    "(4) Focus",
    "(4) Zoom IN-OUT",
    "(4) Aperture",
    "   (3) Mode Setting    ",
    "(3)Preset Tmn Select"
};

//�˵����±곣��
#define MN_MAIN		0
#define MN_MODE		1
#define MN_PANT		2
#define MN_MIC		3
#define MN_CAM		4
//#define MN_MAIN		5
//#define MN_MAIN		6
//#define MN_MAIN		7
//#define MN_MAIN		8
//#define MN_MAIN		9
//#define MN_MAIN		10


//�˵���
TMGrpData gMG[]=  	//Text				    SubMenu			ItemsCount						Level	Min & Max		Position(0)
{
    {(char *)gMGTitle[MGS_MAIN],	(TMenu*)gMMain,	ST_CMN,sizeof(gMMain)/sizeof(TMenu),	1},
    {(char *)gMGTitle[MGS_M11],	(TMenu*)gM11,	ST_CMN,sizeof(gM11)/sizeof(TMenu),		2},
    {(char *)gMGTitle[MGS_M12],	(TMenu*)gM12,	ST_CMN,sizeof(gM12)/sizeof(TMenu),		2},
    {(char *)gMGTitle[MGS_M13],	(TMenu*)gM13,	ST_CMN,sizeof(gM13)/sizeof(TMenu),		2},
    {(char *)gMGTitle[MGS_M21],	(TMenu*)gM21,	ST_CMN,sizeof(gM21)/sizeof(TMenu),		3},
    {(char *)gMGTitle[MGS_CCM],	NULL,			ST_VALUE,MGS_M12,				3,		(0x01 | (0x04<<16))},		//��ǰ����ͷ
    {(char *)gMGTitle[MGS_ONUM],	NULL,			ST_VALUE,MGS_M11,				3,		(0x01 | (0x06<<16))},			//����������
    {(char *)gMGTitle[MGS_ANUM],	NULL,			ST_VALUE,MGS_M11,				3,		(0x01 | (0x06<<16))},			//��������
    {(char *)gMGTitle[MGS_LR],	NULL,			ST_LR,MGS_M21,				4,		(0x00 | (0x01<<16))},			//���ҵ���
    {(char *)gMGTitle[MGS_UD],	NULL,			ST_UD,MGS_M21,				4,		(0x00 | (0x01<<16))},			//���µ���
    {(char *)gMGTitle[MGS_FOCUS],	NULL,			ST_UD,MGS_M21,				4,		(0x00 | (0x01<<16))},			//�۽�����
    {(char *)gMGTitle[MGS_ZOOM],	NULL,		ST_UD,MGS_M21,				4,		(0x00 | (0x01<<16))},			//���ŵ���
    {(char *)gMGTitle[MGS_APERTURE],	NULL,		ST_UD,MGS_M21,				4,		(0x00 | (0x01<<16))},			//��Ȧ����
		{(char *)gMGTitle[MGS_MODE],	NULL,		ST_MODE,MGS_M11,				3,		(0x00 | (0x03<<16))},			//ģʽ�趨
    {(char *)gMGTitle[MGS_TMN_SEL],NULL,     ST_TMN_SEL,MGS_M12,     3},//Ԥ��λ�ն�ѡ��
    {NULL,	                (TMenu*)gMState,	sizeof(gMState)/sizeof(TMenu),	0xFF},
};



#define MAX_MENU_VALUE_NUM  8
TMenuValue gMenuValue[MAX_MENU_VALUE_NUM]=
{
  {1,1,VAL_TEMP_CLOSE},
  {1,2,VAL_CHM_MUSIC},
  {1,3,VAL_CMR_TRACK_EN},
  {1,4,VAL_AUTO_CLOSE},
  {5,0,VAL_CUR_CMR},
  {6,0,VAL_SPKER_LIMIT},
  {7,0,VAL_APPLY_LIMIT},  
  {13,0,VAL_DSCS_MODE},
    
};

#define MODE_TEXT_LEN 8
unsigned char gModeText[][MODE_TEXT_LEN]=
{
  "��ס����",
  "����ģʽ",
  "�Ƚ��ȳ�",
  "����ģʽ",
};
#define MODE_TEXT_E_LEN 10
unsigned char gModeText_E[][MODE_TEXT_E_LEN]=
{
  "PPT MODE  ",
  "LIMIT MODE",
  "FIFO MODE ",
  "APPLY MODE",
};



short gsnCurMGrp;	//��ǰ�Ĳ˵������
int gnaStNum[4];

// ============================================================================
void ChangeMenu(int Mode)
{
  unsigned char   MenuLang;
  ByteDataGet(VAL_MENU_LANG,&MenuLang);
	printf("Mode=%d\n",Mode);
  switch(Mode)
  {
    case PPT_MODE:
      if(CHINESE==MenuLang)
      {
        gMG[1].FirstItem = (TMenu*)gM15;
        gMG[1].Count = sizeof(gM15)/sizeof(TMenu);
      }
      else
      {
        gMG[1].FirstItem = (TMenu*)gM15_E;
        gMG[1].Count = sizeof(gM15_E)/sizeof(TMenu);
      }
      
      break;
    case FIFO_MODE:
      if(CHINESE==MenuLang)
      {
        gMG[1].FirstItem = (TMenu*)gM16;
        gMG[1].Count = sizeof(gM16)/sizeof(TMenu);
      }
      else
      {
        gMG[1].FirstItem = (TMenu*)gM16_E;
        gMG[1].Count = sizeof(gM16_E)/sizeof(TMenu);
      }
      break;
    case APPLY_MODE:
      if(CHINESE==MenuLang)
      {
        gMG[1].FirstItem = (TMenu*)gM14;
        gMG[1].Count = sizeof(gM14)/sizeof(TMenu);
      }
      else
      {
        gMG[1].FirstItem = (TMenu*)gM14_E;
        gMG[1].Count = sizeof(gM14_E)/sizeof(TMenu);
      }
      break;
    case LIMIT_MODE:
      if(CHINESE==MenuLang)
      {
        gMG[1].FirstItem = (TMenu*)gM11;
        gMG[1].Count = sizeof(gM11)/sizeof(TMenu);
      }
      else
      {
        gMG[1].FirstItem = (TMenu*)gM11_E;
        gMG[1].Count = sizeof(gM11_E)/sizeof(TMenu);
      }
      break;
    default:
      break;
  }
}

void GetMenuValue(unsigned char GrpNum,unsigned char LineNum,unsigned int *pValue)
{
	int i;
	for(i=0; i<MAX_MENU_VALUE_NUM; i++)
	{
		if((gMenuValue[i].GrpNum==GrpNum)&&(gMenuValue[i].LineNum==LineNum))
		{
			break;
		}
	}
	if(i<MAX_MENU_VALUE_NUM)
	{
#if 0// yasir change in 2016-4-7
		ByteDataGet(gMenuValue[i].ValueIndex, pValue);
#else
		unsigned char value;
		ByteDataGet(gMenuValue[i].ValueIndex, &value);
		*pValue = (unsigned int)value;
#endif	
	}
}
void SaveMenuValue(unsigned char GrpNum,unsigned char LineNum,unsigned int Value)
{
  unsigned i;
  for(i=0; i<MAX_MENU_VALUE_NUM; i++)
  {
    if((gMenuValue[i].GrpNum==GrpNum)&&(gMenuValue[i].LineNum==LineNum))
    {
      break;
    }
  }
	if(i<MAX_MENU_VALUE_NUM)
	{
#if 0// yasir change in 2016-4-7
		ByteDataSave(gMenuValue[i].ValueIndex, Value);
#else
		ByteDataSave(gMenuValue[i].ValueIndex, (unsigned char)Value);
#endif
	}
}

Bool RenewMenuValue(unsigned char GrpNum,unsigned char LineNum)
{
  unsigned i;
	unsigned char value;
	
  for(i=0; i<MAX_MENU_VALUE_NUM; i++)
  {
    if((gMenuValue[i].GrpNum==GrpNum)&&(gMenuValue[i].LineNum==LineNum))
    {
      break;
    }
  }
  if(i<MAX_MENU_VALUE_NUM)
  {
    if(ST_CMN==gMG[GrpNum].Type) 
    {
			ByteDataGet(gMenuValue[i].ValueIndex,&value);
      if(value)
      {
        gMG[GrpNum].ss.BitState |= (1<<LineNum);
      }
      else
      {
        gMG[GrpNum].ss.BitState &= ~(int)(1<<LineNum);
      }
    }
    else
    {
			ByteDataGet(gMenuValue[i].ValueIndex,&value);
      gMG[GrpNum].Position = (short)value;
    }
    return BOOL_TRUE;
  }
  return BOOL_FALSE;
}
/******************************************************************************
    * ��������send_h_scrollBar
    * 		��������3����,���Ҹ���2���հ׵�(Ӣ��ʱ�ұ�û�пհ�)
    * ��ڲ�����
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾ��ֱ������(���Ĳ˵�ʱ��ʾ,λ�ù̶�)
******************************************************************************/
void send_v_scrollBar(unsigned char x, unsigned char y, unsigned char ucHeight)
{
    send_line_v(x,y+1,ucHeight-2,0);
    send_line_v(x+1,y,ucHeight,0);
    send_line_v(x+2,y+1,ucHeight-2,0);


}
// ============================================================================

/******************************************************************************
    * ��������send_h_StepBar
    * ��ڲ�����
    * ���ڲ�������
    * ����ֵ����
    * ˵������ʾˮƽ������
******************************************************************************/
void send_h_StepBar(unsigned char x, unsigned char y, unsigned char ucWidth,unsigned char BarWidth)
{
    //�����ʾ����
//    clear_block(x,16,119,63);

    //��ʾһ���߿�
//    send_rectangle(x,y,112-x,y+8);
    send_rectangle(x,y,BarWidth-x,y+7);
    y += 1;
    send_line_h(x,y,ucWidth,PIXEL_SET);
    send_line_h(x,y+1,ucWidth,PIXEL_SET);
    send_line_h(x,y+2,ucWidth,PIXEL_SET);
    send_line_h(x,y+3,ucWidth,PIXEL_SET);
    send_line_h(x,y+4,ucWidth,PIXEL_SET);
    send_line_h(x,y+5,ucWidth,PIXEL_SET);
}
// ============================================================================


//�ı���ʾ����
void ChangeLanguage(int nLang)
{
    int i;
    unsigned char Mode;

    ByteDataSave(VAL_MENU_LANG,(unsigned char)nLang);
    if(nLang == CHINESE)
    {
		MI_LENS = MI_C_LENS;
        for(i=0; i<MGS_NUM; i++)
            gMG[i].pTitle = (char *)gMGTitle[i];

        gMG[0].FirstItem = (TMenu*)gMMain;
        gMG[1].FirstItem = (TMenu*)gM11;
        gMG[2].FirstItem = (TMenu*)gM12;
        gMG[3].FirstItem = (TMenu*)gM13;
        gMG[4].FirstItem = (TMenu*)gM21;


    }
    else
    {
		MI_LENS = MI_E_LENS;
        for(i=0; i<MGS_NUM_E; i++)
            gMG[i].pTitle = (char *)gMGTitle_E[i];

        gMG[0].FirstItem = (TMenu*)gMMain_E;
        gMG[1].FirstItem = (TMenu*)gM11_E;
        gMG[2].FirstItem = (TMenu*)gM12_E;
        gMG[3].FirstItem = (TMenu*)gM13_E;
        gMG[4].FirstItem = (TMenu*)gM21_E;

    }
    
    //�����ʾ����
    clear_block(0,0,191,63);
    update_disp_data(0,0,191,63);
    ByteDataGet(VAL_DSCS_MODE,&Mode);
    ChangeMenu((int)Mode);
    //��ʾ���˵�
}
// ============================================================================

//����
//		pMGrp��		��ǰ�˵���ָ��
//		nLine��		��ǰ����Ļ�� ����-1Ϊ��ʾ������⣬(0~MI_LINES)��ʾ����˵���
//		nSeq��		��ǰ�˵��������е���ţ�0~(Count-1)
//		nMode��		��ǰ�˵�����ʾ��ʽ(0=������1=����,2=�հ�)
void DisplayMItem(TMGrpData *pMGrp,int nLine,int nSeq,int nMode)
{
    char caItem[31];	//Ӣ��ʱ��6x10���󣬲˵�����ʾ�������ַ�
    TMenu *pMenu;
    int nState,nMLen;
    unsigned char MenuLang;

    ByteDataGet(VAL_MENU_LANG,&MenuLang);
		memset(caItem,' ',31);	//����ո�
    if(nMode != MD_NULL)	//�����˵�����ʾ
    {
			printf("nSeq=%d\n",nSeq);
			if(pMGrp->FirstItem!=NULL)
			{
				
				
	      pMenu = pMGrp->FirstItem+nSeq;

				//�����ʾ����
	      if(MenuLang == CHINESE)
					nMLen = MI_C_LENS;
	      else
					nMLen = MI_E_LENS;
				memcpy(caItem,pMenu->Text,strlen(pMenu->Text));
				switch(pMenu->Type)		//������ֵ̬	
	      {
					case MT_YESNO:
					case MT_ONOFF:
	            RenewMenuValue(pMGrp-gMG,nSeq);
	            nState = pMGrp->ss.BitState & (1<<nSeq);
	            if(MenuLang == CHINESE)
	            {
	                switch(pMenu->Type)
	                {
	                case MT_YESNO:
	                    if(nState)
	                        memcpy(caItem+MI_LENS-2,"��",2);
	                    else
	                        memcpy(caItem+MI_LENS-2,"��",2);
	                    break;
	                case MT_ONOFF:
	                    if(nState)
	                        memcpy(caItem+MI_LENS-2,"��",2);
	                    else
	                        memcpy(caItem+MI_LENS-2,"��",2);
	                    break;
	                default:
	                    break;
	                }
	            }
	            else
	            {
	                switch(pMenu->Type)
	                {
	                case MT_YESNO:
	                    if(nState)
	                        memcpy(caItem+MI_E_LENS-3,"YES",3);
	                    else
	                        memcpy(caItem+MI_E_LENS-2,"NO",2);
	                    break;
	                case MT_ONOFF:
	                    if(nState)
	                        memcpy(caItem+MI_E_LENS-2,"ON",2);
	                    else
	                        memcpy(caItem+MI_E_LENS-3,"OFF",3);
	                    break;
	                default:
	                    break;
	                }
	            }
	        } 
				}
        if(nLine == -1)//����
        {
            if(MenuLang == CHINESE)
            {
                memcpy(caItem,pMGrp->pTitle,strlen(pMGrp->pTitle));
//				  gstDispMem.ucX1 = (MI_LENS-strlen(pMGrp->pTitle))/2;
//				  gstDispMem.ucX2 = MI_LENS*8;
				gstDispMem.ucX1 = 0;
				gstDispMem.ucX2 = MI_LENS*8-1;
        gstDispMem.ucY1 = 0;
            gstDispMem.ucY2 = gstDispMem.ucY1+16-1;
        	}
            else
            {
                memcpy(caItem,pMGrp->pTitle,strlen(pMGrp->pTitle));
//                gstDispMem.ucX1 = (23-strlen(pMGrp->pTitle))/2;
                gstDispMem.ucX1 = 0;
				gstDispMem.ucX2 = MI_LENS*6-1;
        gstDispMem.ucY1 = 0;
            gstDispMem.ucY2 = gstDispMem.ucY1+10-1;
            }
            
//             }
//             else
//             {
//                 memcpy(caItem,pMGrp->pTitle,MI_E_LENS);
//                 gstDispMem.ucX1 = (MI_E_LENS-sizeof(pMGrp->pTitle))/2;
//                 gstDispMem.ucX2 = MI_LENS*8;
// 				gstDispMem.ucY1 = 0;
// 				gstDispMem.ucY2 = gstDispMem.ucY1+16;
//            }
        }
        else	//ʵ�ʲ˵���
        {
            gstDispMem.ucX1 = 0;

            if(MenuLang == CHINESE)
            {
				gstDispMem.ucY1 = (pMenu->Index%3+1)*16;
                gstDispMem.ucX2 = MI_LENS*8-1;
                gstDispMem.ucY2 = gstDispMem.ucY1+16-1;
            }
            else
            {
				gstDispMem.ucY1 = (pMenu->Index%3+1)*10;
                gstDispMem.ucX2 = MI_LENS*6-1;
                gstDispMem.ucY2 = gstDispMem.ucY1+10-1;
            }
        }
    }
    else	//��ʾ�հ���
    {
        nMode = 0;
        gstDispMem.ucX1 = 0;
        if(MenuLang == CHINESE)
        {
            gstDispMem.ucX2 = MI_LENS*8-1;
            gstDispMem.ucY1 = nLine*16+16;
            gstDispMem.ucY2 = gstDispMem.ucY1+16-1;
        }
        else
        {
            gstDispMem.ucX2 = MI_LENS*6-1;
            gstDispMem.ucY1 = nLine*10+10;
            gstDispMem.ucY2 = gstDispMem.ucY1+10-1;
        }
    }
    // ������ʾ����
    if(MenuLang == CHINESE)
        send_hz16x16(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem,MI_LENS/2, nMode);
    else
        send_zf6x10(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem, 31, nMode);
    update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);

}
// ============================================================================

//��ʾ������(��)���������(��)
void DisplaySBar(int nTotal, int nPosi, int nVert)
{
    unsigned char ucY,ucWidth,ucHeight;
    unsigned char MenuLang;

    ByteDataGet(VAL_MENU_LANG,&MenuLang);
    if(nVert == SB_VERT)//��ֱ
    {
        if(MenuLang == CHINESE)
        {
            gstDispMem.ucX1 = MI_C_LENS*8;
			gstDispMem.ucX2 = gstDispMem.ucX1+8-2;	//��1���㻭�ָ���
            gstDispMem.ucY1 = 16;
            gstDispMem.ucY2 = 63;
            ucHeight = 3*16/nTotal;
            ucY = (48 - ucHeight)/(nTotal-1)*nPosi+16;
        }
        else
        {
            gstDispMem.ucX1 =  MI_E_LENS*6;
            gstDispMem.ucX2 = gstDispMem.ucX1+5;
            gstDispMem.ucY1 = 10;
            gstDispMem.ucY2 = 39;
            ucHeight = 3*10/nTotal;
            ucY = (30 - ucHeight)/(nTotal-1)*nPosi+10;
        }
		if(ucHeight<5)
			ucHeight = 5;
		clear_block(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);

		send_line_h(gstDispMem.ucX1+1,gstDispMem.ucY1,5,0);
		send_line_h(gstDispMem.ucX1+1,gstDispMem.ucY2,5,0);
		send_line_v(gstDispMem.ucX1+1,gstDispMem.ucY1,gstDispMem.ucY2-gstDispMem.ucY1,0);
		send_line_v(gstDispMem.ucX1+5,gstDispMem.ucY1,gstDispMem.ucY2-gstDispMem.ucY1,0);
		
		send_v_scrollBar(gstDispMem.ucX1+2,ucY,ucHeight);

    }
    else//ˮƽ
    {


    }
	update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);

}
// ============================================================================

//��ʾ���ȡ�ֵ
void DisplayVal(short snMGSeq, int nTotal, int nPosi, int nVert)
{
    unsigned char ucAscii[3];
    char ucNumTab[]="0123456789";
    unsigned char x,y;
    unsigned char MenuLang;

    ByteDataGet(VAL_MENU_LANG,&MenuLang);
    if(ENGLISH==MenuLang)
    {
      clear_block(0,10,191,39);
      send_h_StepBar(0,24,191*nPosi/nTotal,191);
    }
    else
    {
      clear_block(0,16,119,63);
      send_h_StepBar(0,32,112*nPosi/nTotal,112);
    }

    if(ENGLISH==MenuLang)
    {
      x=0;
      y=16;
    }
    else
    {
      x=0;
      y=24;
    }
    ucAscii[0] = ucNumTab[gMG[snMGSeq].ss.mm.Min/100];
    ucAscii[1] = ucNumTab[gMG[snMGSeq].ss.mm.Min/10%10];
    ucAscii[2] = ucNumTab[gMG[snMGSeq].ss.mm.Min%10];
    if(gMG[snMGSeq].ss.mm.Min <= 9)
        send_zf5x8(x,y,ucAscii+2,1,0);
    else if(gMG[snMGSeq].ss.mm.Min <= 99)
        send_zf5x8(x,y,ucAscii+1,2,0);
    else
        send_zf5x8(x,y,ucAscii,3,0);

    if(ENGLISH==MenuLang)
    {
      x=176;
      y=16;
    }
    else
    {
      x=97;
      y=24;
    }
    ucAscii[0] = ucNumTab[nTotal/100];
    ucAscii[1] = ucNumTab[nTotal/10%10];
    ucAscii[2] = ucNumTab[nTotal%10];
    if(nTotal <= 9)
        send_zf5x8(x+10,y,ucAscii+2,1,0);
    else if(nTotal <= 99)
        send_zf5x8(x+5,y,ucAscii+1,2,0);
    else
        send_zf5x8(x,y,ucAscii,3,0);

    if(ENGLISH==MenuLang)
    {
      x=95;
      y=32;
    }
    else
    {
      x=48;
      y=48;
    }
    ucAscii[0] = ucNumTab[nPosi/100];
    ucAscii[1] = ucNumTab[nPosi/10%10];
    ucAscii[2] = ucNumTab[nPosi%10];
    if(nPosi <= 9)
        send_zf5x8(x+5,y,ucAscii+2,1,0);
    else if(nPosi <= 99)
        send_zf5x8(x+3,y,ucAscii+1,2,0);
    else
        send_zf5x8(x,y,ucAscii,3,0);


     if(ENGLISH==MenuLang)
    {
      gstDispMem.ucX1 = 0;
      gstDispMem.ucX2 = 191;
      gstDispMem.ucY1 = 10;
      gstDispMem.ucY2 = 39;
    }
    else
    {
      gstDispMem.ucX1 = 0;
      gstDispMem.ucX2 = 118;
      gstDispMem.ucY1 = 16;
      gstDispMem.ucY2 = 63;
    }
    update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);

}
unsigned char GetDiscussModeText(int Mode,unsigned char *pText,int Language)
{
  unsigned char Len=0;
  if(ENGLISH==Language)
  {
    memcpy(pText,gModeText_E[Mode],MODE_TEXT_E_LEN);
    return (MODE_TEXT_E_LEN);
  }
  else
  {
    memcpy(pText,gModeText[Mode],MODE_TEXT_LEN);
    return (MODE_TEXT_LEN);
  }
  
}

void ShowMode(unsigned char Mode)
{
  unsigned char ModeInfoBuf[12];
  unsigned char Len;
  unsigned char MenuLang;
	printf("ShowMode(%d)\n",Mode);
  ByteDataGet(VAL_MENU_LANG,&MenuLang);
  memset(ModeInfoBuf,0,12);
  Len=GetDiscussModeText(Mode,ModeInfoBuf,MenuLang);
  if(ENGLISH==MenuLang)
  {
    send_zf6x10(0,20,(unsigned char *)ModeInfoBuf,Len,0);
    update_disp_data(0,20,191,29);
  }
  else
  {
    send_hz16x16(0,32,(unsigned char *)ModeInfoBuf,Len/2,0);
    update_disp_data(0,32,95,47);
  }
  
}

void ShowSelTmn(unsigned short Addr)
{
  unsigned char ShowInfoBuf[10];
  unsigned char AddrBuf[3];
  unsigned char MenuLang;

  ByteDataGet(VAL_MENU_LANG,&MenuLang);
  memset(ShowInfoBuf,' ',12);

  memset(AddrBuf,' ',3);
  if((Addr/100) > 0)
  {
    AddrBuf[0]=Addr/100+'0';
  }
  if((((Addr%100)/10) > 0)||((Addr/100) > 0))
  {
    AddrBuf[1]=((Addr%100)/10)+'0';
  }
  AddrBuf[2]=Addr%10+'0';

  if(ENGLISH==MenuLang)
  {
    if(Addr<BACKUP_FULL_VIEW_ADDR)
    {
      memcpy(ShowInfoBuf,"Terminal ",10);
    }
		else if(Addr==BACKUP_FULL_VIEW_ADDR)
		{
			memcpy(ShowInfoBuf,"Backup Full View ",17);
      memset(AddrBuf,' ',3);
		}
    else
    {
      memcpy(ShowInfoBuf,"Full View ",10);
      memset(AddrBuf,' ',3);
    }
    send_zf6x10(0,20,(unsigned char *)ShowInfoBuf,10,0);
    update_disp_data(0,20,78,30);
    send_zf6x10(60,20,(unsigned char *)AddrBuf,3,0);
    update_disp_data(0,20,78,30);
  }
  else
  {
    if(Addr<BACKUP_FULL_VIEW_ADDR)
    {
      memcpy(ShowInfoBuf,"�ն�",4);
    }
		else if(Addr==BACKUP_FULL_VIEW_ADDR)
		{
			memcpy(ShowInfoBuf,"ȫ��",4);
      memset(AddrBuf,' ',3);
			AddrBuf[2]='1';
		}
    else
    {
      memcpy(ShowInfoBuf,"ȫ��",4);
      memset(AddrBuf,' ',3);
    }
    send_hz16x16(0,32,(unsigned char *)ShowInfoBuf,8/2,0);
    update_disp_data(0,32,72,48);
    send_zf8x16(48,32,(unsigned char *)AddrBuf,3,0);
    update_disp_data(0,32,72,48);
  }
}
// ============================================================================

//�˵������������µ����ֵ
//	��ת�������ж�ʱ���øú���
//		snMGSeq��	��ǰ�˵������ţ�gMG�����±꣩
//		nInc��		��ǰ������������  ����=-1������=1���������¼��˵�����ʱ��=0
short MenuScroll(short snMGSeq,int nInc)
{
  int nOLine,nCur,nFirst;
  char Temp[2];
  unsigned char Index;
  TMenu *pMenu;
  unsigned char MenuLang;

  ByteDataGet(VAL_MENU_LANG,&MenuLang);
  if(gMG[snMGSeq].Type == ST_TMN_SEL)
  {
    Index = gMG[snMGSeq].Count;
    pMenu = gMG[Index].FirstItem + gMG[Index].Position;
    if(pMenu->fpMenuDo!=NULL)
    {
      pMenu->fpMenuDo(nInc);
    }    
  }
  if(gMG[snMGSeq].Type == ST_MODE)
  {
    gMG[snMGSeq].Position += nInc;
		if(gMG[snMGSeq].Position > gMG[snMGSeq].ss.mm.Max)
			gMG[snMGSeq].Position = gMG[snMGSeq].ss.mm.Max;
		else if(gMG[snMGSeq].Position < gMG[snMGSeq].ss.mm.Min)
			gMG[snMGSeq].Position = gMG[snMGSeq].ss.mm.Min;
//    Index = gMG[snMGSeq].Count;
//    pMenu = gMG[Index].FirstItem + gMG[Index].Position;
//    if(pMenu->fpMenuDo!=NULL)
//    {
//      pMenu->fpMenuDo(gMG[snMGSeq].Position);
//    }    
    ShowMode(gMG[snMGSeq].Position);
//    DisplayOneState(0,gMG[snMGSeq].Position);
  }
	else if(gMG[snMGSeq].Type == ST_VALUE)		//��ʾֵ
    {
        gMG[snMGSeq].Position += nInc;
        if(gMG[snMGSeq].Position > gMG[snMGSeq].ss.mm.Max)
            gMG[snMGSeq].Position = gMG[snMGSeq].ss.mm.Max;
        else if(gMG[snMGSeq].Position < gMG[snMGSeq].ss.mm.Min)
            gMG[snMGSeq].Position = gMG[snMGSeq].ss.mm.Min;
        Index = gMG[snMGSeq].Count;
        pMenu = gMG[Index].FirstItem + gMG[Index].Position;
        if(pMenu->fpMenuDo!=NULL)
        {
            pMenu->fpMenuDo(gMG[snMGSeq].Position);
        }
        DisplayVal(snMGSeq, gMG[snMGSeq].ss.mm.Max-gMG[snMGSeq].ss.mm.Min+1,gMG[snMGSeq].Position,SB_HORZ);
    }
    else if(gMG[snMGSeq].Type == ST_CMN)		//��ʾ�˵�����
    {
        nOLine = gMG[snMGSeq].Position;
        gMG[snMGSeq].Position = (nOLine+gMG[snMGSeq].Count+nInc)% gMG[snMGSeq].Count;
        if((nOLine / MI_LINES)!=(gMG[snMGSeq].Position / MI_LINES)		//��ͬ��
                ||(nOLine == gMG[snMGSeq].Position))					//���¼�����
        {																//����ˢ��
            nFirst = (gMG[snMGSeq].Position / MI_LINES)*MI_LINES;
            for(nCur = 0; nCur < MI_LINES; nCur++)
            {
                if(nFirst+nCur >= gMG[snMGSeq].Count)
                    DisplayMItem(gMG+snMGSeq,nCur,nFirst+nCur,MD_NULL);	//��ʾ����
                else
                {
                    if(nFirst+nCur == gMG[snMGSeq].Position)
                        DisplayMItem(gMG+snMGSeq,nCur,nFirst+nCur,MD_REVERSE);	//����(��ǰ��)
                    else
                        DisplayMItem(gMG+snMGSeq,nCur,nFirst+nCur,MD_NORMAL);	//������ʾ
                }
            }
        }
        else		//ֻˢ����ص�����
        {
            DisplayMItem(gMG+snMGSeq,nOLine%MI_LINES,nOLine,MD_NORMAL);
            DisplayMItem(gMG+snMGSeq,gMG[snMGSeq].Position%MI_LINES,gMG[snMGSeq].Position,MD_REVERSE);
        }
        DisplaySBar(gMG[snMGSeq].Count,gMG[snMGSeq].Position,SB_VERT);
    }
    else if(gMG[snMGSeq].Type == ST_LR)		//����
    {
        if(nInc==-1)
        {
            memcpy(Temp,"��",2);
        }
        else
        {
            memcpy(Temp,"��",2);
        }
        if(ENGLISH==MenuLang)
        {
          send_hz16x16(90,16,(unsigned char *)&Temp, 2, 0);
          update_disp_data(90,16,114,31);
        }
        else
        {
          send_hz16x16(52,32,(unsigned char *)&Temp, 2, 0);
          update_disp_data(52,32,67,47);
        }
        Index = gMG[snMGSeq].Count;
        pMenu = gMG[Index].FirstItem + gMG[Index].Position;
        if(pMenu->fpMenuDo!=NULL)
        {
            pMenu->fpMenuDo((nInc==-1)?0:1);
        }
    }
    else if(gMG[snMGSeq].Type == ST_UD)		//����
    {
        if(nInc==-1)
        {
            memcpy(Temp,"��",2);
        }
        else
        {
            memcpy(Temp,"��",2);
        }
        if(ENGLISH==MenuLang)
        {
          send_hz16x16(90,16,(unsigned char *)&Temp, 2, 0);
          update_disp_data(90,16,114,31);
        }
        else
        {
          send_hz16x16(52,32,(unsigned char *)&Temp, 2, 0);
          update_disp_data(52,32,67,47);
        }
        Index = gMG[snMGSeq].Count;
        pMenu = gMG[Index].FirstItem + gMG[Index].Position;
        if(pMenu->fpMenuDo!=NULL)
        {
            pMenu->fpMenuDo((nInc==-1)?0:1);
        }
    }
    else
    {
		
    }
    return gMG[snMGSeq].Position;
}
// ============================================================================

int IntToRightStr(unsigned char caItem[],int nNum,int nLast)
{
	while(nNum> 0)
	{
		caItem[nLast]= nNum%10+0x30;
		nLast--;
		nNum /= 10;
	}
  caItem[nLast--]=' ';
  caItem[nLast--]=' ';

	return nLast;
}
// ============================================================================


// ��ʾ��ǰ����״̬
//	����:
//  	��һ��:��Ԫ xxx
//	  	�ڶ���:����ģʽ
//	 	������:���� xxx
//	  	������:(ǩ��/��� xxx)����ͷ x
//	Ӣ��:
//
//	
//	��ʾ��ǰ״̬
//	����
//		nStateBit: bit0~4�ֱ�����һ�����ĸ�״̬��Ϣ
//		nStNum[]:  �ֱ��Ӧ����״̬�ĵ�ǰ�����Ӧ�±�
//void DisplayOneState(TMGrpData *pMGrp, int nSeq)
//void DisplayOneState(TMGrpData *pMGrp, int nSeq ,int nLine,int nNum)
void DisplayAllState(int nStateBit,int nStNum[])
{
    unsigned char caItem[16];		//����ʱ��Ҫ8���ַ���Ӣ��ʱ��Ҫ16
    int Temp;
    TMenu *pMenu;
    int nState;
    unsigned char ucLine,ucMode;
    unsigned char Len;

	int i;
	
  unsigned char MenuLang;

  ByteDataGet(VAL_MENU_LANG,&MenuLang);

//     pMenu = pMGrp->FirstItem+nSeq;
//     nState = pMGrp->ss.BitState & (1<<nSeq);
//
//     switch(pMenu->Type)
//     {
//         case MT_YESNO:
//             if(gsnCurMGrp == 1)
//             {
//                 ucLine = 0;
//                 memcpy(caItem,pMenu->Text,strlen(pMenu->Text));
//             }
//             else if(gsnCurMGrp == 4)
//             {
//                 ucLine = 4;
//                 memcpy(caItem,pMenu->Text,strlen(pMenu->Text));
//             }
//             break;
//         case MT_ONOFF:
//             if(gsnCurMGrp == 1)
//             {
//                 ucLine = 2;
//                 memcpy(caItem,pMenu->Text,strlen(pMenu->Text));
//             }
//             break;
//         default:
//           break;
// 	}
    ChangeMenu(nStNum[0]);

    // ������ʾ����
    if(MenuLang == CHINESE)
    {
        gstDispMem.ucX1 = 128;
        gstDispMem.ucX2 = 191;
 		for(i=0;i<4;i++)
		{
			gstDispMem.ucY1 = i*8*2;
			gstDispMem.ucY2 = (i+1)*8*2-1;
			switch(nStateBit & (1<<i))
			{
			case STD_MODE:
        Len=GetDiscussModeText(nStNum[0],caItem,MenuLang);
			  send_hz16x16(gstDispMem.ucX1,i*2*8,caItem, Len/2, 0);
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
				break;
      case STD_CAM:
				if(GetCmrTrack())
				{
					disp_ico(gstDispMem.ucX1,i*2*8,16,16,ico_data[ICO_CMR],0);
				}
				else
				{
        	disp_ico(gstDispMem.ucX1,i*2*8,16,16,ico_data[ICO_CMR_LOCK],0);
				}
				if(nStNum[1]>0)
				{
					Temp=IntToRightStr(caItem,nStNum[1],7);
					send_zf8x16(gstDispMem.ucX1+8*Temp+8,i*2*8,caItem+Temp+1, 7-Temp, 0);
				}
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
				break;
			case STD_LIMIT:
        disp_ico(gstDispMem.ucX1,i*2*8,16,16,ico_data[ICO_SPKER_LIMIT],0);
        if((FIFO_MODE==nStNum[0])||(LIMIT_MODE==nStNum[0]))
        {
  				if(nStNum[2]>0)
  				{
  					Temp=IntToRightStr(caItem,nStNum[2],7);
  					send_zf8x16(gstDispMem.ucX1+8*Temp+8,i*2*8,caItem+Temp+1, 7-Temp, 0);
  				}
        }
        else
        {
          disp_ico(gstDispMem.ucX1+6*8,i*2*8,16,16,ico_data[ICO_PROHIBIT],0);
        }
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
        break;
			case STD_APPLY:
        disp_ico(gstDispMem.ucX1,i*2*8,16,16,ico_data[ICO_APPLY_LIMIT],0);
        if((APPLY_MODE==nStNum[0])||(LIMIT_MODE==nStNum[0]))
        {
  				if(nStNum[3]>0)
  				{
  					Temp=IntToRightStr(caItem,nStNum[3],7);
  					send_zf8x16(gstDispMem.ucX1+8*Temp+8,i*2*8,caItem+Temp+1, 7-Temp, 0);
  				}
        }
        else
        {
          disp_ico(gstDispMem.ucX1+6*8,i*2*8,16,16,ico_data[ICO_PROHIBIT],0);
        }
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);        
        break;
			}		
			
			
			
		}

    }
    else		//English
    {
 		for(i=0;i<4;i++)
		{
			memset(caItem,' ',16);
			
			gstDispMem.ucX1 = (i%2)*96;
			gstDispMem.ucX2 = gstDispMem.ucX1+96-1;
			gstDispMem.ucY1 = (i/2)*10+44;
			gstDispMem.ucY2 =gstDispMem.ucY1+10-1;
			switch(nStateBit & (1<<i))
			{
			case STD_MODE:
        Len=GetDiscussModeText(nStNum[0],caItem,MenuLang);
			  send_zf6x10(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem, Len, 0);
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
				break;
      case STD_CAM:
				memcpy(caItem,"Curr Cam:",9);
				if(nStNum[1]>0)
				{
					Temp=IntToRightStr(caItem,nStNum[1],12);
//					send_zf6x10(gstDispMem.ucX1+8*Temp+8,i*2*8,caItem+Temp+1, 12-Temp, 0);
				}
        send_zf6x10(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem, 16, 0);
			  update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
				break;
			case STD_LIMIT:
        if((FIFO_MODE==nStNum[0])||(LIMIT_MODE==nStNum[0]))
        {
  				memcpy(caItem,"MaxSpeakers:",13);
  				if(nStNum[2]>0)
  				{
  					Temp=IntToRightStr(caItem,nStNum[2],12);
  				}
          send_zf6x10(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem, 16, 0);
        }
         else
        {
          clear_block(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
        }
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
        break;
			case STD_APPLY:
        if((APPLY_MODE==nStNum[0])||(LIMIT_MODE==nStNum[0]))
        {
  				memcpy(caItem,"MaxApplicants:",13);
  				if(nStNum[3]>0)
  				{
  					Temp=IntToRightStr(caItem,nStNum[3],12);
  				}
          send_zf6x10(gstDispMem.ucX1,gstDispMem.ucY1,(unsigned char *)caItem, 16, 0);
  			  
        }
        else
        {
          clear_block(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
        }
        update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
        break;
			}			
 		}
    }

//		update_disp_data(gstDispMem.ucX1,gstDispMem.ucY1,gstDispMem.ucX2,gstDispMem.ucY2);
}

void DisplayOneState(int Line, int Value)
{
  if((Line<0)||(Line>3))
  {
    return;
  }
  gnaStNum[Line]=Value;
  if(!Line)
  {
    DisplayAllState(0x0D,gnaStNum);
  }
  else
  {
    DisplayAllState(1<<Line,gnaStNum);
  }

  
}
// ============================================================================


//��ʾһ���˵���
void DisplayGroup(short snMGSeq)
{
  unsigned char MenuLang;
  unsigned char Temp;

  ByteDataGet(VAL_MENU_LANG,&MenuLang);
  
  DisplayMItem(gMG+snMGSeq,-1,0,MD_NORMAL);	//�����

  MenuScroll(snMGSeq,0);	//��ʾ��ǰλ�õĲ˵�����

	if(MenuLang == CHINESE)
	{
		send_line_v(119,0,64,0);
        update_disp_data(119,0,119,63);		
	}
	else
	{
		send_line_h(0,41,192,0);
        update_disp_data(0,41,191,41);		
	}
  
  ByteDataGet(VAL_DSCS_MODE,&Temp);
  gnaStNum[0]=Temp;
  ByteDataGet(VAL_CUR_CMR,&Temp);
  gnaStNum[1]=Temp;
  ByteDataGet(VAL_SPKER_LIMIT,&Temp);
  gnaStNum[2]=Temp;
  ByteDataGet(VAL_APPLY_LIMIT,&Temp);
  gnaStNum[3]=Temp;
	DisplayAllState(0x0F,gnaStNum);
}


// ============================================================================


//ѡ�в˵���
//	����Ϊ��ǰ�˵���
void ItemSelected(short snMGSeq)
{
    TMenu *pMenu;
    short usNew;
    unsigned char Index;
    int i;
    unsigned char MenuLang;

    
    ByteDataGet(VAL_MENU_LANG,&MenuLang);
    usNew = snMGSeq;
	  if(gMG[snMGSeq].Type==ST_CMN)	//��ͨ�˵���
    {
        pMenu = gMG[snMGSeq].FirstItem + gMG[snMGSeq].Position;
        if(pMenu->Type == MT_MENU)
        {
          if(pMenu->Link!=MGS_NONE)
          {
            usNew = pMenu->Link;
            if(gMG[snMGSeq].Level > gMG[usNew].Level)
                gMG[snMGSeq].Position = 0;

          }
          if(pMenu->fpMenuDo != NULL)
              pMenu->fpMenuDo(NULL);
        }
        else if(pMenu->Type == MT_SWITCH)	//�����л�
        {
          if(MenuLang == CHINESE)
              ChangeLanguage(ENGLISH);
          else
              ChangeLanguage(CHINESE);

          usNew = 0;
          gMG[usNew].Position = 0;
			
        }
        else	if((pMenu->Type==MT_YESNO)||(pMenu->Type==MT_ONOFF)) //����ת��ֱ�Ӹı�ֵ
        {
          gMG[snMGSeq].ss.BitState ^= (1<<gMG[snMGSeq].Position);
          if(pMenu->fpMenuDo != NULL)
              pMenu->fpMenuDo(gMG[snMGSeq].ss.BitState&(1<<gMG[snMGSeq].Position));
          DisplayMItem(gMG+snMGSeq,gMG[snMGSeq].Position%3,gMG[snMGSeq].Position,MD_REVERSE);
            
            return;
        }
        else
        {
          // ��¼��ǰҳ
          if((gsnCurMGrp==0)&&(pMenu->Link!=0))
          {
            UseDisSet(CPT_USE,BOOL_TRUE);
          }
          gsnCurMGrp = pMenu->Link;
					//printf("gsnCurMGrp=%d,gMG[gsnCurMGrp].Position=%d\n",gsnCurMGrp,gMG[gsnCurMGrp].Position);
          DisplayMItem(gMG+gsnCurMGrp,-1,gMG[gsnCurMGrp].Position,MD_NORMAL);
          if(ENGLISH==MenuLang)
          {
            clear_block(0,10,191,39);
            update_disp_data(0,10,191,39);
          }
          else
          {
            clear_block(0,16,118,63);
            update_disp_data(0,16,118,63);
          }
          RenewMenuValue(pMenu->Link,0);
          MenuScroll(pMenu->Link,0);
          return;
        }
        
    }
    else
    {
      usNew = gMG[snMGSeq].Count;
			//printf("snMGSeq=%d,usNew=%d\n",snMGSeq,usNew);
      if(gMG[snMGSeq].Type==ST_VALUE)
      {
        SendMainState(0,NULL,0);
      }
      else if(gMG[snMGSeq].Type==ST_MODE)
      {
        pMenu = gMG[usNew].FirstItem + gMG[usNew].Position;
        if(pMenu->fpMenuDo!=NULL)
        {
          pMenu->fpMenuDo(gMG[snMGSeq].Position);
        }    
        DisplayOneState(0,gMG[snMGSeq].Position);
        SendMainState(0,NULL,0);
      }
    }
    DisplayGroup(usNew);


    // ��ʾ��ǰҳ
//    DisplayGroup(usNew);

    // ��¼��ǰҳ
    if((usNew==0)&&(gsnCurMGrp>0))
    {
      UseDisSet(CPT_USE,BOOL_FALSE);
    }
    else if((gsnCurMGrp==0)&&(usNew>0))
    {
      UseDisSet(CPT_USE,BOOL_TRUE);
    }
    gsnCurMGrp = usNew;
}
Bool CtrlMenuSw(short snMGSeq)
{
  ItemSelected(snMGSeq);
}
extern unsigned char gByteData[PAR_NUM];// yasir change in 2016-4-7
void MenuInit(void)
{
  int i;
  unsigned char MenuLang;

  ByteDataGet(VAL_MENU_LANG,&MenuLang);
      
  if(MenuLang!=ENGLISH)
  {
    MenuLang = CHINESE;
    ByteDataSave(VAL_MENU_LANG,MenuLang);
  }
  else
  {
    ChangeLanguage(ENGLISH);
  }
	gPresetTmnSelPro.RunFlag = 0;
	gPresetTmnSelPro.Addr = FULL_VIEW_ADDR;
	gsnCurMGrp=0;
	ByteDataInit();
	sleep(1);
	clear_block(0,0,192,64);
  update_disp_data(0,0,192,64);
	DisplayGroup(gsnCurMGrp);
#if 0 // yasir change in 2016-4-7
  DisplayOneState(0,PPT_MODE);
#else
  DisplayOneState(0,gByteData[VAL_DSCS_MODE]);
#endif
	sleep(1);
}
// ============================================================================
