#include <errno.h>
#include <stdlib.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ping.h"
#include "wifiMgr.h"
#ifdef CFG_TEST_USB_WIFI_LWIP
#include "ite/ite_usbex.h"
#endif

#if defined (CFG_TEST_SDIO_WIFI_LWIP)
#define test_iperf       0 //Make sure you have iperf lib and open HTTP
#define tcp_client_test  0
#define test_connect     0 //Don't OPEN while test iperf or socket
#define test_scan_ap     0
#define test_ping        0

#elif defined (CFG_TEST_USB_WIFI_LWIP)
#define wifi_on_off      1

#define test_iperf       0 //Make sure you have iperf lib and open HTTP
#define tcp_client_test  0
#define test_connect     0 //Don't OPEN while test iperf or socket
#define test_scan_ap     0
#define test_ping        0
#endif

/* Scan AP test */
#if test_scan_ap
#define test_scan_interval     10 //sec
#endif

/* Connect AP test */
#if test_connect
struct connect_info
{
	char ssid[WIFI_SSID_MAXLEN];
	char password[WIFI_PASSWORD_MAXLEN];
	char secumode[WIFI_SECUMODE_MAXLEN];
};

#define test_connect_interval 20 //sec

#define SSID_1  "CNAP"
#define PW_1    "12345678"
#define SEC_1   "7"

#define SSID_2  "IOT_PZ_2072"
#define PW_2    "12345678"
#define SEC_2   "7"
#endif

/* Scan AP test */
#if test_scan_ap
#define test_scan_interval      10 //sec
#endif

/* iPerf test */
#if test_iperf
#define test_iperf_type         0 //set(0): client.    set(1): server[Warning!!Must use LWIP 2.1.2]
#endif

/* Connect Info */
#define SSID "IOT_PZ_2072"
#define PW   "12345678"
#define SEC  "7"

/* test API */
int iperf_test_client(void);
int iperf_test_server(void);
int network_tcp_client_main(void);
int dns_resolve_main(int argc, char *argv[]);
void dns_resolve(void);
void ping_main(void);
