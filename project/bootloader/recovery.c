#include <sys/ioctl.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"
#include "bootloader.h"
#include "config.h"

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static struct timeval tvStart = { 0, 0 }, tvEnd = { 0, 0 };
static bool networkIsReady, networkToReset;
static int networkSocket;

#ifdef CFG_NET_ENABLE
#include "wifiMgr.h"
#include "ite/itp.h"
#include <time.h>
#include <sys/ioctl.h>
#include <pthread.h>

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
static bool wifi_dongle_hotplug, need_reinit_wifimgr;
static int gInit = 0; // wifi init
#ifdef CFG_NET_WIFI_SDIO_NGPL
#define NGPL_Change_AP_test 0 //close(0), open(1)
#define change_ap_time 30 //don't less than 10
#endif
#else
#endif

static ITPEthernetInfo      networkInfo;
static ITPEthernetSetting   settingCopy;

static bool wifi_ready = false;

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
}

static int wifiCallbackFucntion(int nState)
{
	switch (nState)
	{
	case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
		printf("[Indoor]WifiCallback connection finish \n");
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
		snprintf(gWifiSetting.ssid, WIFI_SSID_MAXLEN, CFG_NET_WIFI_MP_SSID);
		snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, CFG_NET_WIFI_MP_PASSWORD);
#ifdef CFG_NET_WIFI_SDIO_NGPL
		gWifiSetting.secumode = CFG_NET_WIFI_MP_SECURITY;
#else
		snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, CFG_NET_WIFI_MP_SECURITY);
#endif
		break;

	case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_CLEAN_INFO:
		printf("[Indoor]WifiCallback clean connecting info \n");
		snprintf(gWifiSetting.ssid, WIFI_SSID_MAXLEN, "");
		snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, "");
#ifdef CFG_NET_WIFI_SDIO_NGPL
		gWifiSetting.secumode = 0;
#else
		snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, "");
#endif
		break;

	default:
		printf("[Indoor]WifiCallback unknown %d state  \n", nState);
		break;
	}
}

void NetworkWifiModeSwitch(void)
{
	int ret;

	ret = WifiMgr_Switch_ClientSoftAP_Mode(gWifiSetting);
}
#endif

static void ResetEthernet(void)
{
	ITPEthernetSetting  setting[CFG_NET_ETHERNET_COUNT];
	unsigned long       mscnt = 0;
	char                buf[16];
	int                 i;

	for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++)
	{
		memset(&setting[i], 0, sizeof(ITPEthernetSetting));

		setting[i].index = i;
	}

#ifdef CFG_NET_ETHERNET_MULTI_INTERFACE
	NetworkMultiNetif(&setting);
#else //default ethernet netif set
	setting[0].dhcp = 1;

	sleep(1); //wait for eXosip_init ready

	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);
#endif

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

#if !defined(WIN32)
	/* Send ARP to detect IP duplication. */
	settingCopy = setting[CFG_NET_ETHERNET_COUNT - 1];
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_ARP_REPLY, &settingCopy);
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
		char ip[16] = { 0 };

		for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++)
		{
			networkInfo.index = i;
			ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &networkInfo);
			ipaddr_ntoa_r((const ip_addr_t *)&networkInfo.address, ip, sizeof(ip));

			printf("IP address[%d]: %s\n", i, ip);
		}

		networkIsReady = true;
	}
}

static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
	int nTemp;
#else
	ResetEthernet();
#endif

	for (;;)
	{
#ifdef CFG_NET_WIFI
		gettimeofday(&tvEnd, NULL);

		nTemp = itpTimevalDiff(&tvStart, &tvEnd);
		if (nTemp>5000 && gInit == 0){
			printf("[%s] Init wifimgr \n", __FUNCTION__);

#ifndef CFG_NET_WIFI_SDIO_NGPL
			while (!ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_IS_DEVICE_READY, NULL)){
				sleep(1);
				printf("wait ITP_IOCTL_IS_DEVICE_READY \n");
			}
#endif

			WifiMgr_clientMode_switch(1);

			ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
			nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);

			gInit = 1;
		}
		else if (gInit == 1){
			networkIsReady = wifiMgr_is_wifi_available(&nTemp);
			networkIsReady = (bool)nTemp;
#if defined(CFG_POWER_SLEEP) && defined(CFG_NET_WIFI_SDIO_NGPL)
			if (ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_SLEEP_STATUS, NULL) == sleep_to_wakeup){
				// ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_ADD_NETIF, NULL);
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL);

				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_SLEEP, (void *)default_no_sleep_or_wakeup);
				ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_ENABLE, NULL); //determine wifi client mode
				//printf("wifiMgr_init ssid %s secumode 0x%x ============\n",gWifiSetting.ssid,gWifiSetting.secumode);

				nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
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
	snprintf(gWifiSetting.ssid, WIFI_SSID_MAXLEN, CFG_NET_WIFI_MP_SSID);
	snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, CFG_NET_WIFI_MP_PASSWORD);
	snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, CFG_NET_WIFI_MP_SECURITY);
	gWifiSetting.wifiCallback = wifiCallbackFucntion;
	ResetWifi();

	gettimeofday(&tvStart, NULL);
