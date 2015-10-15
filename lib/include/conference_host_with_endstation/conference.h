#ifndef __CONFERENCE_H__
#define __CONFERENCE_H__

#include "jdksavdecc_world.h"
#include "host_controller_debug.h"
#include "jdksavdecc_aecp_aem.h"

#define CONFERENCE_TYPE 0xAB
#define DATAMAXLENGTH 64

//协议头部长度
#define CONFERENCE_COMMON_HEADER_LEN 4
//校验数据长度
#define CONFERENCE_CRC_LEN 1
//终端响应数据位长度
#define END_TO_HOST_DATA_IN_CMD_LEN 1
//主机响应数据位长度
#define HOST_TO_END_DATA_IN_CMD_LEN 1

/*{@会议系统命令在负载中的偏移@}*/
#define CONFERENCE_DATA_IN_CONTROLDATA_OFFSET 24
#define CONFERENCE_DATA_IN_CONTROLDATA_LEN 24

//终端普通响应长度(包括数据位)
#define END_TO_HOST_CMD_LEN (CONFERENCE_COMMON_HEADER_LEN +  CONFERENCE_CRC_LEN +\
	END_TO_HOST_DATA_IN_CMD_LEN)
//终端特殊响应长度(不包括数据位)
#define END_TO_HOST_CMD_SPECIAL_LEN (END_TO_HOST_CMD_LEN + END_TO_HOST_DATA_IN_CMD_LEN)

//主机响应或下发命令头部长度(除了数据长度 (这里依据数据长度位的值确定))
#define HOST_COMMON_TO_END_EXDATA_LEN (CONFERENCE_COMMON_HEADER_LEN + HOST_TO_END_DATA_IN_CMD_LEN +\
	CONFERENCE_CRC_LEN)
//普通终端响应或上发命令的头部长度加校验数据长度(除了数据长度 1)
#define END_COMMON_TO_HOST_LEN (CONFERENCE_COMMON_HEADER_LEN  + CONFERENCE_CRC_LEN)
//特殊终端响应或上发命令的头部与校验数据长度(除了数据长度 (这里依据数据长度位的值确定))
#define END_COMMON_TO_HOST_SPECIAL_LEN (CONFERENCE_COMMON_HEADER_LEN + END_TO_HOST_DATA_IN_CMD_LEN +\
	CONFERENCE_CRC_LEN)


/*{@主机命令*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_QUERY_END 0x01//主机查询终端
#define HOST_TO_ENDSTATION_COMMAND_TYPE_ALLOCATION 0x02//终端要求分配地址
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_STATUS 0x03//设置终端状态
#define HOST_TO_ENDSTATION_COMMAND_TYPE_REALLOCATION 0x04//重新分配地址
/*{@终端按键动作*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_KEYPAD_ACTION 0x05//终端按键动作
/*@终端按键动作}*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_ENDLIGHT 0x06//设置终端指示灯
#define HOST_TO_ENDSTATION_COMMAND_TYPE_NEW_ALLOCATION 0x07//新增终端分配地址
#define HOST_TO_ENDSTATION_COMMAND_TYPE_END_ASSIGN 0x08 //终端报到
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LCD  0x09 //设置终端LCD显示方式
#define HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LCD 0x0a//发送终端LCD显示数据,这里是公共长度

#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_END_LED 0x0b//设置终端LED显示方式
#define HOST_TO_ENDSTATION_COMMAND_TYPE_COMMON_SEND_END_LED 0x0c//发送终端LED显示数据,这里是公共长度
#define HOST_TO_ENDSTATION_COMMAND_TYPE_PRESIDENT_CONTROL  0x0d//主席机控制会议
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_VOTE_RESULT 0x0e //发送表决结果
#define HOST_TO_ENDSTATION_COMMAND_TYPE_TALKTIME_LEN 0x0f //发言时长
#define HOST_TO_ENDSTATION_COMMAND_TYPE_HOST_SEND_STATUS 0x10 //主机发送状态
#define HOST_TO_ENDSTATION_COMMAND_TYPE_SEND_END_LCD_DISPLAY 0x11//发送终端LCD显示屏号
#define HOST_TO_ENDSTATION_COMMAND_TYPE_OPTITION_END 0x12 //操作终端

#define HOST_TO_ENDSTATION_COMMAND_TYPE_SET_MIS_STATUS 0x13 //设置话筒状态

#define HOST_TO_ENDSTATION_COMMAND_TYPE_END_SPETHING 0x14 //终端特殊事件

#define HOST_TO_ENDSTATION_COMMAND_TYPE_CHECK_END_RESULT 0x16 //查询终端表决结果

#define HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_HOST_MSG 0x1e//转发上位机短消息数据的最大长度，此时的data_len需要根据实际的情况确定
#define HOST_TO_ENDSTATION_COMMAND_TYPE_TRANSIT_END_MSG 0x1f //转发终端短信息
/*@}主机命令*/ 

