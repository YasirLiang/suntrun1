/*
**central_control_transmit_unit.h
**9-3-2016
**
*/

// ****************************
// file built 2016-3-9
// 建立会议系统连接表管理机制
// 中心控制单元发送模块
// ****************************

#ifndef __CENTRAL_CONTROL_TRANSMIT_UNIT_H__
#define __CENTRAL_CONTROL_TRANSMIT_UNIT_H__

//************************************

enum ccu_muticast_pro_enum// 中央广播者的初始化处理
{
	CENTRAL_OUT_PRIMITED = 0,// 中心输出未处理
	CENTRAL_OUT_HANDLE,// 中心输出正处理
	CENTRAL_OUT_FINISH//  中心输出初始化完成
};

//************************************//

#endif
