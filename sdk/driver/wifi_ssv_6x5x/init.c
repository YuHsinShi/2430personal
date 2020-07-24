/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <host_apis.h>
#if CLI_ENABLE
#include <cli.h>
#include <cmds/cli_cmd_wifi.h>
#endif
#if HTTPD_SUPPORT
#include <httpserver_raw/httpd.h>
#endif
#if(ENABLE_SMART_CONFIG==1)
#include <SmartConfig/SmartConfig.h>
#endif
#if NETMGR_SUPPORT
#include <net_mgr.h>
#endif
#include <netstack.h>
#include <ssv_devinfo.h>
#include <ssv_drv.h>
#include <ssv_lib.h>
#include <pbuf.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
#if SSV_LOG_DEBUG
ssv_type_u32 g_log_module;
ssv_type_u32 g_log_min_level;
#endif

//Configurations
// ----------------------------------------------------
//Mac address
ssv_type_u8 config_mac[] ={0x60,0x11,0x22,0x33,0x44,0x77};

//Max number of AP list
ssv_type_u8 g_max_num_of_ap_list=NUM_AP_INFO;

//Auto channel selection in AP mode
ssv_type_u16 g_acs_channel_mask=ACS_CHANNEL_MASK;
ssv_type_u32 g_acs_5g_channel_mask=ACS_5G_CHANNEL_MASK;
ssv_type_u16 g_acs_channel_scanning_interval=ACS_SCANNING_INTERVAL;
ssv_type_u8  g_acs_channel_scanning_loop=ACS_SCANNING_LOOP;

//Default channel mask in sta and smart config mode
ssv_type_u16 g_sta_scan_ch_interval=SCANNING_CH_INTERVAL;
ssv_type_u16 g_sta_channel_mask = DEFAULT_STA_CHANNEL_MASK;
ssv_type_u32 g_sta_5g_channel_mask = DEFAULT_STA_5G_CHANNEL_MASK;
// ------------------------- rate control ---------------------------
struct Host_cfg g_host_cfg;
struct vif_flow_ctrl g_vif_fctl[MAX_VIF_NUM];
// ------------- User mode SmartConfig ...............................................
#if(ENABLE_SMART_CONFIG==1)
SSV6XXX_USER_SCONFIG_OP ssv6xxx_user_sconfig_op;
ssv_type_u32 g_sconfig_solution=SMART_CONFIG_SOLUTION;
ssv_type_u8 g_sconfig_auto_join=SMART_CONFIG_AUTO_JOIN;
#endif
// ----------------------------------------------------
ssv_type_u8 g_lwip_tcp_ignore_cwnd=LWIP_TCP_IGNORE_CWND;

#ifdef NET_MGR_AUTO_RETRY
int  g_auto_retry_times_delay = NET_MGR_AUTO_RETRY_DELAY;
int  g_auto_retry_times_max = NET_MGR_AUTO_RETRY_TIMES;
#endif

#if DHCPD_SUPPORT
extern int udhcpd_init(void);
#endif

void stop_and_halt (void)
{
    //while (1) {}
    /*lint -restore */
} // end of - stop_and_halt -

//=====================Task parameter setting========================
extern struct task_info_st g_txrx_task_info[];
extern struct task_info_st g_host_task_info[];
extern struct task_info_st g_timer_task_info[];
#if NETMGR_SUPPORT
#if !NET_MGR_NO_SYS
extern struct task_info_st st_netmgr_task[];
#endif
#endif
#if DHCPD_SUPPORT
extern struct task_info_st st_dhcpd_task[];
#endif
#if (MLME_TASK==1)
extern struct task_info_st g_mlme_task_info[];
#endif

void ssv6xxx_init_task_para(void)
{
    g_txrx_task_info[0].prio = OS_TX_TASK_PRIO;
    g_txrx_task_info[1].prio = OS_RX_TASK_PRIO;
    g_host_task_info[0].prio = OS_CMD_ENG_PRIO;
    g_timer_task_info[0].prio = OS_TMR_TASK_PRIO;
#if NETMGR_SUPPORT
#if !NET_MGR_NO_SYS
    st_netmgr_task[0].prio = OS_NETMGR_TASK_PRIO;
#endif
#endif
#if DHCPD_SUPPORT    
    st_dhcpd_task[0].prio = OS_DHCPD_TASK_PRIO;
#endif
#if (MLME_TASK==1)
    g_mlme_task_info[0].prio = OS_MLME_TASK_PRIO;
#endif
}
//=============================================================

