/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL WiFi functions.
 *
 * @author Jim Tan
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "itp_cfg.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/tcpip.h"
#include "ite/ite_wifi.h"
#include "ite/ite_usbex.h"

err_t wifiif_init(struct netif *netif);
void  wifiif_shutdown(struct netif *netif);
void  wifiif_poll(struct netif *netif);
void  wifiif_ctrl(struct netif *netif, struct net_device_config* cfg);
void  wifiif_info(struct netif *netif, struct net_device_info* info);

static struct netif             wifiNetif;
static struct dhcp              wifiNetifDhcp;
static struct timeval           wifiLastTime;
static struct net_device_config currentDriverConfig;

static int                      WifiCurrentState = 0, WifiAddNetIf = 0, wifiMscnt, sleep_notify = 2;
static bool                     wifiInited, wifiDhcp, linkUp, wifiConnected = false;
static pthread_t                wifi_init_task;
static ip_addr_t                wifiIpaddr, wifiNetmask, wifiGw;

#define WIFI_TEST_INPUT_DONGLE  15
#define STASTATE                1
#define APSTATE                 2

#if defined (CFG_NET_WIFI_SDIO_NGPL)
/* ==================== RTK 8189FTV NETIF INIT ==================== */
// keep add netif
static struct netif *keepNetif;

// test example
#define CFG_NET_WIFI_IPADDR     "192.168.1.1"
#define CFG_NET_WIFI_NETMASK    "255.255.255.0"
#define CFG_NET_WIFI_GATEWAY    "192.168.1.1"

// This function initializes all network interfaces
static void itpWifiNgplLwipInit(struct netif *netif)
{
    ip_addr_t ipaddr, netmask, gw;

    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);
    printf("[Itp Wifi] itpWifiNgplLwipInit netif:0x%X\n", netif);

    netif_add(netif, &ipaddr, &netmask, &gw, NULL, wifiif_init, tcpip_input);
    netif_set_default(netif);
#if LWIP_NETIF_REMOVE_CALLBACK
    netif_set_remove_callback(netif, wifiif_shutdown);
#endif
    dhcp_set_struct(netif, &wifiNetifDhcp);
}

static int WifiRead(int file, char *ptr, int len, void* info)
{
    struct net_device_info* netinfo = (struct net_device_info*)ptr;
    netinfo->infoType = WLAN_INFO_SCAN_GET;
    wifiif_info(&wifiNetif, netinfo);
    return 0;
}

static void WifiSleepNotify(int idx)
{
    sleep_notify = idx; // [initial:2], [sleep -> wakeup:1] , [wakeup -> sleep:0]
    if (sleep_notify == sleep_to_wakeup)
        wifiInited = true;
    printf("###sleep notify: %d\n", sleep_notify);
}

static void WifiStartDHCP(void* setting)
{
    ip_addr_t ipaddr, netmask, gw;

    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);

    printf("dhcp_start\n");

    netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);

    dhcp_start(&wifiNetif);

    wifiDhcp = true;
    gettimeofday(&wifiLastTime, NULL);
    wifiMscnt = 0;
}

static void WifiReset(ITPEthernetSetting* setting)
{

        netif_set_down(&wifiNetif);

        IP4_ADDR(&wifiIpaddr, setting->ipaddr[0], setting->ipaddr[1], setting->ipaddr[2], setting->ipaddr[3]);
        IP4_ADDR(&wifiNetmask, setting->netmask[0], setting->netmask[1], setting->netmask[2], setting->netmask[3]);
        IP4_ADDR(&wifiGw, setting->gw[0], setting->gw[1], setting->gw[2], setting->gw[3]);

        netif_set_addr(keepNetif, &wifiIpaddr, &wifiNetmask, &wifiGw);
        netif_set_up(keepNetif);
        
}

/* No Use for NGPL SDIO WIFI */
void WifiConnect(int type)
{
    //no use, for compiler
}

void WifiDisconnect(int type)
{
    //no use, for compiler
}

