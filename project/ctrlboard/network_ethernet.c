#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include "iniparser/iniparser.h"
#include "ctrlboard.h"
#include "network_config.h"

/* Eason Refined in Mar. 2020 */


/* Ethernet Static Varebles */
static bool                 networkIsReady, networkServerIsReady;
static bool                 networkToReset, networkQuit;
static bool                 networkHeartbeatIsReady;
static time_t               networkLastRecvTime;
static int                  networkSocket;
static unsigned int         networkIpJumpTimer;
static ITPEthernetInfo      networkInfo;
static ITPEthernetSetting   settingCopy;

/* ======================================================================================= */

/**
  *
  *Static Network Functions
  *
  */

/* ======================================================================================= */


#ifdef CFG_UDP_HEARTBEAT
static void NetworkInitHeartbeat(void)
{
    struct sockaddr_in  sockAddr;
    int                 val = 1;

    networkSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (networkSocket == -1)
        return;

    ioctlsocket(networkSocket, FIONBIO, &val);

    memset((char *)&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family         = AF_INET;
    sockAddr.sin_addr.s_addr    = htonl(INADDR_ANY);
    sockAddr.sin_port           = htons(CFG_UDP_HEARTBEAT_PORT);

    if (bind(networkSocket, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
    {
        closesocket(networkSocket);
        networkSocket = -1;
        return;
    }
    networkHeartbeatIsReady = true;
}


static void NetworkResetHeartbeat(void)
{
    if (networkHeartbeatIsReady && networkSocket != -1)
    {
        closesocket(networkSocket);
        networkSocket = -1;
    }
    NetworkInitHeartbeat();
}
#endif // CFG_UDP_HEARTBEAT


static void NetworkIPJump(void)
{
    char ip[16] = {0};

    ++networkIpJumpTimer;
    if (networkIpJumpTimer > ARP_REQUEST_INTERVAL && settingCopy.ipaddr[3] < 255){
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_ETHERNET_SEND_ARP, NULL);

        if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IP_DUPLICATE, NULL))
        {
            if (settingCopy.dhcp)
                settingCopy.dhcp = 0;

            if (settingCopy.ipaddr[3] < 254){
                settingCopy.ipaddr[3]++;
            } else {
                printf("Has no IP can jump, reset to x.x.x.2 and retry\n");
                settingCopy.ipaddr[3] = 2; //192.168.x.1 is also gateway
            }

            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &settingCopy);

            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &networkInfo);
            ipaddr_ntoa_r((const ip_addr_t *)&networkInfo.address, ip, sizeof(ip));
        }

        networkIpJumpTimer = 0;
    }
}


static void NetworkCenterServer(void)
{
    time_t              now = time(NULL);
    struct sockaddr_in  sockAddr;
    int                 slen = sizeof(sockAddr);

#ifdef CFG_UDP_HEARTBEAT
    int                 count, diff;
    char                buf[4];

    if (!networkHeartbeatIsReady)
        NetworkInitHeartbeat();

    // check center is alive or not
    diff = (int) difftime(now, networkLastRecvTime);
    if (diff >= HEARTBEAT_TIMEOUT_SEC)
    {
        networkServerIsReady = false;
    }

    count = recvfrom(networkSocket, buf, 1, 0, (struct sockaddr *)&sockAddr, &slen);
    if (count > 0)
    {
        networkServerIsReady    = true;
        networkLastRecvTime     = now;

        // send heartbeat packet back to center
        //sockAddr.sin_family = AF_INET;
        //sockAddr.sin_addr.s_addr = inet_addr(theCenterInfo.ip);

        count = sendto(networkSocket, "*", 1, 0, (const struct sockaddr *)&sockAddr, sizeof(sockAddr));
    }
#endif         // CFG_UDP_HEARTBEAT
}


