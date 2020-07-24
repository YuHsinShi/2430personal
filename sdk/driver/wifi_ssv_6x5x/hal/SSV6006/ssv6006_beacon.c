#define SSV6006_BEACON_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <rtos.h>
#include <log.h>
#include <ssv_devinfo.h>
#include <ssv_dev.h>
#include <ssv_drv.h>
#include "regs/ssv6006_hw_regs.h"
#include "ssv6006_hal.h"
#include "ssv6006_beacon.h"
#include "ssv6006_pktdef.h"
#include "../ssv_hal_if.h"
#if (AP_MODE_ENABLE == 1)        
#include <ap_def.h>
#include <ap_info.h>
#include <ap_mlme.h>
#endif
#define MTX_BCN_PKTID_CH_LOCK_SHIFT         MTX_BCN_PKTID_CH_LOCK_SFT
#define MTX_BCN_CFG_VLD_SHIFT               MTX_BCN_CFG_VLD_SFT
#define MTX_BCN_CFG_VLD_MASK                MTX_BCN_CFG_VLD_MSK

#define MTX_BCN_TIMER_EN_SHIFT              MTX_BCN_TIMER_EN_SFT															//	0
#define MTX_TSF_TIMER_EN_SHIFT              MTX_TSF_TIMER_EN_SFT															//	5
#define MTX_INT_DTIM_NUM_SHIFT              MTX_INT_DTIM_NUM_SFT	//8
#define MTX_EN_INT_Q4_Q_EMPTY_SHIFT         MTX_EN_INT_Q4_Q_EMPTY_SFT	//24

#define MTX_BCN_ENABLE_MASK                 (MTX_BCN_TIMER_EN_I_MSK)  	//0xffffff9e
#define MTX_TSF_ENABLE_MASK	                (MTX_TSF_TIMER_EN_I_MSK)  	//0xffffff9e

//=====>ADR_MTX_BCN_PRD
#define MTX_BCN_PERIOD_SHIFT                MTX_BCN_PERIOD_SFT		// 0			//bit0~7
#define MTX_DTIM_NUM_SHIFT                  MTX_DTIM_NUM_SFT		// 24			//bit 24~bit31

//=====>ADR_MTX_BCN_CFG0/ADR_MTX_BCN_CFG1
#define MTX_DTIM_OFST0                      MTX_DTIM_OFST0_SFT

#define MTX_FW_BCN_ST_SFT                   16
#define MTX_BCN_ST_MASK                     0x03

#define TX_PB_OFFSET                           SSV6006_TX_PB_OFFSET

extern struct Host_cfg g_host_cfg;

enum ssv6xxx_beacon_type{
	SSV6xxx_BEACON_0,
	SSV6xxx_BEACON_1
};

extern int ssv6006_hal_pbuf_alloc(int size, int type);

#define bcn_cnt 2

static int __ssv6006_hal_beacon_get_hw_valid_cfg(void);
static enum ssv6xxx_beacon_type _ssv6006_hal_beacon_get_hw_valid_cfg(void);
#if (AUTO_BEACON == 0)
static int _ssv6006_hal_beacon_get_fw_valid_cfg(void);
#endif
static int _ssv6006_hal_beacon_fill_content(ssv_type_u32 regaddr, ssv_type_u8 *beacon, int size);
static int _ssv6006_hal_beacon_set_info(ssv_type_u8 beacon_interval, ssv_type_u8 dtim_cnt);
static int _ssv6006_hal_beacon_set_id_dtim(ssv_type_u8 availableBcn, ssv_type_u32 pbufAddr, ssv_type_u32 dtim_offset);
static int _ssv6006_hal_beacon_reg_lock(ssv_type_bool block);
static int _ssv6006_hal_beacon_fill_tx_desc(ssv_type_u8 bcn_len, void *frame);

static ssv_type_u8 beacon_usage=0;
static ssv_type_u8 beacon_enable=FALSE;

struct ssv6xxx_beacon_info hw_bcn_info[bcn_cnt]; // record hw bcn info

static int _ssv6006_hal_beacon_reg_lock(ssv_type_bool block)
{
    ssv_type_u32 val;
    val = block<<MTX_BCN_PKTID_CH_LOCK_SHIFT;
    MAC_REG_WRITE(ADR_MTX_BCN_MISC, val);
    return 0;
}

