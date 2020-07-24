#define SSV6006_PKT_C
#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <rtos.h>
#include <log.h>
#include <hdr80211.h>
#include <ssv_dev.h>
#include <ssv_devinfo.h>
#include <ssv_frame.h>
#include <ssv_drv.h>
#include <ssv_lib.h>

#include <ssv_ether.h>
#include <cmd_def.h>
#if (AP_MODE_ENABLE == 1)        
#include <ap_def.h>
#include <ap_config.h>
#include <ap_info.h>
#endif

#include "../ssv_hal_if.h"
#include "ssv6006_pktdef.h"
#include "ssv6006_hal.h"

#if(SW_8023TO80211==1)
#include <ieee802_11_defs.h>
#include <ap_sta_info.h>

#define WSID_0                          0
#define WSID_1                          1
#define WSID_GROUP_ADDR               0xe
#endif

#define SSV6006_HAL_CFG_HOST_TXREQ0(host_txreq0,len,c_type,f80211,qos,ht,use_4addr,RSVD0,padding,bc_queue,security,more_data,sub_type,extra_info) \
    do{\
    ssv_type_u32 *temp;                                       \
    temp = (ssv_type_u32*)host_txreq0 ;                       \
    *temp = (len<<0) |                              \
            (c_type<<16)|                           \
            (f80211<<19)|                           \
            (qos<<20)|                              \
            (ht<<21)|                               \
            (use_4addr<<22)|                        \
            (RSVD0<<23)|                            \
            (padding<<24)|                          \
            (bc_queue<<26)|                         \
            (security<<27)|                         \
            (more_data<<28)|                        \
            (sub_type<<29)|                         \
            (extra_info<<31);                        \
            }while(0)

extern struct Host_cfg g_host_cfg;

int ssv6006_hal_dump_txinfo(void *_p)
{
    PKT_TXINFO_DATA_TYPE *p=(PKT_TXINFO_DATA_TYPE *)_p;
    ssv_type_u32		payload_len;
    ssv_type_u8		*dat;
    ssv_type_u8		*a;

    LOG_PRINTF("========= TxInfo =========\n\r");
    LOG_PRINTF("%20s : %d\n\r", "len",p->len);
    LOG_PRINTF("%20s : %d\n\r", "c_type",p->c_type);
    LOG_PRINTF("%20s : %d\n\r", "f80211",p->f80211);
    LOG_PRINTF("%20s : %d\n\r", "qos",p->qos);
    LOG_PRINTF("%20s : %d\n\r", "ht",p->ht);
    LOG_PRINTF("%20s : %d\n\r", "use_4addr",p->use_4addr);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_0",p->RSVD_0);
    LOG_PRINTF("%20s : %d\n\r", "bc_que",p->bc_que);
    LOG_PRINTF("%20s : %d\n\r", "security",p->security);
    LOG_PRINTF("%20s : %d\n\r", "more_data",p->more_data);
    LOG_PRINTF("%20s : %d\n\r", "stype_b5b4",p->stype_b5b4);
    LOG_PRINTF("%20s : %d\n\r", "extra_info",p->extra_info);
    LOG_PRINTF("%20s : 0x%08x\n\r", "fCmd",p->fCmd);
    LOG_PRINTF("%20s : %d\n\r", "hdr_offset",p->hdr_offset);
    LOG_PRINTF("%20s : %d\n\r", "frag",p->frag);
    LOG_PRINTF("%20s : %d\n\r", "unicast",p->unicast);
    LOG_PRINTF("%20s : %d\n\r", "hdr_len",p->hdr_len);
    //LOG_PRINTF("%20s : %d\n\r", "tx_report",p->tx_report);
    //LOG_PRINTF("%20s : %d\n\r", "tx_burst",p->tx_burst);
    //LOG_PRINTF("%20s : %d\n\r", "ack_policy",p->ack_policy);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_1",p->RSVD_1);
    //LOG_PRINTF("%20s : %d\n\r", "do_rts_cts",p->do_rts_cts);
    LOG_PRINTF("%20s : %d\n\r", "reason",p->reason);
    //LOG_PRINTF("%20s : %d\n\r", "payload_offset",p->payload_offset);
    //LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",p->next_frag_pid);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_2",p->RSVD_2);
    LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",p->fCmdIdx);
    LOG_PRINTF("%20s : %d\n\r", "wsid",p->wsid);
    LOG_PRINTF("%20s : %d\n\r", "txq_idx",p->txq_idx);
    LOG_PRINTF("%20s : %d\n\r", "TxF_ID",p->TxF_ID);
    //LOG_PRINTF("%20s : %d\n\r", "rts_cts_nav",p->rts_cts_nav);
    //LOG_PRINTF("%20s : %d\n\r", "frame_consume_time",p->frame_consume_time);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_3",p->RSVD_3);
    // printf("%20s : %d\n\r", "RSVD_5",p->RSVD_5);
    LOG_PRINTF("============================\n\r");
    payload_len = p->len - p->hdr_len;
    LOG_PRINTF("%20s : %d\n\r", "payload_len", payload_len);

    dat = (ssv_type_u8 *)p + p->hdr_offset;
    LOG_PRINTF("========== hdr     ==========\n\r");
    LOG_PRINTF("frame ctl     : 0x%04x\n\r", (((ssv_type_u16)dat[1] << 8)|dat[0]));
    LOG_PRINTF("  - more_frag : %d\n\r", GET_HDR80211_FC_MOREFRAG(p));

    a = (ssv_type_u8*)p + p->hdr_offset +  4;
    LOG_PRINTF("address 1     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[0], a[1], a[2], a[3], a[4], a[5]);
    LOG_PRINTF("address 2     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[6], a[7], a[8], a[9], a[10], a[11]);
    LOG_PRINTF("address 3     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[12], a[13], a[14], a[15], a[16], a[17]);

    LOG_PRINTF("seq ctl       : 0x%04x\n\r", (((ssv_type_u16)dat[23] << 8)|dat[22]));
    LOG_PRINTF("  - seq num   : %d\n\r", GET_HDR80211_SC_SEQNUM(p));
    LOG_PRINTF("  - frag num  : %d\n\r", GET_HDR80211_SC_FRAGNUM(p));


    return 0;
}

int ssv6006_hal_get_valid_txinfo_size(void)
{
	#if(SW_8023TO80211==1)
	return 16; //because we only fill the first 4 words in _ssv6006_fill_txinfo
	#else
    return sizeof(SSV6006_HOST_TXREQ0);	
	#endif
}

int ssv6006_hal_get_txreq0_size(void)
{
    return sizeof(SSV6006_HOST_TXREQ0);
}

int ssv6006_hal_get_txreq0_ctype(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->c_type;
}

int ssv6006_hal_set_txreq0_ctype(void *p,ssv_type_u8 c_type)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->c_type=c_type;
    return 0;
}

int ssv6006_hal_get_txreq0_len(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->len;
}

int ssv6006_hal_set_txreq0_len(void *p,ssv_type_u32 len)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->len=len;
    return 0;
}

int ssv6006_hal_get_txreq0_rsvd0(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->rvdtx_0;
}

int ssv6006_hal_set_txreq0_rsvd0(void *p,ssv_type_u32 val)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->rvdtx_0=val;
    return 0;
}
int ssv6006_hal_get_txreq0_padding(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->padding;
}

int ssv6006_hal_set_txreq0_padding(void *p, ssv_type_u32 val)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->padding=val;
    return 0;
}

int ssv6006_hal_get_txreq0_qos(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->qos;
}

int ssv6006_hal_get_txreq0_ht(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->ht;
}

int ssv6006_hal_get_txreq0_4addr(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->use_4addr;
}

int ssv6006_hal_set_txreq0_f80211(void *p,ssv_type_u8 f80211)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->f80211=f80211;
    return 0;
}

int ssv6006_hal_get_txreq0_f80211(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->f80211;
}

