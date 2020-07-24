#define SSV6030_BEACON_C
#include <host_config.h>

#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <config.h>
#include <log.h>
#include <dev.h>
#include "ssv6030_hal.h"
#include "regs/ssv6051_reg.h"
#include "regs/ssv6051_aux.h"
#include "ssv6030_pktdef.h"
#include "../ssv_hal_if.h"

#include <ssv_dev.h>
#include <ap_info.h>

#define MTX_BCN_PKTID_CH_LOCK_SHIFT         MTX_BCN_PKTID_CH_LOCK_SFT
#define MTX_BCN_CFG_VLD_SHIFT               MTX_BCN_CFG_VLD_SFT
#define MTX_BCN_CFG_VLD_MASK                MTX_BCN_CFG_VLD_MSK

#define MTX_BCN_TIMER_EN_SHIFT              MTX_BCN_TIMER_EN_SFT															//	0
#define MTX_TSF_TIMER_EN_SHIFT              MTX_TSF_TIMER_EN_SFT															//	5
#define MTX_HALT_MNG_UNTIL_DTIM_SHIFT       MTX_HALT_MNG_UNTIL_DTIM_SFT	//6
#define MTX_INT_DTIM_NUM_SHIFT              MTX_INT_DTIM_NUM_SFT	//8
#define MTX_EN_INT_Q4_Q_EMPTY_SHIFT         MTX_EN_INT_Q4_Q_EMPTY_SFT	//24

#define MTX_BCN_ENABLE_MASK                 (MTX_BCN_TIMER_EN_I_MSK)  	//0xffffff9e
#define MTX_TSF_ENABLE_MASK	                (MTX_TSF_TIMER_EN_I_MSK)  	//0xffffff9e

//=====>ADR_MTX_BCN_PRD
#define MTX_BCN_PERIOD_SHIFT                MTX_BCN_PERIOD_SFT		// 0			//bit0~7
#define MTX_DTIM_NUM_SHIFT                  MTX_DTIM_NUM_SFT		// 24			//bit 24~bit31

//=====>ADR_MTX_BCN_CFG0/ADR_MTX_BCN_CFG1
#define MTX_DTIM_OFST0                      MTX_DTIM_OFST0_SFT

#define MTX_FW_BCN_ST_SFT                   MTX_INT_DTIM_NUM_SHIFT
#define MTX_BCN_ST_MASK                     0x03

#define TX_PB_OFFSET                           SSV6051_TX_PB_OFFSET

extern struct Host_cfg g_host_cfg;
enum ssv6xxx_beacon_type{
	SSV6xxx_BEACON_0,
	SSV6xxx_BEACON_1
};

#define bcn_cnt 2
struct ssv6xxx_beacon_info {
	ssv_type_u32 pubf_addr;
	ssv_type_u16 len;
	ssv_type_u8 tim_offset;
	ssv_type_u8 tim_cnt;
};
static int __ssv6030_hal_beacon_get_hw_valid_cfg(void);
static enum ssv6xxx_beacon_type _ssv6030_hal_beacon_get_hw_valid_cfg();
#if (AUTO_BEACON == 0)
static int _ssv6030_hal_beacon_get_fw_valid_cfg(void);
#endif
static int _ssv6030_hal_beacon_fill_content(ssv_type_u32 regaddr, ssv_type_u8 *beacon, int size);
static int _ssv6030_hal_beacon_set_info(ssv_type_u8 beacon_interval, ssv_type_u8 dtim_cnt);
static int _ssv6030_hal_beacon_set_id_dtim(ssv_type_u8 availableBcn, ssv_type_u32 pbufAddr, ssv_type_u32 dtim_offset);
static int _ssv6030_hal_beacon_reg_lock(ssv_type_bool block);
static int _ssv6030_hal_beacon_fill_tx_desc(ssv_type_u8 bcn_len, void *frame);

static ssv_type_u8 beacon_usage=0;
static ssv_type_u8 beacon_enable=FALSE;

static struct ssv6xxx_beacon_info hw_bcn_info[bcn_cnt]; // record hw bcn info

static int _ssv6030_hal_beacon_reg_lock(ssv_type_bool block)
{
    ssv_type_u32 val;
    val = block<<MTX_BCN_PKTID_CH_LOCK_SHIFT;
    MAC_REG_WRITE(ADR_MTX_BCN_MISC, val);
    return 0;
}

