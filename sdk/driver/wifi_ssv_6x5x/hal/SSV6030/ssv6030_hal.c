#define SSV6030_HAL_C

#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <config.h>
#include <log.h>
#include <ap_info.h>
#include <dev.h>
#include "ssv6030_hal.h"
#include "regs/ssv6051_configuration.h"
#include "regs/ssv6051_reg.h"
#include "regs/ssv6051_aux.h"
#include "../ssv_hal_if.h"
#include "ssv6030_tbl.h"
#include "ssv6030_decision_tbl.h"
#include "ssv6030_data_flow.h"
#include "ssv6030_beacon.h"
#include "ssv6030_efuse.h"
#include "ssv6030_pkt.h"
#include <ssv_dev.h>
#include <hctrl.h>

#define FW_STATUS_REG           SSV6051_FW_STATUS_REG
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

extern struct Host_cfg g_host_cfg;

//===================HW related=============================
struct ssv6051_hci_txq_info {
	ssv_type_u32 tx_use_page:8;
    ssv_type_u32 tx_use_id:6;
    ssv_type_u32 txq0_size:4;
	ssv_type_u32 txq1_size:4;
	ssv_type_u32 txq2_size:5;
	ssv_type_u32 txq3_size:5;
};

OsSemaphore tx_loopback=0;
struct cfg_tx_loopback_info tx_loopback_info;

static ssv_type_bool _ssv6030_do_firmware_checksum(ssv_type_u32 origin);
#if(DO_IQ_CALIBRATION==1)
static int _ssv6030_hal_do_iq_calib(void);
#endif
static ssv_type_bool _ssv6030_hal_mcu_input_full(void);
static ssv_type_bool _ssv6030_set_hw_table(const ssv_cabrio_reg tbl_[], ssv_type_u32 size);

static ssv_type_bool _ssv6030_set_hw_table(const ssv_cabrio_reg tbl_[], ssv_type_u32 size)
{
    ssv_type_bool ret = FALSE ;
    ssv_type_u32 i=0;
    for(; i<size; i++) {
        ret = MAC_REG_WRITE(tbl_[i].address, tbl_[i].data);
        if (ret==FALSE) break;
    }
    return ret;
}

int ssv6030_hal_chip_init(void)
{
    ssv_type_bool ret;
    ssv_type_u32 reg;
#ifdef CONFIG_SSV_CABRIO_E
        ssv_type_u32 i,regval=0;
#endif

#ifdef CONFIG_SSV_CABRIO_E

        /*
                    Temp solution.Default 26M.
            */
        //priv->crystal_type = SSV6XXX_IQK_CFG_XTAL_26M;
        /*
            //Xctal setting
            Remodify RF setting For 24M 26M 40M or other xtals.
            */
        //if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_26M)
        {
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_26M;
            //printk("SSV6XXX_IQK_CFG_XTAL_26M\n");

            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6051_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00008000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000024;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00EC4CC5;
                }
            }
        }
        #if 0
        else if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_40M)
        {
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_40M;
            printk("SSV6XXX_IQK_CFG_XTAL_40M\n");
            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6xxx_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00000000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000030;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00EC4CC5;
                }
            }
        }
        else if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_24M)
        {
            printk("SSV6XXX_IQK_CFG_XTAL_24M\n");
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_24M;
            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6xxx_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00008000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000028;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00000000;
                }
            }
        }
        else
        {
            printk("Illegal xtal setting \n");
            BUG_ON(1);
        }
        #endif
#endif

    /*===Avoid crash at power on===*/
    // ## clock switch to XOSC
    //    SET_CK_SEL_1_0      (_CLK_SEL_MAC_XTAL_);
    MAC_REG_READ(ADR_CLOCK_SELECTION,reg);
    reg&=0xfffffffc;
    reg|=(_CLK_SEL_MAC_XTAL_<<0);
    MAC_REG_WRITE(ADR_CLOCK_SELECTION,reg);
    OS_MsDelay(10); //wait 10ms (OS_MsDelay at least 1ms)

    //SET_RG_RF_BB_CLK_SEL(_CLK_SEL_PHY_XTAL_);
    MAC_REG_READ(ADR_PHY_EN_0,reg);
    reg&=0x7fffffff;
    reg|=(_CLK_SEL_PHY_XTAL_<<31);
    MAC_REG_WRITE(ADR_PHY_EN_0,reg);

    MAC_REG_WRITE(0xce01005c, 0x80088208);//PLL power down
    MAC_REG_WRITE(0xce01005c, 0x00088008);//PLL power on
    OS_MsDelay(1); //wait 200us (OS_MsDelay at least 1ms)

//    execute in phy table
//    //SET_RG_RF_BB_CLK_SEL(_CLK_SEL_PHY_PLL_);
//    MAC_REG_READ(ADR_PHY_EN_0,reg);
//    reg&=0x7fffffff;
//    reg|=(_CLK_SEL_PHY_PLL_<<31);
//    MAC_REG_WRITE(ADR_PHY_EN_0,reg);
    //======================


    /* reset ssv6200 mac */

    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */


    //write rf table
    ret = _ssv6030_set_hw_table(ssv6200_rf_tbl,(sizeof(ssv6200_rf_tbl)/sizeof(ssv6200_rf_tbl[0])));

    if (ret == TRUE) ret = MAC_REG_WRITE( 0xce000004, 0x00000000); /* ???? */

    /* Turn off phy before configuration */

    //write phy table
    if (ret == TRUE) ret = _ssv6030_set_hw_table(ssv6200_phy_tbl,(sizeof(ssv6200_phy_tbl)/sizeof(ssv6200_phy_tbl[0])));

#ifdef CONFIG_SSV_CABRIO_E
    //Avoid SDIO issue.
	//Issue :CIT23 http://192.168.15.14:8080/browse/CIT-23
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_TRX_DUMMY_REGISTER, 0xEAAAAAAA);
    if (ret == TRUE)  MAC_REG_READ(ADR_TRX_DUMMY_REGISTER,regval);

    if(regval != 0xEAAAAAAA)
    {
        LOG_PRINTF("@@@@@@@@@@@@\r\n");
        LOG_PRINTF(" DUMMY_REGISTER access fail 0xCE01008C=%08x!!\r\n",regval);
        LOG_PRINTF(" It shouble be 0xEAAAAAAA!!\r\n");
        LOG_PRINTF("@@@@@@@@@@@@ \r\n");
        return -1;
    }
#endif


#ifdef CONFIG_SSV_CABRIO_E
    /* Cabrio E: GPIO setting */

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD53, 0x21);          //GPIO 5 -> UART TX
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD54, 0x3000);        //GPIO 6 -> UART RX
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PIN_SEL_0, 0x4000);    //UART RX SEL GPIO6

    //For CONFIG_CHIP_ID == SSV8031B
    //if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD25, 0x21);          //GPIO 1 -> UART TX
    //if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD28, 0x1008);        //GPIO 3 -> UART RX
    //if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PIN_SEL_0, 0x2000);    //UART RX SEL GPIO3

    /* TR switch: */
    if (ret == TRUE) ret = MAC_REG_WRITE(0xc0000304, 0x01);
    if (ret == TRUE) ret = MAC_REG_WRITE(0xc0000308, 0x01);

#endif // CONFIG_SSV_CABRIO_E

    //Switch clock to PLL output of RF
    //MAC and MCU clock selection :   00 : OSC clock   01 : RTC clock   10 : synthesis 80MHz clock   11 : synthesis 40MHz clock


    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_CLOCK_SELECTION, 0x3);


#ifdef CONFIG_SSV_CABRIO_E
        //Avoid consumprion of electricity
        //SDIO issue

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_TRX_DUMMY_REGISTER, 0xAAAAAAAA);
#endif

    //Reset this register to mark MCU is booting.
    //If just HW Reset this value will be 1.
    MAC_REG_WRITE(ADR_SYS_INT_FOR_HOST, 0);

    /* reset ssv6200 mac */
    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */

	if(ret == TRUE)
		return 0;
	else
		return -1;

}


ssv_type_bool cabrio_resource_setup(ssv_type_u8 param)
{
    ssv_type_bool ret = FALSE;
    ssv_type_u32 id_len = 0;
    ssv_type_u8 rx_id_threshold = SSV6051_ID_RX_THRESHOLD;
    ssv_type_u8 rx_page_threshold = SSV6051_PAGE_RX_THRESHOLD;

    MAC_REG_READ(ADR_TRX_ID_THRESHOLD,id_len);
    if (param == 1)
    {
        rx_id_threshold = SSV6051_ID_RX_THRESHOLD_LOW;
        rx_page_threshold = SSV6051_PAGE_RX_THRESHOLD_LOW;
    }

    id_len = (id_len&0xffff0000 ) |
            (SSV6051_ID_TX_THRESHOLD<<TX_ID_THOLD_SFT)|
            (rx_id_threshold<<RX_ID_THOLD_SFT);
    ret = MAC_REG_WRITE(ADR_TRX_ID_THRESHOLD, id_len);

    if(ret != TRUE)
    {
        LOG_PRINTF("Failed to update ID resource for pre-allocate frame\r\n");
        return ret;
    }

    MAC_REG_READ(ADR_ID_LEN_THREADSHOLD1,id_len);
    id_len = (id_len&0x0f )|
            (SSV6051_PAGE_TX_THRESHOLD<<ID_TX_LEN_THOLD_SFT)|
            (rx_page_threshold<<ID_RX_LEN_THOLD_SFT);
    ret = MAC_REG_WRITE(ADR_ID_LEN_THREADSHOLD1, id_len);

    if(ret != TRUE)
        LOG_PRINTF("Failed to update PAGE resource for pre-allocate frame\r\n");

    return ret;
}

