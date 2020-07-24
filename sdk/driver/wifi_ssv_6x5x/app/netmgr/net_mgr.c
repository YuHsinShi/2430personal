/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <ssv_hal.h>

#include <drv/ssv_drv.h>
#include <host_config.h>
#include <ssv_devinfo.h>
#include <netstack.h>
#include "net_mgr.h"


#include <log.h>
#include <rtos.h>
#include <ssv_timer.h>
#include <string.h>

extern ssv_type_u16 g_sta_channel_mask;
extern SSV6XXX_USER_SCONFIG_OP ssv6xxx_user_sconfig_op;
extern struct Host_cfg g_host_cfg;

static ssv_type_bool g_switch_join_cfg_b = false;
static wifi_sta_join_cfg g_join_cfg_data;
static struct netdev g_netifdev[MAX_VIF_NUM];

typedef struct st_netmgr_sconfig_result
{
    //wifi_sec_type    sec_type;
    struct cfg_80211_ssid ssid;
    ssv_type_u8                  password[MAX_PASSWD_LEN+1];
    ssv_type_u8                  channel;
    ssv_type_u8                  dat;
    ssv_type_u8                  valid;
}netmgr_sconfig_result;

static netmgr_sconfig_result sconfig_result;

extern ssv_type_u32 g_sconfig_solution;
extern ssv_type_u16 g_SconfigChannelMask;
extern ssv_type_u32 g_Sconfig5gChannelMask;
extern ssv_type_u8 g_sconfig_auto_join;

ssv_type_bool g_sconfig_user_mode;
ssv_type_bool g_sconfig_running=FALSE;


extern struct ssv6xxx_ieee80211_bss * ssv6xxx_wifi_find_ap_ssid(struct cfg_80211_ssid *ssid);
extern struct ssv6xxx_ieee80211_bss * ssv6xxx_wifi_find_ap_ssid_best(struct cfg_80211_ssid *ssid);

#if !NET_MGR_NO_SYS
static void netmgr_task(void *arg);
#endif
static void netmgr_wifi_reg_event(void);
int netmgr_wifi_switch_to_sta(wifi_sta_join_cfg *join_cfg, ssv_type_u8 join_channel);
static int _netmgr_wifi_switch(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_join_cfg *join_cfg, ssv_type_u16 scanning_channel_mask, ssv_type_u32 scanning_5g_channel_mask, ssv_type_bool sta_reset);
static void _netmgr_wifi_recovery_cb(void);
#if !NET_MGR_NO_SYS
struct task_info_st st_netmgr_task[] =
{
    { "netmgr_task",  (OsMsgQ)0, 10, OS_NETMGR_TASK_PRIO, NETMGR_STACK_SIZE, NULL, netmgr_task   },
};
#endif

static ssv_type_bool g_wifi_is_joining_b = false; //
//struct resp_evt_result *sconfig_done_cpy=NULL;

#ifdef  NET_MGR_AUTO_JOIN
#define NET_MGR_USER_AP_COUNT     10
int netmgr_autojoin_process(void);

typedef struct st_user_ap_info
{
    ssv_type_bool valid;
    struct cfg_80211_ssid ssid;
    char password[MAX_PASSWD_LEN+1];
    int join_times;
    ssv_type_u8 vif_idx;
}user_ap_info;

static user_ap_info g_user_ap_info[NET_MGR_USER_AP_COUNT];

static struct ssv6xxx_ieee80211_bss *g_ap_list_p = NULL;

void netmgr_apinfo_clear();
void netmgr_apinfo_remove(char *ssid);
user_ap_info * netmgr_apinfo_find(char *ssid);
ssv_type_bool netmgr_apinfo_find_in_aplist(struct ssv6xxx_ieee80211_bss * ap_list_p, int count, char * ssid);
user_ap_info * netmgr_apinfo_find_best(struct ssv6xxx_ieee80211_bss * ap_list_p, int count);
void netmgr_apinfo_save();
void netmgr_apinfo_set(user_ap_info *ap_info, ssv_type_bool valid);
static int netmgr_apinfo_autojoin(user_ap_info *ap_info);
void netmgr_apinfo_show();
#endif

#ifdef NET_MGR_AUTO_RETRY

typedef enum {
    S_TRY_INIT,
    S_TRY_RUN,
    S_TRY_STOP,
    S_TRY_INVALID = 0XFF
}E_AUTO_TRY_STATE;

typedef struct st_auto_retry_ap_info
{
    struct cfg_80211_ssid ssid;
    char password[MAX_PASSWD_LEN+1];
    ssv_type_u8                  vif_idx;
    E_AUTO_TRY_STATE g_auto_retry_status;
    int  g_auto_retry_times;
    ssv_type_u32  g_auto_retry_start_time;
}auto_retry_ap_info;

extern int  g_auto_retry_times_delay;
extern int  g_auto_retry_times_max;
static auto_retry_ap_info g_auto_retry_ap[MAX_VIF_NUM];
extern void netmgr_auto_retry_update(ssv_type_u8 vif_idx);
extern void netmgr_auto_retry_show();
void netmgr_auto_retry_reset(ssv_type_u8 vif_idx);
#endif

static netmgr_cfg g_netmgr_config[MAX_VIF_NUM];

static int netmgr_dhcpd_start(ssv_type_bool enable ,ssv_type_u8 vif_idx);
static int netmgr_dhcpc_start(ssv_type_bool enable, ssv_type_u8 vif_idx);

int netmgr_wifi_sconfig_done(ssv_type_u8 *resp_data, ssv_type_u32 len, ssv_type_bool IsUDP,ssv_type_u32 port);

typedef struct st_wifi_sec_info
{
    char *sec_name;
    char dfl_password[MAX_PASSWD_LEN+1];
}wifi_sec_info;

const wifi_sec_info g_sec_info[SSV6XXX_SEC_MAX] =
{
    {"open",   ""},                              // WIFI_SEC_NONE
    {"wep40",  {0x31,0x32,0x33,0x34,0x35,0x00,}}, // WIFI_SEC_WEP_40
    {"wep104", "0123456789012"},                 // WIFI_SEC_WEP_104
    {"wpa",    "secret00"},                      // WIFI_SEC_WPA_PSK
    {"wpa2",   "secret00"},                      // WIFI_SEC_WPA2_PSK
    {"wps",    ""}, // WIFI_SEC_WPS

};
char *if_name[] = {"wlan0", "wlan1"};

#if 0
static void netif_link_change_cb(struct netif *netif)
{
    if (netif->flags & NETIF_FLAG_LINK_UP)
        g_netif_link_up = 1;
    else
        g_netif_link_up = 0;
    LOG_DEBUGF(LOG_L4_NETMGR, ("wlan0: link %s !\r\n", ((g_netif_link_up==1)? "up": "down")));
}

static void netif_status_change_cb(struct netif *netif)
{
    MsgEvent *msg_evt = NULL;
    if (netif->flags & NETIF_FLAG_UP){
        g_netif_status_up = 1;
        msg_evt = msg_evt_alloc();
        if(NULL!=msg_evt)
        {
            msg_evt->MsgType = MEVT_NET_MGR_EVENT;
            msg_evt->MsgData = MSG_SCONFIG_DONE;
            msg_evt_post(st_netmgr_task[0].qevt, msg_evt);
        }
        else
        {
            LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
        }
    }
    else
        g_netif_status_up = 0;

    LOG_DEBUGF(LOG_L4_NETMGR, ("wlan0: status %s !\r\n", ((g_netif_status_up==1)? "up": "down")));
}
#endif

static void netmgr_cfg_default(netmgr_cfg *p_cfg)
{
    if (p_cfg)
    {
        p_cfg->ipaddr  = netstack_inet_addr(DEFAULT_IPADDR);
        p_cfg->netmask = netstack_inet_addr(DEFAULT_SUBNET);
        p_cfg->gw      = netstack_inet_addr(DEFAULT_GATEWAY);
        p_cfg->dns     = netstack_inet_addr(DEFAULT_DNS);

        p_cfg->dhcps.start_ip     = netstack_inet_addr(DEFAULT_DHCP_START_IP);
        p_cfg->dhcps.end_ip     = netstack_inet_addr(DEFAULT_DHCP_END_IP);

        p_cfg->dhcps.max_leases     = DEFAULT_DHCP_MAX_LEASES;
        p_cfg->dhcps.subnet     = netstack_inet_addr(DEFAULT_SUBNET);
        p_cfg->dhcps.gw     = netstack_inet_addr(DEFAULT_GATEWAY);
        p_cfg->dhcps.dns     = netstack_inet_addr(DEFAULT_DNS);

        p_cfg->dhcps.auto_time     = DEFAULT_DHCP_AUTO_TIME;
        p_cfg->dhcps.decline_time     = DEFAULT_DHCP_DECLINE_TIME;
        p_cfg->dhcps.conflict_time     = DEFAULT_DHCP_CONFLICT_TIME;
        p_cfg->dhcps.offer_time     = DEFAULT_DHCP_OFFER_TIME;
        p_cfg->dhcps.max_lease_sec     = DEFAULT_DHCP_MAX_LEASE_SEC;
        p_cfg->dhcps.min_lease_sec     = DEFAULT_DHCP_MIN_LEASE_SEC;

        p_cfg->s_dhcpc_enable = 0;
        p_cfg->s_dhcpd_enable = 0;
        p_cfg->s_dhcpc_status = 0;
        p_cfg->s_dhcpd_status = 0;
    }
}

netdev_link_callback_t g_wifi_link_up_cb = NULL;
netdev_link_callback_t g_wifi_link_down_cb = NULL;

void netmgr_wifi_link_register_cb(netdev_link_callback_t link_up_cb, netdev_link_callback_t link_down_cb)
{
    if (link_up_cb)
    {
        g_wifi_link_up_cb = link_up_cb;
    }

    if (link_down_cb)
    {
        g_wifi_link_down_cb = link_down_cb;
    }
}

void ssv_netmgr_add_netdev(ssv_type_u8 vif_idx,ssv_type_bool init_up)
{

    //if (!g_netmgr_config[vif_idx].s_dhcpc_enable)
    if(g_netifdev[vif_idx].add_state == FALSE)
    {
        LOG_PRINTF("ssv_netmgr_add_netdev vif=%d,init_up=%d\r\n",vif_idx,init_up);
        ssv6xxx_wifi_get_mac((ssv_type_u8 *)g_netifdev[vif_idx].hwmac,vif_idx);
        OS_MemCPY((void *)(g_netifdev[vif_idx].name),if_name[vif_idx], sizeof(WLAN_IFNAME));
        g_netifdev[vif_idx].ipaddr = g_netmgr_config[vif_idx].ipaddr;
        g_netifdev[vif_idx].netmask = g_netmgr_config[vif_idx].netmask;
        g_netifdev[vif_idx].gw =  g_netmgr_config[vif_idx].gw;
        //if(vif_idx == 0)
            netdev_init(&g_netifdev[vif_idx], FALSE, init_up);
        //else
        //    netdev_init(&g_netifdev[vif_idx], FALSE, init_up);
        g_netifdev[vif_idx].add_state = TRUE;
    }
    //if(dhcpd_up)
    //    netmgr_dhcpd_start(true,vif_idx);
}
void ssv_netmgr_init_netdev(ssv_type_bool default_cfg)
{
    //u8 vif_idx;
    //ssv_type_bool dhcpd_start_needed = (netmgr_wifi_check_ap(&vif_idx) && g_netmgr_config[0].s_dhcpd_enable);
    ssv_type_u8 i=0;

    LOG_DEBUGF(LOG_L4_NETMGR,("g_netmgr_config[0].s_dhcpd_enable=%d\r\n",g_netmgr_config[0].s_dhcpd_enable));
    for(i=0;i<MAX_VIF_NUM;i++) //only setup vif 0 at init statge
    {
        if (default_cfg || (g_netmgr_config[i].ipaddr == 0))
        {
            netmgr_cfg_default(&g_netmgr_config[i]);
        }
        OS_MemSET((void *)&g_netifdev[i], 0, sizeof(struct netdev));
    }
    ssv6xxx_wifi_reg_rx_cb((data_handler)netstack_input);

    netmgr_wifi_link_register_cb(netdev_link_up_cb, netdev_link_down_cb);

}
void ssv_netmgr_init(ssv_type_bool default_cfg)
{

#if !NET_MGR_NO_SYS
	OsMsgQ *msgq = NULL;
	ssv_type_s32 qsize = 0;
#endif
    #ifdef NET_MGR_AUTO_JOIN
    netmgr_apinfo_clear();
    #endif

    #ifdef NET_MGR_AUTO_RETRY
    {
        int i;
        //g_auto_retry_status = S_TRY_INVALID;
        if (g_auto_retry_times_delay == 0)
            g_auto_retry_times_delay = NET_MGR_AUTO_RETRY_DELAY;
        //if (g_auto_retry_times_max == 0)
        //    g_auto_retry_times_max = NET_MGR_AUTO_RETRY_TIMES;
        //g_auto_retry_times = 0;
        //g_auto_retry_start_time = 0;
        OS_MemSET((void *)&g_auto_retry_ap, 0, sizeof(g_auto_retry_ap));
        for(i=0;i<MAX_VIF_NUM;i++)
        {
            g_auto_retry_ap[i].g_auto_retry_status = S_TRY_INVALID;
        }
    }
    #endif

    g_wifi_is_joining_b = false;

    #if !NET_MGR_NO_SYS
    msgq = &st_netmgr_task[0].qevt;
    qsize = (ssv_type_s32)st_netmgr_task[0].qlength;
    if (OS_MsgQCreate(msgq, qsize) != OS_SUCCESS)
    {
        LOG_PRINTF("OS_MsgQCreate faild\r\n");
        return;
    }

    if (OS_TaskCreate(st_netmgr_task[0].task_func,
                  st_netmgr_task[0].task_name,
                  st_netmgr_task[0].stack_size<<4,
                  NULL,
                  st_netmgr_task[0].prio,
                  NULL) != OS_SUCCESS)
    {
        LOG_PRINTF("OS_TaskCreate faild\r\n");
        return;
    }
    #endif
    #if(ENABLE_SMART_CONFIG==1)
    g_SconfigChannelMask=DEFAULT_SCONFIG_CHANNEL_MASK;
    g_Sconfig5gChannelMask=DEFAULT_SCONFIG_5G_CHANNEL_MASK;

    if(ssv6xxx_user_sconfig_op.UserSconfigPaserData!=NULL){
        ssv6xxx_wifi_reg_promiscuous_rx_cb((promiscuous_data_handler)ssv6xxx_user_sconfig_op.UserSconfigPaserData);
    }

    OS_MemSET(&sconfig_result,0,sizeof(sconfig_result));
    #endif
    ssv6xxx_wifi_reg_recovery_cb(_netmgr_wifi_recovery_cb);

    netmgr_wifi_reg_event();
}

void netmgr_cfg_get(netmgr_cfg *p_cfg, ssv_type_u8 vif_idx)
{
    if (p_cfg)
    {
        OS_MemCPY(p_cfg, &g_netmgr_config[vif_idx], sizeof(netmgr_cfg));
    }
}

void netmgr_cfg_set(netmgr_cfg *p_cfg, ssv_type_u8 vif_idx)
{
    if (p_cfg)
    {
        OS_MemCPY(&g_netmgr_config[vif_idx], p_cfg, sizeof(netmgr_cfg));
    }
}

