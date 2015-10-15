#ifndef __UPPER_COMPUTER_H__
#define __UPPER_COMPUTER_H__

#include "jdksavdecc_world.h"

#define DATA_PAYLOAD_LEN_MAX 128
#define OTHER_DATA_LENGHT 6
#define UPPER_PAYLOAD_DATA_MAX_LEN ( DATA_PAYLOAD_LEN_MAX + OTHER_DATA_LENGHT)
#define UPPER_COMPUTER_DATA_LOADER 0xac

#define HOST_UPPER_COMPUTER_COMMON_HEAD_LENGTH 5

/*{@主机与上位机协议的具体命令@}*/
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_DISCUSSION_PARAMETER 0x01	// 会讨参数			 
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_SWITCH 0x02 				// 麦克风开关
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_MISCROPHONE_STATUS 0x03 					// 麦克风状态
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SELECT_PROPOSER 0x05 					// 选择申请人
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_EXAMINE_APPLICATION 0x06 				// 审批申请
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CONFERENCE_PERMISSION 0x07 				// 会议权限
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SENDDOWN_MESSAGE 0x08 					// 下发短信息
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_TABLE_TABLET_STANDS_MANAGER 0x09 		// 桌牌管理
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_SIGN 0x0a 							// 开始签到
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_SIGN_SITUATION 0x0b 						// 签到情况
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_END_OF_SIGN 0x0c 						// 签到结束
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ALLOCATION_APPLICATION_ADDRESS 0x0e	// 终端分配应用地址 	
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_REGISTER_STATUS 0x0f 			// 终端报到情况
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_CURRENT_VIDICON 0x10 					// 当前摄像机
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_ENDSTATION_ADDRESS_UNDETERMINED_ALLOCATION 0x11	// 待定位终端应用地址	 
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_CONTROL 0x12 					// 摄像机控制
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_PRERATION_SET 0x13 				// 摄像机预置操作
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_LOCK 0x14 						// 摄像机锁定
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_VIDICON_OUTPUT 0x15 						// 摄像机输出
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_BEGIN_VOTE 0x17 							// 开始表决
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_PAUSE_VOTE 0x18 							// 暂定表决
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_REGAIN_VOTE 0x19 						// 恢复表决
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_END_VOTE 0x20							// 结束表决
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_RESULT_VOTE 0x21 						// 表决结果
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_TRANSMIT_TO_ENDSTATION 0x22 				// 转发给终端
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_REPORT_ENDSTATION_MESSAGE 0x23 			// 上报终端短信息
#define HOST_UPPER_COMPUTER_COMMAND_TYPE_HIGH_DEFINITION_SWITCH_SET 0x24  		// 设置高清摄像头切换命令
/*{@主机与上位机协议的具体命令@}*/



struct host_upper_cmpt_common	// 主机与上位机协议公共头
{
	uint8_t state_loader;			// 引导字节，固定内容为0xAC
	uint8_t deal_type;				// 报文类型
	uint8_t command;				// 命令	
	uint16_t data_len;				// 数据长度
};

struct host_upper_cmpt
{
	uint8_t deal_crc;
	struct host_upper_cmpt_common common_header;
	uint8_t data_payload[DATA_PAYLOAD_LEN_MAX];
};

struct host_upper_cmpt_frame		// 接受udp数据的信息结构体
{
	uint8_t dest_address[32];	 			// 目的ip地址
	int dest_port;							// 目的端口号
	int payload_len;
	uint8_t payload[DATA_PAYLOAD_LEN_MAX + OTHER_DATA_LENGHT]; //接收负载
};

#endif




