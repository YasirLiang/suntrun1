#ifndef __INFLIGHT_H__
#define __INFLIGHT_H__

#include <netinet/in.h>
#include <netinet/ip.h>
#include "jdksavdecc_world.h"
#include "host_time.h"
#include "jdksavdecc_util.h"
#include "jdksavdecc_pdu.h"
#include "jdksavdecc_acmp.h"
#include "util.h"

#define RECV_RESPONSE 1

enum inflight_transmit_data_subtype
{
	INFLIGHT_TRANSMIT_TYPE_ADP = 0x7a,
	INFLIGHT_TRANSMIT_TYPE_AECP = 0x7b,
	INFLIGHT_TRANSMIT_TYPE_ACMP = 0x7c,
	INFLIGHT_TRANSMIT_TYPE_UDP_CLIENT = 0xac,
	INFLIGHT_TRANSMIT_TYPE_UDP_SERVER // 协议未定(150910)
};

enum inflight_command_type
{
	INFLIGHT_ADP,
	INFLIGHT_ACMP,
	INFLIGHT_AECP,
	INFLIGHT_UDP_SERVER,
	INFLIGHT_UDP_CLIENT
};

enum inflight_notification
{
	STOPINFLIGHT,
	RUNINFLIGHT
};

enum background_inflight_cmd_type
{
	INPUT_OUTPUT_STREAM_DESCRIPTOR
};

struct inflight_timeout
{
	bool running;					// 运行
	bool elapsed;					// 超时标志
	uint32_t start_time;			// 开始时间
	uint32_t count_time;			// 超时时间
};

struct background_inflight_cmd
{
	uint8_t background_inflight_cmd_type;
	struct inflight_timeout timeouts;
};

// seq_id发送序列，为acmp与aecp负载中的序列号，acmp与aecp命令的识别条件之一，另一个是data_type。而udp之通过data_type与seq_id，seq_id则作为其的协议的命令类型，此时seq_id的意义已不同
struct _inflight_frame
{
	bool notification_flag;				// 发送标志
	uint8_t data_type;					// 发送数据类型，为负载数据中的类型
	uint8_t *frame;					// 网络数据保存区
	uint16_t seq_id;					// seq_id发送序列
	uint16_t inflight_frame_len;			// 网络数据的长度
	struct jdksavdecc_eui48 raw_dest;		// 只用于原始数据包的目标地址
	struct sockaddr_in sin_in;			// 用于udp数据包
};

struct inflight_flag
{
	bool resend;						// 重复发送标志
	uint8_t retried;					// 发送的最大次数为2(可以改变)，每发送一次+1		
};

struct inflight
{
	uint32_t command_type;				// 主机发送数据的数据类型，有acmp、aecp、udp client、udp serverinflight_command_type
	struct inflight_flag flags;				// inflight 命令的参数
	struct inflight_timeout timeout;		// inflight 命令的超时信息
	struct _inflight_frame inflight_frame;	// 网络数据参数
};

typedef struct _inflight
{
	struct inflight host_tx;
	struct _inflight *prior, *next;
}inflight_list,*inflight_plist;

extern struct raw_context net;			// 原始套接字

bool is_inflight_cmds_retried( inflight_plist station );
void inflight_timer_state_avail( timetype timeout, inflight_plist target );
void inflight_timer_start( timetype timeout, inflight_plist target );
void inflight_timer_stop( inflight_plist target );
void inflight_timer_update( timetype timeout, inflight_plist target );
bool inflight_timer_timeout( inflight_plist target);
bool background_inflight_timeout( struct background_inflight_cmd *binflight_cmd );
void background_inflight_timer_start( timetype timeout, struct background_inflight_cmd *binflight_cmd );


#endif

