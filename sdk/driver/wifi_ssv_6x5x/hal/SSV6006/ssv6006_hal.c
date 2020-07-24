#define SSV6006_HAL_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <rtos.h>
#include <log.h>
#include <ssv_devinfo.h>
#include <ssv_dev.h>
#include <ssv_drv.h>
#include <ssv_ether.h>
#include <ssv_lib.h>
#include <cmd_def.h>
#if (AP_MODE_ENABLE == 1)        
#include <ap_def.h>
#include <ap_config.h>
#include <ap_info.h>
#endif

#include "../ssv_hal_if.h"
#include "ssv6006_tbl.h"
#include "ssv6006_hal.h"
#include "ssv6006_decision_tbl.h"
#include "ssv6006_data_flow.h"
#include "ssv6006_beacon.h"
#include "ssv6006_efuse.h"
#include "ssv6006_pkt.h"
#include "ssv6006_pktdef.h"
#include <hctrl.h>
#include "turismoB_rf_reg.h"
#include "turismoB_wifi_phy_reg.h"
#include "turismoC_rf_reg.h"
#include "turismoC_wifi_phy_reg.h"
#include "turismoC_wifi_mac_reg.h"
#include "turismoC_wifi_rf_cal_reg.h"
#include "turismo_common.h"
#define RENTENION_SRAM          0x200000
#define FW_STATUS_REG           SSV6006_FW_STATUS_REG
#define FW_STATUS_SIZE          (0xF)
#define FW_BUF_SIZE             (FW_BLOCK_SIZE)
#define FW_STATUS_MASK          (FW_STATUS_SIZE)<<28
#define FW_BLOCK_CNT_SIZE       (0xFFF)                     //4096*FW_BLOCK_SIZE  support 4MB FW
#define FW_CHK_SUM_SIZE         (FW_BLOCK_CNT_SIZE)           //4096*FW_BLOCK_SIZE  support 4MB FW
#define FW_MASK                 (0xFFFF<<16)
#define FW_BLOCK_CNT_MASK       (FW_BLOCK_CNT_SIZE)<<16
#define FW_CHK_SUM_MASK         (FW_CHK_SUM_SIZE)<<16
#define FW_STATUS_FW_CHKSUM_BIT      (1<<31)
#define FW_STATUS_HOST_CONFIRM_BIT   (1<<30)

#define ENABLE_FW_SELF_CHECK
#define FW_BLOCK_SIZE                   (1024)
#define FW_CHECKSUM_INIT                (0x12345678)

// ToDo Liam : replace it from header file.
#define RG_GPIO07_OE 1 << 30
#define RG_GPIO06_OE 1 << 26
#define RG_GPIO05_OE 1 << 22
#define RG_GPIO04_OE 1 << 18
#define RG_GPIO03_OE 1 << 14

#define RG_FPGA_CLK_REF_40M_SFT 30
#define RG_FPGA_CLK_REF_40M_MSK 0x40000000
#define RG_PAD_MUX_SEL_SFT 8
#define RG_PAD_MUX_SEL_MSK 0x00000f00


//===================HW related=============================


struct ssv6006_hci_txq_info {
	ssv_type_u32 tx_use_page:8;
    ssv_type_u32 tx_use_id:6;
    ssv_type_u32 txq0_size:4;
	ssv_type_u32 txq1_size:4;
	ssv_type_u32 txq2_size:5;
	ssv_type_u32 txq3_size:5;
};

OsSemaphore tx_loopback;
struct cfg_tx_loopback_info tx_loopback_info;
extern struct Host_cfg g_host_cfg;
extern struct ssv6xxx_beacon_info hw_bcn_info[];
extern ssv_type_s32 _ssv6xxx_wifi_send_cmd_directly(void *pCusData, int nCuslen, ssv6xxx_host_cmd_id eCmdID);
int ssv6006_hal_apply_ap_deci_tbl(void);
static ssv_type_bool _ssv6006_do_firmware_checksum(ssv_type_u32 origin);
static ssv_type_bool _ssv6006_hal_mcu_input_full(void);
static ssv_type_bool _ssv6006_set_hw_table(const ssv_cabrio_reg tbl_[], ssv_type_u32 size);
static int _ssv6006_hal_init_mac(ssv_type_u8 *self_mac, ssv_type_bool fullInit);
static int _ssv6006_hal_init_ap_mac(ssv_type_u8 *bssid,ssv_type_u8 channel, ssv_type_bool fullInit);
static int _ssv6006_hal_init_sta_mac(ssv_type_u32 wifi_mode, ssv_type_bool fullInit);
//static void ssv6006_geminiA_init_cali (void);
//static void ssv6006_geminiA_init_geminiA_trx();
#if(CONFIG_CHIP_ID==SSV6006C)
// Reset CPU (after reset, CPU is stopped)
static int _ssv6006_reset_cpu(void)
{    
    // Keep original interrupt mask
    ssv_type_u32 org_int_mask = GET_MASK_TYPMCU_INT_MAP;
    ssv_type_u32 cnt = 0;

      
    /* Safly reset CPU: (Precondition: CPU must be alive)
     * Through sysctrl to make CPU enter standby first, then do CPU reset. 
     */
    if (GET_RESET_N_CPUN10) {
        // Mask all interrupt for CPU, except SYSCTRL interrupt
        SET_MASK_TYPMCU_INT_MAP(0xffffdfff);        
        // Request CPU enter standby through SYSCTRL COMMAND
        SET_SYSCTRL_CMD(0x0000000e);

        // Confirm if N10 enter standby
        while(!GET_N10_STANDBY) {
            cnt++;
            // 1 ms checking time limit
            if (cnt > 10) {
                LOG_PRINTF("Reset CPU failed! CPU can't enter standby\n");
                return -1;
            }
        }
    }
    
    // Reset CPU
    SET_RESET_N_CPUN10(0);
    // Set original interrupt mask back
    SET_MASK_TYPMCU_INT_MAP(org_int_mask);

    return 0;
}

//SRAM mode selection
enum SSV_SRAM_MODE{
    SRAM_MODE_ILM_64K_DLM_128K = 0,
    SRAM_MODE_ILM_160K_DLM_32K
};
static void _ssv6006_set_sram_mode(enum SSV_SRAM_MODE mode)
{
    //TODO: wait csr to replace raw value of register 
    switch (mode) {
        case SRAM_MODE_ILM_64K_DLM_128K:
            MAC_REG_SET_BITS(0xc0000128, (0<<1), ~0x2);            
            break;
        case SRAM_MODE_ILM_160K_DLM_32K:
            MAC_REG_SET_BITS(0xc0000128, (1<<1), ~0x2);
            break;
    }
}
#endif

static void _ssv6006_load_fw_enable_mcu(void)
{

    #if(CONFIG_CHIP_ID==SSV6006C)
    // After FW loaded, set IVB to 0, boot from SRAM, enable N10 clock, and release N10
    SET_N10CFG_DEFAULT_IVB(0);
 
    SET_CLK_EN_CPUN10(1);
    SET_RESET_N_CPUN10(1); // N10 might be disabled by default. Enable it.     

    #elif (CONFIG_CHIP_ID==SSV6006B)
    // After FW loaded, set IVB to 0, boot from SRAM, enable N10 clock, and release N10
    SET_N10CFG_DEFAULT_IVB(0);

    SET_ROM_REBOOT_FROM_SRAM(1);
    SET_MCU_CLK_EN(1);
    SET_CLK_EN_CPUN10(1);
    SET_RESET_N_CPUN10(1); // N10 might be disabled by default. Enable it.
    SET_N10_WARM_RESET_N(1); // Then release the warm reset.
    #else
    error!!
    #endif
}

static int _ssv6006_load_fw_disable_mcu(void)
{
    #if(CONFIG_CHIP_ID==SSV6006C)
    int ret = 0;
    // Before loading FW, reset N10
    if (_ssv6006_reset_cpu() != 0)
        return -1;

    SET_CLK_EN_CPUN10(0);

    SET_MCU_ENABLE(0);
    SET_RG_REBOOT(1);

    return ret;    
    #elif (CONFIG_CHIP_ID==SSV6006B)
    // Reset CPU

    // Before loading FW, reset N10
    SET_RESET_N_CPUN10(0);
    //SET_N10_WARM_RESET_N(0);
    SET_CLK_EN_CPUN10(0);
    SET_MCU_CLK_EN(0);

    SET_MCU_ENABLE(0);
    SET_RG_REBOOT(1);
    return 0;
    #else
    error!!
    #endif
}
static ssv_type_bool _ssv6006_set_hw_table(const ssv_cabrio_reg tbl_[], ssv_type_u32 size)
{
    ssv_type_bool ret = FALSE ;
    ssv_type_u32 i=0;
    for(; i<size; i++) {
        ret = MAC_REG_WRITE(tbl_[i].address, tbl_[i].data);
        if (ret==FALSE) break;
        OS_MsDelay(1);
    }
    return ret;
}

static void _ssv6006_set_phy_mode(ssv_type_bool val)
{
    if (val) { // set phy mode on without enable
        MAC_REG_WRITE(ADR_WIFI_PHY_COMMON_ENABLE_REG,(RG_PHYRX_MD_EN_MSK | RG_PHYTX_MD_EN_MSK |
            RG_PHY11GN_MD_EN_MSK | RG_PHY11B_MD_EN_MSK | RG_PHYRXFIFO_MD_EN_MSK |
            RG_PHYTXFIFO_MD_EN_MSK | RG_PHY11BGN_MD_EN_MSK));
    } else { //clear phy mode
        MAC_REG_WRITE(ADR_WIFI_PHY_COMMON_ENABLE_REG, 0x00000000);
    }
}

int ssv6006_hal_l2_on(ssv_type_u8 vif_idx)
{
    
    hw_bcn_info[0].pubf_addr=((0x80000000 | (GET_MTX_BCN_PKT_ID0 << 16)));
    hw_bcn_info[1].pubf_addr=((0x80000000 | (GET_MTX_BCN_PKT_ID1 << 16)));
    LOG_PRINTF("Beacon:0x%x\r\n",hw_bcn_info[0].pubf_addr);
    LOG_PRINTF("Beacon:0x%x\r\n",hw_bcn_info[1].pubf_addr);    

#if 0    
    if(gDeviceInfo->vif[vif_idx].hw_mode==SSV6XXX_HWM_AP) 
    {
        //Restore the ap's decision table
        SET_RG_PHY_MD_EN(0);
        ssv6006_hal_apply_ap_deci_tbl();        
        SET_RG_PHY_MD_EN(1);
    }
#endif

#if 0
    if(0!=_ssv6006_hal_init_mac(gDeviceInfo->self_mac, FALSE))
    {
        return -1;
    }

    if(SSV6XXX_HWM_AP == gDeviceInfo->hw_mode)
    {
        if(0!=_ssv6006_hal_init_ap_mac(gDeviceInfo->self_mac,gDeviceInfo->APInfo->nCurrentChannel,FALSE))
            return -1;
    }
    else
    {
        if(0!=_ssv6006_hal_init_sta_mac(SSV6XXX_HWM_STA, FALSE))
            return -1;
    }
#endif

    return 0;
}