#if 0
static void netmgr_net_init(ssv_type_bool default_cfg, char hwmac[6])
{
	struct ip_addr ipaddr, netmask, gw;
    struct netif * pwlan = NULL;
    int ret = ERR_OK;

    /* when g_netmgr_config.ipaddr == 0, it will set default value */
    if (default_cfg || (g_netmgr_config.ipaddr == 0))
    {
        netmgr_cfg_default(&g_netmgr_config);
    }

    /* net if init */
    pwlan = netif_find(WLAN_IFNAME);
    if (pwlan)
    {
		#ifdef NETMGR_USE_NETIF_API
        netifapi_netif_remove(pwlan);
		#else
        netif_remove(pwlan);
		#endif
    }

    OS_MemCPY((void *)(wlan0.hwaddr), hwmac, 6);
    OS_MemCPY((void *)(wlan0.name),WLAN_IFNAME, 6);

    /* if sta mode and dhcpc enable, set ip is 0.0.0.0, otherwise is default ip */
    if (netmgr_wifi_check_sta() && s_dhcpc_enable)
    {
        ip_addr_set_zero(&ipaddr);
        ip_addr_set_zero(&netmask);
        ip_addr_set_zero(&gw);
    }
    else
    {
        ipaddr.addr = g_netmgr_config.ipaddr;
        netmask.addr = g_netmgr_config.netmask;
        gw.addr =  g_netmgr_config.gw;
    }

#ifdef NETMGR_USE_NETIF_API
    ret = netifapi_netif_add(&wlan0, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
    if (ret != ERR_OK)
    {
        LOG_PRINTF("netifapi_netif_add err = %d\r\n", ret);
    }

    ret = netifapi_netif_set_default(&wlan0);
    if (ret != ERR_OK)
    {
        LOG_PRINTF("netifapi_netif_set_default err = %d\r\n", ret);
    }
#else
    netif_add(&wlan0, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&wlan0);
#endif

    netmgr_netif_link_set(false);

    /* if ap mode and dhcpd enable, set ip is default ip and set netif up */
    if (netmgr_wifi_check_ap() && s_dhcpd_enable)
    {
        netmgr_netif_link_set(true);
        netmgr_netif_status_set(true);
        netmgr_dhcpd_start(true);
    }

    /* Register link change callback function */
    netif_set_status_callback(&wlan0, netif_status_change_cb);
    netif_set_link_callback(&wlan0, netif_link_change_cb);

    LOG_PRINTF("MAC[%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
        wlan0.hwaddr[0], wlan0.hwaddr[1], wlan0.hwaddr[2],
        wlan0.hwaddr[3], wlan0.hwaddr[4], wlan0.hwaddr[5]);
}

int netmgr_igmp_enable(ssv_type_bool on)
{
    int ret = 0;
#if LWIP_IGMP
    struct netif *netif = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_netif_igmp_enable %s\r\n",(on==true?"ON":"OFF")));

    netif = netif_find(WLAN_IFNAME);
    if (netif == NULL)
    {
        LOG_PRINTF("igmp_start error\r\n");
        return -1;
    }

    if (on)
    {
        netif->flags |= NETIF_FLAG_IGMP;
        /* start IGMP processing */
        ret = igmp_start(netif);
        if (ret != ERR_OK)
        {
            LOG_PRINTF("igmp_start error\r\n");
        }
        else
        {
            //LOG_PRINTF("igmp_start start\r\n");
        }
    }
    else
    {
        ret = igmp_stop(netif);
        if (ret != ERR_OK)
        {
            LOG_PRINTF("igmp_start error\r\n");
        }
        else
        {
            //LOG_PRINTF("igmp_start start\r\n");
        }

        netif->flags &= ~NETIF_FLAG_IGMP;
    }
#else
    LOG_PRINTF("LWIP_IGMP macro not open \r\n");
#endif

    return ret;
}
#endif //#if 0

void netmgr_netif_status_set(ssv_type_bool on,ssv_type_u8 vif_idx)
{
    LOG_DEBUGF(LOG_L4_NETMGR, ("L3 Link %s\r\n",(on==true?"ON":"OFF")));

    if (on)
        netdev_l3_if_up(g_netifdev[vif_idx].name);
    else
        netdev_l3_if_down(g_netifdev[vif_idx].name);
}

ssv_type_bool netmgr_check_netif_up(ssv_type_u8 vif_idx)
{
    int res;
    res = netdev_check_ifup(g_netifdev[vif_idx].name);

    if (res == NS_OK)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void netmgr_netif_link_set(ssv_type_bool on,ssv_type_u8 vif_idx)
{
    LOG_DEBUGF(LOG_L4_NETMGR, ("L2 Link %s\r\n",(on==true?"ON":"OFF")));

    if (on)
    {
        if(g_wifi_link_up_cb != NULL)
            g_wifi_link_up_cb(g_netifdev[vif_idx].name);
    }
    else
    {
        if(g_wifi_link_down_cb != NULL)
            g_wifi_link_down_cb(g_netifdev[vif_idx].name);
    }
}

ssv_type_bool netmgr_wifi_is_connected(ssv6xxx_hw_mode hmode)
{
    ssv_type_u8 vif_idx;
    switch (hmode)
    {
        case SSV6XXX_HWM_STA:
            if (netmgr_wifi_check_sta(&vif_idx))
            {
                if (netmgr_check_netif_up(vif_idx))
                {
                    return true;
                }
            }
            break;
        case SSV6XXX_HWM_AP:
            if (netmgr_wifi_check_ap(&vif_idx))
            {
                Ap_sta_status info;
                int ret = 0;
                ret = netmgr_wifi_info_get(&info,vif_idx);
                if ((ret == 0) && (info.vif_u[vif_idx].ap_status.stanum > 0))
                {
                    return true;
                }
            }
            break;
    }
    return false;
}

int netmgr_ipinfo_get(char *ifname, ipinfo *info)
{
    int ret = NS_OK;
    ssv_type_u32 ipaddr, gw, netmask;
    ret = netdev_getipv4info(ifname, &ipaddr, &gw, &netmask);

    if (NS_OK == ret)
    {
        info->ipv4 = ipaddr;
        info->netmask = gw;
        info->gateway = netmask;
        info->dns = info->gateway;

        return 0;
    }
    return -1;
}

int netmgr_hwmac_get(char *ifname, void *mac)
{
    return (netdev_getmacaddr(ifname, (ssv_type_u8 *)mac));
}

int netmgr_dhcpd_auto_set(ssv_type_u8 vif_idx)
{
    /* dhcps info auto set */
    if ((((g_netmgr_config[vif_idx].ipaddr >> 24) & 0xff) + 1) < 0xff)
    {
        g_netmgr_config[vif_idx].dhcps.start_ip = g_netmgr_config[vif_idx].ipaddr + (1 << 24);
    }
    else
    {
        g_netmgr_config[vif_idx].dhcps.start_ip = g_netmgr_config[vif_idx].ipaddr & 0x00ffffff + (1 << 24);
    }

    if ((((g_netmgr_config[vif_idx].ipaddr >> 24) & 0xff) + DEFAULT_DHCP_MAX_LEASES) < 0xff)
    {
        g_netmgr_config[vif_idx].dhcps.end_ip = g_netmgr_config[vif_idx].dhcps.start_ip + ((DEFAULT_DHCP_MAX_LEASES - 1) << 24);
    }
    else
    {
        g_netmgr_config[vif_idx].dhcps.end_ip = (g_netmgr_config[vif_idx].dhcps.start_ip & 0x00ffffff) + ((unsigned int)0xfe << 24);
    }

    g_netmgr_config[vif_idx].dhcps.max_leases = ((g_netmgr_config[vif_idx].dhcps.end_ip >> 24) & 0xff) - ((g_netmgr_config[vif_idx].dhcps.start_ip >> 24) & 0xff) + 1;

    g_netmgr_config[vif_idx].dhcps.gw = g_netmgr_config[vif_idx].ipaddr;

    g_netmgr_config[vif_idx].dhcps.dns = g_netmgr_config[vif_idx].dhcps.gw;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_dhcpd_auto_set: ipaddr=%08x start_ip=%08x end_ip=%08x\r\n",g_netmgr_config[vif_idx].ipaddr,\
        g_netmgr_config[vif_idx].dhcps.start_ip, g_netmgr_config[vif_idx].dhcps.end_ip));

    return 0;
}

int netmgr_ipinfo_set(ssv_type_u8 vif_idx, ipinfo *info, ssv_type_bool auto_dhcpd_info)
{
    int ret = NS_OK;
    //u8 i;
    char *ifname =if_name[vif_idx];
    ssv_type_u32 ip = info->ipv4;
    LOG_PRINTF("netmgr_ipinfo_set,ip=%x,if=%s,vif=%d\r\n",info->ipv4,ifname,vif_idx);
    LOG_PRINTF("netmgr_ipinfo_set_ip =|  %3d  |  %3d  |  %3d  |  %3d \r\n",
                      netstack_ip4_addr4_16((ssv_type_u32*)&ip),
                      netstack_ip4_addr3_16((ssv_type_u32*)&ip),
                      netstack_ip4_addr2_16((ssv_type_u32*)&ip),
                      netstack_ip4_addr1_16((ssv_type_u32*)&ip));
    ret = netdev_setipv4info(ifname, info->ipv4, info->gateway, info->netmask);

    //if (ret != NS_OK)
    //    return -1;

    //Update default device here, but better method is needed for multiple device
    //for(i=0;i<MAX_VIF_NUM;i++)
    {
        //if(ssv6xxx_memcmp(ifname, if_name[vif_idx], sizeof(WLAN_IFNAME)) == 0)
        {
            LOG_PRINTF("update default config\r\n");
            g_netifdev[vif_idx].ipaddr = g_netmgr_config[vif_idx].ipaddr = info->ipv4;
            g_netifdev[vif_idx].netmask = g_netmgr_config[vif_idx].netmask = info->netmask;
            g_netifdev[vif_idx].gw = g_netmgr_config[vif_idx].gw = info->gateway;
            //break;
        }
    }
    /* auto dhcpd set*/
    if (auto_dhcpd_info)
    {
        ssv_type_bool dhcpd_status = false;
        ssv_type_bool dhcpc_status = false;
        netmgr_dhcpd_auto_set(vif_idx);
        netmgr_dhcp_status_get(&dhcpd_status, &dhcpc_status,vif_idx);
        if (dhcpd_status)
        {
            netmgr_dhcpd_start(false,vif_idx);
            netmgr_dhcpd_start(true,vif_idx);
        }
    }

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_ipinfo_set\r\n"));
    return 0;
}

int netmgr_dhcps_info_set(dhcps_info *if_dhcps, ssv_type_u8 vif_idx)
{
    if (if_dhcps)
    {
        return netstack_dhcps_info_set(if_dhcps, &(g_netmgr_config[vif_idx].dhcps),vif_idx);
    }

    return 0;
}

static int netmgr_dhcpd_start(ssv_type_bool enable, ssv_type_u8 vif_idx)
{
    int ret = NS_OK;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_dhcpd_start %d vif=%d!!\r\n",enable,vif_idx));

    if (enable)
    {
        ret = netdev_setipv4info(g_netifdev[vif_idx].name,
                                 g_netifdev[vif_idx].ipaddr,
                                 g_netifdev[vif_idx].gw,
                                 g_netifdev[vif_idx].netmask);

        netmgr_netif_link_set(enable,vif_idx);
        netdev_l3_if_up(g_netifdev[vif_idx].name);
        netstack_dhcps_info_set(&(g_netmgr_config[vif_idx].dhcps),&(g_netmgr_config[vif_idx].dhcps),vif_idx);
        ret = netstack_udhcpd_start();
        if(ret)
        {
            LOG_PRINTF("netmgr start dhcpd fail\r\n");
            g_netmgr_config[vif_idx].s_dhcpd_status = false;
            return -1;
        }
        g_netmgr_config[vif_idx].s_dhcpd_status = true;
    }
    else
    {
        ret = netstack_udhcpd_stop();
        g_netmgr_config[vif_idx].s_dhcpd_status = false;
    }

    return ret;
}

static int netmgr_dhcpc_start(ssv_type_bool enable, ssv_type_u8 vif_idx)
{
    int ret = NS_OK;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_dhcpc_start %d !!\r\n",enable));

    if (enable)
    {
        ret = netdev_setipv4info(g_netifdev[vif_idx].name, 0, 0, 0);
        dhcpc_wrapper_set(g_netifdev[vif_idx].name, true);
        g_netmgr_config[vif_idx].s_dhcpc_status = true;
    }
    else
    {
        dhcpc_wrapper_set(g_netifdev[vif_idx].name, false);
        g_netifdev[vif_idx].ipaddr = g_netmgr_config[vif_idx].ipaddr;
        g_netifdev[vif_idx].netmask = g_netmgr_config[vif_idx].netmask;
        g_netifdev[vif_idx].gw =  g_netmgr_config[vif_idx].gw;
        ret = netdev_setipv4info(g_netifdev[vif_idx].name,
                                 g_netifdev[vif_idx].ipaddr ,
                                 g_netifdev[vif_idx].gw ,
                                 g_netifdev[vif_idx].netmask);
        g_netmgr_config[vif_idx].s_dhcpc_status = false;
    }

    return 0;
}


int netmgr_dhcpc_set(ssv_type_bool dhcpc_enable,ssv_type_u8 vif_idx)
{
    if (g_netmgr_config[vif_idx].s_dhcpc_status && !dhcpc_enable)
    {
        netmgr_dhcpc_start(false,vif_idx);
    }

    g_netmgr_config[vif_idx].s_dhcpc_enable = dhcpc_enable;

    return 0;
}

int netmgr_dhcpd_set(ssv_type_bool dhcpd_enable,ssv_type_u8 vif_idx)
{
    if (g_netmgr_config[vif_idx].s_dhcpd_status && !dhcpd_enable)
    {
        netmgr_dhcpd_start(false,vif_idx);
    }

    LOG_PRINTF("netmgr_dhcpd_set=%d,vif=%d\r\n",dhcpd_enable,vif_idx);
    g_netmgr_config[vif_idx].s_dhcpd_enable = dhcpd_enable;

    return 0;
}


int netmgr_dhcp_status_get(ssv_type_bool *dhcpd_status, ssv_type_bool *dhcpc_status, ssv_type_u8 vif_idx)
{
    if (!dhcpd_status || !dhcpc_status)
    {
        return -1;
    }

    *dhcpc_status = g_netmgr_config[vif_idx].s_dhcpc_status;
    *dhcpd_status = g_netmgr_config[vif_idx].s_dhcpd_status;

    return 0;
}

int netmgr_dhcp_getip_bymac(ssv_type_u8 *mac, ssv_type_u32 *ipaddr)
{
    dhcpdipmac *ipmac = NULL;
    int size_count = DEFAULT_DHCP_MAX_LEASES;
    int i;

    //LOG_PRINTF("netmgr_dhcd_getip_bymac MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
    //    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );

    ipmac = (dhcpdipmac *)OS_MemAlloc(DEFAULT_DHCP_MAX_LEASES * sizeof(dhcpdipmac));
    if (ipmac == NULL)
    {
        return -1;
    }
    OS_MemSET((void *)ipmac, 0, DEFAULT_DHCP_MAX_LEASES * sizeof(dhcpdipmac));

    if (netstack_dhcp_ipmac_get(ipmac, &size_count)){
    //    LOG_PRINTF("netmgr_dhcd_ipmac_get return failure \r\n");
        OS_MemFree(ipmac);
        return -1;
    }

    for (i=0; i<size_count; i++){
        if (!ssv6xxx_memcmp(ipmac[i].mac, mac, 6)){
            *ipaddr = ipmac[i].ip;
   //         LOG_PRINTF("netmgr_dhcd_getip_bymac shot ipaddr:0x%X \r\n", ntohl(*ipaddr));
            OS_MemFree(ipmac);
            return 0;
        }
    }

   // LOG_PRINTF("netmgr_dhcd_getip_bymac get ipaddr failure\r\n");
    OS_MemFree(ipmac);
    return -1;
}

int netmgr_send_arp_unicast (ssv_type_u8 *dst_mac)
{
    netstack_ip_addr_t ipaddr;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_send_arp_unicast %02X:%02X:%02X:%02X:%02X:%02X !!\r\n",dst_mac[0],
        dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5]));

    if (!netmgr_dhcp_getip_bymac(dst_mac, (ssv_type_u32 *)&ipaddr))
    {
        return netstack_etharp_unicast(dst_mac, &ipaddr);
    }
    else if(netstack_find_ip_in_arp_table(dst_mac,&ipaddr))
    {
        return netstack_etharp_unicast(dst_mac, &ipaddr);
    }

    return -1;
}

int netmgr_wifi_mode_get(wifi_mode *mode, ssv_type_bool *status, ssv_type_u8 vif_idx)
{
    #if NET_MGR_DEBUG
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
    #endif
    Ap_sta_status *info = NULL;

    if (!mode || !status)
    {
        return -1;
    }

    if(vif_idx >= MAX_VIF_NUM)
        return -1;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if(NULL==info)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return -1;
    }
    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    *mode = info->vif_operate[vif_idx];
    *status = info->status ? true : false;

    #if NET_MGR_DEBUG
    if(info->status)
        LOG_PRINTF("status:ON\r\n");
    else
        LOG_PRINTF("status:OFF\r\n");
    if(SSV6XXX_HWM_STA==info->vif_operate[vif_idx])
    {
        LOG_PRINTF("Mode:Station\r\n");
    }
    else
    {
        LOG_PRINTF("Mode:AP\r\n");
        OS_MemCPY((void*)ssid_buf,(void*)info->vif_u[vif_idx].ap_status.ssid.ssid,info->vif_u[vif_idx].ap_status.ssid.ssid_len);
        LOG_PRINTF("SSID:%s\r\n",ssid_buf);
        LOG_PRINTF("Station number:%d\r\n",info->vif_u[vif_idx].ap_status.stanum);
    }

    LOG_PRINTF("Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        info->vif_u[vif_idx].ap_status.selfmac[0],
        info->vif_u[vif_idx].ap_status.selfmac[1],
        info->vif_u[vif_idx].ap_status.selfmac[2],
        info->vif_u[vif_idx].ap_status.selfmac[3],
        info->vif_u[vif_idx].ap_status.selfmac[4],
        info->vif_u[vif_idx].ap_status.selfmac[5]);

    #endif

    OS_MemFree(info);

    return 0;
}

