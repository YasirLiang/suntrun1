/*
* @file avdecc_net.c
* @brief
* @ingroup
* @cond
******************************************************************************
* Last updated for version 1.0.0
* Last updated on  2017-03-06
*
*                    Moltanisk Liang
*                    ---------------------------
*                    avb auto control system
*
* Copyright (C) Moltanisk Liang, GuangZhou Suntron. All rights reserved.
******************************************************************************
* @endcond
*/
/*including head files------------------------------------------------------*/
#include "avdecc_net.h"
#include <linux/ethtool.h>
#include <linux/sockios.h>
/*$ see NOTE1---------------------------------------------------------------*/
int Ethtool_getNetCart(const char* devname, int* speed,
    int* duplex, int* autoneg, int* link)  
{  
    struct ifreq ifr;
    int fd = 0;
    int err = -1;

    struct ethtool_cmd ecmd;
    struct ethtool_value edata;

    if (devname == NULL) {
        return -2;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("ethtool_gset Cannot get control socket");
        return -1;
    }

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    err = ioctl(fd, SIOCETHTOOL, &ifr);

    if (err < 0) {
        perror("Cannot get device settings");
        return -1;
    }

    printf("PHY xx - %d/%s ", ecmd.speed,
        (ecmd.duplex == DUPLEX_FULL) ? "Full" : "Half");
    printf(" Auto-negotiation: %s ",
        (ecmd.autoneg == AUTONEG_DISABLE) ? "off" : "on");

    switch (ecmd.speed) {
        case SPEED_10:
        case SPEED_100:
        case SPEED_1000:
        case SPEED_2500:
        case SPEED_10000:
            *speed = ecmd.speed;
        break;
        default:
            fprintf(stdout, "Unknown! (%i)\n", ecmd.speed);
        break;
    };

    switch (ecmd.duplex) {
        case DUPLEX_HALF:
        case DUPLEX_FULL:
            *duplex = ecmd.duplex;
        break;
        default:  
            fprintf(stdout, "Unknown! (%i)\n", ecmd.duplex);
        break;
    };
    *autoneg = ecmd.autoneg;

    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err == 0) {
        *link = edata.data ? 1: 0;
        printf(" %s\n", edata.data ? "Up" : "Down");
    }  
    else if (errno != EOPNOTSUPP) {
        perror("Cannot get link status");
    }

    close(fd);

    return 0;  
}  
/*$ see NOTE1---------------------------------------------------------------*/
int Ethtool_setNetCart(const char* devname, int speed,
    int duplex, int autoneg)
{
    int speed_wanted = -1;
    int duplex_wanted = -1;
    int autoneg_wanted = AUTONEG_ENABLE;
    int advertising_wanted = -1;
    struct ethtool_cmd ecmd;
    struct ifreq ifr;
    int fd = 0;
    int err = -1;

    /* pass args */
    if (devname == NULL)  {
        printf("devname emtpy...\n");
        return -2;
    }

    speed_wanted = speed;
    duplex_wanted = duplex;
    autoneg_wanted = autoneg;

    strcpy(ifr.ifr_name, devname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Ethtool_setNetCart Cannot get control socket");
        return -1;
    }

    ecmd.cmd = ETHTOOL_GSET;
    ifr.ifr_data = (caddr_t)&ecmd;  
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err < 0) {
        perror("Cannot get current device settings");
        return -1;
    }

    if (speed_wanted != -1) {
        ecmd.speed = speed_wanted;
    }
    if (duplex_wanted != -1) {
        ecmd.duplex = duplex_wanted;
    }
    if (autoneg_wanted != -1) {
        ecmd.autoneg = autoneg_wanted;
    }

    if ((autoneg_wanted == AUTONEG_ENABLE)
          && (advertising_wanted < 0))
    {
        if ((speed_wanted == SPEED_10)
              && (duplex_wanted == DUPLEX_HALF))
        {
            advertising_wanted = ADVERTISED_10baseT_Half;
        }
        else if ((speed_wanted == SPEED_10)
                      && (duplex_wanted == DUPLEX_FULL))
        {
            advertising_wanted = ADVERTISED_10baseT_Full;
        }
        else if ((speed_wanted == SPEED_100)
                      && (duplex_wanted == DUPLEX_HALF))
        {
            advertising_wanted = ADVERTISED_100baseT_Half;
        } 
        else if ((speed_wanted == SPEED_100)
                      && (duplex_wanted == DUPLEX_FULL))
        {
            advertising_wanted = ADVERTISED_100baseT_Full;
        }
        else if ((speed_wanted == SPEED_1000)
                      && (duplex_wanted == DUPLEX_HALF))
        {
            advertising_wanted = ADVERTISED_1000baseT_Half;
        }
        else if ((speed_wanted == SPEED_1000)
                      &&  (duplex_wanted == DUPLEX_FULL))
        {
            advertising_wanted = ADVERTISED_1000baseT_Full;
        }
        else if ((speed_wanted == SPEED_2500)
                      &&  (duplex_wanted == DUPLEX_FULL))
        {
            advertising_wanted = ADVERTISED_2500baseX_Full;
        }
        else if ((speed_wanted == SPEED_10000)
                    &&  (duplex_wanted == DUPLEX_FULL)) {
            advertising_wanted = ADVERTISED_10000baseT_Full;
        }
        else  {
            advertising_wanted = 0;
        }
    }  

    if (advertising_wanted != -1) {
        if (advertising_wanted == 0) {
            ecmd.advertising = ecmd.supported &
            (ADVERTISED_10baseT_Half |
            ADVERTISED_10baseT_Full |
            ADVERTISED_100baseT_Half |
            ADVERTISED_100baseT_Full |
            ADVERTISED_1000baseT_Half |
            ADVERTISED_1000baseT_Full |
            ADVERTISED_2500baseX_Full |
            ADVERTISED_10000baseT_Full);
        }
        else  {
            ecmd.advertising = advertising_wanted;
        }
    }

    ecmd.cmd = ETHTOOL_SSET;
    ifr.ifr_data = (caddr_t)&ecmd;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err < 0) {
        perror("Cannot set new settings");
    }

    if (err < 0) {
        if (speed_wanted != -1) {
            fprintf(stderr, "  not setting speed\n");
        }

        if (duplex_wanted != -1) {
            fprintf(stderr, "  not setting duplex\n");
        }

        if (autoneg_wanted != -1) {
            fprintf(stderr, "  not setting autoneg\n");
        }

        close(fd);
        return -1;
    }

    close(fd);

    return 0;
} 
/* detect_mii().............................................................*/
int detect_mii(int skfd, char *ifname) {
   struct ifreq ifr;
   uint16_t *data, mii_val;
   unsigned phy_id;

   /* Get the vitals from the interface. */
   strcpy(ifr.ifr_name, ifname);
   if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
        fprintf(stderr, "SIOCGMIIPHY on %s failed: %s\n", ifname,
            strerror(errno));
        (void) close(skfd);
        return 2;
    }

   data = (uint16_t *)(&ifr.ifr_data);
   phy_id = data[0];
   data[1] = 1;

   if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
        fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n",
            ifr.ifr_name, strerror(errno));
        return 2;
    }

   mii_val = data[3];
   return(((mii_val & 0x0016) == 0x0004) ? 0 : 1);
}
/* detect_ethtool().........................................................*/
int detect_ethtool(int skfd, char *ifname);
int detect_ethtool(int skfd, char *ifname) {
    struct ifreq ifr;
    struct ethtool_value edata;
    
    memset(&ifr, 0, sizeof(ifr));
    edata.cmd = ETHTOOL_GLINK;
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
    ifr.ifr_data = (char *)&edata;

    if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1) {
        printf("ETHTOOL_GLINK failed: %s\n", strerror(errno));
        return 2;
    }

    return (edata.data ? 0 : 1);
}
/* this is for interface phy link status */
int check_ifrPhyLinkStatus(char *nicName) {
    int ret = -1;
    int fd = -1;
    
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket error\n");
    }
    
    ret = detect_ethtool(fd, nicName);
    if (ret == 2) {
        ret = detect_mii(fd, nicName);
    }

    close(fd);

    /* ret value:
         2 Could not determine status
         1 Link down
         0 Link up*/
    return ret;
}
/*$ send_udp_frame()........................................................*/
int send_udp_frame(int fd, void *pout,
    int length, const struct sockaddr_in *sin)
{
    int send_len;

    assert(sin && pout);

    send_len = sendto(fd, pout, length, 0,
        (struct sockaddr*)sin, sizeof(struct sockaddr_in));
    if( send_len < 0)
    {
        DEBUG_ERR("udp send failed!");
        assert(send_len >= 0);
    }

    return send_len;
}
/*$ recv_udp_packet().......................................................*/
int recv_udp_packet(int fd, void *pout, int length,
    struct sockaddr_in *sin, socklen_t *sin_length)
{
    assert(pout && sin && sin_length);