/*@{命令高三位的含义*/
#define CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END 0
#define CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND 0
#define CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND 1
#define CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND 0
//000(主机下发, 主动发出的报文,普通代表机发出的报文)
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_0 \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//001(主机下发,主动发出的报文,主席单元发出的报文)	
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_1 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//010(主机下发,响应的报文,普通代表机发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_2 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//011(主机下发,响应的报文,主席单元发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_3 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_HOST_TO_END << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1)|\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//100(终端上发,主动发出的报文, 普通代表机发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_4 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1)|\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//101(终端上发,主动发出的报文, 主席单元发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_5 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_INITIATIVE_SEND << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
//110(终端上发,响应的报文, 普通代表机发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_6 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_COMMON_SEND << 0)
//111(终端上发,响应的报文, 主席单元发出的报文)		
#define CONFERENCE_COMMAND_HIGH3BIT_MEANING_7 \
		(CONFERENCE_COMMAND_HIGH3BIT_BIT7_END_TO_HOST << 2) | \
	(CONFERENCE_COMMAND_HIGH3BIT_BIT6_REPONSE << 1) |\
	(CONFERENCE_COMMAND_HIGH3BIT_BIT5_PRESIDENT_SEND << 0)
/*@命令高三位的含义}*/


/*{@主机下发数据长度*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_QUERY_END 0//主机查询终端
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_ALLOCATION_STATUS1 0x02//终端要求分配地址响应1
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_ALLOCATION_STATUS2 0x0//终端要求分配地址响应2
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_REALLOCATION 0x0//重新分配地址
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_STATUS 0x04//设置终端状态
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_MIS_STATUS 0x01 //设置话筒状态
/*{@终端按键动作*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_COMMONE 0x01//普通响应
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_SPECIAL1 0x03//特殊响应1
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_KEYPAD_SPECIAL2 0x06//特殊响应2
/*@终端按键动作}*/
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_ENDLIGHT 0x02//设置终端指示灯
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_NEW_ALLOCATION 0x00//新增终端分配地址
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_END_ASSIGN 0x00 //终端报到
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_LCD  0x1 //设置终端LCD显示方式
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_COMMON_SEND_END_LCD 0x05//发送终端LCD显示数据,这里是公共长度

#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SET_END_LED 0x02//设置终端LED显示方式
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_COMMON_SEND_END_LED 0x01//发送终端LED显示数据,这里是公共长度
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_PRESIDENT_CONTROL  0x01//主席机控制会议
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SEND_VOTE_RESULT 0x08 //发送表决结果
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TALKTIME_LEN 0x01 //发言时长
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_HOST_SEND_STATUS 0x07 //主机发送状态
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_SEND_END_LCD_DISPLAY 0x02//发送终端LCD显示屏号
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_OPTITON_END 0x01 //操作终端
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_END_SPETHING 0x00 //终端特殊事件
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TRANSIT_HOST_MSG_MAX 120//转发上位机短消息数据的最大长度，此时的data_len需要根据实际的情况确定
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_TRANSIT_END_MSG 0x00 //转发终端短信息
#define HOST_TO_ENDSTATION_COMMAND_TYPE_LENGTH_CHECK_END_RESULT 0x00 //查询终端表决结果
/*@}主机下发数据长度*/

/*@{终端地址*/
#define CONFERENCE_ENDSTATION_ADDRESS_APPLICATION (0x0 & 0xf) << 12//应用地址高十二位标志
#define CONFERENCE_ENDSTATION_ADDRESS_DEVICE (0x1 & 0xf) << 12//设备地址高十二位标志

#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_ALL ( 0x40 & 0x7f ) //全广播高7位标志
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON ( 0x41 & 0x7f ) //广播普通代表高7位标志
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_VIP ( 0x42 & 0x7f ) //广播VIP高7位标志
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_COMMON_PRESIDENT ( 0x44 & 0x7f ) //广播普通主席高7位标志
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_EXECUTE_PRESIDENT (0x48 & 0x7f ) //广播执行主席高7位标志

//广播是否需要回应及时间长度
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_RESPOND ( (0x01 & 0x1) << 8 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_NORESPOND ( (0x00 & 0x1) << 8 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND7F ( ( 0x00 & 0x3 ) << 6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WINDFF ( ( 0x01 & 0x3 ) <<6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND1FF ( ( 0x02 & 0x3 ) << 6 )
#define CONFERENCE_ENDSTATION_ADDRESS_BROADCAST_WIND3FF ( (0x03 & 0x3 ) << 6 )
/*@终端地址}*/

struct conference_end_address_euint16
{
	uint8_t vales[2];
};

// 会议系统协议公共头部格式
struct conference_common_header
{
	uint8_t byte_guide;			// 引导字节
	uint8_t command_control;	// 控制命令
	uint16_t address;			// 终端地址
};

struct host_to_endstation
{
	struct conference_common_header cchdr;	// 公共头部
	uint8_t data_len;						// 数据长度
	uint8_t data[DATAMAXLENGTH];			// 数据区
	uint8_t crc;							// 校验解惑(值为前面数据的异或)
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// 协议的备份
};

struct endstation_to_host					
{
	struct conference_common_header cchdr;	// 公共头部
	uint8_t data;							// 数据区
	uint8_t crc;							// 校验解惑(值为前面数据的异或)
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// 协议的备份
};

struct endstation_to_host_special			// 终端特殊命令格式
{
	struct conference_common_header cchdr;
	uint8_t data_len;
	uint8_t data[DATAMAXLENGTH];
	uint8_t crc;
	uint8_t deal_backups[DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN];	// 协议的备份
};

struct terminal_deal_frame // aecp data conference data frame
{
	struct jdksavdecc_aecpdu_aem aecpdu_aem_header;
	uint16_t payload_len;
	uint8_t payload[ (DATAMAXLENGTH + HOST_COMMON_TO_END_EXDATA_LEN)*2]; // 协议备份
};

inline ssize_t conference_validata_range(size_t bufpos, ssize_t cdata_len,size_t buflen);

#endif