static void ResetEthernet(void)
{
    ITPEthernetSetting  setting[CFG_NET_ETHERNET_COUNT];
    unsigned long       mscnt = 0;
    char                buf[16], *saveptr;
    int                 i;

    for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++)
    {
        memset(&setting[i], 0, sizeof(ITPEthernetSetting));

        setting[i].index = i;
    }

    // dhcp
    if (Ethernet_Wifi_DualMAC == 0)
        setting[0].dhcp     = theConfig.dhcp;
    else
        setting[0].dhcp     = 0; //if eth+wifi daul work, use static IP.

    // autoip
    setting[0].autoip   = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    setting[0].ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting[0].ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    setting[0].netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting[0].netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    setting[0].gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting[0].gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting[0].gw[3] = atoi(strtok_r(NULL, " ", &saveptr));

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);

    printf("Wait ethernet cable to plugin...\n");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        mscnt += 1000;
        putchar('.');
        fflush(stdout);

        /*When DHCP is setting enable but cable is disconnect, use default IP*/
        if (setting[CFG_NET_ETHERNET_COUNT - 1].dhcp == true)
        {
            if (mscnt >= DHCP_TIMEOUT_MSEC)
            {
                printf("\nDHCP timeout, cable no plugin, use default settings\n");
                setting[CFG_NET_ETHERNET_COUNT - 1].dhcp = setting[CFG_NET_ETHERNET_COUNT - 1].autoip = 0;
                ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);

                break;
            }
        }
    }

#if !defined(WIN32) && defined(CFG_NET_ETHERNET_DETECT_IP)
    /* Send ARP to detect IP duplication. */
    settingCopy = setting[CFG_NET_ETHERNET_COUNT - 1];
#if defined(CFG_NET_ETHERNET_DETECT_IP_DHCP)
    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IP_DUPLICATE, NULL)){
        setting.dhcp = 1;

        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
    }
#endif
#endif

    printf("\nWait DHCP settings");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        usleep(100000);
        mscnt += 100;

        putchar('.');
        fflush(stdout);

        if (mscnt >= DHCP_TIMEOUT_MSEC)
        {
            printf("\nDHCP timeout, use default settings\n");
            setting[CFG_NET_ETHERNET_COUNT - 1].dhcp = setting[CFG_NET_ETHERNET_COUNT - 1].autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char ip[16] = {0};

        for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++)
        {
            networkInfo.index   = i;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &networkInfo);
            ipaddr_ntoa_r((const ip_addr_t *)&networkInfo.address, ip, sizeof(ip));

            printf("IP address[%d]: %s\n", i, ip);
        }

        networkIsReady = true;
    }
}

/* ======================================================================================= */

/**
  *
  *Network Functions
  *
  */

/* ======================================================================================= */

void NetworkPreSetting(void)
{
    networkIsReady          = false;
    networkServerIsReady    = true;
#ifdef CFG_UDP_HEARTBEAT
    networkHeartbeatIsReady = false;
#endif
    networkToReset          = false;
    networkQuit             = false;
    networkSocket           = -1;
    networkIpJumpTimer      = 0;
    networkLastRecvTime     = time(NULL);

    /* Reset Ethernet IP*/
    ResetEthernet();

    /* Initial FTPD */
#if defined(CFG_NET_FTP_SERVER)
    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
    ftpd_init();
#endif

    /* If IP is Duplication, do IP juming */
#if !defined(WIN32) && defined(CFG_NET_ETHERNET_DETECT_IP_JUMP)
    NetworkIPJump();
#endif


#if TEST_PING_ETH
    sleep(5);
    ping_main();
#endif
}


/* For Ethernet main task in network_main.c  */
void NetworkEthernetProcess(void)
{
    networkIsReady = (bool)ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);

    if (NetworkServerIsReady())
        NetworkCenterServer();

    if (networkToReset)
    {
        ResetEthernet();
#ifdef CFG_UDP_HEARTBEAT
        NetworkResetHeartbeat();
#endif
        networkToReset = false;
    }
}


bool NetworkIsReady(void)
{
    return networkIsReady;
}


bool NetworkServerIsReady(void)
{
    return networkIsReady && networkServerIsReady;
}


void NetworkReset(void)
{
    networkToReset = true;
}


void NetworkExit(void)
{
    networkQuit = true;
}


bool NetworkIsExit(void)
{
    return networkQuit;
}