int ssv6030_hal_set_ext_rx_int(ssv_type_u32 pin);
int ssv6030_hal_set_gpio_output(ssv_type_bool en, ssv_type_u32 pin);
int ssv6030_hal_init_mac(ssv_type_u8 *self_mac)
{
    int i;
    ssv_type_bool ret;
    ssv_type_u32 regval,j;
    char    chip_id[24]="";
    ssv_type_u32     chip_tag1,chip_tag2;
    ssv_type_u32 *ptr;
    ssv_type_u32 hw_buf_ptr;
    ssv_type_u32 hw_sec_key;
    ssv_type_u32 hw_pinfo;

    if(0==tx_loopback)
    {
        OS_SemInit(&tx_loopback, 1, 0);
    }

    //CHIP TAG

    MAC_REG_READ(ADR_IC_TIME_TAG_1,regval);
    chip_tag1 = regval;
    MAC_REG_READ(ADR_IC_TIME_TAG_0,regval);
    chip_tag2= regval;
    //LOG_DEBUG("CHIP TAG: %llx \r\n",chip_tag);

    //CHIP ID
    MAC_REG_READ(ADR_CHIP_ID_3,regval);
    *((ssv_type_u32 *)&chip_id[0]) = (ssv_type_u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_2,regval);
    *((ssv_type_u32 *)&chip_id[4]) = (ssv_type_u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_1,regval);
    *((ssv_type_u32 *)&chip_id[8]) = (ssv_type_u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_0,regval);

    *((ssv_type_u32 *)&chip_id[12]) = (ssv_type_u32)LONGSWAP(regval);
    if(gDeviceInfo->recovering != TRUE)
    {
        LOG_DEBUG("CHIP ID: %s \r\n", chip_id);
        LOG_DEBUG("RF TABLE: %s ,%d\r\n", chip_sel->chip_str,chip_sel->chip_id);
    }
    //soc set HDR-STRIP-OFF       enable
    //soc set HCI-RX2HOST         enable
    //soc set AUTO-SEQNO          enable
    //soc set ERP-PROTECT          disable
    //soc set MGMT-TXQID            3
    //soc set NONQOS-TXQID      1
    regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|HDR_STRIP_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);
    ret = MAC_REG_WRITE(ADR_CONTROL,regval);

    MAC_REG_READ(ADR_HCI_TX_RX_INFO_SIZE,regval);
    regval&=RX_INFO_SIZE_I_MSK;
    regval|=(SSV6051_RXINFO_SIZE<<RX_INFO_SIZE_SFT);
    ret = MAC_REG_WRITE(ADR_HCI_TX_RX_INFO_SIZE,regval);

    /* Enable hardware timestamp for TSF */
    // 28 => time stamp write location
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_TIME_STAMP_CFG,((28<<MRX_STP_OFST_SFT)|0x01));

    //MMU[decide packet buffer no.]
    /* Setting MMU to 256 pages */
//    MAC_REG_READ(ADR_MMU_CTRL, regval);
//    regval |= (0xff<<MMU_SHARE_MCU_SFT);
//    MAC_REG_WRITE(ADR_MMU_CTRL, regval);

    /**
        * Tx/RX threshold setting for packet buffer resource.
        */

    ret = cabrio_resource_setup(g_host_cfg.pre_alloc_prb_frm);
#ifndef __SSV_UNIX_SIM__
    //update_tx_resource
    OS_MUTEX_LOCK(txsrcMutex);
    ssv6030_hal_get_tx_resources(&tx_rcs.free_page,&tx_rcs.free_id,&tx_rcs.free_space);
    OS_MUTEX_UNLOCK(txsrcMutex);
#endif
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_STA_MAC_0, *((ssv_type_u32 *)&(self_mac[0])));
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_STA_MAC_1, *((ssv_type_u32 *)&(self_mac[4])));
    /**
        * Reset all wsid table entry to invalid.
        */
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID1, 0x00000000);

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_SDIO_MASK, 0xf7ffffff);

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


//-----------------------------------------------------------------------------------------------------------------------------------------
//PHY and security table
    /**
        * Allocate a hardware packet buffer space. This buffer is for security
        * key caching and phy info space.
        */
    /*lint -save -e732  Loss of sign (assignment) (int to unsigned int)*/
    hw_buf_ptr = ssv_hal_pbuf_alloc((ssv_type_s32)sizeof(phy_info_tbl)+
                                            (ssv_type_s32)sizeof(struct ssv6xxx_hw_sec),(int)NOTYPE_BUF);
   /*lint -restore */
    if((hw_buf_ptr>>28) != 8)
    {
    	//asic pbuf address start from 0x8xxxxxxxx
    	LOG_PRINTF("opps allocate pbuf error\n");
    	//WARN_ON(1);
    	ret = 1;
    	goto exit;
    }

    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("%s(): ssv6200 reserved space=0x%08x, size=%d\r\n",
        __FUNCTION__, hw_buf_ptr, (ssv_type_u32)(sizeof(phy_info_tbl)+sizeof(struct ssv6xxx_hw_sec)));