/*************************/
extern int wifi_off(void);
static int WifiIoctl(int file, unsigned long request, void* ptr, void* info)
{
    struct netif *xnetif;

    switch (request)
    {
        case ITP_IOCTL_WIFI_START_DHCP:
        	if(WifiCurrentState == STASTATE)
                WifiStartDHCP((void*)ptr);
            break;

        case ITP_IOCTL_WIFI_STOP_DHCP:
            if(WifiCurrentState == STASTATE)
                dhcp_stop(&wifiNetif);
            break;

        case ITP_IOCTL_WIFI_ADD_NETIF:
            keepNetif = ptr;
            itpWifiNgplLwipInit(ptr);
            break;

        case ITP_IOCTL_INIT:
            printf("WifiIoctl ITP_IOCTL_INIT \n");
            mmpRtlWifiDriverCmdTest();
            break;

        case ITP_IOCTL_EXIT:
            wifi_off();
            break;

        case ITP_IOCTL_RESET:
        WifiReset((void*)ptr);
        break;


        case ITP_IOCTL_SLEEP:
            WifiSleepNotify((int)ptr);
            break;

        case ITP_IOCTL_WIFI_SLEEP_STATUS:
        	return sleep_notify;

        case ITP_IOCTL_WIFIAP_ENABLE:
            {
                ip_addr_t ipaddr, netmask, gw;
                printf("%s ITP_IOCTL_WIFIAP_ENABLE 0x%x \n",__FUNCTION__,ptr);
#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
                //itpWifiLwipInitNetif();
#endif
                ipaddr_aton(CFG_NET_WIFI_IPADDR, &ipaddr);
                ipaddr_aton(CFG_NET_WIFI_NETMASK, &netmask);
                ipaddr_aton(CFG_NET_WIFI_GATEWAY, &gw);

            if (keepNetif){
                netif_set_addr(keepNetif, &ipaddr, &netmask, &gw);
                netif_set_up(keepNetif);
                dhcp_start(keepNetif);
            } else {
                printf("Error netif NULL \n");
            }


                WifiCurrentState = APSTATE;
                printf(" - %s  ITP_IOCTL_WIFIAP_ENABLE \n",__FUNCTION__);
            }
            break;

        case ITP_IOCTL_DISABLE:
            {
                ip_addr_t ipaddr, netmask, gw;

                ip_addr_set_zero(&gw);
                ip_addr_set_zero(&ipaddr);
                ip_addr_set_zero(&netmask);

                netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);
                netif_set_down(&wifiNetif);
                printf("[itp wifi] ITP_IOCTL_DISABLE \n");
                wifiInited = false;
                linkUp = false;
                WifiCurrentState = 0;
            }
            break;

        case ITP_IOCTL_WIFI_NETIF_SHUTDOWN:
            netif_set_link_down(&wifiNetif); //close MAC & DMA
#if defined(CFG_NET_WIFI) && defined(CFG_NET_ETHERNET)
            netif_remove(&wifiNetif);
#else
            netif_remove_all(); //1. call netif_set_down   2.wifiif_shutdown by callback
#endif
            break;

        default:
            errno = (ITP_DEVICE_WIFI << ITP_DEVICE_ERRNO_BIT) | 1;
            return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWifi =
{
    ":wifi",
    itpOpenDefault,
    itpCloseDefault,
    WifiRead,
    itpWriteDefault,
    itpLseekDefault,
    WifiIoctl,
    NULL
};
/* =================================================== */


#elif defined (CFG_NET_WIFI_SDIO_SSV)
/* ==================== SSV NETIF INIT ==================== */
struct netif *Netif_SSV;
struct netdev_ssv
{
    char name[26];
    u16_t mtu;
    u32_t ipaddr;
    u32_t netmask;
    u32_t gw;
    u32_t flags;
    u8_t  hwmac[6];
    u8_t  add_state;
};

static int itpWifiSsvLwipInit(struct netif *pwlan, struct netdev_ssv *pdev)
{
    ip_addr_t ipaddr, netmask, gw;

    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);
    printf("[Itp Wifi] itpWifiSsvLwipInit: netif(0x%X)\n", pwlan);

    /* net if init */
    pwlan = netif_find(pdev->name);
    if (pwlan != NULL)
        netif_remove(pwlan);

    memcpy((void *)(pwlan->hwaddr), pdev->hwmac, 6);
    memcpy((void *)(pwlan->name),   pdev->name,  6);

    ipaddr.addr  = pdev->ipaddr;
    netmask.addr = pdev->netmask;
    gw.addr      = pdev->gw;

    netif_add(pwlan, &ipaddr, &netmask, &gw, NULL, wifiif_init, tcpip_input);
    netif_set_default(pwlan);

#if LWIP_NETIF_REMOVE_CALLBACK
    netif_set_remove_callback(pwlan, wifiif_shutdown);
#endif
    dhcp_set_struct(pwlan, &wifiNetifDhcp);

    printf("Add netif MAC[%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
        pwlan->hwaddr[0], pwlan->hwaddr[1], pwlan->hwaddr[2],
        pwlan->hwaddr[3], pwlan->hwaddr[4], pwlan->hwaddr[5]);

    return 0;
}