int ssv6xxx_start(ssv_vif* vif)
{
    //u32 wifi_mode;
    ssv6xxx_drv_start();

    /* Reset MAC & Re-Init */

    /* Initialize ssv6200 mac */
    if(-1==ssv6xxx_init_mac(vif))
    {
    	return SSV6XXX_FAILED;
    }

    //Set ap or station register
    if ((SSV6XXX_HWM_STA == vif->hw_mode)||(SSV6XXX_HWM_SCONFIG == vif->hw_mode))
    {
        if(-1==ssv6xxx_init_sta_mac(vif->hw_mode))
        {
            return SSV6XXX_FAILED;
        }
    }
    else
    {
        if(-1==ssv6xxx_init_ap_mac(vif))
        {
            return SSV6XXX_FAILED;
        }
    }

    return SSV6XXX_SUCCESS;
}

ssv6xxx_result ssv6xxx_check_ready(ssv6xxx_hw_mode hmode)
{
    ssv_type_u32 start_tick = 0;
    Ap_sta_status *info = NULL;
    ssv_type_bool bRet = false;
    int i=0;

    info = (Ap_sta_status *)OS_MemAlloc(sizeof(Ap_sta_status));
    if (info == NULL)
    {
        return false;
    }

    OS_MemSET((void *)info, 0, sizeof(Ap_sta_status));

    start_tick = OS_GetSysTick();

    while(1)
    {
        ssv6xxx_wifi_status(info);
        switch(hmode)
        {
            case SSV6XXX_HWM_STA:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i]== SSV6XXX_HWM_STA) && (info->status))
                    {
                        bRet = true;
                        //LOG_PRINTF("netmgr check sta vif=%d\r\n",i);
                        break;
                    }
                }            
                break;
            }
            case SSV6XXX_HWM_AP:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i] == SSV6XXX_HWM_AP) && (info->status))
                    {
                        if(info->vif_u[i].ap_status.current_st == AP_STATE_READY)
                            bRet = true;
                            break;
                    }
                }            
                break;
            }
            case SSV6XXX_HWM_SCONFIG:
            {
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if ((info->vif_operate[i] == SSV6XXX_HWM_SCONFIG) && (info->status))
                    {
                         bRet = true;
                         //LOG_PRINTF("netmgr check sconfig vif=%d\r\n",i);
                         break; //for
                    }
                }            
                break;//switch
            }
        }
        
        if((bRet == false)&&(((OS_GetSysTick() - start_tick) * OS_TICK_RATE_MS) < 3000))
        {
            OS_TickDelay(1);
        }
        else
        {
            break; //while
        }
    }
    OS_MemFree(info);

    if (!bRet)
    {
        return SSV6XXX_FAILED;
    }

    return SSV6XXX_SUCCESS;
}

#if 0
#define JOIN_DEFAULT_SSID    "ap-Ian-WPA2" //"china"
#define JOIN_DEFAULT_PSK     "12345678" //"12345678"
ssv_type_u8 join_default_pmk[32]={0xB6, 0x0B, 0x01, 0xDC, 0x84, 0x09, 0xFA, 0x41, 0x55, 0xD3, 0x59, 0x81, 0xDA, 0xAC, 0xCE, 0x6E,
                         0x3F, 0x78, 0x32, 0x7B, 0x74, 0xDC, 0xC3, 0xB3, 0x4C, 0x66, 0xE6, 0xC7, 0x88, 0x83, 0x59, 0xE0};
#define JOIN_DEFAULT_2P4G_CH_MASK    0x02
#define JOIN_DEFAULT_5G_CH_MASK    0x0
#endif

#if 1
#define JOIN_DEFAULT_SSID    "WR941" //"china"
#define JOIN_DEFAULT_PSK     "12345678" //"12345678"
ssv_type_u8 join_default_pmk[32]={0x85, 0x87, 0x6B, 0xDE, 0xC7, 0x8E, 0x20, 0x41, 0xA7, 0xE4, 0xF6, 0x58, 0xDF, 0xD4, 0xC3, 0xCB,
                         0x2E, 0xA1, 0x70, 0xB4, 0xA1, 0x3F, 0x6C, 0x0A, 0xEA, 0x63, 0x1A, 0x54, 0x6E, 0xCE, 0x6A, 0x61};