/**
Part 1. SRAM
	**********************
	*				          *
	*	1. Security key table *
	* 				          *
	* *********************
	*				          *
	*    	2. PHY index table     *
	* 				          *
	* *********************
	* 				          *
	*	3. PHY ll-length table *
	*				          *
	* *********************
=============================================
Part 2. Register
	**********************
	*				          *
	*	PHY Infor Table         *
	* 				          *
	* *********************
*
*/

    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    hw_sec_key = hw_buf_ptr;

	//==>Section 1. Write Sec table to SRAM
    for(j=0; j<sizeof(struct ssv6xxx_hw_sec); j+=4) {
        MAC_REG_WRITE(hw_sec_key+j, 0);
    }
    /*lint -save -e838*/
    regval = ((hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
    MAC_REG_READ(ADR_SCRT_SET, regval);
	regval &= SCRT_PKT_ID_I_MSK;
	regval |= ((hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
	MAC_REG_WRITE(ADR_SCRT_SET, regval);
    /*lint -restore*/

    /**
        * Set default ssv6200 phy infomation table.
        */
    hw_pinfo = hw_sec_key + sizeof(struct ssv6xxx_hw_sec);
    for(i=0, ptr=(ssv_type_u32 *)phy_info_tbl; i<PHY_INFO_TBL1_SIZE; i++, ptr++) {
        regval = *ptr;
        if(g_host_cfg.tx_power_mode == TX_POWER_ENHANCE_ALL)
        {
            regval = (regval&0x03FFFFFF);
            regval |= tx_rf_gain_enhance_table[i]<<26;
        }
        MAC_REG_WRITE(ADR_INFO0+(ssv_type_u32)i*4, regval);
    }


	//==>Section 2. Write PHY index table and PHY ll-length table to SRAM
	for(i=0; i<PHY_INFO_TBL2_SIZE; i++, ptr++) {
        MAC_REG_WRITE(hw_pinfo+(ssv_type_u32)i*4, *ptr);
    }
    for(i=0; i<PHY_INFO_TBL3_SIZE; i++, ptr++) {
        MAC_REG_WRITE(hw_pinfo+(PHY_INFO_TBL2_SIZE<<2)+(ssv_type_u32)i*4, *ptr);
    }


    MAC_REG_WRITE(ADR_INFO_RATE_OFFSET, 0x00040000);

	//Set SRAM address to register
	MAC_REG_READ(ADR_INFO_IDX_ADDR, regval);
    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("ADR_INFO_IDX_ADDR:%08x\r\n",regval);
	MAC_REG_WRITE(ADR_INFO_IDX_ADDR, hw_pinfo);
    MAC_REG_WRITE(ADR_INFO_LEN_ADDR, hw_pinfo+(PHY_INFO_TBL2_SIZE)*4); //4byte for one entry
    MAC_REG_READ(ADR_INFO_IDX_ADDR, regval);
    if(gDeviceInfo->recovering != TRUE)
	    LOG_PRINTF("ADR_INFO_IDX_ADDR[%08x] ADR_INFO_LEN_ADDR[%08x]\r\n", regval, hw_pinfo+(PHY_INFO_TBL2_SIZE)*4);

    //-----------------------------------------------------------------------------------------------------------------------------------------

    if(ret == TRUE) ret = MAC_REG_WRITE(0xca000800,0xffffffff);

    if(ret == TRUE) ret = MAC_REG_WRITE(0xCE000004,0x0000017F);//PHY b/g/n on

    //-----------------------------------------------------------------------------------------------------------------------------------------
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //C_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);

    /* Setup q4 behavior STA mode-> act as normal queue
      *
      */
        MAC_REG_READ(ADR_MTX_BCN_EN_MISC,regval);
        regval&= ~(MTX_HALT_MNG_UNTIL_DTIM_MSK);
        regval |= (0);
    if(ret == TRUE) ret = MAC_REG_WRITE( ADR_MTX_BCN_EN_MISC, regval);

    //-----------------------------------------------------------------------------------------------------------------------------------------

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_INFO_RATE_OFFSET, 0x00040000);

    exit:
    //Load FW
    {
        if(platform_download_firmware() == FALSE)
            return 1;
    }
    if(!g_host_cfg.extRxInt)
        ssv6030_hal_set_gpio_output(0,g_host_cfg.extRxInt);
    else
        ssv6030_hal_set_ext_rx_int(g_host_cfg.rxIntGPIO);

    ssv6xxx_drv_irq_enable(false);
    //Set watchdoginterface
    {
        unsigned char cmd_data[] = {
            0x00, 0x00, 0x00, 0x00};
        cmd_data[0]= RECOVER_ENABLE;
        cmd_data[1]= RECOVER_MECHANISM;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_RECOVER, cmd_data, 4, TRUE, FALSE);
    }
    return ((ret == TRUE)?0:1);
}

int ssv6030_hal_init_sta_mac(ssv_type_u32 wifi_mode)
{
    int i, ret=0;
    ssv_type_u32 regval;
    ssv_type_u16 *mac_deci_tbl;

    if(SSV6XXX_HWM_STA == wifi_mode)
    {
        MAC_REG_WRITE(ADR_GLBLE_SET,
            //(0 << OP_MODE_SFT)  |                           /* STA mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            //(1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            (SSV6051_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6051_TX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
        );
    }
    else
    {
            MAC_REG_WRITE(ADR_GLBLE_SET,
            //(0 << OP_MODE_SFT)  |                           /* STA mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            (1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            (SSV6051_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6051_TX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
        );
    }
    MAC_REG_WRITE(ADR_MRX_LEN_FLT,0x900);                   /*Prevent from RX hang because of receiving large frame*/

    MAC_REG_WRITE(ADR_BSSID_0,   0x00000000);//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   0x00000000);//*((u32 *)&priv->bssid[4]));

     //trap null data
    //SET_RX_NULL_TRAP_EN(1)
    //MAC_REG_SET_BITS(ADR_CONTROL,1 << RX_NULL_TRAP_EN_SFT,RX_NULL_TRAP_EN_MSK);
    {
        ssv_type_u32 tarpEnable;
        MAC_REG_READ(ADR_CONTROL,tarpEnable);
        tarpEnable = tarpEnable & RX_NULL_TRAP_EN_I_MSK;
        tarpEnable |= (1 << RX_NULL_TRAP_EN_SFT);
        MAC_REG_WRITE(ADR_CONTROL, tarpEnable);
    }
     /**
        * Set reason trap to discard frames.
        */
    //
    //MAC_REG_WRITE(ADR_REASON_TRAP0, 0x7FBC7F87);
    //Drop null data frame
    MAC_REG_WRITE(ADR_REASON_TRAP0,0x7FBC3F87);
    MAC_REG_WRITE(ADR_REASON_TRAP1, 0x00000000);

    cabrio_init_sta_mode_data_flow();

    //#set EDCA parameter AP-a/g BK[0], BE[1], VI[2], VO[3]
    //soc set WMM-PARAM[0]      { aifsn=0 acm=0 cwmin=5 cwmax=10 txop=0 backoffvalue=6 }


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,

          (6 << TXQ0_MTX_Q_AIFSN_SFT)  |                           /* aifsn=7 */
          (4 << TXQ0_MTX_Q_ECWMIN_SFT) |                            /*cwmin=4 */
          (10 << TXQ0_MTX_Q_ECWMAX_SFT)                           /* cwmax=10 */

    );


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);

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

          (1 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
          (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
          (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
          (65535 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );


    /* By default, we apply staion decion table. */
    mac_deci_tbl= (ssv_type_u16 *)sta_deci_tbl;

    for(i=0; i<SSV6051_MAC_DECITBL1_SIZE; i++) {

        MAC_REG_WRITE((ADR_MRX_FLT_TB0+(ssv_type_u32)i*4),(ssv_type_u32 )(mac_deci_tbl[i]));

    }
    for(i=0; i<SSV6051_MAC_DECITBL2_SIZE; i++) {

        MAC_REG_WRITE(ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,

        mac_deci_tbl[i+SSV6051_MAC_DECITBL1_SIZE]);

    }
#ifdef CONFIG_SSV_CABRIO_E
        /* Do RF-IQ cali. */
        #if(DO_IQ_CALIBRATION==1)
        if(_ssv6030_hal_do_iq_calib()!=SSV6XXX_SUCCESS)
            return SSV6XXX_FAILED;
        #endif
#endif // CONFIG_SSV_CABRIO_E

    MAC_REG_READ(ADR_PHY_EN_1,regval);
    MAC_REG_WRITE(ADR_PHY_EN_1,(regval|(1<<13)));
    
#if(SSV_IPD==1)
    {
        const unsigned char cmd_data=1;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_IPD, (void *)&cmd_data, sizeof(cmd_data), TRUE, FALSE);
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("SSV_IPD\r\n");
    }
#endif

    ssv6xxx_wifi_set_channel(STA_DEFAULT_CHANNEL,SSV6XXX_HWM_STA);

    ssv6030_hal_set_volt_mode(g_host_cfg.volt_mode);

    return ret;
}

int ssv6030_hal_setup_ampdu_wmm(ssv_type_bool IsAMPDU)
{
    if(IsAMPDU == TRUE)
    {
#if (AUTO_BEACON != 1)
        MAC_REG_WRITE( ADR_TXQ0_MTX_Q_AIFSN,
              (2 << TXQ0_MTX_Q_AIFSN_SFT)  |                   /* aifsn=1 */
              (1 << TXQ0_MTX_Q_ECWMIN_SFT) |                    /*cwmin=0 */
              (1 << TXQ0_MTX_Q_ECWMAX_SFT) |                  /* cwmax=0 */
              (65535 << TXQ0_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
        );
#endif
        MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,

                  (2 << TXQ1_MTX_Q_AIFSN_SFT)  |                       /* aifsn=3 */
                  (4 << TXQ1_MTX_Q_ECWMIN_SFT) |                        /*cwmin=4 */
                  (6 << TXQ1_MTX_Q_ECWMAX_SFT) |                     /* cwmax=6 */
                  (0 << TXQ1_MTX_Q_TXOP_LIMIT_SFT)                         /*  txop=0 */
        );

        MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,

                  (2 << TXQ2_MTX_Q_AIFSN_SFT)  |                       /* aifsn=3 */
                  (4 << TXQ2_MTX_Q_ECWMIN_SFT) |                        /*cwmin=4 */
                  (6 << TXQ2_MTX_Q_ECWMAX_SFT) |                       /* cwmax=6 */
                  (0 << TXQ2_MTX_Q_TXOP_LIMIT_SFT)                        /*  txop=0 */
        );
    }
    else
    {
#if (AUTO_BEACON != 1)
        MAC_REG_WRITE( ADR_TXQ0_MTX_Q_AIFSN,
              (1 << TXQ0_MTX_Q_AIFSN_SFT)  |                   /* aifsn=1 */
              (0 << TXQ0_MTX_Q_ECWMIN_SFT) |                    /*cwmin=0 */
              (0 << TXQ0_MTX_Q_ECWMAX_SFT) |                  /* cwmax=0 */
              (65535 << TXQ0_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
        );
#endif
        MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,

                  (0 << TXQ1_MTX_Q_AIFSN_SFT)  |                       /* aifsn=1 */
                  (2 << TXQ1_MTX_Q_ECWMIN_SFT) |                        /*cwmin=2 */
                  (3 << TXQ1_MTX_Q_ECWMAX_SFT) |                     /* cwmax=3 */
                  (47 << TXQ1_MTX_Q_TXOP_LIMIT_SFT)                         /*  txop=47 */
        );

        MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,

                  (0 << TXQ2_MTX_Q_AIFSN_SFT)  |                       /* aifsn=1 */
                  (2 << TXQ2_MTX_Q_ECWMIN_SFT) |                        /*cwmin=2 */
                  (3 << TXQ2_MTX_Q_ECWMAX_SFT) |                       /* cwmax=3 */
                  (47 << TXQ2_MTX_Q_TXOP_LIMIT_SFT)                        /*  txop=47 */
        );
    }

    return 0;
}

