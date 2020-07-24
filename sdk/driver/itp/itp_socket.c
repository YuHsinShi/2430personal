/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
 /** @file
  * PAL socket functions.
  *
  * @author Jim Tan
  * @version 1.0
  */
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include "lwip/tcpip.h"
#include "itp_cfg.h"
#include "ifaddrs.h"

  /*  IPv6 constants for use in structure assignments (RFC 2553).  */
const struct in6_addr in6addr_any = { { IN6ADDR_ANY_INIT } };
const struct in6_addr in6addr_loopback = { { IN6ADDR_LOOPBACK_INIT } };

#ifndef CFG_IOT_ENABLE
#ifdef __BSD_VISIBLE

int fcntl(int fd, int cmd, ...)
{
    va_list args;
    int rv;

    va_start(args, cmd);
    rv = lwip_fcntl(fd, cmd, va_arg(args, int));
    va_end(args);
    return rv;
}
int select(int __n, fd_set *__readfds, fd_set *__writefds,
    fd_set *__exceptfds, struct timeval *__timeout)
{
    return lwip_select(__n, __readfds, __writefds, __exceptfds, __timeout);
}
#else

int itpSocketFcntl(int fd, int cmd, int arg)
{
    if ((fd & (0xFF << 8)) == 0)
    {
        return lwip_fcntl(fd, cmd, arg);
    }
    return 0;
}

int itpSocketSelect(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
    struct timeval *timeout)
{
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif // __BSD_VISIBLE
#endif /* NON_CFG_IOT_ENABLE */

static int SocketClose(int file, void* ptr)
{
#ifdef CFG_IOT_ENABLE
	return close(file);
#else
    return lwip_close(file);
#endif
}

static int SocketRead(int file, char *ptr, int len, void* info)
{
#ifdef CFG_IOT_ENABLE
    return recv(file, ptr, len, 0);
#else
	return lwip_recv(file, ptr, len, 0);
#endif
}

static int SocketWrite(int file, char *ptr, int len, void* info)
{
#ifdef CFG_IOT_ENABLE
    return send(file, ptr, len, 0);
#else
	return lwip_send(file, ptr, len, 0);
#endif
}

static int SocketGetInterfaceFlags(void* ptr)
{
    struct ifreq* ifr = (struct ifreq*)ptr;

#ifdef CFG_NET_ETHERNET
    {
        ITPEthernetInfo info;
        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            ifr->ifr_ifru.ifru_flags[0] = 0;
            if (info.flags & ITP_ETH_ACTIVE)
                ifr->ifr_ifru.ifru_flags[0] |= IFF_UP;

            return 0;
        }

#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE    
        info.index = 1;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            ifr->ifr_ifru.ifru_flags[0] = 0;
            if (info.flags & ITP_ETH_ACTIVE)
                ifr->ifr_ifru.ifru_flags[0] |= IFF_UP;

            return 0;
        }

#endif        

    }
#endif // CFG_NET_ETHERNET

#ifdef CFG_NET_WIFI
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 1)
    {
        ITPWifiInfo info;  //James Modify
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            ifr->ifr_ifru.ifru_flags[0] = 0;
            if (info.active)
                ifr->ifr_ifru.ifru_flags[0] |= IFF_UP;

            return 0;
        }
    }
#endif // CFG_NET_WIFI

#ifdef CFG_NET_WIFI_HOSTAPD
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 2)
    {
        ITPWifiInfo info;  //James Modify
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            ifr->ifr_ifru.ifru_flags[0] = 0;
            if (info.active)
                ifr->ifr_ifru.ifru_flags[0] |= IFF_UP;

            return 0;
        }
    }
#endif // CFG_NET_WIFI_HOSTAPD

    return -1;
}

static int SocketGetInterfaceAddress(void* ptr)
{
    struct ifreq* ifr = (struct ifreq*)ptr;

#ifdef CFG_NET_ETHERNET
    {
        ITPEthernetInfo info;
        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            return 0;
        }
#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
        info.index = 1;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            printf("eth[1] %s \n", utils_inet_ntoa(info.address));
            return 0;
        }
#endif        
    }
#endif // CFG_NET_ETHERNET

#ifdef CFG_NET_WIFI
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 1)
    {
        ITPWifiInfo info;
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            return 0;
        }
    }
#endif // CFG_NET_WIFI

#ifdef CFG_NET_WIFI_HOSTAPD
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 2)
    {
        ITPWifiInfo info;
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            return 0;
        }
    }
#endif // CFG_NET_WIFI_HOSTAPD

    return -1;
}