int ssv6006_hal_apply_null_deci_tbl(void);
struct ssv6006_cal_result cal_res;
int ssv6006_hal_l2_off(ssv_type_u8 vif_idx)
{
    ssv_type_u32 size=0;
    ssv_type_u32 addr=RENTENION_SRAM;
    ssv_type_u32 saddr=RENTENION_SRAM;
    ssv_type_u32 i=0;
#if 0
    //Total size. unit is word
    size=(sizeof(ssv6006_turismoC_rf_setting)>>2)+
        (sizeof(ssv6006_turismoC_phy_setting)>>2)+
        (sizeof(rf_calibration_result)>>2)+
        (sizeof(mac_restore_tbl)>>2)+
        2*2; //2 is  0xccb0e000 and 0xccb0e004
#endif    
    LOG_PRINTF("rf(%d) phy(%d) cal(%d) mac(%d)\r\n",sizeof(ssv6006_turismoC_rf_setting)
                                                    ,sizeof(ssv6006_turismoC_phy_setting)
                                                    ,sizeof(rf_calibration_result)
                                                    ,sizeof(mac_restore_tbl));
    MAC_REG_WRITE(saddr,size);
    addr+=4;

    //save the rf table
    size=(sizeof(ssv6006_turismoC_rf_setting)>>2);
    for(i=0;i<(sizeof(ssv6006_turismoC_rf_setting)>>(2+1));i++)
    {
        MAC_REG_WRITE(addr,ssv6006_turismoC_rf_setting[i].address);
        MAC_REG_WRITE(addr+4,ssv6006_turismoC_rf_setting[i].data);
        addr+=8;
    }

    
#if 1
    //Read and save rf calibration results
    size+=(sizeof(rf_calibration_result)>>2);
    for(i=0;i<(sizeof(rf_calibration_result)>>(2+1));i++)
    {
        MAC_REG_READ(rf_calibration_result[i].address,rf_calibration_result[i].data);
        MAC_REG_WRITE(addr,rf_calibration_result[i].address);
        MAC_REG_WRITE(addr+4,rf_calibration_result[i].data);
        addr+=8;
    }
#endif
    //save the phy table
    MAC_REG_WRITE(addr,0xccb0e000); //address
    MAC_REG_WRITE(addr+4,0x80010000); //value
    size+=2;
    addr+=8;
    
    size+=(sizeof(ssv6006_turismoC_phy_setting)>>2);
    for(i=0;i<(sizeof(ssv6006_turismoC_phy_setting)>>(2+1));i++)
    {
        MAC_REG_WRITE(addr,ssv6006_turismoC_phy_setting[i].address);
        MAC_REG_WRITE(addr+4,ssv6006_turismoC_phy_setting[i].data);
        addr+=8;
    }

    MAC_REG_WRITE(addr,0xccb0e004); //address
    MAC_REG_WRITE(addr+4,0x17F); //value
    size+=2;
    addr+=8;

    //save the mac restore tble
    
    if(gDeviceInfo->vif[vif_idx].hw_mode==SSV6XXX_HWM_AP) 
    {
        //Reject the all rx data packets during power saving
        ssv6006_hal_apply_null_deci_tbl();
    }

    size+=(sizeof(mac_restore_tbl)>>2);
    for(i=0;i<(sizeof(mac_restore_tbl)>>(2+1));i++)
    {
        MAC_REG_READ(mac_restore_tbl[i].address,mac_restore_tbl[i].data);
        MAC_REG_WRITE(addr,mac_restore_tbl[i].address);
        MAC_REG_WRITE(addr+4,mac_restore_tbl[i].data);
        addr+=8;
    }

#if 0
    {
        u32 cal_len_4byte=0;
        u32 *pCalRes = (u32*)(&cal_res);
        cal_len_4byte = (sizeof(struct ssv6006_cal_result)/4);
        if((sizeof(struct ssv6006_cal_result)%4)!=0)
            cal_len_4byte+1;
        
        size+=cal_len_4byte;
        for(i=0;i<cal_len_4byte;i++)
        {
            MAC_REG_WRITE(addr,*(pCalRes+i));
            addr+=4;
        }
    }
#endif    
    //save the real size in the first word
    MAC_REG_WRITE(saddr,size);

    //save the ap's real decision tbl
    //fw apply the real's decision tbl when host wakeup wifi
    if(gDeviceInfo->vif[vif_idx].hw_mode==SSV6XXX_HWM_AP) 
    {
        size=((SSV6006_MAC_DECITBL1_SIZE+SSV6006_MAC_DECITBL2_SIZE)*2); 
        //LOG_PRINTF("addr(0x%x),size(0x%x)\r\n",addr,size);
        MAC_REG_WRITE(addr,size);
        addr+=4;
        
        for(i=0;i<((SSV6006_MAC_DECITBL1_SIZE+SSV6006_MAC_DECITBL2_SIZE));i++)
        {  
            MAC_REG_WRITE(addr,ADR_MRX_FLT_TB0+i*4);
            MAC_REG_WRITE(addr+4,ap_deci_tbl[i]);
            addr+=8;
        }
    }
    return 0;
}

extern int dump_rf_table(void);
extern struct st_rf_table def_rf_table;
extern ssv_type_bool platform_download_firmware(void);
int _ssv6006_hal_rf_phy_init(void)
{
   
    ssv_type_u32 regval=0;
    ssv_type_u8 data[4]={0}; 
    struct cfg_rf_phy_init init_tbl;
    //dump_rf_table();
    _ssv6006_set_phy_mode(FALSE); 
    if(platform_download_firmware() == FALSE)
        return -1;
    //check if all fw tasks are all ready or not
    do{
        MAC_REG_READ(FW_STATUS_REG,regval);
    }while(regval!=0x5A5AA5A5);

    OS_MemSET((void *)&init_tbl, 0, sizeof(struct cfg_rf_phy_init));
    
    switch(g_host_cfg.xtal)
    {
        case XTAL_24M:
            init_tbl.xtal=24;
            break;
        case XTAL_25M:
            init_tbl.xtal=25;            
            break;            
        case XTAL_26M:
            init_tbl.xtal=26;                        
            break;            
        case XTAL_40M:
            init_tbl.xtal=40;                        
            break;       
        default:
            init_tbl.xtal=24;
            break;
    }
    
    if(g_host_cfg.support_rf_band==RF_5G_BAND)
    {
        init_tbl.support_5g=1;
    }
    else
    {
        init_tbl.support_5g=0;
    }

    if(g_host_cfg.volt_mode==VOLT_DCDC_CONVERT)
    {
        init_tbl.dcdc=1;
    }
    else
    {
        init_tbl.dcdc=0;
    }

    OS_MemCPY((void *)&init_tbl.rf_tbl, (void *)&def_rf_table, sizeof(struct st_rf_table));
    
    MAC_REG_WRITE(FW_STATUS_REG, 0);
    _ssv6xxx_wifi_send_cmd_directly((void *)&init_tbl, sizeof(struct cfg_rf_phy_init), SSV6XXX_HOST_CMD_RF_PHY_INIT);
    
    do{
        MAC_REG_READ(FW_STATUS_REG,regval);
    }while(regval!=0x88888888);
    //Recover xstal_fo

    _ssv6006_set_phy_mode(TRUE); 
    SET_RG_PHY_MD_EN(1);
    
    return 0;
}

static void ssv6006_on3_enable(ssv_type_bool en)
{
     ssv_type_u32 regval=0;
     //0x303:
     //bit[0]: oscillator ready
     //bit[1]: DPLL ready
     //bit[8~9]: PMU in chip on status
     do{
        MAC_REG_READ(ADR_SYSCTRL_STATUS,regval);
     }while(regval!=0x303);

     MAC_REG_WRITE(ADR_POWER_ON_OFF_CTRL,0x7334);
     if(TRUE==en)
     {
        MAC_REG_WRITE(ADR_SYSCTRL_COMMAND,0x80C);
     }
     else
     {
        MAC_REG_WRITE(ADR_SYSCTRL_COMMAND,0x40C);
     }
     
     do{
        MAC_REG_READ(ADR_SYSCTRL_STATUS,regval);
     }while(regval!=0x303);

     return;
}


extern void ssv6006_hal_beacon_reset(void);
int ssv6006_hal_chip_init(cali_result_info* pCliRes_info)
{
    ssv_type_u32 regval=0;

    TU_INIT_TURISMOC_PLL;
    
    
    SET_CLK_FBUS_SEL(8); //Set N10 CLK TO 160MHz  
    SET_CLK_DIGI_SEL(8); //Set SBUS CLK TO 80MHz 
  
   
    ssv6006_hal_beacon_reset();
   
	return 0;

}


static ssv_type_bool _resource_setup(ssv_type_u8 param)
{
    ssv_type_bool ret = FALSE;
    ssv_type_u32 id_len = 0;
    ssv_type_u8 rx_id_threshold = SSV6006_ID_RX_THRESHOLD;
    ssv_type_u8 rx_page_threshold = SSV6006_PAGE_RX_THRESHOLD;

    MAC_REG_READ(ADR_TRX_ID_THRESHOLD,id_len);
    if (param == 1)
    {
        rx_id_threshold = SSV6006_ID_RX_THRESHOLD_LOW;
        rx_page_threshold = SSV6006_PAGE_RX_THRESHOLD_LOW;
    }

    id_len = (id_len&0xffff0000 ) |
            (SSV6006_ID_TX_THRESHOLD<<TX_ID_THOLD_SFT)|
            (rx_id_threshold<<RX_ID_THOLD_SFT);

    ret = MAC_REG_WRITE(ADR_TRX_ID_THRESHOLD, id_len);


    if(ret != TRUE)
    {
        LOG_PRINTF("Failed to update ID resource for pre-allocate frame\r\n");
        return ret;
    }

    MAC_REG_READ(ADR_ID_LEN_THREADSHOLD1,id_len);
    id_len = (id_len&0x0f )|
            (SSV6006_PAGE_TX_THRESHOLD<<ID_TX_LEN_THOLD_SFT)|
            (rx_page_threshold<<ID_RX_LEN_THOLD_SFT);
    ret = MAC_REG_WRITE(ADR_ID_LEN_THREADSHOLD1, id_len);

    if(ret != TRUE)
        LOG_PRINTF("Failed to update PAGE resource for pre-allocate frame\r\n");

    return ret;
}

int ssv6006_hal_set_ext_rx_int(ssv_type_u32 pin);
int ssv6006_hal_set_TR_switch_to_gpio(ssv_type_u8 TX_pin, ssv_type_u8 RX_pin);
int ssv6006_hal_get_tx_resources(void *pinfo);
int ssv6006_hal_pbuf_alloc(int size, int type);
extern ssv_type_s32 _ssv6xxx_wifi_ioctl_Ext(ssv_type_u32 cmd_id, void *data, ssv_type_u32 len, ssv_type_bool blocking,const ssv_type_bool mutexLock);
static int _ssv6006_hal_init_mac(ssv_type_u8 *self_mac, ssv_type_bool fullInit)
{
    int i=0;
    ssv_type_bool ret;
    ssv_type_u32 regval,j;
    char    chip_id[24]="";
    //u32     chip_tag1,chip_tag2;
    ssv_type_u32 hw_buf_ptr;
    ssv_type_u32 hw_sec_key;

    //OS_SemInit(&tx_loopback, 1, 0);

    //phy disable
    //SET_RG_PHY_MD_EN(0);
    
    //do MAC software reset first
    SET_MAC_SW_RST(1);
    i=0;
    do{

        regval=GET_MAC_SW_RST;
        i ++;
        if (i >10000){
            LOG_PRINTF("MAC reset fail !!!!\n\n");
            return -1;
        }
        OS_TickDelay(1);
    } while (regval != 0);

    
    //CHIP ID
    regval=GET_CHIP_ID_127_96;
    *((ssv_type_u32 *)&chip_id[0]) = (ssv_type_u32)LONGSWAP(regval);
    regval=GET_CHIP_ID_95_64;
    *((ssv_type_u32 *)&chip_id[4]) = (ssv_type_u32)LONGSWAP(regval);
    regval=GET_CHIP_ID_63_32;
    *((ssv_type_u32 *)&chip_id[8]) = (ssv_type_u32)LONGSWAP(regval);
    regval=GET_CHIP_ID_31_0;
    *((ssv_type_u32 *)&chip_id[12]) = (ssv_type_u32)LONGSWAP(regval);
    if(gDeviceInfo->recovering != TRUE)
    {
        LOG_DEBUG("CHIP TYPE: %s \r\n", chip_id);
    }
            
    //soc set HDR-STRIP-OFF       enable
    //soc set HCI-RX2HOST         enable
    //soc set AUTO-SEQNO          enable
    //soc set ERP-PROTECT          disable
    //soc set MGMT-TXQID            3
    //soc set NONQOS-TXQID      1
    #if(SW_8023TO80211==1)
    regval = (RX_2_HOST_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);
    #else
    regval = (RX_2_HOST_MSK|HDR_STRIP_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);
    #endif
    ret = MAC_REG_WRITE(ADR_CONTROL,regval);

    SET_RX_INFO_SIZE(SSV6006_RXINFO_SIZE);
    SET_TX_INFO_SIZE(SSV6006_TXINFO_SIZE);

    /* Enable hardware timestamp for TSF */
    // 28 => time stamp write location
    SET_MRX_STP_OFST(28);
    SET_MRX_STP_EN(1);

    SET_ALLOW_SD_SPI_RESET(0);
    /**
        * Tx/RX threshold setting for packet buffer resource.
        */

    ret = _resource_setup(g_host_cfg.pre_alloc_prb_frm);
#ifndef __SSV_UNIX_SIM__
    //update_tx_resource
    OS_MutexLock(txsrcMutex);
    ssv6006_hal_get_tx_resources((void*)&tx_rcs);
    OS_MutexUnLock(txsrcMutex);
#endif
    //Move vif macaddr setting to add interface
    //LOG_PRINTF("mac:%02x:%02x:%02x:%02x:%02x:%02x:\r\n",self_mac[0],self_mac[1],self_mac[2],self_mac[3],self_mac[4],self_mac[5]);
    //SET_STA_MAC_31_0(*((u32 *)&(self_mac[0])));
    ///SET_STA_MAC_47_32(*((u32 *)&(self_mac[4])));
    //LOG_PRINTF("1M1=%x\r\n",GET_STA_MAC_31_0);
    //LOG_PRINTF("1M2=%x\r\n",GET_STA_MAC_47_32);

    /**
        * Reset all wsid table entry to invalid.
        */

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID1, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID2, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID3, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID4, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID5, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID6, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID7, 0x00000000);


    //switch pin mux
    {      
        char if_name[32];
        ssv6xxx_drv_get_name(if_name);
        if(ssv6xxx_strcmp(if_name, "spi") == 0)
        {
            LOG_PRINTF("interface = SPI\r\n");
            //Enable RX interrupt by GPIO
            //if(ret == TRUE) ret = MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP, 0x77F77FFE);
            if(ret == TRUE) ret = MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP, 0xFFFF7FFF);
            if(ret == TRUE) ret = MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP_15, 0xffffffdf);

            //Assign rx int to GPIO8
            ssv6006_hal_set_ext_rx_int(g_host_cfg.rxIntGPIO);

            //Force HCI RX Aggregation disable            
            g_host_cfg.hci_rx_aggr = 0;
        }
        else if((ssv6xxx_strcmp(if_name, "sdio") == 0)&&(g_host_cfg.extRxInt))
        {
            LOG_PRINTF("Setup SDIO GPIO rx int\r\n");
            MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP, 0xFFFF7FFF);
            MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP_15, 0xFFFFFFEF);
            ssv6006_hal_set_ext_rx_int(g_host_cfg.rxIntGPIO);
        }
    }

    if(g_host_cfg.usePA)
    {
        
        LOG_PRINTF("Setup PA TRX_EN GPIO\r\n");
        ssv6006_hal_set_TR_switch_to_gpio(0, 1);
    }

    #if((RECOVER_ENABLE==1)&&(RECOVER_MECHANISM==1))
    MAC_REG_SET_BITS(ADR_MASK_TYPHOST_INT_MAP,0,~(1<<27)); //Enable MS_TIMER 3
    #endif    
    MAC_REG_SET_BITS(ADR_INT_MASK_REG,0,(~0x1));
    //MAC_REG_SET_BITS(ADR_INT_STATUS_REG,0,(~0x400));
    //MAC_REG_SET_BITS(ADR_INT_STATUS_REG,1<<10,(~0x400));
    MAC_REG_SET_BITS(ADR_CCCR_04H_REG,0x3,(~0x3));

    #if(CONFIG_CHIP_ID==SSV6006B)
    SET_MTX_DUR_RSP_TOUT_G(255); //for AMPDU, SET MAX TX Timeout duration
    #endif
    //SET_MTX_DBGOPT_RSPHANDLE_IGNORE_MRX_PROC_FALL(1); //for AMPDU

    //if(ret == TRUE) ret = MAC_REG_WRITE(ADR_MASK_TYPHOST_INT_MAP_15, 0xf7fff0xff0ffffffff);//bit 20,21,22,23

