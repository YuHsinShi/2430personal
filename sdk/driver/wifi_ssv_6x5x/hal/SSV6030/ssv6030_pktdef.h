#ifndef _SSV6030_PKTDEF_H_
#define _SSV6030_PKTDEF_H_

#include <ssv_types.h>

#define M0_HDR_LEN							4
#define M1_HDR_LEN							8
#define M2_HDR_LEN							16

#define RX_M0_HDR_LEN						24

typedef enum __PBuf_Type_E {
    NOTYPE_BUF  = 0,
    TX_BUF      = 1,
    RX_BUF      = 2
} PBuf_Type_E;


/**
 *
 *  Offset Table (register):
 *
 *    c_type            hdr_len
 *  ----------     ----------
 *  M0_TXREQ         8-bytes
 *  M1_TXREQ         12-bytes
 *  M2_TXREQ         sizeof(PKT_TXInfo)
 *  M0_RXEVENT
 *  M1_RXEVENT
 *  HOST_CMD
 *  HOSt_EVENT
 *
 *
 */
#define M0_TXREQ                            0
#define M1_TXREQ                            1
#define M2_TXREQ                            2
#define M0_RXEVENT                          3
#define M2_RXEVENT                          4
#define HOST_CMD                            5
#define HOST_EVENT                          6

#define IS_RX_PKT(_p)       ((_p)->c_type==M0_RXEVENT)
#define IS_TX_PKT(_p)       (/*((_p)->c_type>=M0_TXREQ)&&*/((_p)->c_type<=M2_TXREQ))
#define IS_TX_TEST_CMD(_p)	((_p)->c_type==TEST_CMD)


/*	WMM_Specification_1-1 : Table 14  802.1D Priority to AC mappings

	UP		Access Category
	-------------------------
	1, 2	AC_BK
	0, 3	AC_BE
	4, 5	AC_VI
	6, 7	AC_VO
*/
#define	AC_BK	 0
#define	AC_BE	 1
#define	AC_VI	 2
#define	AC_VO	 3


#define	PBUF_HDR80211(p, i)				(*((ssv_type_u8 *)(p)+(p)->hdr_offset + (i)))

#define	GET_SC_SEQNUM(sc)				(((sc) & 0xfff0) >> 4)
#define	GET_SC_FRAGNUM(sc)				(((sc) & 0x000f)     )

#define GET_QC_TID(qc)					((qc) & 0x000f)
#define	GET_QC_UP(qc)					((qc) & 0x0007)
#define GET_QC_AC(qc) 					((GET_QC_UP(qc) == 0) ? AC_BE : \
										 (GET_QC_UP(qc) == 1) ? AC_BK : \
										 (GET_QC_UP(qc) == 2) ? AC_BK : \
										 (GET_QC_UP(qc) == 3) ? AC_BE : \
										 (GET_QC_UP(qc) == 4) ? AC_VI : \
										 (GET_QC_UP(qc) == 5) ? AC_VI : \
										 (GET_QC_UP(qc) == 6) ? AC_VO : AC_VO)


#define GET_HDR80211_FC(p)				(((p)->f80211==1) ? (((ssv_type_u16)PBUF_HDR80211(p, 1) << 8) | PBUF_HDR80211(p, 0)) : 0)
#define GET_HDR80211_FC_TYPE(p)			((GET_HDR80211_FC(p) & 0x0c) >> 2)
#define GET_HDR80211_FC_TYPE_STR(t)		((t == 0) ? "Mgmt" : ((t == 1) ? "Control" : ((t == 2) ? "Data" : "Reserved")))
#define	GET_HDR80211_FC_VER(p)			((GET_HDR80211_FC(p) & M_FC_VER))
#define GET_HDR80211_FC_TODS(p)			((GET_HDR80211_FC(p) & M_FC_TODS)      >>  8)
#define GET_HDR80211_FC_FROMDS(p)		((GET_HDR80211_FC(p) & M_FC_FROMDS)    >>  9)
#define GET_HDR80211_FC_MOREFRAG(p)		((GET_HDR80211_FC(p) & M_FC_MOREFRAGS) >> 10)
#define	GET_HDR80211_FC_RETRY(p)	    ((GET_HDR80211_FC(p) & M_FC_RETRY)     >> 11)
#define	GET_HDR80211_FC_PWRMGNT(p)		((GET_HDR80211_FC(p) & M_FC_PWRMGMT)   >> 12)
#define	GET_HDR80211_FC_MOREDATA(p)		((GET_HDR80211_FC(p) & M_FC_MOREDATA)  >> 13)
#define	GET_HDR80211_FC_PROTECTED(p)	((GET_HDR80211_FC(p) & M_FC_PROTECTED) >> 14)
#define	GET_HDR80211_FC_ORDER(p)		((GET_HDR80211_FC(p) & M_FC_ORDER)     >> 15)

