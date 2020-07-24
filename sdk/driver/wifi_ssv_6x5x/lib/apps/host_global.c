/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <ssv_devinfo.h>


/*  Global Variables Declaration: */
//ETHER_ADDR WILDCARD_ADDR = { {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };

ssv_type_u32 g_free_msgevt_cnt;


/* Task Count to indicate if all tasks are running. */
ssv_type_u32 g_RunTaskCount;
//u32 g_MaxTaskCount;

//u8 g_sta_mac[6];


//#include <sim_regs.h>
//#include <security/drv_security.h>


//extern SRAM_KEY			*pGSRAM_KEY;

/* For cli_cmd_soc commands: */
//s32 g_soc_cmd_rx_ready = 0;
//char g_soc_cmd_rx_buffer[1024];


//u32 g_sim_net_up = 0;
//u32 g_sim_link_up = 0;

DeviceInfo_st *gDeviceInfo=NULL;
struct rcs_info tx_rcs;
OsMutex txsrcMutex;

ssv_type_u32 gMaxAID;
ssv_type_u32 gMaxTimLen;
//debug
#if CONFIG_STATUS_CHECK
ssv_type_u32 g_l2_tx_packets;
ssv_type_u32 g_l2_tx_copy;
ssv_type_u32 g_l2_tx_late;
ssv_type_u32 g_notpool;
ssv_type_u32 g_heap_used;
ssv_type_u32 g_heap_max;
ssv_type_u32 g_dump_tx;
ssv_type_u32 g_tx_allocate_fail;
#endif

#ifdef __SSV_UNIX_SIM__
OsMutex			g_wsimp_mutex;
#endif
OsMutex ssv_frame_mutex;

void host_global_init(void)
{

#if CONFIG_STATUS_CHECK    
    g_l2_tx_packets = 0;
    g_l2_tx_copy=0;
    g_l2_tx_late=0;
	g_notpool=0;
    g_heap_used=0;
    g_heap_max=0;
    g_dump_tx = 0;
    g_tx_allocate_fail = 0;
#endif 

//    OS_MutexInit(&g_hcmd_blocking_mutex);
    g_RunTaskCount = 0;
//    g_MaxTaskCount = 0;

#ifdef __SSV_UNIX_SIM__
	OS_MutexInit(&g_wsimp_mutex);
#endif	
    OS_MutexInit(&ssv_frame_mutex,"ssv_frame_mutex");
}

void host_global_deinit(void)
{
#ifdef __SSV_UNIX_SIM__
    OS_MutexDelete(g_wsimp_mutex);
#endif	
    OS_MutexDelete(ssv_frame_mutex);
}

