/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifdef __SSV_UNIX_SIM__
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char	ssv_type_u8;

int get_eth0_as_mac(ssv_type_u8 *mac)
{
    struct ifreq s;
    int ret = 1;
    int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    strcpy(s.ifr_name, "eth0");
    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
        int i;
        for (i = 2; i < 6; ++i)
            mac[i]=(ssv_type_u8)s.ifr_addr.sa_data[i];

        ret = 0;
    }
    //lint -e746 -e718
    close(fd);
    return ret;
}
#endif//__SSV_UNIX_SIM__