#define SET_HDR80211_FC_MOREFRAG(p)		(PBUF_HDR80211(p, 1) |= 0x04)
#define UNSET_HDR80211_FC_MOREFRAG(p)	(PBUF_HDR80211(p, 1) &= 0xfb)

#define GET_HDR80211_SC(p)				((ssv_type_u16)PBUF_HDR80211(p, 23) << 8 | (PBUF_HDR80211(p, 22)))
#define GET_HDR80211_SC_SEQNUM(p)		((GET_HDR80211_SC(p) & 0xfff0) >> 4)
#define GET_HDR80211_SC_FRAGNUM(p)		((GET_HDR80211_SC(p) & 0x000f))

//
//	Function			ToDS	FromDS	Addr1	Addr2	Addr3	Addr4
//	-------------------------------------------------------------------------
//	IBSS				0		0		DA		SA		BSSID	Not_Used
//	To AP (infra.)		1		0		BSSID	SA		DA		Not_Used
//	From AP (infra.)	0		1		DA		BSSID	SA		Not_Used
//	WDS (bridge)		1		1		RA		TA		DA		SA
#define HAS_HDR80211_ADDRESS_4(p)		(GET_HDR80211_FC_TODS(p) & GET_HDR80211_FC_FROMDS(p))

// QoS Control Field
#define	GET_HDR80211_QC(p)				(((p)->qos == 1) ? (((ssv_type_u16)PBUF_HDR80211(p, 25 + (HAS_HDR80211_ADDRESS_4(p)*6)) << 8) | PBUF_HDR80211(p, 24 + (HAS_HDR80211_ADDRESS_4(p)*6))) : 0)
#define GET_HDR80211_ADDRESS_1(a, p)	memcpy((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset +  4), 6)
#define GET_HDR80211_ADDRESS_2(a, p)	memcpy((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset + 10), 6)
#define GET_HDR80211_ADDRESS_3(a, p)	memcpy((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset + 16), 6)


#define SSV62XX_TX_MAX_RATES    3

STRUCT_PACKED struct fw_rc_retry_params {
    ssv_type_u32 count:4;
    ssv_type_u32 drate:6;
    ssv_type_u32 crate:6;
    ssv_type_u32 rts_cts_nav:16;
    ssv_type_u32 frame_consume_time:10;
    ssv_type_u32 dl_length:12;
    ssv_type_u32 RSVD:10;
};

/**
* struct ssv6200_tx_desc - ssv6200 tx frame descriptor.
* This descriptor is shared with ssv6200 hardware and driver.
*/
struct ssv6200_tx_desc
{
    /* The definition of WORD_1: */
    ssv_type_u32             len:16;
    ssv_type_u32             c_type:3;
    ssv_type_u32             f80211:1;
    ssv_type_u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    ssv_type_u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    ssv_type_u32             use_4addr:1;
    ssv_type_u32             RSVD_0:3;//used for rate control report event.
    ssv_type_u32             bc_que:1;
    ssv_type_u32             security:1;
    ssv_type_u32             more_data:1;
    ssv_type_u32             stype_b5b4:2;
    ssv_type_u32             extra_info:1;   /* 0: don't trap to cpu after parsing, 1: trap to cpu after parsing */

    /* The definition of WORD_2: */
    ssv_type_u32             fCmd;