int ssv6030_hal_init_ap_mac(ssv_type_u8 *bssid,ssv_type_u8 channel)
{
    int i, ret=0;
    ssv_type_u32 temp;
    ssv_type_u16 *mac_deci_tbl;
    ssv_type_u32 regval;
    MAC_REG_WRITE(ADR_GLBLE_SET,
            (1 << OP_MODE_SFT)  |                           /* AP mode by default */
            //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
            //(1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
            //(1 << AMPDU_SNIFFER_SFT) |                      /*Prevent from RX hang because of receiving AMPDU*/
            (SSV6051_TX_PKT_RSVD_SETTING << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
            (SSV6051_RX_PB_OFFSET << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
    );
    MAC_REG_WRITE(ADR_MRX_LEN_FLT,0x900);                   /*Prevent from RX hang because of receiving large frame*/

    MAC_REG_READ(ADR_SCRT_SET,temp);
    temp = temp & SCRT_RPLY_IGNORE_I_MSK;
    temp |= (1 << SCRT_RPLY_IGNORE_SFT);
    MAC_REG_WRITE(ADR_SCRT_SET, temp);

    MAC_REG_WRITE(ADR_BSSID_0,   *((ssv_type_u32 *)&bssid[0]));//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   *((ssv_type_u32 *)&bssid[4]));//*((u32 *)&priv->bssid[4]));

    cabrio_init_ap_mode_data_flow();

     //soc set REASON-TRAP         { 0x7fbf7f8f 0xffffffff }
    {

    MAC_REG_WRITE(ADR_REASON_TRAP0,0x7FBF7F87);
    MAC_REG_WRITE(ADR_REASON_TRAP1,0xFFFFFFFF);

    }

    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);

    //Update wmm parameters of q1 and q1, default as AMPDU on
    ssv6030_hal_setup_ampdu_wmm(TRUE);

    //soc set WMM-PARAM[3]      { aifsn=0 acm=0 cwmin=2 cwmax=3 txop=47 backoffvalue=3 }
    {
        //info 845: The right argument to operator '|' is certain to be 0

        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,

              (0 << TXQ3_MTX_Q_AIFSN_SFT)  |                   /* aifsn=1 */
              (2 << TXQ3_MTX_Q_ECWMIN_SFT) |                    /*cwmin=2 */
              (3 << TXQ3_MTX_Q_ECWMAX_SFT) |                  /* cwmax=3 */
              (47 << TXQ3_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=47 */
        );


        //MAC_REG_WRITE( ADR_TXQ3_MTX_Q_BKF_CNT,0x00000003);

    }



#if (AUTO_BEACON == 1)
    //#set EDCA parameter AP-a/g BK[0], BE[1], VI[2], VO[3]
    //soc set WMM-PARAM[0]      { aifsn=0 acm=0 cwmin=5 cwmax=10 txop=0 backoffvalue=6 }


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,

          (0 << TXQ0_MTX_Q_AIFSN_SFT)  |                           /* aifsn=1 */
          (2 << TXQ0_MTX_Q_ECWMIN_SFT) |                           /*cwmin=2 */
          (3 << TXQ0_MTX_Q_ECWMAX_SFT) |                           /* cwmax=3 */
          (47 << TXQ0_MTX_Q_TXOP_LIMIT_SFT )                       /*  txop=47 */

    );


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);
#else //#if (AUTO_BEACON == 1)


    /* Set wmm parameter to EDCA Q0
        (use to send beacon frame in AP mode when AUTO_BEACON is 0) */
    {
        //MTX_Q_ECWMIN MTX_Q_ECWMAX are ignored when MTX_Q_BKF_CNT_FIXED = 1
        MAC_REG_READ(ADR_TXQ0_MTX_Q_MISC_EN,temp);
        MAC_REG_WRITE(ADR_TXQ0_MTX_Q_MISC_EN, (1<<TXQ0_MTX_Q_BKF_CNT_FIXED_SFT) |temp );
        // force back off count to MTX_Q_BKF_CNT value
        MAC_REG_WRITE( ADR_TXQ0_MTX_Q_BKF_CNT,0x00000000);

        MAC_REG_WRITE( ADR_TXQ0_MTX_Q_AIFSN,
              (1 << TXQ0_MTX_Q_AIFSN_SFT)  |                   /* aifsn=1 */
              (0 << TXQ0_MTX_Q_ECWMIN_SFT) |                    /*cwmin=0 */
              (0 << TXQ0_MTX_Q_ECWMAX_SFT) |                  /* cwmax=0 */
              (65535 << TXQ0_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
        );

    }
#endif //#if (AUTO_BEACON == 1)

    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //MAC_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
        /*lint -save -e648 Overflow in computing constant for operation:    'shift left'*/
        MAC_REG_WRITE( ADR_TXQ4_MTX_Q_AIFSN,

          (1 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
          (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
          (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
          (65535 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );
    /*lint -restore*/





    /* By default, we apply ap decion table. */

    mac_deci_tbl = (ssv_type_u16 *)ap_deci_tbl;

    for(i=0; i<SSV6051_MAC_DECITBL1_SIZE; i++) {

        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(ssv_type_u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<SSV6051_MAC_DECITBL2_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(ssv_type_u32)i*4,

        mac_deci_tbl[i+SSV6051_MAC_DECITBL1_SIZE]);

    }
    //trap null data
    //SET_RX_NULL_TRAP_EN(1)
    //MAC_REG_SET_BITS(ADR_CONTROL,1 << RX_NULL_TRAP_EN_SFT,RX_NULL_TRAP_EN_MSK);
    MAC_REG_READ(ADR_CONTROL,temp);
    temp = temp & RX_NULL_TRAP_EN_I_MSK;
    temp |= (1 << RX_NULL_TRAP_EN_SFT);
    MAC_REG_WRITE(ADR_CONTROL, temp);


#ifdef CONFIG_SSV_CABRIO_E
        /* Do RF-IQ cali. */
        #if(DO_IQ_CALIBRATION==1)
        if(_ssv6030_hal_do_iq_calib()!=SSV6XXX_SUCCESS)
            return SSV6XXX_FAILED;
        #endif
#endif // CONFIG_SSV_CABRIO_E


    MAC_REG_READ(ADR_PHY_EN_1,regval);
    MAC_REG_WRITE(ADR_PHY_EN_1,(regval|(1<<13)));
#if(SSV_IPD==1)
    {
        const unsigned char cmd_data=1;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_IPD, (void *)&cmd_data, sizeof(cmd_data), TRUE, FALSE);
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("SSV_IPD\r\n");
    }
#endif


    ssv6xxx_wifi_set_channel(gDeviceInfo->APInfo->nCurrentChannel,SSV6XXX_HWM_AP);

    ssv6030_hal_set_volt_mode(g_host_cfg.volt_mode);

    return ret;
}

int ssv6030_hal_ap_wep_setting(ssv6xxx_sec_type sec_type, ssv_type_u8 *password, ssv_type_u8 vif_idx, ssv_type_u8* sta_mac_addr)
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
int ssv6030_hal_promiscuous_enable(void)
{
    ssv_type_u32 regval;
    //regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|            (3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)
    //regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|HDR_STRIP_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);
    MAC_REG_READ(ADR_CONTROL,regval);
    regval&=HDR_STRIP_I_MSK;
    regval&=RX_ETHER_TRAP_EN_I_MSK;
    MAC_REG_WRITE(ADR_CONTROL,regval);;

    return 0;
}

int ssv6030_hal_promiscuous_disable(void)
{
    ssv_type_u32 regval;
    //regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|            (3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)
    //regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|HDR_STRIP_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);
    MAC_REG_READ(ADR_CONTROL,regval);
    regval|=(HDR_STRIP_MSK|RX_ETHER_TRAP_EN_MSK);
    MAC_REG_WRITE(ADR_CONTROL,regval);

    return 0;
}

int ssv6030_hal_watchdog_enable(void)
{
    MAC_REG_WRITE(ADR_SYS_WDOG_REG,1<<31); //    SET_SYS_WDOG_ENA(1);
    return 0;
}

int ssv6030_hal_watchdog_disable(void)
{
    MAC_REG_WRITE(ADR_SYS_WDOG_REG,0);  //SET_SYS_WDOG_ENA(0);
    return 0;
}

int ssv6030_hal_mcu_enable(void)
{
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x1);
    return 0;
}

int ssv6030_hal_mcu_disable(void)
{
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x0);
    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */
    return 0;
}

int ssv6030_hal_sw_reset(ssv_type_u32 comp)
{
    switch (comp)
    {
        case SW_RESET_MAC:
            MAC_REG_WRITE(ADR_BRG_SW_RST, MAC_SW_RST);
            break;
        case SW_RESET_MCU:
            MAC_REG_WRITE(ADR_BRG_SW_RST, MCU_SW_RST);
            break;
        case SW_RESET_SDIO:
            MAC_REG_WRITE(ADR_BRG_SW_RST, SDIO_SW_RST);
            break;
        case SW_RESET_SPI_SLV:
            MAC_REG_WRITE(ADR_BRG_SW_RST, SPI_SLV_SW_RST);
            break;
        case SW_RESET_SYS_ALL:
            MAC_REG_WRITE(ADR_BRG_SW_RST, SYS_ALL_RST);
            break;
    }
    return 0;
}

int ssv6030_hal_set_gpio_output(ssv_type_bool en, ssv_type_u32 pin)
{
//#define SET_PAD20_OD(_VAL_) (REG32(ADR_PAD20))= (((_VAL_) << 8)  | ((REG32(ADR_PAD20))& 0xfffffeff))
    ssv_type_u32 tmp;

    MAC_REG_WRITE(ADR_PAD20, 0x81); //output low
    tmp = ((MAC_REG_READ(ADR_PAD20,tmp))& 0xfffffeff);
    if(en)
        tmp |= (1<<8);

    MAC_REG_WRITE(ADR_PAD20, tmp);
    //OS_MsDelay(10);
    return 0;
}

int ssv6030_hal_set_ext_rx_int(ssv_type_u32 pin)
{
    MAC_REG_WRITE(ADR_SYS_INT_FOR_HOST,0x00000000);
    MAC_REG_WRITE(ADR_PAD20,0x00000001);
    return 0;
}

int ssv6030_hal_pause_resuem_recovery_int(ssv_type_bool resume)
{
    ssv_type_u32 val;
    
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

    return 0;
}

int ssv6030_hal_set_TXQ_SRC_limit(ssv_type_u32 qidx,ssv_type_u32 val)
{
    ssv_type_u32 tmp;
    if(val < 1 || val > 7)
        return;

    if(qidx < 1 || qidx > 4)
        return;

    val++;
    val = (val | ((MAC_REG_READ((ADR_TXQ1_MTX_Q_RC_LIMIT+0x100*(qidx-1)),tmp))& 0xffffff00));
    MAC_REG_WRITE((ADR_TXQ1_MTX_Q_RC_LIMIT+0x100*(qidx-1)),val);

    LOG_PRINTF("TXQ%d RC limit =%x\r\n",qidx,(MAC_REG_READ((ADR_TXQ1_MTX_Q_RC_LIMIT+0x100*(qidx-1)),val)));
    return 0;
}

int ssv6030_hal_halt_txq(ssv_type_u32 qidx,ssv_type_bool bHalt)
{
    ssv_type_u32 regval;
    
    MAC_REG_READ(ADR_MTX_MISC_EN, regval);
	if(EDCA_AC_ALL == qidx)
    {   
        regval = regval &0xffc0ffff;
		if(TRUE==bHalt)
        {
            regval = (regval | (0x3F<<16));
        }
    }
	else
	{
        regval = regval & ((ssv_type_u32)~(1 << (16+qidx)));
        regval = regval | ((bHalt) << (16+qidx));
	}
    //LOG_PRINTF("halt_tx_queue,regval=0x%x\r\n",regval);
    MAC_REG_WRITE(ADR_MTX_MISC_EN, regval);
    return 0;
}

static ssv_inline void _SET_RG_SARADC_THERMAL(ssv_type_u32 _VAL_)  
{
    ssv_type_u32 value = 0;
    MAC_REG_READ(0xce010030, value);
    value = (((_VAL_) << 26) | ((value)& 0xfbffffff));
    MAC_REG_WRITE(0xce010030, value);
}

static ssv_inline void _SET_RG_EN_SARADC(ssv_type_u32 _VAL_)
{ 
    ssv_type_u32 value = 0; 
    MAC_REG_READ(0xce010030, value); 
    value = (((_VAL_) << 30) | ((value)& 0xbfffffff)); 
    MAC_REG_WRITE(0xce010030, value); 
} 

static ssv_inline ssv_type_u32 _GET_SAR_ADC_FSM_RDY() 
{
    ssv_type_u32 value = 0; 
    MAC_REG_READ(0xce010094, value);
    value = ((value & 0x00400000 ) >> 22);
    return value;
}

static ssv_inline ssv_type_u32 _GET_RG_SARADC_BIT() 
{
    ssv_type_u32 value = 0;
    MAC_REG_READ(0xce010094, value);
    value = ((value& 0x003f0000 ) >> 16);
    return value;
}

static signed char sarTotemperature[]={
        -25, // 0
        -25, // 1
        -25, // 2
        -25, // 3
        -25, // 4
        -20, // 5
        -17, // 6
        -15, // 7
        -13, // 8
        -10, // 9
        -5, // 10
        -3, // 11
        0,  // 12
        2,  // 13        
        5,  // 14
        7,  // 15        
        10, // 16
        15, // 17
        17, // 18        
        20, // 19
        22, // 20        
        25, // 21
        30, // 22
        32, // 23        
        35, // 24
        37, // 25        
        40, // 26
        42, // 27       
        45, // 28
        50, // 29
        52, // 30        
        55, // 31  
        60, // 32
        62, // 33        
        65, // 34
        67, // 35        
        70, // 36 
        75, // 37
        77, // 38        
        80, // 39
        82, // 40        
        85, // 41
        87, // 42        
        90, // 43
        95, // 44
        97, // 45        
        100, // 46
        102, // 47        
        105, // 48
        110, // 49
        112, // 50        
        115, // 51
        117, // 52        
        120  // 53        
};
int ssv6030_hal_get_temperature(ssv_type_u8 *sar_code, signed char *temperature)
{
    ssv_type_u32 _sar_code=0;
    _SET_RG_SARADC_THERMAL(1);      // ce010030[26]
    _SET_RG_EN_SARADC(1);           // ce010030 [30]
    while(!_GET_SAR_ADC_FSM_RDY());   // ce010094[23]
    _sar_code =  _GET_RG_SARADC_BIT(); // ce010094[21:16]
    _SET_RG_SARADC_THERMAL(0);
    _SET_RG_EN_SARADC(0);    
    
    *sar_code=(ssv_type_u8)_sar_code;
    if(_sar_code < sizeof(sarTotemperature))
        *temperature=sarTotemperature[_sar_code];
    else
        *temperature=120;
    return 0;
}

int ssv6030_hal_set_volt_mode(ssv_type_u32 mode)
{
    if(mode == VOLT_DCDC_CONVERT)
    {
        MAC_REG_WRITE(0xC0001D08, 0x00000001);   //DCDC
    }
    else if(mode == VOLT_LDO_REGULATOR)
    {
        MAC_REG_WRITE(0xC0001D08, 0x00000000);   //LDO
    }
    //MAC_REG_READ(0xC0001D08,rtemp);
    LOG_PRINTF("ssv6030_hal_set_volt_mode=%d\r\n",mode);
    return 0;
}

int ssv6030_hal_gen_random(ssv_type_u8 *data, ssv_type_u32 len)
{
    ssv_type_u32 idx=0;
    ssv_type_u32 rtemp;
    MAC_REG_WRITE(ADR_RAND_EN,1);
    MAC_REG_READ(ADR_RAND_NUM,rtemp);
    for(idx=0;idx<len;idx+=4){
        MAC_REG_READ(ADR_RAND_NUM,rtemp);
        *(data+idx)=rtemp&0xff;
        *(data+1+idx)=(rtemp>>8)&0xff;
        *(data+2+idx)=(rtemp>>16)&0xff;
        *(data+3+idx)=(rtemp>>24)&0xff;
    }
    MAC_REG_WRITE(ADR_RAND_EN,0);
    return 0;
}

int ssv6030_hal_get_rssi_from_reg(ssv_type_u8 vif_idx)
{
    ssv_type_u32 regVal=0;
    MAC_REG_READ(ADR_DBG_SPI_TO_PHY_PARAM2, regVal);
    regVal=(regVal&0xffff0000)>>16;
    return regVal;
}

int ssv6030_hal_get_agc_gain(void)
{
    ssv_type_u32 regVal=0;
    MAC_REG_READ(ADR_PHY_REG_03_AGC, regVal);
    return regVal;
}

int ssv6030_hal_set_agc_gain(ssv_type_u32 gain)
{
    MAC_REG_WRITE(ADR_PHY_REG_03_AGC,gain);
    return 0;
}

int ssv6030_hal_set_acs_agc_gain(void)
{
    ssv6030_hal_set_agc_gain(SSV6051_ACS_AGC_GAIN);
    return 0;
}

int ssv6030_hal_get_rc_info(ssv_type_u16 *tx_cnt, ssv_type_u16 *retry_cnt, ssv_type_u16 *phy_rate)
{
    ssv_type_u32 regval = 0;

    MAC_REG_READ(ADR_DBG_SPI_TO_PHY_PARAM1, regval);
    MAC_REG_WRITE(ADR_DBG_SPI_TO_PHY_PARAM1, 0);
    *tx_cnt = regval&0x0000ffff;
    *retry_cnt = (regval&0xffff0000)>>16;
    MAC_REG_READ(ADR_DBG_SPI_TO_PHY_PARAM2, regval);
    *phy_rate = (ssv_type_u16)regval;
    return 0;
}


ssv_type_bool ssv6030_hal_get_diagnosis(void)
{
    ssv_type_u32 SysIrq=0;
    ssv_type_u32 Wdg=0;
    ssv_type_u32 ChipId0;
    ssv_type_u32 ChipId2;
    MAC_REG_READ(ADR_SYS_INT_FOR_HOST,(SysIrq));
    MAC_REG_READ(ADR_SYS_WDOG_REG,(Wdg));
    MAC_REG_READ(ADR_CHIP_ID_0,(ChipId0));
    MAC_REG_READ(ADR_CHIP_ID_2,(ChipId2));

    if ((SysIrq&0x1)&&(Wdg==0)&&
            (ChipId0&0x31333131)&&(ChipId2&0x32303041)) // watchdog wack up & reset
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int ssv6030_hal_is_heartbeat(void)
{
    ssv_type_u32 isr_status;
    MAC_REG_READ(ADR_SDIO_IRQ_STS,isr_status);
    return (isr_status>>27)&0x01;
}

int ssv6030_hal_reset_heartbeat(void)
{
    MAC_REG_WRITE(ADR_SDIO_IRQ_STS,(1<<27));
    return 0;
}

int ssv6030_hal_get_fw_status(ssv_type_u32 *val)
{
    MAC_REG_READ(SSV6051_FW_STATUS_REG,(*val));
    return 0;
}

int ssv6030_hal_set_fw_status(ssv_type_u32 val)
{
    MAC_REG_WRITE(SSV6051_FW_STATUS_REG,(val));
    return 0;
}

int ssv6030_hal_reset_soc_irq(void)
{
    MAC_REG_WRITE(ADR_SYS_INT_FOR_HOST,0); //  system reset interrupt for host
    return 0;
}

static ssv_type_bool _ssv6030_hal_mcu_input_full(void)
{
    ssv_type_u32 regval=0;
    MAC_REG_READ(ADR_MCU_STATUS, regval);
    return CH0_FULL_MSK&regval;
}


int ssv6030_hal_pbuf_free(int addr)
{
    ssv_type_u32  regval=0;
    ssv_type_u16  failCount=0;

    while (_ssv6030_hal_mcu_input_full())
    {
        if (failCount++ < 1000)
            continue;
        LOG_PRINTF("=============>ERROR!!MAILBOX Block[%d]\n", failCount);
        return -1;
    } //Wait until input queue of cho is not full.

    // {HWID[3:0], PKTID[6:0]}
    regval = ((M_ENG_TRASH_CAN << SSV6051_HW_ID_OFFSET) |(addr >> SSV6051_ADDRESS_OFFSET));

    MAC_REG_WRITE(ADR_CH0_TRIG_1, regval);

    return 0;
}

int ssv6030_hal_pbuf_alloc(int size, int type)
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

int ssv6030_hal_set_short_slot_time(ssv_type_bool enable)
{
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
    return 0;
}

int ssv6030_hal_get_tx_resources(ssv_type_u16 *pFreePages, ssv_type_u16 *pFreeIDs, ssv_type_u16 *pFreeSpaces)
{
    struct ssv6051_hci_txq_info *pInfo=NULL;
    ssv_type_u32 regVal;
    ssv_type_u16 free_page = 0;
    ssv_type_u16 free_id = 0 ;
    ssv_type_u16 free_spaces=0;

    MAC_REG_READ(ADR_TX_ID_ALL_INFO,regVal);
    pInfo=(struct ssv6051_hci_txq_info *)&regVal;
    free_page = SSV6051_PAGE_TX_THRESHOLD-pInfo->tx_use_page;
    free_id = SSV6051_ID_TX_THRESHOLD-pInfo->tx_use_id;
     free_spaces=M_ENG_HWHCI_INT-GET_FF1_CNT; //how many space of HCI input queue for incoming packets
    if( free_page<=SSV6051_PAGE_TX_THRESHOLD && free_id<=SSV6051_ID_TX_THRESHOLD && free_spaces<=M_ENG_HWHCI_INT)
    {
        *pFreePages = free_page;
        *pFreeIDs = free_id;
        *pFreeSpaces = free_spaces;
    }
    else
    {
        *pFreePages = 0;
        *pFreeIDs = 0;
        *pFreeSpaces = 0;
    }

    return 0;
}

int ssv6030_hal_bytes_to_pages(ssv_type_u32 size)
{
    ssv_type_u32 page_count=0;
    page_count= (size + SSV6051_ALLOC_RSVD);

    if (page_count & SSV6051_HW_MMU_PAGE_MASK)
        page_count = (page_count >> SSV6051_HW_MMU_PAGE_SHIFT) + 1;
    else
        page_count = page_count >> SSV6051_HW_MMU_PAGE_SHIFT;

    return page_count;
}

static ssv_type_bool _ssv6030_do_firmware_checksum(ssv_type_u32 origin)
{
    #define RETRY_COUNT 20
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
                LOG_PRINTF("%s(): fw check sum = 0x%x, check sum = 0x%x\r\n",__FUNCTION__, fw_checksum, origin);
            }

            if (fw_checksum == origin)
            {
                if(gDeviceInfo->recovering != TRUE)
                {
                    LOG_PRINTF("%s(): [ssv] check sum is the same.\r\n",__FUNCTION__);
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
        OS_MsDelay(50);
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

int ssv6030_hal_download_fw(ssv_type_u8 *fw_bin, ssv_type_u32 fw_bin_len)
{
    ssv_type_bool ret = FALSE;

#ifdef ENABLE_FW_SELF_CHECK
    ssv_type_u32   checksum = FW_CHECKSUM_INIT;
    ssv_type_u32   retry_count = 1;
#else
    int   writesize=0;
    ssv_type_u32   retry_count = 1;
#endif
    ssv_type_u32 clk_en;
    int block_count = 0;

    if((fw_bin==NULL)||(fw_bin_len==0))
    {
        LOG_PRINTF("%s():wrong input parameters\r\n",__FUNCTION__);
        return -1;
    }

    do { //retry loop
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("%s(): bin size=0x%x\r\n",__FUNCTION__,fw_bin_len);
        }

        ret=TRUE;

        if(ret==TRUE)
        {
            ret = MAC_REG_WRITE(ADR_BRG_SW_RST, 0x0);
        }

        if(ret==TRUE)
        {
            ret = MAC_REG_WRITE(ADR_BOOT, 0x01);
        }

        if(ret==TRUE)
        {
            MAC_REG_READ(ADR_PLATFORM_CLOCK_ENABLE, clk_en);
        }

        if(ret==TRUE)
        {
            ret = MAC_REG_WRITE(ADR_PLATFORM_CLOCK_ENABLE, clk_en | (1 << 2));
        }

        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("%s(): Writing firmware to SSV6XXX...\r\n",__FUNCTION__);
        }
        checksum = FW_CHECKSUM_INIT;
        checksum += ssv6xxx_drv_write_fw_to_sram(fw_bin,fw_bin_len,FW_BLOCK_SIZE);
        //checksum = ((checksum >> 24) + (checksum >> 16) + (checksum >> 8) + checksum) & 0x0FF;
        //checksum <<= 16;
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("%s(): checksum = 0x%x\r\n",__FUNCTION__, checksum);
        }

        block_count = fw_bin_len / FW_BLOCK_SIZE;
        block_count = ((fw_bin_len % FW_BLOCK_SIZE)> 0)?block_count+1:block_count;
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("%s(): block_count = 0x%x\r\n",__FUNCTION__, block_count);
        }

        if(ret==TRUE)
        {
            ret= MAC_REG_WRITE(FW_STATUS_REG, (block_count << 16));
        }
        //ssv_spi_read_reg(spi, FW_STATUS_REG, &tmp);

        if(ret==TRUE)
        {
            ret = MAC_REG_WRITE(ADR_BRG_SW_RST, 0x01);
        }
        //OS_MsDelay(10);
        //printk(KERN_INFO "test output z to uart.\n");

        //ssv_spi_write_reg(spi, 0xc0000c00, 'z');
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("%s(): Firmware \" loaded, checksum = %x\r\n",__FUNCTION__, checksum);
        }
        // Wait FW to calculate checksum.
        if((ret=_ssv6030_do_firmware_checksum(checksum))==TRUE)
        {
            ret=TRUE;
            break;
        }

    } while (--retry_count); //do { //retry loop

    return (ret==TRUE)?0:-1;
}

