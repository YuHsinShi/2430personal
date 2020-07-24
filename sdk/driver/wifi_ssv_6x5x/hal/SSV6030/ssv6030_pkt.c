#define SSV6030_PKT_C

#include <host_config.h>
#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <config.h>
#include <log.h>
#include <hdr80211.h>
#include <ssv_dev.h>
#include <ap_info.h>
#include <dev.h>
#include "ssv6030_hal.h"
#include "../ssv_hal_if.h"
#include "ssv6030_pktdef.h"
#define SSV6051_HAL_CFG_HOST_TXREQ0(host_txreq0,len,c_type,f80211,qos,ht,use_4addr,RSVD0,padding,bc_queue,security,more_data,sub_type,extra_info) \
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


int ssv6030_hal_dump_txinfo(void *_p)
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
    LOG_PRINTF("%20s : %d\n\r", "RSVD_0",p->RSVD_0);
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
    LOG_PRINTF("%20s : %d\n\r", "tx_report",p->tx_report);
    LOG_PRINTF("%20s : %d\n\r", "tx_burst",p->tx_burst);
    LOG_PRINTF("%20s : %d\n\r", "ack_policy",p->ack_policy);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_1",p->RSVD_1);
    LOG_PRINTF("%20s : %d\n\r", "do_rts_cts",p->do_rts_cts);
    LOG_PRINTF("%20s : %d\n\r", "reason",p->reason);
    LOG_PRINTF("%20s : %d\n\r", "payload_offset",p->payload_offset);
    LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",p->next_frag_pid);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_2",p->RSVD_2);
    LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",p->fCmdIdx);
    LOG_PRINTF("%20s : %d\n\r", "wsid",p->wsid);
    LOG_PRINTF("%20s : %d\n\r", "txq_idx",p->txq_idx);
    LOG_PRINTF("%20s : %d\n\r", "TxF_ID",p->TxF_ID);
    LOG_PRINTF("%20s : %d\n\r", "rts_cts_nav",p->rts_cts_nav);
    LOG_PRINTF("%20s : %d\n\r", "frame_consume_time",p->frame_consume_time);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_3",p->RSVD_3);
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

int ssv6030_hal_get_valid_txinfo_size(void)
{
    return sizeof(SSV6051_HOST_TXREQ0);	
}

int ssv6030_hal_get_txreq0_size(void)
{
    return sizeof(SSV6051_HOST_TXREQ0);
}

int ssv6030_hal_get_txreq0_ctype(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->c_type;
}

int ssv6030_hal_set_txreq0_ctype(void *p,ssv_type_u8 c_type)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->c_type=c_type;
    return 0;
}

int ssv6030_hal_get_txreq0_len(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->len;
}

int ssv6030_hal_set_txreq0_len(void *p,ssv_type_u32 len)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->len=len;
    return 0;
}

int ssv6030_hal_get_txreq0_rsvd0(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->RSVD0;
}

int ssv6030_hal_set_txreq0_rsvd0(void *p,ssv_type_u32 val)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->RSVD0=val;
    return 0;
}
int ssv6030_hal_get_txreq0_padding(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->padding;
}

int ssv6030_hal_set_txreq0_padding(void *p, ssv_type_u32 val)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->padding=val;
    return 0;
}

int ssv6030_hal_get_txreq0_qos(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->qos;
}

int ssv6030_hal_get_txreq0_ht(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->ht;
}

int ssv6030_hal_get_txreq0_4addr(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->use_4addr;
}

int ssv6030_hal_set_txreq0_f80211(void *p,ssv_type_u8 f80211)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->f80211=f80211;
    return 0;
}

int ssv6030_hal_get_txreq0_f80211(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->f80211;
}

int ssv6030_hal_get_txreq0_more_data(void *p)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    return _p->more_data;
}

int ssv6030_hal_set_txreq0_more_data(void *p,ssv_type_u8 more_data)
{
    SSV6051_HOST_TXREQ0 *_p=(SSV6051_HOST_TXREQ0 *)p;
    _p->more_data=more_data;
    return 0;
}