static int __ssv6006_hal_beacon_get_hw_valid_cfg(void)
{
    ssv_type_u32 regval =0;
    //MAC_REG_READ(ADR_MTX_BCN_MISC, regval);
    //regval &= MTX_BCN_CFG_VLD_MASK;
    //regval = regval >>MTX_BCN_CFG_VLD_SHIFT;
    regval = GET_MTX_BCN_CFG_VLD;

    return regval;
}
#if (AUTO_BEACON == 0)
static int _ssv6006_hal_beacon_get_fw_valid_cfg(void)
{
    ssv_type_u32 regval =0;
    //Occupy this field for sw beacon,and the ADR_STAT_CONF0[29] must be disabled
    //MAC_REG_READ(ADR_STAT_CONF0, regval);
    //regval = ((regval>>MTX_FW_BCN_ST_SFT)&MTX_BCN_ST_MASK);

    regval = GET_STAT_PKT_ID;
    return regval;
}
static void _ssv6006_hal_beacon_set_fw_valid_cfg(ssv_type_u8 value)
{
    SET_STAT_PKT_ID(value);
}

#endif
static int _ssv6006_hal_beacon_set_info(ssv_type_u8 beacon_interval, ssv_type_u8 dtim_cnt)
{
    //u32 val;

    //if default is 0 set to our default
    if(beacon_interval==0)
    {
        beacon_interval = 100;
    }

    //val = (beacon_interval<<MTX_BCN_PERIOD_SHIFT)| (dtim_cnt<<MTX_DTIM_NUM_SHIFT);
    MAC_REG_WRITE( ADR_MTX_BCN_PRD, beacon_interval);
    MAC_REG_WRITE(ADR_MTX_BCN_DTIM_CONFG,dtim_cnt);
    return 0;
}

ssv_type_bool ssv6006_hal_is_beacon_enable(void)
{
    return beacon_enable;
}

extern ssv_type_s32 _ssv6xxx_wifi_ioctl_Ext(ssv_type_u32 cmd_id, void *data, ssv_type_u32 len, ssv_type_bool blocking,const ssv_type_bool mutexLock);
extern int ssv6006_hal_pbuf_free(int addr);
extern ssv_type_s32 ssv6xxx_wifi_send_cmd_direct_txq(void *pCusData, int nCuslen, ssv6xxx_host_cmd_id eCmdID);
int ssv6006_hal_beacon_enable(ssv_type_bool bEnable,ssv_type_bool bFreePbuf, ssv_type_bool block)
{

    int ret = 0;
#if (AUTO_BEACON != 1)
    ssv_type_u8 cmd_data[] = {0x00, 0x00, 0x00, 0x00};
#endif
    //ssv_type_u32 regval=0;


    //If there is no beacon set to register, beacon could not be turn on.
    if(bEnable && !beacon_usage)
    {
        LOG_PRINTF("[A] Reject to set beacon!!!.ssv6xxx_beacon_enable bEnable[%d] sc->beacon_usage[%d]\r\n",bEnable ,beacon_usage);
        beacon_enable = FALSE;
        return 0;
    }

    if((bEnable && (beacon_enable))||
        (!bEnable && !beacon_enable))
    {
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("[A] ssv6xxx_beacon_enable bEnable[%d] and sc->beacon_enable[%d] are the same. no need to execute.\r\n",bEnable ,beacon_enable);
        //return -1;
        if(bEnable){
            LOG_PRINTF("        Ignore enable beacon cmd!!!!\r\n");
            return 0;
        }
    }

    if(bEnable==TRUE)
    {

    //SET_MTX_BCN_AUTO_SEQ_NO(1); //seqnum auto-fill, remove by sw seq in mcu FW
    SET_MTX_TIME_STAMP_AUTO_FILL(1); //timestamp auto-fill

    SET_MTX_DTIM_CNT_AUTO_FILL(1);
    SET_MTX_TSF_TIMER_EN(1); //This big is also setted in fw/dbg_timer.c
#if (AUTO_BEACON == 1)
    SET_MTX_BCN_TIMER_EN(1);
    /*
    MAC_REG_READ(ADR_MTX_INT_EN, regval);
    regval&= MTX_EN_INT_Q4_Q_EMPTY_I_MSK;
	regval|=(bEnable<<MTX_EN_INT_Q4_Q_EMPTY_SHIFT);
	MAC_REG_WRITE(ADR_MTX_INT_EN, regval);
	*/
#else
    cmd_data[0] = bEnable;
    if(block)
        ssv6xxx_wifi_send_cmd_direct_txq(cmd_data, 4, SSV6XXX_HOST_CMD_SELF_BCN_ENABLE);
    else{   
        while(_ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SELF_BCN_ENABLE, cmd_data, 4, FALSE, FALSE) != SSV6XXX_SUCCESS)
        {
            LOG_PRINTF("SELF_BCN_ENABLE retry\r\n");
            OS_MsDelay(10);
        }
    }
    
#endif

        beacon_enable = bEnable;
    }
    else
    {
        int i;
#if (AUTO_BEACON == 1)
        SET_MTX_BCN_TIMER_EN(0);
        /*
        MAC_REG_READ(ADR_MTX_INT_EN, regval);
        regval&= MTX_EN_INT_Q4_Q_EMPTY_I_MSK;
        regval|=(bEnable<<MTX_EN_INT_Q4_Q_EMPTY_SHIFT);
        MAC_REG_WRITE(ADR_MTX_INT_EN, regval);
        */
#else
        cmd_data[0] = bEnable;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SELF_BCN_ENABLE, cmd_data, 4, TRUE, FALSE);
        OS_MsDelay(g_host_cfg.bcn_interval*2);
        _ssv6006_hal_beacon_set_fw_valid_cfg(0);
