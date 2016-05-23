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

#include "output_channel.h"
#include "global.h"
#include "descriptor.h"
#include "connector_subject.h"

//************************************

#define CCU_CONTROL_TRANSMIT_UNIT_NAME central_control_unit_name
#define CCU_CONTROL_TRANSMIT_UNIT_MAX_NUM central_control_unit_max_num
#define CCU_CONTROL_TRANSMIT_UINT_OUTPUT central_control_transmit_uint_output
#define CCU_CONTROL_TRANSMIT_UINT_MAX_OUTPUT_NUM 4

enum _enum_ccu_chout_pro// 中央广播者的初始化处理
{
	CENTRAL_OUT_PRIMITED = 0,// 中心输出未处理
	CENTRAL_OUT_HANDLE,// 中心输出正处理
	CENTRAL_OUT_FINISH//  中心输出初始化完成
};

typedef enum ccu_transmit_model_output_state
{
	TCCU_LEISURE,// 空闲
	TCCU_EXCHANGE,// 正在改变广播者的状态
	TCCU_MUTISCASTING,// 正在广播
}EccuTModelOutputState;

typedef struct _type_central_control_trans_model// 中央传输模块
{
	uint64_t tarker_id;
	TOutChannel out_ch; // 输出通道，表为空则无输出
	struct list_head list; 
}TccuTModel,*T_pccuTModel;

extern int central_control_transmit_unit_init( const uint8_t *frame, int pos, size_t frame_len, const desc_pdblist desc_node, const uint64_t endtity_id );
extern void central_control_transmit_unit_update( subject_data_elem reflesh_data );// 更新会议接收单元模块的连接状态
extern void central_control_transmit_unit_model_pro_init( void );

//************************************//

#endif
