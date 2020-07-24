#include <pthread.h>
#include <sys/ioctl.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "test_config.h"


#define DHCP_TIMEOUT_MSEC (5 * 1000) //5sec
static bool networkIsReady, networkToReset;

#if network_ap_layer
static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;

    // dhcp
    setting.dhcp = 1;

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
		
		if(networkIsReady){
#if test_dns
			dns_resolve();

			sleep(2);
#endif
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
#endif

void* TestFunc(void* arg)
{
    pthread_t task_ap;
    pthread_attr_t attr_ap;
	int socket_test_state;

    // init pal
    printf("====>init itp\n");
    itpInit();
    usleep(5*100*1000);

#if network_ap_layer
    printf("====>NetworkInit\n");
    NetworkInit();

/* iperf test */
#if (CFG_NET_HTTP && test_iperf)
	sleep(5);

	printf("====>Start iperf test\n");
	iperf_test_client();
#endif

/* ping test */
#if test_ping
	sleep(5);

	ping_main();
#endif
#endif

	for (;;)
    {
/* socket test */
#if tcp_client_test
		socket_test_state = network_tcp_client_main();
		printf("====>socket connect/recv failed\n");
#endif
		sleep(1);
    }

	return NULL;
}