#if 0
    //Enable EDCA low threshold
    MAC_REG_WRITE(ADR_MB_THRESHOLD6, 0x80000000);
    //Enable EDCA low threshold EDCA-1[8] EDCA-0[4]
    MAC_REG_WRITE(ADR_MB_THRESHOLD8, 0x08040000);
    //Enable EDCA low threshold EDCA-3[8] EDCA-2[8]
    MAC_REG_WRITE(ADR_MB_THRESHOLD9, 0x00000808);
#endif

    /**
        * Disable tx/rx ether trap table.
        */

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_TX_ETHER_TYPE_0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_TX_ETHER_TYPE_1, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_ETHER_TYPE_0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_ETHER_TYPE_1, 0x00000000);

    /**
        * Allocate a hardware packet buffer space. This buffer is for security
        * key caching and phy info space.
        */
    /*lint -save -e732  Loss of sign (assignment) (int to unsigned int)*/
    hw_buf_ptr = ssv6006_hal_pbuf_alloc((ssv_type_s32)sizeof(struct ssv6006_hw_sec),(int)NOTYPE_BUF);
    /*lint -restore */
    if((hw_buf_ptr>>28) != 8)
    {
    	//asic pbuf address start from 0x8xxxxxxxx
    	LOG_PRINTF("opps allocate pbuf error\n");
    	//WARN_ON(1);
    	return -1;
    }

    //if(gDeviceInfo->recovering != TRUE)
        //LOG_PRINTF("%s(): ssv6006 reserved space=0x%08x, size=%d\r\n",__FUNCTION__, hw_buf_ptr, sizeof(struct ssv6006_hw_sec));

    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    hw_sec_key = hw_buf_ptr;
    //LOG_PRINTF("hw_sec_key(0x%x)\r\n",(u32)hw_sec_key);
	//==>Section 1. Write Sec table to SRAM
    for(j=0; j<sizeof(struct ssv6006_hw_sec); j+=4) {
        MAC_REG_WRITE(hw_sec_key+j, 0);
    }
    /*lint -save -e838*/
    regval = ((hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
    MAC_REG_READ(ADR_SCRT_SET, regval);
	regval &= SCRT_PKT_ID_I_MSK;
	regval |= ((hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
	MAC_REG_WRITE(ADR_SCRT_SET, regval);
    /*lint -restore*/
    /* set B mode ack/cts rate */
    //B_2M long preamble response 1M control rate
    SET_MTX_RESPFRM_RATE_01(0);

    //B_5.5M long preamble  response 1M control rate
    SET_MTX_RESPFRM_RATE_02(0);

    //B_11M long preamble  response 1M control rate
    SET_MTX_RESPFRM_RATE_03(0);

    //B_2M short preamble response 1M control rate
    SET_MTX_RESPFRM_RATE_11(0);

    //B_5.5M short preamble  response 1M control rate
    SET_MTX_RESPFRM_RATE_12(0);

    //B_11M short preamble  response 1M control rate
    SET_MTX_RESPFRM_RATE_13(0);

    /* set G mode ack/cts rate */
    //G_12M response 6M control rate
    SET_MTX_RESPFRM_RATE_92_B2(0x9090);

    //G_24M response 12M control rate
    SET_MTX_RESPFRM_RATE_94_B4(0x9292);

    /* set N mode ack/cts rate */
    //MCS1(LGI) response 6M control rate
    SET_MTX_RESPFRM_RATE_C1_E1(0x9090);

    //MCS3(LGI) response 12M control rate
    SET_MTX_RESPFRM_RATE_C3_E3(0x9292);

    //MCS1(SGI) response 6M control rate
    SET_MTX_RESPFRM_RATE_D1_F1(0x9090);

    //MCS3(SGI) response 12M control rate
    SET_MTX_RESPFRM_RATE_D3_F3(0x9292);


    //-----------------------------------------------------------------------------------------------------------------------------------------

    //if(ret == TRUE) ret = MAC_REG_WRITE(0xca000800,0xffffffff);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_MIB_EN,0xffffffff);

    //-----------------------------------------------------------------------------------------------------------------------------------------
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //C_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);

    /* Setup q4 behavior STA mode-> act as normal queue
      *
      */

    //-----------------------------------------------------------------------------------------------------------------------------------------

    //Now, CPU has two output queue(from MBX view).
    //The new queue is linked to irq number 18, the original queue is linked to irq number 19
    //Rate Report and BA trap to CPU through irq number 18, and others trap to CPU through 19
    //The priority of irq 18 is higher than irq 19

    SET_TX_RX_TRAP_HW_ID_SELECT_ENABLE(1);
    SET_MTX_RATERPT_HWID(2);
    SET_BA_HW_ID(2);
    SET_BA_H_QUEUE_EN(1);

#if 0        
    //Load FW
    {
        extern ssv_type_bool platform_download_firmware(void);
        if(platform_download_firmware() == FALSE)
            return -1;
    }
#endif
    //MAC_REG_SET_BITS(ADR_WIFI_PHY_COMMON_ENABLE_REG, (1 << RG_PHY_MD_EN_SFT), RG_PHY_MD_EN_I_MSK);
    //SET_RG_PHY_MD_EN(1);
    //ssv6xxx_drv_irq_enable(0);
    //Set watchdoginterface
#if RECOVER_ENABLE
    {
        unsigned char cmd_data[] = {
            0x00, 0x00, 0x00, 0x00};
        cmd_data[0]= RECOVER_ENABLE;
        cmd_data[1]= RECOVER_MECHANISM;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_RECOVER, cmd_data, 4, TRUE, FALSE);
    }
#endif

    return ((ret == TRUE)?0:-1);
}

int ssv6006_hal_init_mac(ssv_type_u8 *self_mac)
{
    return _ssv6006_hal_init_mac(self_mac, TRUE);
}

int ssv6006_hal_apply_sta_deci_tbl(void);
int ssv6006_hal_apply_generic_deci_tbl(void);

static int _ssv6006_hal_init_sta_mac(ssv_type_u32 wifi_mode, ssv_type_bool fullInit)
{
    int ret=0;
    

    if(SSV6XXX_HWM_STA == wifi_mode)
    {
        MAC_REG_WRITE(ADR_GLBLE_SET,
            (0 << OP_MODE_SFT)  |                           /* STA mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            //(1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            //(1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            (1 << LUT_SEL_V2_SFT) |
            (SSV6006_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6006_TX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
        );
    }
    else
    {
        MAC_REG_WRITE(ADR_GLBLE_SET,
            (0 << OP_MODE_SFT)  |                           /* STA mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            (1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            //(1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            (1 << LUT_SEL_V2_SFT) |
            (SSV6006_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6006_TX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
        );

    }

    MAC_REG_WRITE(ADR_MRX_LEN_FLT,0x900);                   /*Prevent from RX hang because of receiving large frame*/

    MAC_REG_WRITE(ADR_BSSID_0,   0x00000000);//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   0x00000000);//*((u32 *)&priv->bssid[4]));
     //trap null data
    //SET_RX_NULL_TRAP_EN(1)
    //{
    //    u32 tarpEnable;
    //    MAC_REG_READ(ADR_CONTROL,tarpEnable);
    //    tarpEnable = tarpEnable & RX_NULL_TRAP_EN_I_MSK;
    //    tarpEnable |= (1 << RX_NULL_TRAP_EN_SFT);
    //    MAC_REG_WRITE(ADR_CONTROL, tarpEnable);
    //}
     /**
        * Set reason trap to discard frames.
        */
    //
    //MAC_REG_WRITE(ADR_REASON_TRAP0, 0x7FBC7F87);
    //Drop null data frame
    MAC_REG_WRITE(ADR_REASON_TRAP0,0x7FBC3F87);
    MAC_REG_WRITE(ADR_REASON_TRAP1, 0x0000003F);

    cabrio_init_sta_mode_data_flow();

    //#set EDCA parameter AP-a/g BK[0], BE[1], VI[2], VO[3]
    //soc set WMM-PARAM[0]      { aifsn=0 acm=0 cwmin=5 cwmax=10 txop=0 backoffvalue=6 }


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,

          (6 << TXQ0_MTX_Q_AIFSN_SFT)  |                           /* aifsn=7 */
          (4 << TXQ0_MTX_Q_ECWMIN_SFT) |                            /*cwmin=4 */
          (10 << TXQ0_MTX_Q_ECWMAX_SFT)                           /* cwmax=10 */

    );

    #if(CONFIG_CHIP_ID==SSV6006B)	
    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);
    #endif

    //soc set WMM-PARAM[1]      { aifsn=0 acm=0 cwmin=4 cwmax=10 txop=0 backoffvalue=5 }
    MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,

              (2 << TXQ1_MTX_Q_AIFSN_SFT)  |                       /* aifsn=3 */
              (4 << TXQ1_MTX_Q_ECWMIN_SFT) |                        /*cwmin=4 */
              (10 << TXQ1_MTX_Q_ECWMAX_SFT)                      /* cwmax=10 */
    );


    //soc set WMM-PARAM[2]      { aifsn=0 acm=0 cwmin=3 cwmax=4 txop=94 backoffvalue=4 }
    {

        MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,

              (1 << TXQ2_MTX_Q_AIFSN_SFT)  |                       /* aifsn=2 */
              (3 << TXQ2_MTX_Q_ECWMIN_SFT) |                        /*cwmin=3 */
              (4 << TXQ2_MTX_Q_ECWMAX_SFT) |                       /* cwmax=4 */
              (94 << TXQ2_MTX_Q_TXOP_LIMIT_SFT)                        /*  txop=94 */
        );

    }

    //soc set WMM-PARAM[3]      { aifsn=0 acm=0 cwmin=2 cwmax=3 txop=47 backoffvalue=3 }
    {
        //info 845: The right argument to operator '|' is certain to be 0

        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,

              (1 << TXQ3_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
              (2 << TXQ3_MTX_Q_ECWMIN_SFT) |                    /*cwmin=2 */
              (3 << TXQ3_MTX_Q_ECWMAX_SFT) |                  /* cwmax=3 */
              (47 << TXQ3_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=47 */
        );

    }
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //MAC_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
        /*lint -save -e648 Overflow in computing constant for operation:    'shift left'*/
        MAC_REG_WRITE( ADR_TXQ4_MTX_Q_AIFSN,

          (2 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=3 */
          (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
          (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
          (94 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );

    /* By default, we apply staion decion table. */
    ssv6006_hal_apply_sta_deci_tbl();

    _ssv6006_hal_rf_phy_init();
    //ssv6xxx_wifi_set_channel(STA_DEFAULT_CHANNEL,SSV6XXX_HWM_STA);

    return ret;
}

int ssv6006_hal_init_sta_mac(ssv_type_u32 wifi_mode)
{

    return _ssv6006_hal_init_sta_mac(wifi_mode, TRUE);
}

static int _ssv6006_hal_init_ap_mac(ssv_type_u8 *bssid,ssv_type_u8 channel, ssv_type_bool fullInit)
{
    int ret=0;
    //ssv_type_u32 temp;    

#if (AP_MODE_ENABLE == 1)        
    MAC_REG_WRITE(ADR_GLBLE_SET,
            (1 << OP_MODE_SFT)  |                           /* AP mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            //(1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            (1 << LUT_SEL_V2_SFT) |
            //(1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            (SSV6006_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6006_RX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
    );
    MAC_REG_WRITE(ADR_MRX_LEN_FLT,0x900);                   /*Prevent from RX hang because of receiving large frame*/

    //MAC_REG_READ(ADR_SCRT_SET,temp);
    //temp = temp & SCRT_RPLY_IGNORE_I_MSK;
    //temp |= (1 << SCRT_RPLY_IGNORE_SFT);
    //MAC_REG_WRITE(ADR_SCRT_SET, temp);

    MAC_REG_WRITE(ADR_BSSID_0,   *((ssv_type_u32 *)&bssid[0]));//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   *((ssv_type_u32 *)&bssid[4]));//*((u32 *)&priv->bssid[4]));

    cabrio_init_ap_mode_data_flow();

     //soc set REASON-TRAP         { 0x7fbf7f8f 0xffffffff }
     MAC_REG_WRITE(ADR_REASON_TRAP0,0x7FBC3F87);
     MAC_REG_WRITE(ADR_REASON_TRAP1, 0x0000003F);

    #if(CONFIG_CHIP_ID==SSV6006B)	
    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);
    #endif



    //Update wmm parameters of q1 and q1, default as AMPDU on
    //ssv6006_hal_setup_ampdu_wmm(TRUE);
    //BK
    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,
        (6 << TXQ0_MTX_Q_AIFSN_SFT)  |                  /* aifsn=7 */
        (3 << TXQ0_MTX_Q_ECWMIN_SFT) |                  /*cwmin=4 */
        (9 << TXQ0_MTX_Q_ECWMAX_SFT)                    /* cwmax=10 */
    );
    //BE
    MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,
        (2 << TXQ1_MTX_Q_AIFSN_SFT)  |                  /* aifsn=3 */
        (3 << TXQ1_MTX_Q_ECWMIN_SFT) |                  /*cwmin=4 */
        (9 << TXQ1_MTX_Q_ECWMAX_SFT)                    /* cwmax=10 */
    );
    //Video
    MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,
        (1 << TXQ2_MTX_Q_AIFSN_SFT)  |                  /* aifsn=2 */
        (2 << TXQ2_MTX_Q_ECWMIN_SFT) |                  /*cwmin=3 */
        (3 << TXQ2_MTX_Q_ECWMAX_SFT) |                  /* cwmax=4 */
        (94 << TXQ2_MTX_Q_TXOP_LIMIT_SFT )              /*  txop=94 */
    );
    //Voice
    MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,
        (1 << TXQ3_MTX_Q_AIFSN_SFT)  |                  /* aifsn=2 */
        (1 << TXQ3_MTX_Q_ECWMIN_SFT) |                  /*cwmin=2 */
        (2 << TXQ3_MTX_Q_ECWMAX_SFT) |                  /* cwmax=3 */
          (47 << TXQ3_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=47 */
    );


    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //MAC_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
        /*lint -save -e648 Overflow in computing constant for operation:    'shift left'*/
    MAC_REG_WRITE( ADR_TXQ4_MTX_Q_AIFSN,

    (1 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
    (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
    (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
    ((unsigned int)65535 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );
    /*lint -restore*/

    /* By default, we apply ap decion table. */
    ssv6006_hal_apply_ap_deci_tbl();

    _ssv6006_hal_rf_phy_init();

    //trap null data
    //SET_RX_NULL_TRAP_EN(1)
    //MAC_REG_READ(ADR_CONTROL,temp);
    //temp = temp & RX_NULL_TRAP_EN_I_MSK;
    //temp |= (1 << RX_NULL_TRAP_EN_SFT);
    //MAC_REG_WRITE(ADR_CONTROL, temp);

    //ssv6xxx_wifi_set_channel(gDeviceInfo->APInfo->nCurrentChannel,SSV6XXX_HWM_AP);
#endif
    return ret;
}

int ssv6006_hal_init_ap_mac(ssv_type_u8 *bssid,ssv_type_u8 channel)
{
    return _ssv6006_hal_init_ap_mac(bssid,channel,TRUE);
}

int ssv6006_hal_set_concurrent(ssv_type_bool enable)
{
    //int i;
    //ssv_type_u16 *mac_deci_tbl;

    if(enable)
    {
        LOG_PRINTF("enable_concurrent_setting\r\n");
        cabrio_init_ap_mode_data_flow();
        ssv6006_hal_apply_generic_deci_tbl();
        SET_IDX_EXTEND(1); //enable decition tbl extend bit for dual interface
    }
    else
    {
        LOG_PRINTF("disable_concurrent_setting\r\n");
        cabrio_init_sta_mode_data_flow();
        ssv6006_hal_apply_sta_deci_tbl();
        SET_IDX_EXTEND(0); //enable decition tbl extend bit for dual interface
    }
    
    return 0;
}
ssv6xxx_hw_mode vif0_mode=SSV6XXX_HWM_INVALID;
ssv6xxx_hw_mode vif1_mode=SSV6XXX_HWM_INVALID;
int ssv6006_hal_add_interface(ssv_type_u8 itf_idx,ssv6xxx_hw_mode hmode,ssv_type_u8 *selfmac,ssv_type_u8 channel)
{

    ssv_type_u32 mac1=0,mac2=0;
    //ssv6xxx_hw_mode mode0,mode1;
    
    LOG_DEBUGF(LOG_TXRX,("Add vif=%d,hmode=%d,channel=%d\r\n",itf_idx,hmode,channel));
    
    mac1 = ((selfmac[3]<<24)|(selfmac[2]<<16)|(selfmac[1]<<8)|selfmac[0]);
    mac2 = ((selfmac[5]<<8)|selfmac[4]);
    LOG_DEBUGF(LOG_TXRX,("vif MAC[%02x:%02x:%02x:%02x:%02x:%02x],M1=%x,M2=%x\r\n",
        selfmac[0], selfmac[1], selfmac[2],
        selfmac[3], selfmac[4], selfmac[5],mac1,mac2));
    switch(itf_idx)
    {
        case 0:
        {
            if(hmode == SSV6XXX_HWM_AP)
            {
                SET_BSSID_31_0(mac1);
                SET_BSSID_47_32(mac2);
                //LOG_PRINTF("0bs1=%x\r\n",GET_BSSID_31_0);
                //LOG_PRINTF("0bs2=%x\r\n",GET_BSSID_47_32);
            }
            else
            {
                SET_BSSID_31_0(0);
                SET_BSSID_47_32(0);
            }
            SET_STA_MAC_31_0(mac1);
            SET_STA_MAC_47_32(mac2);
            //LOG_PRINTF("0M1=%x\r\n",GET_STA_MAC_31_0);
            //LOG_PRINTF("0M2=%x\r\n",GET_STA_MAC_47_32);
            SET_OP_MODE(hmode);
            vif0_mode = hmode;  
            //LOG_PRINTF("H0 mode=%x\r\n",GET_OP_MODE);
            break;
        }
        case 1:
        {
            if(hmode == SSV6XXX_HWM_AP)
            {
                SET_BSSID1_31_0(mac1);
                SET_BSSID1_47_32(mac2);
                //LOG_PRINTF("1bs1=%x\r\n",GET_BSSID1_31_0);
                //LOG_PRINTF("1bs2=%x\r\n",GET_BSSID1_47_32);
            }
            else
            {
                SET_BSSID1_31_0(0);
                SET_BSSID1_47_32(0);
            }
            SET_STA_MAC1_31_0(mac1);
            SET_STA_MAC1_47_32(mac2);
            //LOG_PRINTF("1M1=%x\r\n",GET_STA_MAC1_31_0);
            //LOG_PRINTF("1M2=%x\r\n",GET_STA_MAC1_47_32);
            SET_OP_MODE1(hmode);
            vif1_mode = hmode;  
            //LOG_PRINTF("H1 mode=%x\r\n",GET_OP_MODE1);
            break;
        }
    }
    
    if((vif0_mode != SSV6XXX_HWM_INVALID)&&(vif1_mode != SSV6XXX_HWM_INVALID))
    {
        if((vif0_mode == SSV6XXX_HWM_AP)||(vif1_mode == SSV6XXX_HWM_AP))
        {
            ssv6006_hal_set_concurrent(1);
        }
        else if((vif0_mode == SSV6XXX_HWM_STA)&&(vif1_mode == SSV6XXX_HWM_STA))
        {
            ssv6006_hal_set_concurrent(0);
        }
    }
    
    //if(channel)
    //    ssv6xxx_wifi_set_channel(itf_idx, channel, hmode);
    return 0;
}

int ssv6006_hal_remove_interface(ssv_type_u8 itf_idx)
{

    switch(itf_idx)
    {
        case 0:
        {
            SET_BSSID_31_0(0);
            SET_BSSID_47_32(0);
            SET_STA_MAC_31_0(0);
            SET_STA_MAC_47_32(0);
            SET_OP_MODE(0);
            vif0_mode = SSV6XXX_HWM_INVALID;  
            break;
        }
        case 1:
        {
            SET_BSSID1_31_0(0);
            SET_BSSID1_47_32(0);
            SET_STA_MAC1_31_0(0);
            SET_STA_MAC1_47_32(0);
            SET_OP_MODE1(0);
            vif1_mode = SSV6XXX_HWM_INVALID;  
            break;
        }
    }
    return 0;
}

int ssv6006_hal_ap_wep_setting(ssv6xxx_sec_type sec_type, ssv_type_u8 *password, ssv_type_u8 vif_idx, ssv_type_u8* sta_mac_addr)
{
	struct securityEntry    sec_entry ;
    switch (sec_type)  {
        case SSV6XXX_SEC_WEP_40:
            sec_entry.cipher = CIPHER_HOST_WEP40;
            sec_entry.wpaUnicast = 1;
            sec_entry.keyLen = ssv6xxx_strlen((void*)password);
            sec_entry.keyIndex = 0;
            sec_entry.vif_idx = vif_idx;
            OS_MemCPY(sec_entry.key,  password, ssv6xxx_strlen((void*)password));
            OS_MemCPY(sec_entry.bssid,(void *)sta_mac_addr,6);
            _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_SECURITY_ENTRY, &sec_entry, sizeof(struct securityEntry), TRUE, FALSE);

            break;

        case SSV6XXX_SEC_WEP_104:
            sec_entry.cipher = CIPHER_HOST_WEP104;
            sec_entry.wpaUnicast = 1;
            sec_entry.keyLen = ssv6xxx_strlen((void*)password);
            sec_entry.keyIndex = 0;
            sec_entry.vif_idx = vif_idx;
            OS_MemCPY(sec_entry.key, password, ssv6xxx_strlen((void*)password));
            OS_MemCPY(sec_entry.bssid,(void *)sta_mac_addr,6);
            _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_SECURITY_ENTRY, &sec_entry, sizeof(struct securityEntry), TRUE, FALSE);
            break;

        case SSV6XXX_SEC_WPA_PSK:
        case SSV6XXX_SEC_WPA2_PSK:
        default:
            break;
    }
    return 0;
}
int ssv6006_hal_promiscuous_enable(void)
{

    ssv_type_u32 regval;
    MAC_REG_READ(ADR_CONTROL,regval);
#if(SW_8023TO80211==0)    
    regval&=HDR_STRIP_I_MSK;
    regval&=RX_ETHER_TRAP_EN_I_MSK;
#else
    regval&=RX_ETHER_TRAP_EN_I_MSK;
#endif
    regval&=RX_ETHER_TRAP_EN_I_MSK;
    MAC_REG_WRITE(ADR_CONTROL,regval);;
//#endif
    return 0;
}

int ssv6006_hal_promiscuous_disable(void)
{
    ssv_type_u32 regval;
    MAC_REG_READ(ADR_CONTROL,regval);
#if(SW_8023TO80211==0)        
    regval|=(HDR_STRIP_MSK|RX_ETHER_TRAP_EN_MSK);
#else
    regval|=(RX_ETHER_TRAP_EN_MSK);
#endif
    MAC_REG_WRITE(ADR_CONTROL,regval);
//#endif
    return 0;
}

int ssv6006_hal_watchdog_enable(void)
{
    SET_SYS_WDOG_ENA(1);
    return 0;
}

int ssv6006_hal_watchdog_disable(void)
{
    SET_SYS_WDOG_ENA(0);    
    return 0;
}

int ssv6006_hal_mcu_enable(void)
{
    #if 0
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x1);
    #else
    _ssv6006_load_fw_enable_mcu();
    #endif
    return 0;
}

int ssv6006_hal_mcu_disable(void)
{
    #if 0
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x0);
    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */
    #else
    SET_MAC_SW_RST(1);
    _ssv6006_load_fw_disable_mcu();
    #endif
    return 0;
}

int ssv6006_hal_set_gpio_output(ssv_type_bool hi_lo, ssv_type_u32 pin)
{
    MAC_REG_SET_BITS(ADR_MANUAL_IO,1<<pin,~(1<<pin));

    //output
    MAC_REG_SET_BITS(ADR_IO_PIE,0<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_IO_POEN,0<<pin,~(1<<pin));

    MAC_REG_SET_BITS(ADR_IO_PO,hi_lo<<pin,~(1<<pin));
    return 0;
}

int ssv6006_hal_set_ext_rx_int(ssv_type_u32 pin)
{
    //Assign rx int
    MAC_REG_SET_BITS(ADR_MANUAL_IO,1<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_MANUAL_PD,1<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_IO_PDE,1<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_INT_THRU_GPIO,1<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_IO_PIE,0<<pin,~(1<<pin));
    MAC_REG_SET_BITS(ADR_IO_POEN,0<<pin,~(1<<pin));         
    return 0;
}

int ssv6006_hal_set_TR_switch_to_gpio(ssv_type_u8 TX_pin, ssv_type_u8 RX_pin)
{
    //Assign rx int
    MAC_REG_SET_BITS(ADR_MANUAL_IO,1<<TX_pin,~(1<<TX_pin));
    MAC_REG_SET_BITS(ADR_MANUAL_IO,1<<RX_pin,~(1<<RX_pin));
    //MAC_REG_SET_BITS(ADR_IO_PO,1<<TX_pin,~(1<<TX_pin));
    //MAC_REG_SET_BITS(ADR_IO_PO,1<<RX_pin,~(1<<RX_pin));
    
    //MAC_REG_SET_BITS(ADR_IO_POEN,0<<TX_pin,~(1<<TX_pin));         
    //MAC_REG_SET_BITS(ADR_IO_POEN,0<<RX_pin,~(1<<RX_pin));
    SET_SEL_BTCX(1);
    return 0;
}

int ssv6006_hal_pause_resuem_recovery_int(ssv_type_bool resume)
{
    #if 0
    u32 val;

    MAC_REG_READ(ADR_INT_MASK,val);
    MAC_REG_WRITE(ADR_SDIO_IRQ_STS,(1<<27));
    if(resume)
    {
        val |= (1<<27);
    }
    else
    {
        val &= ~(1<<27);
    }
    MAC_REG_WRITE(ADR_INT_MASK,val);
    #endif
    return 0;
}

ssv_type_u32 rcnt=1;
int ssv6006_hal_gen_random(ssv_type_u8 *data, ssv_type_u32 len)
{
    ssv_type_u32 sft=0,i;
    ssv_type_u32 rtemp ;
    #if 1
    
    for(i=0;i<len;i+=4){
        MAC_REG_READ(ADR_READ_ONLY_FLAGS_ADC,rtemp); 
        sft = rtemp & 0x0f;
        *(data+i)=(rtemp>>(3+sft))&0xff;
        MAC_REG_READ(ADR_READ_ONLY_FLAGS_ADC,rtemp);
        sft = rtemp & 0x0f;
        *(data+1+i)=(rtemp>>(4+sft))&0xff;
        *(data+2+i)=(rtemp>>(8+sft))&0xff;
        MAC_REG_READ(ADR_READ_ONLY_FLAGS_ADC,rtemp);
        sft = rtemp & 0x0f;
        *(data+3+i)=(rtemp>>(3+sft))&0xff;
    }
    #else
    //MAC_REG_WRITE(ADR_RAND_EN,1);
    //MAC_REG_READ(ADR_READ_ONLY_FLAGS_ADC,rtemp);
    for(i=0;i<len;i+=4){
        //MAC_REG_READ(ADR_RAND_NUM,rtemp);
        *(data+i)= 0xA5;//rtemp&0xff;
        *(data+1+i)= 0x5A;//(rtemp>>8)&0xff;
        *(data+2+i)=0xA5;//(rtemp>>16)&0xff;
        *(data+3+i)=0x5A;//(rtemp>>24)&0xff;
    }
    //MAC_REG_WRITE(ADR_RAND_EN,0);
    #endif
    return 0;
}

int ssv6006_hal_get_rssi_from_reg(ssv_type_u8 vif_idx)
{
    ssv_type_u32 regVal=0;
    regVal=GET_RG_RAM_01;
    if(vif_idx==0)
        regVal=GET_RG_RAM_01;
    else
        regVal=GET_RG_RAM_02;
    return regVal;
}

int ssv6006_hal_get_agc_gain(void)
{
    ssv_type_u32 regVal=0;
    //Eric need to confirm
    //MAC_REG_READ(ADR_PHY_REG_03_AGC, regVal);
    return regVal;
}

int ssv6006_hal_set_agc_gain(ssv_type_u32 gain)
{
    //Eric need to confirm
    //MAC_REG_WRITE(ADR_PHY_REG_03_AGC,gain);
    return 0;
}

int ssv6006_hal_set_acs_agc_gain(void)
{
    ssv6006_hal_set_agc_gain(SSV6006_ACS_AGC_GAIN);
    return 0;
}

int ssv6006_hal_get_rc_info(ssv_type_u16 *tx_cnt, ssv_type_u16 *retry_cnt, ssv_type_u16 *phy_rate)
{
    ssv_type_u32 regval = 0;
    //Eric need to confirm
    //MAC_REG_READ(ADR_DBG_SPI_TO_PHY_PARAM1, regval);
    //MAC_REG_WRITE(ADR_DBG_SPI_TO_PHY_PARAM1, 0);
    *tx_cnt = regval&0x0000ffff;
    *retry_cnt = (regval&0xffff0000)>>16;
    //MAC_REG_READ(ADR_DBG_SPI_TO_PHY_PARAM2, regval);
    *phy_rate = (ssv_type_u16)regval;
    return 0;
}

ssv_type_bool ssv6006_hal_get_diagnosis(void)
{
    ssv_type_u32 SysIrq=0;
    ssv_type_u32 Wdg=0;
    ssv_type_u32 ChipId0;
    ssv_type_u32 ChipId2;
    //MAC_REG_READ(ADR_BOOT_INFO,(SysIrq));
    MAC_REG_READ(ADR_SYS_WDOG_REG,(Wdg));
    MAC_REG_READ(ADR_CHIP_ID_0,(ChipId0));
    MAC_REG_READ(ADR_CHIP_ID_2,(ChipId2));
    LOG_PRINTF("diagnosis: %x %x %x %x\r\n",SysIrq,Wdg,ChipId0,ChipId2);
    if (/*(SysIrq&0x2)&&*/(Wdg&0x10000)&&
            (ChipId0==0x20202020)&&(ChipId2==0x30303643)) // watchdog wack up & reset
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


int ssv6006_hal_is_heartbeat(void)
{
    ssv_type_u32 isr_status;
    MAC_REG_READ(ADR_RAW_TYPHOST_INT_MAP,isr_status);
    return (isr_status>>27)&0x01;
}

int ssv6006_hal_reset_heartbeat(void)
{
    MAC_REG_WRITE(ADR_CLR_INT_STS0,1<<27);
    return 0;
}

int ssv6006_hal_get_fw_status(ssv_type_u32 *val)
{
    MAC_REG_READ(SSV6006_FW_STATUS_REG,(*val));
    return 0;
}

int ssv6006_hal_set_fw_status(ssv_type_u32 val)
{
    MAC_REG_WRITE(SSV6006_FW_STATUS_REG,(val));
    return 0;
}

int ssv6006_hal_reset_soc_irq(void)
{
    //Eric
    //MAC_REG_WRITE(ADR_SYS_INT_FOR_HOST,0); //  system reset interrupt for host
    return 0;
}

static ssv_type_bool _ssv6006_hal_mcu_input_full(void)
{
    ssv_type_u32 regval=0;
    MAC_REG_READ(ADR_MCU_STATUS, regval);
    return CH0_FULL_MSK&regval;
}


int ssv6006_hal_pbuf_free(int addr)
{
    ssv_type_u32  regval=0;
    ssv_type_u16  failCount=0;

    while (_ssv6006_hal_mcu_input_full())
    {
        if (failCount++ < 1000)
            continue;
        LOG_PRINTF("=============>ERROR!!MAILBOX Block[%d]\n", failCount);
        return -1;
    } //Wait until input queue of cho is not full.

    // {HWID[3:0], PKTID[6:0]}
    regval = ((M_ENG_TRASH_CAN << SSV6006_HW_ID_OFFSET) |(addr >> SSV6006_ADDRESS_OFFSET));

    MAC_REG_WRITE(ADR_CH0_TRIG_1, regval);

    return 0;
}

int ssv6006_hal_pbuf_alloc(int size, int type)
{
#define MAX_RETRY_COUNT         20
    ssv_type_u32 pad;
    int regval=0;
    int cnt = MAX_RETRY_COUNT;
    //int page_cnt = (size + ((1 << HW_MMU_PAGE_SHIFT) - 1)) >> HW_MMU_PAGE_SHIFT;

    //mutex_lock(&sc->mem_mutex);

    //brust could be dividen by 4
    pad = (ssv_type_u32)size%4;
    size += (int)pad;

    do{
        //printk("[A] ssv6xxx_pbuf_alloc\n");

        MAC_REG_WRITE(ADR_WR_ALC, (size | (type << 16)));
        MAC_REG_READ(ADR_WR_ALC, regval);

        if (regval == 0) {
            cnt--;
            //msleep(1);
            OS_TickDelay(1);
        }
        else
            break;
    } while (cnt);

    // If TX buffer is allocated, AMPDU maximum size m
    /*
    if (type == TX_BUF)
    {
        sc->sh->tx_page_available -= page_cnt;
        sc->sh->page_count[PACKET_ADDR_2_ID(regval)] = page_cnt;
    }
    */
    //mutex_unlock(&sc->mem_mutex);
    if(regval!=0)
        return regval;
    else
        return -1;
}

int ssv6006_hal_set_short_slot_time(ssv_type_bool enable)
{
#if(CONFIG_CHIP_ID==SSV6006B)	
    ssv_type_u32 regval=0;
	
    if (enable)
    {
        MAC_REG_READ(ADR_MTX_DUR_SIFS_G, regval);
        regval = regval & MTX_DUR_BURST_SIFS_G_I_MSK;
        regval |= 0xa << MTX_DUR_BURST_SIFS_G_SFT;
        regval = regval & MTX_DUR_SLOT_G_I_MSK;
        regval |= 9 << MTX_DUR_SLOT_G_SFT;
        MAC_REG_WRITE(ADR_MTX_DUR_SIFS_G, regval);

        MAC_REG_READ(ADR_MTX_DUR_IFS, regval);
        regval = regval & MTX_DUR_BURST_SIFS_I_MSK;
        regval |= 0xa << MTX_DUR_BURST_SIFS_SFT;
        regval = regval & MTX_DUR_SLOT_I_MSK;
        regval |= 9 << MTX_DUR_SLOT_SFT;
        MAC_REG_WRITE(ADR_MTX_DUR_IFS, regval);
        //slottime = 9;
    }
    else
    {

        MAC_REG_READ(ADR_MTX_DUR_SIFS_G, regval);
        regval = regval & MTX_DUR_BURST_SIFS_G_I_MSK;
        regval |= 0xa << MTX_DUR_BURST_SIFS_G_SFT;
        regval = regval & MTX_DUR_SLOT_G_I_MSK;
        regval |= 20 << MTX_DUR_SLOT_G_SFT;
        MAC_REG_WRITE(ADR_MTX_DUR_SIFS_G, regval);

        MAC_REG_READ(ADR_MTX_DUR_IFS, regval);
        regval = regval & MTX_DUR_BURST_SIFS_I_MSK;
        regval |= 0xa << MTX_DUR_BURST_SIFS_SFT;
        regval = regval & MTX_DUR_SLOT_I_MSK;
        regval |= 20 << MTX_DUR_SLOT_SFT;
        MAC_REG_WRITE(ADR_MTX_DUR_IFS, regval);
        //slottime = 20;

    }
#endif
    return 0;
}

int ssv6006_hal_get_tx_resources(void *pinfo)
{
    struct ssv6006_hci_txq_info *pInfo=NULL;
    struct rcs_info * rs_info = (struct rcs_info *)pinfo;
    ssv_type_u32 regVal;
    ssv_type_u16 free_page = 0;
    ssv_type_u16 free_id = 0 ;
    ssv_type_u16 free_spaces=0;
    MAC_REG_READ(ADR_TX_ID_ALL_INFO,regVal);
    pInfo=(struct ssv6006_hci_txq_info *)&regVal;
    free_page = SSV6006_PAGE_TX_THRESHOLD-pInfo->tx_use_page;
    free_id = SSV6006_ID_TX_THRESHOLD-pInfo->tx_use_id;
    free_spaces=M_ENG_HWHCI_INT-GET_FF1_CNT; //how many space of HCI input queue for incoming packets
    if( free_page<=SSV6006_PAGE_TX_THRESHOLD && free_id<=SSV6006_ID_TX_THRESHOLD && free_spaces<=M_ENG_HWHCI_INT)
    {
        rs_info->free_page = free_page;
        rs_info->free_id = free_id;
        rs_info->free_space = free_spaces;
    }
    else
    {
        rs_info->free_page = 0;
        rs_info->free_id = 0;
        rs_info->free_space = 0;
    }
    
    rs_info->edca_fifo_cnt[0]= pInfo->txq0_size;
    rs_info->edca_fifo_cnt[1]= pInfo->txq1_size;
    rs_info->edca_fifo_cnt[2]= pInfo->txq2_size;
    rs_info->edca_fifo_cnt[3]= pInfo->txq3_size;
    return 0;
}

int ssv6006_hal_is_hw_hci_ready(void)
{

    return 0;
}

int ssv6006_hal_bytes_to_pages(ssv_type_u32 size)
{
    ssv_type_u32 page_count=0;
    page_count= (size + SSV6006_ALLOC_RSVD);

    if (page_count & SSV6006_HW_MMU_PAGE_MASK)
        page_count = (page_count >> SSV6006_HW_MMU_PAGE_SHIFT) + 1;
    else
        page_count = page_count >> SSV6006_HW_MMU_PAGE_SHIFT;

    return page_count;
}
static ssv_type_bool _ssv6006_do_firmware_checksum(ssv_type_u32 origin)
{
    #define RETRY_COUNT 100
    ssv_type_u32 retry=0;
    ssv_type_u32 fw_checksum=0;
	ssv_type_u32 fw_status = 0;

	origin = (((origin >> 24) + (origin >> 16) + (origin >> 8) + origin) & FW_CHK_SUM_SIZE);
    retry=0;
    do{

        MAC_REG_READ(FW_STATUS_REG,fw_status);

        if(fw_status & FW_STATUS_FW_CHKSUM_BIT)
        {
            fw_checksum = (fw_status & FW_CHK_SUM_MASK)>>16;


            if(gDeviceInfo->recovering != TRUE)
            {
                LOG_DEBUGF(LOG_TXRX,("%s(): fw check sum = 0x%x, check sum = 0x%x\r\n",__FUNCTION__, fw_checksum, origin));
            }

            if (fw_checksum == origin)
            {
                if(gDeviceInfo->recovering != TRUE)
                {
                    LOG_DEBUGF(LOG_TXRX,("%s(): [ssv] check sum is the same.\r\n",__FUNCTION__));
                }

                MAC_REG_WRITE(FW_STATUS_REG, (~origin & FW_STATUS_MASK));
                break;
            }
            else
            {
                LOG_PRINTF("%s(): [ssv] check sum is fail.\r\n",__FUNCTION__);
            }
        }

        retry++;
        OS_MsDelay(1);
    }while(retry!=RETRY_COUNT);

    if(retry==RETRY_COUNT)
    {
        LOG_PRINTF("%s(): [ssv] check sum is fail.sum = 0x%x, org = 0x%x\r\n",__FUNCTION__,fw_checksum, origin);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
extern ssv_type_u32 ssv6xxx_drv_write_fw_to_sram(ssv_type_u8 *fw_bin, ssv_type_u32 fw_bin_len, ssv_type_u32 block_size);
int ssv6006_hal_download_fw(ssv_type_u8 *fw_bin, ssv_type_u32 fw_bin_len)
{
    ssv_type_bool ret = FALSE;

#ifdef ENABLE_FW_SELF_CHECK
    ssv_type_u32   checksum = FW_CHECKSUM_INIT;
    ssv_type_u32   retry_count = 1;
#else
    int   writesize=0;
    ssv_type_u32   retry_count = 1;
#endif
    int block_count = 0;

    if((fw_bin==NULL)||(fw_bin_len==0))
    {
        LOG_PRINTF("%s():wrong input parameters\r\n",__FUNCTION__);
        return -1;
    }
#if FAST_CONNECT_PROFILE        
    LOG_PRINTF("\33[32mLoad FW %d\33[0m\r\n",OS_GetSysTick());
#endif

    do { //retry loop
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_DEBUGF(LOG_TXRX,("%s(): bin size=0x%x\r\n",__FUNCTION__,fw_bin_len));
        }

        ret=TRUE;

        // Before loading FW, reset N10
        _ssv6006_load_fw_disable_mcu();

        #if(CONFIG_CHIP_ID==SSV6006C)
        _ssv6006_set_sram_mode(SRAM_MODE_ILM_160K_DLM_32K);
        #endif
        
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_DEBUGF(LOG_TXRX,("%s(): Writing firmware to SSV6XXX...\r\n",__FUNCTION__));
        }
        checksum = FW_CHECKSUM_INIT;
        checksum += ssv6xxx_drv_write_fw_to_sram(fw_bin,fw_bin_len,FW_BLOCK_SIZE);
        //checksum = ((checksum >> 24) + (checksum >> 16) + (checksum >> 8) + checksum) & 0x0FF;
        //checksum <<= 16;
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_DEBUGF(LOG_TXRX,("%s(): checksum = 0x%x\r\n",__FUNCTION__, checksum));
        }

        block_count = fw_bin_len / FW_BLOCK_SIZE;
        block_count = ((fw_bin_len % FW_BLOCK_SIZE)> 0)?block_count+1:block_count;

        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_DEBUGF(LOG_TXRX,("%s(): block_count = 0x%x\r\n",__FUNCTION__, block_count));
        }

        if(ret==TRUE)
        {
            ret= MAC_REG_WRITE(FW_STATUS_REG, (block_count << 16));
        }
        //ssv_spi_read_reg(spi, FW_STATUS_REG, &tmp);
        // After FW loaded, set IVB to 0, boot from SRAM, enable N10 clock, and release N10
        _ssv6006_load_fw_enable_mcu();
        //OS_MsDelay(10);
        //printk(KERN_INFO "test output z to uart.\n");

        //ssv_spi_write_reg(spi, 0xc0000c00, 'z');
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_DEBUGF(LOG_TXRX,("%s(): Firmware \" loaded, checksum = %x\r\n",__FUNCTION__, checksum));
        }
        // Wait FW to calculate checksum.
        if((ret=_ssv6006_do_firmware_checksum(checksum))==TRUE)
        {
            ret=TRUE;
            break;
        }

    } while (--retry_count); //do { //retry loop
#if FAST_CONNECT_PROFILE
    if(ret==TRUE)
    {
        LOG_PRINTF("\33[32mLoad FW done %d\33[0m\r\n",OS_GetSysTick());
    }
#endif
    return (ret==TRUE)?0:-1;
}