#define JOIN_DEFAULT_2P4G_CH_MASK    0x02
#define JOIN_DEFAULT_5G_CH_MASK    0x0

#endif
//#define JOIN_DEFAULT_SSID    "Default_ap" //"china"
//#define JOIN_DEFAULT_PSK     "12345678" //"12345678"

#define AP_DEFAULT_SSID    "ssv-6030-AP"
#define AP_DEFAULT_PSK     "12345678"
extern void ssv_netmgr_init_netdev(ssv_type_bool default_cfg);
extern void ssv6xxx_wifi_station_async(void* pcfg);
extern int ssv6xxx_wifi_init_regdom(void);
ssv6xxx_result STAmode_default(ssv_type_bool bJoin, ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{
    int ret;
    ssv6xxx_hw_mode mode;
    Sta_setting sta;

    mode = SSV6XXX_HWM_STA;
    OS_MemSET(&sta, 0 , sizeof(Sta_setting));    
    sta.status = TRUE;
    sta.vif_idx = 0;
    sta.mode = mode;

#if NETMGR_SUPPORT

#if USE_ICOMM_LWIP
    if (static_ip)
    {
        netmgr_dhcpc_set(false,0);
        {
            ipinfo info;
    
            info.ipv4 = ipaddr_addr("192.168.100.100");
            info.netmask = ipaddr_addr("255.255.255.0");
            info.gateway = ipaddr_addr("192.168.100.1");
            info.dns = ipaddr_addr("192.168.100.1");
    
            netmgr_ipinfo_set(0, &info, false);
        }
    }
    else
    {
        netmgr_dhcpc_set(true,0);
    }
#else
    netmgr_dhcpc_set(true,0);
#endif

    if(bJoin)
    {
        wifi_sta_join_cfg join_cfg;
        OS_MemSET((void * )&join_cfg, 0, sizeof(join_cfg));       
        OS_MemCPY((void *)join_cfg.ssid.ssid,JOIN_DEFAULT_SSID,ssv6xxx_strlen(JOIN_DEFAULT_SSID));
        join_cfg.ssid.ssid_len=ssv6xxx_strlen(JOIN_DEFAULT_SSID);
        ssv6xxx_strcpy((void *)join_cfg.password, JOIN_DEFAULT_PSK);
        join_cfg.vif_idx = 0;
        join_cfg.pmk_valid=1;
        OS_MemCPY((void *)join_cfg.pmk,join_default_pmk,32);
        join_cfg.def_ch_mask=JOIN_DEFAULT_2P4G_CH_MASK;
        join_cfg.def_5g_ch_mask=JOIN_DEFAULT_5G_CH_MASK;
        ret = netmgr_wifi_switch_async(mode, NULL, &join_cfg);
    }
    else
    {
        ret = netmgr_wifi_control_async(mode, NULL, &sta);
    }
#else
    ssv6xxx_wifi_station_async((void*)&sta);
#endif    
    if (ret != 0)
    {
        LOG_PRINTF("STA mode error (%d)\r\n", bJoin);
        return SSV6XXX_FAILED;
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_STA))
    {
        LOG_PRINTF("STA mode timeout\r\n");
        return SSV6XXX_FAILED;
    }

    //LOG_PRINTF("STA mode success\r\n");
    return SSV6XXX_SUCCESS;
}