ssv_type_u8* ssv6030_hal_get_txreq0_qos_ptr(void *_req0)
{
    SSV6051_HOST_TXREQ0 *req0=(SSV6051_HOST_TXREQ0 *)_req0;
    ssv_type_u16 nOffset = 0;
    switch (req0->c_type)
    {
        case SSV6XXX_TX_REQ0:
            nOffset += sizeof(SSV6051_HOST_TXREQ0);
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

ssv_type_u8* ssv6030_hal_get_txreq0_data_ptr(void *_req0)
{
    SSV6051_HOST_TXREQ0 *req0=(SSV6051_HOST_TXREQ0 *)_req0;
    ssv_type_u16 nOffset = 0;
    switch (req0->c_type)
    {
        case SSV6XXX_TX_REQ0:
        	nOffset += sizeof(SSV6051_HOST_TXREQ0);
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

void * ssv6030_hal_fill_txreq0(void *frame, ssv_type_u32 len, ssv_type_u32 priority,
                                       ssv_type_u16 *qos, ssv_type_u32 *ht, ssv_type_u8 *addr4,
                                       ssv_type_bool f80211, ssv_type_u8 security, ssv_type_u8 tx_dscrp_flag,ssv_type_u8 vif_idx)
{
    SSV6051_HOST_TXREQ0 *host_txreq0=NULL;
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

    extra_len += ssv6030_hal_get_txreq0_size();
    os_frame_push(frame,extra_len);

    padding_len=(((ssv_type_u32)OS_FRAME_GET_DATA(frame))&(0x3));
    if(0!=padding_len){
        padding_len=4-padding_len;
        os_frame_push(frame,padding_len);
    }

    host_txreq0 = (SSV6051_HOST_TXREQ0 *)OS_FRAME_GET_DATA(frame);
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

    SSV6051_HAL_CFG_HOST_TXREQ0(host_txreq0,
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

    return frame;
}

int ssv6030_hal_dump_rxinfo(void *_p)
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
    LOG_PRINTF("%20s : %d\n\r", "l3cs_err",p->l3cs_err);
    LOG_PRINTF("%20s : %d\n\r", "l4cs_err",p->l4cs_err);
    LOG_PRINTF("%20s : %d\n\r", "use_4addr",p->use_4addr);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_0",p->RSVD_0);
    LOG_PRINTF("%20s : %d\n\r", "psm",p->psm);
    LOG_PRINTF("%20s : %d\n\r", "stype_b5b4",p->stype_b5b4);
    LOG_PRINTF("%20s : %d\n\r", "extra_info",p->extra_info);
    LOG_PRINTF("%20s : 0x%08x\n\r", "fCmd",p->fCmd);
    LOG_PRINTF("%20s : %d\n\r", "hdr_offset",p->hdr_offset);
    LOG_PRINTF("%20s : %d\n\r", "frag",p->frag);
    LOG_PRINTF("%20s : %d\n\r", "unicast",p->unicast);
    LOG_PRINTF("%20s : %d\n\r", "hdr_len",p->hdr_len);
    LOG_PRINTF("%20s : 0x%x\n\r", "RxResult",p->RxResult);
    LOG_PRINTF("%20s : %d\n\r", "wildcard_bssid",p->wildcard_bssid);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_1",p->RSVD_1);
    LOG_PRINTF("%20s : %d\n\r", "reason",p->reason);
    LOG_PRINTF("%20s : %d\n\r", "payload_offset",p->payload_offset);
    LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",p->next_frag_pid);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_2",p->RSVD_2);
    LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",p->fCmdIdx);
    LOG_PRINTF("%20s : %d\n\r", "wsid",p->wsid);
    LOG_PRINTF("%20s : %d\n\r", "RSVD_3",p->RSVD_3);
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

int ssv6030_hal_get_rxpkt_size(void)
{
    return sizeof(SSV6051_HOST_RXPKT);
}

int ssv6030_hal_get_rxpkt_ctype(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->c_type;
}

int ssv6030_hal_get_rxpkt_len(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->len;
}
int ssv6030_hal_get_rxpkt_seqnum(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->seq_num;
}
int ssv6030_hal_get_rxpkt_tid(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->tid;
}
int ssv6030_hal_get_rxpkt_rsvd(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->RSVD;
}

int ssv6030_hal_get_rxpkt_rcpi(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->RCPI;
}

int ssv6030_hal_set_rxpkt_rcpi(void *p, ssv_type_u32 RCPI)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    _p->RCPI=RCPI;
    return 0;
}

int ssv6030_hal_get_rxpkt_qos(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->qos;
}

int ssv6030_hal_get_rxpkt_f80211(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->f80211;
}
int ssv6030_hal_get_rxpkt_wsid(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->wsid;
}

int ssv6030_hal_get_rxpkt_psm(void *p)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)p;
    return _p->psm;
}

ssv_type_u8* ssv6030_hal_get_rxpkt_qos_ptr(void *_rxpkt)
{
    SSV6051_HOST_RXPKT *rxpkt=(SSV6051_HOST_RXPKT *)_rxpkt;
    ssv_type_u16 nOffset = 0;
    switch (rxpkt->c_type)
    {
        case M0_RXEVENT:
            nOffset += sizeof(SSV6051_HOST_RXPKT);
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

ssv_type_u8 *ssv6030_hal_get_rxpkt_data_ptr(void *_rxpkt)
{
    ssv_type_u16 offset=0;
    SSV6051_HOST_RXPKT *rxpkt=(SSV6051_HOST_RXPKT *)_rxpkt;
    if(rxpkt->f80211==0)
    {
        do{
            switch(rxpkt->c_type)
            {
            case M0_RXEVENT:
                offset = RX_M0_HDR_LEN;
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
            if(rxpkt->ht == 1)
                offset +=IEEE80211_HT_CTL_LEN;

            if(rxpkt->qos == 1)
                offset +=IEEE80211_QOS_CTL_LEN;

            if(rxpkt->use_4addr == 1)
                offset +=ETHER_ADDR_LEN;

            if(rxpkt->align2 == 1)
                offset +=2;
        }while(0);

        return (ssv_type_u8*)(((ssv_type_u8*)rxpkt)+offset);
    }
    else
    {
        return (ssv_type_u8*)(((ssv_type_u8*)rxpkt)+SSV6051_RXINFO_SIZE);
    }
}

int ssv6030_hal_get_rxpkt_data_len(void *rxpkt)
{
    SSV6051_HOST_RXPKT *_p=(SSV6051_HOST_RXPKT *)rxpkt;
    if(_p->f80211==0)
    {
        ssv_type_u8 *raw = ssv6030_hal_get_rxpkt_data_ptr(_p);
        return _p->len - (ssv_type_u32)(raw-(ssv_type_u8*)_p);
    }
    else
    {
        return _p->len - SSV6051_RXINFO_SIZE;
    }
}

ssv_type_u8 ssv6030_hal_get_rxpkt_bssid_idx(void *rxpkt)
{
    //SSV6006_HOST_RXPKT *_p=(SSV6006_HOST_RXPKT *)rxpkt;
    return 0;
}

#endif //#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