int ssv6006_hal_tx_loopback_done(ssv_type_u8 *dat)
{
    #if 0
    OS_MemCPY(&tx_loopback_info,dat,sizeof(struct cfg_tx_loopback_info));

    OS_SemSignal(tx_loopback);

    if(!tx_loopback_info.result)
        LOG_PRINTF("\33[35mTX loopback fail\33[0m\r\n");
    #endif
    return 0;
}

static int _ssv6006_hal_if_enable_disable(ssv_type_bool val)
{
    //int regval;

	// set rf channel manual on
	//MAC_REG_READ(ADR_MODE_REGISTER, regval);// dummy read
    //MAC_REG_SET_BITS(ADR_MODE_REGISTER, 1 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_I_MSK);
    //OS_TickDelay(1); // for hs3w send

    if (val){
        // set RG_MODE to WIFI_RX
        //MAC_REG_READ(ADR_MODE_REGISTER, regval);// dummy read
        //MAC_REG_SET_BITS(ADR_MODE_REGISTER, 3 << RG_MODE_SFT, RG_MODE_I_MSK);
        SET_RG_SOFT_RST_N_11B_RX(1);
        SET_RG_SOFT_RST_N_11GN_RX(1);
        LOG_PRINTF("set RG_MODE to WIFI_RX\r\n");
	} else {
        // set RG_MODE to IDLE mode
        LOG_PRINTF("set RG_MODE to IDLE mode\r\n");
        //MAC_REG_READ(ADR_MODE_REGISTER, regval);// dummy read
        //MAC_REG_SET_BITS(ADR_MODE_REGISTER, 0 << RG_MODE_SFT, RG_MODE_I_MSK);
        
        SET_RG_SOFT_RST_N_11B_RX(0);
        SET_RG_SOFT_RST_N_11GN_RX(0);
	  }

    //OS_TickDelay(1); // for hs3w send

    //set RG_MODE_MANUAL off
    //MAC_REG_READ(ADR_MODE_REGISTER, regval);// dummy read
    //MAC_REG_SET_BITS(ADR_MODE_REGISTER, 0 << RG_MODE_MANUAL_SFT, RG_MODE_MANUAL_I_MSK);

	return true;

}
// define RF mode
#define RF_MODE_SHUTDOWN	0
#define RF_MODE_STANDBY   1
#define RF_MODE_TRX_EN    2
int ssv6006_hal_rf_enable(void)
{
    _ssv6006_hal_if_enable_disable(1);
    return 0;
}