    /* The definition of WORD_3: */
    ssv_type_u32             hdr_offset:8;
    ssv_type_u32             frag:1;
    ssv_type_u32             unicast:1;
    ssv_type_u32             hdr_len:6;
    ssv_type_u32             tx_report:1;
    ssv_type_u32             tx_burst:1;     /* 0: normal, 1: burst tx */
    ssv_type_u32             ack_policy:2;   /* See Table 8-6, IEEE 802.11 Spec. 2012 */
    ssv_type_u32             aggregation:1;
    ssv_type_u32             RSVD_1:3;//Used for AMPDU retry counter
    ssv_type_u32             do_rts_cts:2;   /* 0: no RTS/CTS, 1: need RTS/CTS */
                                    /* 2: CTS protection, 3: RSVD */
    ssv_type_u32             reason:6;

    /* The definition of WORD_4: */
    ssv_type_u32             payload_offset:8;
    ssv_type_u32             RSVD_4:7;
    ssv_type_u32             RSVD_2:1;
    ssv_type_u32             fCmdIdx:3;
    ssv_type_u32             wsid:4;
    ssv_type_u32             txq_idx:3;
    ssv_type_u32             TxF_ID:6;

    /* The definition of WORD_5: */
    ssv_type_u32             rts_cts_nav:16;
    ssv_type_u32             frame_consume_time:10;  //32 units
    ssv_type_u32             crate_idx:6;

    /* The definition of WORD_6: */
    ssv_type_u32             drate_idx:6;
    ssv_type_u32             dl_length:12;
    ssv_type_u32             RSVD_3:14;
    /* The definition of WORD_7~15: */
    ssv_type_u32             RESERVED[8];
    /* The definition of WORD_16~20: */
    struct fw_rc_retry_params rc_params[SSV62XX_TX_MAX_RATES];
};


/**
 * struct cfg_host_txreq0 - Host frame transmission Request  Header
 *
 * TX-REQ0 uses 4-byte header to carry host message to wifi-controller.
 * The first two-byte is the length indicating the whole message length (
 * including 2-byte header length).
 */
 struct cfg_host_txreq0 {
    ssv_type_u32               len:16;
    ssv_type_u32               c_type:3;
    ssv_type_u32               f80211:1;
    ssv_type_u32               qos:1;
    ssv_type_u32               ht:1;
    ssv_type_u32               use_4addr:1;
    ssv_type_u32               RSVD0:1;			//AP mode use one bit to know if this packet is buffered frame(Power saving) AP_PS_FRAME
    ssv_type_u32               padding:2;
    ssv_type_u32				  bc_queue:1;
    ssv_type_u32               security:1;
    ssv_type_u32               more_data:1;
    ssv_type_u32               sub_type:2;
    ssv_type_u32               extra_info:1;
 }STRUCT_PACKED;

#if 0
/**
 *  struct cfg_host_txreq1 - Host frame transmission Request  Header
 */
struct cfg_host_txreq1 {
    u32               len:16;
    u32               c_type:3;
    u32               f80211:1;
    u32               qos:1;
    u32               ht:1;
    u32               use_4addr:1;
    u32               RSVD0:4;
	u32               security:1;
    u32               more_data:1;
    u32               sub_type:2;
    u32               extra_info:1;

    u32               f_cmd;
} STRUCT_PACKED;


/**
 *  struct cfg_host_txreq2 - Host frame transmission Request  Header
 */
struct cfg_host_txreq2 {
#pragma message("===================================================")
#pragma message("     cfg_host_txreq2 not implement yet")
#pragma message("===================================================")
    u32               len:16;
    u32               c_type:3;
    u32               f80211:1;
    u32               qos:1;
    u32               ht:1;
    u32               use_4addr:1;
    u32               RSVD0:4;
	u32               security:1;
    u32               more_data:1;
    u32               sub_type:2;
    u32               extra_info:1;

    u32               f_cmd;
    u32               AAA;
}; //__attribute__((packed))

 /**
 *  struct cfg_host_txreq - Host frame transmission Request Parameters
 */
struct cfg_host_txreq {
	struct cfg_host_txreq0 txreq0;
        u16           qos;
        u32           ht;
        u8            addr4[ETHER_ADDR_LEN];
	u8            priority;
}; //__attribute__((packed));


#endif

struct cfg_host_rxpkt {