static int __ssv6030_hal_beacon_get_hw_valid_cfg(void)
{
    ssv_type_u32 regval =0;
    MAC_REG_READ(ADR_MTX_BCN_MISC, regval);
    regval &= MTX_BCN_CFG_VLD_MASK;
    regval = regval >>MTX_BCN_CFG_VLD_SHIFT;
    return regval;
}
#if (AUTO_BEACON == 0)
static int _ssv6030_hal_beacon_get_fw_valid_cfg(void)
{
    ssv_type_u32 regval =0;
    MAC_REG_READ(ADR_MTX_BCN_EN_MISC, regval);
    regval = ((regval>>MTX_FW_BCN_ST_SFT)&MTX_BCN_ST_MASK);
    return regval;
}
#endif
static int _ssv6030_hal_beacon_set_info(ssv_type_u8 beacon_interval, ssv_type_u8 dtim_cnt)
{
    ssv_type_u32 val;

    //if default is 0 set to our default
    if(beacon_interval==0)
    {
        beacon_interval = 100;
    }

    val = (beacon_interval<<MTX_BCN_PERIOD_SHIFT)| (dtim_cnt<<MTX_DTIM_NUM_SHIFT);
    MAC_REG_WRITE( ADR_MTX_BCN_PRD, val);
    return 0;
}

ssv_type_bool ssv6030_hal_is_beacon_enable(void)
{
    return beacon_enable;
}

int ssv6030_hal_beacon_enable(ssv_type_bool bEnable)
{

    int ret = 0;
#if (AUTO_BEACON != 1)
    ssv_type_u8 cmd_data[] = {0x00, 0x00, 0x00, 0x00};
#else
    ssv_type_u32 regval=0;
#endif

    //If there is no beacon set to register, beacon could not be turn on.
    if(bEnable && !beacon_usage)
    {
        LOG_PRINTF("[A] Reject to set beacon!!!.        ssv6xxx_beacon_enable bEnable[%d] sc->beacon_usage[%d]\n",bEnable ,beacon_usage);
        beacon_enable = FALSE;
        return 0;
    }

    if((bEnable && (beacon_enable))||
        (!bEnable && !beacon_enable))
    {
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("[A] ssv6xxx_beacon_enable bEnable[%d] and sc->beacon_enable[%d] are the same. no need to execute.\n",bEnable ,beacon_enable);
        //return -1;
        if(bEnable){
            LOG_PRINTF("        Ignore enable beacon cmd!!!!\n");
            return 0;
        }
    }

    if(bEnable==TRUE)
    {
#if (AUTO_BEACON == 1)
    MAC_REG_READ(ADR_MTX_BCN_EN_MISC, regval);
    regval&= MTX_BCN_ENABLE_MASK;
	regval|=(bEnable<<MTX_BCN_TIMER_EN_SHIFT)|
	(bEnable<<MTX_TSF_TIMER_EN_SHIFT) |
	(bEnable<<MTX_HALT_MNG_UNTIL_DTIM_SHIFT)|
	(bEnable<<MTX_INT_DTIM_NUM_SHIFT);
	MAC_REG_WRITE(ADR_MTX_BCN_EN_MISC, regval);

    MAC_REG_READ(ADR_MTX_INT_EN, regval);
    regval&= MTX_EN_INT_Q4_Q_EMPTY_I_MSK;
	regval|=(bEnable<<MTX_EN_INT_Q4_Q_EMPTY_SHIFT);
	MAC_REG_WRITE(ADR_MTX_INT_EN, regval);
#else    
    cmd_data[0] = bEnable;
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SELF_BCN_ENABLE, cmd_data, 4, TRUE, FALSE);
#endif

        beacon_enable = bEnable;
    }
    else
    {
        OS_MemSET(hw_bcn_info,0,sizeof(struct ssv6xxx_beacon_info)*bcn_cnt);
        beacon_usage =0;
        beacon_enable = FALSE;
        if(gDeviceInfo->recovering != TRUE)
        	LOG_TRACE("%s Stop to send Beacon\r\n",__func__);
    }

    return ret;

}

static int _ssv6030_hal_beacon_set_id_dtim(ssv_type_u8 availableBcn, ssv_type_u32 pbufAddr, ssv_type_u32 dtim_offset)
{
    ssv_type_u32 val;
    ssv_type_u32 reg_tx_beacon_adr[] = {ADR_MTX_BCN_CFG0, ADR_MTX_BCN_CFG1};
    val = (SSV6051_PBUF_MapPkttoID(pbufAddr))|(dtim_offset<<MTX_DTIM_OFST0);
    MAC_REG_WRITE( reg_tx_beacon_adr[availableBcn], (ssv_type_u32)val);
    return 0;
}

static int _ssv6030_hal_beacon_fill_tx_desc(ssv_type_u8 bcn_len, void *frame)
{
    struct ssv6200_tx_desc *tx_desc = (struct ssv6200_tx_desc *)frame;

    //length
    tx_desc->len            = bcn_len;
    tx_desc->c_type         = M2_TXREQ;
    tx_desc->f80211         = 1;
    tx_desc->ack_policy     = 1;//no ack;
    tx_desc->hdr_offset 	= TX_PB_OFFSET;
    tx_desc->hdr_len 		= 24;
    tx_desc->payload_offset = tx_desc->hdr_offset + tx_desc->hdr_len;
    return 0;
}

static enum ssv6xxx_beacon_type _ssv6030_hal_beacon_get_hw_valid_cfg()
{
	ssv_type_u32 regval =0;
	regval=__ssv6030_hal_beacon_get_hw_valid_cfg();