#if(DO_IQ_CALIBRATION==1)
extern const ssv_rf_temperature rf_temper_setting[RF_TEMPER_ARRARY_SIZE];
struct ssv6xxx_iqk_cfg {
    ssv_type_u32 cfg_xtal:8;
    ssv_type_u32 cfg_pa:8;
    ssv_type_u32 cfg_pabias_ctrl:8;
    ssv_type_u32 cfg_pacascode_ctrl:8;
    ssv_type_u32 cfg_tssi_trgt:8;
    ssv_type_u32 cfg_tssi_div:8;
    ssv_type_u32 cfg_def_tx_scale_11b:8;
    ssv_type_u32 cfg_def_tx_scale_11b_p0d5:8;
    ssv_type_u32 cfg_def_tx_scale_11g:8;
    ssv_type_u32 cfg_def_tx_scale_11g_p0d5:8;
    ssv_type_u32 cmd_sel;
    union {
        ssv_type_u32 fx_sel;
        ssv_type_u32 argv;
    }un;
    ssv_type_u32 phy_tbl_size;
    ssv_type_u32 rf_tbl_size;
    ssv_type_u32 rf_temper_tbl_size;
};

#define IQK_CFG_LEN         (sizeof(struct ssv6xxx_iqk_cfg))
#define RF_SETTING_SIZE     (sizeof(asic_rf_setting))

