#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "ctrlboard.h"
#include "wifiMgr.h"
#ifdef CFG_NET_FTP_SERVER
#include <unistd.h>
#include "ftpd.h"
#endif

//#define TEST_WIFI_DOWNLOAD

#ifdef TEST_WIFI_DOWNLOAD
    #include "curl/curl.h"
    #define HTTP_DOWNLOAD_ADDRESS "http://192.168.191.100:23455/WAV/002-96000-24.wav"
#endif


#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
static bool networkIsReady, networkToReset;
static int networkSocket;

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
static bool wifi_dongle_hotplug, need_reinit_wifimgr;
static int gInit =0; // wifi init
#ifdef CFG_NET_WIFI_SDIO_NGPL
#define NGPL_Change_AP_test 0 //close(0), open(1)
#define change_ap_time 30 //don't less than 10
#endif
#endif
#ifdef TEST_WIFI_DOWNLOAD
static int                  gnTest = -1;

struct FtpBuf
{
    uint8_t     *buf;
    uint32_t    pos;
};

static size_t FtpWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpBuf   *out = (struct FtpBuf *)stream;
    size_t          s;
/*
    LOG_DEBUG "FtpWrite(0x%X,%d,%d,0x%X)\n", buffer, size, nmemb, stream LOG_END
 */
    // assert(out->buf);

    s           = size * nmemb;
    memcpy(&out->buf[out->pos], buffer, s);
    out->pos    += s;
    if (out->pos > 510000)
    {
        printf("FtpWrite(0x%X,%d,%d,0x%X)\n", buffer, size, nmemb, stream);
        out->pos = 0;
    }

    return s;
}

static int  httpDownload()
{
    CURL            *curl;
    CURLcode        res;
    struct FtpBuf   ftpBuf;

    //InitNetwork();
    printf("[httpDownload] download : start\r\n");

    ftpBuf.buf  = malloc(0x200000);
    ftpBuf.pos  = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl        = curl_easy_init();
    if (!curl)
    {
        printf("curl_easy_init() fail.\n");
        goto error;
    }

    curl_easy_setopt(   curl,   CURLOPT_URL,            HTTP_DOWNLOAD_ADDRESS);
    //curl_easy_setopt(curl, CURLOPT_USERPWD, FTP_USER_NAME_PASSWORD);
    curl_easy_setopt(   curl,   CURLOPT_WRITEFUNCTION,  FtpWrite);
    curl_easy_setopt(   curl,   CURLOPT_WRITEDATA,      &ftpBuf);

    #ifndef NDEBUG
    curl_easy_setopt(   curl,   CURLOPT_VERBOSE,        1L);
    #endif

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (CURLE_OK != res)
    {
        printf("curl fail: %d\n", res);
        goto error;
    }

    curl_global_cleanup();

    if (ftpBuf.buf)
    {
        free(ftpBuf.buf);
        ftpBuf.buf = NULL;
    }
    return NULL;

error:
    curl_global_cleanup();
    if (ftpBuf.buf)
    {
        free(ftpBuf.buf);
        ftpBuf.buf = NULL;
    }
    return NULL;
}

static int TestHTTPDownload()
{
    int nRet;
    int bIsAvail;
    do
    {
        nRet = wifiMgr_is_wifi_available(&bIsAvail);
        if (bIsAvail)
        {
            httpDownload();
        }
        usleep( 2000000);
        usleep( 2000000);
        printf("ready to download \n\n");
        usleep(2000000);
    } while (1);
}

static int createHttpThread()
{
    pthread_t       task;
    pthread_attr_t  attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, (255 * 1024));
    pthread_create(&task, &attr, TestHTTPDownload, NULL);
    return 0;
}

#endif

#if NGPL_Change_AP_test
static unsigned int link_loop_time = 0;
static void link_differnet_ap(void)
{
    ++link_loop_time;
    if((link_loop_time % (change_ap_time*2)) == 0){
        wifiMgr_clientMode_disconnect();
        usleep(500*1000);
        wifiMgr_clientMode_connect_ap("SSID_1", "Password_1", RTW_SECURITY_WPA_WPA2_MIXED);
    }else if((link_loop_time % (change_ap_time*2)) == change_ap_time){
        wifiMgr_clientMode_disconnect();
        usleep(500*1000);
        wifiMgr_clientMode_connect_ap("SSID_2", "Password_2", RTW_SECURITY_WPA_WPA2_MIXED);
    }else{
        printf("====>link_loop_time: %d\n", link_loop_time);
    }
};
#endif

static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;

    // dhcp
    setting.dhcp = theConfig.dhcp;

    // autoip
    setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);

    printf("Wait ethernet cable to plugin");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }

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
            setting.dhcp = setting.autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char ip[16] = {0};

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ipaddr_ntoa_r((const ip_addr_t*)&info.address, ip, sizeof(ip));

        printf("IP address: %s\n", ip);

        networkIsReady = true;
    }
}