	//get MTX_BCN_CFG_VLD

	if(regval==0x2 || regval == 0x0)//bcn 0 is availabke to use.
		return SSV6xxx_BEACON_0;
	else if(regval==0x1)//bcn 1 is availabke to use.
		return SSV6xxx_BEACON_1;
	else
		LOG_PRINTF("=============>ERROR!!drv_bcn_reg_available\n");//ASSERT(FALSE);// 11 error happened need check with ASIC.


	return SSV6xxx_BEACON_0;
}

static int _ssv6030_hal_beacon_fill_content(ssv_type_u32 regaddr, ssv_type_u8 *beacon, int size)
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

int ssv6030_hal_beacon_set(void* beacon_skb, int dtim_offset)
{
    int size = (((struct ssv6200_tx_desc *)beacon_skb)->len)+SSV6051_TX_PB_OFFSET;
	enum ssv6xxx_beacon_type avl_bcn_type = SSV6xxx_BEACON_0;
	ssv_type_bool ret = true;
#if (AUTO_BEACON == 0)
    ssv_type_u8 fw_bcn_st = 0, avl_bcn = 0;
#endif
    //u32 pubf_addr;
    _ssv6030_hal_beacon_reg_lock(1);

    //1.Decide which register can be used to set
	avl_bcn_type = _ssv6030_hal_beacon_get_hw_valid_cfg();

#if (AUTO_BEACON == 0)
    avl_bcn = (ssv_type_u8)(avl_bcn_type + 1);
    fw_bcn_st=_ssv6030_hal_beacon_get_fw_valid_cfg();
    if ((avl_bcn & fw_bcn_st) != 0)
    {
        LOG_DEBUGF(LOG_L2_AP|LOG_LEVEL_WARNING, ("!! Collision %d, %d \r\n",avl_bcn, fw_bcn_st));
        ret = false;
        goto out;
    }
    if(size > 512)
        LOG_ERROR("\33[31m %s:Size is too big for self-sending beacon, need to check fw\33[0m\r\n", __FUNCTION__);

#endif
	//2.Get Pbuf from ASIC
	do{
		if((beacon_usage) & (0x01<<(avl_bcn_type) ))
		{
			if (hw_bcn_info[avl_bcn_type].len >= size)
			{
				break;
			}
			else
			{
				//old beacon too small, need to free
				if(-1 == ssv6030_hal_pbuf_free(hw_bcn_info[avl_bcn_type].pubf_addr))
				{
					ret = false;
					goto out;
				}
                beacon_usage &= ~(0x01<<avl_bcn_type);
			}
		}


		//Allocate new one
		hw_bcn_info[avl_bcn_type].pubf_addr = ssv_hal_pbuf_alloc(size, RX_BUF);
		hw_bcn_info[avl_bcn_type].len = size;
        LOG_PRINTF("Beacon addr %x\r\n",hw_bcn_info[avl_bcn_type].pubf_addr);
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
	_ssv6030_hal_beacon_fill_content(hw_bcn_info[avl_bcn_type].pubf_addr, beacon_skb, size);

	//4. Assign to register let tx know. Beacon is updated.
	_ssv6030_hal_beacon_set_id_dtim(avl_bcn_type,hw_bcn_info[avl_bcn_type].pubf_addr,dtim_offset);

out:
	_ssv6030_hal_beacon_reg_lock(0);

#if (AUTO_BEACON != 0)
    if((ret == true) && beacon_usage && (!beacon_enable))
    {
        if(gDeviceInfo->recovering != TRUE)
            LOG_PRINTF("[A] enable beacon for BEACON_WAITING_ENABLED flags\r\n");
        _ssv6030_hal_beacon_set_info(g_host_cfg.bcn_interval,AP_DEFAULT_DTIM_PERIOD-1);
        ssv6030_hal_beacon_enable(true);
    }
#else
    if(ret == true)
    {
        if (beacon_usage && (!beacon_enable))
        {
            if(gDeviceInfo->recovering != TRUE)
                LOG_PRINTF("[A] enable beacon for BEACON_WAITING_ENABLED flags\r\n");
            _ssv6030_hal_beacon_set_info(g_host_cfg.bcn_interval,AP_DEFAULT_DTIM_PERIOD-1);
            ssv6030_hal_beacon_enable(true);
        }
        OS_MsDelay(g_host_cfg.bcn_interval+10);
    }
#endif


	return (ret==true)?0:-1;
}

int ssv6030_hal_soc_set_bcn(enum ssv6xxx_tx_extra_type extra_type, void *frame, struct cfg_bcn_info *bcn_info, ssv_type_u8 dtim_cnt, ssv_type_u16 bcn_itv)
{
    struct ssv6200_tx_desc *req = (void *)frame;
    ssv_type_u16 len=sizeof(struct ssv6200_tx_desc)+ bcn_info->bcn_len;
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
    _ssv6030_hal_beacon_fill_tx_desc(bcn_info->bcn_len,frame);

    return 0;
}

#endif //#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
