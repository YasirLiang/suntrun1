#ifndef __AVDECC_NET_H__
#define __AVDECC_NET_H__

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/un.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>

#include "jdksavdecc_world.h"
#include "avdecc_main.h"
#include "host_controller_debug.h"

#define SRV_IP "192.168.0.102"
#define SRV_PORT 7012

#define CLT_IP "192.168.0.102"
#define CLT_PORT 7990


#define ETHER_HDR_SIZE 14
#define ZERO_OFFSET_IN_PAYLOAD 0

int send_udp_frame(int fd, void *pout, int length, const struct sockaddr_in* sin);
int recv_udp_packet( int fd, void *pout, int length, struct sockaddr_in *sin, socklen_t *sin_length );
int conference_host_raw_receive( int sockfd, uint16_t *ethertype, uint8_t src_mac[6], uint8_t dest_mac[6], void *payload_buf, ssize_t payload_buf_max_size  );
int udp_socket( struct udp_context *self, const int port, bool isserver, const char *interface);
void build_socket( struct fds *all_fds, struct raw_context *self, const char *network_port, struct udp_context *both );

#endif