#endif
        if(bFreePbuf)
        {
            for(i=0;i<bcn_cnt;i++)
            {
                if(hw_bcn_info[i].pubf_addr)
                {
                    LOG_PRINTF("free bcn=0x%x\r\n",hw_bcn_info[i].pubf_addr);
                    ssv6006_hal_pbuf_free(hw_bcn_info[i].pubf_addr);
                }
            }
            OS_MemSET(hw_bcn_info,0,sizeof(struct ssv6xxx_beacon_info)*bcn_cnt);
        }
        
        beacon_usage =0;
        beacon_enable = FALSE;
        if(gDeviceInfo->recovering != TRUE)
        	LOG_TRACE("%s Stop to send Beacon\r\n",__func__);
    }

    return ret;

}

static int _ssv6006_hal_beacon_set_id_dtim(ssv_type_u8 availableBcn, ssv_type_u32 pbufAddr, ssv_type_u32 dtim_offset)
{
    ssv_type_u32 reg_tx_beacon_id_adr[] = {ADR_MTX_BCN_PKT_SET0, ADR_MTX_BCN_PKT_SET1};
    ssv_type_u32 reg_tx_beacon_dtim_adr[] = {ADR_MTX_BCN_DTIM_SET0, ADR_MTX_BCN_DTIM_SET1};

    MAC_REG_WRITE( reg_tx_beacon_id_adr[availableBcn], (ssv_type_u32)SSV6006_PBUF_MapPkttoID(pbufAddr));
    MAC_REG_WRITE( reg_tx_beacon_dtim_adr[availableBcn], dtim_offset);
    return 0;
}

static int _ssv6006_hal_beacon_fill_tx_desc(ssv_type_u8 bcn_len, void *frame)
{
    struct ssv6006_tx_desc *tx_desc = (struct ssv6006_tx_desc *)frame;

    //length
    tx_desc->len            = bcn_len;
    tx_desc->c_type         = M2_TXREQ;
    tx_desc->f80211         = 1;
    tx_desc->ack_policy_obsolete = 1;//no ack;
    tx_desc->hdr_offset 	= TX_PB_OFFSET;
    tx_desc->hdr_len 		= 24;
    tx_desc->ack_policy0    =1;
    tx_desc->ack_policy1    =1;
    tx_desc->ack_policy2    =1;
    tx_desc->ack_policy3    =1;
    tx_desc->payload_offset_obsolete = tx_desc->hdr_offset + tx_desc->hdr_len;
#if (AP_MODE_ENABLE == 1)        
    if(IS_AP_IN_5G_BAND())
    {
        tx_desc->drate_idx0=0x80;
    }
#endif
    return 0;
}