static int SocketGetInterfaceConfig(void* ptr)
{
    struct ifconf* ifc = (struct ifconf*)ptr;
    struct ifreq* ifr;
    int n = ifc->ifc_len / sizeof(struct ifreq);
    int i = 0;

#ifdef CFG_NET_ETHERNET
    if (i < n)
    {
        ITPEthernetInfo info;
        struct sockaddr_in* addr;

        info.index = i;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ifr = &ifc->ifc_req[i];
        strncpy(ifr->ifr_name, info.name, IFNAMSIZ);
        addr = (struct sockaddr_in*)&ifr->ifr_addr;
        addr->sin_len = sizeof(struct sockaddr_in);
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = info.address;
        i++;
    }
#endif // CFG_NET_ETHERNET

#ifdef CFG_NET_WIFI
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 1)
    {
        if (i < n)
        {
            ITPWifiInfo info;
            struct sockaddr_in* addr;

            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
            ifr = &ifc->ifc_req[i];
            strncpy(ifr->ifr_name, info.name, IFNAMSIZ);
            addr = (struct sockaddr_in*)&ifr->ifr_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            i++;
        }
    }
#endif // CFG_NET_WIFI

#ifdef CFG_NET_WIFI_HOSTAPD
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 2)
    {
        if (i < n)
        {
            ITPWifiInfo info;
            struct sockaddr_in* addr;

            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_GET_INFO, &info);
            ifr = &ifc->ifc_req[i];
            strncpy(ifr->ifr_name, info.name, IFNAMSIZ);
            addr = (struct sockaddr_in*)&ifr->ifr_addr;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.address;
            i++;
        }
    }
#endif // CFG_NET_WIFI_HOSTAPD


    ifc->ifc_len = i * sizeof(struct ifreq);

    return 0;
}

static int SocketGetInterfaceNetmask(void* ptr)
{
    struct ifreq* ifr = (struct ifreq*)ptr;

#ifdef CFG_NET_ETHERNET
    {
        ITPEthernetInfo info;
        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_netmask;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.netmask;
            return 0;
        }
    }
#endif // CFG_NET_ETHERNET

#ifdef CFG_NET_WIFI
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 1)
    {
        ITPWifiInfo info;
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_netmask;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.netmask;
            return 0;
        }
    }
#endif // CFG_NET_WIFI

#ifdef CFG_NET_WIFI_HOSTAPD
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 2)
    {
        ITPWifiInfo info;
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            struct sockaddr_in* addr = (struct sockaddr_in*) &ifr->ifr_ifru.ifru_netmask;
            addr->sin_len = sizeof(struct sockaddr_in);
            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = info.netmask;
            return 0;
        }
    }
#endif // CFG_NET_WIFI_HOSTAPD

    return -1;
}

static int SocketGetInterfaceHardwareAddress(void* ptr)
{
    struct ifreq* ifr = (struct ifreq*)ptr;

#ifdef CFG_NET_ETHERNET
    {
        ITPEthernetInfo info;

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            memcpy(ifr->ifr_ifru.ifru_hwaddr.sa_data, info.hardwareAddress, 6);
            return 0;
        }
    }
#endif // CFG_NET_ETHERNET

#ifdef CFG_NET_WIFI
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 1)
    {
        ITPWifiInfo info;

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            memcpy(ifr->ifr_ifru.ifru_hwaddr.sa_data, info.hardwareAddress, 6);
            return 0;
        }
    }
#endif // CFG_NET_WIFI

#ifdef CFG_NET_WIFI_HOSTAPD
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_MODE, NULL) == 2)
    {
        ITPWifiInfo info;

        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_GET_INFO, &info);
        if (strncmp(ifr->ifr_name, info.name, IFNAMSIZ) == 0)
        {
            memcpy(ifr->ifr_ifru.ifru_hwaddr.sa_data, info.hardwareAddress, 6);
            return 0;
        }
    }
#endif // CFG_NET_WIFI_HOSTAPD

    return -1;
}