int netmgr_wifi_info_get(Ap_sta_status *info, ssv_type_u8 vif_idx)
{
#if NET_MGR_DEBUG
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
#endif

    if (info == NULL)
    {
        return -1;
    }
    if(vif_idx >= MAX_VIF_NUM)
        return -1;

    OS_MemSET(info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

#if NET_MGR_DEBUG
    if(info->status)
        LOG_PRINTF("status:ON\r\n");
    else
        LOG_PRINTF("status:OFF\r\n");
    if(SSV6XXX_HWM_STA==info->vif_operate[vif_idx])
    {
        LOG_PRINTF("Mode:Station\r\n");
    }
    else
    {
        LOG_PRINTF("Mode:AP\r\n");
        OS_MemCPY((void*)ssid_buf,(void*)info->vif_u[vif_idx].ap_status.ssid.ssid,info->vif_u[vif_idx].ap_status.ssid.ssid_len);
        LOG_PRINTF("SSID:%s\r\n",ssid_buf);
        LOG_PRINTF("Station number:%d\r\n",info->vif_u[vif_idx].ap_status.stanum);
    }

    LOG_PRINTF("Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
        info->vif_u[vif_idx].ap_status.selfmac[0],
        info->vif_u[vif_idx].ap_status.selfmac[1],
        info->vif_u[vif_idx].ap_status.selfmac[2],
        info->vif_u[vif_idx].ap_status.selfmac[3],
        info->vif_u[vif_idx].ap_status.selfmac[4],
        info->vif_u[vif_idx].ap_status.selfmac[5]);

#endif

    return 0;
}

ssv_type_bool netmgr_wifi_check_mac(unsigned char * mac)
{
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    unsigned int i = 0,j;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    for(j=0;j<MAX_VIF_NUM;j++)
    {
        if (info->vif_operate[j] == SSV6XXX_HWM_AP)
        {
             bRet = false;

             for(i = 0; i < info->vif_u[j].ap_status.stanum; i++)
             {
                if (ssv6xxx_memcmp(mac, info->vif_u[j].ap_status.stainfo[i].Mac, 6) == 0)
                {
                    bRet = true;
                    break;
                }
                /*
                 LOG_PRINTF("station Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                     info->u.ap.stainfo[statemp].Mac[0],
                     info->u.ap.stainfo[statemp].Mac[1],
                     info->u.ap.stainfo[statemp].Mac[2],
                     info->u.ap.stainfo[statemp].Mac[3],
                     info->u.ap.stainfo[statemp].Mac[4],
                     info->u.ap.stainfo[statemp].Mac[5]);
                */
             }
        }
    }

    OS_MemFree(info);

    return bRet;
}

ssv_type_u8 netmgr_wifi_get_ava_vif(void)
{
    Ap_sta_status *info = NULL;
    // ssv_type_bool bRet = false;
    ssv_type_u8 i;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if(info->vif_operate[i]==SSV6XXX_HWM_INVALID)
        {
            LOG_PRINTF("Netmgr get ava vif=%d\r\n",i);
            break;
        }
    }
    return i;
}

ssv_type_bool netmgr_wifi_check_sconfig(ssv_type_u8* vif_idx)
{
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    int i;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if ((info->vif_operate[i] == SSV6XXX_HWM_SCONFIG) && (info->status))
        {
             bRet = true;
             //LOG_PRINTF("netmgr check sconfig vif=%d\r\n",i);
             break;
        }
    }

    if(vif_idx)
        *vif_idx = i;
    OS_MemFree(info);

    return bRet;
}
//Seek for a STA VIF
ssv_type_bool netmgr_wifi_check_sta(ssv_type_u8* vif_idx)
{
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    ssv_type_u8 i;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if ((info->vif_operate[i]== SSV6XXX_HWM_STA) && (info->status))
        {
            bRet = true;
            //LOG_PRINTF("netmgr check sta vif=%d\r\n",i);
            break;
        }
    }

    if(vif_idx)
        *vif_idx = i;
    OS_MemFree(info);

    return bRet;
}

ssv_type_bool netmgr_wifi_check_ap(ssv_type_u8* vif_idex)
{
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    int i=0;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if ((info->vif_operate[i] == SSV6XXX_HWM_AP) && (info->status))
        {
             bRet = true;
             LOG_PRINTF("netmgr check ap vif=%d\r\n",i);
             break;
        }
    }
    *vif_idex = i;
    OS_MemFree(info);

    return bRet;
}

ssv6xxx_hw_mode netmgr_wifi_get_vif_mode(ssv_type_u8 vif_idex)
{
    Ap_sta_status *info = NULL;
    //ssv_type_bool bRet = false;
    //int i=0;
    ssv6xxx_hw_mode hmode;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);
    hmode = info->vif_operate[vif_idex];
    OS_MemFree(info);
    return hmode;
}

ssv_type_bool netmgr_wifi_check_sta_connected(ssv_type_u8 vif_idx)
{
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    //int i;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);

    SSV_ASSERT((vif_idx)<MAX_VIF_NUM);

    //for(i=0;i<MAX_VIF_NUM;i++)
    {
        if ((info->vif_operate[vif_idx] == SSV6XXX_HWM_STA) && (info->status))
        {
            if(info->vif_u[vif_idx].station_status.apinfo.status == DISCONNECT)
            {
                bRet = false;
                //*vif_idx = i;
                //break;
            }
            else if(info->vif_u[vif_idx].station_status.apinfo.status == CONNECT)
            {
                bRet = true;
                //*vif_idx = i;
            }
        }
    }

    OS_MemFree(info);

    return bRet;
}

#if(ENABLE_SMART_CONFIG==1)

static int _netmgr_wifi_sconfig_async(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask)
{
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_sconfig_async %x \r\n",channel_mask));

    msg_evt = msg_evt_alloc();

    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s:msg evt alloc fail",__FUNCTION__);
        return -1;
    }

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_SCONFIG_REQ;
    msg_evt->MsgData1 = (ssv_type_u32)(channel_mask);
    msg_evt->MsgData2 = (ssv_type_u32)(channel_5g_mask);
    msg_evt->MsgData3 = 0;

    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        LOG_PRINTF("sconfig_async retry\r\n");
        OS_MsDelay(10);
    }
    #else

    netmgr_wifi_sconfig(channel_mask);
    #endif


    return 0;
}

int netmgr_wifi_sconfig_async(ssv_type_u16 channel_mask)
{
    return _netmgr_wifi_sconfig_async(channel_mask, 0);
}

int netmgr_wifi_sconfig_ex_async(ssv_type_u16 channel_mask,ssv_type_u32 channel_5g_mask)
{
    return _netmgr_wifi_sconfig_async(channel_mask, channel_5g_mask);
}

#endif
static int _netmgr_wifi_scan_async(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask, char *ssids[], int ssids_count)
{
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_scan_async %x \r\n",channel_mask));

    msg_evt = msg_evt_alloc();

    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s: msg alloc fail\r\n",__FUNCTION__);
        return -1;
    }

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_SCAN_REQ;
    msg_evt->MsgData1 = (ssv_type_u32)(channel_mask);
    msg_evt->MsgData2 = (ssv_type_u32)(channel_5g_mask);
    msg_evt->MsgData3 = 0;

    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("scan_async retry");
    }
    #else
    netmgr_wifi_scan_ex(channel_mask, channel_5g_mask, 0, 0);
    #endif
    return 0;
}

int netmgr_wifi_scan_async(ssv_type_u16 channel_mask, char *ssids[], int ssids_count)
{
    return _netmgr_wifi_scan_async(channel_mask, 0, ssids, ssids_count);
}

int netmgr_wifi_scan_ex_async(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask, char *ssids[], int ssids_count)
{
    return _netmgr_wifi_scan_async(channel_mask, channel_5g_mask, ssids, ssids_count);
}

int netmgr_wifi_join_async(wifi_sta_join_cfg *join_cfg)
{
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
    wifi_sta_join_cfg *join_cfg_msg = NULL;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join_async \r\n"));

    //if (netmgr_wifi_check_sta_connected(&vif_idx))
    //{
    //    LOG_PRINTF("netmgr_wifi_join_async: mode error.\r\n");
    //    return -1;
    //}

    msg_evt = msg_evt_alloc();
    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s: msg alloc fail\r\n",__FUNCTION__);
        return -1;
    }

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_JOIN_REQ;

    if (join_cfg)
    {
        join_cfg_msg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
        if(join_cfg_msg==NULL)
        {
            msg_evt_free(msg_evt);
            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )join_cfg_msg, (void * )join_cfg, sizeof(wifi_sta_join_cfg));
    }

    msg_evt->MsgData1 = (ssv_type_u32)(join_cfg_msg);
    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("join_async retry");
    }
    #else
    netmgr_wifi_join(join_cfg);
    #endif
    return 0;
}

int netmgr_wifi_join_other_async(wifi_sta_join_cfg *join_cfg)
{
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
    wifi_sta_join_cfg *join_cfg_msg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join_other_async \r\n"));

    msg_evt = msg_evt_alloc();
    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s: msg alloc fail\r\n",__FUNCTION__);
        return -1;
    }

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_JOIN_OTHER_REQ;

    if (join_cfg)
    {
        join_cfg_msg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
        if(NULL==join_cfg_msg)
        {
            msg_evt_free(msg_evt);
            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )join_cfg_msg, (void * )join_cfg, sizeof(wifi_sta_join_cfg));
    }

    msg_evt->MsgData1 = (ssv_type_u32)(join_cfg_msg);
    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("join_other_async retry");
    }
    #else

    netmgr_wifi_join_other(join_cfg);
    #endif
    return 0;
}

int netmgr_wifi_leave_async(ssv_type_u8 vif_idx)
{
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_leave_async \r\n"));

    msg_evt = msg_evt_alloc();

    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
        return -1;
    }

    #ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
    #endif

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_LEAVE_REQ;
    msg_evt->MsgData1 = vif_idx;
    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("leave_async retry");
    }
    #else
    #ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
    #endif

    netmgr_wifi_leave(vif_idx);
    #endif
    return 0;
}

int netmgr_wifi_vif_off_async(ssv_type_u8 vif_idx)
{
    Ap_sta_status *info = NULL;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return -1;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    ssv6xxx_wifi_status(info);
    switch(info->vif_operate[vif_idx])
    {
        case SSV6XXX_HWM_STA:
            if(info->vif_u[vif_idx].station_status.apinfo.status > DISCONNECT)
            {
                netmgr_wifi_leave_async(vif_idx);
                OS_MsDelay(500);
            }
            break;
        case SSV6XXX_HWM_AP:
            //netmgr_wifi_ap_off(vif_idx);
            break;
        case SSV6XXX_HWM_SCONFIG:
            //netmgr_wifi_sconfig_off(vif_idx);
            break;
    }
    OS_MemFree(info);


#if !NET_MGR_NO_SYS
    {
        MsgEvent *msg_evt = NULL;

        LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_vif_off_async \r\n"));

        msg_evt = msg_evt_alloc();

        if(NULL==msg_evt)
        {
            LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
            return -1;
        }
        msg_evt->MsgType = MEVT_NET_MGR_EVENT;
        msg_evt->MsgData = MSG_VIF_OFF_REQ;
        msg_evt->MsgData1 = vif_idx;
        while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
        {
            OS_MsDelay(10);
            LOG_PRINTF("off_async retry");
        }
    }
#else

    netmgr_wifi_vif_off(vif_idx);
#endif
    return 0;
}

int netmgr_wifi_vif_set_ip_async(ssv_type_u8 vif_idx, ipinfo* ip)
{
//#if !NET_MGR_NO_SYS
    {
        MsgEvent *msg_evt = NULL;
        ipinfo* info=NULL;

        LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_vif_set_ip_async \r\n"));
        info = (ipinfo *)OS_MemAlloc(sizeof(ipinfo));
        if(info)
        {
            *info = *ip;
            msg_evt = msg_evt_alloc();

            if(NULL==msg_evt)
            {
                LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                return -1;
            }
            msg_evt->MsgType = MEVT_NET_MGR_EVENT;
            msg_evt->MsgData = MSG_AP_IP_CHANGE;
            msg_evt->MsgData1 = vif_idx;
            msg_evt->MsgData2 = (ssv_type_u32)info;
            while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
            {
                OS_MsDelay(10);
                LOG_PRINTF("setip_async retry");
            }
        }
        else
        {
            return -1;
        }
    }
//#else

//#endif
    return 0;
}

int netmgr_wifi_control_async(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_cfg *sta_cfg)
{
    int ret = 0;
    ssv_type_u8 vif_idx;
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
    ssv_wifi_ap_cfg *ap_cfg_msg = NULL;
    wifi_sta_cfg *sta_cfg_msg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_control_async \r\n"));

    g_wifi_is_joining_b = false;

    msg_evt = msg_evt_alloc();
    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
        return -1;
    }

    g_sconfig_running = FALSE;

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_CONTROL_REQ;

#if (AP_MODE_ENABLE == 1)
    if (ap_cfg)
    {
        netmgr_wifi_check_ap(&vif_idx);

        if((vif_idx != ap_cfg->vif_idx) && ((vif_idx != MAX_VIF_NUM)))
        {
            LOG_PRINTF("Not allow AP+AP\r\n");
            return -1;
        }

        if(ap_cfg->channel!=EN_CHANNEL_AUTO_SELECT)
        {
            if(FALSE==ssv6xxx_wifi_is_available_channel(SSV6XXX_HWM_AP,ap_cfg->channel))
            {
                LOG_PRINTF("%d not available_channel\r\n",ap_cfg->channel);
                return -1;
            }
        }
        ap_cfg_msg = (ssv_wifi_ap_cfg *)OS_MemAlloc(sizeof(ssv_wifi_ap_cfg));
        if(NULL==ap_cfg_msg)
        {
            msg_evt_free(msg_evt);
            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )ap_cfg_msg, (void * )ap_cfg, sizeof(ssv_wifi_ap_cfg));
        vif_idx = ap_cfg->vif_idx;
    }
#endif
    if (sta_cfg)
    {
        sta_cfg_msg = (wifi_sta_cfg *)OS_MemAlloc(sizeof(wifi_sta_cfg));
        if(NULL==sta_cfg_msg)
        {
            msg_evt_free(msg_evt);
            OS_MemFree(ap_cfg_msg);
            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )sta_cfg_msg, (void * )sta_cfg, sizeof(wifi_sta_cfg));
        vif_idx = sta_cfg->vif_idx;
    }

#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif
    msg_evt->MsgData1 = (ssv_type_u32)(mode);
    msg_evt->MsgData2 = (ssv_type_u32)(ap_cfg_msg);
    msg_evt->MsgData3 = (ssv_type_u32)(sta_cfg_msg);
    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("control_async retry");
    }
#else //#if !NET_MGR_NO_SYS

#if (AP_MODE_ENABLE == 1)
    if(ap_cfg)
    {
       LOG_PRINTF("use vif=%d for AP\r\n",ap_cfg->vif_idx);
       if(ap_cfg->channel!=EN_CHANNEL_AUTO_SELECT)
       {
           if(FALSE==ssv6xxx_wifi_is_available_channel(SSV6XXX_HWM_AP,ap_cfg->channel))
           {
               LOG_PRINTF("%d not available_channel\r\n",ap_cfg->channel);
               return -1;
           }
       }
       vif_idx = ap_cfg->vif_idx;
    }
#endif

    if(sta_cfg)
    {
        LOG_PRINTF("use vif=%d for STA\r\n",sta_cfg->vif_idx);
        vif_idx = sta_cfg->vif_idx;
    }
#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif
    ret = netmgr_wifi_control(mode, ap_cfg, sta_cfg);
#endif //#if !NET_MGR_NO_SYS

    return ret;
}