static int WifiIoctl(int file, unsigned long request, void* ptr, void* info)
{
    struct netif *xnetif;
    int ssv_init;

    switch (request)
    {
        case ITP_IOCTL_WIFI_ADD_NETIF:
            ssv_init = itpWifiSsvLwipInit(Netif_SSV, (struct netdev_ssv*)ptr);
            return ssv_init;

        case ITP_IOCTL_INIT:
            mmpSsvWifiDriverCmdTest();
            break;

        default:
            errno = (ITP_DEVICE_WIFI << ITP_DEVICE_ERRNO_BIT) | 1;
            return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWifi =
{
    ":wifi icomm",
    itpOpenDefault,
    itpCloseDefault,
    NULL,
    itpWriteDefault,
    itpLseekDefault,
    WifiIoctl,
    NULL
};
/* =================================================== */


#else
static int WifiRead(int file, char *ptr, int len, void* info)
{
    struct net_device_info* netinfo = (struct net_device_info*)ptr;
    netinfo->infoType = WLAN_INFO_SCAN_GET;
    wifiif_info(&wifiNetif, netinfo);
    return 0;
}

static void WifiSleepNotify(int idx)
{
    sleep_notify = idx; // [initial:2], [sleep -> wakeup:1] , [wakeup -> sleep:0]
    if (sleep_notify == sleep_to_wakeup)
        wifiInited = true;
    printf("###sleep notify: %d\n", sleep_notify);
}

// This function initializes all network interfaces
void itpWifiLwipInit(void)
{
    ip_addr_t ipaddr, netmask, gw;

    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);

#if defined(CFG_NET_WIFI_MAY_NOT_EXIST)
    // check wifi dongle status
    if (!wifiConnected){
        printf("[Itp Wifi] itpWifiLwipInit no wifi dongle \n");
        return;
    }
#endif

    netif_add(&wifiNetif, &ipaddr, &netmask, &gw, NULL, wifiif_init, tcpip_input);
    //printf("itpWifiLwipInit don't netif_set_default  \n\n");
#if !defined(CFG_NET_ETHERNET)
    netif_set_default(&wifiNetif);
#endif
#if LWIP_NETIF_REMOVE_CALLBACK
    netif_set_remove_callback(&wifiNetif, wifiif_shutdown);
#endif
    WifiAddNetIf = 1;
    dhcp_set_struct(&wifiNetif, &wifiNetifDhcp);
}


// This function initializes all network interfaces
void itpWifiLwipInitNetif(void)
{
    ip_addr_t ipaddr, netmask, gw;
    int timeout = 0;
    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);

    // avoid wifi dirver not initializes
    do {
        timeout++;
        if (timeout>10)
            break;
        usleep(100*1000);
        printf("Net device not ready , wait %d \n",timeout);
    } while (!smNetGetDevice());

    netif_add(&wifiNetif, &ipaddr, &netmask, &gw, NULL, wifiif_init, tcpip_input);
    printf("itpWifiLwipInitNetif %d \n\n",WifiAddNetIf);
    WifiAddNetIf = 1;
    netif_set_default(&wifiNetif);
    dhcp_set_struct(&wifiNetif, &wifiNetifDhcp);
}

static void WifiScan(void)
{
    static struct net_device_info info = {0};
    info.infoType = WLAN_INFO_SCAN;
    wifiif_info(&wifiNetif, &info);
}

static int WifiState(void)
{
    static struct net_device_info info = {0};
    info.infoType = WLAN_INFO_WIFI_STATE;
    wifiif_info(&wifiNetif, &info);
    return info.driverState;
}

static int WifiStaNum(void)
{
    static struct net_device_info info = {0};
    info.infoType = WLAN_INFO_WIFI_STATE;
    wifiif_info(&wifiNetif, &info);
    return info.staCount;
}