static void LwipInit(void* arg)
{
    sys_sem_t *init_sem = (sys_sem_t*)arg;

#if defined(CFG_NET_ETHERNET) || defined(CFG_USB_ECM) || defined(CFG_USBD_NCM)
    {
        printf("itpEthernetLwipInit()-----------------------------------------------\n");
        itpEthernetLwipInit();
    }
#endif

#if defined(CFG_NET_WIFI) && (!defined(CFG_NET_WIFI_SDIO_NGPL) && !defined (CFG_NET_WIFI_SDIO_SSV))
    {
        printf("itpWifiLwipInit()---------------------------------------------------\n");
        itpWifiLwipInit();
    }
#endif

    /*
    #if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI) && !defined(CFG_NET_ETHERNET_WIFI)
        if ( iteEthGetLink2() )
        {
            printf("-----------------------------------------------itpEthernetLwipInit()\n");
            itpEthernetLwipInit();
        }
        else
        {
            printf("-----------------------------------------------itpWifiLwipInit()\n");
            itpWifiLwipInit();
        }
    #elif defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI) && defined(CFG_NET_ETHERNET_WIFI)
        {
            printf("itpEthernetLwipInit()-----------------------------------------------\n");
            itpEthernetLwipInit();
        }
        {
            printf("itpWifiLwipInit()---------------------------------------------------\n");
            itpWifiLwipInit();
        }
    #else
    #ifdef CFG_NET_ETHERNET
        itpEthernetLwipInit();
    #endif
    #ifdef CFG_NET_WIFI
        itpWifiLwipInit();
    #endif
    #endif
    */
    sys_sem_signal(init_sem);
}

void SocketInit(void)
{
    sys_sem_t init_sem;
    err_t err = sys_sem_new(&init_sem, 0);

    // initialize lwIP stack, network interfaces and applications
#if !defined(CFG_NET_WIFI_SDIO_SSV)
    tcpip_init(LwipInit, &init_sem);
#endif

    // we have to wait for initialization to finish before calling update_adapter()!
    sys_sem_wait(&init_sem);
    sys_sem_free(&init_sem);
}

static int SocketIoctl(int file, unsigned long request, void* ptr, void* info)
{
#ifdef CFG_NET_WIFI

    if ((SIOCIWFIRST <= request) && (request <= SIOCIWFIRSTPRIV))
    {
        int ret = 0;
        //printf("ioctl\n");
        ret = ioctl(ITP_DEVICE_WIFI, request, ptr);
        return ret;
    }

    if ((request == (SIOCIWFIRSTPRIV + 30)) || (request == (SIOCIWFIRSTPRIV + 28)))
    {
        int ret = 0;
        //printf("SocketIoctl request = 0x%X\n",request);
        ret = ioctl(ITP_DEVICE_WIFI, request, ptr);
        return ret;
    }
#endif

    //printf("other ioctl\n");

    switch (request)
    {
    case SIOCGIFFLAGS:
        return SocketGetInterfaceFlags(ptr);

    case SIOCGIFADDR:
        return SocketGetInterfaceAddress(ptr);

    case SIOCGIFCONF:
        return SocketGetInterfaceConfig(ptr);

    case SIOCGIFNETMASK:
        return SocketGetInterfaceNetmask(ptr);

    case SIOCGIFHWADDR:
        return SocketGetInterfaceHardwareAddress(ptr);

    case ITP_IOCTL_INIT:
        SocketInit();
        break;

    case SIOCGIFINDEX:
    {
        struct ifreq* ifr = (struct ifreq*)ptr;
        ifr->ifr_ifru.ifru_index = 0;
    }
    return 0;

    default:
#ifdef CFG_IOT_ENABLE
        return ioctlsocket(file, request, ptr);
#else
		return lwip_ioctl(file, request, ptr);
#endif
    }
    return 0;
}

const ITPDevice itpDeviceSocket =
{
    ":socket",
    itpOpenDefault,
    SocketClose,
    SocketRead,
    SocketWrite,
    itpLseekDefault,
    SocketIoctl,
    NULL
};

