/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _PBUF_H_
#define _PBUF_H_
#include <porting.h>
#include <host_config.h>

#if((CONFIG_CHIP_ID==SSV6051Q)||(CONFIG_CHIP_ID==SSV6051Z)||(CONFIG_CHIP_ID==SSV6030P))
#include <SSV6030/ssv6030_pktdef.h>
#endif

#if((CONFIG_CHIP_ID==SSV6006B)||(CONFIG_CHIP_ID==SSV6006C))
#include <SSV6006/ssv6006_pktdef.h>
#endif

typedef enum SSV_PBUF_TPY {
    SSV_NOTYPE_BUF  = 0,
    SSV_TX_BUF      = 1,
    SSV_RX_BUF      = 2
} SSV_PBUF_TPY_E;

#define SSV_PBUF_DBG_FLAG_RESET      (0)
#define SSV_PBUF_DBG_FLAG_TCP        (1)
#define SSV_PBUF_DBG_FLAG_UDP        (2)
#define SSV_PBUF_DBG_FLAG_IP         (3)
#define SSV_PBUF_DBG_FLAG_ARP                   (4)
#define SSV_PBUF_DBG_FLAG_L2                    (5)
#define SSV_PBUF_DBG_FLAG_L2_TX_DRIVER          (6)
#define SSV_PBUF_DBG_FLAG_L2_CMDENG          	(7)

void        *PBUF_MAlloc_Raw(ssv_type_u32 size, ssv_type_u32 need_header, SSV_PBUF_TPY_E bftype);
void		_PBUF_MFree(void *PKTMSG);

#define PBUF_MAlloc(size, type)  PBUF_MAlloc_Raw(size, 1, type);
#define PBUF_MFree(PKTMSG) _PBUF_MFree(PKTMSG)
#define PBUF_isPkt(addr)    (1)
#define SIZEOF_STRUCT_PBUF        LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf)) //from lwip/pbuf.c

ssv_type_s32 PBUF_Init(ssv_type_u32 pbuf_no);
ssv_type_s32 PBUF_DeInit(void);

#endif /* _PBUF_H_ */