int netmgr_wifi_switch_async(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_join_cfg *join_cfg)
{
    int ret = 0;
    ssv_type_u8 vif_idx;
    #if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
    ssv_wifi_ap_cfg *ap_cfg_msg = NULL;
    wifi_sta_join_cfg *join_cfg_msg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_switch_async \r\n"));

    g_wifi_is_joining_b = false;

    msg_evt = msg_evt_alloc();
    if(NULL==msg_evt)
    {
        LOG_PRINTF("%s: msg allocate fail\r\n",__FUNCTION__);
        return -1;
    }

    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
    msg_evt->MsgData = MSG_SWITCH_REQ;

#if (AP_MODE_ENABLE == 1)
    if (ap_cfg)
    {
        ap_cfg_msg = (ssv_wifi_ap_cfg *)OS_MemAlloc(sizeof(ssv_wifi_ap_cfg));
        if(NULL==ap_cfg_msg)
        {
            msg_evt_free(msg_evt);
            LOG_PRINTF("%s:malloc fail(%d)\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )ap_cfg_msg, (void * )ap_cfg, sizeof(ssv_wifi_ap_cfg));
        vif_idx= ap_cfg->vif_idx;
    }
#endif

    if (join_cfg)
    {
        join_cfg_msg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
        if(NULL==join_cfg_msg)
        {
            msg_evt_free(msg_evt);
            OS_MemFree(ap_cfg_msg);
            LOG_PRINTF("%s:malloc fail(%d)\r\n",__FUNCTION__,__LINE__);
            return -1;
        }
        OS_MemCPY((void * )join_cfg_msg, (void * )join_cfg, sizeof(wifi_sta_join_cfg));
        vif_idx= join_cfg->vif_idx;
    }

#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif
    msg_evt->MsgData1 = (ssv_type_u32)(mode);
    msg_evt->MsgData2 = (ssv_type_u32)(ap_cfg_msg);
    msg_evt->MsgData3 = (ssv_type_u32)(join_cfg_msg);
    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
    {
        OS_MsDelay(10);
        LOG_PRINTF("switch_async retry");
    }
    #else

    if (ap_cfg)
    {
        vif_idx= ap_cfg->vif_idx;
    }
    if (join_cfg)
    {
        vif_idx= join_cfg->vif_idx;
    }
#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif
    ret = netmgr_wifi_switch(mode, ap_cfg, join_cfg);
    #endif
    return ret;
}


void netmgr_wifi_station_off(ssv_type_u8 vif_idx)
{
    wifi_mode mode;
    wifi_sta_cfg *sta_cfg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_station_off \r\n"));

    sta_cfg = (wifi_sta_cfg *)OS_MemAlloc(sizeof(wifi_sta_cfg));
    if (sta_cfg == NULL)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    OS_MemSET((void *)sta_cfg, 0, sizeof(wifi_sta_cfg));

    mode = SSV6XXX_HWM_STA;
    sta_cfg->status = false;
    sta_cfg->vif_idx = vif_idx;

    netmgr_wifi_control(mode, NULL, sta_cfg);

    OS_MemFree(sta_cfg);
}

#if(ENABLE_SMART_CONFIG==1)
void netmgr_wifi_sconfig_off(ssv_type_u8 vif_idx)
{
    wifi_mode mode;
    wifi_sta_cfg *sta_cfg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_sconfig_off \r\n"));

    sta_cfg = (wifi_sta_cfg *)OS_MemAlloc(sizeof(wifi_sta_cfg));
    if (sta_cfg == NULL)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    OS_MemSET((void *)sta_cfg, 0, sizeof(wifi_sta_cfg));
    if((ssv6xxx_user_sconfig_op.UserSconfigDeinit!=NULL)&&(g_sconfig_user_mode==TRUE)){
        ssv6xxx_user_sconfig_op.UserSconfigDeinit();
    }
    g_sconfig_running=FALSE;

    mode = SSV6XXX_HWM_SCONFIG;
    sta_cfg->status = false;
    sta_cfg->vif_idx = vif_idx;
    netmgr_wifi_control(mode, NULL, sta_cfg);

    OS_MemFree(sta_cfg);
}
#endif


void netmgr_wifi_ap_off(ssv_type_u8 vif_idx)
{
    wifi_mode mode;
    ssv_wifi_ap_cfg *ap_cfg = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_ap_off \r\n"));

    ap_cfg = (ssv_wifi_ap_cfg *)OS_MemAlloc(sizeof(ssv_wifi_ap_cfg));
    if (ap_cfg == NULL)
    {
        return;
    }

    OS_MemSET((void *)ap_cfg, 0, sizeof(ssv_wifi_ap_cfg));

    mode = SSV6XXX_HWM_AP;
    ap_cfg->status = false;
    ap_cfg->vif_idx = vif_idx;
    netmgr_wifi_control(mode, ap_cfg, NULL);

    OS_MemFree(ap_cfg);
}

void netmgr_wifi_vif_off(ssv_type_u8 vif_idx)
{
    Ap_sta_status *info = NULL;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    netmgr_netif_link_set(LINK_DOWN,vif_idx);
    ssv6xxx_wifi_status(info);
    switch(info->vif_operate[vif_idx])
    {
        case SSV6XXX_HWM_STA:
            netmgr_wifi_station_off(vif_idx);
            break;
        case SSV6XXX_HWM_AP:
            netmgr_wifi_ap_off(vif_idx);
            break;

#if(ENABLE_SMART_CONFIG==1)
        case SSV6XXX_HWM_SCONFIG:
            netmgr_wifi_sconfig_off(vif_idx);
            break;
#endif
    }

    OS_MemFree(info);
}

#if(ENABLE_SMART_CONFIG==1)

int netmgr_wifi_sconfig_done(ssv_type_u8 *resp_data, ssv_type_u32 len, ssv_type_bool IsUDP,ssv_type_u32 port)
{
    if(IsUDP==FALSE)
    {
        return -1;
    }

    LOG_PRINTF("IP address is ready!!\r\n");
    return (netstack_udp_send(resp_data, len, 0, ((ssv_type_u16)port-1), 0xffffffff, port, 50));
}

static int _netmgr_wifi_sconfig(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask)
{
    struct cfg_sconfig_request *SconfigReq = NULL;
    ssv_type_u8 vif_idx;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_sconfig \r\n"));

    if (!netmgr_wifi_check_sconfig(&vif_idx))
    {
        LOG_PRINTF("mode error.\r\n");
        return -1;
    }

#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif

    if(g_sconfig_running==TRUE){
        LOG_PRINTF("SmartConfig is running. Please restart the SCONFIG mode\r\n");
        return -1;
    }
    g_sconfig_running=TRUE;

    if(g_sconfig_solution==WECHAT_AIRKISS_IN_FW){
        g_sconfig_user_mode=FALSE;
    }
    else{
        g_sconfig_user_mode=TRUE;
    }

    //Here, if channel_mask is 0, it means that user prefer to use the default mask
    #if 0
    if(channel_mask==0){
        channel_mask=g_sta_channel_mask;
    }

    channel_mask&=~(0x8001); //unmask ch0 and ch15;
    #endif
    //Here, if channel_mask is 0, it means we don't need to scan any channel
    #if 0
    if(channel_mask==0){
        g_sconfig_running=FALSE;
        LOG_PRINTF("channel_mask is zero\r\n");
        return 0;
    }
    #endif
    ssv6xxx_wifi_align_available_channel_mask(SSV6XXX_HWM_SCONFIG, &channel_mask,&channel_5g_mask);

    if(g_sconfig_user_mode==TRUE)
    {
        ssv6xxx_promiscuous_enable();
        if(ssv6xxx_user_sconfig_op.UserSconfigInit!=NULL)
        {
            ssv6xxx_user_sconfig_op.UserSconfigInit();
        }
    }
    else
    {
        ssv6xxx_promiscuous_disable();
    }

    SconfigReq = (void *)OS_MemAlloc(sizeof(*SconfigReq));
    if (!SconfigReq)
    {
        g_sconfig_running=FALSE;
        return -1;
    }
    SconfigReq->channel_mask = channel_mask;
    SconfigReq->channel_5g_mask = channel_5g_mask;
    SconfigReq->dwell_time = 10;
    SconfigReq->user_mode=g_sconfig_user_mode;

    if (ssv6xxx_wifi_sconfig(SconfigReq) < 0)
    {
       	LOG_PRINTF("Command failed !!\r\n");
        OS_MemFree(SconfigReq);
        g_sconfig_running=FALSE;
        return -1;
    }

    OS_MemFree(SconfigReq);

    return 0;
}

int netmgr_wifi_sconfig_ex(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask)
{
    if(channel_mask==SCAN_ALL_2G_CHANNEL){
        channel_mask=g_sta_channel_mask;
    }

    if(channel_5g_mask==SCAN_ALL_5G_CHANNEL){
        channel_5g_mask=g_sta_5g_channel_mask;
    }

    return _netmgr_wifi_sconfig(channel_mask, channel_5g_mask);
}

int netmgr_wifi_sconfig(ssv_type_u16 channel_mask)
{
    if(channel_mask==SCAN_ALL_2G_CHANNEL){
        channel_mask=g_sta_channel_mask;
    }

    return _netmgr_wifi_sconfig(channel_mask,0);
}
#endif


extern ssv_type_u16 g_sta_scan_ch_interval;
static int _netmgr_wifi_scan(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask, char *ssids[], int ssids_count)
{

    struct cfg_scan_request *ScanReq = NULL;
    int                      i = 0;
    ssv_type_u8                      vif_idx;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_scan \r\n"));

    //Find a sta vif
    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if (netmgr_wifi_get_vif_mode(i)==SSV6XXX_HWM_STA)
        {
            vif_idx = i;
            break;
        }
    }
    if(vif_idx >= MAX_VIF_NUM)
    {
        LOG_PRINTF("mode error.\r\n");
        return -1;
    }
    LOG_DEBUGF(LOG_L4_NETMGR,("scan use sta vif =%d\r\n",vif_idx));

    //Here, if channel_mask is 0, it means that user prefer to use the default mask
    #if 0
    if(channel_mask==0){
        channel_mask=g_sta_channel_mask;
    }

    if(channel_5g_mask==0){
        channel_5g_mask=g_sta_5g_channel_mask;
    }
    #endif

    //channel_mask&=~(0x8001); //unmask ch0 and ch15;

    //Here, if channel_mask is 0, it means we don't need to scan any channel

    #if 0
    if(channel_mask==0){
        LOG_PRINTF("channel_mask is zero\r\n");
        return 0;
    }
    #endif
    ssv6xxx_wifi_align_available_channel_mask(SSV6XXX_HWM_STA, &channel_mask,&channel_5g_mask);
    ScanReq = (void *)OS_MemAlloc(sizeof(*ScanReq) + ssids_count*sizeof(struct cfg_80211_ssid));
    if (!ScanReq)
    {
        return -1;
    }
    ScanReq->is_active      = true;
    ScanReq->n_ssids        = ssids_count;
    ScanReq->channel_mask   = channel_mask;
    ScanReq->vif_idx        = vif_idx;
    if(TRUE==ssv6xxx_wifi_support_5g_band())
    {
        ScanReq->channel_5g_mask   = channel_5g_mask;
    }
    else
    {
        ScanReq->channel_5g_mask   = 0;
    }
    ScanReq->dwell_time = g_sta_scan_ch_interval/10;

    for (i = 0; i < ssids_count; i++)
    {
        OS_MemCPY((void*)(ScanReq->ssids[i].ssid), (void*)ssids[i], sizeof(struct cfg_80211_ssid));
    }

    if (ssv6xxx_wifi_scan(ScanReq) < 0)
    {
       	LOG_PRINTF("Command failed !!\r\n");
        OS_MemFree(ScanReq);
        return -1;
    }

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_scan done g_switch_join_cfg_b=%d\r\n", g_switch_join_cfg_b));

    OS_MemFree(ScanReq);

    return 0;
}

int netmgr_wifi_scan_ex(ssv_type_u16 channel_mask, ssv_type_u32 channel_5g_mask, char *ssids[], int ssids_count)
{
    if(channel_mask==SCAN_ALL_2G_CHANNEL){
        channel_mask=g_sta_channel_mask;
    }

    if(channel_5g_mask==SCAN_ALL_5G_CHANNEL){
        channel_5g_mask=g_sta_5g_channel_mask;
    }

    return _netmgr_wifi_scan(channel_mask, channel_5g_mask, ssids, ssids_count);
}

int netmgr_wifi_scan(ssv_type_u16 channel_mask, char *ssids[], int ssids_count)
{
    if(channel_mask==SCAN_ALL_2G_CHANNEL){
        channel_mask=g_sta_channel_mask;
    }

    return _netmgr_wifi_scan(channel_mask, 0, ssids, ssids_count);
}

char CharToHex(char bChar)
{
    if((bChar>=0x30)&&(bChar<=0x39))
	{
        bChar -= 0x30;
	}
    else if((bChar>=0x41)&&(bChar<=0x46))
	{
        bChar -= 0x37;
	}
    else if((bChar>=0x61)&&(bChar<=0x66))
	{
        bChar -= 0x57;
	}
    else
	{
        bChar = 0xff;
	}
    return bChar;
}

int netmgr_wifi_join(wifi_sta_join_cfg *join_cfg)
{
    ssv_type_s32    size = 0;
    struct ssv6xxx_ieee80211_bss       *ap_info_bss = NULL;
    struct cfg_join_request *JoinReq = NULL;
    //ssv_type_u32 channel = 0;
    wifi_sec_type    sec_type = WIFI_SEC_NONE;
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};

    ssv_type_u8 tmp, vif_idx=MAX_VIF_NUM;
	ssv_type_u8 i = 0;
	int outi = 0;
	int length = ssv6xxx_strlen((char *)join_cfg->password);
	char output[MAX_PASSWD_LEN+1];


    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join \r\n"));

    if (g_wifi_is_joining_b)
    {
        return -1;
    }

    //Find a sta vif and non-connected
    if(join_cfg->vif_idx >= MAX_VIF_NUM)
    {
        for(i=0;i<MAX_VIF_NUM;i++)
        {
            if (netmgr_wifi_get_vif_mode(i)==SSV6XXX_HWM_STA)
            {
                vif_idx = i;
                if(!netmgr_wifi_check_sta_connected(vif_idx))
                {
                    break;
                }
            }
        }
    }
    else
    {
        if (netmgr_wifi_get_vif_mode(join_cfg->vif_idx)==SSV6XXX_HWM_STA)
            vif_idx = join_cfg->vif_idx; //Assign by application
        else
            vif_idx = MAX_VIF_NUM;
    }

    if(vif_idx >= MAX_VIF_NUM)
    {
        LOG_PRINTF("join mode error.\r\n");
        return -1;
    }
    LOG_DEBUGF(LOG_L4_NETMGR,("use sta vif =%d\r\n",vif_idx));
#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif
    if(netmgr_wifi_check_sta_connected(vif_idx))
    {
        int time = 3;
        LOG_PRINTF("leave old ap,vif_idx=%d\r\n",vif_idx);
        netmgr_wifi_leave(vif_idx);
        while (netmgr_wifi_check_sta_connected(vif_idx) && (time > 0))
        {
            time--;
            OS_MsDelay(1000);
        }

        if (netmgr_wifi_check_sta_connected(vif_idx))
        {
            LOG_PRINTF("leave old ap timeout\r\n");
            return -1;
        }
        else
        {
            LOG_PRINTF("leave old ap success\r\n");
        }
    }

    if ((join_cfg->ssid.ssid_len == 0) || (ssv6xxx_strlen((char *)join_cfg->password) > MAX_PASSWD_LEN))
    {
        LOG_PRINTF("netmgr_wifi_join parameter error.\r\n");
        return -1;
    }

    //ap_info_bss = ssv6xxx_wifi_find_ap_ssid(&join_cfg->ssid);
    ap_info_bss = ssv6xxx_wifi_find_ap_ssid_best(&join_cfg->ssid);

    if (ap_info_bss == NULL)
    {
        OS_MemCPY((void*)ssid_buf,(void*)join_cfg->ssid.ssid,join_cfg->ssid.ssid_len);
        LOG_PRINTF("No AP \"%s\" was found.\r\n", ssid_buf);
        return -1;
    }

    if((!ssv6xxx_wifi_support_5g_band())&&IS_5G_BAND(ap_info_bss->channel_id))
    {
        LOG_PRINTF("Don't support 5G AP now\r\n");
        return -1;
    }

    ssv6xxx_wifi_set_channel(vif_idx, ap_info_bss->channel_id,SSV6XXX_HWM_STA);
    size = sizeof(struct cfg_join_request) + sizeof(struct ssv6xxx_ieee80211_bss);
    JoinReq = (struct cfg_join_request *)OS_MemAlloc(size);
    if(NULL==JoinReq)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return -1;
    }
    OS_MemSET((void *)JoinReq, 0, size);
    JoinReq->vif_idx = vif_idx;

    if (ap_info_bss->capab_info&BIT(4))
    {
        if (ap_info_bss->proto&WPA_PROTO_WPA)
        {
            sec_type = WIFI_SEC_WPA_PSK;

        }
        else if (ap_info_bss->proto&WPA_PROTO_RSN)
        {
            sec_type = WIFI_SEC_WPA2_PSK;
        }
        else
        {
            sec_type = WIFI_SEC_WEP;
        }
    }
    else
    {
        sec_type = WIFI_SEC_NONE;
    }

    JoinReq->pmk_valid = join_cfg->pmk_valid;
    memcpy(JoinReq->pmk,join_cfg->pmk,32);

    if (sec_type == WIFI_SEC_NONE)
    {
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_NONE;
    }
    else if (sec_type == WIFI_SEC_WEP)
    {
        #if 1  // defined(SEC_USE_WEP40_OPEN) || defined(SEC_USE_WEP104_OPEN)
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        #else
        JoinReq->auth_alg = WPA_AUTH_ALG_SHARED;
        #endif
        JoinReq->wep_keyidx = 0;
        if (ssv6xxx_strlen((char *)join_cfg->password) == 5)
        {
            JoinReq->sec_type = SSV6XXX_SEC_WEP_40;
        }
        else if (ssv6xxx_strlen((char *)join_cfg->password) == 13)
        {
            JoinReq->sec_type = SSV6XXX_SEC_WEP_104;
        }

        else if ((length == 10) || (length == 26))
        {
			if(length == 10)
				JoinReq->sec_type = SSV6XXX_SEC_WEP_40;
			else
				JoinReq->sec_type = SSV6XXX_SEC_WEP_104;

            for (i = 0; i < length; ++i)
			{
				tmp = CharToHex(join_cfg->password[i]);
				if (tmp  == 0xff)
				{
                    OS_MemFree(JoinReq);
					return -1;
				}

				if(i%2 == 0)
				{
					output[outi] = (tmp << 4) & 0xf0;
				}
				else
				{
					output[outi] |= tmp;
					++outi;
				}
			}
			OS_MemCPY((void *)(join_cfg->password), (char *)output, length+ 1);
        }
        else
        {
            LOG_PRINTF("wrong password failed !!\r\n");
            OS_MemFree(JoinReq);
            return -1;
        }
    }
    else if (sec_type == WIFI_SEC_WPA_PSK)
    {
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_WPA_PSK;
    }
    else if (sec_type == WIFI_SEC_WPA2_PSK)
    {
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_WPA2_PSK;
    }
    else
    {
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_NONE;
        LOG_PRINTF("ERROR: unkown security type: %d\r\n", sec_type);
    }

    if((sec_type == WIFI_SEC_NONE) && (ssv6xxx_strlen((char *)join_cfg->password)!=0))
    {
        OS_MemCPY((void*)ssid_buf,(void*)join_cfg->ssid.ssid,join_cfg->ssid.ssid_len);
        LOG_PRINTF("The password of AP \"%s\" is error\r\n", ssid_buf);
        OS_MemFree(JoinReq);
        return -1;
    }

    if (ssv6xxx_strlen((char *)join_cfg->password) == 0)
    {
        OS_MemCPY((void *)(JoinReq->password), g_sec_info[JoinReq->sec_type].dfl_password, ssv6xxx_strlen(g_sec_info[JoinReq->sec_type].dfl_password) + 1);
    }
    else
    {
        OS_MemCPY((void *)(JoinReq->password), (char *)join_cfg->password, ssv6xxx_strlen((char *)join_cfg->password) + 1);
    }

    OS_MemCPY((void*)&JoinReq->bss, (void*)ap_info_bss, sizeof(struct ssv6xxx_ieee80211_bss));
    LOG_DEBUGF(LOG_L4_NETMGR,("dtim_period = %d, vif=%d\r\n",JoinReq->bss.dtim_period,JoinReq->vif_idx));
    LOG_DEBUGF(LOG_L4_NETMGR,("wmm_used    = %d\r\n",JoinReq->bss.wmm_used));
    OS_MemCPY((void*)ssid_buf,(void*)JoinReq->bss.ssid.ssid,JoinReq->bss.ssid.ssid_len);
    JoinReq->no_bcn_timeout = g_host_cfg.sta_no_bcn_timeout;

    if(g_netmgr_config[JoinReq->vif_idx].s_dhcpc_enable)
    {
        LOG_PRINTF("Set Default to %s\r\n",if_name[JoinReq->vif_idx]);
        netdev_set_default(if_name[JoinReq->vif_idx]);
    }
    if (ssv6xxx_wifi_join(JoinReq) < 0)
    {
        LOG_PRINTF("ssv6xxx_wifi_join failed !!\r\n");
        OS_MemFree(JoinReq);
        return -1;
    }
    g_wifi_is_joining_b = true;


