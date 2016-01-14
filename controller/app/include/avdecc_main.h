#ifndef __AVDECC_MAIN_H__
#define __AVDECC_MAIN_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "raw.h"
#include "host_controller_debug.h"
#include "pipe.h"
#include "entity.h"
#include "inflight.h"
#include "descriptor.h"
#include "func_proccess.h"
#include "data.h"

#ifdef __DEBUG__

#ifndef __TEST_DEBUG_CM__ // ����͸������
//#define __TEST_DEBUG_CM__
#endif
#endif

#define NETWORT_INTERFACE "eth0"

enum useful_enums
{
	PIPE_RD = 0,
	PIPE_WR = 1,
	POLL_COUNT = 4,
	TIME_PERIOD_1_MILLISECONDS = 1,
	TIME_PERIOD_25_MILLISECONDS = 25
};
       
struct fds
{
	int raw_fd;		// raw socket
	int udp_server_fd;	// udp server
	int udp_client_fd;	// udp client
	int tx_pipe[2];		// pipe
};

struct socket_info_s
{
	int sock_fd;
	struct sockaddr_in sock_addr;
	socklen_t sock_len;
};

struct udp_context
{
	struct socket_info_s udp_srv, udp_clt;
};

#define THREADS_MUX_NUM 16
struct threads_info
{
	pthread_t tid[THREADS_MUX_NUM];
	int pthread_nums;
};

extern struct fds net_fd;						// ����ͨ���׽������̼߳�ͨ���׽���
extern struct socket_info_s upper_udp_client;		// ��λ����ͨ����Ϣ
extern struct socket_info_s pc_controller_server;	// ������ʾ��Ϣ������ͷ��������ͨ����Ϣ 
extern solid_pdblist endpoint_list;				// ϵͳ���ն������ڱ��ڵ�
extern inflight_plist command_send_guard;		// ϵͳ�з��������������������ڱ��ڵ�
extern struct raw_context net;				// ԭʼ�׽���
extern desc_pdblist descptor_guard;			// ϵͳ�������������ڱ��ڵ�
const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS]; // �������б�

#endif
