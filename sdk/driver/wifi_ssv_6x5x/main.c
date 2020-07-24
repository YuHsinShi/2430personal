/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <ssv_types.h>
#include <msgevt.h>
#include <pbuf.h>
#include <cmd_def.h>
#include <log.h>
#if CLI_ENABLE
#include <cli.h>
#include <cmds/cli_cmd_wifi.h>
#endif
#include <net_mgr.h>
#include <drv/ssv_drv.h>
#include <host_apis.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
#include "ite/itp.h" //ioctl

/* Macro */
#define Join_After_Scan 1
#define cmd_size_max    4

/* Cmd list */
static char* iw_cmd[][cmd_size_max] = {
    {"iw", "scan",          "0xFFFF",       "0xFFFFFFFF"},  /*0*/
    {"iw", "list",          NULL,           NULL},          /*1*/
    {"iw", "join",          "IOT_PZ_2072",  "12345678"},    /*2*/
    {"iw", "join-other",    "IOT_PZ_2072",  "12345678"},    /*3*/
    {"iw", "leave",         NULL,           NULL},
    {}
};

char*  ctl_cmd[][cmd_size_max] = {
    {"ctl", "status",       NULL,           NULL},          /*0*/
    {}
};


/**
 *  Entry point of the firmware code. After system booting, this is the
 *  first function to be called from boot code. This function need to 
 *  initialize the chip register, software protoctol, RTOS and create
 *  tasks. Note that, all memory resources needed for each software
 *  modulle shall be pre-allocated in initialization time.
 */
/* return int to avoid from compiler warning */

#if CONFIG_PLATFORM_CHECK
//big   : 0x01 0x23 0x45 0x67
//little: 0x67 0x45 0x23 0x01
static void ssv6xxx_plform_chk()
{
    volatile ssv_type_u32 i=0x01234567;
    ssv_type_u8 *ptr;
    ssv_type_u16* ptr_u16;
    ssv_type_bool b_le = FALSE;
    // return 0 for big endian, 1 for little endian.
    LOG_PRINTF("============================================================\r\n");
    LOG_PRINTF("\t\t Platorm Check\r\n");
    LOG_PRINTF("************************************************************\r\n");

    if(((*((ssv_type_u8*)(&i))) == 0x67)){
        LOG_PRINTF("Little Endian\r\n");
        b_le=TRUE;
    }else{
        LOG_PRINTF("Big Endian\r\n");
    }
    ptr = (ssv_type_u8*)&i;
    LOG_PRINTF("Addr[%p] for i\r\n", ptr);
    
    ptr++;        
    ptr_u16 = (ssv_type_u16*)ptr;

    if(*ptr_u16 == 0x2345)
        LOG_PRINTF("Addr[%p] Support odd alignment\r\n", ptr_u16);
    else
        LOG_PRINTF("Addr[%p] NOT Support odd alignment\r\n", ptr_u16);
    
    ptr++;
    ptr_u16 = (ssv_type_u16*)ptr;
    
    if(b_le){
        if(*ptr_u16 == 0x0123)
            LOG_PRINTF("Addr[%p] Support 2 byte alignment\r\n", ptr_u16);
        else
            LOG_PRINTF("Addr[%p] NOT Support 2 byte alignment\r\n", ptr_u16);
            
    }else{
        
        if(*ptr_u16 == 0x4567)
            LOG_PRINTF("Addr[%p] Support 2 byte alignment\r\n", ptr_u16);
        else
            LOG_PRINTF("Addr[%p] NOT Support 2 byte alignment\r\n", ptr_u16);

    }

    LOG_PRINTF("============================================================\r\n");

}
#endif//CONFIG_PLATFORM_CHECK
extern int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode);

/* Add from Eason */
/* ================= ====================*/
static void ssv6xxx_status_info(void)
{
    Ap_sta_status   info;
    ssv_type_u8     ssid_buf[MAX_SSID_LEN+1]={0};
    ssv_type_u32    i;

    OS_MemSET(&info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&info);
    if(info.status)
        LOG_PRINTF("status:ON\r\n");
    else
        LOG_PRINTF("status:OFF\r\n");
    for(i=0;i<MAX_VIF_NUM;i++)
    {
        LOG_PRINTF("\r\nMode:%s, %s\r\n",(SSV6XXX_HWM_STA==info.vif_operate[i])?"Station":"Sconfig",(info.vif_u[i].station_status.apinfo.status == CONNECT) ? "connected" :"disconnected");
        LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            info.vif_u[i].station_status.selfmac[0],
            info.vif_u[i].station_status.selfmac[1],
            info.vif_u[i].station_status.selfmac[2],
            info.vif_u[i].station_status.selfmac[3],
            info.vif_u[i].station_status.selfmac[4],
            info.vif_u[i].station_status.selfmac[5]);
        OS_MemSET(ssid_buf,0,sizeof(ssid_buf));
        OS_MemCPY((void*)ssid_buf,(void*)info.vif_u[i].station_status.ssid.ssid,info.vif_u[i].station_status.ssid.ssid_len);
        LOG_PRINTF("SSID:%s\r\n",ssid_buf);
        LOG_PRINTF("channel:%d\r\n",info.vif_u[i].station_status.channel);
        LOG_PRINTF("AP Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            info.vif_u[i].station_status.apinfo.Mac[0],
            info.vif_u[i].station_status.apinfo.Mac[1],
            info.vif_u[i].station_status.apinfo.Mac[2],
            info.vif_u[i].station_status.apinfo.Mac[3],
            info.vif_u[i].station_status.apinfo.Mac[4],
            info.vif_u[i].station_status.apinfo.Mac[5]);
        if(ssv_is_valid_ether_addr(info.vif_u[i].station_status.apinfo.Mac)){
            LOG_PRINTF("RSSI = -%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac(info.vif_u[i].station_status.apinfo.Mac, i));
        }
        else{
            LOG_PRINTF("RSSI = 0 (dBm)\r\n");
        }
    }
}