#ifdef CFG_NET_WIFI
static void ResetWifi(void)
{
//    ITPEthernetSetting setting;
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    gWifiSetting.setting.dhcp = 1;

    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    gWifiSetting.setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    gWifiSetting.setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    gWifiSetting.setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));
}

static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
        #ifdef TEST_WIFI_DOWNLOAD
            sleep(5);
            createHttpThread();
        #endif
            WebServerInit();

#ifdef CFG_NET_FTP_SERVER
		    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
		    ftpd_init();
#endif
#ifdef CFG_NET_WIFI_SDIO_NGPL
            if (theConfig.wifi_mode == WIFI_SOFTAP){
                snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
                snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
                gWifiSetting.secumode = theConfig.secumode;
            }
#endif
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S:
            printf("[Indoor]WifiCallback connection disconnect 30s \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION:
            printf("[Indoor]WifiCallback connection reconnection \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT:
            printf("[Indoor]WifiCallback connection temp disconnect \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL:
            printf("[Indoor]WifiCallback connecting fail, please check ssid,password,secmode \n");
			wifiMgr_clientMode_disconnect();
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_CANCEL:
            printf("[Indoor]WifiCallback connecting end to sleep/cancel \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_SAVE_INFO:
            snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
#ifdef CFG_NET_WIFI_SDIO_NGPL
            gWifiSetting.secumode = theConfig.secumode;
#else
            snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
#endif
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;
    }
}

void NetworkWifiModeSwitch(void)
{
	int ret;

	ret = WifiMgr_Switch_ClientSoftAP_Mode(gWifiSetting);
}
#endif

static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
    int nTemp;
#else
    ResetEthernet();
#endif

#if defined(CFG_NET_FTP_SERVER) && !defined(CFG_NET_WIFI)
    ftpd_setlogin(theConfig.user_id, theConfig.user_password);
    ftpd_init();
#endif

    for (;;)
    {
#ifdef CFG_NET_WIFI
        gettimeofday(&tvEnd, NULL);

        nTemp = itpTimevalDiff(&tvStart, &tvEnd);
        if (nTemp>5000 && gInit == 0){
            printf("[%s] Init wifimgr \n", __FUNCTION__);

#ifndef CFG_NET_WIFI_SDIO
            while(!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
                sleep(1);
                printf("wait ITP_IOCTL_IS_DEVICE_READY \n");
            }
#endif

            WifiMgr_clientMode_switch(theConfig.wifi_on_off);
            printf("[NetworkTask] WIFI mode: %d, ON/OFF: %d \n", theConfig.wifi_mode, theConfig.wifi_on_off);

            if (theConfig.wifi_mode == WIFI_SOFTAP){
                //ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL); //determine wifi softAP mode
#if defined(CFG_NET_WIFI_SDIO_NGPL)
                WifiMgr_firstStartSoftAP_Mode();
#endif
                nTemp = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
            }else {
                ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
                nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
            }

            gInit = 1;
        } else if (gInit == 1){
            networkIsReady = wifiMgr_is_wifi_available(&nTemp);
            networkIsReady = (bool)nTemp;
#if defined(CFG_POWER_SLEEP) && defined(CFG_NET_WIFI_SDIO_NGPL)
            if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup){
               // ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
                ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);

                ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);
                if (theConfig.wifi_mode == WIFI_SOFTAP){
                    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFIAP_ENABLE, NULL); //determine wifi softAP mode
                    nTemp = wifiMgr_init(WIFIMGR_MODE_SOFTAP, 0, gWifiSetting);
                }else {
                    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
                    //printf("wifiMgr_init ssid %s secumode 0x%x ============\n",gWifiSetting.ssid,gWifiSetting.secumode);

                    nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
                }
            }
#endif

#if NGPL_Change_AP_test
            if (networkIsReady)
                link_differnet_ap();
#endif
        }
#else
        networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
#endif

        if (networkToReset)
        {
#ifndef CFG_NET_WIFI
            ResetEthernet();
#endif
            networkToReset = false;
        }
        sleep(1);
    }
    return NULL;
}

void NetworkInit(void)
{
    pthread_t task;

    networkIsReady = false;
    networkToReset = false;
    networkSocket = -1;
#ifdef CFG_NET_WIFI
    snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
#ifdef CFG_NET_WIFI_SDIO_NGPL
     gWifiSetting.secumode = theConfig.secumode;
     if (theConfig.wifi_mode == WIFI_SOFTAP){
        snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ap_ssid);
        snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.ap_password);
     }
#else
    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
#endif
    gWifiSetting.wifiCallback = wifiCallbackFucntion;
    ResetWifi();

    gettimeofday(&tvStart, NULL);
#endif

    pthread_create(&task, NULL, NetworkTask, NULL);
}

bool NetworkIsReady(void)
{
    return networkIsReady;
}

void NetworkReset(void)
{
    networkToReset  = true;
}