static enum ssv6xxx_beacon_type _ssv6006_hal_beacon_get_hw_valid_cfg(void)
{
	ssv_type_u32 regval =0;
	regval=__ssv6006_hal_beacon_get_hw_valid_cfg();

	//get MTX_BCN_CFG_VLD

	if(regval==0x2 || regval == 0x0)//bcn 0 is availabke to use.
		return SSV6xxx_BEACON_0;
	else if(regval==0x1)//bcn 1 is availabke to use.
		return SSV6xxx_BEACON_1;
	else
		LOG_PRINTF("=============>ERROR!!drv_bcn_reg_available\n");//ASSERT(FALSE);// 11 error happened need check with ASIC.


	return SSV6xxx_BEACON_0;
}

static int _ssv6006_hal_beacon_fill_content(ssv_type_u32 regaddr, ssv_type_u8 *beacon, int size)
{
	int i;
    ssv_type_u32 val;
	ssv_type_u32 *ptr = (ssv_type_u32*)beacon;
	size = size/4 + ((size%4)>0?1:0);
    //size = size*4;

	for(i=0; i<size; i++)
	{
		val = (ssv_type_u32)(*(ptr+i));

		MAC_REG_WRITE(regaddr+(ssv_type_u32)i*4, val);
	}

    return 0;
}

int ssv6006_hal_beacon_set(void* beacon_skb, int dtim_offset, ssv_type_bool block)
{
    int size = (((struct ssv6006_tx_desc *)beacon_skb)->len)+SSV6006_TX_PB_OFFSET;
    enum ssv6xxx_beacon_type avl_bcn_type = SSV6xxx_BEACON_0;
    ssv_type_bool ret = true;
#if (AP_MODE_ENABLE == 1)        

#if (AUTO_BEACON == 0)
    ssv_type_u8 fw_bcn_st = 0, avl_bcn = 0;
#endif
    //u32 pubf_addr;
    _ssv6006_hal_beacon_reg_lock(1);

    //1.Decide which register can be used to set
    avl_bcn_type = _ssv6006_hal_beacon_get_hw_valid_cfg();

#if (AUTO_BEACON == 0)
    avl_bcn = (ssv_type_u8)(avl_bcn_type + 1);
    fw_bcn_st=_ssv6006_hal_beacon_get_fw_valid_cfg();
    //LOG_PRINTF("avl_bcn_type=%d,avl_bcn:(%d/%d) fw_bcn_st=%x\r\n",avl_bcn_type,avl_bcn,__ssv6006_hal_beacon_get_hw_valid_cfg(),fw_bcn_st);
    if ((avl_bcn & fw_bcn_st) != 0)
    {
        //LOG_PRINTF("!! Collision %d, %d \r\n",avl_bcn, fw_bcn_st);
        ret = false;
        goto out;
    }
    if(size > 512)
        LOG_ERROR("\33[31m %s:Size is too big for self-sending beacon, need to check fw\33[0m\r\n", __FUNCTION__);

#endif
	//2.Get Pbuf from ASIC
    do{
        //LOG_PRINTF("beacon_usage=%d,avl_bcn_type=%d,len=%d,size=%d\r\n",beacon_usage,avl_bcn_type,hw_bcn_info[avl_bcn_type].len,size);
        if((beacon_usage) & (0x01<<(avl_bcn_type) ))
        {
            //if size under 512, no need to re-allocate, since we gave the max 512 in the beginning.
            //In face, we don't allow beacon size > 512, so the re-allocate won't happen.
            if (hw_bcn_info[avl_bcn_type].allocat_len >= size) 
            {
                break;
            }
            else
            {
                //old beacon too small, need to free
                LOG_PRINTF("Resize bcn buf,size=%d\r\n",size);
                if(-1 == ssv6006_hal_pbuf_free(hw_bcn_info[avl_bcn_type].pubf_addr))
                {
                    ret = false;
                    goto out;
                }
            beacon_usage &= ~(0x01<<avl_bcn_type);
            hw_bcn_info[avl_bcn_type].pubf_addr = 0;
            }
        }

        //Allocate new one
        if(!hw_bcn_info[avl_bcn_type].pubf_addr)
        {      
            hw_bcn_info[avl_bcn_type].pubf_addr = ssv6006_hal_pbuf_alloc(512, RX_BUF); //Give the max pbuf space
            hw_bcn_info[avl_bcn_type].allocat_len = 512;
        }
        else
            LOG_PRINTF("use old one\r\n");
        
        LOG_PRINTF("Beacon addr %x,size=%d\r\n",hw_bcn_info[avl_bcn_type].pubf_addr,size);
        //if can't allocate beacon, just leave.
        if(hw_bcn_info[avl_bcn_type].pubf_addr == 0)
        {
            ret = false;
            goto out;
        }

        //Indicate reg is stored packet buf.
        beacon_usage |= (0x01<<avl_bcn_type);
    }while(0);

    //3. Write Beacon content. 
    _ssv6006_hal_beacon_fill_content(hw_bcn_info[avl_bcn_type].pubf_addr, beacon_skb, size);
    //4. Assign to register let tx know. Beacon is updated.
    _ssv6006_hal_beacon_set_id_dtim(avl_bcn_type,hw_bcn_info[avl_bcn_type].pubf_addr,(dtim_offset-24)); //24: 802.11 header length

out:
    _ssv6006_hal_beacon_reg_lock(0);

#if (AUTO_BEACON != 0)
    if((ret == true) && ((beacon_usage&0x03) == 0x03) && (!beacon_enable))
    {
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("[A] enable beacon for BEACON_WAITING_ENABLED flags\r\n");
        _ssv6006_hal_beacon_set_info(g_host_cfg.bcn_interval,AP_DEFAULT_DTIM_PERIOD-1);
        ssv6006_hal_beacon_enable(true,false,block);
    }
#else
    if(ret == true)
    {
        if (((beacon_usage&0x03) == 0x03) && (!beacon_enable))
        {
            if(gDeviceInfo->recovering != TRUE)
                LOG_PRINTF("[A] enable beacon for BEACON_WAITING_ENABLED flags\r\n");
            _ssv6006_hal_beacon_set_info(g_host_cfg.bcn_interval,AP_DEFAULT_DTIM_PERIOD-1);
            ssv6006_hal_beacon_enable(true,false,block);
        }
    //OS_MsDelay(g_host_cfg.bcn_interval+10);
    }
#endif
    //LOG_PRINTF("beacon_set,%d\r\n",__LINE__);
#endif
	return (ret==true)?0:-1;
}