static int WifiBestChannel(void)
{
    static struct net_device_info info = {0};
    info.infoType = WLAN_INFO_WIFI_STATE;
    wifiif_info(&wifiNetif, &info);
    return info.channelId;
}


static void WifiapGetInfo(ITPWifiInfo* info)
{
    static struct net_device_info netinfo = {0};
    memset(info, 0, sizeof (ITPWifiInfo));

    netinfo.infoType = WLAN_INFO_AP;
    wifiif_info(&wifiNetif, &netinfo);

    info->active = 1;//wifiapNetif.ip_addr.addr ? 1 : 0;
    info->address = wifiNetif.ip_addr.addr;
    info->netmask = wifiNetif.netmask.addr;
    strcpy(info->displayName, "wlan0");

    info->hardwareAddress[0] = netinfo.staMacAddr[0];
    info->hardwareAddress[1] = netinfo.staMacAddr[1];
    info->hardwareAddress[2] = netinfo.staMacAddr[2];
    info->hardwareAddress[3] = netinfo.staMacAddr[3];
    info->hardwareAddress[4] = netinfo.staMacAddr[4];
    info->hardwareAddress[5] = netinfo.staMacAddr[5];

    strcpy(info->name, "wlan0");
}

static void WifiGetInfo(ITPWifiInfo* info)
{
    static struct net_device_info netinfo = {0};
    memset(info, 0, sizeof (ITPWifiInfo));

#ifndef CFG_NET_WIFI_WPA
    if (!(netif_is_link_up(&wifiNetif)))
    {
        LOG_ERR "wifi is not init yet\n" LOG_END
        return;
    }
#endif

    netinfo.infoType = WLAN_INFO_AP;
    wifiif_info(&wifiNetif, &netinfo);

    info->active = wifiNetif.ip_addr.addr ? 1 : 0;
    info->address = wifiNetif.ip_addr.addr;
    info->netmask = wifiNetif.netmask.addr;
    strcpy(info->displayName, "wlan0");

    info->hardwareAddress[0] = netinfo.staMacAddr[0];
    info->hardwareAddress[1] = netinfo.staMacAddr[1];
    info->hardwareAddress[2] = netinfo.staMacAddr[2];
    info->hardwareAddress[3] = netinfo.staMacAddr[3];
    info->hardwareAddress[4] = netinfo.staMacAddr[4];
    info->hardwareAddress[5] = netinfo.staMacAddr[5];
    info->rfQualityQuant = netinfo.avgQuant;
    info->rfQualityRSSI = netinfo.avgRSSI;
    strcpy(info->name, "wlan0");
}

static void WifiLinkHidden(struct net_device_info* setting)
{
    if (!(wifiInited))
    {
        LOG_ERR "wifi is not init yet\n" LOG_END
        return;
    }

    setting->infoType = WLAN_INFO_LINK_HIDDEN;
    wifiif_info(&wifiNetif, setting);
}

static void WifiLinkAP(void* setting)
{
    struct net_device_config *driverConfig = (struct net_device_config*)setting;
    if (!(wifiInited))
    {
        LOG_ERR "wifi is not init yet\n" LOG_END
        return;
    }

#ifndef CFG_NET_WIFI_WPA
    wifiif_ctrl(&wifiNetif, driverConfig);
    usleep(1000*1000*10);
#endif
}

static void WifiStartDHCP(void* setting)
{
    ip_addr_t ipaddr, netmask, gw;

    ip_addr_set_zero(&gw);
    ip_addr_set_zero(&ipaddr);
    ip_addr_set_zero(&netmask);

    printf("dhcp_start\n");

    netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);

    dhcp_start(&wifiNetif);

    wifiDhcp = true;
    gettimeofday(&wifiLastTime, NULL);
    wifiMscnt = 0;
}

