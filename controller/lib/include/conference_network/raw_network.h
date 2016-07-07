/*
  *file:raw_network.h
  *Author:梁永富
  *Build data:2016-07-07
  *descriptor:1722网络实现的实际的接口文件，若需改接收1722协议数据
  *			的端口，修改其接口的实现即可
  */
#ifndef __RAW_NETWORK_H__
#define __RAW_NETWORK_H__

// ==========================================
//{@修改即可实现不同接口的用户信息，
struct etherII
{
	uint8_t destmac[6];
	uint8_t srcmac[6];
	uint8_t type[2];
};

struct ipheader
{
	uint8_t ver_len;
	uint8_t service;
	uint8_t len[2];
	uint8_t ident[2];
	uint8_t flags;
	uint8_t frag_offset[2];
	uint8_t ttl;
	uint8_t protocol;
	uint8_t chksum[2];
	uint8_t sourceip[4];
	uint8_t destip[4];
};

struct udpheader
{
	uint8_t srcport[2];
	uint8_t destport[2];
	uint8_t len[2];
	uint8_t chksum[2];
};

struct raw_socket_user_info
{
	struct ipheader ip_hdr_store;
	struct udpheader udp_hdr_store;
	uint32_t total_devs;
	int rawsock;
	int ifindex;
	uint64_t mac;
	uint16_t ethertype;
	char ifnames[10][256];
};
// ==========================================@}

typedef struct raw_socket_user_info raw_net_1722_user_info;// 1722端口接口用户信息
extern int raw_network_init(void**user_obj);
extern int raw_network_send(void* frame, int mem_buf_len, const void *user_obj);
extern int raw_network_recv(void* frame, int recv_buf_Len, const void *user_obj);
extern int raw_network_cleanup(void **user_obj);

#endif
