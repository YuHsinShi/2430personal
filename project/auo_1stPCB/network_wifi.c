#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include "iniparser/iniparser.h"
#include "ctrlboard.h"
#include "network_config.h"

/* Eason Refined in Mar. 2020 */


/* WIFI Static Varibles */
static struct timeval       tvStart     = {0, 0}, tvEnd     = {0, 0},
                            tvStartWifi = {0, 0}, tvEndWifi = {0, 0};
static bool                 networkWifiIsReady, process_set  = false;
static int                  wifimgr_init = WIFIMGR_ECODE_NOT_INIT;

WIFI_MGR_SETTING            gWifiSetting;



/* ======================================================================================= */

/**
  *
  *Static Network Functions
  *
  */

/* ======================================================================================= */

static void ResetWifi(void)
{
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    if (Ethernet_Wifi_DualMAC == 0)
        gWifiSetting.setting.dhcp     = theConfig.dhcp;
    else
        gWifiSetting.setting.dhcp     = 1; //if eth+wifi daul work, use DHCP IP.


    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    gWifiSetting.setting.ipaddr[0] = atoi(strtok_r(buf,  ".", &saveptr));
    gWifiSetting.setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    gWifiSetting.setting.netmask[0] = atoi(strtok_r(buf,  ".", &saveptr));
    gWifiSetting.setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    gWifiSetting.setting.gw[0] = atoi(strtok_r(buf,  ".", &saveptr));
    gWifiSetting.setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));
}


static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Ctrlboard] WifiCallback connection finish \n");
            networkWifiIsReady = true;
#if TEST_WIFI_DOWNLOAD
            sleep(5);
            createHttpThread();
#endif
            if (Ethernet_Wifi_DualMAC == 0)
                WebServerInit();

#ifdef CFG_NET_FTP_SERVER
		    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
		    ftpd_init();
#endif

#if defined(CFG_NET_WIFI_SDIO_NGPL)
            if (theConfig.wifi_mode == WIFIMGR_MODE_SOFTAP){
                snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       theConfig.ssid);
                snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   theConfig.password);
                gWifiSetting.secumode = WifiMgr_Secu_ITE_To_8189F(theConfig.secumode);
            }
#endif
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S:
            printf("[Ctrlboard] WifiCallback connection disconnect 30s \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION:
            printf("[Ctrlboard] WifiCallback connection reconnection \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT:
            printf("[Ctrlboard] WifiCallback connection temp disconnect \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL:
            printf("[Ctrlboard] WifiCallback connecting fail, please check ssid,password,secmode \n");
			WifiMgr_Sta_Disconnect();
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_CANCEL:
            printf("[Ctrlboard] WifiCallback connecting end to sleep/cancel \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_SAVE_INFO:
            snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       theConfig.ssid);
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   theConfig.password);
#if defined(CFG_NET_WIFI_SDIO_NGPL)
            gWifiSetting.secumode = WifiMgr_Secu_ITE_To_8189F(theConfig.secumode);
#else
            snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN,   theConfig.secumode);
#endif
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_CLEAN_INFO:
            printf("[Ctrlboard] WifiCallback clean connecting info \n");
            snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       "");
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   "");
#if defined(CFG_NET_WIFI_SDIO_NGPL)
            gWifiSetting.secumode = 0;
#else
            snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN,   "");
#endif
        break;

        default:
            printf("[Ctrlboard] WifiCallback unknown %d state  \n",nState);
        break;
    }
}


static int NetworkWifiPowerSleep(void)
{
    int  ret = WIFIMGR_ECODE_NOT_INIT;
    int  process_tv; //msec
    bool WifiNotReady = false;

#if defined(CFG_NET_WIFI_SDIO_NGPL)

    /* ======================  For 8189FTV  ====================== */
    /* Confirm current status of sleep mode */
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup) {
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);

        if (theConfig.wifi_mode == WIFIMGR_MODE_SOFTAP) {
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL); //determine wifi softAP mode
            ret = WifiMgr_Init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
        } else {
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
            ret = WifiMgr_Init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
        }
    }
    /* ====================================================== */