static void WifiReset(ITPEthernetSetting* setting)
{
    if (setting->dhcp || setting->autoip)
    {
        if (!setting->dhcp && wifiDhcp)
        {
            dhcp_stop(&wifiNetif);
            wifiDhcp = false;
        }

        if (setting->dhcp && !wifiDhcp)
        {
            dhcp_set_struct(&wifiNetif, &wifiNetifDhcp);
            dhcp_start(&wifiNetif);
            wifiDhcp = true;
        }

        gettimeofday(&wifiLastTime, NULL);
        wifiMscnt = 0;
    }
    else
    {
        if (wifiDhcp)
            dhcp_stop(&wifiNetif);

        netif_set_down(&wifiNetif);

        IP4_ADDR(&wifiIpaddr, setting->ipaddr[0], setting->ipaddr[1], setting->ipaddr[2], setting->ipaddr[3]);
        IP4_ADDR(&wifiNetmask, setting->netmask[0], setting->netmask[1], setting->netmask[2], setting->netmask[3]);
        IP4_ADDR(&wifiGw, setting->gw[0], setting->gw[1], setting->gw[2], setting->gw[3]);

        netif_set_addr(&wifiNetif, &wifiIpaddr, &wifiNetmask, &wifiGw);
        netif_set_up(&wifiNetif);
        wifiDhcp     = false;
    }
}

static void WifiPoll(void)
{
    if ((wifiNetif.ip_addr.addr == 0) && (wifiDhcp))
    {
        struct timeval currTime;

        gettimeofday(&currTime, NULL);
        if (itpTimevalDiff(&wifiLastTime, &currTime) >= DHCP_FINE_TIMER_MSECS)
        {
            if (wifiDhcp)
                dhcp_fine_tmr();

            wifiLastTime = currTime;
            wifiMscnt += DHCP_FINE_TIMER_MSECS;
        }
        else if (wifiMscnt >= DHCP_COARSE_TIMER_MSECS)
        {
            if (wifiDhcp)
                dhcp_coarse_tmr();

            wifiMscnt = 0;
        }
    }

    if(linkUp == true)
    {
        if (!(wifiNetif.flags & NETIF_FLAG_LINK_UP))
        {
    	    linkUp = false;
        }
    }
}

#if CFG_NET_WIFI_POLL_INTERVAL > 0

static void WifiPollHandler(timer_t timerid, int arg)
{
    WifiPoll();
}
#endif // CFG_NET_WIFI_POLL_INTERVAL > 0

#ifdef CFG_NET_WIFI_REDEFINE
static void* WifiInitTask(void* arg)
{
    int result = 0;

    wifiInited = false;
    linkUp = false;

#if CFG_NET_WIFI_POLL_INTERVAL > 0
    {
        timer_t timer;
        struct itimerspec value;
        timer_create(CLOCK_REALTIME, NULL, &timer);
        timer_connect(timer, (VOIDFUNCPTR)WifiPollHandler, 0);
        value.it_value.tv_sec = value.it_interval.tv_sec = 0;
        value.it_value.tv_nsec = value.it_interval.tv_nsec = CFG_NET_WIFI_POLL_INTERVAL * 1000000;
        timer_settime(timer, 0, &value, NULL);
    }
#endif // CFG_NET_WIFI_POLL_INTERVAL > 0

    //wait wifi connected
#if !defined(CFG_NET_WIFI_SDIO_NGPL)
    while(!wifiConnected)
    {
        printf("[%s] Wait wifi initialize\n", __FUNCTION__);
        usleep(1000*1000);
    }
#endif

    // init socket device
    itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
    ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
}
#endif

static void WifiInit(void)
{
#if defined(CFG_NET_WIFI_MAY_NOT_EXIST)
    int nTemp = 0;
#endif
#ifdef CFG_NET_WIFI_REDEFINE
    if (!wifi_init_task)
    {
        pthread_attr_t attr;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&wifi_init_task, &attr, WifiInitTask, NULL);
    }
#else
    int result = 0;

    wifiInited = false;
    linkUp = false;

    //wait wifi connected
	while(!wifiConnected)
    {
        printf("wait wifi connected\n");
        usleep(1000*100);
#if defined(CFG_NET_WIFI_MAY_NOT_EXIST)
        // if there is no wifi dongle , return
        nTemp++;
        if (nTemp>WIFI_TEST_INPUT_DONGLE){
            printf("[Itp Wifi] WifiInit no wifi dongle \n");
            return;
        }
#endif
    }

#if CFG_NET_WIFI_POLL_INTERVAL > 0
    {
        timer_t timer;
        struct itimerspec value;
        timer_create(CLOCK_REALTIME, NULL, &timer);
        timer_connect(timer, (VOIDFUNCPTR)WifiPollHandler, 0);
        value.it_value.tv_sec = value.it_interval.tv_sec = 0;
        value.it_value.tv_nsec = value.it_interval.tv_nsec = CFG_NET_WIFI_POLL_INTERVAL * 1000000;
        timer_settime(timer, 0, &value, NULL);
    }