int ssv6006_hal_set_txreq0_bssid_idx(void *p,ssv_type_u8 idx)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->bssidx=idx;
    return 0;
}

int ssv6006_hal_get_txreq0_bssid_idx(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->bssidx;
}

int ssv6006_hal_get_txreq0_more_data(void *p)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    return _p->more_data;
}

int ssv6006_hal_set_txreq0_more_data(void *p,ssv_type_u8 more_data)
{
    SSV6006_HOST_TXREQ0 *_p=(SSV6006_HOST_TXREQ0 *)p;
    _p->more_data=more_data;
    return 0;
}

ssv_type_u8* ssv6006_hal_get_txreq0_qos_ptr(void *_req0)
{
    SSV6006_HOST_TXREQ0 *req0=(SSV6006_HOST_TXREQ0 *)_req0;
    ssv_type_u16 nOffset = 0;
    switch (req0->c_type)
    {
        #if(SW_8023TO80211==1)
        case SSV6XXX_TX_REQ2:
        #endif
        case SSV6XXX_TX_REQ0:
            nOffset += sizeof(SSV6006_HOST_TXREQ0);
            break;
        //case SSV6XXX_TX_REQ1:
            //nOffset += sizeof(struct cfg_host_txreq1);
            //break;
        // case SSV6XXX_TX_REQ2:
            //nOffset += sizeof(struct cfg_host_txreq2);
            //break;
        default:
            SSV_ASSERT(FALSE);
            break;
    }

    if (req0->use_4addr)
        nOffset+=ETHER_ADDR_LEN;

    if (!req0->qos)
        return NULL;

    if (req0->ht)
        nOffset+=IEEE80211_HT_CTL_LEN;

    return (ssv_type_u8*)(((ssv_type_u8*)req0)+nOffset);
}

ssv_type_u8* ssv6006_hal_get_txreq0_data_ptr(void *_req0)
{
    SSV6006_HOST_TXREQ0 *req0=(SSV6006_HOST_TXREQ0 *)_req0;
    ssv_type_u16 nOffset = 0;
    switch (req0->c_type)
    {
        #if(SW_8023TO80211==1)
        case SSV6XXX_TX_REQ2:
        #endif
        case SSV6XXX_TX_REQ0:
        	nOffset += sizeof(SSV6006_HOST_TXREQ0);
        	break;
        //case SSV6XXX_TX_REQ1:
            //nOffset += sizeof(struct cfg_host_txreq1);
            //break;
        //case SSV6XXX_TX_REQ2:
            //nOffset += sizeof(struct cfg_host_txreq2);
            //break;
        default:
            SSV_ASSERT(FALSE);
            break;
    }


    if (req0->use_4addr)
        nOffset+=ETHER_ADDR_LEN;

    if (req0->qos)
        nOffset+=IEEE80211_QOS_CTL_LEN;

    if (req0->ht)
        nOffset+=IEEE80211_HT_CTL_LEN;

    nOffset+=req0->padding;
    return (ssv_type_u8*)(((ssv_type_u8*)req0)+nOffset);
}


#if(SW_8023TO80211==1)
ssv_type_u32 _ssv6006_hal_tx_8023to80211(void *frame, ssv_type_u8 l3offset, ssv_type_u16 *qos, ssv_type_u8 security,u8 up)
{
    ETHER_ADDR da, sa;
    ssv_type_u8 ds;
    ssv_type_u8 *raw=NULL;
    ssv_type_bool bqos=FALSE,bht=FALSE,baddr4=FALSE;
    ssv_type_u16 eth_type;
    ssv_type_u8 fromDS, toDS;
    ssv_type_u32 _ht=0;
    ssv_type_u16 _qos=0;
    // the frame fromat from LWIP
    // ======================================================================
    // | DA(6Bytes)|SA(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|
    // ======================================================================

    //Save the da & sa
    raw=OS_FRAME_GET_DATA(frame);
    raw+=l3offset;

    da = *((ETHER_ADDR *)raw);
    sa = *((ETHER_ADDR *)(raw+ETHER_ADDR_LEN));
    eth_type = (raw[12]<<8) | raw[13];

    raw+=ETHER_ADDR_LEN; //Ponit to SA

    //Replace SA to LLC header
    raw[0] = 0xAA;
    raw[1] = 0xAA;
    raw[2] = 0x03;
    raw[3] = 0x00;
    raw[4] = 0x00;
    if (eth_type==0x80f3 || eth_type==0x809b || eth_type==0x8137)
        raw[5] = 0xF8;  /* 802.1H LLC Encapsulation*/
    else raw[5] = 0x00; /* RFC_1024 LLC Encapsulation*/


    //Fill the ht field of MPDU
    #if 0
    if(ht!=NULL)
    {
        raw-=IEEE80211_HT_CTL_LEN;
        _ht = *ht;
        OS_MemCPY(raw,(u8 *)(&_ht),IEEE80211_HT_CTL_LEN);
        bht=TRUE;
    }
    #endif

    //Fill the qos field of MPDU
    if(qos!=NULL)
    {
        raw-=IEEE80211_QOS_CTL_LEN;
        _qos = ((*qos)&0xFFF8)|up;
        OS_MemCPY(raw,(ssv_type_u8 *)(&_qos),IEEE80211_QOS_CTL_LEN);
        bqos=TRUE;
    }

    #if 0
    //Fill the addr4 field of MPDU
    if(addr4!=NULL)
    {
        raw-=ETHER_ADDR_LEN;
        OS_MemCPY(raw,addr4,ETHER_ADDR_LEN);
        baddr4=TRUE;
    }
    #endif


    //Fill 802.11 header
    raw-=24;

    #if 0
    SSV_PACKED_STRUCT_BEGIN  struct FC_Field_st {
        u16                 ver:2;
        u16                 type:2;
        u16                 subtype:4;
        u16                 toDS:1;
        u16                 fromDS:1;
        u16                 MoreFlag:1;
        u16                 Retry:1;
        u16                 PwrMgmt:1;
        u16                 MoreData:1;
        u16                 Protected:1;
        u16                 order:1;

    };
    #endif
    raw[0]=0;
    raw[0]=(0)|                  //ver:2
           ((FT_DATA>>2)<<2)|    //type:2
           ((0 | (bqos<<3))<<4); //subtype:4

    raw[1]=0;
    toDS=(gDeviceInfo->hw_mode == SSV6XXX_HWM_STA)?1:0;
    fromDS=(gDeviceInfo->hw_mode == SSV6XXX_HWM_AP)?1:0;

    raw[1]=(toDS)|                  //toDS:1
           (fromDS<<1) |            //fromDS:1
           (0<<2) |                 //MoreFlag:1
           (0<<3) |                 //Retry:1
           (0<<4) |                 //PwrMgmt:1
           (0<<5) |                 //MoreData:1
           ((security&0x01)<<6)|    //Protected:1
           (0<<7);                  //order:1

    #if 0
    typedef struct HDR80211_Data_st {
    	FC_Field            fc;
    	u16                 dur;
    	ETHER_ADDR          addr1;
    	ETHER_ADDR          addr2;
    	ETHER_ADDR          addr3;
    	u16                 seq_ctrl;
    } HDR80211_Data, *PHDR80211_Data;
    #endif
    raw[2]=0; //dur
    raw[3]=0; //dur
    ds = (fromDS<<1) | toDS;
    switch(ds)
    {
    case 0x01: /* To-AP: addr1=BSSID, addr2=SA, addr3=DA */
        //drv_mac_get_bssid(&(hdr_data->addr1.addr[0]));
        //hdr_data->addr2 = sa;
        //hdr_data->addr3 = da;
        OS_MemCPY((void *)(&raw[4]),(void *)(gDeviceInfo->StaInfo->joincfg->bss.bssid.addr),ETHER_ADDR_LEN);
        OS_MemCPY((void *)(&raw[10]),(void *)(sa.addr),ETHER_ADDR_LEN);
        OS_MemCPY((void *)(&raw[16]),(void *)(da.addr),ETHER_ADDR_LEN);
        break;
    case 0x02: /* From-AP: addr1=DA, addr2=BSSID, addr3=SA */
        //hdr_data->addr1 = da;
        //drv_mac_get_bssid(&(hdr_data->addr2.addr[0]));
        //hdr_data->addr3 = sa;
        OS_MemCPY((void *)(&raw[4]),(void *)(da.addr),ETHER_ADDR_LEN);
        OS_MemCPY((void *)(&raw[10]),(void *)(gDeviceInfo->self_mac),ETHER_ADDR_LEN);
        OS_MemCPY((void *)(&raw[16]),(void *)(sa.addr),ETHER_ADDR_LEN);
        break;
    default: ssv_assert(0);
    }

    if(gDeviceInfo->hw_mode == SSV6XXX_HWM_AP)
    {
        //not yet
        APStaInfo_st *apStaInfo=APStaInfo_FindStaByAddr((ETHER_ADDR *)da.addr);
        if(apStaInfo!=NULL)
        {
            apStaInfo->seq_ctl[up]++;
            raw[22]=(apStaInfo->seq_ctl[up]&0xFFF)<<4; //seq_ctrl
            raw[23]=(apStaInfo->seq_ctl[up]&0xFFF)>>8; //seq_ctrl
        }
        else
        {
            raw[22]=0;
            raw[23]=0;
        }
    }
    else
    {
        struct StaInfo *staInfo = gDeviceInfo->StaInfo;
        staInfo->seq_ctl[up]++;
        raw[22]=(staInfo->seq_ctl[up]&0xFFF)<<4; //seq_ctrl
        raw[23]=(staInfo->seq_ctl[up]&0xFFF)>>8; //seq_ctrl
    }
    return ((ssv_type_u32)raw-(ssv_type_u32)OS_FRAME_GET_DATA(frame));
}