#elif defined(CFG_NET_WIFI_8188EUS) || defined(CFG_NET_WIFI_8188FTV)

    /* ======================  For 8188EUS  ====================== */
    /* Confirm current status of sleep mode */
    if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup) {
        while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)) {
            printf("[%s] Wait wifi dongle plugin... \n", __FUNCTION__);
            usleep(200*1000);
            WifiNotReady = true;
        }

        // sleep to wakeup , wait for 5s to initialize
        gettimeofday(&tvStartWifi, NULL);
        process_tv = 0;

        do {
            usleep(1000*1000);
            gettimeofday(&tvEndWifi, NULL);
            process_tv = (int)itpTimevalDiff(&tvStartWifi, &tvEndWifi);

            if (process_tv > Network_Time_Delay) {
                printf("[%s] ready to init wifi \n", __FUNCTION__);
                break;
            }

            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep) {
                printf("[%s] fast to sleep \n", __FUNCTION__);
                break;
            }
        } while (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup);

        if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep) {
            // fast sleep , do not init wifi mgr
        } else {
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);

            snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       theConfig.ssid);
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   theConfig.password);
            snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN,   theConfig.secumode);

            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == wakeup_to_sleep) {
                // fast sleep , do not init wifi mgr
            } else {
                ret = WifiMgr_Init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
                usleep(200*1000);
                //Delay a while to ensure wifi mgr is inited and task is ready.
                ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);
            }
        }
    }
    /* ====================================================== */
#endif

    return ret;
}




/* ======================================================================================= */

/**
  *
  *Network Functions
  *
  */

/* ======================================================================================= */

void NetworkWifiPreSetting(void)
{
    networkWifiIsReady = false;

    snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       theConfig.ssid);
    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   theConfig.password);
#if defined(CFG_NET_WIFI_SDIO_NGPL)
    gWifiSetting.secumode = WifiMgr_Secu_ITE_To_8189F(theConfig.secumode);

    if (theConfig.wifi_mode == WIFIMGR_MODE_SOFTAP){
        snprintf(gWifiSetting.ssid,     WIFI_SSID_MAXLEN,       theConfig.ap_ssid);
        snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN,   theConfig.ap_password);
    }
#else
    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN,   theConfig.secumode);
#endif

    gWifiSetting.wifiCallback = wifiCallbackFucntion;

    /* Reset WIFI IP*/
    ResetWifi();

    gettimeofday(&tvStart, NULL);
}


/* For WIFI main task in network_main.c  */
void NetworkWifiProcess(void)
{
    int process_tv; //msec

    gettimeofday(&tvEnd, NULL);

    process_tv = (int)itpTimevalDiff(&tvStart, &tvEnd);

    if (process_tv > Network_Time_Delay && process_set == false) {
#if !defined(CFG_NET_WIFI_SDIO_NGPL)
        while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)) {
            usleep(100*1000);
            printf("Wait WIFI device(USB) get ready... \n");
        }
#endif

        WifiMgr_Sta_Switch(theConfig.wifi_on_off);
        printf("[%s] WIFI mode: %s mode, WIFI(%s) \n", __FUNCTION__,
            theConfig.wifi_mode ? "Soft AP":"Station", theConfig.wifi_on_off ? "ON":"OFF");

        if (theConfig.wifi_mode == WIFIMGR_MODE_SOFTAP){
#if defined(CFG_NET_WIFI_SDIO_NGPL)
            WifiMgr_HostAP_First_Start();
#endif
            wifimgr_init = WifiMgr_Init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
        } else {
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode

            WifiMgr_Sta_Switch(theConfig.wifi_on_off);
            wifimgr_init = WifiMgr_Init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
        }

#if TEST_PING_WIFI
        while(!networkWifiIsReady){
            usleep(100*1000);
        }
        ping_main();
#endif

        process_set = true;
    } else if (process_set == true) {
        networkWifiIsReady = (bool)WifiMgr_Sta_Is_Available(&process_tv); //Return: NGPL(SSID Len)

#if defined(CFG_POWER_SLEEP)
        wifimgr_init = NetworkWifiPowerSleep();
#endif

#if TEST_CHANGE_AP
        if (networkWifiIsReady)
            link_differnet_ap();
#endif
    }
}


bool NetworkWifiIsReady(void)
{
    return networkWifiIsReady;
}


void NetworkWifiModeSwitch(void)
{
	int ret;

	ret = WifiMgr_Sta_HostAP_Switch(gWifiSetting);
}

/* ======================================================================================= */




/* Useless!! For only build WIFI in indoor project */
#if !defined(CFG_NET_ETHERNET)
bool NetworkIsReady(void)
{
    return false;
}

bool NetworkServerIsReady(void)
{
    return false;
}

void NetworkExit(void)
{
    //Not implement in network_wifi.c
}

void NetworkSntpUpdate(void)
{
    //Not implement in network_wifi.c
}
#endif

