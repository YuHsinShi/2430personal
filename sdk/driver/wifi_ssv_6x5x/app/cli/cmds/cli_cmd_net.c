/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#include <porting.h>
#include <ssv_common.h>
#include <host_config.h>

#include <netstack.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
#include <drv/ssv_drv.h>
#include <net_mgr.h>

#include <log.h>
#include "cli_cmd.h"

#include "cli_cmd_net.h"

#if (CLI_ENABLE==1)
extern ssv_type_s32 netmgr_show(void);

static void netif_display(struct netdev *netif)
{
    ssv_type_u32 bcast;
    ssv_assert(netif!=NULL);

    LOG_PRINTF("%s\tLink encap:Ethernet  HWaddr %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        netif->name,  netif->hwmac[0], netif->hwmac[1], netif->hwmac[2],
        netif->hwmac[3], netif->hwmac[4], netif->hwmac[5]);
    bcast = (netif->ipaddr & netif->netmask) | ~netif->netmask;
    LOG_PRINTF("\tinet addr:%d.%d.%d.%d  ", IPV4_ADDR(&netif->ipaddr));
    LOG_PRINTF("Bcast:%d.%d.%d.%d  ", IPV4_ADDR(&bcast));
    LOG_PRINTF("Mask:%d.%d.%d.%d\r\n", IPV4_ADDR(&netif->netmask));

#if USE_ICOMM_LWIP
    LOG_PRINTF("\t%s ", ((netif->flags&NETDEV_IF_UP)? "UP": "DOWN"));
    LOG_PRINTF("%s ", ((netif->flags&NETDEV_IF_BROADCAST)? "BROADCAST": "POINT-TO-POINT"));
    LOG_PRINTF("%s ", ((netif->flags&NETDEV_IF_LINK_UP)? "RUNNING": "LINK-DOWN"));
#endif
    LOG_PRINTF(" MTU:%d  GW:%d.%d.%d.%d\r\n", netif->mtu, IPV4_ADDR(&netif->gw));
    LOG_PRINTF("\r\n");

}


#if 0
static struct netif *netif_malloc(char *name)
{
    s32 i;
    for(i=0; i<CONFIG_MAX_NETIF; i++) {
        if (sg_netif_mask & (1<<i))
            continue;
        sg_netif_mask |= (1<<i);
        memset((void *)&sg_netif_list[i], 0, sizeof(struct netif));
        strcpy(sg_netif_list[i].name, name);
        return &(sg_netif_list[i]);
    }
    return NULL;
}

static void netif_free(struct netif *netif)
{
    s32 i;
    for(i=0; i<CONFIG_MAX_NETIF; i++) {
        if (netif != &sg_netif_list[i])
            continue;
        sg_netif_mask &= ~(1<<i);
    }
}
#endif



void cmd_ifconfig(ssv_type_s32 argc, char *argv[])
{
#if 0
    extern struct netif *netif_list;
    struct netif *netif;
#else
    ssv_type_u32 num_dev = 3, real_num = 0;
    struct netdev dev[3];
    struct netdev *ptr;
#endif
//    struct ip_addr ipaddr, netmask, gw;
//    s32 res, mac[6];

    /**
         * Usage: ifconfig <name> [up|down|ip-addr] [netmask xxx.xxx.xxx.xxx] [hw xx:xx:xx:xx:xx.xx]
         */
    if (argc == 1) {
#if 0
        for(netif=netif_list; netif!=NULL; netif=netif->next)
            netif_display(netif);
#else
        real_num = netdev_getallnetdev(dev, num_dev);
        for(ptr = dev;real_num > 0;real_num--)
        {
            netif_display(ptr);
            ptr++;
        }
#endif
        return;
    }
    else
    {
        char * cmd_name = "ifconfig";
        show_invalid_para(cmd_name);
        return;
    }
#if 0
    if (argc == 2 || argc == 3) {
        netif = netif_find(argv[1]);
        if (netif == NULL) {
            LOG_PRINTF("ifconfig: unknown device %s\r\n", argv[1]);
            return;
        }
        if (argc == 2) {
            /**
                       * Display information of the specified network interface
                       */
            netif_display(netif);
            return;
        }
        else if (strcmp(argv[2], "up") == 0) {
            /* Interface up */
          netif_set_up(netif);
        }
        else if (strcmp(argv[2], "down") == 0) {
            /* Interface down */
            netif_set_down(netif);
        }
        else LOG_PRINTF("ifconfig: unknown parameter %s\r\n", argv[2]);
        return;
    }

    if (((argc==5)||(argc==7)) && strcmp(argv[3], "netmask")==0) {
        ipaddr.addr = netstack_inet_addr(argv[2]);
        netmask.addr = netstack_inet_addr(argv[4]);
        netif = netif_find(argv[1]);
        if (netif == NULL) {
            LOG_PRINTF("ifconfig: dev %s not found !\r\n", argv[1]);
            return;
        }

        if (argc==7 && strcmp(argv[5], "hw")==0) {
            res = sscanf(argv[6], "%02x:%02x:%02x:%02x:%02x:%02x",
                mac, mac+1, mac+2, mac+3, mac+4, mac+5);
            if (res != 6) {
                LOG_PRINTF("ifconfig: invalid parameter %s\r\n", argv[5]);
                return;
            }
            netif->hwaddr[0] = mac[0];
            netif->hwaddr[1] = mac[1];
            netif->hwaddr[2] = mac[2];
            netif->hwaddr[3] = mac[3];
            netif->hwaddr[4] = mac[4];
            netif->hwaddr[5] = mac[5];
        }
        gw.addr = 0;
        netif_add(netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);

        return;
    }

    LOG_PRINTF("\nifconfig: invalid parameter(s)!!\n\n");
#endif
}