ssv_type_u8 _wsid_lookup(ETHER_ADDR *da)
{
    ssv_type_u8 index, max_index=g_host_cfg.ap_supported_sta_num;

    if(da==NULL)
    {
        return WSID_NOT_FOUND;
    }

    if(da->addr[0]&0x01)
    {
        return WSID_GROUP_ADDR;
    }

    if(gDeviceInfo->hw_mode==SSV6XXX_HWM_STA)
    {
        if(gDeviceInfo->StaInfo->status!=CONNECT)
        {
            return WSID_NOT_FOUND;
        }
        return WSID_0;
    }


    for(index=0;index<max_index;index++)
    {
        struct APStaInfo *pStaInfo=NULL;
        pStaInfo=(struct APStaInfo *)(&gDeviceInfo->APInfo->StaConInfo[index]);
        if(pStaInfo->aid!=0)
        {
            if(0==ssv6xxx_memcmp((void *)pStaInfo->addr,(void *)da->addr,ETHER_ADDR_LEN))
            {
                return pStaInfo->wsid_idx;
            }
        }
    }

    return WSID_NOT_FOUND;

}

extern ssv_type_u32 g_ap_tx_data_flow;
extern ssv_type_u32 g_ap_tx_mgmt_flow;
extern ssv_type_u32 g_sta_tx_data_flow;

