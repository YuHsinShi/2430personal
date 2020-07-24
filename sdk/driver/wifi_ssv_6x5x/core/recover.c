/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <txrx_hdl.h>
#include "recover.h"
#include "txrx_task.h"
#include <ssv_hal.h>
#include <ssv_timer.h>
#include <log.h>
#include <drv/ssv_drv.h>
#include <ssv_dev.h>
#include <ssv_hal.h>
#if (AP_MODE_ENABLE == 1)        
#include <ap_info.h>
#ifndef CONFIG_NO_WPA2
#include <wpa_auth.h>
#endif
#endif
#include <ap_drv_cmd.h>

extern struct Host_cfg g_host_cfg;
extern ssv_type_u8 ts_bk,tr_bk;
extern ssv6xxx_result TXRXTask_SetOpMode(ModeType mode);
extern int ssv6xxx_start(ssv_vif* vif);
extern void  _ssv6xxx_wifi_set_host_cfg(ssv_type_bool FromAPI);
extern ssv_type_s32 _ssv6xxx_set_ampdu_param(ssv_type_u8 opt, ssv_type_u8 value, ssv_type_u8 vif_idx, ssv_type_bool FromAPI);
extern void CmdEng_FlushPendingCmds(void);
extern HOST_API_STATE _ssv6xxx_wifi_host_api_state(const ssv_type_bool mutexLock);
extern ssv_type_bool ssv6xxx_beacon_enable( ssv_type_bool bEnable, ssv_type_bool bFreePbuf);
extern void StartBeacon(void);
extern void send_deauth_and_remove_all(void);

void ssv6xxx_wifi_ap_recover(ssv_type_u8 vif_idx)
{
#if (AP_MODE_ENABLE == 1)
    ssv_type_u16 operation_mode = host_to_le16(gDeviceInfo->APInfo->ht_op_mode);
    gDeviceInfo->recovering=TRUE;
    //LOG_PRINTF("ssv6xxx_wifi_ap_recover:%d \r\n",gDeviceInfo->reload_fw_cnt);
    /*************************************/
    //turn off tx queue
    /*************************************/
    //TXRXTask_TxLock(TRUE);
    TXRXTask_SetOpMode(MT_STOP);
    //spi init
    ssv6xxx_drv_init();

    /*************************************/
    //stop ap mode
    /*************************************/

    //clean beacon status
    ssv6xxx_beacon_enable(false,TRUE);

    //HCI stop
    ssv6xxx_drv_stop();

    //Disable HW
    ssv6xxx_HW_disable();

    ssv_hal_remove_interface(vif_idx);
    /*************************************/
    //reload and setting ap mode
    /*************************************/
    ssv_hal_chip_init(&gDeviceInfo->CliRes_info);
    //Load FW, init mac
    ssv6xxx_start(&gDeviceInfo->vif[vif_idx]);

    //update FW HT operation info
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_HT_PROTECT, &operation_mode, sizeof(ssv_type_bool),TRUE,FALSE);

    //update FW ERP info
    if (gDeviceInfo->APInfo->num_sta_non_erp >= 1)
    {
        //Send ERP protect cmd
        ssv_type_bool erp_protect = TRUE;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_ERP_PROTECT, &erp_protect, sizeof(ssv_type_bool),TRUE,FALSE);
    }
		//set slot time to register
    if (gDeviceInfo->APInfo->eCurrentApMode ==AP_MODE_IEEE80211G &&gDeviceInfo->APInfo->num_sta_no_short_slot_time >= 1)
        ssv_hal_set_short_slot_time(FALSE);
    else
        ssv_hal_set_short_slot_time(TRUE);


    //GenBeacon
	StartBeacon();
	ap_soc_set_bcn(SSV6XXX_SET_INIT_BEACON, gDeviceInfo->APInfo->bcn, &gDeviceInfo->APInfo->bcn_info, AP_DEFAULT_DTIM_PERIOD-1, g_host_cfg.bcn_interval, TRUE);

    //Enable HW
    TXRXTask_SetOpMode(MT_RUNNING);
    ssv6xxx_HW_enable();

    //Re-set rate control related values
    _ssv6xxx_wifi_set_host_cfg(FALSE);
    ssv6xxx_wifi_set_tx_pwr_mode(g_host_cfg.tx_power_mode);

    //ampdu tx
    _ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_ENABLE,g_host_cfg.ampdu_tx_enable,vif_idx,FALSE);
    //ampdu rx
    _ssv6xxx_set_ampdu_param(AMPDU_RX_OPT_ENABLE,g_host_cfg.ampdu_rx_enable,vif_idx,FALSE);
    //ampdu rx aggr max
    _ssv6xxx_set_ampdu_param(AMPDU_RX_OPT_BUF_SIZE,g_host_cfg.ampdu_rx_buf_size,vif_idx,FALSE);
    ts_bk = g_host_cfg.tx_sleep;
    tr_bk = g_host_cfg.tx_retry_cnt;

    /*************************************/
    //remove station
    /*************************************/
    send_deauth_and_remove_all();