#define PHY_SETTING_SIZE sizeof(phy_setting)

/*
    If change defallt value .please recompiler firmware image.
*/
#define MAX_PHY_SETTING_TABLE_SIZE    1920
#define MAX_RF_SETTING_TABLE_SIZE    512

typedef enum {
    SSV6XXX_IQK_CFG_XTAL_26M = 0,
    SSV6XXX_IQK_CFG_XTAL_40M,
    SSV6XXX_IQK_CFG_XTAL_24M,
    SSV6XXX_IQK_CFG_XTAL_MAX
} ssv6xxx_iqk_cfg_xtal;

typedef enum {
    SSV6XXX_IQK_CFG_PA_DEF = 0,
    SSV6XXX_IQK_CFG_PA_LI_MPB,
    SSV6XXX_IQK_CFG_PA_LI_EVB,
    SSV6XXX_IQK_CFG_PA_HP
} ssv6xxx_iqk_cfg_pa;

typedef enum {
    SSV6XXX_IQK_CMD_INIT_CALI = 0,
    SSV6XXX_IQK_CMD_RTBL_LOAD,
    SSV6XXX_IQK_CMD_RTBL_LOAD_DEF,
    SSV6XXX_IQK_CMD_RTBL_RESET,
    SSV6XXX_IQK_CMD_RTBL_SET,
    SSV6XXX_IQK_CMD_RTBL_EXPORT,
    SSV6XXX_IQK_CMD_TK_EVM,
    SSV6XXX_IQK_CMD_TK_TONE,
    SSV6XXX_IQK_CMD_TK_CHCH,
    SSV6XXX_IQK_CMD_TK_RXCNT
} ssv6xxx_iqk_cmd_sel;

#define SSV6XXX_IQK_TEMPERATURE 0x00000004
#define SSV6XXX_IQK_RXDC        0x00000008
#define SSV6XXX_IQK_RXRC        0x00000010
#define SSV6XXX_IQK_TXDC        0x00000020
#define SSV6XXX_IQK_TXIQ        0x00000040
#define SSV6XXX_IQK_RXIQ        0x00000080
#define SSV6XXX_IQK_TSSI        0x00000100
#define SSV6XXX_IQK_PAPD        0x00000200

