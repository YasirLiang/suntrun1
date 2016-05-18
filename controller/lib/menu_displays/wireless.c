#include <string.h>
#include "wireless.h"
#include "menu_exe.h"
#include "menu_f.h"

#define WIRE_ADDR_BUF_SIZE  3

unsigned char gpWirAddr[WIRE_ADDR_BUF_SIZE];//����ң������ID������󱣴���Ƭ��EEPROM
unsigned char gu8RevDataBak[5];//������һ�ν��յ�����(������У���ֽ�)
int gRCtrlAlignEn;



/********************************************************************************
  * ��������    Judge_Comm
  * ˵����   	�жϽ��յ��ǲ����ϴη��͵�����
********************************************************************************/
int Judge_Comm(unsigned char *pu8RevData)
{
	int flag;
	flag=0;
	if ((gu8RevDataBak[0]!=pu8RevData[0])||(gu8RevDataBak[1]!=pu8RevData[1])||
	    (gu8RevDataBak[2]!=pu8RevData[2])||(gu8RevDataBak[3]!=pu8RevData[3])||
	    (gu8RevDataBak[4]!=pu8RevData[4]))
	{
		flag=1;
	}

	//��������
	memcpy(gu8RevDataBak,pu8RevData,5);

	return(flag);
}
//===============================================================================

/********************************************************************************
  * ��������    Judge_Addr
  * ��ڲ�����  ��
  * ���ڲ�����  ��
  *	����ֵ��	��
  * ˵����   	�ж��ǲ��Ǳ�����ң�������͵�����
********************************************************************************/

int Judge_Addr(unsigned char *pu8RevData)
{
    int flag=0;
    if ((pu8RevData[0]==gpWirAddr[0])&&
        (pu8RevData[1]==gpWirAddr[1])&&
        (pu8RevData[2]==gpWirAddr[2]))
    {
        flag=1;
    }
    return (flag);
}

void RCtrlAlignEn(int flag) // 1:ʹ�ܣ�0:��ֹ
{
  gRCtrlAlignEn = flag?1:0;
}

void wireless_pross(unsigned char *pWirData)
{
    unsigned char Value;
    if(gRCtrlAlignEn)//����ڶ�����ʾҳ
    {
        if(!Judge_Comm(pWirData))//������һ�ε�ң��������
            return;
        if(pWirData[WIRE_ADDR_BUF_SIZE] == STUDY_KEY_VALUE)//����Ƕ����
        {
          memcpy(gpWirAddr,pWirData,WIRE_ADDR_BUF_SIZE);//����ID����
          CtrlMenuSw(MGS_M12);
          SaveWirelessAddr(gpWirAddr,WIRE_ADDR_BUF_SIZE);
          return;
        }
    }  
    if(!Judge_Addr(pWirData))//���Ǳ�����ң����
        return;
    if(!Judge_Comm(pWirData))//������һ�ε�����
        return;
    switch(pWirData[3])
    {
      case QUERY_KEY_VALUE:
        SaveCmrPreSet(0,NULL,0);
        break;
      case CHANGE_KEY_VALUE:

        break;
      case UP_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_UP);
        break;
      case DOWN_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_DOWN);
        break;
      case LEFT_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_LEFT);
        break;
      case RIGHT_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_RIGHT);
        break;
      case UP_LEFT_KEY_VALUE:

        break;
      case UP_RIGHT_KEY_VALUE:

        break;
      case DOWN_RIGHT_KEY_VALUE:

        break;
      case DOWN_LEFT_KEY_VALUE:

        break;
      case ZOOM_IN_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_ZOOM_TELE);
        break;
      case ZOOM_OUT_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_ZOOM_WIDE);
        break;
      case AUTO_FOCUS_KEY_VALUE:

        break;
      case FOCUS_FAR_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_FOUCE_FAR);
        break;
      case FOCUS_NEAR_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_FOUCE_NEAR);
        break;
      case LOCK_KEY_VALUE:
	//CmrLockSave(3); // yasirLiang change in 20160509
        CmrLockSave();
        break;
      case CHANGE_CAM1_KEY_VALUE:
        Value = 1;
        SaveMenuValue(5,0,Value);
        DisplayOneState(1,Value);
        CameraSelect(0,&Value,1);
        break;
      case CHANGE_CAM2_KEY_VALUE:
        Value = 2;
        SaveMenuValue(5,0,Value);
        DisplayOneState(1,Value);
        CameraSelect(0,&Value,1);
        break;
      case CHANGE_CAM3_KEY_VALUE:
        Value = 3;
        SaveMenuValue(5,0,Value);
        DisplayOneState(1,Value);
        CameraSelect(0,&Value,1);
        break;
      case CHANGE_CAM4_KEY_VALUE:
        Value = 4;
        SaveMenuValue(5,0,Value);
        DisplayOneState(1,Value);
        CameraSelect(0,&Value,1);
        break;
      case STOP_KEY_VALUE:
        CmrCtrlDirect(CMR_CTRL_STOP);
        break;
      default:
        break;
    }
}


