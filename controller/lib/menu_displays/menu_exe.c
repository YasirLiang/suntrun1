#include <stdio.h>
#include "menu_exe.h"
#include "menu_f.h"
#include "menu_func.h"
#include "host_controller_debug.h"// yasir add in 2016-3-29

unsigned char  gUseDis;
unsigned char gByteData[PAR_NUM];

Bool ByteDataGet(unsigned char Index, unsigned char *pValue);// yasir add in 2016-3-29
Bool ByteDataSave(unsigned char Index, unsigned char Value);// yasir add in 2016-3-29
Bool UseDisSet(unsigned char Use,Bool Set);// yasir add in 2016-3-29

void EnterCmrPreset(unsigned int value)
{
	printf("EnterCmrPreset(%d)\n",value);
	menu_cmd_run( MENU_UI_ENTERESCPRESET, 1,NULL );
}

void EscCmrPreset(unsigned int value)
{
	printf("EscCmrPreset(%d)\n",value); 
	menu_cmd_run( MENU_UI_ENTERESCPRESET, 0,NULL );
}

//遥控器对码
void RCtrlAlign(unsigned int value)
{
	printf("RCtrlAlign(%d)\n",value); 
	menu_cmd_run( MENU_UI_DISTANCECTL, value,NULL );
}

void SpkLmtSet(unsigned int value)
{
	SaveMenuValue(6,0,value);
	DisplayOneState(2,value);
	printf("SpkLmtSet(%d)\n",value); 
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}

void ApplyLmtSet(unsigned int value)
{
	SaveMenuValue(7,0,value);
	DisplayOneState(3,value);
	printf("ApplyLmtSet(%d)\n",value);
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}
void SwitchCamera(unsigned int value)
{
	printf("SwitchCamera(%d)\n",value);
	unsigned char sa = (unsigned char)value;
	ByteDataSave( VAL_CUR_CMR, sa );
	menu_cmd_run( MENU_UI_SWITCHCMR, value,NULL );
}

void SavePreset(unsigned int value)
{
	printf("SavePreset(%d)\n",value);
	menu_cmd_run( MENU_UI_PRESETSAVE, value,NULL );
}

void ClearPreset(unsigned int value)
{
	printf("ClearPreset(%d)\n",value);
	menu_cmd_run( MENU_UI_CLEARPRESET, value,NULL );
}