static int _ssv6006_fill_txinfo(void *frame)
{
    ssv_type_u8 AC_TBL[8]={ AC_BE, AC_BK, AC_BK, AC_BE, AC_VI, AC_VI, AC_VO, AC_VO };
    PKT_TxInfo *PktInfo=NULL;
    ssv_type_u8 *raw=NULL;
    ssv_type_u8 *p=NULL;
    ETHER_ADDR *da=NULL;
    ssv_type_u16 fc=0;
    ssv_type_u8 *qos, tid, bad_len, hdr_len=24, ack_policy=0,wsid;
    ssv_type_u32 i=0;
    ssv_type_u32 temp=0;
    ssv_type_u8 hdr_offset=SSV6006_TX_PB_OFFSET;

    ssv_type_u8 padding_len=0;
    padding_len=(((ssv_type_u32)OS_FRAME_GET_DATA(frame))&(0x3));
    if(0!=padding_len){
        padding_len=4-padding_len;
        os_frame_push(frame,padding_len);
    }

    PktInfo=(PKT_TxInfo *)OS_FRAME_GET_DATA(frame);

    OS_MemSET(PktInfo,0,sizeof(PKT_TxInfo));
    raw=(ssv_type_u8 *)((ssv_type_u32)PktInfo+sizeof(PKT_TxInfo)+padding_len);
    //ssv6xxx_raw_dump((char *)raw,32);
    da=(ETHER_ADDR *)(raw+4);
    fc=(raw[1]<<8) | raw[0];

    /**
        * Check DATA frame length. The frame length shall at least be
        * 24, 26, 30 or 36 bytes.
        */
    #if 0
    if (IS_4ADDR_FORMAT(fc))
        hdr_len += ETHER_ADDR_LEN;
    #endif

    if (IS_QOS_DATA(fc)) {
        qos = raw + hdr_len;
        tid = qos[0]&0x0F;
        ack_policy = (raw[hdr_len]>>5)&0x03;
        hdr_len += IEEE80211_QOS_CTL_LEN;
        if (IS_ORDER_SET(fc))
            hdr_len += IEEE80211_HT_CTL_LEN;
    }


    /* Decide ACK Policy: */
    if (!(raw[4]&0x01)) { /* unicast */
        if (IS_QOS_DATA(fc))
            ack_policy = ack_policy;
        else
            ack_policy = 0x00; /* normal ack */
    }
    else
        ack_policy = 0x01; /* frames with group address: no ack */

    wsid=_wsid_lookup(da);
    /*
    Only check the data frame here.
    If DUT as AP mode, the probe response frame is WSID_NOT_FOUND
    */
    if(IS_QOS_DATA(fc))//(gDeviceInfo->hw_mode != SSV6XXX_HWM_AP)
    {
        if(wsid==WSID_NOT_FOUND)
        {
            LOG_PRINTF("Error!! wsid not found da=[%2x:%2x:%2x:%2x:%2x:%2x]\r\n",da->addr[0],da->addr[1],da->addr[2],da->addr[3],da->addr[4],da->addr[5]);
            return -1;
        }
    }

    /* The definition of WORD_1: */
    #if 0
    u32             len:16;
    u32             c_type:3;
    u32             f80211:1;
    u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    u32             use_4addr:1;
    u32             RSVD_0:1;
    u32             padding:2;
    u32             bc_que:1;
    u32				security:1;
    u32             more_data:1;
    u32             stype_b5b4:2;
    u32             extra_info:1;   /* 0: don't trap to cpu after parsing, 1: trap to cpu after parsing */
    #endif
    temp=0;
    temp=OS_FRAME_GET_DATA_LEN(frame)|      //(hdr_offset+(ether_frame_len-ETHER_ADDR_LEN+24+2))|  //len:16
         (M2_TXREQ<<16)|                    //c_type:3
         (1<<19)|                           //f80211:1
         (((IS_QOS_DATA(fc))?1:0)<<20)|     //qos:1
         (0<<21)|                           //ht:1
         (0<<22)|                           //use_4addr:1
         (0<<23)|                           //RSVD_0:1
         ((padding_len&0x03)<<24)|          //padding:2
         (0<<26)|                           //bc_que:1
         (((IS_PROTECT_SET(fc))?1:0)<<27)|  //security:1
         (0<<28)|                           //more_data:1
         (0<<29)|                           //stype_b5b4:2
         (0<<31);                           //extra_info:1
    p=(ssv_type_u8 *)((ssv_type_u32)PktInfo);
    p[0]=((ssv_type_u8 *)(&temp))[0];
    p[1]=((ssv_type_u8 *)(&temp))[1];
    p[2]=((ssv_type_u8 *)(&temp))[2];
    p[3]=((ssv_type_u8 *)(&temp))[3];
    //LOG_PRINTF("WORD_1=0x%08x\r\n",temp,p[0]);

    /* The definition of WORD_2: */
    #if 0
    u32             fCmd;
    #endif
    if(gDeviceInfo->hw_mode==SSV6XXX_HWM_AP)
    {
        if(IS_DATA_FRAME(fc))
        {
            temp=g_ap_tx_data_flow;
        }
        else
        {
            temp=g_ap_tx_mgmt_flow;
        }

        p=(ssv_type_u8 *)((ssv_type_u32)PktInfo+4);
        p[0]=((ssv_type_u8 *)(&temp))[0];
        p[1]=((ssv_type_u8 *)(&temp))[1];
        p[2]=((ssv_type_u8 *)(&temp))[2];
        p[3]=((ssv_type_u8 *)(&temp))[3];
        //LOG_PRINTF("WORD_2=0x%08x\r\n",g_ap_tx_data_flow);
    }
    else
    {
        //We don't take care mgmt frames here, becuase mamgt frames are handled in fw
        p=(ssv_type_u8 *)((ssv_type_u32)PktInfo+4);
        p[0]=((ssv_type_u8 *)(&g_sta_tx_data_flow))[0];
        p[1]=((ssv_type_u8 *)(&g_sta_tx_data_flow))[1];
        p[2]=((ssv_type_u8 *)(&g_sta_tx_data_flow))[2];
        p[3]=((ssv_type_u8 *)(&g_sta_tx_data_flow))[3];
        //LOG_PRINTF("WORD_2=0x%08x\r\n",g_sta_tx_data_flow);
    }


    /* The definition of WORD_3: */
    #if 0
     u32             hdr_offset:8;
     u32             frag:1;
     u32             unicast:1;
     u32             hdr_len:6;
     u32             no_pkt_buf_reduction:1;
     u32             tx_burst_obsolete:1;     /* 0: normal, 1: burst tx */
     u32             ack_policy_obsolete:2; //ack_policy_obsolete:2;   /* See Table 8-6, IEEE 802.11 Spec. 2012 */
     u32             aggr:2;
     u32             rsvdtx_1:1;              // for sw retry ampdu
     u32             is_rate_stat_sample_pkt:1;
                                              // new , v2.13
     u32             bssidx:2;                // new , v2.13, change location
     u32             reason:6;
    #endif
    temp=0;
    temp=(hdr_offset)|                                        //hdr_offset:8
          (((IS_MOREFLAG_SET(fc)||(raw[22]&0x0F))? 1: 0)<<8)| //frag:1
          (((raw[4]&0x01)? 0: 1)<<9)|                         //unicast:1
          (hdr_len<<10) |                                     //hdr_len:6
          (0<<16) |                                           //no_pkt_buf_reduction:1
          (0<<17) |                                           //tx_burst_obsolete:1
          (0<<18) |                                           //ack_policy_obsolete:2
          (0<<20) |                                           //aggr:2
          (0<<22) |                                           //rsvdtx_1:1
          (0<<23) |                                           //is_rate_stat_sample_pkt:1
          (0<<24) |                                           //bssidx:2
          (0<<26);                                            //reason :6

    p=(ssv_type_u8 *)((ssv_type_u32)PktInfo+8);
    p[0]=((ssv_type_u8 *)(&temp))[0];
    p[1]=((ssv_type_u8 *)(&temp))[1];
    p[2]=((ssv_type_u8 *)(&temp))[2];
    p[3]=((ssv_type_u8 *)(&temp))[3];
    //LOG_PRINTF("WORD_3=0x%08x\r\n",temp);

    /* The definition of WORD_4: */
    #if 0
    u32             payload_offset_obsolete:8;
    u32             tx_pkt_run_no:8;         // new, v2.13
    u32             fCmdIdx:3;
    u32             wsid:4;
    u32             txq_idx:3;
    u32             TxF_ID:6;
    #endif
    temp=0;
    temp=(0)|                                            //payload_offset_obsolete:8
         (0<<8)|                                         //tx_pkt_run_no:8
         (1<<16)|                                        //fCmdIdx:3
         (wsid<<19)|                                     //wsid:4
         (((IS_QOS_DATA(fc))?AC_TBL[tid&0x07]:0)<<23)|   //txq_idx:3
         (0<<26);                                        //TxF_ID:6

    p=(ssv_type_u8 *)((ssv_type_u32)PktInfo+12);
    p[0]=((ssv_type_u8 *)(&temp))[0];
    p[1]=((ssv_type_u8 *)(&temp))[1];
    p[2]=((ssv_type_u8 *)(&temp))[2];
    p[3]=((ssv_type_u8 *)(&temp))[3];
    //LOG_PRINTF("WORD_4=0x%08x\r\n",temp);

    return 0;

}

void * ssv6006_hal_fill_txreq0(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security, ssv_type_u8 tx_dscrp_flag)
{
    //Fill M2 Type
    if(-1==_ssv6006_fill_txinfo(frame))
    {
        return NULL;
    }
    else
    {
        //ssv6xxx_raw_dump((char *)OS_FRAME_GET_DATA(frame),128);
        return frame;
    }

}

int ssv6006_hal_tx_8023to80211(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security)
{
    ssv_type_u8 push_len=0;
    /*
    #define DRV_TRX_HDR_LEN 80+20
    g_host_cfg.trx_hdr_len=DRV_TRX_HDR_LEN

    TXINFO_SIZE is 80 bytes for PKTTxInfo
    20 bytes is for 802.11 header and QoS Field.

    802.3  v.s. 802.11 (QoS)
    |PBUF STRUCT|------DRV_TRX_HDR_LEN(100Bytes)-----|     DA(6BYTES)   | SA(6BYTES)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|
    |PBUF STRUCT|TX INFO SIZE(80Byts)|802.11 header(24BYTES)|QOS(2Bytes)|LLC(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|
                                     |<-    20Bytes->|

    802.3  v.s. 802.11 (w/o QoS)
    |PBUF STRUCT|-----DRV_TRX_HDR_LEN(100Bytes)-2-----|     DA(6BYTES)   | SA(6BYTES)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|
    |PBUF STRUCT|     TX INFO SIZE(80Byts)            |802.11 header(24BYTES)|LLC(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|

    */

    if(qos!=NULL)
        push_len=g_host_cfg.trx_hdr_len;
    else
        push_len=g_host_cfg.trx_hdr_len-2;

    if(NULL==os_frame_push(frame,push_len))
    {
        /*
        This is a error handle for ARP Request ...

        1. Received ARP Request frame
        |PBUF STRUCT|RX INFO SIZE(80Byts)|802.11 header(24BYTES)|LLC(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|

        2. 802.11 to 802.3 for ARP Request
        |PBUF STRUCT|RX INFO SIZE(80Byts)|--18Bytes--|DA(6Bytes)| SA(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|

        3. LWIP resue pbuf of ARP Request for ARP Response, and sent the same pbuf to L2 driver
        |PBUF STRUCT|RX INFO SIZE(80Byts)|--18Bytes--|DA(6Bytes)| SA(6Bytes)|TYPE(2BYTES)|IP Header|TCP Header|TCP Payload|

        The L2 driver only have 80+18 free spaces, it's not enough to fill the 802.11 header + QoS Field

        */
        push_len-=2;
        if(NULL==os_frame_push(frame,push_len))
        {
            LOG_PRINTF("ssv6006_hal_fill_txreq0 push frame fail\r\n");
            return -1;
        }
        qos=NULL;
    }

    _ssv6006_hal_tx_8023to80211(frame,(ssv_type_u32)push_len,qos,security,priority);

    return 0;
}


#else //#if(SW_8023TO80211==1)

