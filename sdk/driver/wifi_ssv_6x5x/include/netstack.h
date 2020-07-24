/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
#ifndef _NETSTACK_H_
#define _NETSTACK_H_
#include <ssv_types.h>
#if USE_ICOMM_LWIP
#include <netstack_def.h>
#endif

#define socket(a,b,c)           lwip_socket(a,b,c)
#define sendto(a,b,c,d,e,f)     lwip_sendto(a,b,c,d,e,f)
#define close(s)                lwip_close(s)
#define bind(a,b,c)             lwip_bind(a,b,c)
#define connect(a,b,c)          lwip_connect(a,b,c)
#define send(a,b,c,d)           lwip_send(a,b,c,d)

struct netdev
{
    char name[26];
    ssv_type_u16 mtu;
    ssv_type_u32 ipaddr;
    ssv_type_u32 netmask;
    ssv_type_u32 gw;
    ssv_type_u32 flags;    
    unsigned char hwmac[6];        
    ssv_type_bool add_state;
};


typedef struct st_dhcps_info{
	/* start,end are in host order: we need to compare start <= ip <= end */
	ssv_type_u32 start_ip;              /* start address of leases, in host order */
	ssv_type_u32 end_ip;                /* end of leases, in host order */
	ssv_type_u32 max_leases;            /* maximum number of leases (including reserved addresses) */

    ssv_type_u32 subnet;
    ssv_type_u32 gw;
    ssv_type_u32 dns;

	ssv_type_u32 auto_time;             /* how long should udhcpd wait before writing a config file.
			                         * if this is zero, it will only write one on SIGUSR1 */
	ssv_type_u32 decline_time;          /* how long an address is reserved if a client returns a
			                         * decline message */
	ssv_type_u32 conflict_time;         /* how long an arp conflict offender is leased for */
	ssv_type_u32 offer_time;            /* how long an offered address is reserved */
	ssv_type_u32 max_lease_sec;         /* maximum lease time (host order) */
	ssv_type_u32 min_lease_sec;         /* minimum lease time a client can request */
}dhcps_info;

typedef struct st_dhcpdipmac
{
    ssv_type_u32 ip;
    ssv_type_u8 mac[6];
    ssv_type_u8 reserved[2];
}dhcpdipmac;

struct netstack_ip_addr {
  ssv_type_u32 addr;
};

typedef struct netstack_ip_addr netstack_ip_addr_t;

#define NS_OK           0   //Everything is fine
#define NS_NG           -1
#define NS_ERR_MEM      -2  //Out of memory
#define NS_ERR_ARG      -3  //Invalid arguement
#define NS_ERR_IMP      -4  //Not implement yet
#define NS_ERR_CALLER   -5  //Not define error, need to check

typedef int (*eth_input_fn)(void *dat, ssv_type_u32 len);
typedef int (*reg_fn)(eth_input_fn);
typedef void (*netdev_link_callback_t)(void *dat);
#define netstack_inet_addr(cp)         netstack_ipaddr_addr(cp)

// Transfer L2 packet to netstack
int netstack_input(void *data, ssv_type_u32 len, ssv_type_u8 vif_idx);

// Transfer netstack packet to L2
int netstack_output(void* net_interface, void *data, ssv_type_u32 len);

//init netstack
int netstack_init(void *config);
int netstack_deinit(void *config);
//Add device with specific setting
int netdev_init(struct netdev * pdev, ssv_type_bool dft_dev, ssv_type_bool init_up);

//get hw mac
int netdev_getmacaddr(const char *ifname, ssv_type_u8 *macaddr);
//get ipinfo
int netdev_getipv4info(const char *ifname, ssv_type_u32 *ip, ssv_type_u32 *gw, ssv_type_u32 *netmask);
//set ipinfo
int netdev_setipv4info(const char *ifname, ssv_type_u32 ip, ssv_type_u32 gw, ssv_type_u32 netmask);

//get dns server
//int netdev_get_ipv4dnsaddr(const char *ifname, u32 *dnsserver);
//set dns server
//int netdev_set_ipv4dnsaddr(const char *ifname, const u32 *dnsserver);

//get interface status
int netdev_check_ifup(const char *ifname);

//set interface up
int netdev_l3_if_up(const char *ifname);
//set interface down
int netdev_l3_if_down(const char *ifname);
//interface link up cb
void netdev_link_up_cb(void *ifname);
//interface link down cb
void netdev_link_down_cb(void *ifname);
//get all netdev
ssv_type_u32 netdev_getallnetdev(struct netdev * pdev, ssv_type_u32 num);
// set default
int netdev_set_default(const char *ifname);

ssv_type_s32 netdev_get_netdev(struct netdev *pdev, ssv_type_u8 if_idx);
//set dhcp client on dev
int dhcpc_wrapper_set(const char *ifname, const ssv_type_bool enable);

// UDP operation

int netstack_udp_send(void* data, ssv_type_u32 len, ssv_type_u32 srcip, ssv_type_u16 srcport, ssv_type_u32 dstip, ssv_type_u16 dstport, ssv_type_s16 rptsndtimes);
int netstack_tcp_send(void* data, ssv_type_u32 len, ssv_type_u32 srcip, ssv_type_u16 srcport, ssv_type_u32 dstip, ssv_type_u16 dstport);
int netstack_dhcps_info_set(dhcps_info *if_dhcps, dhcps_info *des_if_dhcps, ssv_type_u8 vif_idx);
int netstack_udhcpd_start(void);
int netstack_udhcpd_stop(void);
int netstack_dhcp_ipmac_get(dhcpdipmac *ipmac, int *size_count);
int netstack_find_ip_in_arp_table(ssv_type_u8 * mac,netstack_ip_addr_t *ipaddr);
int netstack_find_mac_in_arp_table(ssv_type_u8 * mac,netstack_ip_addr_t *ipaddr);
int netstack_dhcp_ip_get_by_mac(ssv_type_u8* Mac, ssv_type_u32* ip);
int netstack_etharp_unicast (ssv_type_u8 *dst_mac, netstack_ip_addr_t *ipaddr);

ssv_type_u32 netstack_ipaddr_addr(const char *cp);
char *netstack_inet_ntoa(netstack_ip_addr_t addr);

ssv_type_u16 netstack_ip4_addr1_16(ssv_type_u32* ipaddr);
ssv_type_u16 netstack_ip4_addr2_16(ssv_type_u32* ipaddr);
ssv_type_u16 netstack_ip4_addr3_16(ssv_type_u32* ipaddr);
ssv_type_u16 netstack_ip4_addr4_16(ssv_type_u32* ipaddr);

#endif //#ifndef _NETSTACK_H_