	 /* The definition of WORD_1: */
	ssv_type_u32             len:16;
	ssv_type_u32             c_type:3;
    ssv_type_u32             f80211:1;
	ssv_type_u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    ssv_type_u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    ssv_type_u32             use_4addr:1;
	ssv_type_u32             l3cs_err:1;
    ssv_type_u32             l4cs_err:1;
    ssv_type_u32             align2:1;
    ssv_type_u32             RSVD_0:2;
	ssv_type_u32             psm:1;
    ssv_type_u32             stype_b5b4:2;
    ssv_type_u32             extra_info:1;

    /* The definition of WORD_2: */
    ssv_type_u32             fCmd;

    /* The definition of WORD_3: */
    ssv_type_u32             hdr_offset:8;
    ssv_type_u32             frag:1;
    ssv_type_u32             unicast:1;
    ssv_type_u32             hdr_len:6;
    ssv_type_u32             RxResult:8;
    ssv_type_u32             wildcard_bssid:1;
    ssv_type_u32             RSVD_1:1;
    ssv_type_u32             reason:6;

    /* The definition of WORD_4: */
    ssv_type_u32             payload_offset:8;
    ssv_type_u32             next_frag_pid:7;
    ssv_type_u32             RSVD_2:1;
    ssv_type_u32             fCmdIdx:3;
    ssv_type_u32             wsid:4;
	ssv_type_u32				RSVD_3:3;
	ssv_type_u32				drate_idx:6;

	/* The definition of WORD_5: */
	ssv_type_u32 			seq_num:16;
	ssv_type_u32				tid:16;

	/* The definition of WORD_6: */
	ssv_type_u32				pkt_type:8;
	ssv_type_u32				RCPI:8;
	ssv_type_u32				SNR:8;
	ssv_type_u32				RSVD:8;

};

typedef struct PKT_TxPhyInfo_st
{
	/* The definition of WORD_1: */
	ssv_type_u32 Llength:8;

	ssv_type_u32 Mlength:8;

	ssv_type_u32 RESV1:8;

	ssv_type_u32 RESV2:8;

	/* The definition of WORD_2: */
	ssv_type_u32 mode:3;
	ssv_type_u32 ch_bw:3;
	ssv_type_u32 preamble_option:1;
	ssv_type_u32 HTshortGI:1;

	ssv_type_u32 rate:7;
	ssv_type_u32 RESV3:1;

	ssv_type_u32 smoothing:1;
	ssv_type_u32 no_sounding:1;
	ssv_type_u32 aggregation:1;
	ssv_type_u32 stbc:2;
	ssv_type_u32 fec:1;
	ssv_type_u32 n_ess:2;

	ssv_type_u32 txpwrlvl:8;

	/* The definition of WORD_3: */
	ssv_type_u32 Ll_length:8;

	ssv_type_u32 Ml_length:4;
	ssv_type_u32 l_rate:3;
	ssv_type_u32 RESV4:1;

	ssv_type_u32 RESV5:16;

	/* The definition of WORD_4: */
	ssv_type_u32 RESV6:32;

	/* The definition of WORD_5: */
	ssv_type_u32 RESV7:16;

	ssv_type_u32 Lservice:8;

	ssv_type_u32 Mservice:8;

	/* The definition of WORD_6: */
	ssv_type_u32 RESV8:32;

	/* The definition of WORD_7: */
	ssv_type_u32 RESV9:32;
}PKT_TxPhyInfo, *PPKT_TxPhyInfo;

typedef struct PKT_RxPhyInfo_st
{
	/* The definition of WORD_1: */
	ssv_type_u32 Llength:8;

	ssv_type_u32 Mlength:8;

	ssv_type_u32 RESV1:8;

	ssv_type_u32 RESV2:8;

	/* The definition of WORD_2: */
	ssv_type_u32 mode:3;
	ssv_type_u32 ch_bw:3;
	ssv_type_u32 preamble_option:1;
	ssv_type_u32 HTshortGI:1;

	ssv_type_u32 rate:7;
	ssv_type_u32 RESV3:1;

	ssv_type_u32 smoothing:1;
	ssv_type_u32 no_sounding:1;
	ssv_type_u32 aggregation:1;
	ssv_type_u32 stbc:2;
	ssv_type_u32 fec:1;
	ssv_type_u32 n_ess:2;

	ssv_type_u32 RESV4:8;

	/* The definition of WORD_3: */
	ssv_type_u32 Ll_length:8;

	ssv_type_u32 Ml_length:4;
	ssv_type_u32 l_rate:3;
	ssv_type_u32 RESV5:1;

	ssv_type_u32 RESV6:16;

	/* The definition of WORD_4: */
	ssv_type_u32 RESV7:32;

	/* The definition of WORD_5: */
	ssv_type_u32 RESV8:16;

	ssv_type_u32 Lservice:8;

	ssv_type_u32 Mservice:8;

}PKT_RxPhyInfo, *PPKT_RxPhyInfo;