int ssv6006_hal_tx_8023to80211(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security)
{
    return -1;
}

void * ssv6006_hal_fill_txreq0(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security, ssv_type_u8 tx_dscrp_flag, ssv_type_u8 vif_idx)
{
    SSV6006_HOST_TXREQ0 *host_txreq0=NULL;
    ssv_type_u32 *pht;
    ssv_type_u8 *pos;
    ssv_type_u32 extra_len=0;
    ssv_type_u32 padding_len=0;
    ssv_type_u8 sub_type=0;
    ssv_type_bool bqos=FALSE,bht=FALSE,baddr4=FALSE;
    if(f80211 == 0)
    {
        extra_len += (qos!=NULL)? IEEE80211_QOS_CTL_LEN: 0;
        extra_len += (ht!=NULL)? IEEE80211_HT_CTL_LEN: 0;
        extra_len += (addr4!=NULL)? ETHER_ADDR_LEN: 0;
    }

    extra_len += ssv_hal_get_txreq0_size();
    os_frame_push(frame,extra_len);

    padding_len=(((ssv_type_u32)OS_FRAME_GET_DATA(frame))&(0x3));
    if(0!=padding_len){
        padding_len=4-padding_len;
        os_frame_push(frame,padding_len);
    }

    host_txreq0 = (SSV6006_HOST_TXREQ0 *)OS_FRAME_GET_DATA(frame);
    OS_MemSET((void*)host_txreq0,0,sizeof(SSV6006_HOST_TXREQ0));
    pos = (ssv_type_u8*)host_txreq0 + ssv_hal_get_txreq0_size();

    //LOG_PRINTF("host_txreq0=0x%x,extra_len = %d,txrq_s=%d,host_txreq0->len=%d,tot_len=%d,%d\r\n",(u32)host_txreq0,
    //    extra_len,sizeof(struct cfg_host_txreq0),host_txreq0->len,((struct pbuf *)frame)->tot_len,OS_FRAME_GET_DATA_LEN(frame));

    //host_txreq0->extra_info = !!IS_BIT_SET(tx_dscrp_flag, TX_DSCRP_SET_EXTRA_INFO);
    //host_txreq0->bc_queue = !!IS_BIT_SET(tx_dscrp_flag, TX_DSCRP_SET_BC_QUE);

    if (f80211 == 0) {

        if (NULL!=addr4) {
            OS_MemCPY(pos, addr4, ETHER_ADDR_LEN);
            pos += ETHER_ADDR_LEN;
            baddr4=TRUE;
        }

        if (NULL!=qos) {
            ssv_type_u16* q_ctrl = (ssv_type_u16*)pos;
            *q_ctrl = ((*qos)&0xFFF8)|priority;
            pos += IEEE80211_QOS_CTL_LEN;
            bqos=TRUE;
        }

        if (NULL!=ht) {
            //ht_st = (struct ht_ctrl_st *)pos;
            //ht_st->ht = gDeviceInfo->ht_ctrl;
            pht = (ssv_type_u32 *)pos;
            *pht = *ht;
            pos += IEEE80211_HT_CTL_LEN;
            bht=TRUE;
        }

    }
    else
    {
        /* speicify "stype_b5b4" field of TxInfo */
        sub_type = pos[0]>>4;
    }

    SSV6006_HAL_CFG_HOST_TXREQ0(host_txreq0,
        OS_FRAME_GET_DATA_LEN(frame),
        M0_TXREQ,
        f80211,
        bqos,
        bht,
        baddr4,
        0,
        padding_len,
        (!!SSV_IS_BIT_SET(tx_dscrp_flag, TX_DSCRP_SET_BC_QUE)<<26),
        security,
        0,
        sub_type,
        (!!SSV_IS_BIT_SET(tx_dscrp_flag, TX_DSCRP_SET_EXTRA_INFO)<<31));

    host_txreq0->bssidx=vif_idx;//0;
    host_txreq0->rsvdtx_1=0; //clear this filed for sw retry ampdu in wifi-fw
    return frame;
}
#endif //#if(SW_8023TO80211==1)


struct ADDROffsetTbl {
    ssv_type_u16                 ra;
    ssv_type_u16                 ta;
    ssv_type_u16                 da;
    ssv_type_u16                 sa;
    ssv_type_u16                 bssid;
};

struct ADDROffsetTbl AddrOffsetTbl[4] = {
    /*        RA                    TA                    DA                   SA                  BSSID        */
    { OFFSET_ADDR1, OFFSET_ADDR2, OFFSET_ADDR1, OFFSET_ADDR2, OFFSET_ADDR3 },
    { OFFSET_ADDR1, OFFSET_ADDR2, OFFSET_ADDR3, OFFSET_ADDR2, OFFSET_ADDR1 },
    { OFFSET_ADDR1, OFFSET_ADDR2, OFFSET_ADDR1, OFFSET_ADDR3, OFFSET_ADDR2 },
    { OFFSET_ADDR1, OFFSET_ADDR2, OFFSET_ADDR3, OFFSET_ADDR4, 0            },
};

#define IS_4_BYTE_ALIGN(x)              ((((ssv_type_u32)(x))&0x03) == 0)
#define IS_2_BYTE_ALIGN(x)              ((((ssv_type_u32)(x))&0x01) == 0)
#define GET_PKT_PAYLOAD_OFFSET(p) (((PKT_Info *)p)->hdr_offset+((PKT_Info *)p)->hdr_len)

