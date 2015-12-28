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

#ifndef __TEST_DEBUG_CM__ // 测试透传功能
//#define __TEST_DEBUG_CM__
#endif
#endif

#define NETWORT_INTERFACE "eth0"

enum useful_enums
{
	PIPE_RD = 0,
	PIPE_WR = 1,
	POLL_COUNT = 2,
	TIME_PERIOD_25_MILLISECONDS = 25
};
       
struct fds
{
	int raw_fd;		// raw socket
	int udp_server_fd;	// udp server
	int udp_client_fd;	// udp client
	int tx_pipe[2];		// pipe
};

struct udp_server
{
	int s_fd;
	struct sockaddr_in srvaddr;
	socklen_t srvlen;
};

struct udp_client
{
	int c_fd;
	struct sockaddr_in cltaddr;
	socklen_t cltlen;
};

struct udp_context
{
	struct udp_server udp_srv;
	struct udp_client udp_clt;
};

#define THREADS_MUX_NUM 16
struct threads_info
{
	pthread_t tid[THREADS_MUX_NUM];
	int pthread_nums;
};

extern struct fds net_fd;						// 网络通信套接字与线程间通信套接字
extern struct udp_client upper_udp_client;		// 上位机的通信信息
extern struct udp_server pc_controller_server;	// 主机显示信息与摄像头控制器的通信信息 
extern solid_pdblist endpoint_list;				// 系统中终端链表哨兵节点
extern inflight_plist command_send_guard;		// 系统中发送网络数据命令链表哨兵节点
extern struct raw_context net;				// 原始套接字
extern desc_pdblist descptor_guard;			// 系统中描述符链表哨兵节点
const proccess_func_items proccess_func_link_tables[MAX_FUNC_LINK_ITEMS]; // 处理函数列表

#endif
