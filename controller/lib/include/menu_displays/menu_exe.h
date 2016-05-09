#ifndef _MENU_EXE_H_
#define _MENU_EXE_H_

#define CMR_CTRL_STOP       0x0000
#define CMR_CTRL_SAVE_PRESET  0x0300
#define CMR_CTRL_GET_PRESET   0x0700
#define CMR_CTRL_RIGHT      0x0200
#define CMR_CTRL_LEFT       0x0400
#define CMR_CTRL_UP         0x0800
#define CMR_CTRL_DOWN       0x1000
#define CMR_CTRL_ZOOM_TELE  0x2000
#define CMR_CTRL_ZOOM_WIDE  0x4000
#define CMR_CTRL_FOUCE_FAR  0x8000
#define CMR_CTRL_FOUCE_NEAR 0x0001
#define CMR_CTRL_IRIS_OPEN  0x0002
#define CMR_CTRL_IRIS_CLOSE 0x0004
#define CMR_CTRL_AUTO_SCAN  0x0010


typedef struct
{
	unsigned int RunFlag;
	unsigned int Time;
	unsigned short Addr;
}TPresetTmnSelPro;

typedef enum
{
	VAL_TEMP_CLOSE=0,	//��ʱ�ر�
	VAL_CHM_MUSIC,		//��ϯ����
	VAL_CMR_TRACK_EN,	//�������ʹ��
	VAL_AUTO_CLOSE,		//�Զ��ر�
	VAL_CUR_CMR,		//��ǰ����ͷ
	VAL_SPKER_LIMIT,		//������������
	VAL_APPLY_LIMIT,		//������������
	VAL_DSCS_MODE, 		//����ģʽ
	VAL_MENU_LANG,		//�˵�����
	PAR_NUM
}EPar;

extern TPresetTmnSelPro gPresetTmnSelPro;

void EnterCmrPreset(unsigned int value);

void EscCmrPreset(unsigned int value);

//ң��������
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

void SaveWirelessAddr(unsigned char *pAddr, unsigned AddrSize);
void GetWirelessAddr(unsigned char *pAddr, unsigned AddrSize);
void SaveCmrPreSet( unsigned char cmd ,unsigned char *data, unsigned short data_len );
void CmrCtrlDirect(unsigned short cmd);
void CmrLockSave(void);
void CameraSelect( unsigned char cmd ,unsigned char *data, unsigned short data_len );	// 1~4����1~4������ͷ

#endif