#ifdef NET_MGR_AUTO_JOIN
    {
        user_ap_info ap_item;
        OS_MemSET(&ap_item, 0, sizeof(ap_item));
        ap_item.valid = false;
        ap_item.join_times = 0;
        ap_item.vif_idx = JoinReq->vif_idx;
        //ssv6xxx_strcpy((char *)(ap_item.ssid.ssid), (char *)join_cfg->ssid.ssid);
        OS_MemCPY((void *)(ap_item.ssid), (void *)join_cfg->ssid, sizeof(struct cfg_80211_ssid));
        ssv6xxx_strcpy((char *)ap_item.password, (char *)JoinReq->password);
        netmgr_apinfo_save(&ap_item);
        OS_MemCPY((void*)ssid_buf,(void*)ap_item.ssid.ssid,ap_item.ssid.ssid_len);
        LOG_DEBUGF(LOG_L4_NETMGR, ("AutoJoin: SAVE SSID[%s] info, waiting join success \r\n", (char *)ssid_buf));
    }
#endif

#ifdef NET_MGR_AUTO_RETRY
     if (g_auto_retry_ap[JoinReq->vif_idx].g_auto_retry_status == S_TRY_INVALID)
     {
          g_auto_retry_ap[JoinReq->vif_idx].g_auto_retry_status = S_TRY_INIT;
          //ssv6xxx_strcpy((char *)(g_auto_retry_ap.ssid.ssid), (char *)join_cfg->ssid.ssid);
          OS_MemCPY((void *)&(g_auto_retry_ap[JoinReq->vif_idx].ssid), (void *)&join_cfg->ssid, sizeof(struct cfg_80211_ssid));
          ssv6xxx_strcpy((char *)g_auto_retry_ap[JoinReq->vif_idx].password, (char *)JoinReq->password);
          OS_MemCPY((void*)ssid_buf,(void*)g_auto_retry_ap[JoinReq->vif_idx].ssid.ssid,g_auto_retry_ap[JoinReq->vif_idx].ssid.ssid_len);
          g_auto_retry_ap[JoinReq->vif_idx].vif_idx = JoinReq->vif_idx;
          LOG_DEBUGF(LOG_L4_NETMGR, ("AutoTry: netmgr_wifi_join SAVE SSID[%s] info, waiting join success\r\n", (char *)(ssid_buf)));
     }
#endif

    OS_MemFree(JoinReq);

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join done\r\n"));

    return 0;
}

int netmgr_wifi_join_other(wifi_sta_join_cfg *join_cfg)
{
    int ret=0;
    ssv_type_u8 vif_idx,i;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join_other \r\n"));

    if (g_wifi_is_joining_b)
    {
        return -1;
    }

    //Find a sta vif and non-connected
    if(join_cfg->vif_idx >= MAX_VIF_NUM)
    {
        for(i=0;i<MAX_VIF_NUM;i++)
        {
            if (netmgr_wifi_get_vif_mode(i)==SSV6XXX_HWM_STA)
            {
                vif_idx = i;
                if(!netmgr_wifi_check_sta_connected(vif_idx))
                {
                    break;
                }
            }
        }
    }
    else
    {
        if (netmgr_wifi_get_vif_mode(join_cfg->vif_idx)==SSV6XXX_HWM_STA)
            vif_idx = join_cfg->vif_idx; //Assign by application
        else
            vif_idx = MAX_VIF_NUM;
    }

    if(vif_idx >= MAX_VIF_NUM)
    {
        LOG_PRINTF("join_other mode error.\r\n");
        return -1;
    }

#ifdef NET_MGR_AUTO_RETRY
    netmgr_auto_retry_reset(vif_idx);
#endif

    if (netmgr_wifi_check_sta_connected(vif_idx))
    {
        LOG_PRINTF("leave old ap,vif_idx=%d\r\n",vif_idx);
        netmgr_wifi_leave(vif_idx);
        Sleep(1000);
    }
    join_cfg->vif_idx = vif_idx;
    if ((join_cfg->ssid.ssid_len) == 0 || (ssv6xxx_strlen((char *)join_cfg->password) > MAX_PASSWD_LEN))
    {
        LOG_PRINTF("netmgr_wifi_join_other parameter error.\r\n");
        return -1;
    }

    ret=_netmgr_wifi_switch(SSV6XXX_HWM_STA, NULL, join_cfg, g_sta_channel_mask, g_sta_5g_channel_mask,false);
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_join_other \r\n"));
    return ret;

}

int netmgr_wifi_leave(ssv_type_u8 vif_idx)
{
    struct cfg_leave_request *LeaveReq = NULL;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_leave \r\n"));
    LOG_PRINTF("netmgr_wifi_leave vif_idx=%d\r\n",vif_idx);

    if (!netmgr_wifi_check_sta(NULL))
    {
        LOG_PRINTF("mode error.\r\n");
        return -1;
    }

	LeaveReq = (void *)OS_MemAlloc(sizeof(struct cfg_leave_request));
    if(NULL==LeaveReq)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return -1;
    }
	LeaveReq->reason = 1;
    LeaveReq->vif_idx = vif_idx;
    if (ssv6xxx_wifi_leave(LeaveReq) < 0)
    {
        LOG_PRINTF("netmgr leave fail\r\n");
        OS_MemFree(LeaveReq);
        return -1;
    }
    else
    {
        netmgr_netif_link_set(LINK_DOWN,vif_idx);
    }

    OS_MemFree(LeaveReq);
    return 0;
}
extern void ssv6xxx_wifi_station_async(void* pcfg);
extern ssv6xxx_result ssv6xxx_wifi_ap_async(Ap_setting *ap_setting);
int netmgr_wifi_control(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_cfg *sta_cfg)
{
    int ret = 0;

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_control \r\n"));

    if (mode >= SSV6XXX_HWM_INVALID)
    {
        return -1;
    }

    if ((mode == SSV6XXX_HWM_STA)||(mode == SSV6XXX_HWM_SCONFIG))
    {
        if (sta_cfg)
        {
            wifi_sta_cfg sta_setting;
            OS_MemSET(&sta_setting,0,sizeof(wifi_sta_cfg));
            if (g_netmgr_config[sta_cfg->vif_idx].s_dhcpd_enable && sta_cfg->status)
            {
                netmgr_dhcpd_start(false,sta_cfg->vif_idx);
            }
            sta_setting.mode = mode;
            sta_setting.status = sta_cfg->status;
            sta_setting.vif_idx = sta_cfg->vif_idx;

            LOG_PRINTF("Nmgr ctl STA vif=%d\r\n",sta_cfg->vif_idx);

            ssv6xxx_wifi_station_async((void*)&sta_setting);
            //_ssv6xxx_wifi_send_cmd((void *)&sta_setting, sizeof(struct stamode_setting),SSV6XXX_HOST_CMD_SET_STA_CFG);
            //OS_SemWait(ap_sta_on_off_sphr,0);

            //ret = ssv6xxx_wifi_station(mode,sta_cfg);

            ssv_netmgr_add_netdev(sta_cfg->vif_idx,FALSE);
            if ((ret == (int)SSV6XXX_SUCCESS) && g_netmgr_config[sta_cfg->vif_idx].s_dhcpc_enable)
            {
                netmgr_netif_link_set(LINK_DOWN,sta_cfg->vif_idx);
                netmgr_netif_status_set(false,sta_cfg->vif_idx);
                if (sta_cfg->status)
                {
                    //netmgr_dhcpc_set(true);
                }
                else
                {
                    netmgr_dhcpc_start(false,sta_cfg->vif_idx);
                }
            }
            else if(!g_netmgr_config[sta_cfg->vif_idx].s_dhcpc_enable)
            {
                netmgr_netif_link_set(LINK_UP,sta_cfg->vif_idx);
                //netmgr_netif_status_set(true,sta_cfg->vif_idx);
            }
        }
    }
    else if(mode == SSV6XXX_HWM_AP)
    {
        if (ap_cfg)
        {
            Ap_setting ap_setting;
            OS_MemSET(&ap_setting,0,sizeof(Ap_setting));
			ap_setting = *ap_cfg;
            LOG_PRINTF("Nmgr ctl AP vif=%d\r\n",ap_cfg->vif_idx);
            if((!ap_cfg->static_ip)&&(ap_cfg->status))
                netmgr_dhcpd_set(TRUE,ap_cfg->vif_idx);
            if((ap_cfg->channel!=EN_CHANNEL_AUTO_SELECT)&&(ap_cfg->status))
            {
                if(FALSE==ssv6xxx_wifi_is_available_channel(SSV6XXX_HWM_AP,ap_cfg->channel))
                {
                    return -1;
                }
            }

            if (g_netmgr_config[ap_cfg->vif_idx].s_dhcpc_enable && ap_cfg->status)
            {
                netmgr_dhcpc_start(false,ap_cfg->vif_idx);
            }

            ap_setting.step = 0;
            ssv6xxx_wifi_ap_async(&ap_setting);
            if ((ret == (int)SSV6XXX_SUCCESS) && g_netmgr_config[ap_cfg->vif_idx].s_dhcpd_enable)
            {
                if (ap_cfg->status)
                {
                    LOG_PRINTF("AP vif=%d link up, dhcpd=%d\r\n",ap_cfg->vif_idx,g_netmgr_config[ap_cfg->vif_idx].s_dhcpd_enable);
                    ssv_netmgr_add_netdev(ap_cfg->vif_idx,FALSE);
                    LOG_PRINTF("AP link up, ap_cfg->status=%d\r\n",ap_cfg->status);
                    netmgr_netif_link_set(LINK_UP,ap_cfg->vif_idx);
                    netmgr_netif_status_set(true,ap_cfg->vif_idx);
                    //if((g_netmgr_config[ap_cfg->vif_idx].dhcps.start_ip)&&(g_netmgr_config[ap_cfg->vif_idx].dhcps.end_ip))
                        netmgr_dhcpd_start(true,ap_cfg->vif_idx);
                }
                else
                {
                    netmgr_netif_link_set(LINK_DOWN,ap_cfg->vif_idx);
                    netmgr_dhcpd_start(false,ap_cfg->vif_idx);
                }
            }
            else if (!g_netmgr_config[ap_cfg->vif_idx].s_dhcpd_enable)
            {
                netmgr_netif_link_set(LINK_UP,ap_cfg->vif_idx);
                netmgr_netif_status_set(true,ap_cfg->vif_idx);
            }
        }
    }
    else
    {
        // not support
    }

    return ret;
}

//For switch function, user shall assign an interface to switch
static int _netmgr_wifi_switch(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_join_cfg *join_cfg, ssv_type_u16 scanning_channel_mask, ssv_type_u32 scanning_5g_channel_mask, ssv_type_bool sta_reset)
{
    int ret = 0;
    wifi_sta_cfg sta_cfg;
    char *ssid[1];
    //ssv_type_u8 vif_idx;
    ssv6xxx_hw_mode vif_mode;
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_switch \r\n"));

    if (mode >= SSV6XXX_HWM_INVALID)
    {
        return -1;
    }

    if (mode == SSV6XXX_HWM_STA)
    {
        vif_mode = netmgr_wifi_get_vif_mode(join_cfg->vif_idx);
#if (AP_MODE_ENABLE == 1)
        if(vif_mode == SSV6XXX_HWM_AP)
        {
            netmgr_wifi_ap_off(join_cfg->vif_idx);
        }
#endif
#if(ENABLE_SMART_CONFIG==1)

        if(vif_mode == SSV6XXX_HWM_SCONFIG)
        {
            netmgr_wifi_sconfig_off(join_cfg->vif_idx);
        }
#endif

        if (g_netmgr_config[join_cfg->vif_idx].s_dhcpd_enable)
        {
            netmgr_dhcpd_start(false,join_cfg->vif_idx);
        }

        netmgr_netif_link_set(LINK_DOWN,join_cfg->vif_idx);
        netmgr_netif_status_set(false,join_cfg->vif_idx);

        if (sta_reset)
        {
            sta_cfg.status = TRUE;
            sta_cfg.vif_idx = join_cfg->vif_idx;
            sta_cfg.mode = SSV6XXX_HWM_STA;
            ret = ssv6xxx_wifi_station(&sta_cfg);
        }

        if (join_cfg)
        {
            // do scan for join.
            ssv_type_u32 _ch_mask=0;
            ssv_type_u32 _5g_ch_mask=0;
            g_switch_join_cfg_b = true;
            OS_MemCPY((void * )&g_join_cfg_data, (void * )join_cfg, sizeof(wifi_sta_join_cfg));
            //ssid[0]=(char *)join_cfg->ssid.ssid;
            ssid[0]=(char *)&join_cfg->ssid;
            _ch_mask=(join_cfg->def_ch_mask==0)?scanning_channel_mask:join_cfg->def_ch_mask;
            _5g_ch_mask=(join_cfg->def_5g_ch_mask==0)?scanning_5g_channel_mask:join_cfg->def_5g_ch_mask;

            netmgr_wifi_scan_ex(_ch_mask, _5g_ch_mask, ssid, 1);
        }

    }
#if (AP_MODE_ENABLE == 1)
    else if(mode == SSV6XXX_HWM_AP)
    {
        vif_mode = netmgr_wifi_get_vif_mode(ap_cfg->vif_idx);
        if (vif_mode == SSV6XXX_HWM_STA)
        {
            netmgr_wifi_station_off(ap_cfg->vif_idx);
        }
#if(ENABLE_SMART_CONFIG==1)
        if (vif_mode == SSV6XXX_HWM_SCONFIG)
        {
            netmgr_wifi_sconfig_off(ap_cfg->vif_idx);
        }
#endif

        if (ap_cfg)
        {
            Ap_setting ap_setting;
            OS_MemSET(&ap_setting,0,sizeof(Ap_setting));
			ap_setting = *ap_cfg;

            if(ap_cfg->channel!=EN_CHANNEL_AUTO_SELECT)
            {
                if(FALSE==ssv6xxx_wifi_is_available_channel(SSV6XXX_HWM_AP,ap_cfg->channel))
                {
                    return -1;
                }
            }

            if (g_netmgr_config[ap_cfg->vif_idx].s_dhcpc_enable)
            {
                netmgr_dhcpc_start(false,ap_cfg->vif_idx);
            }

            ap_setting.step = 0;
            ap_setting.status = true;
            ssv6xxx_wifi_ap_async(&ap_setting);

            if ((ret == (int)SSV6XXX_SUCCESS) && g_netmgr_config[ap_cfg->vif_idx].s_dhcpd_enable)
            {
                netmgr_dhcpd_start(true,ap_cfg->vif_idx);
            }
        }
    }
#endif
    else
    {
        // not support
    }

    return ret;
}

int netmgr_wifi_switch(wifi_mode mode, ssv_wifi_ap_cfg *ap_cfg, wifi_sta_join_cfg *join_cfg)
{
    return _netmgr_wifi_switch(mode, ap_cfg, join_cfg, g_sta_channel_mask, g_sta_5g_channel_mask,true);
}

int netmgr_wifi_switch_to_sta(wifi_sta_join_cfg *join_cfg, ssv_type_u8 join_channel) //for Sconfig
{
    ssv_type_u8 bit_nm=0;
    bit_nm=ssv6xxx_wifi_ch_to_bitmask(join_channel);
    if(IS_5G_BAND(join_channel))
        return _netmgr_wifi_switch(SSV6XXX_HWM_STA, NULL, join_cfg, 0, (1<<bit_nm), true);
    else
        return _netmgr_wifi_switch(SSV6XXX_HWM_STA, NULL, join_cfg, (1<<bit_nm), 0, true);
}

