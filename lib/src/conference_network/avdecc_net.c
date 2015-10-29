#include "avdecc_net.h"

int send_udp_frame(int fd, void *pout, int length, const struct sockaddr_in *sin)
{
	assert( sin && pout );
	
	int send_len = 0;
	send_len = sendto( fd, pout, length, 0, (struct sockaddr*)sin, sizeof(struct sockaddr_in));
	if( send_len < 0)
	{
		DEBUG_ERR("udp send failed!");
		assert(send_len >= 0);
	}

	return send_len;
}

int recv_udp_packet( int fd, void *pout, int length, struct sockaddr_in *sin, socklen_t *sin_length )
{
	assert( pout && sin && sin_length );

	int get_len = 0;
	get_len = recvfrom( fd, pout, length, 0, ( struct sockaddr * )sin, sin_length );
	if( get_len >= 0 )
	{
		//DEBUG_INFO("Dest address = %s:%d", inet_ntoa( sin->sin_addr ), ntohs( sin->sin_port ));
		//DEBUG_RECV( (uint8_t*)pout, get_len,"UDP PACKET" );
		return get_len;
	}
	else
	{
		DEBUG_ERR("receive udp packet error!");
		assert( get_len >= 0 );
	}

	return get_len;
}

int conference_host_raw_receive( int sockfd, uint16_t *ethertype, uint8_t src_mac[6], uint8_t dest_mac[6], void *payload_buf, ssize_t payload_buf_max_size  )
{
	ssize_t r = -1;
	ssize_t buf_len;
	uint8_t buf[2048];

	buf_len = recv( sockfd, buf, sizeof( buf ), 0 );
	if ( buf_len >= 0 )
	{
		if ( src_mac )
		{
			memcpy( src_mac, &buf[6], 6 );
		}
		if ( dest_mac )
		{
			memcpy( dest_mac, &buf[0], 6 );
		}
		if ( payload_buf && ( payload_buf_max_size > buf_len - 14 ) )
		{
			*ethertype = ( ( buf[12] << 8 ) & 0xff00 ) + buf[13];
			memcpy( payload_buf, &buf[14], buf_len - 14 );
			r = buf_len - 14;
		}
	}
	else
	{
		DEBUG_ERR("raw recv failed!");
		assert( buf_len >= 0 );
	}
	
	return r;
}

// 创建主机与上位机，主机与pc控制主机之间的套接字
int udp_socket( struct udp_context *self, const int port, bool isserver, const char *interface)
{
	int fd;
	int ret;
	struct ifreq ifr;
	struct sockaddr_in sin;
	char ip[32] = {0};
	
	fd = socket( AF_INET,  SOCK_DGRAM, 0 );
	if( fd == -1 )
	{
		DEBUG_INFO("udp socket failed!" );
		assert(fd != -1);
	}

	strcpy(ifr.ifr_name, interface);
	if( (ret = ioctl( fd, SIOCGIFHWADDR, &ifr )) < 0 )	// 获取本机mac地址
	{
		DEBUG_ERR("ioctl siocgifhwaddr:");
		assert( ret >=  0 );
	}

	//获取本机ip地址
	if ( (ret = ioctl ( fd, SIOCGIFADDR, &ifr )) < 0 )
	{
		DEBUG_ERR("ioctl SIOCGIFADDR:");
		close ( fd );
		assert( ret >=  0 );
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof( sin ));
	strcpy(ip, inet_ntoa(sin.sin_addr));

	if( isserver )
	{
		self->udp_srv.s_fd = fd;
		self->udp_srv.srvlen = sizeof( struct sockaddr_in );
		self->udp_srv.srvaddr.sin_family = AF_INET;
		self->udp_srv.srvaddr.sin_port = htons( port );
		inet_aton( ip, &self->udp_srv.srvaddr.sin_addr );
		ret = bind( fd, (struct sockaddr *)&(self->udp_srv.srvaddr), \
				self->udp_srv.srvlen );
		if( ret == -1 )
		{
			DEBUG_ERR( "udp server:" );
			DEBUG_ONINFO( "erro udp bind" );
			assert( ret != -1);
		}
		
		DEBUG_INFO("host server bind-> address = %s:%d sfd =  %d", inet_ntoa( self->udp_srv.srvaddr.sin_addr ), ntohs( self->udp_srv.srvaddr.sin_port ), self->udp_srv.s_fd);
	}
	else
	{
		self->udp_clt.c_fd = fd;
		self->udp_clt.cltlen = sizeof( struct sockaddr_in );
		self->udp_clt.cltaddr.sin_family = AF_INET;
		self->udp_clt.cltaddr.sin_port = htons( port );
		inet_aton( ip, &self->udp_clt.cltaddr.sin_addr );
		ret = bind( fd, (struct sockaddr *)&self->udp_clt.cltaddr, \
				self->udp_clt.cltlen);
		if( ret == -1 )
		{
			DEBUG_ERR( "udp client:" );
			DEBUG_ONINFO( "erro udp bind" );
			assert( ret != -1);
		}

		DEBUG_INFO("host client bind-> address = %s:%d sfd =  %d", inet_ntoa( self->udp_srv.srvaddr.sin_addr ), ntohs( self->udp_clt.cltaddr.sin_port ), self->udp_clt.c_fd);
	}
	
	return fd;
}

void build_socket( struct fds *all_fds, struct raw_context *self, const char *network_port, struct udp_context *both )
{
	all_fds->raw_fd = raw_socket( self, JDKSAVDECC_AVTP_ETHERTYPE, network_port, jdksavdecc_multicast_adp_acmp.value );
	all_fds->udp_server_fd = udp_socket( both, SRV_PORT, true,  network_port );
	all_fds->udp_client_fd = udp_socket( both, CLT_PORT, false, network_port );
		
	return;
}