static void ssv6xxx_join_ap(char* join_info[], ssv_type_bool join_after_scan)
{
    wifi_sta_join_cfg *join_cfg = NULL;

    join_cfg = (wifi_sta_join_cfg *)OS_MemAlloc(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }
    OS_MemSET((void *)join_cfg,0,sizeof(wifi_sta_join_cfg));

    join_cfg->vif_idx = MAX_VIF_NUM;
    join_cfg->ssid.ssid_len=ssv6xxx_strlen(join_info[0]);
    ssv6xxx_strcpy((char *)join_cfg->ssid.ssid, join_info[0]);
    ssv6xxx_strcpy((char *)join_cfg->password,  join_info[1]);

    if (join_after_scan){
        netmgr_wifi_scan_async(SCAN_ALL_2G_CHANNEL, NULL, 0); //2.4G

        usleep(500*1000);
        netmgr_wifi_join_async(join_cfg);
    } else
    netmgr_wifi_join_other_async(join_cfg);
}
/* ================= ====================*/


#ifdef __MTRACE_CHECK__
//Check memory leak tool
#include <mcheck.h>

#endif//#ifdef __MTRACE_CHECK__
OsTaskHandle dev_init_task_handle;

void dev_init_task(void *args)
{
        ssv6xxx_dev_init(SSV6XXX_HWM_STA);
#if CONFIG_PLATFORM_CHECK
        ssv6xxx_plform_chk();
#endif//CONFIG_PLATFORM_CHECK
        OS_TaskDelete(dev_init_task_handle);


}
#define DEV_INIT_PRIORITY   2

struct task_info_st dev_init_task_info[] =
{
    { "dev_init_task",  (OsMsgQ)0, 1, DEV_INIT_PRIORITY, DEV_INIT_STACK_SIZE, NULL, dev_init_task},
};

/************************************************************************************************************************************************************************
*
*   This main function just use in FREERTOS """"""SIM"""" platform
*   other platform need to call the function,ssv6xxx_dev_init to initialize wifi module
*   in the customer's task. 
*   Note: ssv6xxx_dev_init need to be call after os starting schedule(this function include some os commands inside)
*
************************************************************************************************************************************************************************/
ssv_type_s32 ssv_main(ssv_type_s32 argc, char *argv[])
{
    ssv_type_s32 task_ret = OS_FAILED;

#ifdef __MTRACE_CHECK__
	//log memroy heap status
	mtrace();
#endif//#ifdef __MTRACE_CHECK__

    task_ret = OS_TaskCreate(dev_init_task_info[0].task_func,
              dev_init_task_info[0].task_name,
              dev_init_task_info[0].stack_size<<4,
              NULL,
              dev_init_task_info[0].prio,
              &dev_init_task_handle);

	/* Start the scheduler so our tasks start executing. */
    OS_StartScheduler();   

    return task_ret;
}

/* Ioctl for WIFI base functions */
static int WifiSsvIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
#if CLI_ENABLE
    	case ITP_IOCTL_SCAN:
    	{
    	    /* scan and get list*/
    	    cmd_iw(cmd_size_max,     iw_cmd[0]);
    	    usleep(500*1000);
    	    cmd_iw(cmd_size_max - 2, iw_cmd[1]);
        }
        break;


        case ITP_IOCTL_GET_TABLE:
            cmd_iw(cmd_size_max - 2, iw_cmd[1]);
        break;


        case ITP_IOCTL_WIFI_LINK_AP:
            if (Join_After_Scan) {
                cmd_iw(cmd_size_max, iw_cmd[0]);

            usleep(500*1000);
                cmd_iw(cmd_size_max, iw_cmd[2]);
            } else
                cmd_iw(cmd_size_max, iw_cmd[3]);
        break;


        case ITP_IOCTL_EXIT:
            cmd_iw(cmd_size_max - 2, iw_cmd[3]);
        break;


        case ITP_IOCTL_GET_INFO:
            cmd_ctl(cmd_size_max,    ctl_cmd[0]);
        break;


        case ITP_IOCTL_IS_AVAIL:
            return netmgr_wifi_is_connected(SSV6XXX_HWM_STA);

#else
        case ITP_IOCTL_SCAN:
            netmgr_wifi_scan_async(SCAN_ALL_2G_CHANNEL, NULL, 0); //2.4G
        break;


        case ITP_IOCTL_WIFI_LINK_AP:
        {
            char* join_info[] = {"IOT_PZ_2072", "12345678"};
            ssv6xxx_join_ap(join_info, Join_After_Scan);
        }
        break;


        case ITP_IOCTL_GET_INFO:
            ssv6xxx_status_info();
        break;


        case ITP_IOCTL_IS_AVAIL:
            return netmgr_wifi_is_connected(SSV6XXX_HWM_STA);

#endif
    }
}


const ITPDevice itpDeviceWifiSsv =
{
    ":SSV SDIO Wifi",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    WifiSsvIoctl,
    NULL
};