#ifndef CONFIG_NO_WPA2
    /*************************************/
    //WPA2 Related
    /*************************************/
    wpa_auth_recovery();

#endif
    /*************************************/
    //turn on tx queue
    /*************************************/
    //TXRXTask_TxLock(FALSE);

    gDeviceInfo->reload_fw_cnt ++;
    gDeviceInfo->recovering=FALSE;
    //OS_ExitCritical(CPUSR);
#endif
}

void ssv6xxx_wifi_sta_recover(ssv_type_u8 vif_idx)
{

    ssv_type_u32 i;//,CPUSR;
    if(gDeviceInfo->recovering==TRUE){
        LOG_PRINTF("STA is in the recovery phase\r\n");
        return;
    }
    //CPUSR = OS_EnterCritical();
    gDeviceInfo->recovering = TRUE;

    /*************************************/
    //turn off tx queue
    /*************************************/
    //TXRXTask_TxLock(TRUE);
    TXRXTask_SetOpMode(MT_STOP);
    //spi init
    ssv6xxx_drv_init();

    /*************************************/
    //stop sta mode
    /*************************************/
    //HCI stop
    ssv6xxx_drv_stop();

    //Disable HW
    ssv6xxx_HW_disable();

    ssv_hal_remove_interface(vif_idx);
    /*************************************/
    //reload and setting ap mode
    /*************************************/
    ssv_hal_chip_init(&gDeviceInfo->CliRes_info);

    //Load FW, init mac
    ssv6xxx_start(&gDeviceInfo->vif[vif_idx]);

    //Enable HW
    TXRXTask_SetOpMode(MT_RUNNING);
    ssv6xxx_HW_enable();

    //Re-set rate control related values
    _ssv6xxx_wifi_set_host_cfg(FALSE);
    ssv6xxx_wifi_set_tx_pwr_mode(g_host_cfg.tx_power_mode);

    //ampdu tx
    _ssv6xxx_set_ampdu_param(AMPDU_TX_OPT_ENABLE,g_host_cfg.ampdu_tx_enable,vif_idx,FALSE);
    //ampdu rx
    _ssv6xxx_set_ampdu_param(AMPDU_RX_OPT_ENABLE,g_host_cfg.ampdu_rx_enable,vif_idx,FALSE);
    //ampdu rx aggr max
    _ssv6xxx_set_ampdu_param(AMPDU_RX_OPT_BUF_SIZE,g_host_cfg.ampdu_rx_buf_size,vif_idx,FALSE);
    ts_bk = g_host_cfg.tx_sleep;
    tr_bk = g_host_cfg.tx_retry_cnt;

    //Unlock the blocking status in host_cmdeng
    CmdEng_FlushPendingCmds();

    //Unlock the blocking status in up layer(netmgr_task)
    for(i=0;i<HOST_RECOVERY_CB_NUM;i++){
        if(gDeviceInfo->recovery_cb[i]!=NULL){
            gDeviceInfo->recovery_cb[i]();
        }
    }

    /*************************************/
    //Re -join
    /*************************************/
    if(gDeviceInfo->vif[vif_idx].m_info.StaInfo->status==CONNECT){
        ssv6xxx_wifi_join(gDeviceInfo->vif[vif_idx].m_info.StaInfo->joincfg);
        //gDeviceInfo->status can't recover here, the status should recover after getting the JOIN_RESULT
    }
    else{
        gDeviceInfo->recovering=FALSE;
    }

    gDeviceInfo->reload_fw_cnt++ ;
    //OS_ExitCritical(CPUSR);

}