#if (AP_MODE_ENABLE == 1)
ssv6xxx_result APmode_default(ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{
    ssv6xxx_hw_mode mode;
    Ap_setting ap_cfg;
    int ret = 0;
    
    OS_MemSET((void * )&ap_cfg, 0, sizeof(ap_cfg));
    mode = SSV6XXX_HWM_AP;
#if 1
    ap_cfg.status = TRUE;
    OS_MemCPY((void *)ap_cfg.ssid.ssid,AP_DEFAULT_SSID,ssv6xxx_strlen(AP_DEFAULT_SSID));
    ap_cfg.ssid.ssid_len = ssv6xxx_strlen(AP_DEFAULT_SSID);
    ssv6xxx_strcpy((void *)(ap_cfg.password),AP_DEFAULT_PSK);
    ap_cfg.security =   SSV6XXX_SEC_WPA2_PSK;
    ap_cfg.proto = WPA_PROTO_RSN;
    ap_cfg.key_mgmt = WPA_KEY_MGMT_PSK ;
    ap_cfg.group_cipher=WPA_CIPHER_CCMP;
    ap_cfg.pairwise_cipher = WPA_CIPHER_CCMP;
    ap_cfg.channel = EN_CHANNEL_AUTO_SELECT;
    ap_cfg.vif_idx = 0;
    ap_cfg.step = 0;
#else
    {
        extern ssv_wifi_ap_cfg g_ssv_wifi_ap_cfg;
        OS_MemCPY((void *)&ap_cfg, &g_ssv_wifi_ap_cfg,sizeof(g_ssv_wifi_ap_cfg));
    }
#endif

#if NETMGR_SUPPORT

#if USE_ICOMM_LWIP
    if (static_ip)
    {
        ipinfo info;
    
        info.ipv4 = ipaddr_addr("172.16.10.1");
        info.netmask = ipaddr_addr("255.255.255.0");
        info.gateway = ipaddr_addr("172.16.10.1");
        info.dns = ipaddr_addr("172.16.10.1");
    
        netmgr_ipinfo_set(0,&info, true);
    }
    
    if (dhcp_server)
    {
        netmgr_dhcpd_set(true,0);
    }
    else
    {
        netmgr_dhcpd_set(false,0);
    }
#endif
    ret = netmgr_wifi_control_async(mode , &ap_cfg, NULL);
#else
    ssv6xxx_wifi_ap_async(&ap_cfg);
#endif
    if (ret != 0)
    {
        LOG_PRINTF("AP mode error\r\n");
        return SSV6XXX_FAILED;
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_AP))
    {
        LOG_PRINTF("AP mode timeout\r\n");
        return SSV6XXX_FAILED;
    }
    
    //LOG_PRINTF("AP mode success\r\n");
    return SSV6XXX_SUCCESS;
}
#endif

#if(ENABLE_SMART_CONFIG==1)

extern ssv_type_u16 g_SconfigChannelMask;

ssv6xxx_result SCONFIGmode_default()
{
    ssv6xxx_hw_mode mode;
    Sta_setting sta;
    int ret = 0;
    
    mode = SSV6XXX_HWM_SCONFIG;
    OS_MemSET(&sta, 0 , sizeof(Sta_setting));
    
    sta.status = TRUE;
    sta.vif_idx = 0;
    sta.mode = mode;
#if NETMGR_SUPPORT
    ret = netmgr_wifi_control_async(mode, NULL, &sta);
#else    
    ssv6xxx_wifi_station_async((void*)&sta);
#endif
    if (ret != 0)
    {
        LOG_PRINTF("SCONFIG mode error\r\n");
    }
    
    if (ssv6xxx_check_ready(SSV6XXX_HWM_SCONFIG))
    {
        LOG_PRINTF("SCONFIG mode timeout\r\n");
        return SSV6XXX_FAILED;
    }
    
    g_SconfigChannelMask=0x3FFE; //This variable is only for RD use, customers don't need to modify it.
    g_sconfig_solution=SMART_CONFIG_SOLUTION;
    ret = netmgr_wifi_sconfig_async(g_SconfigChannelMask);
    if (ret != 0)
    {
        LOG_PRINTF("SCONFIG async error\r\n");
        return SSV6XXX_FAILED;
    }
    
    LOG_PRINTF("SCONFIG mode success\r\n");
    return SSV6XXX_SUCCESS;
}
#endif

ssv6xxx_result wifi_start(ssv6xxx_hw_mode mode, ssv_type_bool static_ip, ssv_type_bool dhcp_server)
{

    ssv6xxx_result res;
    /* station mode */
    if (mode == SSV6XXX_HWM_STA)
    {
        res = STAmode_default(false, static_ip, dhcp_server); //false: don't auto join AP_DEFAULT_SSDI
    }

    #if (AP_MODE_ENABLE == 1)
    /* ap mode */
    if (mode == SSV6XXX_HWM_AP)
    {
        res = APmode_default(static_ip, dhcp_server);
    }
    #endif
    #if(ENABLE_SMART_CONFIG==1)
    /* smart link mode */
    if (mode == SSV6XXX_HWM_SCONFIG)
    {
        res = SCONFIGmode_default();
    }
    #endif

    //LOG_PRINTF("wifi start \r\n");
    return res;
}

/**
 *  Entry point of the firmware code. After system booting, this is the
 *  first function to be called from boot code. This function need to
 *  initialize the chip register, software protoctol, RTOS and create
 *  tasks. Note that, all memory resources needed for each software
 *  modulle shall be pre-allocated in initialization time.
 */
