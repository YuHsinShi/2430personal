#include <pthread.h>
#include <sys/ioctl.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "test_config.h"

static char* SENDER_IP_ADDR;
static char ping_target;
static bool wifi_connect_all_finish = false;
static int AP_count, link_loop_time = 0;
static WIFI_MGR_SCANAP_LIST pList[64];
static WIFI_MGR_SETTING gWifiSetting;

////////////////////test scan function////////////////
#if test_scan_ap
static void scan_ap_loop(void)
{
    sleep(test_scan_interval);

    printf("========>start scan\n");
    AP_count = wifiMgr_get_scan_ap_info(pList);
    printf("========>found %d AP\n\n", AP_count);
};
#endif
/////////////////////////////////////////////////

/////////////////////////////////////////////////
#if test_ping
void ping_main(void)
{
	printf("====>ping test\n");
	ping_set_target("172.217.160.78"); //midea(101.37.128.72), google(172.217.160.78)
	ping_init();
}
#endif
/////////////////////////////////////////////////

#define DHCP_TIMEOUT_MSEC (5 * 1000) //5sec
static bool networkIsReady, networkToReset;
#if CFG_NET_ETHERNET
static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    printf("====>ResetEthernet\n");
    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;

    // dhcp
    setting.dhcp = 0;

    // autoip
    setting.autoip = 0;

    // ipaddr
    strcpy(buf, "192.168.190.61");
    setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, "255.255.255.0");
    setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, "192.168.190.1");
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
        char* ip;

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ip = ipaddr_ntoa((const ip_addr_t*)&info.address);

        printf("IP address: %s\n", ip);

        networkIsReady = true;
    }
}

static void* NetworkTask(void* arg)
{
	ResetEthernet();

	for (;;)
	{
		networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
		if (networkToReset)
		{
			ResetEthernet();
			networkToReset = false;
		}

		usleep(100*1000);
	}
	return NULL;
}

void NetworkInit_Eth(void)
{
    pthread_t task;

    networkIsReady = false;
	networkToReset = false;
    pthread_create(&task, NULL, NetworkTask, NULL);
}
#endif

#ifdef CFG_NET_WIFI
static int wifiCallbackFucntion(int nState)
{
    ITPWifiInfo wifiInfo;
    char* wifi_ip;

    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
			wifi_connect_all_finish = true;
#if tcp_client_test
            ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
            wifi_ip = ipaddr_ntoa((const ip_addr_t*)&wifiInfo.address);
			SENDER_IP_ADDR = wifi_ip;
            printf("====>Wifi IP: %s, Socket sender IP: %s\n", wifi_ip, SENDER_IP_ADDR);
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
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;

    }
}

static void ResetWifi()
{
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));
	printf("====>Reset Wifi\n");

    // dhcp
    gWifiSetting.setting.dhcp = 1; //always open

    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    gWifiSetting.setting.ipaddr[0] = 192;
    gWifiSetting.setting.ipaddr[1] = 168;
    gWifiSetting.setting.ipaddr[2] = 190;
    gWifiSetting.setting.ipaddr[3] = 101;

    // netmask
    gWifiSetting.setting.netmask[0] = 255;
    gWifiSetting.setting.netmask[1] = 255;
    gWifiSetting.setting.netmask[2] = 255;
    gWifiSetting.setting.netmask[3] = 0;

    // gateway
    gWifiSetting.setting.gw[0] = 192;
    gWifiSetting.setting.gw[1] = 168;
    gWifiSetting.setting.gw[2] = 190;
    gWifiSetting.setting.gw[3] = 1;

}

void NetworkInit_WIFI(void)
{
	snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, SSID_1);
	snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, PW_1);
	snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, "6");

	gWifiSetting.wifiCallback = wifiCallbackFucntion;
	ResetWifi();
}
#endif

void* TestAPFunc(void* arg)
{
	sleep(5);
	WifiMgr_clientMode_switch(wifi_on_off);

    printf("====>NetworkInit(WIFI part)\n");
    NetworkInit_WIFI();

    printf("====>init wifiMgr\n");
    wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
	usleep(500*1000);

#if test_ping
	sleep(10);

	ping_main();
#endif

	for (;;)
	{
#if test_scan_ap
		scan_ap_loop();
#endif

#if(!(test_scan_ap || test_connect))
		sleep(1);
#endif
	}
	return NULL;
}

void* TestFunc_Wifi_Ethernet(void* arg)
{
	pthread_t task_ap;
	pthread_attr_t attr_ap;

	// init pal
	printf("====>init itp, wait...\n");
	itpInit();
	usleep(5*100*1000);

    printf("====>NetworkInit(Eth part)\n");
    NetworkInit_Eth();

	pthread_attr_init(&attr_ap);
	pthread_create(&task_ap, &attr_ap, TestAPFunc, NULL); //WIFI Task

#if (CFG_NET_HTTP && test_iperf)
    sleep(10);
	iperf_test_client();
#endif

	for (;;)
	{
#if tcp_client_test
		network_tcp_client_main();
		printf("====>socket connect/recv failed\n");
#endif

		sleep(1);
	}
}