ssv_type_s32 ssv6xxx_wifi_get_recover_cnt(void)
{
    return gDeviceInfo->reload_fw_cnt;
}

extern struct task_info_st g_host_task_info[];
extern ssv_type_u32 g_hw_enable;
extern ssv_type_u32 chip_interrupt;
extern ssv_type_bool reset_host_init(void);

void check_watchdog_timer(void *data1, void *data2)
{
    ssv_type_u32 now_time,i;
    ssv_type_u32 trx_time;
    HOST_API_STATE host_api;
    now_time =OS_TICK2MS(OS_GetSysTick());
    trx_time= ssv6xxx_drv_get_TRX_time_stamp();
    host_api=_ssv6xxx_wifi_host_api_state(FALSE);
    if(((now_time-trx_time)>IPC_CHECK_TIMER)&&((now_time-chip_interrupt)>IPC_CHECK_TIMER)&& host_api == HOST_API_ACTIVE)
    {
        //LOG_PRINTF("check_watchdog_timer:%d;%d;%d\r\n",now_time,trx_time,chip_interrupt);
        if(TRUE==ssv6xxx_drv_detect_card())
        {
            if(TRUE==ssv_hal_get_diagnosis())
            {
                ssv_type_u32 fw_status = 0;
                ssv_hal_get_fw_status(&fw_status);

                //LOG_PRINTF("***********************************************g_hw_enable:%d fw_status:%08x\r\n", g_hw_enable, fw_status);
                if(fw_status == 0x5A5AA5A5 ||g_hw_enable == false){
                    //We need to enable the irq again here, because irq will be disabled in ssv6xxx_drv_detect_card;
                    //If the system really need to recover, irq will be enabled in recover function.
                    ssv6xxx_drv_irq_enable(FALSE);

                    os_create_timer(IPC_CHECK_TIMER,check_watchdog_timer,NULL,NULL,(void*)TIMEOUT_TASK);
                    return;
                }
                ssv_hal_reset_soc_irq(); //  system reset interrupt for host

                LOG_PRINTF("Bark! Bark! Bark!\r\n");
                for(i=0;i<MAX_VIF_NUM;i++)
                {
                    if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_AP){
                        ssv6xxx_wifi_ap_recover(i);
                    }
                    else if(gDeviceInfo->vif[i].hw_mode == SSV6XXX_HWM_STA)
                    {
                        ssv6xxx_wifi_sta_recover(i);
                    }
                }
                return;
            }

            //We need to enable the irq again here, because irq will be disabled in ssv6xxx_drv_detect_card;
            //If the system really need to recover, irq will be enabled in recover function.
            ssv6xxx_drv_irq_enable(FALSE);

        }

    }

    os_create_timer(IPC_CHECK_TIMER,check_watchdog_timer,NULL,NULL,(void*)TIMEOUT_TASK);
}

void recovery_pause_resume(ssv_type_bool resume)
{
#if(RECOVER_MECHANISM == 1)
    ssv_hal_pause_resuem_recovery_int(resume);
#endif
}