int ssv6006_hal_rf_disable(void)
{
    _ssv6006_hal_if_enable_disable(0);
    return 0;
}

int ssv6006_hal_display_hw_queue_status(void)
{
    // 0 - MCU0
    // 1 - HCI
    // 2 -
    // 3 - Security
    // 4 - MRX
    // 5 - MIC
    // 6 - TX 0
    // 7 - TX 1
    // 8 - TX 2
    // 9 - TX 3
    // A - TX MNG
    // B - Security Sec
    // C - MIC Sec
    // D -
    // E -
    // F - Trash Can
    LOG_PRINTF("\n---HW Queue Status ---\r\n");
    LOG_PRINTF("[TAG]  MCU - HCI - SEC -  RX - MIC - TX0 - TX1 - TX2 - TX3 - TX4 - RX SEC - RX MIC - TSH\r\n");
    LOG_PRINTF("OUTPUT %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d   -   %3d -  %3d\r\n",
            GET_FFO0_CNT, GET_FFO1_CNT, GET_FFO3_CNT, GET_FFO4_CNT, GET_FFO5_CNT, GET_FFO6_CNT,
            GET_FFO7_CNT, GET_FFO8_CNT, GET_FFO9_CNT, GET_FFO10_CNT, GET_FFO11_CNT, GET_FFO12_CNT, GET_FFO15_CNT);
    LOG_PRINTF("INPUT  %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d   -   %3d -  %3d\r\n",
            GET_FF0_CNT, GET_FF1_CNT, GET_FF3_CNT, GET_FF4_CNT, GET_FF5_CNT, GET_FF6_CNT,
            GET_FF7_CNT, GET_FF8_CNT, GET_FF9_CNT, GET_FF10_CNT, GET_FF11_CNT, GET_FF12_CNT, GET_FF15_CNT);

    LOG_PRINTF("PAUSE                                %3d - %3d - %3d - %3d - %3d\r\n", GET_MTX_HALT_Q_MB&BIT(0),
                                                   (GET_MTX_HALT_Q_MB&BIT(1))>>1,
                                                   (GET_MTX_HALT_Q_MB&BIT(2))>>2,
                                                   (GET_MTX_HALT_Q_MB&BIT(3))>>3,
                                                   (GET_MTX_HALT_Q_MB&BIT(4))>>4);

    //used page
    LOG_PRINTF("TX used[%d], RX used[%d], AVA[%d],TX1=[%d]\r\n",GET_TX_ID_ALC_LEN,GET_RX_ID_ALC_LEN,GET_AVA_TAG,GET_FFO7_CNT);

    //used id
    //printf("TxID[%d]\n", GET_TX_ID_USE_5_0);
    LOG_PRINTF("ID  MAP[0x%08x%08x%08x%08x]\r\n", GET_ID_TB3, GET_ID_TB2, GET_ID_TB1, GET_ID_TB0);
    LOG_PRINTF("TX_ID MAP[0x%08x%08x%08x%08x]\r\n",GET_TX_ID_TB3, GET_TX_ID_TB2, GET_TX_ID_TB1, GET_TX_ID_TB0);
    LOG_PRINTF("RX_ID MAP[0x%08x%08x%08x%08x]\r\n", GET_RX_ID_TB3,GET_RX_ID_TB2, GET_RX_ID_TB1,GET_RX_ID_TB0);
    return 0;
}