#endif // CFG_NET_WIFI_POLL_INTERVAL > 0
#endif
}

extern int wifiif_ioctrl(struct netif *netif, void* ptr,int cmd);
static int WifiIoctl(int file, unsigned long request, void* ptr, void* info)
{
    if ((SIOCIWFIRST <= request) && (request <= SIOCIWLAST))
    {
        int ret = 0;
        ret = wifiif_ioctrl(&wifiNetif, (void*)ptr,request);
        return ret;
    }

    if(request == (SIOCIWFIRSTPRIV+30) || request == (SIOCIWFIRSTPRIV+28))
    {
        int ret = 0;
        ret = wifiif_ioctrl(&wifiNetif, (void*)ptr,request);
        return ret;
    }



    switch (request)
    {
        case ITP_IOCTL_POLL:
        WifiPoll();
        break;

        case ITP_IOCTL_IS_AVAIL:
        if(WifiCurrentState == STASTATE)
        {
            return wifiNetif.ip_addr.addr? 1 : 0;
        }
        else if(WifiCurrentState == APSTATE)
        {
            return 1;
        }
        return 0;

        case ITP_IOCTL_IS_CONNECTED:
        if(WifiCurrentState == STASTATE)
        {
            static struct net_device_info netinfo = {0};

            if (!(wifiInited))
            {
                LOG_ERR "wifi is not init yet\n" LOG_END
                return 0;
            }

            netinfo.infoType = WLAN_INFO_LINK;
            wifiif_info(&wifiNetif, &netinfo);
            if(netinfo.linkInfo == WLAN_LINK_ON)
            {
                linkUp = true;
            }

            return linkUp;
        }
        else if(WifiCurrentState == APSTATE)
        {
            return 1;
        }
        return 0;

        case ITP_IOCTL_IS_DEVICE_READY:
        return wifiConnected;

        case ITP_IOCTL_SLEEP:
        WifiSleepNotify((int)ptr);
        break;

        case ITP_IOCTL_WIFI_SLEEP_STATUS:
        return sleep_notify;

        case ITP_IOCTL_GET_INFO:
        if(WifiCurrentState == STASTATE)
        {
            WifiGetInfo((ITPWifiInfo*)ptr);
        }
        else if(WifiCurrentState == APSTATE)
        {
            WifiapGetInfo((ITPWifiInfo*)ptr);
        }
        else
        {
            WifiGetInfo((ITPWifiInfo*)ptr);
        }
        break;

        case ITP_IOCTL_WIFIAP_GET_INFO:
        WifiapGetInfo((ITPWifiInfo*)ptr);
        break;

        case ITP_IOCTL_INIT:
        WifiInit();
        break;

#if defined(CFG_NET_WIFI_8188CUS) || defined(CFG_NET_WIFI_8188EUS) || defined(CFG_NET_WIFI_8188FTV) || defined(CFG_NET_WIFI_5G)
        case ITP_IOCTL_WIFI_DRV_REINIT:
        mmpRtlWifiDriverRegister();
        wifiInited = true;
        break;
#endif

        case ITP_IOCTL_WIFI_HIDDEN_SSID:
            WifiLinkHidden((struct net_device_info*)ptr);
        break;

        case ITP_IOCTL_SCAN:
        WifiScan();
        break;

        case ITP_IOCTL_RESET:
        WifiReset((void*)ptr);
        break;

        case ITP_IOCTL_ENABLE:
        printf("[itp wifi] ITP_IOCTL_ENABLE \n");
        netif_set_up(&wifiNetif);
        wifiInited = true;
        WifiCurrentState = STASTATE;
        break;

        case ITP_IOCTL_DISABLE:
        {
            ip_addr_t ipaddr, netmask, gw;

            ip_addr_set_zero(&gw);
            ip_addr_set_zero(&ipaddr);
            ip_addr_set_zero(&netmask);

            netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);
            netif_set_down(&wifiNetif);
            printf("[itp wifi] ITP_IOCTL_DISABLE \n");
            wifiInited = false;
            linkUp = false;
            WifiCurrentState = 0;
        }
        break;

        case ITP_IOCTL_WIFI_NETIF_SHUTDOWN:
        netif_set_link_down(&wifiNetif); //close MAC & DMA
#if defined(CFG_NET_WIFI) && defined(CFG_NET_ETHERNET)
        netif_remove(&wifiNetif);
#else
        netif_remove_all(); //1. call netif_set_down   2.wifiif_shutdown by callback
#endif
        break;

        case ITP_IOCTL_RESET_DEFAULT:
        printf("itp wifi set netif  default \n");
        netif_set_default(&wifiNetif);
        break;


#ifdef CFG_NET_WIFI_HOSTAPD
        case ITP_IOCTL_WIFIAP_ENABLE:
        {
            ip_addr_t ipaddr, netmask, gw;
            printf("%s ITP_IOCTL_WIFIAP_ENABLE \n",__FUNCTION__);
#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
            itpWifiLwipInitNetif();
#endif
            ipaddr_aton(CFG_NET_WIFI_IPADDR, &ipaddr);
            ipaddr_aton(CFG_NET_WIFI_NETMASK, &netmask);
            ipaddr_aton(CFG_NET_WIFI_GATEWAY, &gw);
            //ip4_addr1(&ipaddr);

            netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);

            netif_set_up(&wifiNetif);
            WifiCurrentState = APSTATE;
            printf(" - %s  ITP_IOCTL_WIFIAP_ENABLE \n",__FUNCTION__);
        }
        break;

        case ITP_IOCTL_WIFIAP_DISABLE:
        {
            ip_addr_t ipaddr, netmask, gw;

            ip_addr_set_zero(&gw);
            ip_addr_set_zero(&ipaddr);
            ip_addr_set_zero(&netmask);

            netif_set_addr(&wifiNetif, &ipaddr, &netmask, &gw);
            netif_set_down(&wifiNetif);
            WifiCurrentState = 0;
        }
        break;
