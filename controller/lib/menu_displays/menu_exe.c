#include <stdio.h>
#include "wireless.h"
#include "menu_exe.h"
#include "menu_f.h"
#include "menu_func.h"
#include "host_controller_debug.h"// yasir add in 2016-3-29
#include "host_time.h"

#ifdef __DEBUG__
//#define __MENU_EXE_DEBUG__
#endif

#ifdef __MENU_EXE_DEBUG__
#define printf_MENU_EXE( fmt, args...) \
	printf( ""fmt"", ##args )
#else 
#define printf_MENU_EXE( fmt, args...)
#endif

unsigned char  gUseDis;
unsigned char gByteData[PAR_NUM];

Bool ByteDataGet(unsigned char Index, unsigned char *pValue);// yasir add in 2016-3-29
Bool ByteDataSave(unsigned char Index, unsigned char Value);// yasir add in 2016-3-29
Bool UseDisSet(unsigned char Use,Bool Set);// yasir add in 2016-3-29
void EnterCmrPreset(unsigned int value)
{
	printf_MENU_EXE("EnterCmrPreset(%d)\n",value); 
	menu_cmd_run( MENU_UI_ENTERESCPRESET, 1,NULL );
}
void EscCmrPreset(unsigned int value)
{
	printf_MENU_EXE("EscCmrPreset(%d)\n",value); 
	menu_cmd_run( MENU_UI_ENTERESCPRESET, 0,NULL );
}
  //遥控器对码

void RCtrlAlign(unsigned int value)
{
	int Temp;
	Temp = (value?1:0);
	RCtrlAlignEn(Temp);

	printf_MENU_EXE("RCtrlAlign(%d)\n",value); 
	menu_cmd_run( MENU_UI_DISTANCECTL, value, NULL );
}

void SpkLmtSet(unsigned int value)
{
	SaveMenuValue(6,0,value);
	DisplayOneState(2,value);

	printf_MENU_EXE("SpkLmtSet(%d)\n",value);
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}

void ApplyLmtSet(unsigned int value)
{
	SaveMenuValue(7,0,value);
	DisplayOneState(3,value);

	printf_MENU_EXE("ApplyLmtSet(%d)\n",value);
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}
void SwitchCamera(unsigned int value)
{
	SaveMenuValue(5,0,value);
	DisplayOneState(1,value);
	printf_MENU_EXE("SwitchCamera(%d)\n",value);
	unsigned char sa = (unsigned char)value;
	ByteDataSave( VAL_CUR_CMR, sa );
	menu_cmd_run( MENU_UI_SWITCHCMR, (uint16_t)value,NULL );
}

void SavePreset(unsigned int value)
{
	printf_MENU_EXE("SavePreset(%d)\n",value);
	menu_cmd_run( MENU_UI_PRESETSAVE, value,NULL );
}

void ClearPreset(unsigned int value)
{
	printf_MENU_EXE("ClearPreset(%d)\n",value);
	menu_cmd_run( MENU_UI_CLEARPRESET, value,NULL );
}

void CameraLR(unsigned int value)
{
	printf_MENU_EXE("CameraLR(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLLEFTRIGHT, value,NULL );
}


void CameraUD(unsigned int value)
{
	printf_MENU_EXE("CameraUD(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLUPDOWN, value,NULL );
}


void CameraFouce(unsigned int value)
{
	printf_MENU_EXE("CameraFouce(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLFOUCE, value,NULL );
}

void CameraZoom(unsigned int value)
{
	printf_MENU_EXE("CameraZoom(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLZOOM, value,NULL );
}


void CameraAperture(unsigned int value)
{
	printf_MENU_EXE("CameraAperture(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERACTLIRIS, value,NULL );
}

extern uint16_t terminal_pro_get_address( int get_flags, uint16_t addr_cur );
unsigned short FindTmnAddr(unsigned short Addr, Bool NextFlag)
{
	unsigned short addr = 0xffff;
	
	printf_MENU_EXE("CameraAperture(Addr=0x%x,NextFlag=%d)\n",Addr,NextFlag);
	if( NextFlag )
	{
		addr = terminal_pro_get_address( 1, Addr );
	}
	else
	{
		addr = terminal_pro_get_address( -1, Addr );
	}
	
	return addr;
}

TPresetTmnSelPro gPresetTmnSelPro;
void PresetTmnSel(unsigned int value)
{
  unsigned short Addr;
  printf_MENU_EXE(" gPresetTmnSelPro.Addr = 0x%04x\n", gPresetTmnSelPro.Addr);
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
	    Addr = FindTmnAddr(gPresetTmnSelPro.Addr,BOOL_TRUE);
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
    	Addr = FindTmnAddr(gPresetTmnSelPro.Addr,BOOL_FALSE);
		}
  }
  ShowSelTmn(Addr);
	gPresetTmnSelPro.Addr = Addr;
	gPresetTmnSelPro.Time= get_current_time();
	gPresetTmnSelPro.RunFlag = BOOL_TRUE;
	printf_MENU_EXE("PresetTmnSel(value=%d,Addr=0x%04x)\n",value,Addr);
	menu_cmd_run( MENU_UI_SELECTPRESETADDR, gPresetTmnSelPro.Addr,NULL );
}

#if 0
void PresetTmnSelPro(void)// 处理预置位选择
{
	if(gPresetTmnSelPro.RunFlag)
	{
		if((get_current_time()-gPresetTmnSelPro.Time)>500)
		{
			menu_cmd_run( MENU_UI_SELECTPRESETADDR, gPresetTmnSelPro.Addr,NULL );
			gPresetTmnSelPro.RunFlag = BOOL_FALSE;
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
	printf_MENU_EXE("ModeSet(%d)\n",value);
	Mode = value;
	SaveMenuValue(13,0,value);
	menu_cmd_run( MENU_UI_MODESET, value,NULL );
}

void ChmPriorEnSet(unsigned int value)
{
	unsigned PriorEn;
	PriorEn = value?1:0;
	SaveMenuValue(1,1,value);
	printf_MENU_EXE("ChmPriorEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_TEMPCLOSE, value,NULL );
}
void ChmMusicEnSet(unsigned int value)
{
	unsigned MusicEn;
	MusicEn = value?1:0;
	SaveMenuValue(1,2,value);
	printf_MENU_EXE("ChmMusicEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_CHAIRMANHINT, value,NULL );
}
void AutoOffEnSet(unsigned int value)
{
	unsigned AutoOffEn;
	AutoOffEn = value?1:0;
	SaveMenuValue(1,4,value);
	printf_MENU_EXE("AutoOffEnSet(%d)\n",value);
	menu_cmd_run( MENU_UI_AUTOCLOSE, value,NULL );
}
void CmrTrackSet(unsigned int value)
{
	unsigned int CmrNum;
  	SaveMenuValue(1,3,value);
	GetMenuValue(5,0,&CmrNum);
	printf_MENU_EXE("CmrTrackSet(%d)\n",value);
	menu_cmd_run( MENU_UI_CAMERATRACK, value,NULL );
}
int GetCmrTrack(void)
{
	unsigned int value;
	GetMenuValue(1,3,&value);
	printf_MENU_EXE("GetCmrTrack,value=%d\n",value);
	return (value?1:0);
}

void TerminalReAllotAddr(unsigned int value)
{
	printf_MENU_EXE("TerminalReAllotAddr(%d)\n",value);
	menu_cmd_run( MENU_UI_REALLOT, value,NULL );
}
void TerminalAddAllotAddr(unsigned int value)
{
	printf_MENU_EXE("TerminalAddAllotAddr(%d)\n",value);
	menu_cmd_run( MENU_UI_NEWALLOT, value, NULL );
}
void TerminalAllotAddrOver(unsigned int value)
{
	printf_MENU_EXE("TerminalAllotAddrOver(%d)\n",value);
	menu_cmd_run( MENU_UI_SETFINISH, value, NULL );
}

void ByteDataInit(void)
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
	printf_MENU_EXE("ByteDataGet(Index=%d)",Index);
	if(Index<PAR_NUM)
	{
		menu_cmd_run( MENU_UI_GET_PARAM, Index, &gByteData[Index] );
		*pValue=gByteData[Index];
		printf_MENU_EXE("value = %d\n", *pValue );
		return BOOL_TRUE;
	}
	
	return BOOL_FALSE;
}

Bool ByteDataSave(unsigned char Index, unsigned char Value)
{
	printf_MENU_EXE("ByteDataSave(Index=%d)\n",Index);
	if(Index<PAR_NUM)
	{
		gByteData[Index]=Value;
		menu_cmd_run( MENU_UI_SAVE_PARAM, Index, &gByteData[Index] );
		return BOOL_TRUE;
	}
	
	return BOOL_FALSE;

}

extern bool use_dis_set( uint8_t  user, bool set );// yasir add in 2016-4-8
Bool UseDisSet(unsigned char Use,Bool Set)
{
	printf_MENU_EXE("UseDisSet(Use=%d,Set=%d)\n",Use,Set);
	if(!(Use&(TMN_USE|MENU_USE|SYS_USE|CPT_USE)))
	{
		return BOOL_FALSE;
	}
	
	if(Set)
	{
		gUseDis |= Use;
	}
	else
	{
		gUseDis &= (~Use);
	}
	
	use_dis_set(Use, (Use == CPT_USE)?!Set:Set);// yasir add in 2016-4-8 , 不在主界面时上位机不能控制主机，界面传的set的值与实际的相反
	
	return BOOL_TRUE;
}

int SendMainState(short int snCmd, void *pData, unsigned long ulLen)
{
	printf_MENU_EXE("SendMainState(snCmd=%d)\n",snCmd);
	menu_cmd_run( MENU_UI_SEND_MAIN_STATE, 0, NULL );

	return 0;
}

void SaveWirelessAddr(unsigned char *pAddr, unsigned AddrSize)
{
	printf_MENU_EXE( "SaveWirelessAddr AddrSize = %d\n", AddrSize );
	assert( pAddr != NULL);
	if( pAddr == NULL || AddrSize != 3 )
		return;

	unsigned char addrlist[3]={0};
	memcpy( addrlist, pAddr, AddrSize );
	menu_cmd_run( MENU_UI_SAVE_WIRE_ADDR, AddrSize, addrlist );
}

void GetWirelessAddr(unsigned char *pAddr, unsigned AddrSize)
{
	printf_MENU_EXE( "GetWirelessAddr AddrSize = %d\n", AddrSize );
	assert( pAddr != NULL);
	if( pAddr == NULL || AddrSize != 3 )
		return;

	unsigned char addrlist[3]={0};
	if( 0 == menu_cmd_run( MENU_UI_GET_WIRE_ADDR, AddrSize, addrlist )) // get right ?
	{
		memcpy( pAddr, addrlist, AddrSize );	
	}
}

//摄像头预置位
void SaveCmrPreSet( unsigned char cmd ,unsigned char *data, unsigned short data_len )
{
	printf_MENU_EXE("SaveCmrPreSet\n");
	menu_cmd_run( MENU_UI_PRESETSAVE, 0,NULL );
}

void CmrCtrlDirect(unsigned short cmd)
{
	printf_MENU_EXE("SaveCmrPreSet( cmd = 0x%02x)\n",cmd);

	switch( cmd )
	{
		case CMR_CTRL_UP:
			menu_cmd_run( MENU_UI_CAMERACTLUPDOWN, true, NULL );
			break;
		case CMR_CTRL_DOWN:
			menu_cmd_run( MENU_UI_CAMERACTLUPDOWN, false, NULL );
			break;
		case CMR_CTRL_LEFT:
			menu_cmd_run( MENU_UI_CAMERACTLLEFTRIGHT, false, NULL );
			break;
		case CMR_CTRL_RIGHT:
			menu_cmd_run( MENU_UI_CAMERACTLLEFTRIGHT, true, NULL );
			break;
		case CMR_CTRL_ZOOM_TELE:
			menu_cmd_run( MENU_UI_CAMERACTLZOOM, true, NULL );
			break;
		case CMR_CTRL_ZOOM_WIDE:
			menu_cmd_run( MENU_UI_CAMERACTLZOOM, false, NULL );
			break;
		case CMR_CTRL_FOUCE_FAR:
			menu_cmd_run( MENU_UI_CAMERACTLFOUCE, false, NULL );
			break;
		case CMR_CTRL_FOUCE_NEAR:
			menu_cmd_run( MENU_UI_CAMERACTLZOOM, true, NULL );
			break;
		case CMR_CTRL_STOP:
			break;
		default:
			break;
	}
}

void CmrLockSave(void)
{
	unsigned char CmrTrack;
	ByteDataGet(VAL_CMR_TRACK_EN,&CmrTrack);
	CmrTrack=CmrTrack?0:1;
	SaveMenuValue(1,3,CmrTrack);
	SendMainState(0,NULL,0);
	printf_MENU_EXE("CmrLockSave\n");
}

void CameraSelect( unsigned char cmd ,unsigned char *data, unsigned short data_len )	// 1~4代表1~4号摄像头
{
	uint16_t camera_num;
	assert( data != NULL );
	if( data == NULL )
		return;

	camera_num = *data;
	printf_MENU_EXE( "CameraSelect(%d)\n", camera_num );
	menu_cmd_run( MENU_UI_SWITCHCMR, camera_num,NULL );
}


