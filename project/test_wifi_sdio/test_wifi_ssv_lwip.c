#include <pthread.h>
#include <sys/ioctl.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"
#include "test_config.h"

/* Connect AP test */
#if test_connect
#define test_connect_interval 10 //sec
#endif

/* Scan AP test */
#if test_scan_ap
#define test_scan_interval     10 //sec
#endif

#define NGPL_WIFI_INFO_TIMER   20 //sec

////////////////////test scan function////////////////
#if test_scan_ap
static void scan_ap_loop(void)
{
	sleep(test_scan_interval);

	ioctl(ITP_DEVICE_WIFI_SSV, ITP_IOCTL_SCAN, NULL);
};
#endif
/////////////////////////////////////////////////

void NetworkInit(void)
{
}

void* TestAPFunc(void* arg)
{
    unsigned short info_time = 0;

	//Connect infomation: wifi_interactive_mode.c -> WifiNgplIoctl()
	printf("====>NGPL TestAPFunc: WIFI_LINK_AP");
	ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_WIFI_LINK_AP, NULL);
	usleep(500*1000);

	for (;;)
	{
        if (info_time == NGPL_WIFI_INFO_TIMER){
            ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_GET_INFO, NULL);
            info_time = 0;
        }
        info_time++;

		printf("WIFI Available: %s\n", ioctl(ITP_DEVICE_WIFI_NGPL, ITP_IOCTL_IS_AVAIL, NULL) ? "Yes":"No");

#if test_dns
        dns_resolve();

        sleep(2);
#endif

#if test_scan_ap
		scan_ap_loop();
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
	printf("====>NGPL: init itp\n");
	itpInit();
	usleep(5*100*1000);

	pthread_attr_init(&attr_ap);
	pthread_create(&task_ap, &attr_ap, TestAPFunc, NULL);

#if (CFG_NET_HTTP && test_iperf)
    sleep(10);
	iperf_test_client();
#endif

/* ping test */
#if test_ping
    sleep(5);

    ping_main();
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