#endif
        case ITP_IOCTL_WIFI_LINK_AP:
        WifiLinkAP((void*)ptr);
        break;

        case ITP_IOCTL_WIFI_START_DHCP:
        if(WifiCurrentState == STASTATE)
            WifiStartDHCP((void*)ptr);
        break;

        case ITP_IOCTL_WIFI_STOP_DHCP:
        if(WifiCurrentState == STASTATE)
            dhcp_stop(&wifiNetif);
        break;

        case ITP_IOCTL_RENEW_DHCP:
        if(WifiCurrentState == STASTATE)
            dhcp_network_changed(&wifiNetif);
        break;

        case ITP_IOCTL_WIFI_STATE:
        return WifiState();

        case ITP_IOCTL_WIFI_STANUM:
        return WifiStaNum() - 2;

        case ITP_IOCTL_WIFI_BEST_CHANNEL:
        return WifiBestChannel();

        case ITP_IOCTL_WIFI_MODE:
        return WifiCurrentState;

        case ITP_IOCTL_WIFI_ADD_NETIF:
            itpWifiLwipInitNetif();
        break;

        case ITP_IOCTL_WIFI_NETIF_STATUS:
            return WifiAddNetIf;

        case ITP_IOCTL_WIFI_GET_NET_DEVICE:
        if (smNetGetDevice() != NULL)
            return 1;
        else
            return 0;

        default:
            errno = (ITP_DEVICE_WIFI << ITP_DEVICE_ERRNO_BIT) | 1;
            return -1;
    }
    return 0;
}

const ITPDevice itpDeviceWifi =
{
    ":wifi",
    itpOpenDefault,
    itpCloseDefault,
    WifiRead,
    itpWriteDefault,
    itpLseekDefault,
    WifiIoctl,
    NULL
};

void WifiConnect(int type)
{
    printf("WifiConnect\n");
    if(USB_DEVICE_WIFI(type))
    {
        printf(" USB : WIFI device is interted!! \n");
        wifiConnected = true;
    }
}

void WifiDisconnect(int type)
{
    if(USB_DEVICE_WIFI(type))
    {
        printf(" USB : WIFI device is disconnected!\n");
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_STOP_DHCP, NULL);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_DISABLE, NULL);
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_SHUTDOWN, NULL);
        tasklet_deinit();
        usleep(20000);
        iteTimerCtrlTerminate();
        usleep(20000);
        wifiConnected = false;
        wifiInited = false;
    }
}
#endif