static int _ssv6030_hal_do_iq_calib(void)
{
#define IQK_ALIGN_SIZE(size) (((size) + 4 - 1) & ~(4-1))

    struct ssv6xxx_iqk_cfg init_iqk_cfg;
    void          *frame;
    struct cfg_host_cmd     *host_cmd;
    ssv_type_u32 i=0;
    ssv_type_u8 *pwValue=NULL;
    ssv_type_u32 hwPBUF=0, wAddr=0;
    ssv_type_u32 len=0;
    ssv_type_u32 fw_status=0;
    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("# Do init_cali (iq)\r\n");
    if((PHY_SETTING_SIZE > MAX_PHY_SETTING_TABLE_SIZE) ||
        (RF_SETTING_SIZE > MAX_RF_SETTING_TABLE_SIZE))
    {
        LOG_PRINTF("Please recheck RF or PHY table size!!!\n");
        return 0;
    }

    //Load PHY/RF/RF Temper table
    len=IQK_ALIGN_SIZE(PHY_SETTING_SIZE) + IQK_ALIGN_SIZE(RF_SETTING_SIZE)+ IQK_ALIGN_SIZE(RF_TEMPER_SETTING_SIZE);
    LOG_PRINTF("PHY_SETTING_SIZE(%d,%d)\r\n",PHY_SETTING_SIZE,IQK_ALIGN_SIZE(PHY_SETTING_SIZE));
    LOG_PRINTF("RF_SETTING_SIZE(%d,%d)\r\n",RF_SETTING_SIZE,IQK_ALIGN_SIZE(RF_SETTING_SIZE));    
    LOG_PRINTF("RF_TEMPER_SETTING_SIZE(%d,%d)\r\n",RF_TEMPER_SETTING_SIZE,IQK_ALIGN_SIZE(RF_TEMPER_SETTING_SIZE));        
    
    hwPBUF=(ssv_type_u32)ssv_hal_pbuf_alloc(len,TX_BUF);
    if((ssv_type_s32)hwPBUF==-1)
    {
        LOG_PRINTF("init _ssv6xxx_do_iq_calib fail!!!\n");
        return 0;
    }
    
    LOG_PRINTF("HW PBUF Addr(0x%x) for PHY/RF/RF Temper table\r\n",(ssv_type_u32)hwPBUF);
    
    wAddr=hwPBUF;
    pwValue=(ssv_type_u8 *)phy_setting;
    for(i=0;i<IQK_ALIGN_SIZE(PHY_SETTING_SIZE);i+=4)
    {
        MAC_REG_WRITE(wAddr,*((ssv_type_u32 *)pwValue));
        wAddr+=4;
        pwValue+=4;
    }

    wAddr=hwPBUF+IQK_ALIGN_SIZE(PHY_SETTING_SIZE);
    pwValue=(ssv_type_u8 *)asic_rf_setting;
    for(i=0;i<IQK_ALIGN_SIZE(RF_SETTING_SIZE);i+=4)
    {
        MAC_REG_WRITE(wAddr,*((ssv_type_u32 *)pwValue));
        wAddr+=4;
        pwValue+=4;
    }

    wAddr=hwPBUF+IQK_ALIGN_SIZE(PHY_SETTING_SIZE)+IQK_ALIGN_SIZE(RF_SETTING_SIZE);
    pwValue=(ssv_type_u8 *)rf_temper_setting;
    for(i=0;i<IQK_ALIGN_SIZE(RF_TEMPER_SETTING_SIZE);i+=4)
    {
        MAC_REG_WRITE(wAddr,*((ssv_type_u32 *)pwValue));
        wAddr+=4;
        pwValue+=4;
    }    

    // make command packet
    frame = (void*)OS_MemAlloc(HOST_CMD_HDR_LEN + IQK_CFG_LEN+4); //last 4 bytes are for PHY/RF/RF Temper table address

    if(frame == NULL)
    {
        LOG_PRINTF("init _ssv6xxx_do_iq_calib fail!!!\n");
        ssv_hal_pbuf_free(hwPBUF);
        return 0;
    }

    host_cmd = (struct cfg_host_cmd *)frame;
    host_cmd->c_type = HOST_CMD;
    host_cmd->h_cmd  = (ssv_type_u8)SSV6XXX_HOST_CMD_INIT_CALI;
    host_cmd->len    = HOST_CMD_HDR_LEN + IQK_CFG_LEN+4;

    init_iqk_cfg.cfg_xtal=SSV6XXX_IQK_CFG_XTAL_26M;
#ifdef CONFIG_SSV_DPD
    init_iqk_cfg.cfg_pa=SSV6XXX_IQK_CFG_PA_LI_MPB;
#else
    init_iqk_cfg.cfg_pa=SSV6XXX_IQK_CFG_PA_DEF;
#endif
    init_iqk_cfg.cfg_pabias_ctrl = 0;
    init_iqk_cfg.cfg_pacascode_ctrl = 0;
    init_iqk_cfg.cfg_tssi_trgt=26;
    init_iqk_cfg.cfg_tssi_div=3;

    if(chip_sel->chip_id == SSV6051Z)
    {
        init_iqk_cfg.cfg_def_tx_scale_11b=(wifi_tx_gain[3]>>0) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11b_p0d5=(wifi_tx_gain[3]>>8) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g=(wifi_tx_gain[6]>>16) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g_p0d5=(wifi_tx_gain[6]>>24) & 0xff;
    }
    else if(chip_sel->chip_id == SSV6030P)//0x798067c2
    {
        init_iqk_cfg.cfg_def_tx_scale_11b=(wifi_tx_gain[3]>>0) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11b_p0d5=(wifi_tx_gain[3]>>8) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g=(wifi_tx_gain[1]>>16) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g_p0d5=(wifi_tx_gain[1]>>24) & 0xff;
    }
    else if(chip_sel->chip_id == SSV6052Q)//0x80808080
    {
        init_iqk_cfg.cfg_def_tx_scale_11b=(wifi_tx_gain[0]>>0) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11b_p0d5=(wifi_tx_gain[0]>>8) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g=(wifi_tx_gain[0]>>16) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g_p0d5=(wifi_tx_gain[0]>>24) & 0xff;
    }
    else
    {
        init_iqk_cfg.cfg_def_tx_scale_11b=(wifi_tx_gain[6]>>0) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11b_p0d5=(wifi_tx_gain[6]>>8) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g=(wifi_tx_gain[4]>>16) & 0xff;
        init_iqk_cfg.cfg_def_tx_scale_11g_p0d5=(wifi_tx_gain[4]>>24) & 0xff;
    }
    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("%x,%x,%x,%x,Fill tx gain\r\n",
        init_iqk_cfg.cfg_def_tx_scale_11b,
        init_iqk_cfg.cfg_def_tx_scale_11b_p0d5,
        init_iqk_cfg.cfg_def_tx_scale_11g,
        init_iqk_cfg.cfg_def_tx_scale_11g_p0d5);

    init_iqk_cfg.cmd_sel=SSV6XXX_IQK_CMD_INIT_CALI;
#ifdef CONFIG_SSV_DPD
    init_iqk_cfg.un.fx_sel=SSV6XXX_IQK_TEMPERATURE+SSV6XXX_IQK_RXDC+SSV6XXX_IQK_RXRC+ SSV6XXX_IQK_TXDC+ SSV6XXX_IQK_TXIQ+ SSV6XXX_IQK_RXIQ+SSV6XXX_IQK_PAPD;
#else
    init_iqk_cfg.un.fx_sel=SSV6XXX_IQK_TEMPERATURE+SSV6XXX_IQK_RXDC+SSV6XXX_IQK_RXRC+ SSV6XXX_IQK_TXDC+ SSV6XXX_IQK_TXIQ+ SSV6XXX_IQK_RXIQ;
#endif
    init_iqk_cfg.phy_tbl_size = PHY_SETTING_SIZE;
    init_iqk_cfg.rf_tbl_size = RF_SETTING_SIZE;
    init_iqk_cfg.rf_temper_tbl_size=RF_TEMPER_SETTING_SIZE;

    OS_MemCPY(host_cmd->un.dat32, (void *)&init_iqk_cfg, IQK_CFG_LEN);
    *((ssv_type_u8 *)((ssv_type_u32)host_cmd->un.dat8+IQK_CFG_LEN))=((ssv_type_u32)hwPBUF)&0xFF;
    *((ssv_type_u8 *)((ssv_type_u32)host_cmd->un.dat8+IQK_CFG_LEN+1))=((ssv_type_u32)hwPBUF>>8)&0xFF;
    *((ssv_type_u8 *)((ssv_type_u32)host_cmd->un.dat8+IQK_CFG_LEN+2))=((ssv_type_u32)hwPBUF>>16)&0xFF;    
    *((ssv_type_u8 *)((ssv_type_u32)host_cmd->un.dat8+IQK_CFG_LEN+3))=((ssv_type_u32)hwPBUF>>24)&0xFF;        

    MAC_REG_WRITE(FW_STATUS_REG,fw_status);

    ssv6xxx_drv_send(host_cmd,host_cmd->len);

    OS_MemFree(frame);
    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("Wait iq ... \r\n");

    tx_loopback_info.result = FALSE;
    tx_loopback_info.reg = 0;

    if(OS_SemWait(tx_loopback,OS_MS2TICK(500)) == OS_FAILED) // wait 500ms
    {
        LOG_PRINTF("\33[35mWait TX loopback Timeout\33[0m\r\n");
    }

    if(tx_loopback_info.reg&0x3)
        LOG_PRINTF("\33[35mPMU wake up reg:C000_1D0C:%x\33[0m\r\n",tx_loopback_info.reg);

    if(tx_loopback_info.result == FALSE)
    {
        ssv6030_hal_pbuf_free(hwPBUF);
        return SSV6XXX_FAILED;
    }

    if(gDeviceInfo->recovering != TRUE)
        LOG_PRINTF("IQ Done ... \r\n");

    ssv6030_hal_pbuf_free(hwPBUF);
    return SSV6XXX_SUCCESS;
}
#endif

int ssv6030_hal_tx_loopback_done(ssv_type_u8 *dat)
{
    OS_MemCPY(&tx_loopback_info,dat,sizeof(struct cfg_tx_loopback_info));

    OS_SemSignal(tx_loopback);

    if(!tx_loopback_info.result)
        LOG_PRINTF("\33[35mTX loopback fail\33[0m\r\n");
    return 0;
}

int ssv6030_hal_rf_enable(void)
{
    ssv_type_u32 _regval;

    MAC_REG_READ(0xce010000, _regval);
    _regval &= ~(0x03<<12);
    _regval |= (0x02<<12);
    MAC_REG_WRITE(0xce010000, _regval);

    return 0;
}

int ssv6030_hal_rf_disable(void)
{
    ssv_type_u32 _regval;

    MAC_REG_READ(0xce010000, _regval);
    _regval &= ~(0x03<<12);
    _regval |= (0x01<<12);
    MAC_REG_WRITE(0xce010000, _regval);

    return 0;
}

int ssv6030_hal_rf_load_default_setting(void)
{
    ssv_type_bool ret=FALSE;

    //write rf table
    LOG_PRINTF("rf_load_default_setting\r\n");
    ret = _ssv6030_set_hw_table(ssv6200_rf_tbl,(sizeof(ssv6200_rf_tbl)/sizeof(ssv6200_rf_tbl[0])));
    ssv6030_hal_set_volt_mode(g_host_cfg.volt_mode);

    return ret;
}


#if(ENABLE_DYNAMIC_RX_SENSITIVE==0)