void CameraLR(unsigned int value)
{
	printf("CameraLR(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLLEFTRIGHT, value,NULL );
}


void CameraUD(unsigned int value)
{
	printf("CameraUD(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLUPDOWN, value,NULL );
}


void CameraFouce(unsigned int value)
{
	printf("CameraFouce(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLFOUCE, value,NULL );
}

void CameraZoom(unsigned int value)
{
	printf("CameraZoom(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLZOOM, value,NULL );
}


void CameraAperture(unsigned int value)
{
	printf("CameraAperture(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLIRIS, value,NULL );
}

unsigned short FindTmnAddr(unsigned short Addr, Bool NextFlag)// 未完成 2016-3-29
{
	printf("CameraAperture(Addr=0x%x,NextFlag=%d)\n",Addr,NextFlag);
	//menu_cmd_run( MENU_UI_SELECTPRESETADDR, Addr,NULL );

	return 0;
}

TPresetTmnSelPro gPresetTmnSelPro;
void PresetTmnSel(unsigned int value)// 未完成 2016-3-29
{
	unsigned short Addr;

	if(gPresetTmnSelPro.Addr>FULL_VIEW_ADDR)
	{
		Addr=FULL_VIEW_ADDR;
	}
	else if(!value)
	{
		Addr = gPresetTmnSelPro.Addr;
	}
	else if(1==value)
	{
		if(gPresetTmnSelPro.Addr<BACKUP_FULL_VIEW_ADDR)
		{
			Addr = FindTmnAddr(gPresetTmnSelPro.Addr,TRUE);
			if(0xFFFF==Addr)
			{
				Addr=BACKUP_FULL_VIEW_ADDR;
			}
		}
		else
		{
			Addr=FULL_VIEW_ADDR;
		}
	}
	else if(-1==value)
	{
		if(gPresetTmnSelPro.Addr==FULL_VIEW_ADDR)
		{
			Addr=BACKUP_FULL_VIEW_ADDR;
		}
		else
		{
			Addr = FindTmnAddr(gPresetTmnSelPro.Addr,FALSE);
		}
	}

	ShowSelTmn(Addr);
	gPresetTmnSelPro.Addr = Addr;
	//gPresetTmnSelPro.Time=gu32SysTick;
	//gPresetTmnSelPro.RunFlag = TRUE;
	printf("PresetTmnSel(value=%d,Addr=0x%x)\n",value,Addr);
}

#if 0
void PresetTmnSelPro(void)
{
	if(gPresetTmnSelPro.RunFlag)
	{
		if(gu32SysTick-gPresetTmnSelPro.Time>500)
		{
			FindFuncId(SYS_USE,SYS_PRESET_ADDR,0,(unsigned char *)&gPresetTmnSelPro.Addr,2);
			gPresetTmnSelPro.RunFlag = FALSE;
		}
	}
}
#endif

/*
#define PPT_MODE		0
#define	LIMIT_MODE	1
#define	FIFO_MODE 	2
#define	APPLY_MODE	3
*/
void ModeSet(unsigned int value)
{
	unsigned Mode;
	printf("ModeSet(%d)\n",value);
	Mode = value;
	SaveMenuValue(13,0,value);
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}

void ChmPriorEnSet(unsigned int value)// 这里是临时关闭，注:yasir 2016-3-29
{
	unsigned PriorEn;
	PriorEn = value?1:0;
	SaveMenuValue(1,1,value);
	printf("ChmPriorEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_TEMPCLOSE, value,NULL );
}
void ChmMusicEnSet(unsigned int value)
{
	unsigned MusicEn;
	MusicEn = value?1:0;
	SaveMenuValue(1,2,value);
	printf("ChmMusicEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_CHAIRMANHINT, value,NULL );
}
void AutoOffEnSet(unsigned int value)
{
	unsigned AutoOffEn;
	AutoOffEn = value?1:0;
	SaveMenuValue(1,4,value);
	printf("AutoOffEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_AUTOCLOSE, value,NULL );
}
void CmrTrackSet(unsigned int value)
{
	unsigned int CmrNum;
	SaveMenuValue(1,3,value);
	GetMenuValue(5,0,&CmrNum);
	printf("CmrTrackSet(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERATRACK, value,NULL );
}
int GetCmrTrack(void)
{
	unsigned int value;
	GetMenuValue(1,3,&value);
	printf("GetCmrTrack,value=%d\n",value);
	return (value?1:0);
}

void TerminalReAllotAddr(unsigned int value)
{
	printf("TerminalReAllotAddr(%d)\n",value);
	menu_cmd_run( MENU_UI_REALLOT, value,NULL );
}
void TerminalAddAllotAddr(unsigned int value)
{
	printf("TerminalAddAllotAddr(%d)\n",value);
	menu_cmd_run( MENU_UI_NEWALLOT, value, NULL );
}
void TerminalAllotAddrOver(unsigned int value)
{
	printf("TerminalAllotAddrOver(%d)\n",value);
	menu_cmd_run( MENU_UI_SETFINISH, value, NULL );
}

void ByteDataInit(void)// yasir chang in 2016-3-29
{
	unsigned char  get_byte = 0;
	ByteDataGet( VAL_TEMP_CLOSE, &get_byte );
	ByteDataGet( VAL_CHM_MUSIC, &get_byte );
	ByteDataGet( VAL_CMR_TRACK_EN, &get_byte );
	ByteDataGet( VAL_AUTO_CLOSE, &get_byte );
	ByteDataGet( VAL_CUR_CMR, &get_byte );
	ByteDataGet( VAL_SPKER_LIMIT, &get_byte );
	ByteDataGet( VAL_APPLY_LIMIT, &get_byte );
	ByteDataGet( VAL_DSCS_MODE, &get_byte );
	ByteDataGet( VAL_MENU_LANG, &get_byte );
}

Bool ByteDataGet(unsigned char Index, unsigned char *pValue)
{
	printf("ByteDataGet(Index=%d)",Index);
	if(Index<PAR_NUM)
	{
		menu_cmd_run( MENU_UI_GET_PARAM, Index, &gByteData[Index] );
		*pValue=gByteData[Index];
		printf("value = %d\n", *pValue );
		return TRUE;
	}
	
	return FALSE;
}

Bool ByteDataSave(unsigned char Index, unsigned char Value)
{
	printf("ByteDataSave(Index=%d)\n",Index);
	if(Index<PAR_NUM)
	{
		gByteData[Index]=Value;
		menu_cmd_run( MENU_UI_SAVE_PARAM, Index, &gByteData[Index] );
		return TRUE;
	}

	return FALSE;
}

Bool UseDisSet(unsigned char Use,Bool Set)
{
	printf("UseDisSet(Use=%d,Set=%d)\n",Use,Set);
	if(!(Use&(TMN_USE|MENU_USE|SYS_USE|CPT_USE)))
	{
		return FALSE;
	}
	
	if(Set)
	{
		gUseDis |= Use;
	}
	else
	{
		gUseDis &= (~Use);
	}
	
	return TRUE;
}

int SendMainState(short int snCmd, void *pData, unsigned long ulLen)
{
	printf("SendMainState(snCmd=%d)\n",snCmd);
	menu_cmd_run( MENU_UI_SEND_MAIN_STATE, 0, NULL );

	return 0;
}