int ssv6006_hal_rx_80211to8023(void *_PktRxInfo)
{
    PKT_RxInfo *PktRxInfo=(PKT_RxInfo *)_PktRxInfo;
    ssv_type_u16 fc;
    ssv_type_u8 rfc1042[]={ 0xAA, 0xAA, 0x03, 0x00, 0x00, 0x00 };
    ssv_type_u8 _802_1h[]={ 0xAA, 0xAA, 0x03, 0x00, 0x00, 0xF8 };
    HDR80211_Data *hdr_data;
    ETHER_ADDR da, sa;
    ssv_type_u8 *dat, ds;
    ETHER_ADDR a4;
	ssv_type_u32 ht_ctl;
    ssv_type_u16 encap_offset=0,qos_ctl;
    dat = (ssv_type_u8 *)((ssv_type_u32)PktRxInfo + PktRxInfo->hdr_offset);
    hdr_data = (HDR80211_Data *)dat;
    fc = dat[0] | (dat[1]<<8);
    //dat = (u8 *)PktRxInfo + PktRxInfo->payload_offset;
    dat = (ssv_type_u8 *)PktRxInfo + GET_PKT_PAYLOAD_OFFSET(PktRxInfo);



    if (IS_DATA_FRAME(fc))
    {
        /**
       * Check if LLC Header in the Rx frame is correct. If the LLC
       * Header is invalid, trapping to CPU or discard by hardware.
       */
       //LOG_PRINTF("rx 80211to8023 (len=%d, hdr_len=%d, hdr_offset=%d)\r\n",PktRxInfo->len,PktRxInfo->hdr_len,PktRxInfo->hdr_offset);
       //ssv6xxx_raw_dump((char *)_PktRxInfo,128);
        if(!IS_NULL_DATA(fc))
        {
            if ((ssv6xxx_memcmp(dat, rfc1042, ETHER_ADDR_LEN)!=0) &&
                (ssv6xxx_memcmp(dat, _802_1h, ETHER_ADDR_LEN)!=0)) {
                //Now, we prefer to discard in hardware, beause we didn't enable the trapreason 15 of ADR_REASON_TRAP0
                //LOG_PRINTF("LLC header in the RX frame is not correct\r\n");
                //ssv6xxx_raw_dump((char *)_PktRxInfo,128);
                goto err;
            }
        }
        //ssv6xxx_raw_dump((char *)PktRxInfo,128);
        //return 0;
        /* DMAC & SMAC: */
        dat = (ssv_type_u8 *)PktRxInfo + PktRxInfo->hdr_offset;
        ds = (hdr_data->fc.fromDS<<1) | hdr_data->fc.toDS;
        OS_MemCPY((void *)(&da), (void *)(dat+AddrOffsetTbl[ds].da), ETHER_ADDR_LEN);
        OS_MemCPY((void *)(&sa), (void *)(dat+AddrOffsetTbl[ds].sa), ETHER_ADDR_LEN);

        /*|MAC|QOS|HT|*/
        #if 0
        if(PktRxInfo->use_4addr== 1)
        {
            OS_MemCPY((void *)(a4.addr), (void *)(dat+OFFSET_ADDR4), ETHER_ADDR_LEN);
            encap_offset += ETHER_ADDR_LEN;
        }
        #endif

        if(PktRxInfo->qos== 1)
        {
            OS_MemCPY((void *)(&qos_ctl), (void *)(dat+OFFSET_ADDR4+encap_offset), IEEE80211_QOS_CTL_LEN);
            encap_offset += IEEE80211_QOS_CTL_LEN;
        }

        #if 0
        if(PktRxInfo->ht== 1)
        {
            OS_MemCPY((void *)(&ht_ctl), (void *)(dat+OFFSET_ADDR4+encap_offset), IEEE80211_HT_CTL_LEN);
            encap_offset += IEEE80211_HT_CTL_LEN;
        }
        #endif

        /**
              * Nothing wrong to LLC and ether headers. Then we can decapsulate
              * IEEE 802.11 Header and LLC from the frame and encapsulate ether
              * header to the frame. After that we need to adjust the info of packet
              * description header (Rx Packet Info).
              */
        dat = (ssv_type_u8 *)PktRxInfo + GET_PKT_PAYLOAD_OFFSET(PktRxInfo);
        //Overwrite LLC (6bytes)
        OS_MemCPY((void *)dat, (void *)(&sa), ETHER_ADDR_LEN);
        dat -= ETHER_ADDR_LEN;
        OS_MemCPY((void *)dat, (void *)(&da), ETHER_ADDR_LEN);

        /*|MAC|QOS|HT|*/
        if(PktRxInfo->ht== 1)
        {
            dat -= IEEE80211_HT_CTL_LEN;
            OS_MemCPY((void *)(dat), (void *)(&ht_ctl), IEEE80211_HT_CTL_LEN);
        }
        if(PktRxInfo->qos== 1)
        {
            dat -= IEEE80211_QOS_CTL_LEN;
            OS_MemCPY((void *)dat, (void *)(&qos_ctl), IEEE80211_QOS_CTL_LEN);
        }
        if(PktRxInfo->use_4addr== 1)
        {
            dat -= ETHER_ADDR_LEN;
            OS_MemCPY((void *)dat, (void *)(a4.addr), ETHER_ADDR_LEN);
        }

        PktRxInfo->len = PktRxInfo->len - PktRxInfo->hdr_len + ETHER_ADDR_LEN + encap_offset;
        PktRxInfo->hdr_offset =GET_PKT_PAYLOAD_OFFSET(PktRxInfo)-ETHER_ADDR_LEN-encap_offset;
        PktRxInfo->hdr_len = encap_offset;
        PktRxInfo->f80211 = 0;

        //LOG_PRINTF("rx 80211to8023 end (len=%d hdr_len=%d, hdr_offset=%d,encap_offset=%d)\r\n",PktRxInfo->len,PktRxInfo->hdr_len,PktRxInfo->hdr_offset,encap_offset);
        //ssv6xxx_raw_dump((char *)_PktRxInfo,128);

    }
    /* For Simulation Only. ASIC shall ignore this !! */
    return 0;
err:
    return -1;
}


int ssv6006_hal_dump_rxinfo(void *_p)
{
    PKT_RXINFO_DATA_TYPE *p=(PKT_RXINFO_DATA_TYPE *)_p;
    ssv_type_u32		payload_len;
    ssv_type_u8		*dat;
    ssv_type_u8		*a;

    LOG_PRINTF("========= RxInfo =========\n\r");
    LOG_PRINTF("%20s : %d\n\r", "len",p->len);
    LOG_PRINTF("%20s : %d\n\r", "c_type",p->c_type);
    LOG_PRINTF("%20s : %d\n\r", "f80211",p->f80211);
    LOG_PRINTF("%20s : %d\n\r", "qos",p->qos);
    LOG_PRINTF("%20s : %d\n\r", "ht",p->ht);
    //LOG_PRINTF("%20s : %d\n\r", "l3cs_err",p->l3cs_err);
    //LOG_PRINTF("%20s : %d\n\r", "l4cs_err",p->l4cs_err);
    LOG_PRINTF("%20s : %d\n\r", "use_4addr",p->use_4addr);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_0",p->RSVD_0);
    LOG_PRINTF("%20s : %d\n\r", "psm",p->psm);
    LOG_PRINTF("%20s : %d\n\r", "stype_b5b4",p->stype_b5b4);
    //LOG_PRINTF("%20s : %d\n\r", "extra_info",p->extra_info);
    //LOG_PRINTF("%20s : 0x%08x\n\r", "fCmd",p->fCmd);
    LOG_PRINTF("%20s : %d\n\r", "hdr_offset",p->hdr_offset);
    LOG_PRINTF("%20s : %d\n\r", "frag",p->frag);
    LOG_PRINTF("%20s : %d\n\r", "unicast",p->unicast);
    LOG_PRINTF("%20s : %d\n\r", "hdr_len",p->hdr_len);
    LOG_PRINTF("%20s : 0x%x\n\r", "RxResult",p->RxResult);
    //LOG_PRINTF("%20s : %d\n\r", "wildcard_bssid",p->wildcard_bssid);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_1",p->RSVD_1);
    LOG_PRINTF("%20s : %d\n\r", "reason",p->reason);
    //LOG_PRINTF("%20s : %d\n\r", "payload_offset",p->payload_offset);
    //LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",p->next_frag_pid);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_2",p->RSVD_2);
    LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",p->fCmdIdx);
    LOG_PRINTF("%20s : %d\n\r", "wsid",p->wsid);
    //LOG_PRINTF("%20s : %d\n\r", "RSVD_3",p->RSVD_3);
    //	LOG_PRINTF("%20s : %d\n\r", "RxF_ID",p->RxF_ID);
    LOG_PRINTF("============================\n\r");

    payload_len = p->len - p->hdr_len;
    LOG_PRINTF("%20s : %d\n\r", "payload_len", payload_len);

    dat = (ssv_type_u8 *)p + p->hdr_offset;
    LOG_PRINTF("========== hdr     ==========\n\r");
    LOG_PRINTF("frame ctl     : 0x%04x\n\r", (((ssv_type_u16)dat[1] << 8)|dat[0]));
    LOG_PRINTF("  - more_frag : %d\n\r", GET_HDR80211_FC_MOREFRAG(p));

    a = (ssv_type_u8*)p + p->hdr_offset +  4;
    LOG_PRINTF("address 1     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[0], a[1], a[2], a[3], a[4], a[5]);
    LOG_PRINTF("address 2     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[6], a[7], a[8], a[9], a[10], a[11]);
    LOG_PRINTF("address 3     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[12], a[13], a[14], a[15], a[16], a[17]);

    LOG_PRINTF("seq ctl       : 0x%04x\n\r", (((ssv_type_u16)dat[23] << 8)|dat[22]));
    LOG_PRINTF("  - seq num   : %d\n\r", GET_HDR80211_SC_SEQNUM(p));
    LOG_PRINTF("  - frag num  : %d\n\r", GET_HDR80211_SC_FRAGNUM(p));

    return 0;
}

int ssv6006_hal_get_rxpkt_size(void)
{
    return sizeof(SSV6006_HOST_RXPKT);
}

int ssv6006_hal_get_rxpkt_ctype(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->c_type;
}

int ssv6006_hal_get_rxpkt_len(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->len;
}


int ssv6006_hal_get_rxpkt_channel(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    //Eric ?? Get channel info for smart config
    return _p->channel;
}

int ssv6006_hal_get_rxpkt_rcpi(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    //Eric
    return _p->phy_rssi;
}

int ssv6006_hal_set_rxpkt_rcpi(void *p, ssv_type_u32 RCPI)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    //Eric
    _p->phy_rssi=RCPI;
    return 0;
}

int ssv6006_hal_get_rxpkt_qos(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->qos;
}

int ssv6006_hal_get_rxpkt_f80211(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->f80211;
}


int ssv6006_hal_get_rxpkt_wsid(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->wsid;
}

int ssv6006_hal_get_rxpkt_tid(void *p)
{
    #if 0
    return -1;
    #else
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->tid;
    #endif
}

int ssv6006_hal_get_rxpkt_seqnum(void *p)
{
    #if 0
    return -1;
    #else
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->seq_num;
    #endif
}



int ssv6006_hal_get_rxpkt_psm(void *p)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)p;
    return _p->psm;
}

