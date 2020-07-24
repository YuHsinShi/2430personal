#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/ith.h"
#include "ite/itp.h"
#ifdef CFG_NET_WIFI
#include "wifiMgr.h"
#endif
#ifdef CFG_NET_FTP_SERVER
#include <unistd.h>
#include "ftpd.h"
#endif


#define TEST_WIFI_DOWNLOAD  0
#define TEST_PING_ETH       0
#define TEST_PING_WIFI      0
#if defined (CFG_NET_WIFI_SDIO_NGPL)
#define TEST_CHANGE_AP      0  //close(0), open(1)
#define change_ap_time     30  //don't less than 10
#endif


/* Ethernet Network Macro */
#define DHCP_TIMEOUT_MSEC       (60 * 1000) //60sec
#define HEARTBEAT_TIMEOUT_SEC   (30 * 60)
#define ARP_REQUEST_INTERVAL    5


/* WIFI Network Macro */
#define Network_Time_Delay      (5 * 1000) //Default 5 sec


/* Ethernet WIFI Dual MAC Macro */
#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
#define Ethernet_Wifi_DualMAC 1
#else
#define Ethernet_Wifi_DualMAC 0
#endif


/* test API */
int createHttpThread(void);
void link_differnet_ap(void);
void ping_main(void);