#if 0
void cmd_route(s32 argc, char *argv[])
{
    extern struct netif *netif_list;
    extern struct netif *netif_default;
    struct netif *netif;
    struct ip_addr gw, subnet;
    u8 bufstr[20];

    /**
         *  route add default  xxx.xxx.xxx.xxx
         */
    if (argc==4 && !strcmp(argv[1], "add") && !strcmp(argv[2], "default")) {
        gw.addr = netstack_inet_addr(argv[3]);
        for(netif = netif_list; netif != NULL; netif = netif->next) {
            if (!ip_addr_netcmp(&gw, &netif->ip_addr, &netif->netmask))
                continue;
            netif_set_gw(netif, &gw);
            netif_set_default(netif);
            return;
        }
        LOG_PRINTF("route: unknown gateway %s\n", argv[3]);
        return;
    }


    LOG_PRINTF("%-20s %-20s %-20s %-10s\r\n", "Destination",
    "Gateway", "Netmask", "Iface");

    for(netif = netif_list; netif != NULL; netif = netif->next) {
      if (! netif_is_up(netif))
          continue;

        subnet.addr = netif->ip_addr.addr & netif->netmask.addr;
        LOG_PRINTF("%-20s ", ipv4_to_str(subnet.addr, bufstr));
        LOG_PRINTF("%-20s ", "*");
        LOG_PRINTF("%-20s ", ipv4_to_str(netif->netmask.addr, bufstr));
        LOG_PRINTF("%-10s\r\n", netif->name);

    }

    if (netif_default != NULL) {
        LOG_PRINTF("%-20s ", "default");
        LOG_PRINTF("%-20s ", ipv4_to_str(netif_default->gw.addr, bufstr));
        LOG_PRINTF("%-20s ", "0.0.0.0");
        LOG_PRINTF("%-10s\r\n", netif_default->name);
    }

}
#endif //#if 0


#if(PING_SUPPORT==1)
void cmd_ping(ssv_type_s32 argc, char *argv[])
{
    ssv_type_s32 res;
    #if NETAPP_SUPPORT
    res = net_app_run(argc, argv);
    #else
    res = net_app_ping(argc, argv);
    #endif
    if (res < 0)
        LOG_PRINTF("ping failure !!\n");
}
#endif

#ifdef __APP_TTCP__
void cmd_ttcp(ssv_type_s32 argc, char *argv[])
{
    ssv_type_s32 res;
    res = net_app_run(argc, argv);
    if (res < 0)
        LOG_PRINTF("ttcp failure !!\n");
}
#endif

#if(IPERF3_ENABLE==1)
void cmd_iperf3(ssv_type_s32 argc, char *argv[])
{
    ssv_type_s32 res;
    #if NETAPP_SUPPORT
    res = net_app_run(argc, argv);
    #else
    res = net_app_iperf3(argc, argv);
    #endif
    if (res < 0)
        LOG_PRINTF("iperf failure !!!\n");
}
#endif

#if NETAPP_SUPPORT
void cmd_net_app(ssv_type_s32 argc, char *argv[])
{
    if (argc==2 && ssv6xxx_strcmp(argv[1], "show")==0)
    {
        net_app_show();
    }
    else
    {
        char * cmd_name = "netapp";
        show_invalid_para(cmd_name);
        return;
    }
}
#endif

void cmd_net_mgr(ssv_type_s32 argc, char *argv[])
{
    if (argc==2 && ssv6xxx_strcmp(argv[1], "show")==0)
        netmgr_show();
#ifdef  NET_MGR_AUTO_JOIN
    else if (argc==3 && ssv6xxx_strcmp(argv[1], "remove")==0)
        netmgr_apinfo_remove(argv[2]);
#endif
    else
    {
       char * cmd_name = "netmgr";
       show_invalid_para(cmd_name);
    }
}
#endif
