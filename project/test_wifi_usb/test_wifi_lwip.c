#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "test_config.h"


static char* SENDER_IP_ADDR;
static char ping_target;
static bool wifi_connect_all_finish = false;
static int AP_count, link_loop_time = 0;
static WIFI_MGR_SCANAP_LIST pList[64];
static WIFI_MGR_SETTING gWifiSetting;

#if network_ap_layer
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


////////////////////test link function////////////////
#if test_connect
struct connect_info
{
	char ssid[WIFI_SSID_MAXLEN];
	char password[WIFI_PASSWORD_MAXLEN];
	char secumode[WIFI_SECUMODE_MAXLEN];
};

static struct connect_info ap_info[2];
static void link_info_setting(void)
{
	snprintf(ap_info[0].ssid, WIFI_SSID_MAXLEN, SSID_1);
	snprintf(ap_info[0].password, WIFI_PASSWORD_MAXLEN, PW_1);
	snprintf(ap_info[0].secumode, WIFI_SECUMODE_MAXLEN, SEC_1);

	snprintf(ap_info[1].ssid, WIFI_SSID_MAXLEN, SSID_2);
	snprintf(ap_info[1].password, WIFI_PASSWORD_MAXLEN, PW_2);
	snprintf(ap_info[1].secumode, WIFI_SECUMODE_MAXLEN, SEC_2);
};

static void link_differnet_ap(void)
{
	if(wifi_connect_all_finish)
		wifiMgr_clientMode_disconnect();

	usleep(500*1000);

	++link_loop_time;
	printf("\n========>[%d times] Connect to [%s]\n", link_loop_time, ap_info[link_loop_time%2].ssid);
	wifiMgr_clientMode_connect_ap(ap_info[link_loop_time%2].ssid, ap_info[link_loop_time%2].password, ap_info[link_loop_time%2].secumode);

	sleep(test_connect_interval);
};
#endif
/////////////////////////////////////////////////

////////////////////test DNS function////////////////
#if test_dns
static void dns_resolve(void)
{
	char* argv[] = {"showip", "iot1.midea.com.cn"};

	dns_resolve_main(ITH_COUNT_OF(argv), argv);
}

int dns_resolve_main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[46];

	if (argc != 2) {
		fprintf(stderr,"usage: showip hostname\n");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = lwip_getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
		//lwip_freeaddrinfo(res);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("\n\nIP addresses for %s:\n\n", argv[1]);

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;

		if (p->ai_family == AF_INET) { // IPv4
		  struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		  addr = &(ipv4->sin_addr);
		  ipver = "IPv4";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n\n", ipver, ipstr);
	}

	lwip_freeaddrinfo(res);

	return 0;
}
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
    gWifiSetting.setting.dhcp = 1;

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

void NetworkInit(void)
{
	snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, SSID);
	snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, PW);
	snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, SEC);
#if test_connect
	link_info_setting();
#endif
	gWifiSetting.wifiCallback = wifiCallbackFucntion;
	ResetWifi();
}
#endif

void* TestAPFunc(void* arg)
{
	WifiMgr_clientMode_switch(wifi_on_off);

#if network_ap_layer
    printf("====>init network\n");
    NetworkInit();

    printf("====>init wifi\n");
    wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);

#if test_connect
	while (!wifi_connect_all_finish){
		usleep(100*1000);
	}
#endif
#endif

#if test_dns
	//sleep(5);

	printf("First time doing DNS\n");
	dns_resolve();

	sleep(5);
#endif

#if test_ping
	sleep(10);

	ping_main();
#endif


    for (;;)
    {
		//printf("\n\nTask start!!!!!!!!!\n");
	    while (!wifi_connect_all_finish){
			usleep(100*1000);
		}
#if test_scan_ap
		scan_ap_loop();
#endif

#if test_connect
		link_differnet_ap();
#endif

#if test_dns
		dns_resolve();

		sleep(2);
#endif

		//printf("Task end!!!!!!!!!\n\n");
#if test_arp_request
		printf("====>ping count: %d\n", ping_count);
		if(ping_count%60 == 0){
			printf("====>Send DHCP Discover!!!!!\n");
			ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_RENEW_DHCP, NULL);
		}
#endif

#if(!(test_scan_ap || test_connect))
			sleep(1);
#endif
    }

    return NULL;
}

void* TestFunc(void* arg)
{
    pthread_t task_ap;
    pthread_attr_t attr_ap;

    // init pal
    printf("====>init itp\n");
    itpInit();
    usleep(5*100*1000);

	pthread_attr_init(&attr_ap);
	pthread_create(&task_ap, &attr_ap, TestAPFunc, NULL);

#if network_ap_layer
	while (!wifi_connect_all_finish){
		sleep(1);
	}

#if (CFG_NET_HTTP && test_iperf && !test_connect)
	sleep(10);

	iperf_test_client();
#endif

	for (;;)
    {
#if (tcp_client_test && !test_connect)
		network_tcp_client_main();
		printf("====>socket connect/recv failed\n");
#endif

        sleep(1);
    }
#endif
}