ssv_type_u8* ssv6006_hal_get_rxpkt_qos_ptr(void *_rxpkt)
{
    SSV6006_HOST_RXPKT *rxpkt=(SSV6006_HOST_RXPKT *)_rxpkt;
    ssv_type_u16 nOffset = 0;
    switch (rxpkt->c_type)
    {
        case M0_RXEVENT:
        #if(SW_8023TO80211==1)
            nOffset += rxpkt->hdr_offset;
        #else
            nOffset += sizeof(SSV6006_HOST_RXPKT);
        #endif
            break;
        default:
            SSV_ASSERT(FALSE);
            break;
    }
    if (rxpkt->use_4addr)
        nOffset+=6;

    if (rxpkt->ht)
        nOffset+=4;

    if (!rxpkt->qos)
        return NULL;

    return (((ssv_type_u8*)rxpkt)+nOffset);
}

ssv_type_u8 *ssv6006_hal_get_rxpkt_data_ptr(void *_rxpkt)
{
    ssv_type_u16 offset=0;
    SSV6006_HOST_RXPKT *rxpkt=(SSV6006_HOST_RXPKT *)_rxpkt;

    if(rxpkt->f80211==0)
    {
        do{
            switch(rxpkt->c_type)
            {
            case M0_RXEVENT:
                #if(SW_8023TO80211==1)
                offset = rxpkt->hdr_offset;
                #else
                offset = RX_M0_HDR_LEN;
                #endif
                break;
            case M2_RXEVENT:
                SSV_ASSERT(FALSE);
                offset = M2_HDR_LEN;
                break;

            default:
                break;
            }

            //mac80211 no need to put extra header.
            if(rxpkt->f80211)
                break;

            /*|(AL)|MAC|QOS|HT|*/
            #if 0
            if(rxpkt->ht == 1)
                offset +=IEEE80211_HT_CTL_LEN;
            #endif
            if(rxpkt->qos == 1){
                offset +=IEEE80211_QOS_CTL_LEN;
            }
            #if 0
            if(rxpkt->use_4addr == 1)
                offset +=ETHER_ADDR_LEN;
            #endif

            #if(SW_8023TO80211==0)
                offset +=2;
            #endif
        }while(0);
        return (ssv_type_u8*)(((ssv_type_u8*)rxpkt)+offset);
    }
    else
    {
        //LOG_PRINTF("\33[35m %s():%d \33[0m\r\n",__func__,__LINE__);
        return (ssv_type_u8*)(((ssv_type_u8*)rxpkt)+SSV6006_RXINFO_SIZE);
    }
}

int ssv6006_hal_get_rxpkt_data_len(void *rxpkt)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)rxpkt;
    if(_p->f80211==0)
    {
        #if(SW_8023TO80211==1)
        return _p->len-SSV6006_RXINFO_SIZE;
        #else
        ssv_type_u8 *raw = ssv6006_hal_get_rxpkt_data_ptr(_p);
        return _p->len - (ssv_type_u32)((ssv_type_u32)raw-(ssv_type_u32)_p);
        #endif
    }
    else
    {
        return _p->len - SSV6006_RXINFO_SIZE;
    }
}

ssv_type_u8 ssv6006_hal_get_rxpkt_bssid_idx(void *rxpkt)
{
    SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)rxpkt;
    return _p->bssid;
}

static ssv_type_bool ssv6xxx_aggr_frame_is_last(unsigned char *data, int offset, int length)
{
    struct hci_rx_aggr_info *rx_aggr_info = (struct hci_rx_aggr_info *)data;
    struct hci_rx_aggr_info *next_rx_aggr_info;

    if ((rx_aggr_info->jmp_mpdu_len + offset) == (length - 1))
        return TRUE;

    next_rx_aggr_info = (struct hci_rx_aggr_info *)(data + rx_aggr_info->jmp_mpdu_len);
    if ((next_rx_aggr_info->jmp_mpdu_len == 0) || 
        (next_rx_aggr_info->jmp_mpdu_len > (length - (offset + rx_aggr_info->jmp_mpdu_len))))
        return TRUE;

    return FALSE;
}

