#include <string.h>
#include "wireless.h"
#include "menu_exe.h"
#include "menu_f.h"

#define WIRE_ADDR_BUF_SIZE  3

unsigned char gpWirAddr[WIRE_ADDR_BUF_SIZE];//保存遥控器的ID，对码后保存在片外EEPROM
unsigned char gu8RevDataBak[5];//保存上一次接收的数据(除最后的校验字节)
int gRCtrlAlignEn;



/********************************************************************************
  * 程序名：    Judge_Comm
  * 说明：   	判断接收的是不是上次发送的数据
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

	//保存数据
	memcpy(gu8RevDataBak,pu8RevData,5);

	return(flag);
}
//===============================================================================

/********************************************************************************
  * 程序名：    Judge_Addr
  * 入口参数：  无
  * 出口参数：  无
  *	返回值：	无
  * 说明：   	判断是不是本机的遥控器发送的数据
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

void RCtrlAlignEn(int flag) // 1:使能，0:禁止
{
  gRCtrlAlignEn = flag?1:0;
}

void wireless_pross(unsigned char *pWirData)
{
    unsigned char Value;
    if(gRCtrlAlignEn)//如果在对码显示页
    {
        if(!Judge_Comm(pWirData))//不是上一次的遥控器数据
            return;
        if(pWirData[WIRE_ADDR_BUF_SIZE] == STUDY_KEY_VALUE)//如果是对码键
        {
          memcpy(gpWirAddr,pWirData,WIRE_ADDR_BUF_SIZE);//保存ID数据
          CtrlMenuSw(MGS_M12);
          SaveWirelessAddr(gpWirAddr,WIRE_ADDR_BUF_SIZE);
          return;
        }
    }  
    if(!Judge_Addr(pWirData))//不是本机的遥控器
        return;
    if(!Judge_Comm(pWirData))//不是上一次的数据
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