int ssv6006_hal_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv)
{
    struct ssv6006_tx_desc *req = (void *)frame;
    ssv_type_u16 len=sizeof(struct ssv6006_tx_desc)+ bcn_info->bcn_len;
	ssv_type_u8* raw = (ssv_type_u8*)req;
	ssv_type_u8 extra_len;


	//Fill Extra info
	raw +=  len;

	//Fill EID
	*raw = extra_type;//-------------------------------------------------------------------->
	raw++;

	if(extra_type == SSV6XXX_SET_INIT_BEACON)
	{
		struct cfg_set_init_bcn init_bcn;
		init_bcn.bcn_info = *bcn_info;
		//init_bcn.param.bcn_enable = TRUE;
		init_bcn.param.bcn_itv = bcn_itv;
		init_bcn.param.dtim_cnt = dtim_cnt;

		extra_len = sizeof(struct cfg_set_init_bcn);

		//Fill E length
		*raw = extra_len;//------------------------------------------------------------------>
		raw++;

		//Fill data
		OS_MemCPY(raw, &init_bcn, extra_len);//---------------------------------------------->
	}
	else
	{
		//SSV6XXX_SET_BEACON

		extra_len = sizeof(struct cfg_bcn_info);

		//Fill E length
		*raw = extra_len;//-------------------------------------------------------------------->
		raw++;


		//Fill data
		OS_MemCPY(raw, bcn_info, extra_len);//------------------------------------------------>
	}

	raw+=extra_len;


	//Extra total length
	*(ssv_type_u8*)raw = extra_len+2;//data length plus EID and length
	raw+=2;
    req->len=(ssv_type_u32)(raw-(ssv_type_u8*)req);


// set tx_desc
    _ssv6006_hal_beacon_fill_tx_desc(bcn_info->bcn_len,frame);

    return 0;
}

void ssv6006_hal_beacon_reset(void)
{
    OS_MemSET(hw_bcn_info,0,sizeof(struct ssv6xxx_beacon_info)*bcn_cnt);
}

#endif //#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
