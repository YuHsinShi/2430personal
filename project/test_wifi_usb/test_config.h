#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ping.h"
#include "wifiMgr.h"
#include "ite/ite_usbex.h"

/* Only test by Wifi USB type project */
#define wifi_on_off      1
#define network_ap_layer 1 //always open

#if network_ap_layer
#define test_iperf		 0 //Make sure you have iperf lib and open HTTP
#define tcp_client_test	 0
#define test_connect	 0 //Don't OPEN while test iperf or socket
#define test_scan_ap	 0
#define test_dns         0
#define test_ping        0
#define test_arp_request 0
#endif

/* Connect AP test */
#if test_connect
#define test_connect_interval 10 //sec, must bigger than 10

#define SSID_1 "CNAP"
#define PW_1 "12345678"
#define SEC_1 "7"

#define SSID_2 "dd-wrt-ITE"
#define PW_2 "12345678"
#define SEC_2 "7"
#endif

/* Scan AP test */
#if test_scan_ap
#define test_scan_interval    10 //sec
#endif

#define SSID "IOT_PZ_2072"
#define PW   "12345678"
#define SEC  "7"

/* test API */
int iperf_test_client(void);
int network_tcp_client_main(void);
int dns_resolve_main(int argc, char *argv[]);
void dns_resolve(void);
void ping_main(void);