#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip/ip.h"
#include "ite/itp.h"
#include "scene.h"
#include "ctrlboard.h"

static ITUText* settingSysInfoIPAddrText;
static ITUText* settingSysInfoMacText;
static ITUText* settingSysInfoHWVersionText;
static ITUText* settingSysInfoSWVersionText;

#ifdef CFG_NET_WIFI_SDIO_NGPL
#include "wifi_conf.h"
extern struct netif xnetif[NET_IF_NUM];
extern uint8_t* LwIP_GetMAC(struct netif *pnetif);
extern uint8_t* LwIP_GetIP(struct netif *pnetif);
#endif

bool SettingSysInfoOnEnter(ITUWidget* widget, char* param)
{
#if defined(CFG_NET_WIFI)
#ifdef CFG_NET_WIFI_SDIO_NGPL
	unsigned char *ip_r = LwIP_GetIP(&xnetif[0]);
	unsigned char *mac = LwIP_GetMAC(&xnetif[0]);
#else
    ITPWifiInfo netInfo;
#endif
#elif defined(CFG_NET_ETHERNET)
    ITPEthernetInfo netInfo;
#endif
    char buf[32];
    char ip[16] = {0};

    if (!settingSysInfoIPAddrText)
    {
        settingSysInfoIPAddrText = ituSceneFindWidget(&theScene, "settingSysInfoIPAddrText");
        assert(settingSysInfoIPAddrText);

        settingSysInfoMacText = ituSceneFindWidget(&theScene, "settingSysInfoMacText");
        assert(settingSysInfoMacText);

        settingSysInfoHWVersionText = ituSceneFindWidget(&theScene, "settingSysInfoHWVersionText");
        assert(settingSysInfoHWVersionText);

        settingSysInfoSWVersionText = ituSceneFindWidget(&theScene, "settingSysInfoSWVersionText");
        assert(settingSysInfoSWVersionText);
    }

#ifdef CFG_NET_WIFI
#ifdef CFG_NET_WIFI_SDIO_NGPL
	ipaddr_ntoa_r((const ip_addr_t*)ip_r, ip, sizeof(ip));
#else
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &netInfo);
    ipaddr_ntoa_r((const ip_addr_t*)&netInfo.address, ip, sizeof(ip));
#endif
#elif defined(CFG_NET_ETHERNET)
    netInfo.index = 0;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &netInfo);
    ipaddr_ntoa_r((const ip_addr_t*)&netInfo.address, ip, sizeof(ip));
#else
    strcpy(ip, theConfig.ipaddr);
#endif // CFG_NET_WIFI

    ituTextSetString(settingSysInfoIPAddrText, ip);

    buf[0] = '\0';
#ifdef CFG_NET_ENABLE
#ifdef CFG_NET_WIFI_SDIO_NGPL
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)mac[0],
        (uint8_t)mac[1],
        (uint8_t)mac[2],
        (uint8_t)mac[3],
        (uint8_t)mac[4],
        (uint8_t)mac[5]);
#else
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
        (uint8_t)netInfo.hardwareAddress[0],
        (uint8_t)netInfo.hardwareAddress[1],
        (uint8_t)netInfo.hardwareAddress[2],
        (uint8_t)netInfo.hardwareAddress[3],
        (uint8_t)netInfo.hardwareAddress[4],
        (uint8_t)netInfo.hardwareAddress[5]);
#endif
#endif // CFG_NET_ENABLE
    ituTextSetString(settingSysInfoMacText, buf);

    ituTextSetString(settingSysInfoHWVersionText, CFG_HW_VERSION);
    ituTextSetString(settingSysInfoSWVersionText, CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);

    return true;
}

void SettingSysInfoReset(void)
{
    settingSysInfoIPAddrText = NULL;
}