typedef struct PKT_TxInfo_st
{
    /* The definition of WORD_1: */
	ssv_type_u32             len:16;
	ssv_type_u32             c_type:3;
    ssv_type_u32             f80211:1;
    ssv_type_u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    ssv_type_u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    ssv_type_u32             use_4addr:1;
    ssv_type_u32             RSVD_0:3;
	ssv_type_u32             bc_que:1;
	ssv_type_u32				security:1;
    ssv_type_u32             more_data:1;
    ssv_type_u32             stype_b5b4:2;
    ssv_type_u32             extra_info:1;   /* 0: don't trap to cpu after parsing, 1: trap to cpu after parsing */

    /* The definition of WORD_2: */
    ssv_type_u32             fCmd;

    /* The definition of WORD_3: */
    ssv_type_u32             hdr_offset:8;
    ssv_type_u32             frag:1;
    ssv_type_u32             unicast:1;
    ssv_type_u32             hdr_len:6;
    ssv_type_u32             tx_report:1;
    ssv_type_u32             tx_burst:1;     /* 0: normal, 1: burst tx */
    ssv_type_u32             ack_policy:2;   /* 0: normal ack, 1: no_ack, 2: PSMP Ack(Reserved), 3:Block Ack. See Table 8-6, IEEE 802.11 Spec. 2012 p.490*/
	ssv_type_u32             aggregation:1;
    ssv_type_u32             RSVD_1:3;
	ssv_type_u32             do_rts_cts:2;   /* 0: no RTS/CTS, 1: need RTS/CTS */
                                    /* 2: CTS protection, 3: RSVD */
    ssv_type_u32             reason:6;

    /* The definition of WORD_4: */
    ssv_type_u32             payload_offset:8;
    ssv_type_u32             next_frag_pid:7;
    ssv_type_u32             RSVD_2:1;
    ssv_type_u32             fCmdIdx:3;
    ssv_type_u32             wsid:4;
    ssv_type_u32             txq_idx:3;
	ssv_type_u32				TxF_ID:6;


	/* The definition of WORD_4: */
	ssv_type_u32				rts_cts_nav:16;
	ssv_type_u32				frame_consume_time:10;	//32 units
	ssv_type_u32				crate_idx:6;

	/* The definition of WORD_5: */
	ssv_type_u32				drate_idx:6;
	ssv_type_u32				dl_length:12;
	ssv_type_u32				RSVD_3:14;


	union{
		PKT_TxPhyInfo  tx_phy_info;
		ssv_type_u8 			   phy_info[28];
	}un;

} PKT_TxInfo, *PPKT_TxInfo;

typedef struct PKT_RxInfo_st
{
    /* The definition of WORD_1: */
	ssv_type_u32             len:16;
	ssv_type_u32             c_type:3;
    ssv_type_u32             f80211:1;
	ssv_type_u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    ssv_type_u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    ssv_type_u32             use_4addr:1;
	ssv_type_u32             l3cs_err:1;
    ssv_type_u32             l4cs_err:1;
    ssv_type_u32             align2:1;
    ssv_type_u32             RSVD_0:2;
	ssv_type_u32             psm:1;
    ssv_type_u32             stype_b5b4:2;
    ssv_type_u32             extra_info:1;

    /* The definition of WORD_2: */
    ssv_type_u32             fCmd;

    /* The definition of WORD_3: */
    ssv_type_u32             hdr_offset:8;
    ssv_type_u32             frag:1;
    ssv_type_u32             unicast:1;
    ssv_type_u32             hdr_len:6;
    ssv_type_u32             RxResult:8;
    ssv_type_u32             wildcard_bssid:1;
    ssv_type_u32             RSVD_1:1;
    ssv_type_u32             reason:6;

    /* The definition of WORD_4: */
    ssv_type_u32             payload_offset:8;
    ssv_type_u32             next_frag_pid:7;
    ssv_type_u32             RSVD_2:1;
    ssv_type_u32             fCmdIdx:3;
    ssv_type_u32             wsid:4;
	ssv_type_u32				RSVD_3:3;
	ssv_type_u32				drate_idx:6;

	union{
		PKT_RxPhyInfo  rx_phy_info;
		ssv_type_u8 			   phy_info[20];
	}un;

} PKT_RxInfo, *PPKT_RxInfo;

