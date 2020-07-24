#include <pthread.h>
#include <sys/ioctl.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "test_config.h"

/*=====================WIFI=====================*/
static WIFI_MGR_SETTING     gWifiSetting;
static WIFI_MGR_SCANAP_LIST pList[64];
static unsigned int wifi_connect_all_finish = 0;
static int link_loop_time = 0;

////////////////////test scan function////////////////
#if test_scan_ap
static void scan_ap_loop(void)
{
    int get_scan_count = 0;
	sleep(test_scan_interval);

	//ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_SCAN, NULL);
	get_scan_count = wifiMgr_get_scan_ap_info(pList);
    printf("====>Scan count(%d)\n", get_scan_count);
};
#endif
/////////////////////////////////////////////////


////////////////////test link function////////////////
#if test_connect
static struct connect_info Connect_Info[2];
static void link_info_setting(void)
{
	snprintf(Connect_Info[0].ssid,      WIFI_SSID_MAXLEN,       SSID_1);
	snprintf(Connect_Info[0].password,  WIFI_PASSWORD_MAXLEN,   PW_1);
	snprintf(Connect_Info[0].secumode,  WIFI_SECUMODE_MAXLEN,   SEC_1);

	snprintf(Connect_Info[1].ssid,      WIFI_SSID_MAXLEN,       SSID_2);
	snprintf(Connect_Info[1].password,  WIFI_PASSWORD_MAXLEN,   PW_2);
	snprintf(Connect_Info[1].secumode,  WIFI_SECUMODE_MAXLEN,   SEC_2);
};

static void link_differnet_ap(void)
{
	if(wifi_connect_all_finish){
		if (wifiMgr_clientMode_disconnect() == WIFIMGR_ECODE_OK)
		    wifi_connect_all_finish = 0;
	}

	usleep(1000*1000);

	++link_loop_time;
	printf("\n========>[%d times] Connect to [%s]\n", link_loop_time, Connect_Info[link_loop_time%2].ssid);
	wifiMgr_clientMode_connect_ap(Connect_Info[link_loop_time%2].ssid, Connect_Info[link_loop_time%2].password, *Connect_Info[link_loop_time%2].secumode);

	sleep(test_connect_interval);
};
#endif
/////////////////////////////////////////////////

int CallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
            wifi_connect_all_finish = 1;
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
            snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, SSID);
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, PW);
            gWifiSetting.secumode = WifiMgr_Secu_ITE_To_8189F(SEC);
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_SLEEP_CLEAN_INFO:
            printf("[Indoor]WifiCallback clean connecting info \n");
            snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, "");
            snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, "");
            gWifiSetting.secumode = 0;
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;
    }
}


static void PreSettingWifi(void)
{
    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index  = 0;
    gWifiSetting.setting.dhcp   = 1;
    gWifiSetting.setting.autoip = 0;

    for (int i = 0; i < 4; i++){
        gWifiSetting.setting.ipaddr[i] = 0;
        gWifiSetting.setting.netmask[i]= 0;
        gWifiSetting.setting.gw[i]     = 0;
    }
}
/*==============================================*/



/*=====================Ethernet====================*/
#define DHCP_TIMEOUT_MSEC (5 * 1000) //5sec
static bool networkIsReady, networkToReset;

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

void NetworkInit(void)
{
    pthread_t task;

    networkIsReady = false;
	networkToReset = false;
    pthread_create(&task, NULL, NetworkTask, NULL);
}
/*=============================================*/

void* TestAPFunc(void* arg)
{
	//Connect infomation: test_config.h(SSID/PW, and SEC no need)
	sleep(5);
	printf("====>NGPL TestAPFunc: WIFI_LINK_AP");
	wifiMgr_clientMode_connect_ap(SSID, PW, SEC);
	usleep(500*1000);

    while(!ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
        usleep(100*1000);
    }

    #if test_connect
    link_info_setting();
    #endif

	for (;;)
	{
    #if test_scan_ap
		scan_ap_loop();
    #endif

    #if test_connect
		link_differnet_ap();
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
	printf("====>NGPL: init itp, wait...\n");
	itpInit();
	usleep(5*100*1000);

#ifdef CFG_NET_WIFI
    gWifiSetting.wifiCallback = CallbackFucntion;
    PreSettingWifi();

    WifiMgr_clientMode_switch(1);
    wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
#endif

#if CFG_NET_ETHERNET
    printf("====>Ethernet NetworkInit\n");
    NetworkInit();
#endif

#ifdef CFG_NET_WIFI
	pthread_attr_init(&attr_ap);
	pthread_create(&task_ap, &attr_ap, TestAPFunc, NULL);
#endif

    while(!ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL)){
        usleep(200*1000);
    }

    #if (CFG_NET_HTTP && test_iperf)
    #if test_iperf_type
    #ifdef CFG_NET_LWIP_2
	iperf_test_server(); //Warning!!Only support LWIP 2.1.2
    #endif
    #else
    iperf_test_client(); //Support LWIP 1.4.1/2.1.2
    #endif
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


