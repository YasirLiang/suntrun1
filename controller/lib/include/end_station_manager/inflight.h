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
	INFLIGHT_TRANSMIT_TYPE_UDP_SERVER // Э��δ��(150910)
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
	bool running;					// ����
	bool elapsed;					// ��ʱ��־
	uint32_t start_time;			// ��ʼʱ��
	uint32_t count_time;			// ��ʱʱ��
};

struct background_inflight_cmd
{
	uint8_t background_inflight_cmd_type;
	struct inflight_timeout timeouts;
};

// ��1722Э���д���Ļ���ϵͳ��Э�����������ʶ�𷽷���ͨ���ǹ㲥(0x8000)���ն�Ӧ�õ�ַ�����Э��������ʶ��, 
//���������͵��ǹ㲥��ַ(inflight��)�����ȼ�������Ƿ�ԣ�����ȷ�����inflight����
struct conference_data_in_aecp_payload_recgnize 
{
	uint8_t conference_command;
	uint16_t address;
};

// seq_id�������У�Ϊacmp��aecp�����е����кţ�acmp��aecp�����ʶ������֮һ����һ����data_type����udp֮ͨ��data_type��seq_id��seq_id����Ϊ���Э����������ͣ���ʱseq_id�������Ѳ�ͬ
struct _inflight_frame
{
	bool notification_flag;				// ���ͱ�־
	uint8_t data_type;					// �����������ͣ�Ϊ���������е�����
	uint8_t *frame;					// �������ݱ�����
	struct conference_data_in_aecp_payload_recgnize conference_data_recgnize;// ����ϵͳЭ�����ݵ���Ӧʶ��ֻ���������еĵ�һ���� 
// ��ͬһ���ն��ط�һ����ͬ��������ʶ��2016-3-2��ʹ��ʱ��֤�����в���ͬʱ������ͬ�ն���ͬ����Ľڵ�������������������ʱ�����κδ���(2016-3-2)
	uint16_t seq_id;					// seq_id��������(��������1722Э�����ݵĴ���(������aecp���� �Ļ���Э��Э��))��Э�������(��������udp����������)
	uint16_t inflight_frame_len;			// �������ݵĳ���
	struct jdksavdecc_eui48 raw_dest;		// ֻ����ԭʼ���ݰ���Ŀ���ַ
	struct sockaddr_in sin_in;			// ����udp���ݰ�
};

struct inflight_flag
{
	bool resend;						// �ظ����ͱ�־
	uint8_t retried;					// ���͵�������Ϊ2(���Ըı�)��ÿ����һ��+1		
};

//#pragma pack(1)
struct inflight
{
	uint32_t command_type;				// �����������ݵ��������ͣ���acmp��aecp��udp client��udp server inflight_command_type
	struct inflight_flag flags;				// inflight ����Ĳ���
	struct inflight_timeout timeout;		// inflight ����ĳ�ʱ��Ϣ
	struct _inflight_frame inflight_frame;	// �������ݲ���
};
//#pragma pack()

typedef struct _inflight
{
	struct inflight host_tx;
	struct _inflight *prior, *next;
}inflight_list,*inflight_plist;

typedef struct _type_inflight_proccess
{
	bool running;
	pthread_mutex_t mutex;
}tinflight_proccess;

extern struct raw_context net;			// ԭʼ�׽���
extern volatile bool is_inflight_timeout;
extern tinflight_proccess ginflight_pro;

void inflight_proccess_init( void );
bool is_inflight_cmds_retried( inflight_plist station );
void inflight_timer_state_avail( timetype timeout, inflight_plist target );
void inflight_timer_start( timetype timeout, inflight_plist target );
void inflight_timer_stop( inflight_plist target );
void inflight_timer_update( timetype timeout, inflight_plist target );
bool inflight_timer_timeout( inflight_plist target);
bool background_inflight_timeout( struct background_inflight_cmd *binflight_cmd );
void background_inflight_timer_start( timetype timeout, struct background_inflight_cmd *binflight_cmd );


#endif