/* return int to avoid from compiler warning */

#if (AP_MODE_ENABLE == 1)
extern ssv_type_s32 AP_Init(ssv_type_u32 max_sta_num);
#endif
extern void host_global_init(void);
extern void host_global_deinit(void);
extern void os_timer_init(void);
extern void os_timer_deinit(void);
extern ssv6xxx_result ssv6xxx_wifi_deinit(ssv_type_u32 step);
void ssv6xxx_dev_deinit(void)
{   
    //stop Layer3 data first
    netstack_deinit(NULL); 
    //stop wifi operation
    ssv6xxx_wifi_deinit(0);
    //Stop all task
    ssv6xxx_wifi_deinit(1);
    //release resource
    ssv6xxx_wifi_deinit(2);
    os_timer_deinit();
    ssv6xxx_drv_module_release();
#if (CONFIG_USE_LWIP_PBUF==0)
    PBUF_DeInit();
#endif
    msg_evt_deinit();
    host_global_deinit();
    LOG_PRINTF("%s done\r\n",__FUNCTION__);
    
}
#if (AP_MODE_ENABLE == 1)
extern void ssv6xxx_eapol_data_reg_cb(void);
#endif
int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode)
{
    ssv6xxx_result res;
    LOG_PRINTF("\33[32mSSV dev init %d\33[0m\r\n",OS_GetSysTick());

    //hmode = SSV6XXX_HWM_AP;
#ifndef __SSV_UNIX_SIM_
#if SSV_LDO_EN_PIN_
    platform_ldo_en_pin_init();
    //ldo_en 0 -> 1 equal to HW reset.
    platform_ldo_en(0);
    OS_MsDelay(10);
    platform_ldo_en(1);
#endif
#endif

#if SSV_LOG_DEBUG
	g_log_module = CONFIG_LOG_MODULE;
	g_log_min_level = CONFIG_LOG_LEVEL;
#endif    


#if(ENABLE_SMART_CONFIG==1)
    //This function must be assigned before netmgr_init
    #if(SMART_CONFIG_SOLUTION==CUSTOMER_SOLUTION)
    // Register customer operation function
    // ssv6xxx_user_sconfig_op.UserSconfigInit= xxx ;
    // ssv6xxx_user_sconfig_op.UserSconfigPaserData= xxx ;
    // ssv6xxx_user_sconfig_op.UserSconfigSM= xxx;
    // ssv6xxx_user_sconfig_op.UserSconfigConnect= xxx;
    // ssv6xxx_user_sconfig_op.UserSconfigDeinit= xxx;
    #else
    ssv6xxx_user_sconfig_op.UserSconfigInit=SmartConfigInit;
    ssv6xxx_user_sconfig_op.UserSconfigPaserData=SmartConfigPaserData;
    ssv6xxx_user_sconfig_op.UserSconfigSM=SmartConfigSM;
    ssv6xxx_user_sconfig_op.UserSconfigConnect=SmartConfigConnect;
    ssv6xxx_user_sconfig_op.UserSconfigDeinit=SmartConfigDeinit;
    #endif
#endif    
    //host config default value
    OS_MemSET((void*)&g_host_cfg,0,sizeof(g_host_cfg)); //how about ssv6xxx_memset?
    g_host_cfg.rate_mask= RC_DEFAULT_RATE_MSK;
    g_host_cfg.resent_fail_report= RC_DEFAULT_RESENT_REPORT;
    g_host_cfg.upgrade_per= RC_DEFAULT_UP_PF;
    g_host_cfg.downgrade_per= RC_DEFAULT_DOWN_PF;
    g_host_cfg.pre_alloc_prb_frm= RC_DEFAULT_PREPRBFRM;
    g_host_cfg.direct_rc_down= RC_DIRECT_DOWN;
    g_host_cfg.rc_drate_endian=RC_DEFAULT_DRATE_ENDIAN;
    g_host_cfg.tx_power_mode = CONFIG_TX_PWR_MODE;
    g_host_cfg.pool_size = POOL_SIZE;
    g_host_cfg.pool_sec_size = POOL_SEC_SIZE;
    g_host_cfg.recv_buf_size = RECV_BUF_SIZE;
    g_host_cfg.bcn_interval = AP_BEACON_INT;
    g_host_cfg.trx_hdr_len = TRX_HDR_LEN;
    g_host_cfg.erp = AP_ERP;
    g_host_cfg.b_short_preamble= AP_B_SHORT_PREAMBLE;
    g_host_cfg.tx_sleep = TX_TASK_SLEEP;
    g_host_cfg.tx_sleep_tick = TX_TASK_SLEEP_TICK;    
    g_host_cfg.tx_retry_cnt= TX_TASK_RETRY_CNT;    
    g_host_cfg.tx_res_page = TX_RESOURCE_PAGE;
    g_host_cfg.rx_res_page = RX_RESOURCE_PAGE;
    g_host_cfg.ap_rx_short_GI = AP_RX_SHORT_GI;
    g_host_cfg.ap_rx_support_legacy_rate_msk = AP_RX_SUPPORT_BASIC_RATE;
    g_host_cfg.ap_rx_support_mcs_rate_msk = AP_RX_SUPPORT_MCS_RATE;
    g_host_cfg.txduty.mode = TXDUTY_MODE;
    g_host_cfg.volt_mode = SSV_VOLT_REGULATOR;
    g_host_cfg.ampdu_rx_buf_size= AMPDU_RX_BUF_SIZE;
    g_host_cfg.support_ht = PHY_SUPPORT_HT;
    if(g_host_cfg.support_ht)
    {
        g_host_cfg.ampdu_rx_enable= AMPDU_RX_ENABLE;
        g_host_cfg.ampdu_tx_enable= AMPDU_TX_ENABLE;
    }
    else
    {
        g_host_cfg.ampdu_rx_enable= 0;
        g_host_cfg.ampdu_tx_enable= 0;
    }
    g_host_cfg.support_rf_band = RF_BAND;
    g_host_cfg.support_tx_SGI = RC_DEFAULT_TX_SGI;
    g_host_cfg.ap_no_dfs = AP_NO_DFS_FUN;
    g_host_cfg.ap_bw = AP_BW;
    g_host_cfg.sta_no_bcn_timeout = STA_NO_BCN_TIMEOUT;
    g_host_cfg.hci_rx_aggr = HCI_RX_AGGR;
    g_host_cfg.hci_aggr_tx = HCI_AGGR_TX;
    g_host_cfg.rxIntGPIO   = RXINTGPIO;
    g_host_cfg.usePA = USE_EXT_PA;
    g_host_cfg.AP_TimAllOne = AP_MODE_BEACON_VIRT_BMAP_0XFF;
    g_host_cfg.ApStaInactiveTime = AP_SAT_INACTIVE;
    // Red Bull support dual interfaces, if dual interfaces are working, and one interfaces as AP mode, and the ohter interfaces as STA mode,
    // ap mode just can support 7 clients, because we need to reserve a client for STA mode
    g_host_cfg.ap_supported_sta_num = AP_SUPPORT_CLIENT_NUM; //Reserve one clinet for STA mode
    g_host_cfg.sw_q_force_unlock_time = SW_Q_FORCE_UNLOCK_TIME;
    
    //Vif 0
    g_vif_fctl[0].retry_cnt = TX_TASK_RETRY_CNT;
    g_vif_fctl[0].sleep_tick= TX_TASK_SLEEP_TICK;
    g_vif_fctl[0].priority = FALSE;

    //VIF 1
    g_vif_fctl[1].retry_cnt = TX_TASK_RETRY_CNT;
    g_vif_fctl[1].sleep_tick= TX_TASK_SLEEP_TICK;
    g_vif_fctl[1].priority = TRUE;
    if(ssv6xxx_strcmp(INTERFACE, "spi") == 0)
    {
        g_host_cfg.extRxInt = 1;
    }
    else
    {
#ifdef EXT_RX_INT
        g_host_cfg.extRxInt = 1;
#else
        g_host_cfg.extRxInt = 0;
#endif
    }

    g_host_cfg.xtal=XTAL_SETTING;
    g_host_cfg.info_scan_result = INFO_SCAN_RESULT_TO_APP;
	/**
        * On simulation/emulation platform, initialize RTOS (simulation OS)
        * first. We use this simulation RTOS to create the whole simulation
        * and emulation platform.
        */
    SSV_ASSERT( OS_Init() == OS_SUCCESS );
    host_global_init();
    ssv6xxx_init_task_para();

	LOG_init(true, true, LOG_LEVEL_ON, LOG_MODULE_MASK(LOG_MODULE_EMPTY), false);
#ifdef __SSV_UNIX_SIM__
	LOG_out_dst_open(LOG_OUT_HOST_TERM, NULL);
	LOG_out_dst_turn_on(LOG_OUT_HOST_TERM);
#endif

    SSV_ASSERT( msg_evt_init(g_host_cfg.pool_size+g_host_cfg.pool_sec_size+SSV_TMR_MAX) == OS_SUCCESS );

#if (CONFIG_USE_LWIP_PBUF==0)
    SSV_ASSERT( PBUF_Init(POOL_SIZE) == OS_SUCCESS );
#endif//#if CONFIG_USE_LWIP_PBUF

#if (BUS_TEST_MODE == 0)
    netstack_init(NULL);
#endif
    /**
        * Initialize Host simulation platform. The Host initialization sequence
        * shall be the same as the sequence on the real host platform.
        *   @ Initialize host device drivers (SDIO/SIM/UART/SPI ...)
        */

    SSV_ASSERT(ssv6xxx_drv_module_init() == true);
    //LOG_PRINTF("Try to connecting CABRIO via %s...\n\r",INTERFACE);
#if FAST_CONNECT_PROFILE
    LOG_PRINTF("\33[32msdio s %d\33[0m\n",OS_GetSysTick());
#endif    
    if (ssv6xxx_drv_select(INTERFACE) == false)
    {

        {
        LOG_PRINTF("==============================\r\n");
    	LOG_PRINTF("Please Insert %S wifi device\n",INTERFACE);
		LOG_PRINTF("==============================\r\n");
    	}
		return -1;
    }
#if FAST_CONNECT_PROFILE    
    LOG_PRINTF("\33[32msdio e %d\33[0m\n",OS_GetSysTick());
#endif
    ssv_assert(ssv_hal_init() == true);

    if(ssv6xxx_wifi_init()!=SSV6XXX_SUCCESS){
        LOG_PRINTF("ssv6xxx_wifi_init fail\r\n");
		return -1;
    } else {
        LOG_PRINTF("ssv6xxx_wifi_init success\r\n");
    }

    os_timer_init();
    
    if(ssv6xxx_platform_check() != SSV6XXX_SUCCESS)
        return SSV6XXX_FAILED;

#if (BUS_TEST_MODE == 0)
    ssv6xxx_wifi_init_regdom(); 
    ssv6xxx_wifi_set_reg(DEFAULT_COUNTRY_CODE);

    
#if (AP_MODE_ENABLE == 1)
    AP_Init(g_host_cfg.ap_supported_sta_num);
#endif
    /**
    * Initialize TCP/IP Protocol Stack. If tcpip is init, the net_app_ask()
    * also need to be init.
    */

    
#if DHCPD_SUPPORT
    if(udhcpd_init() != 0)
    {
        LOG_PRINTF("udhcpd_init fail\r\n");
        return SSV6XXX_FAILED;
    }
#endif

#if HTTPD_SUPPORT
    httpd_init();
#endif

#if NETAPP_SUPPORT
    if(net_app_init()== 1)
    {
        LOG_PRINTF("net_app_init fail\n\r");
        return SSV6XXX_FAILED;
    }
#endif

#if NETMGR_SUPPORT
    /* netmgr int */
    ssv_netmgr_init(true);

    /* we can set default ip address and default dhcpd server config */
    ssv_netmgr_init_netdev(true);
#endif
    switch (hmode)
    {
        case SSV6XXX_HWM_STA: //AUTO_INIT_STATION	    
        {
            res = wifi_start(SSV6XXX_HWM_STA,false,false);
            break;
        }
        case SSV6XXX_HWM_AP:
        {
#if (AP_MODE_ENABLE == 1)
           res = wifi_start(SSV6XXX_HWM_AP,false,true);
#else
           res = SSV6XXX_FAILED;
#endif
           break;
        }
        case SSV6XXX_HWM_SCONFIG:
        {

#if(ENABLE_SMART_CONFIG==1)
            res = wifi_start(SSV6XXX_HWM_SCONFIG,false,true);
#else
            res = SSV6XXX_FAILED;
#endif

            break;
        }
        default:
        {
            res = wifi_start(SSV6XXX_HWM_STA,false,false);
        }
    }
    
        

#else //BUS_TEST_MODE
    res = SSV6XXX_SUCCESS;
    {
        extern ssv6xxx_result bus_mode_on(ssv_type_u8 vif_idx);
        ssv6xxx_drv_start();
        bus_mode_on(0);
    }
#endif
#if (CLI_ENABLE==1)
    Cli_Init(0, NULL);
#if (BUS_TEST_MODE == 0)
    ssv6xxx_wifi_cfg();
#endif
#else
#ifndef CONFIG_NO_WPA2
#if (AP_MODE_ENABLE == 1)
    ssv6xxx_eapol_data_reg_cb();
#endif
#endif

#endif

#if (BUS_TEST_MODE == 0)
#if(MLME_TASK ==1)
    mlme_init();    //MLME task initial
#endif
#endif

    return res;
}