typedef struct PKT_Info_st
{
    /* The definition of WORD_1: */
    ssv_type_u32             len:16;
    ssv_type_u32             c_type:3;
    ssv_type_u32             f80211:1;
    ssv_type_u32             qos:1;          /* 0: without qos control field, 1: with qos control field */
    ssv_type_u32             ht:1;           /* 0: without ht control field, 1: with ht control field */
    ssv_type_u32             use_4addr:1;
    ssv_type_u32             RSVD_0:5;
    ssv_type_u32             more_data:1;
    ssv_type_u32             stype_b5b4:2;
    ssv_type_u32             extra_info:1;

    /* The definition of WORD_2: */
    ssv_type_u32             fCmd;

    /* The definition of WORD_3: */
    ssv_type_u32             hdr_offset:8;
    ssv_type_u32             frag:1;
    ssv_type_u32             unicast:1;
    ssv_type_u32             hdr_len:6;
    ssv_type_u32             RSVD_1:10;
    ssv_type_u32             reason:6;

    /* The definition of WORD_4: */
    ssv_type_u32             payload_offset:8;
    ssv_type_u32             next_frag_pid:7;
    ssv_type_u32             RSVD_2:1;
    ssv_type_u32             fCmdIdx:3;
    ssv_type_u32             wsid:4;
    ssv_type_u32             RSVD_3:3;
	ssv_type_u32				drate_idx:6;

} PKT_Info, *PPKT_Info;

typedef struct PHY_Info_st
{
    ssv_type_u32             WORD1;
    ssv_type_u32             WORD2;
    ssv_type_u32             WORD3;
    ssv_type_u32             WORD4;
    ssv_type_u32             WORD5;
    ssv_type_u32             WORD6;
    ssv_type_u32             WORD7;

} PHY_Info, *PPHY_Info;

#define SSV6051_HOST_TXREQ0         struct cfg_host_txreq0
#define SSV6051_HOST_RXPKT          struct cfg_host_rxpkt
#define PKT_INFO_DATA_TYPE          PKT_Info
#define PKT_TXINFO_DATA_TYPE        PKT_TxInfo
#define PKT_RXINFO_DATA_TYPE        PKT_RxInfo

/**
 * Define constants for do_rts_cts field of PKT_TxInfo structure
 *
 * @ TX_NO_RTS_CTS
 * @ TX_RTS_CTS
 * @ TX_CTS
 */
#define TX_NO_RTS_CTS                   0
#define TX_RTS_CTS                      1
#define TX_CTS                          2

#ifdef THROUGHPUT_TEST

typedef struct SDIO_THROUGHPUT_COMMAND_st
{
	ssv_type_u32		transferCount;
	ssv_type_u16		transferLength;
	ssv_type_u16		resverd;
} SDIO_THROUGHPUT_COMMAND;


typedef struct THROUGHPUT_COMMAND_st
{
    ssv_type_u8		mode;
	ssv_type_u8		rateIndex;//0-38
	ssv_type_u8      noack;
	ssv_type_u16		transferLength;
	ssv_type_u16      qos;
	ssv_type_u32		transferCount;
} THROUGHPUT_COMMAND;

#endif

enum fcmd_seek_type {
    FCMD_SEEK_PREV  = 0,
    FCMD_SEEK_CUR,
    FCMD_SEEK_NEXT
};


#endif	/* _SSV6030_PKTDEF_H_ */

