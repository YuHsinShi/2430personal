#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/ith.h"
#include "ite/itp.h"
#include "ping.h"
#include "iniparser/iniparser.h"
#include "doorbell.h"

/* Only test by Ethernet project */
#define network_ap_layer 1 //always open

#if network_ap_layer
#define test_iperf		 0 //Make sure you have iperf lib and open HTTP
#define tcp_client_test	 0
#define test_dns         0
#define test_ping        0
#endif

/* test API */
int iperf_test_client(void);
int iperf_test_server(void);
int network_tcp_client_main(void);
void dns_resolve(void);
void ping_main(void);