//#define SSV_IGMP_DEMO // multicast udp demo

#ifdef SSV_IGMP_DEMO

struct udp_pcb * g_upcb = NULL;
struct ip_addr ipMultiCast;
#define LISTEN_PORT       52000
#define LISTEN_ADDR       0xef0101ff   // 239.1.1.255
#define DST_PORT          52001

void ssv_test_print(const char *title, const ssv_type_u8 *buf,
                             ssv_type_size_t len)
{
    ssv_type_size_t i;
    LOG_PRINTF("%s - hexdump(len=%d):\r\n    ", title, len);
    if (buf == NULL) {
        LOG_PRINTF(" [NULL]");
    }else{
        for (i = 0; i < 16; i++)
            LOG_PRINTF("%02X ", i);

        LOG_PRINTF("\r\n---\r\n00|");
       for (i = 0; i < len; i++){
            LOG_PRINTF(" %02x", buf[i]);
            if((i+1)%16 ==0)
                LOG_PRINTF("\r\n%02x|", (i+1));
        }
    }
    LOG_PRINTF("\r\n-----------------------------\r\n");
}

int ssv_test_udp_tx(u8_t *data, u16_t len, u16_t port)
{
    int ret = -2;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);

    OS_MemCPY(p->payload, data, len);

    ret = udp_sendto(g_upcb, p,(struct ip_addr *) (&ipMultiCast), port);

    pbuf_free(p);

    return ret;
}