void netmgr_wifi_event_cb(ssv_type_u32 evt_id, void *data, ssv_type_s32 len, void* priv)
{

#if !NET_MGR_NO_SYS
    MsgEvent *msg_evt = NULL;
#else
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
#endif

    //LOG_PRINTF("evt_id = %d\r\n", evt_id);

    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_wifi_event_cb evt_id = %d\r\n", evt_id));

    switch (evt_id)
    {
        case SOC_EVT_SCAN_DONE:
        {
            #if !NET_MGR_NO_SYS

            struct resp_evt_result *scan_done = (struct resp_evt_result *)data;

            if (scan_done)
            {
                msg_evt = msg_evt_alloc();
                if(NULL!=msg_evt)
                {
                    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                    msg_evt->MsgData = MSG_SCAN_DONE;
                    msg_evt->MsgData1 = scan_done->u.scan_done.result_code;
                    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                    {
                        OS_MsDelay(10);
                        LOG_PRINTF("SCAN_DONE retry");
                    }
                }
                else
                {
                    LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                }
            }

            #else

            struct resp_evt_result *scan_done = (struct resp_evt_result *)data;
            if (scan_done->u.scan_done.result_code == 0)
            {
                if (g_switch_join_cfg_b)
                {
                    // do join
                    netmgr_wifi_join(&g_join_cfg_data);
                }
            }
            else
            {
                if (g_switch_join_cfg_b)
                {
                    // can't join
                    OS_MemCPY((void*)ssid_buf,(void*)g_join_cfg_data.ssid.ssid,g_join_cfg_data.ssid.ssid_len);
                    LOG_PRINTF("Scan FAIL, can't join [%s]\r\n", ssid_buf);
                }
            }

            g_switch_join_cfg_b = false;

            #ifdef  NET_MGR_AUTO_JOIN
            netmgr_autojoin_process();
            #endif

            #endif
            break;
        }

        case SOC_EVT_SCAN_RESULT: // join result
        {
#if !NET_MGR_NO_SYS

            ap_info_state *scan_res = (ap_info_state *) data;
            if (scan_res)
            {
                msg_evt = msg_evt_alloc();
                if(NULL!=msg_evt)
                {
                    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                    msg_evt->MsgData = MSG_SCAN_RESULT;
                    msg_evt->MsgData1 = (ssv_type_u32)scan_res->apInfo;
                    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                    {
                        OS_MsDelay(10);
                        LOG_PRINTF("SCAN_RESULT retry");
                    }
                }
                else
                {
                    LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                }
            }
#else
            #ifdef  NET_MGR_AUTO_JOIN

            ap_info_state *scan_res = (ap_info_state *)data;
            if (scan_res)
            {
                g_ap_list_p = scan_res->apInfo;
            }
            #endif
#endif
            break;
        }
        case SOC_EVT_SCONFIG_SCAN_DONE: // join result
        {
#if !NET_MGR_NO_SYS
            struct resp_evt_result *sconfig_done = (struct resp_evt_result *)data;
            if (sconfig_done)
            {
                msg_evt = msg_evt_alloc();
                if(NULL!=msg_evt)
                {
                    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                    msg_evt->MsgData = MSG_SCONFIG_SCANNING_DONE;

                    sconfig_result.valid=0;
                    OS_MemCPY((void *)sconfig_result.ssid.ssid, (void *)sconfig_done->u.sconfig_done.ssid,sconfig_done->u.sconfig_done.ssid_len);
                    sconfig_result.ssid.ssid_len=sconfig_done->u.sconfig_done.ssid_len;
                    ssv6xxx_strcpy((char *)sconfig_result.password, (char *)sconfig_done->u.sconfig_done.pwd);
                    sconfig_result.channel=sconfig_done->u.sconfig_done.channel;
                    sconfig_result.dat=sconfig_done->u.sconfig_done.rand;
                    sconfig_result.valid=1;

                    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                    {
                        OS_MsDelay(10);
                        LOG_PRINTF("SCONFIG_SCAN_DONE retry");
                    }
                }
                else
                {
                    LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                }
            }
#else

#endif
            break;
        }
        case SOC_EVT_JOIN_RESULT: // join result
        {

#if !NET_MGR_NO_SYS
            struct resp_evt_result *join_res = (struct resp_evt_result *)data;
            g_wifi_is_joining_b = false;

            if (join_res)
            {
                msg_evt = msg_evt_alloc();
                if(NULL!=msg_evt)
                {
                    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                    msg_evt->MsgData = MSG_JOIN_RESULT;
                    msg_evt->MsgData1 = (join_res->u.join.status_code != 0) ? DISCONNECT : CONNECT;
                    msg_evt->MsgData2 = join_res->u.join.bssid_idx;
                    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                    {
                        OS_MsDelay(10);
                        LOG_PRINTF("JOIN_RESULT retry");
                    }
                }
                else
                {
                    LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                }
            }
#else
            struct ip_addr ipaddr, netmask, gw;
            int join_status;
            ssv_type_u8 vif_idx;
            join_status = (((struct resp_evt_result *)data)->u.join.status_code != 0) ? DISCONNECT : CONNECT;
            vif_idx = ((struct resp_evt_result *)data)->bssid_idx;
            g_wifi_is_joining_b = false;

            /* join success */
            if (join_status == CONNECT)
            {
                netmgr_netif_link_set(true,vif_idx);

                if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                {
                    netmgr_dhcpc_start(true,vif_idx);
                }
                else
                {
                    netmgr_netif_status_set(true,vif_idx);
                }

                #ifdef NET_MGR_AUTO_JOIN
                {
                    user_ap_info *ap_info;
                    Ap_sta_status *info = NULL;

                    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
                    if(NULL!=info)
                    {
                        if (netmgr_wifi_info_get(info,vif_idx) == 0)
                        {
                            OS_MemCPY((void*)ssid_buf,(void*)info->vif_u[vif_idx].station_status.ssid.ssid,info->vif_u[vif_idx].station_status.ssid.ssid_len);
                            LOG_PRINTF("#### SSID[%s] connected \r\n", ssid_buf);
                                                    ap_info = netmgr_apinfo_find((char *)info->u.station.ssid.ssid);
                            if (ap_info)
                            {
                               netmgr_apinfo_set(ap_info, true);
                            }
                        }

                        OS_MemFree(info);
                    }
                    else
                    {
                        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
                    }
                }
                #endif
            }
            /* join failure */
            else if (join_status == DISCONNECT)
            {
                int ret = ERR_OK;
                netmgr_netif_link_set(false);
                netmgr_netif_status_set(false);

                if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                {
                    ret = netdev_setipv4info(g_netifdev.name,0,0,0);
                    dhcpc_wrapper_set(g_netifdev.name, false);
                    g_netmgr_config[vif_idx].s_dhcpc_status = false;
                }
                else
                {
                    netmgr_netif_status_set(false);
                }

                #ifdef NET_MGR_AUTO_JOIN
                //netmgr_autojoin_process();
                #endif
            }

#endif
            break;
        }
        case SOC_EVT_LEAVE_RESULT: // leave result include disconnnet
        {
#if !NET_MGR_NO_SYS
            struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
            g_wifi_is_joining_b = false;

            if (leave_res)
            {
                msg_evt = msg_evt_alloc();
                if(NULL!=msg_evt)
                {
                    msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                    msg_evt->MsgData = MSG_LEAVE_RESULT;
                    msg_evt->MsgData1 = (ssv_type_u32)(leave_res->u.leave.reason_code);
                    msg_evt->MsgData2 = leave_res->u.leave.bssid_idx;
                    while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                    {
                        OS_MsDelay(10);
                        LOG_PRINTF("LEAVE_RESULT retry");
                    }
                }
                else
                {
                    LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
                }
            }
#else
            int leave_reason;
            struct ip_addr ipaddr, netmask, gw;
            ssv_type_u8 vif_idx = ((struct resp_evt_result *)data)->bssid_idx;
            leave_reason = ((struct resp_evt_result *)data)->u.leave.reason_code;

            g_wifi_is_joining_b = false;

            /* leave success */
            if (netmgr_wifi_check_sta(NULL))
            {
                int ret = ERR_OK;
                netmgr_netif_link_set(false,vif_idx);
                netmgr_netif_status_set(false,vif_idx);

                if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                {
                    ret = netdev_setipv4info(g_netifdev[vif_idx].name,0,0,0);
                    dhcpc_wrapper_set(g_netifdev[vif_idx].name, false);
                    g_netmgr_config[vif_idx].s_dhcpc_status = false;
                }
                else
                {
                    netmgr_netif_status_set(false,vif_idx);
                }

                #ifdef NET_MGR_AUTO_JOIN
                if (leave_reason != 0)
                {
                    netmgr_autojoin_process();
                }
                #endif
            }
            else
            {
                // do nothing
            }

#endif
            break;
        }
        case SOC_EVT_POLL_STATION: // ARP request
        {
            if(netmgr_send_arp_unicast(data)== -1)
            {
                ssv_type_u8 * mac = data;
                 LOG_PRINTF("Poll station fail, MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
                   mac[0], mac[1], mac[2],mac[3],mac[4],mac[5] );
            }
            break;
        }
        case SOC_EVT_PS_SETUP_OK:
        case SOC_EVT_PS_WAKENED:
        {
            ssv_type_u8* pmd= (ssv_type_u8 *)data;
            msg_evt = msg_evt_alloc();
            if(NULL!=msg_evt)
            {
                msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                //LOG_PRINTF("ps ev=%d,md=%d\r\n",evt_id,*pmd);
                if(evt_id == SOC_EVT_PS_WAKENED)
                {
                    struct cfg_ps_request wowreq;
                    wowreq.mode = *pmd;
                    ssv6xxx_wifi_pwr_saving(&wowreq,FALSE);
                    OS_MsDelay(15);
                    ssv6xxx_set_wakeup_bb_gpio(0,g_host_cfg.rxIntGPIO); //Wakeup BB low

                    if(g_host_cfg.extRxInt)
                        ssv_hal_set_ext_rx_int(g_host_cfg.rxIntGPIO);

                    msg_evt->MsgData = MSG_PS_WAKENED;
                }
                else if(evt_id == SOC_EVT_PS_SETUP_OK)
                {
                    msg_evt->MsgData = MSG_PS_SETUP_OK;
                }

                msg_evt->MsgData1 = *pmd;
                while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                {
                    OS_MsDelay(10);
                    LOG_PRINTF("SOC_EVT_PS retry");
                }
            }
            else
            {
                LOG_PRINTF("%s,%d:msg alloc fail\r\n",__FUNCTION__, __LINE__);
            }
            break;
        }
        case SOC_EVT_STA_STATUS: // Station add or remove
        {
            struct cfg_set_sta *cfg_sta =data;

            if(cfg_sta->sta_oper == CFG_STA_ADD)
            {
                LOG_PRINTF("ADD station, MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
                cfg_sta->wsid_info.addr.addr[0],
                cfg_sta->wsid_info.addr.addr[1],
                cfg_sta->wsid_info.addr.addr[2],
                cfg_sta->wsid_info.addr.addr[3],
                cfg_sta->wsid_info.addr.addr[4],
                cfg_sta->wsid_info.addr.addr[5] );
                netdev_set_default(if_name[cfg_sta->vif_idx]);

            }
            else if(cfg_sta->sta_oper == CFG_STA_DEL)
            {
                LOG_PRINTF("DEL station, MAC:[%02x:%02x:%02x:%02x:%02x:%02x] \r\n",
                cfg_sta->wsid_info.addr.addr[0],
                cfg_sta->wsid_info.addr.addr[1],
                cfg_sta->wsid_info.addr.addr[2],
                cfg_sta->wsid_info.addr.addr[3],
                cfg_sta->wsid_info.addr.addr[4],
                cfg_sta->wsid_info.addr.addr[5] );
            }

            break;
        }
        default:
            // do nothing
            break;
    }

    return;
}

void netmgr_wifi_reg_event(void)
{
    ssv6xxx_wifi_reg_evt_cb(netmgr_wifi_event_cb,NULL);
}

void netmgr_ifup_cb(ssv_type_u8 vif_idx)
{
    MsgEvent *msg_evt=NULL;
    if (!((sconfig_result.valid == 0) && (g_sconfig_user_mode == FALSE)))
    {
        msg_evt = msg_evt_alloc();
        if(NULL!=msg_evt)
        {
            LOG_PRINTF("%s:post message\r\n",__FUNCTION__);
            msg_evt->MsgType = MEVT_NET_MGR_EVENT;
            msg_evt->MsgData = MSG_SCONFIG_DONE;
            msg_evt->MsgData1 = vif_idx;
            while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
            {
                OS_MsDelay(10);
                LOG_PRINTF("ifup_cb retry");
            }
        }
        else
        {
            LOG_PRINTF("%s:msg alloc fail\r\n",__FUNCTION__);
        }
    }
}