ssv_type_bool ssv6006_hal_support_5g_band(void)
{
    ssv_type_u32 efuse_chip_id=0;
    efuse_chip_id = ssv6006_hal_read_chip_id();

/*
#define CHIP_ID_SV6155P 0x70000000
#define CHIP_ID_SV6156P 0x71000000
#define CHIP_ID_SV6166P 0x72000000
#define CHIP_ID_SV6167Q 0x74000000
#define CHIP_ID_SV6166F 0x75000000
#define CHIP_ID_SV6166M 0x76000000
*/
    if((efuse_chip_id==CHIP_ID_SV6155P)||
        (efuse_chip_id==CHIP_ID_SV6156P)||
        (efuse_chip_id==CHIP_ID_SV6166P)||        
        (efuse_chip_id==CHIP_ID_SV6167Q)||        
        (efuse_chip_id==CHIP_ID_SV6166F)||        
        (efuse_chip_id==CHIP_ID_SV6166FS)||
        (efuse_chip_id==CHIP_ID_SV6166M)||
        (efuse_chip_id==CHIP_ID_SV6151P)||
        (efuse_chip_id==CHIP_ID_SV6152P))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
typedef struct WSID_ENTRY_st
{
    ssv_type_u32     WINFO;
    ssv_type_u32     MAC00_31;
    ssv_type_u32     MAC32_47;

    struct {
    ssv_type_u32     UP_ACK_POLICY;
    ssv_type_u32     UP_TX_SEQCTL;
    } s[8];

    ssv_type_u32     RSVD;

} WSID_ENTRY, *PWSID_ENTRY;

ssv_type_u32 ssv6006_hal_get_ava_wsid(ssv_type_u32 init_id)
{
    ssv_type_u32 i,idx;
    ssv_type_u32 mac32[2] = {0};
    ssv_type_u32 WINFO =0;
    //WSID_ENTRY *pWSID=NULL;

AVA_WSID_RTY:
    for(i=init_id;i<SSV_MAX_WSID;i++)
    {
        idx=i;
        if(idx<2)
        {
            MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)),WINFO);
            MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
            MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)+8),mac32[1]);
        }
        else
        {
            idx-=2;
            MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)),WINFO);
            MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
            MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)+8),mac32[1]);
        }  
        mac32[1] &= 0x0000FFFF; 

        LOG_PRINTF("i=%d,WINFO=0x%x,mac32_0=0x%x,mac32_1=0x%x\r\n",i,WINFO,mac32[0],mac32[1]);
        if((mac32[0]==0)&&(mac32[1]==0))
        {
            idx = i;
            mac32[0] = 0xffffffff;
            if(idx<2)
            {
                MAC_REG_WRITE((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
            }
            else
            {
                idx-=2;
                MAC_REG_WRITE((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
            }  
            break;
        }
    }

    if((init_id!=0)&&(i==SSV_MAX_WSID)) //Check idx 0 is ava or not
    {
        init_id=0;
        goto AVA_WSID_RTY;
    }
    if(i<SSV_MAX_WSID)
        return i;
    else
        return 0xFFFFFFFF;
}

ssv_type_bool ssv6006_hal_check_wsid_free(ssv_type_u32 wsid)
{
    ssv_type_u32 idx;
    ssv_type_u32 mac32[2] = {0};
    ssv_type_u32 WINFO =0;
    idx = wsid;
    
    if(idx<2)
    {
        MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)),WINFO);
        MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
        MAC_REG_READ((ADR_WSID0+(sizeof(WSID_ENTRY)*idx)+8),mac32[1]);
    }
    else
    {
        idx-=2;
        MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)),WINFO);
        MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)+4),mac32[0]);
        MAC_REG_READ((ADR_WSID2+(sizeof(WSID_ENTRY)*idx)+8),mac32[1]);
    }  
    mac32[1] &= 0x0000FFFF; 

    if ((mac32[0] == 0x0) && (mac32[1] == 0x0))
    {
        return true;
    }
    return false;
}

#if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
int ssv6006_hal_update_cci_setting(ssv_type_u16 input_level)
{
    return -1;
}

int ssv6006_hal_reduce_phy_cca_bits(void)
{
    return -1;
}