    int get_len = 0;
    get_len = recvfrom(fd, pout, length, 0,
        ( struct sockaddr * )sin, sin_length);
    if (get_len >= 0) {
        return get_len;
    }
    else {
        DEBUG_ERR("receive udp packet error!");
        assert(get_len >= 0);
    }

    return get_len;
}
/*$ conference_host_raw_receive()...........................................*/
int conference_host_raw_receive(int sockfd, uint16_t *ethertype,
    uint8_t src_mac[6], uint8_t dest_mac[6],
    void *payload_buf, ssize_t payload_buf_max_size)
{
    ssize_t r = -1;
    ssize_t buf_len;
    uint8_t buf[2048];

    buf_len = recv( sockfd, buf, sizeof( buf ), 0 );
    if (buf_len >= 0) {
        if (src_mac) {
            memcpy( src_mac, &buf[6], 6 );
        }
        
        if (dest_mac) {
            memcpy( dest_mac, &buf[0], 6 );
        }
        
        if ((payload_buf)
              && (payload_buf_max_size > buf_len - 14))
        {
            *ethertype = ( ( buf[12] << 8 ) & 0xff00 ) + buf[13];
            memcpy( payload_buf, &buf[14], buf_len - 14 );
            r = buf_len - 14;
        }
    }
    else {
        DEBUG_ERR("raw recv failed!");
        assert( buf_len >= 0 );
    }

    return r;
}
/*$ AvdeccNet_udpSocket()...................................................*/
static int AvdeccNet_udpSocket(struct udp_context *self, const int port,
    bool isserver, const char *interface);