int socketpair(int __domain, int __type, int __protocol, int *__socket_vec)
{
    int i;
    if (!__socket_vec)
    {
        errno = EINVAL;
        return -1;
    }

#define PAIRMAXTRYIES 800
    for (i = 0; i < PAIRMAXTRYIES; i++)
    {
        struct sockaddr_in listen_addr;
        int listen_s;
        static const int c_addinlen = sizeof(struct sockaddr_in); /* help compiler to optimize */
        int addr_len = c_addinlen;
        int opt = 1;

        listen_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (-1 == listen_s)
            break; /* can't create even single socket */

        listen_addr.sin_family = AF_INET;
        listen_addr.sin_port = htons(0);
        listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (0 == bind(listen_s, (struct sockaddr*) &listen_addr, c_addinlen)
            && 0 == listen(listen_s, 1)
            && 0 == getsockname(listen_s, (struct sockaddr*) &listen_addr,
                &addr_len))
        {
            int client_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (-1 != client_s)
            {
                if (0 == ioctlsocket(client_s, FIONBIO, (u_long*)&opt))
                {
                    struct sockaddr_in accepted_from_addr;
                    int server_s;
                    connect(client_s, (struct sockaddr*) &listen_addr, c_addinlen);
                    addr_len = c_addinlen;
                    server_s = accept(listen_s,
                        (struct sockaddr*) &accepted_from_addr, &addr_len);
                    if (-1 != server_s)
                    {
                        struct sockaddr_in client_addr;
                        addr_len = c_addinlen;
                        opt = 0;
                        if (0 == getsockname(client_s, (struct sockaddr*) &client_addr, &addr_len)
                            && accepted_from_addr.sin_family == client_addr.sin_family
                            && accepted_from_addr.sin_port == client_addr.sin_port
                            && accepted_from_addr.sin_addr.s_addr == client_addr.sin_addr.s_addr
                            && 0 == ioctlsocket(client_s, FIONBIO, (u_long*)&opt)
                            && 0 == ioctlsocket(server_s, FIONBIO, (u_long*)&opt))
                        {
                            closesocket(listen_s);
                            __socket_vec[0] = client_s;
                            __socket_vec[1] = server_s;
                            return 0;
                        }
                        closesocket(server_s);
                    }
                }
                closesocket(client_s);
            }
        }
        closesocket(listen_s);
    }

    __socket_vec[0] = -1;
    __socket_vec[1] = -1;
    return -1;
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
{
    ssize_t ret;
    size_t tot = 0;
    int i;
    char *buf, *p;
    struct iovec *iov = msg->msg_iov;

    for (i = 0; i < msg->msg_iovlen; ++i)
        tot += iov[i].iov_len;

    buf = malloc(tot);
    if (tot != 0 && buf == NULL)
    {
        errno = ENOMEM;
        return -1;
    }
    p = buf;
    for (i = 0; i < msg->msg_iovlen; ++i)
    {
        memcpy(p, iov[i].iov_base, iov[i].iov_len);
        p += iov[i].iov_len;
    }
    ret = sendto(s, buf, tot, flags, msg->msg_name, msg->msg_namelen);
    free(buf);
    return ret;
}

ssize_t recvmsg(int s, struct msghdr *msg, int flags)
{
    ssize_t ret, nb;
    size_t tot = 0;
    int i;
    char *buf, *p;
    struct iovec *iov = msg->msg_iov;

    for (i = 0; i < msg->msg_iovlen; ++i)
        tot += iov[i].iov_len;

    buf = malloc(tot);
    if (tot != 0 && buf == NULL)
    {
        errno = ENOMEM;
        return -1;
    }
    nb = ret = recvfrom(s, buf, tot, flags, msg->msg_name, &msg->msg_namelen);
    p = buf;
    while (nb > 0)
    {
        ssize_t cnt = ITH_MIN(nb, (ssize_t)iov->iov_len);
        memcpy(iov->iov_base, p, cnt);
        p += cnt;
        nb -= cnt;
        ++iov;
    }
    free(buf);
    return ret;
}

const char	*inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (af == AF_INET)
    {
        static const char fmt[] = "%u.%u.%u.%u";
        char tmp[sizeof("255.255.255.255")];
        const u_char *s = (const u_char *)src;

        if (sprintf(tmp, fmt, s[0], s[1], s[2], s[3]) > (int)size)
        {
            errno = ENOSPC;
            return NULL;
        }
        strcpy(dst, tmp);
        return dst;
    }
    else
    {
        errno = EAFNOSUPPORT;
        return NULL;
    }
}

int
inet_pton(int af, const char *src, void *dst)
{
    if (af == AF_INET)
    {
        static const char digits[] = "0123456789";
        int saw_digit, octets, ch;
        u_char tmp[4], *tp;

        saw_digit = 0;
        octets = 0;
        *(tp = tmp) = 0;
        while ((ch = *src++) != '\0') {
            const char *pch;

            if ((pch = strchr(digits, ch)) != NULL) {
                u_int new = *tp * 10 + (pch - digits);

                if (new > 255)
                    return (0);
                *tp = new;
                if (!saw_digit) {
                    if (++octets > 4)
                        return (0);
                    saw_digit = 1;
                }
            } else if (ch == '.' && saw_digit) {
                if (octets == 4)
                    return (0);
                *++tp = 0;
                saw_digit = 0;
            } else
                return (0);
        }
        if (octets < 4)
            return (0);

        memcpy(dst, tmp, 4);
        return (1);
    }
    else
    {
        errno = EAFNOSUPPORT;
        return -1;
    }
}

unsigned if_nametoindex(const char *ifname)
{
    return 0;
}

char *if_indextoname(unsigned ifindex, char *ifname)
{
    return NULL;
}

int getifaddrs(struct ifaddrs **ifap)
{
    return -1;
}

void freeifaddrs(struct ifaddrs *ifa)
{
}