int ssv6006_hal_recover_phy_cca_bits(void)
{
    return -1;
}

#else //#if(ENABLE_DYNAMIC_RX_SENSITIVE==0)
struct ssv6xxx_b_cca_control {
    ssv_type_u32 down_level;
    ssv_type_u32 upper_level;
    //u32 base_level;
    ssv_type_u32 adjust_cca_control;//ADR_WIFI_11B_RX_REG_040, 0xCCB0E8A0
    ssv_type_u32 adjust_cca_1;//ADR_WIFI_11GN_RX_REG_040, 0xCCB0F0A0
};

const struct ssv6xxx_b_cca_control adjust_cci[] = {
    {0 , 43, 0x01162000, 0x20000180},
    {40, 48, 0x01161000, 0x10000180},
    {45, 53, 0x01160800, 0x08000180},
    {50, 63, 0x01160400, 0x04000180},
    {60, 68, 0x01160200, 0x02000180},
    {65, 73, 0x01160100, 0x01000180},
    {70, 128, 0x00000000, 0x00000000},
};

#define MAX_CCI_LEVEL 128
ssv_type_bool cci_init=FALSE;
int ssv6006_hal_update_cci_setting(ssv_type_u16 input_level)
{
    #define ENABLE_CCI_DEBUG_MSG 1


    if (input_level > MAX_CCI_LEVEL) {
            LOG_PRINTF("%s:input error[%d]!!\r\n",__FUNCTION__,input_level);
            return -1;
    }

    if ((gDeviceInfo->cci_current_level == 0)&&(cci_init==FALSE)) {
            gDeviceInfo->cci_current_level = MAX_CCI_LEVEL;
            gDeviceInfo->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;
            MAC_REG_WRITE(ADR_WIFI_11B_RX_REG_040, adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control);
            MAC_REG_WRITE(ADR_WIFI_11GN_RX_REG_040, adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1);            
            cci_init=TRUE;
    }

    if(( input_level >= adjust_cci[gDeviceInfo->cci_current_gate].down_level) && (input_level <= adjust_cci[gDeviceInfo->cci_current_gate].upper_level)) {
            gDeviceInfo->cci_current_level = input_level;
#if ENABLE_CCI_DEBUG_MSG
            LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("Keep the 0xCCB0E8A0[%x] 0xCCB0F0A0[%x]!!(RCPI=%d)\r\n"
                ,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1,input_level));
#endif
        }
    else
    {
        // [current_level]30 -> [input_level]75
        if(gDeviceInfo->cci_current_level < input_level)
        {
            ssv_type_u32 i=0;
            for (i = 0; i < (sizeof(adjust_cci)/sizeof(adjust_cci[0])); i++) {
                if (input_level <= adjust_cci[i].upper_level) {

#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("gate=%d, input_level=%d, adjust_cci[%d].upper_level=%d, value=%08x\r\n",
                            gDeviceInfo->cci_current_gate, input_level, i, adjust_cci[i].upper_level, adjust_cci[i].adjust_cca_control));
#endif

                    gDeviceInfo->cci_current_level = input_level;
                    gDeviceInfo->cci_current_gate = i;

                    MAC_REG_WRITE(ADR_WIFI_11B_RX_REG_040, adjust_cci[i].adjust_cca_control);
                    MAC_REG_WRITE(ADR_WIFI_11GN_RX_REG_040, adjust_cci[i].adjust_cca_1);
#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("##Set to the 0xCCB0E8A0[%x] 0xCCB0F0A0[%x]##!!(RCPI=%d)\r\n"
                        ,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1,input_level));
#endif
                    return 0;
                }
            }
        }
        // [current_level]75 -> [input_level]30
        else
        {
            ssv_type_s32 i=0;
            for (i = (sizeof(adjust_cci)/sizeof(adjust_cci[0]) -1); i >= 0; i--) {
                if (input_level >= adjust_cci[i].down_level) {

#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("gate=%d, input_level=%d, adjust_cci[%d].down_level=%d, value=%08x \r\n",
                            gDeviceInfo->cci_current_gate, input_level, i, adjust_cci[i].down_level, adjust_cci[i].adjust_cca_control));
#endif

                    gDeviceInfo->cci_current_level = input_level;
                    gDeviceInfo->cci_current_gate = i;

                    MAC_REG_WRITE(ADR_WIFI_11B_RX_REG_040, adjust_cci[i].adjust_cca_control);
                    MAC_REG_WRITE(ADR_WIFI_11GN_RX_REG_040, adjust_cci[i].adjust_cca_1);
#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("##Set to the 0xCCB0E8A0[%x] 0xCCB0F0A0[%x]##!!(RCPI=%d)\r\n"
                        ,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1,input_level));
#endif
                    return 0;
                }
            }
        }
    }

    return 0;
}

//u32 gCE01000C=0;
int ssv6006_hal_reduce_phy_cca_bits(void)
{
#if 1
    ssv6006_hal_update_cci_setting(MIN_CCI_SENSITIVE);
    //MAC_REG_READ(0xCE01000C,gCE01000C);
    //MAC_REG_WRITE(0xCE01000C,(gCE01000C|(0x3F<<3)));
    return 0;
#else
    return 0;
#endif
}

int ssv6006_hal_recover_phy_cca_bits(void)
{

    if(gDeviceInfo->vif[0].m_info.StaInfo->status!=CONNECT)
    {
        ssv6006_hal_update_cci_setting(MAX_CCI_SENSITIVE);
    }
    else
    {

        ssv6006_hal_update_cci_setting(gDeviceInfo->cci_current_level);
    }

    //MAC_REG_WRITE(0xCE01000C,gCE01000C);
    return 0;
}
#endif //#if(ENABLE_DYNAMIC_RX_SENSITIVE==1)

int ssv6006_hal_read_hw_queue(void)
{
     // 0 - MCU0
    // 1 - HCI
    // 2 -
    // 3 - Security
    // 4 - MRX
    // 5 - MIC
    // 6 - TX 0
    // 7 - TX 1
    // 8 - TX 2
    // 9 - TX 3
    // A - TX MNG
    // B - Security Sec
    // C - MIC Sec
    // D -
    // E -
    // F - Trash Can
    LOG_PRINTF("\n---HW Queue Status ---\r\n");
    LOG_PRINTF("[TAG]  MCU - HCI - SEC -  RX - MIC - TX0 - TX1 - TX2 - TX3 - TX4 - RX SEC - RX MIC - TSH\r\n");
    LOG_PRINTF("OUTPUT %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d   -   %3d -  %3d\r\n",
            GET_FFO0_CNT, GET_FFO1_CNT, GET_FFO3_CNT, GET_FFO4_CNT, GET_FFO5_CNT, GET_FFO6_CNT,
            GET_FFO7_CNT, GET_FFO8_CNT, GET_FFO9_CNT, GET_FFO10_CNT, GET_FFO11_CNT, GET_FFO12_CNT, GET_FFO15_CNT);
    LOG_PRINTF("INPUT  %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d - %3d   -   %3d -  %3d\r\n",
            GET_FF0_CNT, GET_FF1_CNT, GET_FF3_CNT, GET_FF4_CNT, GET_FF5_CNT, GET_FF6_CNT,
            GET_FF7_CNT, GET_FF8_CNT, GET_FF9_CNT, GET_FF10_CNT, GET_FF11_CNT, GET_FF12_CNT, GET_FF15_CNT);

    LOG_PRINTF("PAUSE                                %3d - %3d - %3d - %3d - %3d\r\n", GET_MTX_HALT_Q_MB&BIT(0),
                                                   (GET_MTX_HALT_Q_MB&BIT(1))>>1,
                                                   (GET_MTX_HALT_Q_MB&BIT(2))>>2,
                                                   (GET_MTX_HALT_Q_MB&BIT(3))>>3,
                                                   (GET_MTX_HALT_Q_MB&BIT(4))>>4);

    //used page
    LOG_PRINTF("TX used[%d], RX used[%d], AVA[%d],TX1=[%d]\r\n",GET_TX_ID_ALC_LEN,GET_RX_ID_ALC_LEN,GET_AVA_TAG,GET_FFO7_CNT);

    //used id
    //printf("TxID[%d]\n", GET_TX_ID_USE_5_0);
    LOG_PRINTF("ID  MAP[0x%08x%08x%08x%08x]\r\n", GET_ID_TB3, GET_ID_TB2, GET_ID_TB1, GET_ID_TB0);
    LOG_PRINTF("TX_ID MAP[0x%08x%08x%08x%08x]\r\n",GET_TX_ID_TB3, GET_TX_ID_TB2, GET_TX_ID_TB1, GET_TX_ID_TB0);
    LOG_PRINTF("RX_ID MAP[0x%08x%08x%08x%08x]\r\n", GET_RX_ID_TB3,GET_RX_ID_TB2, GET_RX_ID_TB1,GET_RX_ID_TB0);

    return 0;
}

int ssv6006_hal_apply_ap_deci_tbl(void)
{
    /* By default, we apply ap decion table. */
    ssv_type_u16 *mac_deci_tbl=NULL;
    ssv_type_u8 i=0;
    mac_deci_tbl = ap_deci_tbl;

    for(i=0; i<SSV6006_MAC_DECITBL1_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(ssv_type_u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<SSV6006_MAC_DECITBL2_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,
        mac_deci_tbl[i+SSV6006_MAC_DECITBL1_SIZE]);
    }
    return 0;
}

int ssv6006_hal_apply_sta_deci_tbl(void)
{
    /* By default, we apply ap decion table. */
    ssv_type_u16 *mac_deci_tbl=NULL;
    ssv_type_u8 i=0;
    mac_deci_tbl = sta_deci_tbl;

    for(i=0; i<SSV6006_MAC_DECITBL1_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(ssv_type_u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<SSV6006_MAC_DECITBL2_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,
        mac_deci_tbl[i+SSV6006_MAC_DECITBL1_SIZE]);
    }
    return 0;
}

int ssv6006_hal_apply_generic_deci_tbl(void)
{
    /* By default, we apply ap decion table. */
    ssv_type_u16 *mac_deci_tbl=NULL;
    ssv_type_u8 i=0;
    mac_deci_tbl = generic_deci_tbl;

    for(i=0; i<SSV6006_MAC_DECITBL1_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(ssv_type_u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<(SSV6006_MAC_DECITBL2_SIZE+2); i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,
        mac_deci_tbl[i+SSV6006_MAC_DECITBL1_SIZE]);
    }
    return 0;
}


int ssv6006_hal_apply_null_deci_tbl(void)
{
    /* By default, we apply ap decion table. */
    ssv_type_u16 *mac_deci_tbl=NULL;
    ssv_type_u8 i=0;
    mac_deci_tbl = null_deci_tbl;

    for(i=0; i<SSV6006_MAC_DECITBL1_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(ssv_type_u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<SSV6006_MAC_DECITBL2_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,
        mac_deci_tbl[i+SSV6006_MAC_DECITBL1_SIZE]);
    }
    return 0;
}

int ssv6006_hal_get_b_mode_count(ssv_type_u32 *total_count, ssv_type_u32 *err_count)
{
    *err_count = GET_RO_11B_PACKET_ERR_CNT;
    *total_count = GET_RO_11B_PACKET_CNT;
    return 0;
}

int ssv6006_hal_get_gn_mode_count(ssv_type_u32 *total_count, ssv_type_u32 *err_count)
{
    *err_count = GET_RO_11GN_PACKET_ERR_CNT;
    *total_count = GET_RO_11GN_PACKET_CNT;
    return 0;
}


static int _ssv6006_hal_mib_inite(void)
{
    volatile ssv_type_u32 * pointer;

    SET_TRX_DEBUG_CNT_ENA(1);

    //Reset PHY MIB
    SET_RG_MRX_EN_CNT_RST_N(0);
    SET_RG_PACKET_STAT_EN_11B_RX(0);
    SET_RG_PACKET_STAT_EN_11GN_RX(0);

    SET_RG_MRX_EN_CNT_RST_N(1);
    SET_RG_PACKET_STAT_EN_11B_RX(1);   
    SET_RG_PACKET_STAT_EN_11GN_RX(1);     

    //Reset MAC MIB
    pointer = (ssv_type_u32 *)MIB_REG_BASE;
    *pointer = 0x00;
    *pointer = 0xffffffff;

    return 0;
}

int ssv6006_hal_mib_reset(void)
{
    _ssv6006_hal_mib_inite();
    LOG_PRINTF("MIB reset success!!\r\n");
    return 0;
}

int ssv6006_hal_mib_on(void)
{
    _ssv6006_hal_mib_inite();
    LOG_PRINTF("MIB on!!\r\n");
    return 0;
}

int ssv6006_hal_mib_off(void)
{
    volatile ssv_type_u32 * pointer;
    SET_TRX_DEBUG_CNT_ENA(0);
    
    pointer = (ssv_type_u32 *)MIB_REG_BASE;
    *pointer = 0x00;
    LOG_PRINTF("MIB off!!\r\n");
    return 0;
}