static void* _get_new_empty_frame(void)
{
    void * rFrame = NULL;
#if(SW_8023TO80211==1)
        while ((rFrame = (ssv_type_u8 *)os_frame_alloc((g_host_cfg.recv_buf_size-g_host_cfg.trx_hdr_len),TRUE)) == NULL)
#else
        while ((rFrame = (ssv_type_u8 *)os_frame_alloc(g_host_cfg.recv_buf_size,TRUE)) == NULL)
#endif
        {
            OS_TickDelay(1);
            LOG_DEBUGF(LOG_TXRX|LOG_LEVEL_WARNING, ("[rx_aggr]: wakeup from sleep!\r\n"));
            //continue;
        }
        os_frame_push(rFrame, g_host_cfg.trx_hdr_len);
        return rFrame;
}
ssv_type_u8 aggcnt_log[20];
ssv_type_u8 cnt_idx=0;
ssv_type_u32 ssv6006_hal_process_hci_rx_aggr(void* pdata, ssv_type_u32 data_length, RxPktHdr cbk_fh)
{
    ssv_type_bool last_packet = FALSE;
    ssv_type_u32 data_offset=0;
    struct hci_rx_aggr_info *rx_aggr_info;
    void * rFrame = NULL;
    ssv_type_u8  *msg_data = NULL, *aggrdata=(ssv_type_u8*)pdata;
    SSV6006_HOST_RXPKT *_p=NULL;
    ssv_type_u32 next_aggr_len=0,next_aggr_len2=0;
    ssv_type_u8 rxaggrcnt=0;
    //LOG_PRINTF("data_length=%d\r\n",data_length);
    for (data_offset = 0; data_offset < data_length; ) {

        last_packet = ssv6xxx_aggr_frame_is_last(aggrdata, data_offset, data_length);
        if (last_packet) {
            
            //LOG_PRINTF("last_packet\r\n");
            rx_aggr_info = (struct hci_rx_aggr_info *)aggrdata;
            if (((data_offset + sizeof(struct hci_rx_aggr_info)) > data_length) ||
                ((data_offset + rx_aggr_info->jmp_mpdu_len) > data_length)) {
                //LOG_PRINTF("[hci_aggr_2]%d,%d,%d\r\n",rx_aggr_info->jmp_mpdu_len,data_offset,data_length);
                break;    
            }
            rFrame = _get_new_empty_frame();
            msg_data = OS_FRAME_GET_DATA(rFrame);
            //remove rx aggregation header
            OS_MemCPY(msg_data,(void*)(aggrdata+sizeof(struct hci_rx_aggr_info)),(rx_aggr_info->jmp_mpdu_len-sizeof(struct hci_rx_aggr_info)));
            data_offset = data_length;

            //Peek next lenght
            if (rx_aggr_info->accu_rx_len <= (MAX_HCI_AGGR_SIZE + MAX_RX_PKT_RSVD)) {
                next_aggr_len = rx_aggr_info->accu_rx_len;
                next_aggr_len2 = rx_aggr_info->accu_rx_len-rx_aggr_info->jmp_mpdu_len;
            } else {
                //LOG_PRINTF("%s: accu_rx_len %u is too big!\r\n", __func__, rx_aggr_info->accu_rx_len);
                next_aggr_len = 0;
                next_aggr_len2 = 0;
            }
        } else { 
        
            rx_aggr_info = (struct hci_rx_aggr_info *)aggrdata;

            if ((rx_aggr_info->jmp_mpdu_len == 0) ||
                (rx_aggr_info->jmp_mpdu_len > MAX_FRAME_SIZE) ||
                ((data_offset + sizeof(struct hci_rx_aggr_info)) > data_length) ||
                ((data_offset + rx_aggr_info->jmp_mpdu_len) > data_length)) {
                
                LOG_PRINTF("[hci_aggr_1]%d,%d,%d\r\n",rx_aggr_info->jmp_mpdu_len,data_offset,data_length);
                break;
            }
            rFrame = _get_new_empty_frame();
            msg_data = OS_FRAME_GET_DATA(rFrame);

            OS_MemCPY(msg_data, (void*)(aggrdata+sizeof(struct hci_rx_aggr_info)),
                    (rx_aggr_info->jmp_mpdu_len - sizeof(struct hci_rx_aggr_info)));
            
            aggrdata += rx_aggr_info->jmp_mpdu_len;
            data_offset += rx_aggr_info->jmp_mpdu_len;
        }
        _p=(SSV6006_HOST_RXPKT *)msg_data;
        OS_FRAME_SET_DATA_LEN(rFrame, _p->len);
        
        //LOG_PRINTF("deagg_len=%d\r\n",_p->len);
        //hex_dump(rFrame,(_p->len/4+1));
        cbk_fh(rFrame);
        rxaggrcnt++;
    }
#if 0
    if(cnt_idx > 20)
    {
        //int i;
        LOG_PRINTF("\r\n");
        for(i=0;i<cnt_idx;i++)
            LOG_PRINTF("c=%d ",aggcnt_log[i]);
        LOG_PRINTF("\r\n");
        cnt_idx=0;
    }
    aggcnt_log[cnt_idx] = rxaggrcnt;
    cnt_idx++;
#else
    //LOG_PRINTF("c=%d,L1=%d,L2=%d\r\n\r\n",rxaggrcnt,next_aggr_len,next_aggr_len2);
#endif
    return next_aggr_len2;
}

struct tx_aggr_extra
{
    ssv_type_u32 aggr_n:8;
    ssv_type_u32 totol_len:16;
    ssv_type_u32 RSVD:8;
};
ssv_type_u32 ssv6006_hal_process_hci_aggr_tx(void* tFrame, void* aggr_buf, ssv_type_u32* aggr_len)
{
    ssv_type_u32 align_size = (((OS_FRAME_GET_DATA_LEN(tFrame)) + 4 - 1) & ~(4 - 1));
    ssv_type_u8* start_addr = (ssv_type_u8*)aggr_buf;
    struct hci_tx_aggr_info* aggr_tx_info = (struct hci_tx_aggr_info*)(aggr_buf);
    struct tx_aggr_extra* extra_info=(struct tx_aggr_extra*)&aggr_tx_info->un.extra_info;

    *aggr_len=0;
    if(extra_info->totol_len)
    {
        start_addr += extra_info->totol_len;  
        aggr_tx_info = (struct hci_tx_aggr_info*)(start_addr);
    }
 
    aggr_tx_info->jmp_len0 = aggr_tx_info->jmp_len1 = align_size+sizeof(struct hci_tx_aggr_info);
    OS_MemCPY((void*)(start_addr+sizeof(struct hci_tx_aggr_info)),OS_FRAME_GET_DATA(tFrame),align_size);
    extra_info->totol_len += aggr_tx_info->jmp_len0;

    *aggr_len = extra_info->totol_len+4;
    extra_info->aggr_n++;
    start_addr += aggr_tx_info->jmp_len0;//(align_size+sizeof(struct hci_tx_aggr_info));
    aggr_tx_info = (struct hci_tx_aggr_info*)(start_addr);
    aggr_tx_info->jmp_len0 = aggr_tx_info->jmp_len1 = 0;
     
    //LOG_PRINTF("align size=%d,%d,aggr_n=%d,aggr_buf=0x%x,aggr_len=%d\r\n",align_size,OS_FRAME_GET_DATA_LEN(tFrame),extra_info->aggr_n,(u32)aggr_buf,*aggr_len);
    return extra_info->aggr_n;
}

int ssv6006_hal_hci_aggr_en(HCI_AGGR_HW trx, ssv_type_bool en)
{
    ssv_type_u32 regval,i=0;
    int ret;
    if(trx == SSV_HCI_RX)//RX
    {        
        regval = 0;
        if(en)
        {
            /* 
                  * HCI RX Aggregation
                  * FLOW CONTROL
                  * 1. STOP HCI RX
                  * 2. CONFIRM RX STATUS
                  * 3. ENABLE HCI RX AGGREGATION
                  * 4. SET AGGREGATION LENGTH[LENGTH=AGGREGATION+MPDU]
                  * 5. START HCI RX
                  */
             //LOG_PRINTF("rxaggr en\r\n");
             MAC_REG_SET_BITS(ADR_HCI_TRX_MODE, (0<<HCI_RX_EN_SFT), ~HCI_RX_EN_MSK);
             do {
                 MAC_REG_READ(ADR_RX_PACKET_LENGTH_STATUS, regval);
                 regval &= HCI_RX_LEN_I_MSK;
                 i++;
                 if (i > 10000) {
                     LOG_PRINTF("CANNOT ENABLE HCI RX AGGREGATION!!!\r\n");
                     //WARN_ON(1);
                     ret = FALSE;
                     return ret;
                 }
             } while (regval != 0);

            MAC_REG_SET_BITS(ADR_HCI_TRX_MODE, (1<<HCI_RX_FORM_1_SFT), ~HCI_RX_FORM_1_MSK);
            regval = (3 << RX_AGG_CNT_SFT) |
                     (0 << RX_AGG_METHOD_3_SFT) |
                     (20 << RX_AGG_TIMER_RELOAD_VALUE_SFT);
            MAC_REG_WRITE(ADR_FORCE_RX_AGGREGATION_MODE, regval);
            MAC_REG_SET_BITS(ADR_HCI_FORCE_PRE_BULK_IN, HCI_AGGR_SIZE, ~HCI_BULK_IN_HOST_SIZE_MSK);    
        }
        else
        {
            //LOG_PRINTF("rxaggr dis\r\n");
            MAC_REG_SET_BITS(ADR_HCI_TRX_MODE, (0<<HCI_RX_FORM_1_SFT), ~HCI_RX_FORM_1_MSK);
            //MAC_REG_WRITE(ADR_FORCE_RX_AGGREGATION_MODE, 0);
            MAC_REG_SET_BITS(ADR_HCI_FORCE_PRE_BULK_IN, 0, ~HCI_BULK_IN_HOST_SIZE_MSK);            
        }
        MAC_REG_SET_BITS(ADR_HCI_TRX_MODE, (1<<HCI_RX_EN_SFT), ~HCI_RX_EN_MSK);
    }
    else if(trx == SSV_HCI_TX)
    {
        //if(en)
        {
            MAC_REG_SET_BITS(ADR_HCI_TRX_MODE, (en<<HCI_TX_AGG_EN_SFT), ~HCI_TX_AGG_EN_MSK);
        }
    }
    return TRUE;

}

#endif //#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))