#endif

	pthread_create(&task, NULL, NetworkTask, NULL);
}
#endif

static ITCArrayStream arrayStream;
extern char tftppara[128];


struct FtpBuf
{
    uint8_t* buf;
    uint32_t pos;
};
 
static size_t FtpWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpBuf* out = (struct FtpBuf*)stream;
    size_t s;

    //LOG_DBG "FtpWrite(0x%X,%d,%d,0x%X)\n", buffer, size, nmemb, stream LOG_END

    assert(out->buf);
    s = size * nmemb;
    memcpy(&out->buf[out->pos], buffer, s);
    out->pos += s;
	
	LOG_DBG "FtpWrite(size: %d, pos: %d)\n", (int)s, out->pos LOG_END
    return s;
}

ITCStream* OpenRecoveryPackage(void)
{
    CURL *curl;
    CURLcode res;
    struct FtpBuf ftpBuf;

	// init ethernet device
#if defined(CFG_NET_ETHERNET) || defined(CFG_USB_ECM)
	itpRegisterDevice(ITP_DEVICE_ETHERNET, &itpDeviceEthernet);
	ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_INIT, NULL);
#endif

	// enable gpio interrupt
	ithIntrEnableIrq(ITH_INTR_GPIO);

	// init wifi device
#if defined (CFG_NET_WIFI)
	// init sdio device
#ifdef CFG_SDIO_ENABLE
	itpRegisterDevice(ITP_DEVICE_SDIO, &itpDeviceSdio);
	ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_INIT, NULL);
#endif

	// init sd0 device
#ifdef CFG_SD0_STATIC
	itpRegisterDevice(ITP_DEVICE_SD0, &itpDeviceSd0);
	ioctl(ITP_DEVICE_SD0, ITP_IOCTL_INIT, NULL);
#endif

	// init sd1 device
#ifdef CFG_SD1_STATIC
	itpRegisterDevice(ITP_DEVICE_SD1, &itpDeviceSd1);
	ioctl(ITP_DEVICE_SD1, ITP_IOCTL_INIT, NULL);
#endif

#if defined(CFG_USB0_ENABLE) || defined(CFG_USB1_ENABLE)
	if (ioctl(ITP_DEVICE_USB, ITP_IOCTL_IS_AVAIL, NULL))
#endif
	{
		itpRegisterDevice(ITP_DEVICE_WIFI, &itpDeviceWifi);
		printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI)\n");
#if !defined(CFG_NET_WIFI_SDIO_NGPL)
		ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
#endif
	}

#if defined(CFG_NET_WIFI_SDIO_NGPL)
	{
		itpRegisterDevice(ITP_DEVICE_WIFI_NGPL, &itpDeviceWifiNgpl);
		printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI_NGPL)\n");
		ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);
	}
#endif
#endif
	// init socket device
#if defined(CFG_NET_ENABLE) && !defined(CFG_NET_WIFI_REDEFINE) && !defined(CFG_USB_ECM)
	itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
	ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
#endif
	
	NetworkInit();

	while (!networkIsReady)
		sleep(1);

	ftpBuf.buf = malloc(0x1000000);
	if (!ftpBuf.buf)
	{
		LOG_ERR "malloc fail.\n" LOG_END
		return NULL;
	}
    ftpBuf.pos = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl)
    {
        LOG_ERR "curl_easy_init() fail.\n" LOG_END
        goto error;
    }
#if defined (CFG_UPGRADE_RECOVERY_TFTP)
    curl_easy_setopt(curl, CURLOPT_URL, "tftp://" CFG_UPGRADE_RECOVERY_SERVER_ADDR "/" CFG_UPGRADE_FILENAME);
#elif defined (CFG_ENABLE_UART_CLI)
	printf("\ntftppara=%s\n", tftppara);
	curl_easy_setopt(curl, CURLOPT_URL, tftppara);	
#else
    curl_easy_setopt(curl, CURLOPT_URL, "ftp://" CFG_UPGRADE_RECOVERY_SERVER_ADDR "/" CFG_UPGRADE_FILENAME);
#endif
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FtpWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpBuf);
 
#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
    res = curl_easy_perform(curl);
    /* always cleanup */ 
    curl_easy_cleanup(curl);
    if (CURLE_OK != res)
    {
        LOG_ERR "curl fail: %d\n", res LOG_END
        goto error;
    }
    curl_global_cleanup();
    itcArrayStreamOpen(&arrayStream, ftpBuf.buf, ftpBuf.pos);
    return &arrayStream.stream;

error:
    curl_global_cleanup();
    return NULL;
}