int ssv6006_hal_mib_show_rx(void)
{
    LOG_PRINTF("\r\nMIB RX Statistic:\r\n");

    LOG_PRINTF("MAC RX status:\r\n");
    LOG_PRINTF("    RX duplicated frame                 :%08d\r\n", GET_MRX_DUP);
    LOG_PRINTF("    RX fragment frame                   :%08d\r\n", GET_MRX_FRG);
    LOG_PRINTF("    RX group frame                      :%08d\r\n", GET_MRX_GRP);
    LOG_PRINTF("    RX fcs error frame                  :%08d\r\n", GET_MRX_FCS_ERR);
    LOG_PRINTF("    RX fcs success frame                :%08d\r\n", GET_MRX_FCS_SUC);
    LOG_PRINTF("    RX miss a pcaket from PHY frame     :%08d\r\n", GET_MRX_MISS);
    LOG_PRINTF("    Allocation failure                  :%08d\r\n", GET_MRX_ALC_FAIL);
    LOG_PRINTF("    RX ACK notify                       :%08d\r\n", GET_MRX_DAT_NTF);
    LOG_PRINTF("    RX rts frame notify                 :%08d\r\n", GET_MRX_RTS_NTF);
    LOG_PRINTF("    RX cts frame notify                 :%08d\r\n", GET_MRX_CTS_NTF);
    LOG_PRINTF("    RX receive ACK frames               :%08d\r\n", GET_MRX_ACK_NTF);
    LOG_PRINTF("    RX BA frame notify                  :%08d\r\n", GET_MRX_BA_NTF);
    LOG_PRINTF("    RX data frame notify                :%08d\r\n", GET_MRX_DATA_NTF);
    LOG_PRINTF("    RX manage frame notify              :%08d\r\n", GET_MRX_MNG_NTF);
    LOG_PRINTF("    RX ACK notify but crc error         :%08d\r\n", GET_MRX_DAT_CRC_NTF);
    LOG_PRINTF("    RX BAR frame notify                 :%08d\r\n", GET_MRX_BAR_NTF);
    LOG_PRINTF("    RX MBOX miss                        :%08d\r\n", GET_MRX_MB_MISS);
    LOG_PRINTF("    Not-in-IDLE                         :%08d\r\n", GET_MRX_NIDLE_MISS);
    LOG_PRINTF("    CSR notify                          :%08d\r\n", GET_MRX_CSR_NTF);

    LOG_PRINTF("PHY total Rx\t:[%08x]\r\n", GET_RO_MRX_EN_CNT );

    LOG_PRINTF("PHY B mode:\r\n");
    LOG_PRINTF("%-10s\t%-10s\t%-10s\t%-10s\t%-10s\r\n", "SFD_CNT","CRC_CNT","PKT_ERR","CCA","PKT_CNT");
    LOG_PRINTF("[%08x]\t", GET_RO_11B_SFD_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_11B_CRC_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_11B_PACKET_ERR_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_11B_CCA_CNT);
    LOG_PRINTF("[%08x]\t\r\n", GET_RO_11B_PACKET_CNT);

    LOG_PRINTF("PHY G/N mode:\r\n");

    LOG_PRINTF("%-10s\t%-10s\t%-10s\t%-10s\t%-10s\r\n","AMPDU ERR", "AMPDU PKT","PKT_ERR","CCA","PKT_CNT");
    LOG_PRINTF("[%08x]\t", GET_RO_AMPDU_PACKET_ERR_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_AMPDU_PACKET_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_11GN_PACKET_ERR_CNT);
    LOG_PRINTF("[%08x]\t", GET_RO_11GN_CCA_CNT);   
    LOG_PRINTF("[%08x]\t\r\n\r\n", GET_RO_11GN_PACKET_CNT);            
    return 0;
}

int ssv6006_hal_mib_show_tx(void)
{
    LOG_PRINTF("\r\nMIB TX Statistic:\r\n");

    LOG_PRINTF("    Tx Group            :%08d\r\n", GET_MTX_GRP);
    LOG_PRINTF("    Tx Fial             :%08d\r\n", GET_MTX_FAIL);
    LOG_PRINTF("    Tx Retry            :%08d\r\n", GET_MTX_RETRY);
    LOG_PRINTF("    Tx Multi Retry      :%08d\r\n", GET_MTX_MULTI_RETRY);
    LOG_PRINTF("    Tx RTS success      :%08d\r\n", GET_MTX_RTS_SUCC);
    LOG_PRINTF("    Tx RTS Fail         :%08d\r\n", GET_MTX_RTS_FAIL);
    LOG_PRINTF("    Tx ACK Fail         :%08d\r\n", GET_MTX_ACK_FAIL);
    LOG_PRINTF("    Tx total frame count:%08d\r\n", GET_MTX_FRM);
    LOG_PRINTF("    Tx ack frame count  :%08d\r\n", GET_MTX_ACK_TX);
    LOG_PRINTF("    Tx WSID-0 success   :%08d\r\n", GET_MTX_WSID0_SUCC);
    LOG_PRINTF("    Tx WSID-0 frame     :%08d\r\n", GET_MTX_WSID0_FRM);
    LOG_PRINTF("    Tx WSID-0 retry     :%08d\r\n", GET_MTX_WSID0_RETRY);
    LOG_PRINTF("    Tx WSID-0 Total     :%08d\r\n", GET_MTX_WSID0_TOTAL);
    return 0;
}

int ssv6006_hal_get_temperature(ssv_type_u8 *sar_code, signed char *temperature)
{
    int ret = -1;
	int value2 = 0;
    int i = 0;
    //ssv_type_u32 reg_value = 0;
    SET_RG_SARADC_THERMAL(1);

    SET_RG_EN_SARADC(1);
    for (i = 0; i < 100;i++)
    {
        //Sleep(1);
        if(GET_SAR_ADC_FSM_RDY)
        {
            ret = 0;
            break;
        }
    }

    if(-1==ret)
    {
        return -1;
    }
    value2 = GET_DB_DA_SARADC_BIT;
    *sar_code=value2;
    //printf("SARDAC = %d\n", value2);
    if((GET_CHIP_ID_63_32 == 0x30202020) && (GET_CHIP_ID_95_64 == 0x30303643) ) 
    {
         //TurismoC, Temperature 1
        *temperature = (value2 - 31)*2.5;
    }
    else if(GET_CHIP_ID_95_64 == 0x30303644)
    {
        //TurismoD and TurismoD ECO, Temperature 2
        *temperature = (value2-14)*3.05;
    }
    else if( (GET_CHIP_ID_63_32 == 0x32202020) && (GET_CHIP_ID_95_64 == 0x30303643))
    {
        //TurismoC ECO, Temperature 3
        //Need implement when get new sar code table
    }
    else
    {
        //undefined , use default 
        *temperature = (value2 - 31)*2.5;
    }
    
    SET_RG_SARADC_THERMAL(0);

    SET_RG_EN_SARADC(0);
    //printf("temperature = %d\n", *pvalue);
    
    return 0;
}

struct ssv_hal_ops	g_hal_ssv6006=
{
    "SSV6006",
    ssv6006_hal_chip_init,
    ssv6006_hal_init_mac,
    ssv6006_hal_init_sta_mac,
    ssv6006_hal_init_ap_mac,
    ssv6006_hal_ap_wep_setting,
    ssv6006_hal_tx_loopback_done,
    ssv6006_hal_add_interface,
    ssv6006_hal_remove_interface,
    NULL,//ssv6006_hal_setup_ampdu_wmm,
    ssv6006_hal_pbuf_alloc,
    ssv6006_hal_pbuf_free,
    ssv6006_hal_rf_enable,
    ssv6006_hal_rf_disable,
    NULL, //int (* rf_load_default_setting)(void);
    ssv6006_hal_watchdog_enable,
    ssv6006_hal_watchdog_disable,
    ssv6006_hal_mcu_enable,
    ssv6006_hal_mcu_disable,
    NULL, //int (* sw_reset)(u32 com);
    ssv6006_hal_gen_random,
    ssv6006_hal_promiscuous_enable,
    ssv6006_hal_promiscuous_disable,
    ssv6006_hal_read_chip_id,
    ssv6006_hal_read_efuse_macaddr,
    ssv6006_hal_write_efuse_macaddr,
    ssv6006_hal_dump_efuse_data,
    ssv6006_hal_efuse_read_usb_vid,
    ssv6006_hal_efuse_read_usb_pid,
    ssv6006_hal_efuse_read_tx_power1,
    ssv6006_hal_efuse_read_tx_power2,
    ssv6006_hal_efuse_read_xtal,
    ssv6006_hal_efuse_write_xtal,
    ssv6006_hal_efuse_write_rate_gain_b_n40,
    ssv6006_hal_efuse_read_rate_gain_b_n40,
    ssv6006_hal_efuse_write_rate_gain_g_n20,
    ssv6006_hal_efuse_read_rate_gain_g_n20,
    ssv6006_hal_get_diagnosis,
    ssv6006_hal_is_heartbeat,
    ssv6006_hal_reset_heartbeat,
    ssv6006_hal_get_fw_status,
    ssv6006_hal_set_fw_status,
    ssv6006_hal_reset_soc_irq,
    ssv6006_hal_set_gpio_output,
    ssv6006_hal_set_short_slot_time,
    ssv6006_hal_soc_set_bcn,
    ssv6006_hal_beacon_set,
    ssv6006_hal_beacon_enable,
    ssv6006_hal_is_beacon_enable,
    ssv6006_hal_get_tx_resources,
    ssv6006_hal_bytes_to_pages,
    ssv6006_hal_get_rssi_from_reg,
    ssv6006_hal_get_rc_info,
    ssv6006_hal_get_agc_gain,
    ssv6006_hal_set_agc_gain,
    ssv6006_hal_set_acs_agc_gain,
    ssv6006_hal_ap_listen_neighborhood,
    ssv6006_hal_reduce_phy_cca_bits,
    ssv6006_hal_recover_phy_cca_bits,
    ssv6006_hal_update_cci_setting,
    ssv6006_hal_set_ext_rx_int,
    ssv6006_hal_pause_resuem_recovery_int,
    NULL, //int (* set_TXQ_SRC_limit)(u32 qidx,u32 val);
    NULL, //int (* halt_txq)(u32 qidx,ssv_type_bool bHalt);
    ssv6006_hal_get_temperature,
    NULL, //int	(* set_voltage_mode)(u32 mode);
    ssv6006_hal_display_hw_queue_status,
    ssv6006_hal_support_5g_band,
    ssv6006_hal_get_ava_wsid,
    ssv6006_hal_check_wsid_free,
    ssv6006_hal_read_hw_queue,
    ssv6006_hal_l2_off,
    ssv6006_hal_l2_on,
    ssv6006_hal_get_b_mode_count,
    ssv6006_hal_get_gn_mode_count,
    ssv6006_hal_mib_reset,
    ssv6006_hal_mib_on,
    ssv6006_hal_mib_off,
    ssv6006_hal_mib_show_rx,
    ssv6006_hal_mib_show_tx,
    /** tx descriptor **/
    ssv6006_hal_dump_txinfo,
    ssv6006_hal_get_valid_txinfo_size,
    ssv6006_hal_get_txreq0_size,
    ssv6006_hal_get_txreq0_ctype,
    ssv6006_hal_set_txreq0_ctype,
    ssv6006_hal_get_txreq0_len,
    ssv6006_hal_set_txreq0_len,
    ssv6006_hal_get_txreq0_rsvd0,
    ssv6006_hal_set_txreq0_rsvd0,
    ssv6006_hal_get_txreq0_padding,
    ssv6006_hal_set_txreq0_padding,
    ssv6006_hal_get_txreq0_qos,
    ssv6006_hal_get_txreq0_ht,
    ssv6006_hal_get_txreq0_4addr,
    ssv6006_hal_set_txreq0_f80211,
    ssv6006_hal_get_txreq0_f80211,
    ssv6006_hal_set_txreq0_bssid_idx,
    ssv6006_hal_get_txreq0_bssid_idx,
    ssv6006_hal_get_txreq0_more_data,
    ssv6006_hal_set_txreq0_more_data,
    ssv6006_hal_get_txreq0_qos_ptr,
    ssv6006_hal_get_txreq0_data_ptr,
    ssv6006_hal_tx_8023to80211,
    ssv6006_hal_fill_txreq0,
    /** rx descriptor **/
    ssv6006_hal_rx_80211to8023,
    ssv6006_hal_dump_rxinfo,
    ssv6006_hal_get_rxpkt_size,
    ssv6006_hal_get_rxpkt_ctype,
    ssv6006_hal_get_rxpkt_len,
    ssv6006_hal_get_rxpkt_rcpi,
    ssv6006_hal_set_rxpkt_rcpi,
    ssv6006_hal_get_rxpkt_qos,
    ssv6006_hal_get_rxpkt_f80211,
    ssv6006_hal_get_rxpkt_psm,
    ssv6006_hal_get_rxpkt_channel,
    ssv6006_hal_get_rxpkt_wsid,
    ssv6006_hal_get_rxpkt_tid,
    ssv6006_hal_get_rxpkt_seqnum,
    ssv6006_hal_get_rxpkt_qos_ptr,
    ssv6006_hal_get_rxpkt_data_ptr,
    ssv6006_hal_get_rxpkt_data_len,
    ssv6006_hal_get_rxpkt_bssid_idx,
    /** HCI TRX Aggregation**/
    ssv6006_hal_process_hci_rx_aggr,
    ssv6006_hal_process_hci_aggr_tx,
    ssv6006_hal_hci_aggr_en,
    /** download fw **/
    ssv6006_hal_download_fw,
    /**decision table**/
    ssv6006_hal_accept_none_wsid_frame,
    ssv6006_hal_drop_none_wsid_frame,
    NULL,
    ssv6006_hal_sta_rcv_all_bcn,
    ssv6006_hal_sta_rcv_specific_bcn,
    ssv6006_hal_sta_reject_bcn,
    /** data flow**/
    ssv6006_hal_acs_rx_mgmt_flow,
    ssv6006_hal_ap_rx_mgmt_flow,
    ssv6006_hal_sconfig_rx_data_flow,
    ssv6006_hal_sta_rx_data_flow,
};
#endif //#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
