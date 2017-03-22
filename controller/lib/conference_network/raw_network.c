#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <errno.h>
#include <strings.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/select.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <linux/filter.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "jdksavdecc_world.h"
#include "avdecc_main.h"
#include "raw_network.h"

/* Code generated from ./mkbpf in this dir */
static int ethernet_proto_bpf[1] = {
	0x22f0
};
/*
tcpdump -d  ether proto 0x22f0 or vlan 2
(000) ldh      [12]
(001) jeq      #0x22f0          jt 6	jf 2
(002) jeq      #0x8100          jt 3	jf 7
(003) ldh      [14]
(004) and      #0xfff
(005) jeq      #0x2             jt 6	jf 7
(006) ret      #65535
(007) ret      #0
tcpdump -dd  ether proto 0x22f0 or vlan 2
*/
struct sock_filter BPF_code[] = {
#if 0
{ 0x28, 0, 0, 0x0000000c },
{ 0x15, 4, 0, 0x000022f0 },
{ 0x15, 0, 4, 0x00008100 },
{ 0x28, 0, 0, 0x0000000e },
{ 0x54, 0, 0, 0x00000fff },
{ 0x15, 0, 1, 0x00000002 },
{ 0x6, 0, 0, 0x0000ffff },
{ 0x6, 0, 0, 0x00000000 },
#else
{ 0x28, 0, 0, 0x0000000c },
{ 0x15, 0, 1, 0x000022f0 },
{ 0x6, 0, 0, 0x0000ffff },
{ 0x6, 0, 0, 0x00000000 },
#endif
};

static void convert_eui48_to_uint64_raw(const uint8_t value[6], uint64_t *new_value)
{
	*new_value = 0;
	uint32_t i = 0;

	for( i = 0; i < 6; i++)
	{
		*new_value |= (uint64_t) value[i] << ((5 - i) * 8);
	}
}

static void net_interface_imp(raw_net_1722_user_info* user_info)
{
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];
        char ifname[256]={0};

        user_info->total_devs = 0;
        if (getifaddrs(&ifaddr) == -1)
        {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

        /* Walk through linked list, maintaining head pointer so we
        can free list later */
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
                continue;

            family = ifa->ifa_addr->sa_family;

            /* Display interface name and family (including symbolic
            form of the latter for the common families) */
            if (family == AF_INET)
            {
                s = getnameinfo(ifa->ifa_addr,
                                (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                sizeof(struct sockaddr_in6),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0)
                {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }
		memset( ifname, sizeof(ifname), 0);
		snprintf(ifname, sizeof(ifname), "%s, address: <%s>",ifa->ifa_name, host);
		memcpy(user_info->ifnames[user_info->total_devs], ifname, sizeof(ifname));
                user_info->total_devs++;
            }
        }

        freeifaddrs(ifaddr);
 }

static int getifindex(int rawsock, const char *iface)
{
        struct ifreq ifr;
        int ret;

        memset(&ifr,0,sizeof(ifr));
        strncpy(ifr.ifr_name,iface,sizeof(ifr.ifr_name));

        ret=ioctl(rawsock,SIOCGIFINDEX,&ifr);

        if(ret<0)
        {
            return ret;
        }

        return ifr.ifr_ifindex;
}

static int setpromiscuous(int rawsock, int ifindex)
{
        struct packet_mreq mr;

        memset(&mr,0,sizeof(mr));
        mr.mr_ifindex=ifindex;
        mr.mr_type=PACKET_MR_ALLMULTI;

        return setsockopt(rawsock, SOL_PACKET,
                          PACKET_ADD_MEMBERSHIP,&mr,sizeof(mr));
}