int ssv6030_hal_reduce_phy_cca_bits(void)
{
    ssv_type_bool ret=FALSE;
    ret = _ssv6030_set_hw_table(ssv6200_temp_phy_tbl,
        (sizeof(ssv6200_temp_phy_tbl)/sizeof(ssv6200_temp_phy_tbl[0])));

    if(ret==FALSE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int ssv6030_hal_recover_phy_cca_bits(void)
{
    int temp_size=sizeof(ssv6200_temp_phy_tbl)/sizeof(ssv6200_temp_phy_tbl[0]);
    int phy_size=sizeof(ssv6200_phy_tbl)/sizeof(ssv6200_phy_tbl[0]);
    int i=0,j=0;
    ssv_type_bool ret=0;

    for(i=0;i<temp_size;i++)
    {
        for(j=0;j<phy_size;j++)
        {
            if(ssv6200_temp_phy_tbl[i].address==ssv6200_phy_tbl[j].address)
            {
                ret=MAC_REG_WRITE(ssv6200_phy_tbl[j].address, ssv6200_phy_tbl[j].data);
                if (ret==FALSE) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

int ssv6030_hal_update_cci_setting(ssv_type_u16 input_level)
{
    return -1;
}
#else
#define MAX_CCI_LEVEL 128
ssv_type_bool cci_init=FALSE;
int ssv6030_hal_update_cci_setting(ssv_type_u16 input_level)
{
    #define ENABLE_CCI_DEBUG_MSG 1


    if (input_level > MAX_CCI_LEVEL) {
            LOG_PRINTF("%s:input error[%d]!!\r\n",__FUNCTION__,input_level);
            return -1;
    }

    if ((gDeviceInfo->cci_current_level == 0)&&(cci_init==FALSE)) {
            gDeviceInfo->cci_current_level = MAX_CCI_LEVEL;
            gDeviceInfo->cci_current_gate = (sizeof(adjust_cci)/sizeof(adjust_cci[0])) - 1;
            MAC_REG_WRITE(ADR_RX_11B_CCA_CONTROL, adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control);
            MAC_REG_WRITE(ADR_RX_11B_CCA_1, adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1);
            cci_init=TRUE;
    }

    if(( input_level >= adjust_cci[gDeviceInfo->cci_current_gate].down_level) && (input_level <= adjust_cci[gDeviceInfo->cci_current_gate].upper_level)) {
            gDeviceInfo->cci_current_level = input_level;
#if ENABLE_CCI_DEBUG_MSG
            LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("Keep the 0xce0020a0[%x] 0xce002008[%x]!!(RCPI=%d)\r\n"
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

                    MAC_REG_WRITE(ADR_RX_11B_CCA_CONTROL, adjust_cci[i].adjust_cca_control);
                    MAC_REG_WRITE(ADR_RX_11B_CCA_1, adjust_cci[i].adjust_cca_1);
#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("##Set to the 0xce0020a0[%x] 0xce002008[%x]##!!(RCPI=%d)\r\n"
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

                    MAC_REG_WRITE(ADR_RX_11B_CCA_CONTROL, adjust_cci[i].adjust_cca_control);
                    MAC_REG_WRITE(ADR_RX_11B_CCA_1, adjust_cci[i].adjust_cca_1);
#if ENABLE_CCI_DEBUG_MSG
                    LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING,("##Set to the 0xce0020a0[%x] 0xce002008[%x]##!!(RCPI=%d)\r\n"
                        ,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_control,adjust_cci[gDeviceInfo->cci_current_gate].adjust_cca_1,input_level));
#endif
                    return 0;
                }
            }
        }
    }

    return 0;
}

ssv_type_u32 gCE01000C=0;
int ssv6030_hal_reduce_phy_cca_bits(void)
{
#if 1
    ssv6030_hal_update_cci_setting(MIN_CCI_SENSITIVE);
    MAC_REG_READ(0xCE01000C,gCE01000C);
    MAC_REG_WRITE(0xCE01000C,(gCE01000C|(0x3F<<3)));
    return 0;
#else
    ssv_type_bool ret=FALSE;
    ret = SSV6XXX_SET_HW_TABLE(ssv6200_temp_phy_tbl,
        (sizeof(ssv6200_temp_phy_tbl)/sizeof(ssv6200_temp_phy_tbl[0])));

    MAC_REG_READ(0xCE01000C,gCE01000C);
    MAC_REG_WRITE(0xCE01000C,(gCE01000C|(0x3F<<3)));

    if(ret==FALSE)
    {
        return -1;
    }
    else
    {
        return 0;
    }
#endif
}

int ssv6030_hal_recover_phy_cca_bits(void)
{

    //int temp_size=sizeof(ssv6200_temp_phy_tbl)/sizeof(ssv6200_temp_phy_tbl[0]);
    //int phy_size=sizeof(ssv6200_phy_tbl)/sizeof(ssv6200_phy_tbl[0]);
    int i=0,j=0;
    //ssv_type_bool ret=0;

    for(i=0;i<MAX_VIF_NUM;i++)
    {
        if(gDeviceInfo->vif[i].m_info.StaInfo->status!=CONNECT)
    {
        #if 1
        ssv6030_hal_update_cci_setting(MAX_CCI_SENSITIVE);
        #else
        for(i=0;i<temp_size;i++)
        {
            for(j=0;j<phy_size;j++)
            {
                if(ssv6200_temp_phy_tbl[i].address==ssv6200_phy_tbl[j].address)
                {
                    ret=MAC_REG_WRITE(ssv6200_phy_tbl[j].address, ssv6200_phy_tbl[j].data);
                    if (ret==FALSE) {
                        return -1;
                    }
                }
            }
        }
        #endif
    }
    else
    {

        ssv6030_hal_update_cci_setting(gDeviceInfo->cci_current_level);
    }
    }
    MAC_REG_WRITE(0xCE01000C,gCE01000C);
    return 0;
}
#endif

ssv_type_bool ssv6030_hal_support_5g_band(void)
{
    return FALSE;
}
const struct ssv_hal_ops	g_hal_ssv6030=
{
    "SSV6030",
    ssv6030_hal_chip_init,
    ssv6030_hal_init_mac,
    ssv6030_hal_init_sta_mac,
    ssv6030_hal_init_ap_mac,
    ssv6030_hal_ap_wep_setting,
    ssv6030_hal_tx_loopback_done,
    NULL,//Add vif
    NULL,//remove vif
    ssv6030_hal_setup_ampdu_wmm,
    ssv6030_hal_pbuf_alloc,
    ssv6030_hal_pbuf_free,
    ssv6030_hal_rf_enable,
    ssv6030_hal_rf_disable,
    ssv6030_hal_rf_load_default_setting,
    ssv6030_hal_watchdog_enable,
    ssv6030_hal_watchdog_disable,
    ssv6030_hal_mcu_enable,
    ssv6030_hal_mcu_disable,
    ssv6030_hal_sw_reset,
    ssv6030_hal_gen_random,
    ssv6030_hal_promiscuous_enable,
    ssv6030_hal_promiscuous_disable,
    ssv6030_hal_read_chip_id,
    ssv6030_hal_read_efuse_macaddr,
    NULL, //Write Efuse mac
    NULL, //dump_efuse
    NULL,//    int (* efuse_read_usb_vid)(u16 *pvid);
    NULL,//    int (* efuse_read_usb_pid)(u16 *ppid);
    NULL,//    int (* efuse_read_tx_power1)(u8* pvalue);
    NULL,//    int (* efuse_read_tx_power2)(u8* pvalue);
    NULL,//    int (* efuse_read_xtal)(u8* pxtal);
    ssv6030_hal_get_diagnosis,
    ssv6030_hal_is_heartbeat,
    ssv6030_hal_reset_heartbeat,
    ssv6030_hal_get_fw_status,
    ssv6030_hal_set_fw_status,
    ssv6030_hal_reset_soc_irq,
    ssv6030_hal_set_gpio_output,
    ssv6030_hal_set_short_slot_time,
    ssv6030_hal_soc_set_bcn,
    ssv6030_hal_beacon_set,
    ssv6030_hal_beacon_enable,
    ssv6030_hal_is_beacon_enable,
    ssv6030_hal_get_tx_resources,
    ssv6030_hal_bytes_to_pages,
    ssv6030_hal_get_rssi_from_reg,
    ssv6030_hal_get_rc_info,
    ssv6030_hal_get_agc_gain,
    ssv6030_hal_set_agc_gain,
    ssv6030_hal_set_acs_agc_gain,
    ssv6030_hal_ap_listen_neighborhood,
    ssv6030_hal_reduce_phy_cca_bits,
    ssv6030_hal_recover_phy_cca_bits,
    ssv6030_hal_update_cci_setting,
    ssv6030_hal_set_ext_rx_int,
    ssv6030_hal_pause_resuem_recovery_int,
    ssv6030_hal_set_TXQ_SRC_limit,
    ssv6030_hal_halt_txq,
    ssv6030_hal_get_temperature,
    ssv6030_hal_set_volt_mode,
    NULL, //int (*display_hw_queue_status)(void);
    ssv6030_hal_support_5g_band,
    NULL, //get ava wsid
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    /** tx descriptor **/
    ssv6030_hal_dump_txinfo,
    ssv6030_hal_get_valid_txinfo_size,
    ssv6030_hal_get_txreq0_size,
    ssv6030_hal_get_txreq0_ctype,
    ssv6030_hal_set_txreq0_ctype,
    ssv6030_hal_get_txreq0_len,
    ssv6030_hal_set_txreq0_len,
    ssv6030_hal_get_txreq0_rsvd0,
    ssv6030_hal_set_txreq0_rsvd0,
    ssv6030_hal_get_txreq0_padding,
    ssv6030_hal_set_txreq0_padding,
    ssv6030_hal_get_txreq0_qos,
    ssv6030_hal_get_txreq0_ht,
    ssv6030_hal_get_txreq0_4addr,
    ssv6030_hal_set_txreq0_f80211,
    ssv6030_hal_get_txreq0_f80211,
    ssv6030_hal_get_txreq0_more_data,
    ssv6030_hal_set_txreq0_more_data,
    ssv6030_hal_get_txreq0_qos_ptr,
    ssv6030_hal_get_txreq0_data_ptr,
    NULL,//oid *(* fill_txreq0)(void *frame, .....
    ssv6030_hal_fill_txreq0,
    /** rx descriptor **/
    NULL,//int (* rx_11to3)(void *p);
    ssv6030_hal_dump_rxinfo,
    ssv6030_hal_get_rxpkt_size,
    ssv6030_hal_get_rxpkt_ctype,
    ssv6030_hal_get_rxpkt_len,
    ssv6030_hal_get_rxpkt_rcpi,
    ssv6030_hal_set_rxpkt_rcpi,
    ssv6030_hal_get_rxpkt_qos,
    ssv6030_hal_get_rxpkt_f80211,
    ssv6030_hal_get_rxpkt_psm,
    ssv6030_hal_get_rxpkt_rsvd,
    ssv6030_hal_get_rxpkt_wsid,
    ssv6030_hal_get_rxpkt_tid,
    ssv6030_hal_get_rxpkt_seqnum,
    ssv6030_hal_get_rxpkt_qos_ptr,
    ssv6030_hal_get_rxpkt_data_ptr,
    ssv6030_hal_get_rxpkt_data_len,
    ssv6030_hal_get_rxpkt_bssid_idx,
    NULL, //hci rx aggr process;
    NULL, //hci tx aggr process;
    NULL, //hci aggr en;
    /** download fw **/
    ssv6030_hal_download_fw,
    /**decision table**/
    ssv6030_hal_accept_none_wsid_frame,
    ssv6030_hal_drop_none_wsid_frame,
    ssv6030_hal_drop_probe_request,
    ssv6030_hal_sta_rcv_all_bcn,
    ssv6030_hal_sta_rcv_specific_bcn,
    ssv6030_hal_sta_reject_bcn,
    /** data flow**/
    ssv6030_hal_acs_rx_mgmt_flow,
    ssv6030_hal_ap_rx_mgmt_flow,
    ssv6030_hal_sconfig_rx_data_flow,
    ssv6030_hal_sta_rx_data_flow,
};
#endif //#if((CONFIG_CHIP_ID==ID_SSV6051Q)||(CONFIG_CHIP_ID==ID_SSV6051Z)||(CONFIG_CHIP_ID==ID_SSV6030P))
