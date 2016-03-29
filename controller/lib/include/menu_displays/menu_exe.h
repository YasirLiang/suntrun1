#ifndef _MENU_EXE_H_
#define _MENU_EXE_H_

typedef struct
{
	unsigned int RunFlag;
	unsigned int Time;
	unsigned short Addr;
}TPresetTmnSelPro;

typedef enum
{
	VAL_TEMP_CLOSE=0,	//暂时关闭
	VAL_CHM_MUSIC,		//主席音乐
	VAL_CMR_TRACK_EN,	//摄像跟踪使能
	VAL_AUTO_CLOSE,		//自动关闭
	VAL_CUR_CMR,		//当前摄像头
	VAL_SPKER_LIMIT,		//发言人数上限
	VAL_APPLY_LIMIT,		//申请人数上限
	VAL_DSCS_MODE, 		//会讨模式
	VAL_MENU_LANG,		//菜单语言
	PAR_NUM
}EPar;

extern TPresetTmnSelPro gPresetTmnSelPro;

void EnterCmrPreset(unsigned int value);

void EscCmrPreset(unsigned int value);

//遥控器对码
void RCtrlAlign(unsigned int value);

void SpkLmtSet(unsigned int value);

void ApplyLmtSet(unsigned int value);

void SwitchCamera(unsigned int value);

void SavePreset(unsigned int value);

void ClearPreset(unsigned int value);

void CameraLR(unsigned int value);


void CameraUD(unsigned int value);


void CameraFouce(unsigned int value);

void CameraZoom(unsigned int value);


void CameraAperture(unsigned int value);


void ModeSet(unsigned int value);

void ChmPriorEnSet(unsigned int value);

void ChmMusicEnSet(unsigned int value);

void AutoOffEnSet(unsigned int value);

void CmrTrackSet(unsigned int value);

int GetCmrTrack(void);

void PresetTmnSel(unsigned int value);

void PresetTmnSelPro(void);

void TerminalReAllotAddr(unsigned int value);

void TerminalAddAllotAddr(unsigned int value);

void TerminalAllotAddrOver(unsigned int value);

void ByteDataInit(void);

#endif