#if !NET_MGR_NO_SYS
void netmgr_task(void *arg)
{
    MsgEvent *msg_evt = NULL;
    OsMsgQ mbox = st_netmgr_task[0].qevt;
    ssv_type_s32 res = 0;
    //u32 lastTRX_time=0;
    int ret = 0;
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};

    while(1)
    {
        #ifdef NET_MGR_AUTO_RETRY
        res = msg_evt_fetch_timeout(mbox, &msg_evt, 1000/TICK_RATE_MS);
        #else
        res = msg_evt_fetch(mbox, &msg_evt);
        #endif
        if (res != OS_SUCCESS)
        {
            #ifdef NET_MGR_AUTO_RETRY
            int j;
            for(j=0;j<MAX_VIF_NUM;j++)
            {
                if ((g_auto_retry_ap[j].g_auto_retry_status == S_TRY_RUN)&&(true == netmgr_wifi_check_sta(NULL)))
                {
                    ssv_type_u32 curr_time = OS_TICK2MS(OS_GetSysTick()) / 1000;

                    if(( g_switch_join_cfg_b)||(curr_time < (g_auto_retry_ap[j].g_auto_retry_start_time + g_auto_retry_times_delay)))
                    {
                        continue;
                    }

                    if ((++g_auto_retry_ap[j].g_auto_retry_times) <= g_auto_retry_times_max)
                    {
                        if (g_auto_retry_ap[j].ssid.ssid_len > 0)
                        {
                            wifi_sta_join_cfg *join_cfg = NULL;
                            LOG_DEBUGF(LOG_L4_NETMGR, ("\r\nAUTO RETRY [%d]  %u\r\n", g_auto_retry_ap[j].g_auto_retry_times,  curr_time));
                            join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
                            if(NULL!=join_cfg)
                            {
                                OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));
                                //join_cfg->ssid.ssid_len=ssv6xxx_strlen((const char *)g_auto_retry_ap.ssid.ssid);
                                //ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, (char *)g_auto_retry_ap.ssid.ssid);
                                OS_MemCPY((void *)&join_cfg->ssid, (void *)&g_auto_retry_ap[j].ssid, sizeof(struct cfg_80211_ssid));
                                ssv6xxx_strcpy((char *)join_cfg->password, (char *)g_auto_retry_ap[j].password);
                                join_cfg->vif_idx = g_auto_retry_ap[j].vif_idx;
                                ret = _netmgr_wifi_switch(SSV6XXX_HWM_STA, NULL, join_cfg, g_sta_channel_mask, g_sta_5g_channel_mask, false);
                                if (ret != 0)
                                {
                                    g_switch_join_cfg_b = false;
                                }

                                OS_MemFree(join_cfg);
                            }
                            else
                            {
                                LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
                            }
                        }

                        g_auto_retry_ap[j].g_auto_retry_start_time = OS_TICK2MS(OS_GetSysTick()) / 1000;
                    }
                    else
                    {
                        LOG_DEBUGF(LOG_L4_NETMGR, ("\r\nAUTO RETRY [%d], time out\r\n", g_auto_retry_ap[j].g_auto_retry_times));
                        g_auto_retry_ap[j].g_auto_retry_status = S_TRY_STOP;
                        g_auto_retry_ap[j].g_auto_retry_times = 0;
                        g_auto_retry_ap[j].g_auto_retry_start_time = 0;

                    }
                }
            }
            #endif
            continue;
        }

        else if (msg_evt && (msg_evt->MsgType == MEVT_HOST_TIMER))
        {
            os_timer_expired((void *)msg_evt);
            os_msg_free(msg_evt);
        }
        else if (msg_evt && (msg_evt->MsgType == MEVT_NET_MGR_EVENT))
        {
            LOG_DEBUGF(LOG_L4_NETMGR, ("EVENT [%d]\r\n", msg_evt->MsgData));
            switch (msg_evt->MsgData)
            {
                case MSG_SCAN_DONE:
                {
                    ssv_type_u8 result_code=(ssv_type_u8)msg_evt->MsgData1;
                    ssv_type_bool scan_join_fail = false;
                    if (result_code == 0)
                    {
                        if (g_switch_join_cfg_b)
                        {
                            // do join
                            ret = netmgr_wifi_join(&g_join_cfg_data);
                            if (ret != 0)
                            {
                                g_switch_join_cfg_b = false;
                                scan_join_fail = true;
                            }
                        }
                    }
                    else
                    {
                        if (g_switch_join_cfg_b)
                        {
                            g_switch_join_cfg_b = false;
                            scan_join_fail = true;
                            // can't join
                            OS_MemCPY((void*)ssid_buf,(void*)g_join_cfg_data.ssid.ssid,g_join_cfg_data.ssid.ssid_len);
                            LOG_PRINTF("Scan FAIL, can't join [%s]\r\n", ssid_buf);
                        }
                    }

                    #ifdef  NET_MGR_AUTO_JOIN
                    netmgr_autojoin_process();
                    #endif

                    #ifdef NET_MGR_AUTO_RETRY
                    if (scan_join_fail)
                    {
                        if (g_auto_retry_ap[g_join_cfg_data.vif_idx].g_auto_retry_status == S_TRY_INVALID)
                        {
                            g_auto_retry_ap[g_join_cfg_data.vif_idx].g_auto_retry_status = S_TRY_INIT;
                        }

                        //OS_MemCPY((void *)&(g_auto_retry_ap[g_join_cfg_data.].ssid), (void *)&(g_join_cfg_data.ssid), sizeof(struct cfg_80211_ssid));
                        //ssv6xxx_strcpy((char *)g_auto_retry_ap.password, (char *)g_join_cfg_data.password);
                        OS_MemCPY((void*)ssid_buf,(void*)g_join_cfg_data.ssid.ssid,g_join_cfg_data.ssid.ssid_len);
                        //g_auto_retry_ap.vif_idx = g_join_cfg_data.vif_idx;

                        LOG_DEBUGF(LOG_L4_NETMGR, ("AutoTry: scan_join_fail SAVE SSID[%s] info, waiting join success\r\n", ssid_buf));

                        netmgr_auto_retry_update(g_join_cfg_data.vif_idx);
                    }
                    #endif
                    break;
                }

#if(ENABLE_SMART_CONFIG==1)
                case MSG_SCONFIG_REQ:
                {
                    ssv_type_u16 channel_mask = msg_evt->MsgData1;
                    ssv_type_u32 channel_5g_mask = msg_evt->MsgData2;
                    netmgr_wifi_sconfig_ex(channel_mask,channel_5g_mask);
                    if(TRUE==g_sconfig_user_mode){
                        //Reuse the msg_evt
                        msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                        msg_evt->MsgData = MSG_SCONFIG_PROCESS;
                        msg_evt->MsgData1 = 0;
                        msg_evt->MsgData2 = 0;
                        msg_evt->MsgData3 = 0;
                        while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                        {
                            OS_MsDelay(10);
                            LOG_PRINTF("MSG_SCONFIG_REQ retry");
                        }
                        msg_evt=NULL;
                    }
                    break;
                }
                #endif
                case MSG_SCAN_REQ:
                {
                    ssv_type_u16 channel_mask = msg_evt->MsgData1;
                    ssv_type_u32 channel_5g_mask = msg_evt->MsgData2;

                    netmgr_wifi_scan_ex(channel_mask, channel_5g_mask, 0, 0);
                    break;
                }

                case MSG_JOIN_REQ:
                {
                    wifi_sta_join_cfg *join_cfg_msg = (wifi_sta_join_cfg *)msg_evt->MsgData1;

                    netmgr_wifi_join(join_cfg_msg);

                    if (join_cfg_msg)
                    {
                        OS_MemFree(join_cfg_msg);
                    }
                    break;
                }
                case MSG_JOIN_OTHER_REQ:
                {
                    wifi_sta_join_cfg *join_cfg_msg = (wifi_sta_join_cfg *)msg_evt->MsgData1;

                    netmgr_wifi_join_other(join_cfg_msg);

                    if (join_cfg_msg)
                    {
                        OS_MemFree(join_cfg_msg);
                    }
                    break;
                }
                case MSG_LEAVE_REQ:
                {
                    LOG_PRINTF("MSG_LEAVE_REQ vif_idx=%d\r\n",msg_evt->MsgData1);
                    netmgr_wifi_leave(msg_evt->MsgData1);
                    break;
                }
                case MSG_VIF_OFF_REQ:
                {
                    LOG_PRINTF("MSG_VIF_OFF_REQ vif_idx=%d\r\n",msg_evt->MsgData1);
                    netmgr_wifi_vif_off(msg_evt->MsgData1);
                    break;
                }
                case MSG_AP_IP_CHANGE:
                {
                    ssv_type_u8 vif_idx = msg_evt->MsgData1;
                    ipinfo *info = (ipinfo*)(msg_evt->MsgData2);

                    netmgr_ipinfo_set(vif_idx,info, true);
                    if(info)
                    {
                        OS_MemFree(info);
                    }

                    break;
                }
                case MSG_CONTROL_REQ:
                {
                    wifi_mode mode = (wifi_mode)msg_evt->MsgData1;
                    ssv_wifi_ap_cfg *ap_cfg_msg = (ssv_wifi_ap_cfg *)msg_evt->MsgData2;
                    wifi_sta_cfg *sta_cfg_msg = (wifi_sta_cfg *)msg_evt->MsgData3;
                    //ssv_type_u8 vif_idx;

                    if(ap_cfg_msg)
                    {
                       LOG_PRINTF("use vif=%d for AP\r\n",ap_cfg_msg->vif_idx);
                    }

                    if(sta_cfg_msg)
                    {
                        LOG_PRINTF("use vif=%d for STA\r\n",sta_cfg_msg->vif_idx);
                    }

                    netmgr_wifi_control(mode, ap_cfg_msg, sta_cfg_msg);

                    if (ap_cfg_msg)
                    {
                        OS_MemFree(ap_cfg_msg);
                    }
                    if (sta_cfg_msg)
                    {
                        OS_MemFree(sta_cfg_msg);
                    }
                    break;
                }
                case MSG_SWITCH_REQ:
                {
                    wifi_mode mode = (wifi_mode)msg_evt->MsgData1;
                    ssv_wifi_ap_cfg *ap_cfg_msg = (ssv_wifi_ap_cfg *)msg_evt->MsgData2;
                    wifi_sta_join_cfg *join_cfg_msg = (wifi_sta_join_cfg *)msg_evt->MsgData3;

                    netmgr_wifi_switch(mode, ap_cfg_msg, join_cfg_msg);

                    if (ap_cfg_msg)
                    {
                        OS_MemFree(ap_cfg_msg);
                    }
                    if (join_cfg_msg)
                    {
                        OS_MemFree(join_cfg_msg);
                    }
                    break;
                }

                case MSG_SCAN_RESULT:
                {
                    #ifdef  NET_MGR_AUTO_JOIN
                    g_ap_list_p = (struct ssv6xxx_ieee80211_bss *)msg_evt->MsgData1;
                    #endif
                    break;
                }

#if(ENABLE_SMART_CONFIG==1)
                case MSG_SCONFIG_DONE:
                {
                    Ap_sta_status *pinfo=NULL;
                    ssv_type_u32 scfg_vif_idx;

                    scfg_vif_idx = msg_evt->MsgData1;
                    pinfo=OS_MemAlloc(sizeof(Ap_sta_status));
                    if(pinfo==NULL)
                    {
                        break;
                    }

                    if(-1!=netmgr_wifi_info_get(pinfo,scfg_vif_idx))
                    {
                        if(sconfig_result.valid==1)
                        {
                            if(0==ssv6xxx_memcmp((void *)(pinfo->vif_u[scfg_vif_idx].station_status.ssid.ssid),(void *)(sconfig_result.ssid.ssid),sconfig_result.ssid.ssid_len))
                            {
                                if(g_sconfig_user_mode==TRUE)
                                {
                                    if(netmgr_wifi_check_sta(NULL))
                                    {
                                        if(ssv6xxx_user_sconfig_op.UserSconfigConnect!=NULL)
                                        {
                                            ssv6xxx_user_sconfig_op.UserSconfigConnect();
                                        }
                                    }

                                    if(ssv6xxx_user_sconfig_op.UserSconfigDeinit!=NULL)
                                    {
                                        ssv6xxx_user_sconfig_op.UserSconfigDeinit();
                                    }

                                    g_sconfig_user_mode=FALSE;
                                }
                                else
                                {
                                    if(netmgr_wifi_check_sta(NULL))
                                    {
                                        netmgr_wifi_sconfig_done((ssv_type_u8 *)&sconfig_result.dat,1,TRUE,10000);
                                    }
                                }
                                sconfig_result.valid=0;
                            }
                        }
                    }

                    OS_MemFree(pinfo);
                    break;
                }
                case MSG_SCONFIG_SCANNING_DONE:
                {
                    wifi_sta_join_cfg *join_cfg = NULL;
                    if((1==g_sconfig_auto_join)&&(sconfig_result.valid==1))
                    {
                        join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
                        if(NULL!=join_cfg)
                        {
                            OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));
                            //join_cfg->ssid.ssid_len=ssv6xxx_strlen((const char *)sconfig_done_cpy->u.sconfig_done.ssid);
                            //ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, (char *)sconfig_done_cpy->u.sconfig_done.ssid);
                            join_cfg->ssid.ssid_len=sconfig_result.ssid.ssid_len;
                            for(join_cfg->vif_idx=0;join_cfg->vif_idx<MAX_VIF_NUM;join_cfg->vif_idx++)
                            {
                                if (netmgr_wifi_get_vif_mode(join_cfg->vif_idx)==SSV6XXX_HWM_SCONFIG)
                                {
                                    LOG_PRINTF("Get Sconfig vif=%d\r\n",join_cfg->vif_idx);
                                    break;
                                }
                            }
                            if(join_cfg->vif_idx >= MAX_VIF_NUM)
                            {
                                LOG_PRINTF("use default vif 0\r\n");
                                join_cfg->vif_idx=0;
                            }
                            OS_MemCPY((void *)join_cfg->ssid.ssid, (void *)sconfig_result.ssid.ssid,sconfig_result.ssid.ssid_len);
                            ssv6xxx_strcpy((char *)join_cfg->password, (char *)sconfig_result.password);
                            netmgr_wifi_switch_to_sta(join_cfg,sconfig_result.channel);
                            OS_MemFree(join_cfg);
                        }
                        else
                        {
                            LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
                        }
                    }
                    #if 0
                    else
                    {

                        sta_cfg.status = TRUE;
                        ssv6xxx_wifi_station(SSV6XXX_HWM_STA, &sta_cfg);
                    }
                    #endif

                    break;
                }
#endif
                case MSG_JOIN_RESULT:
                {
                    int join_status;
                    ssv_type_u8 vif_idx;
                    join_status = msg_evt->MsgData1;
                    vif_idx = (ssv_type_u8)msg_evt->MsgData2;
                    g_switch_join_cfg_b = false;
                    LOG_DEBUGF(LOG_L4_NETMGR,("MSG_JOIN_RESULT=%d,vif_idx=%d\r\n",join_status,vif_idx));
                    /* join success */
                    if (join_status == CONNECT)
                    {
                        netmgr_netif_link_set(true,vif_idx);

                        if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                        {
                            netmgr_dhcpc_start(true,vif_idx);
                        }
                        else
                        {
                            netmgr_netif_status_set(true,vif_idx);
                        }

                        LOG_DEBUGF(LOG_L4_NETMGR, ("JOIN SUCCESS\r\n"));

                        #ifdef NET_MGR_AUTO_JOIN
                        {
                            user_ap_info *ap_info;
                            Ap_sta_status *info = NULL;

                            info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
                            if(NULL!=info)
                            {
                                if (netmgr_wifi_info_get(info,vif_idx) == 0)
                                {
                                    OS_MemCPY((void*)ssid_buf,(void*)info->vif_u[vif_idx].station_status.ssid.ssid,info->vif_u[vif_idx].station_status.ssid.ssid_len);
                                    LOG_DEBUGF(LOG_L4_NETMGR, ("AutoJoin: SSID[%s] connected \r\n", ssid_buf));
                                    ap_info = netmgr_apinfo_find((char *)info->vif_u[vif_idx].station_status.ssid.ssid);
                                    if (ap_info)
                                    {
                                       netmgr_apinfo_set(ap_info, true);
                                    }
                                }

                                OS_MemFree(info);
                            }
                            else
                            {
                                LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
                            }
                        }
                        #endif

                        #ifdef NET_MGR_AUTO_RETRY
                        {
                            g_auto_retry_ap[vif_idx].g_auto_retry_status = S_TRY_STOP;
                            g_auto_retry_ap[vif_idx].g_auto_retry_times = 0;
                            g_auto_retry_ap[vif_idx].g_auto_retry_start_time = 0;

                            LOG_DEBUGF(LOG_L4_NETMGR, ("AUTO RETRY S_TRY_STOP\r\n"));
                        }
                        #endif
                    }
                    /* join failure */
                    else if (join_status == DISCONNECT)
                    {
                        int ret = NS_OK;
                        LOG_DEBUGF(LOG_L4_NETMGR, ("JOIN FAILED\r\n"));
                        netmgr_netif_link_set(false,vif_idx);
                        netmgr_netif_status_set(false,vif_idx);

                        if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                        {
                            ret = netdev_setipv4info(g_netifdev[vif_idx].name,0,0,0);
                            dhcpc_wrapper_set(g_netifdev[vif_idx].name, false);
                            g_netmgr_config[vif_idx].s_dhcpc_status = false;
                        }
                        else
                        {
	                        netmgr_netif_status_set(false,vif_idx);
                        }

                        //#ifdef NET_MGR_AUTO_JOIN
                        //netmgr_autojoin_process();
                        //#endif

                        #ifdef NET_MGR_AUTO_RETRY
                        netmgr_auto_retry_update(vif_idx);
                        #endif
                    }
                    break;
                }
                case MSG_LEAVE_RESULT:
                {
                    int leave_reason;
                    ssv_type_u8 vif_idx;
                    leave_reason = msg_evt->MsgData1;
                    vif_idx = msg_evt->MsgData2;
                    LOG_PRINTF("leave result reason = %d,vif_idx=%d\r\n", leave_reason,vif_idx);

                    /* leave success */
                    if (netmgr_wifi_get_vif_mode(vif_idx)==SSV6XXX_HWM_STA)
                    {
                        int ret = NS_OK;

                        netmgr_netif_link_set(false,vif_idx);
                        netmgr_netif_status_set(false,vif_idx);

                        if (g_netmgr_config[vif_idx].s_dhcpc_enable)
                        {
                            ret = netdev_setipv4info(g_netifdev[vif_idx].name,0,0,0);
                            dhcpc_wrapper_set(g_netifdev[vif_idx].name, false);
                            g_netmgr_config[vif_idx].s_dhcpc_status = false;
                        }
                        else
                        {
	                        netmgr_netif_status_set(false,vif_idx);
                        }

                        #ifdef NET_MGR_AUTO_JOIN
                        if ((leave_reason == 4) || (leave_reason == 3))
                        {
                            netmgr_wifi_scan_ex(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL, 0, 0);
                        }
                        else
                        #endif
                        #ifdef NET_MGR_AUTO_RETRY
                        {
                            if((leave_reason != 14)&&(leave_reason<18))
                            {
                                netmgr_auto_retry_update(vif_idx);
                            }
                            else
                            {
                                netmgr_auto_retry_reset(vif_idx); // Password error no more retry.
                            }
                        }
                        #endif
                    }

                    break;
                }
                case MSG_PS_WAKENED:
                {
                    LOG_PRINTF("MSG_PS_WAKENED\r\n");
                    //lastTRX_time = ssv6xxx_drv_get_TRX_time_stamp();
                    break;
                }
                case MSG_PS_SETUP_OK:
                {
                    struct cfg_ps_request wowreq;
                    LOG_PRINTF("MSG_PS_SETUP_OK\r\n");
                    wowreq.host_ps_st = HOST_PS_START;
                    wowreq.mode = msg_evt->MsgData1;
                    ssv6xxx_wifi_pwr_saving(&wowreq,true);
                    break;
                }

#if(ENABLE_SMART_CONFIG==1)
                case MSG_SCONFIG_PROCESS:
                {
                    if(FALSE==netmgr_wifi_check_sconfig(NULL))
                    {
                        g_sconfig_user_mode=FALSE;
                    }
                    else
                    {
                        if((ssv6xxx_user_sconfig_op.UserSconfigSM!=NULL)&&(g_sconfig_user_mode==TRUE)){
                            if(1== ssv6xxx_user_sconfig_op.UserSconfigSM()){
                                OS_MsDelay(50);
                                //Reuse the msg_evt
                                msg_evt->MsgType = MEVT_NET_MGR_EVENT;
                                msg_evt->MsgData = MSG_SCONFIG_PROCESS;
                                msg_evt->MsgData1 = 0;
                                msg_evt->MsgData2 = 0;
                                msg_evt->MsgData3 = 0;
                                while(msg_evt_post(st_netmgr_task[0].qevt, msg_evt)==OS_FAILED)
                                {
                                    OS_MsDelay(10);
                                    LOG_PRINTF("MSG_SCONFIG_PROCESS retry");
                                }
                                msg_evt = NULL;
                            }
                        }
                    }
                    break;
                }

#endif
            }
            if(NULL!=msg_evt)
            {
                msg_evt_free(msg_evt);
            }
        }
    }
}
#endif