static int AvdeccNet_udpSocket(struct udp_context *self, const int port,
    bool isserver, const char *interface)
{
    int fd = -1;
    int ret = -1;
    struct ifreq ifr;
    char ip[32] = {0};
    struct sockaddr_in sin;
    struct socket_info_s *socket_class = NULL;

    fd = socket(AF_INET,  SOCK_DGRAM, 0);
    PERROR( fd, "socket" );

    int on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    PERROR(ret, "setsockopt");

    strcpy(ifr.ifr_name, interface);
    /* get mac */
    if((ret = ioctl( fd, SIOCGIFHWADDR, &ifr)) < 0 ) {
        PERROR(ret, "ioctl siocgifhwaddr:");
        exit(-1);
    }

    /* get ip */
    if ((ret = ioctl ( fd, SIOCGIFADDR, &ifr )) < 0) {
        close (fd);
        PERROR(ret, "ioctl SIOCGIFADDR:");
    }
    memcpy(&sin, &ifr.ifr_addr, sizeof( sin ));
    strcpy(ip, inet_ntoa(sin.sin_addr));

    if (isserver) {
        socket_class = &self->udp_srv;
    }
    else {
        socket_class = &self->udp_clt;
    }

    socket_class->sock_fd = fd;
    socket_class->sock_len = sizeof(struct sockaddr_in);
    socket_class->sock_addr.sin_family = AF_INET;
    socket_class->sock_addr.sin_port = htons(port);
    inet_aton(ip, &socket_class->sock_addr.sin_addr);
    ret = bind(fd, (struct sockaddr*)&(socket_class->sock_addr),
    socket_class->sock_len);
    if (ret < -1) {
        PERROR( ret, "bind" );
    }

    DEBUG_INFO("host %s bind-> address = %s: %d-> sfd =  %d",
        isserver ? "server" : "client",\
        inet_ntoa( socket_class->sock_addr.sin_addr ), \
        ntohs( socket_class->sock_addr.sin_port ),
            socket_class->sock_fd );

    return fd;
}
/*$ AvdeccNet_buildSocket().................................................*/
void AvdeccNet_buildSocket(struct fds *all_fds, struct raw_context *self,
    const char *network_port, struct udp_context *both)
{
    all_fds->udp_server_fd = AvdeccNet_udpSocket(both, SRV_PORT,
        true, network_port);
    all_fds->udp_client_fd = AvdeccNet_udpSocket(both, CLT_PORT,
        false, network_port);
}
/*$ AvdeccNet_init()........................................................*/
void AvdeccNet_init(const char *network_port) {
    int speed = 0;
    int duplex = 0;
    int autoneg = 0;
    int link = 0;
    
    Ethtool_getNetCart(network_port, &speed, &duplex, &autoneg, &link);

    /* set 100M\full\enable autoneg */
    speed = SPEED_100;
    duplex = DUPLEX_FULL;
    autoneg = AUTONEG_ENABLE;
    Ethtool_setNetCart(network_port, speed, duplex, autoneg);

    speed = 0;
    duplex = 0;
    autoneg = 0;
    link = 0;
    Ethtool_getNetCart(network_port, &speed, &duplex, &autoneg, &link);
}

/*****************************************************************************
NOTE1
    When setting Fast or Gigabit,
    if open autoneg in the meantime, network will be offline and
    connect again;otherwise, network will be not offline, but it
    will be invalid from Gigabit to Fast.Therefore, it's default setting
    is enable autoneg.
*****************************************************************************/