void ssv_test_udp_rx(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    unsigned char g_tData[256];
    int ret = 0;

    //LOG_PRINTF("addr:0x%x port:%d len:%d\r\n", addr->addr, port, p->tot_len);

    //ssv_test_print("ssv_test_udp_rx", OS_FRAME_GET_DATA(p), OS_FRAME_GET_DATA_LEN(p));

    ret = udp_connect(upcb, addr, port);            /* connect to the remote host */
    if (ret != 0)
    {
        LOG_PRINTF("udp_connect: ret = %d\r\n", ret);
    }

    if (p->len >= 256) p->len = 128;

    OS_MemCPY(g_tData, p->payload, p->len);
    g_tData[p->len] = 0;
    LOG_PRINTF("rxdata: %s\r\n", g_tData);

    ssv6xxx_strcpy((char *)g_tData, "recv it, ok!");
    ssv_test_udp_tx(g_tData, ssv6xxx_strlen((char *)g_tData), port);

    pbuf_free(p);   /* don't leak the pbuf! */
}


int ssv_test_udp_init(void)
{
    int ret = 1;

#if LWIP_IGMP
    ret =  netmgr_igmp_enable(true);
    if (ret != 0)
    {
        LOG_PRINTF("netmgr_igmp_enable: ret = %d\r\n", ret);
        return ret;
    }
#endif /* LWIP_IGMP */

    g_upcb = udp_new();
    if (g_upcb == NULL)
    {
        LOG_PRINTF("udp_new fail\r\n");
        return -1;
    }

    ipMultiCast.addr = lwip_htonl(LISTEN_ADDR);   // 239.1.1.255

#if LWIP_IGMP
    ret = igmp_joingroup(IP_ADDR_ANY,(struct ip_addr *) (&ipMultiCast));
    if (ret != 0)
    {
        LOG_PRINTF("igmp_joingroup: ret = %d\r\n", ret);
        return ret;
    }
#endif

    ret = udp_bind(g_upcb, IP_ADDR_ANY, LISTEN_PORT);
    if (ret != 0)
    {
        LOG_PRINTF("udp_bind: ret = %d\r\n", ret);
        return ret;
    }

    udp_recv(g_upcb, ssv_test_udp_rx, (void *)0);

    LOG_PRINTF("ssv_test_udp_init ok\r\n");

    return ret;
}
#endif