static int set_capture_ether_type(uint16_t *ether_type, uint32_t count, int rawsock)
{
        struct sock_fprog Filter;
	unsigned int i = 0;

        Filter.len = sizeof(BPF_code) / 8;
        Filter.filter = BPF_code;

        for (i = 0; i < count; i++)
        {
            if (ether_type[i] != (uint16_t)ethernet_proto_bpf[i])
                fprintf(stderr, "NETIF - packet filter mismatch\n");
        }

        // attach filter to socket
        if(setsockopt(rawsock, SOL_SOCKET, SO_ATTACH_FILTER, &Filter, sizeof(Filter)) == -1)
        {
            fprintf(stderr, "socket attach filter failed! %s\n", strerror(errno));
            close(rawsock);
            exit(EXIT_FAILURE);
        }

        return 0;
}

static int select_interface_by_num(uint32_t interface_num, raw_net_1722_user_info* user_info)
{
        struct sockaddr_ll sll;
        struct ifreq if_mac;
        const char *ifname;
        char *s;

        /* adjust interface numnber since count starts at 1 */
        interface_num--;

        ifname = user_info->ifnames[interface_num];
        s = (char *)ifname;
        while(*s != ',')
        {
            s++;
        }
        *s = 0;
        user_info->rawsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (user_info->rawsock == -1)
        {
            fprintf(stderr, "Socket open failed! %s\nuse sudo?\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        user_info->ifindex = getifindex(user_info->rawsock, ifname);
	
        // get the mac address of the eth0 interface
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, ifname, IFNAMSIZ-1);
        if (ioctl(user_info->rawsock, SIOCGIFHWADDR, &if_mac) < 0)
        {
            perror("SIOCGIFHWADDR");
            exit(EXIT_FAILURE);
        }

        setpromiscuous(user_info->rawsock, user_info->ifindex);

        memset(&sll,0, sizeof(sll));
        sll.sll_family = AF_PACKET;
        sll.sll_ifindex = user_info->ifindex;
        sll.sll_protocol = htons(ETH_P_ALL);
        bind(user_info->rawsock, (struct sockaddr *)&sll, sizeof(sll));

        convert_eui48_to_uint64_raw((uint8_t *)if_mac.ifr_hwaddr.sa_data, &user_info->mac);

        uint16_t etypes[1] = {0x22f0};
        set_capture_ether_type(etypes, 1, user_info->rawsock);
	user_info->ethertype = etypes[0];

#if 0// 改变套接字的缓冲区大小		
	int snd_size = 0;
	int rcv_size = 0;
	socklen_t optlen;
	int err = -1;

	optlen = sizeof(snd_size); 
	err = getsockopt(user_info->rawsock, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen); 
	if(err<0){ 
	}

	optlen = sizeof(rcv_size); 
	err = getsockopt(user_info->rawsock, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen); 
	if(err<0){ 
	} 

	printf("before send size = %d\n",snd_size); 
	printf("before recv size %d\n",rcv_size); 

	snd_size = 10*1024;    /* 发送缓冲区大小为8K */ 
	optlen = sizeof(snd_size); 
	err = setsockopt(user_info->rawsock, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen); 
	if(err<0){ 
	} 

	rcv_size = 200*1024;    /* 接收缓冲区大小为8K */ 
	optlen = sizeof(rcv_size); 
	err = setsockopt(user_info->rawsock,SOL_SOCKET,SO_RCVBUF, (char *)&rcv_size, optlen); 
	if(err<0){ 
	} 

	optlen = sizeof(snd_size); 
	err = getsockopt(user_info->rawsock, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen); 
	if(err<0){ 
	}   

	optlen = sizeof(rcv_size); 
	err = getsockopt(user_info->rawsock, SOL_SOCKET, SO_RCVBUF,(char *)&rcv_size, &optlen); 
	if(err<0){ 
	} 

	printf("after send size = %d\n",snd_size); 
	printf("after recv size = %d\n",rcv_size); 
#endif
        return 0;
}


static int print_interfaces_and_select(char *interface, raw_net_1722_user_info* user_info)
{
    int interface_num = -1;
    uint32_t i = 1;

    for(i = 1; i < user_info->total_devs + 1; i++)
    {
        char *dev_desc = user_info->ifnames[i-1];
        if (!interface)
        {
            printf("%d (%s)\n", i, dev_desc);
        }
        else
        {
            if (strcmp(dev_desc, interface) == 0)
            {
                interface_num = i;
                break;
            }
        }
    }

    if (!interface)
    {// 注:在这里可以通过命令设置interface_num
        interface_num = 2;
    }
    else if(interface_num == -1)
    {// not found, use default interface number 2.
    	interface_num = 2;
    }

    printf("Your Interface num = %d", interface_num);
    select_interface_by_num(interface_num, user_info);

    return 0;
}

/*有具体的接口提供，作用是返回raw_network的信息供外部使用*/ 
static void raw_network_user_object_create(void **user_obj)
{
	if (NULL != user_obj)
	{
		raw_net_1722_user_info* user_info_node = (raw_net_1722_user_info*)malloc(sizeof(raw_net_1722_user_info));
		if (NULL != user_info_node)
		{
			memset( user_info_node, sizeof(raw_net_1722_user_info), 0);
			*user_obj = user_info_node;
		}
		else
			*user_obj = NULL;// failed create, set to NULL.
	}
}

static void raw_network_user_object_destroy(void **user_obj)
{
	if (*user_obj != NULL)
	{
		free(*user_obj);
		*user_obj = NULL;
	}
}

int raw_network_init(void**user_obj)
{
	raw_network_user_object_create(user_obj);
	if (*user_obj != NULL)
	{// create user obj success!
		net_interface_imp((raw_net_1722_user_info*)(*user_obj));
		print_interfaces_and_select("eth0, address: <192.168.0.238>", (raw_net_1722_user_info*)(*user_obj));
	}
	
	return 0;
}

int raw_network_send(void* frame, int mem_buf_len, const void *user_obj)
{
	 int send_result;
	 const raw_net_1722_user_info* usr_info_node =  (raw_net_1722_user_info*)user_obj;

	 if (usr_info_node == NULL)
	 	return -1;

        /*target address*/
        struct sockaddr_ll socket_address;

        /*prepare sockaddr_ll*/

        /*RAW communication*/
        socket_address.sll_family   = PF_PACKET;
        socket_address.sll_protocol = htons(usr_info_node->ethertype);

        /*index of the network device
        see full code later how to retrieve it*/
        socket_address.sll_ifindex  = usr_info_node->ifindex;

        /*ARP hardware identifier is ethernet*/
        socket_address.sll_hatype = ARPHRD_ETHER;

        /*target is another host*/
        socket_address.sll_pkttype = PACKET_OTHERHOST;

        /*address length*/
        socket_address.sll_halen = ETH_ALEN;
        /*MAC - begin*/
	uint8_t *p_frame = (uint8_t*)frame;
        memcpy(&socket_address.sll_addr[0], &p_frame[0], 6);
        /*MAC - end*/
        socket_address.sll_addr[6]  = 0x00;/*not used*/
        socket_address.sll_addr[7]  = 0x00;/*not used*/

        /*send the packet*/
        send_result = sendto(usr_info_node->rawsock, frame, mem_buf_len, 0,
                             (struct sockaddr*)&socket_address, sizeof(socket_address));

        return send_result;
}

int raw_network_recv(void* frame, int recv_buf_Len, const void *user_obj)
{
	int recv_length = -1;
	const raw_net_1722_user_info* usr_info_node =  (raw_net_1722_user_info*)user_obj;

	if (usr_info_node == NULL)
	 	return -1;


	recv_length = recv(usr_info_node->rawsock, frame, recv_buf_Len, 0);

	return recv_length;
}

int raw_network_cleanup(void **user_obj)
{
	close(((raw_net_1722_user_info*)(*user_obj))->rawsock);
	raw_network_user_object_destroy(user_obj);

	return 0;
}