ssv_type_s32 netmgr_show(void)
{
    ssv_type_bool dhcpd_status, dhcpc_status;
    int i;
    netmgr_cfg cfg;
    netstack_ip_addr_t ip,mask,gw,dns;

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        netmgr_cfg_get(&cfg,i);

        LOG_PRINTF("\r\n");

        LOG_PRINTF("NET CONFIG, VIF=%d\r\n",i);
        ip.addr = cfg.ipaddr;
        mask.addr = cfg.netmask;
        gw.addr = cfg.gw;
        dns.addr = cfg.dns;
        LOG_PRINTF("IP  : %s\r\n", netstack_inet_ntoa(ip));
        LOG_PRINTF("MASK: %s\r\n", netstack_inet_ntoa(mask));
        LOG_PRINTF("GW  : %s\r\n", netstack_inet_ntoa(gw));
        LOG_PRINTF("DNS : %s\r\n", netstack_inet_ntoa(dns));

        LOG_PRINTF("\r\n");
        LOG_PRINTF("DHCPD CONFIG\r\n");

        ip.addr = (cfg.dhcps.start_ip);
        LOG_PRINTF("start ip      : %s\r\n", netstack_inet_ntoa(ip));

        ip.addr = (cfg.dhcps.end_ip);
        LOG_PRINTF("end ip        : %s\r\n", netstack_inet_ntoa(ip));

        LOG_PRINTF("max leases    : %d\r\n", cfg.dhcps.max_leases);

        mask.addr = (cfg.dhcps.subnet);
        LOG_PRINTF("subnet        : %s\r\n", netstack_inet_ntoa(mask));

        gw.addr = (cfg.dhcps.gw);
        LOG_PRINTF("gate way      : %s\r\n", netstack_inet_ntoa(gw));

        dns.addr = (cfg.dhcps.dns);
        LOG_PRINTF("dns           : %s\r\n", netstack_inet_ntoa(dns));

        LOG_PRINTF("auto time     : %d\r\n", cfg.dhcps.auto_time);

        LOG_PRINTF("decline time  : %d\r\n", cfg.dhcps.decline_time);

        LOG_PRINTF("conflict time : %d\r\n", cfg.dhcps.conflict_time);

        LOG_PRINTF("offer time    : %d\r\n", cfg.dhcps.offer_time);

        LOG_PRINTF("max leases sec: %d\r\n", cfg.dhcps.max_lease_sec);

        LOG_PRINTF("min leases sec: %d\r\n", cfg.dhcps.min_lease_sec);

        netmgr_dhcp_status_get(&dhcpd_status, &dhcpc_status, i);
        LOG_PRINTF("Dhcpd: %s\r\n", dhcpd_status ? "on" : "off");
        LOG_PRINTF("Dhcpc: %s\r\n", dhcpc_status ? "on" : "off");
    }

    LOG_PRINTF("\r\n");

    if (dhcpd_status)
    {
        dhcpdipmac ipmac[10];
        int i, count = 10;
        int ret = 0;

        ret = netstack_dhcp_ipmac_get(ipmac, &count);

        if ((ret == 0) && (count > 0))
        {
            LOG_PRINTF("DHCPD CLIENT\r\n");
            LOG_PRINTF("-----------------------------------------\r\n");
            LOG_PRINTF("|        MAC         |          IP      |\r\n");
            LOG_PRINTF("-----------------------------------------\r\n");
            for (i = 0; i < count; i++)
            {
                LOG_PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x] --- [%d.%d.%d.%d]\r\n", ipmac[i].mac[0], ipmac[i].mac[1], ipmac[i].mac[2],
                                    ipmac[i].mac[3], ipmac[i].mac[4], ipmac[i].mac[5],
                                     netstack_ip4_addr1_16(&ipmac[i].ip),
                          netstack_ip4_addr2_16(&ipmac[i].ip),
                          netstack_ip4_addr3_16(&ipmac[i].ip),
                          netstack_ip4_addr4_16(&ipmac[i].ip));
            }
        }
    }

#ifdef  NET_MGR_AUTO_JOIN
        netmgr_apinfo_show();
#endif

#ifdef  NET_MGR_AUTO_RETRY
        netmgr_auto_retry_show();
#endif

    return 0;
}

#ifdef  NET_MGR_AUTO_JOIN
int netmgr_apinfo_get_last(void);
void timer_rescan_for_auto_join(void* data1, void* data2)
{
    netmgr_wifi_scan_ex(SCAN_ALL_2G_CHANNEL, SCAN_ALL_5G_CHANNEL, 0, 0);
}
int netmgr_autojoin_process(void)
{
    ssv_type_u32 AP_cnt,i;
    struct ssv6xxx_ieee80211_bss *ap_list = NULL;
    int last_index;

    last_index = netmgr_apinfo_get_last();

    if (last_index != -1)
    {
        user_ap_info *ap_info;
        AP_cnt = ssv6xxx_get_aplist_info((void *)&ap_list);
        LOG_PRINTF("netmgr_autojoin_process AP cnt=%d\r\n",AP_cnt);
        ap_info = netmgr_apinfo_find_best(ap_list, AP_cnt);
        OS_MemFree((void *)ap_list);
        if ((ap_info != NULL) && ap_info->valid)
        {
            netmgr_apinfo_autojoin(ap_info);
            return 1;
        }
        else
        {
            LOG_PRINTF("Auto Join not found AP\r\n");

            os_create_timer(5000,timer_rescan_for_auto_join,NULL,NULL,(void*)st_netmgr_task[0].qevt);
        }
    }

    return 0;
}

void netmgr_apinfo_clear()
{
    int i = 0;

    for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
    {
        OS_MemSET(&g_user_ap_info[i], 0, sizeof(g_user_ap_info[i]));
    }
}

void netmgr_apinfo_remove(char *ssid)
{
    int i = 0;

    if (!ssid)
    {
        return;
    }

    for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
    {
        //if (ssv6xxx_strcmp((char *)ssid, (char *)g_user_ap_info[i].ssid.ssid) == 0)
        if (ssv6xxx_memcmp((void *)ssid, (void *)g_user_ap_info[i].ssid.ssid, g_user_ap_info[i].ssid.ssid_len) == 0)
        {
            OS_MemSET(&g_user_ap_info[i], 0, sizeof(g_user_ap_info[i]));
            return;
        }
    }

    return;
}

user_ap_info * netmgr_apinfo_find(char *ssid)
{
    int i = 0;

    if (!ssid)
    {
        return NULL;
    }

    for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
    {
        //if (ssv6xxx_strcmp((char *)ssid, (char *)g_user_ap_info[i].ssid.ssid) == 0)
        if (ssv6xxx_memcmp((void *)ssid, (void *)g_user_ap_info[i].ssid.ssid, g_user_ap_info[i].ssid.ssid_len) == 0)
        {
            return &(g_user_ap_info[i]);
        }
    }

    return NULL;
}

ssv_type_bool netmgr_apinfo_find_in_aplist(struct ssv6xxx_ieee80211_bss * ap_list_p, int count, char * ssid)
{
    int i = 0;
    struct ssv6xxx_ieee80211_bss *item = NULL;

    if (!ssid || !ap_list_p || !(count > 0))
    {
        return false;
    }
    //LOG_PRINTF("target ssid=%s\r\n",ssid);
    for (i = 0; i < count; i++)
    {
        item = (ap_list_p + i);

        if (item && (item->channel_id != 0))
        {
            //if (ssv6xxx_strcmp((char *)ssid, (char *)(item->ssid.ssid)) == 0)
            //LOG_PRINTF("item ssid=%s,len=%d\r\n",item->ssid.ssid,item->ssid.ssid_len);
            if (ssv6xxx_memcmp((void *)ssid, (void *)(item->ssid.ssid),item->ssid.ssid_len) == 0)
            {
                return true;
            }
        }

    }

    return false;
}

user_ap_info * netmgr_apinfo_find_best(struct ssv6xxx_ieee80211_bss * ap_list_p, int count)
{
    int i = 0;

    for (i = NET_MGR_USER_AP_COUNT - 1; i >= 0; i--)
    {
        if(g_user_ap_info[i].valid)
        {
            if (netmgr_apinfo_find_in_aplist(ap_list_p, count, (char *)(char *)g_user_ap_info[i].ssid.ssid))
            {
                return &g_user_ap_info[i];
            }
        }
    }

    return NULL;
}

int netmgr_apinfo_get_last(void)
{
    int i = 0;

    for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
    {
        if ((g_user_ap_info[i].valid) == 0)
        {
            break;
        }
    }

    if (i >= NET_MGR_USER_AP_COUNT)
    {
        return (NET_MGR_USER_AP_COUNT - 1);
    }

    if (i == 0)
    {
        return -1;
    }

    return (i - 1);
}

void netmgr_apinfo_save(user_ap_info *ap_info)
{
    int i = 0, j = 0;
    int last_index = 0;

    if (ap_info == NULL)
    {
        return;
    }

    last_index = netmgr_apinfo_get_last();

    if (last_index < 0)
    {
        OS_MemCPY((void *)&(g_user_ap_info[i]), (char *)ap_info, sizeof(user_ap_info));
    }
    else
    {
        for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
        {
            //if (ssv6xxx_strcmp((char *)ap_info->ssid.ssid, (char *)g_user_ap_info[i].ssid.ssid) == 0)
            if (ssv6xxx_memcmp((void *)ap_info->ssid.ssid, (void *)g_user_ap_info[i].ssid.ssid, g_user_ap_info[i].ssid.ssid_len) == 0)
            {
                break;
            }
        }

        if (i < NET_MGR_USER_AP_COUNT)
        {
            for (j = i; j < last_index; j++)
            {
                OS_MemCPY((void *)&(g_user_ap_info[j]), (void *)&(g_user_ap_info[j + 1]), sizeof(user_ap_info));
            }

            // replace old item, but valid/join time not change
            OS_MemCPY((void *)&(g_user_ap_info[last_index].ssid), (char *)&(ap_info->ssid), sizeof(struct cfg_80211_ssid));
            OS_MemCPY((void *)g_user_ap_info[last_index].password, (char *)ap_info->password, sizeof(char)*(MAX_PASSWD_LEN+1));
        }
        else
        {
            if (last_index == (NET_MGR_USER_AP_COUNT - 1))
            {
                for (j = 0; j < (NET_MGR_USER_AP_COUNT - 1); j++)
                {
                    OS_MemCPY((void *)&(g_user_ap_info[j]), (void *)&(g_user_ap_info[j + 1]), sizeof(user_ap_info));
                }

                OS_MemCPY((void *)&(g_user_ap_info[last_index]), (char *)ap_info, sizeof(user_ap_info));
            }
            else
            {
                OS_MemCPY((void *)&(g_user_ap_info[last_index + 1]), (char *)ap_info, sizeof(user_ap_info));
            }
        }
    }
}

void netmgr_apinfo_set(user_ap_info *ap_info, ssv_type_bool valid)
{
    if (ap_info == NULL)
    {
        return;
    }

    ap_info->valid = valid;

    if (!valid)
    {
        OS_MemSET(ap_info, 0, sizeof(user_ap_info));
    }
}

static int netmgr_apinfo_autojoin(user_ap_info *ap_info)
{
    int ret = 0;
    wifi_sta_join_cfg *join_cfg = NULL;
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};

    if (ap_info == NULL)
    {
        return -1;
    }

    join_cfg = OS_MemAlloc(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return -1;
    }
    OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));
    //ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, (char *)ap_info->ssid.ssid);
    OS_MemCPY((void *)&join_cfg->ssid, (void *)&ap_info->ssid, sizeof(struct cfg_80211_ssid));
    ssv6xxx_strcpy((char *)join_cfg->password, (char *)ap_info->password);
    join_cfg->vif_idx = ap_info->vif_idx;
    ret = netmgr_wifi_join(join_cfg);
    if (ret != 0)
    {
        // do nothing
    }
    else
    {
        ap_info=netmgr_apinfo_find((char *)join_cfg->ssid.ssid);
        if(ap_info!=NULL){
            OS_MemCPY((void*)ssid_buf,(void*)ap_info->ssid.ssid,ap_info->ssid.ssid_len);
            LOG_PRINTF("\r\nAuto join [%s]\r\n", ssid_buf);
            ap_info->join_times++;
        }
    }

    OS_MemFree(join_cfg);

    return ret;
}

void netmgr_apinfo_show()
{
    int i = 0;
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};

    //LOG_PRINTF("  netmgr_apinfo_show  \r\n");
    LOG_PRINTF("\r\n");
    LOG_PRINTF("-------------------------------------------------\r\n");
    LOG_PRINTF("|%20s|    |%s|   |%8s|  V\r\n", "ssid        ",  "password", "autoJoin");
    LOG_PRINTF("-------------------------------------------------\r\n");
    for (i = 0; i < NET_MGR_USER_AP_COUNT; i++)
    {
        //if(g_user_ap_info[i].valid)
        {
            OS_MemCPY((void*)ssid_buf,(void*)g_user_ap_info[i].ssid.ssid,g_user_ap_info[i].ssid.ssid_len);
            LOG_PRINTF("|%20.20s      %s            %3d  |  %d\r\n", ssid_buf, "****", \
            g_user_ap_info[i].join_times, g_user_ap_info[i].valid);
        }
    }

    LOG_PRINTF("-------------------------------------------------\r\n");
}

#endif

#ifdef  NET_MGR_AUTO_RETRY
void netmgr_auto_retry_reset(ssv_type_u8 vif_idx)
{
    //int i;
    //for(i=0;i<MAX_VIF_NUM;i++)
    {
        if (g_auto_retry_ap[vif_idx].g_auto_retry_status != S_TRY_INVALID)
        {
           g_auto_retry_ap[vif_idx].g_auto_retry_status = S_TRY_INVALID;
           LOG_DEBUGF(LOG_L4_NETMGR, ("AUTO RETRY RESET\r\n"));
        }
    }
}

void netmgr_auto_retry_update(ssv_type_u8 vif_idx)
{
    LOG_DEBUGF(LOG_L4_NETMGR, ("netmgr_auto_retry_update: g_auto_retry_status=%d\r\n", g_auto_retry_ap[vif_idx].g_auto_retry_status));

    if ((g_auto_retry_ap[vif_idx].g_auto_retry_status == S_TRY_INIT) || (g_auto_retry_ap[vif_idx].g_auto_retry_status == S_TRY_STOP))
    {
        g_auto_retry_ap[vif_idx].g_auto_retry_status = S_TRY_RUN;
        g_auto_retry_ap[vif_idx].g_auto_retry_times = 0;
        g_auto_retry_ap[vif_idx].g_auto_retry_start_time = OS_TICK2MS(OS_GetSysTick()) / 1000;
        LOG_DEBUGF(LOG_L4_NETMGR, ("AUTO RETRY S_TRY_RUN, TIMES = %u, TIME = %u\r\n",
            g_auto_retry_ap[vif_idx].g_auto_retry_times + 1, g_auto_retry_ap[vif_idx].g_auto_retry_start_time));
    }
    else if (g_auto_retry_ap[vif_idx].g_auto_retry_status == S_TRY_RUN)
    {
        if ((g_auto_retry_ap[vif_idx].g_auto_retry_times + 1) > g_auto_retry_times_max)
        {
            g_auto_retry_ap[vif_idx].g_auto_retry_status = S_TRY_STOP;
            g_auto_retry_ap[vif_idx].g_auto_retry_times  = 0;
            g_auto_retry_ap[vif_idx].g_auto_retry_start_time = 0;
            LOG_DEBUGF(LOG_L4_NETMGR, ("AUTO RETRY S_TRY_STOP\r\n"));
        }
        else
        {
            g_auto_retry_ap[vif_idx].g_auto_retry_start_time = OS_TICK2MS(OS_GetSysTick()) / 1000;
            LOG_DEBUGF(LOG_L4_NETMGR, ("AUTO RETRY S_TRY_RUN, TIMES = %u, TIME = %u\r\n",
                g_auto_retry_ap[vif_idx].g_auto_retry_times + 1, g_auto_retry_ap[vif_idx].g_auto_retry_start_time));
        }
    }
}

void netmgr_auto_retry_show()
{
    ssv_type_u8 ssid_buf[MAX_SSID_LEN+1]={0};
    int i;
    for(i=0;i<MAX_VIF_NUM;i++)
    {
        LOG_PRINTF("\r\nNETMGR AUTO RETRY :   \r\n");

        if (g_auto_retry_ap[i].g_auto_retry_status == S_TRY_INIT)
            LOG_PRINTF("STATUS            : INIT\r\n");
        else if (g_auto_retry_ap[i].g_auto_retry_status == S_TRY_RUN)
            LOG_PRINTF("STATUS            : RUN\r\n");
        else if (g_auto_retry_ap[i].g_auto_retry_status == S_TRY_STOP)
            LOG_PRINTF("STATUS            : STOP\r\n");
        else
            LOG_PRINTF("STATUS            : INVALID\r\n");

        LOG_PRINTF("DELAY             : %d\r\n", g_auto_retry_times_delay);
        LOG_PRINTF("MAX TIMES         : %d\r\n", g_auto_retry_times_max);
        LOG_PRINTF("CUR TIMES         : %d\r\n", g_auto_retry_ap[i].g_auto_retry_times);
        OS_MemCPY((void*)ssid_buf,(void*)g_auto_retry_ap[i].ssid.ssid,g_auto_retry_ap[i].ssid.ssid_len);
        LOG_PRINTF("SSID              : %s\r\n", ssid_buf);
    }
}

#endif

static void _netmgr_wifi_recovery_cb(void)
{
    LOG_PRINTF("Clear status for recovery\r\n");
    g_wifi_is_joining_b=FALSE;
    return;
}

int netmgr_wifi_get_sconfig_result(wifi_sconfig_result *res)
{
    if(res==NULL)
    {
        return -1;
    }

    if(sconfig_result.valid==0)
    {
        return -1;
    }
    OS_MemCPY((void *)&res->ssid,(void *)(&sconfig_result.ssid),sizeof(struct cfg_80211_ssid));
    ssv6xxx_strcpy((void *)res->password,(void *)(sconfig_result.password));
    return 0;
}
