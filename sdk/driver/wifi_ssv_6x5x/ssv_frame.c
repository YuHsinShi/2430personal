/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

#include <log.h>
//#include "lib-impl.h"
#include <ssv_lib.h>
#include <ssv_frame.h>

//#define SSV_FRAME_HEADER_RESV 100

#if (CONFIG_USE_LWIP_PBUF==1)
#include "lwip/pbuf.h"
#include "lwip/mem.h"
#elif (USE_LINUX_SKB == 1)
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#else
#endif

#include <pbuf.h>
extern OsMutex ssv_frame_mutex;

#define DRV_TRX_HDR_LEN         40

void _os_frame_free(void *frame)
{
    OS_MutexLock(ssv_frame_mutex);
#if (CONFIG_USE_LWIP_PBUF==1)
    if(frame)
    {
#if CONFIG_MEMP_DEBUG
    PBUF_DBG_FLAGS(((struct pbuf *)frame), PBUF_DBG_FLAG_RESET);
#endif
        pbuf_free((struct pbuf *)frame);
    }
    
#elif (USE_LINUX_SKB == 1)
    if(frame)
        dev_kfree_skb_any((struct sk_buff *)frame);
#else//#if USE_LWIP_PBUF
	if(frame)
		PBUF_MFree(frame);
#endif//#if USE_LWIP_PBUF
    OS_MutexUnLock(ssv_frame_mutex);
}


void* os_frame_alloc_fn(ssv_type_u32 size, ssv_type_bool SecPool,const char* file, const int line)
{

#if (CONFIG_USE_LWIP_PBUF==1)

    struct pbuf * p = NULL;
    OS_MutexLock(ssv_frame_mutex);

#if CONFIG_MEMP_DEBUG
    p = pbuf_alloc_fn(PBUF_RAW, size, PBUF_POOL, file, line);    
    
    if(p == NULL)
    {
        if(SecPool)
        {
            p = pbuf_alloc_fn(PBUF_RAW, size, PBUF_POOL_SEC, file, line);
        }
    }
    
    if(p != NULL)
        PBUF_DBG_FLAGS(p, PBUF_DBG_FLAG_L2);

#else
    p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
#endif
    OS_MutexUnLock(ssv_frame_mutex);
    return (void*)p;

#elif (USE_LINUX_SKB == 1)
    struct sk_buff *skb;
    OS_MutexLock(ssv_frame_mutex);
	skb = dev_alloc_skb(size + (80+ 3*16));//(PBU_OFFSET+ TXPB_RVSD*TX_PKT_RES_BASE)
    if(!skb)
    {
        LOG_PRINTF("%s,line=%d,size=%d\r\n",file,line,size);
        SSV_ASSERT(0);
    }
	skb_reserve(skb, 80);
    skb->len=size;
    OS_MutexUnLock(ssv_frame_mutex);
    return (void*)skb;
    
#else//CONFIG_USE_LWIP_PBUF
    PKT_Info * pPKtInfo;
    OS_MutexLock(ssv_frame_mutex);
	pPKtInfo = (PKT_Info *)PBUF_MAlloc(size ,SSV_NOTYPE_BUF);

    if((pPKtInfo == NULL)&&(SecPool))
    {
        pPKtInfo = (PKT_Info *)PBUF_MAlloc(size ,SSV_RX_BUF);
    }

	if(pPKtInfo)
    {   
    	pPKtInfo->len = size;
    	pPKtInfo->hdr_offset = 80;
    }
    OS_MutexUnLock(ssv_frame_mutex);
	return pPKtInfo;
#endif//#if USE_LWIP_PBUF
}


void* os_frame_dup(void *frame)
{
#if (CONFIG_USE_LWIP_PBUF==1)
    struct pbuf *new_p, *old_p;

	//u32 *p_end;
	ssv_type_u8 *p_end;
    ssv_type_u32 size;
    ssv_type_u32 shift_size;

    OS_MutexLock(ssv_frame_mutex);
    old_p = (struct pbuf *)frame;

	//p_end = (u32*)((u8*)old_p->payload + old_p->tot_len);
	p_end = (ssv_type_u8*)old_p->payload + old_p->tot_len;


    //original raw size
    size = (ssv_type_u32)p_end - (ssv_type_u32)((u8_t *)old_p+LWIP_MEM_ALIGN_SIZE(SIZEOF_STRUCT_PBUF+DRV_TRX_HDR_LEN));
	new_p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);

    if(new_p == NULL){        
        OS_MutexUnLock(ssv_frame_mutex);
        return (void*)new_p;
    }

    shift_size = old_p->tot_len - size;

    do{
        //+,-->increse header
        //-,-->decrease header
        if(pbuf_header(new_p, shift_size) != ERR_OK){
            pbuf_free(new_p);
            new_p = NULL;
            break;
        }

        if (pbuf_copy(new_p, old_p) != ERR_OK) {
              pbuf_free(new_p);
              new_p = NULL;
              break;
        }
    }while(0);

    OS_MutexUnLock(ssv_frame_mutex);
    return (void*)new_p;
#elif (USE_LINUX_SKB == 1)
    void* cln_skb;
    OS_MutexLock(ssv_frame_mutex);
    cln_skb = (void*)skb_clone((struct sk_buff*)frame, GFP_ATOMIC);
    OS_MutexUnLock(ssv_frame_mutex);
    return cln_skb;
#else//#if USE_LWIP_PBUF
    void * dframe;
    PKT_Info * pPKtInfo;


    OS_MutexLock(ssv_frame_mutex);

	pPKtInfo = (PKT_Info *)PBUF_MAlloc((((PKT_Info *)frame)->len) ,SSV_NOTYPE_BUF);

	if(pPKtInfo)
    {   
    	pPKtInfo->len = (((PKT_Info *)frame)->len);
    	pPKtInfo->hdr_offset = 80;
    }
    
	dframe = pPKtInfo;
    
	if(dframe)
		OS_MemCPY(((ssv_type_u8*)(dframe)+(((PKT_Info *)dframe)->hdr_offset)), ((ssv_type_u8*)(frame)+(((PKT_Info *)frame)->hdr_offset)), (((PKT_Info *)frame)->len));

    OS_MutexUnLock(ssv_frame_mutex);
    
	return (void*)dframe;

#endif//#if USE_LWIP_PBUF
}

//increase space to set header
void* os_frame_push(void *frame, ssv_type_u32 len)
{
    void *pdata;
#if CONFIG_USE_LWIP_PBUF
    struct pbuf *p = (struct pbuf *)frame;
    OS_MutexLock(ssv_frame_mutex);
    if(pbuf_header(p, len)!= ERR_OK){
       OS_MutexUnLock(ssv_frame_mutex);
       return NULL;
    }

    pdata = (void*)p->payload;
#elif (USE_LINUX_SKB == 1)

    struct sk_buff *skb = (struct sk_buff *)frame;
    OS_MutexLock(ssv_frame_mutex);
    pdata = skb_push(skb, len);

#else//#if USE_LWIP_PBUF
	PKT_Info * pPKtInfo = (PKT_Info * )frame;

    OS_MutexLock(ssv_frame_mutex);
	pPKtInfo->hdr_offset -= len;
	pPKtInfo->len  += len;

	pdata = (ssv_type_u8*)pPKtInfo+pPKtInfo->hdr_offset;
#endif//#if USE_LWIP_PBUF
    OS_MutexUnLock(ssv_frame_mutex);
    return pdata;
}


//decrease data space.
void* os_frame_pull(void *frame, ssv_type_u32 len)
{
    void *pdata;
#if (CONFIG_USE_LWIP_PBUF==1)

    struct pbuf *p = (struct pbuf *)frame;
    OS_MutexLock(ssv_frame_mutex);
    if(pbuf_header(p, -len)!= ERR_OK){
       OS_MutexUnLock(ssv_frame_mutex);
       return NULL;
    }

    pdata = (void*)p->payload;
#elif (USE_LINUX_SKB == 1)

    struct sk_buff *skb = (struct sk_buff *)frame;
    OS_MutexLock(ssv_frame_mutex);
    pdata = (void*)skb_pull(skb, len);
    
#else//#if USE_LWIP_PBUF
	PKT_Info * pPKtInfo = (PKT_Info * )frame;

    OS_MutexLock(ssv_frame_mutex);
	pPKtInfo->hdr_offset += len;
	pPKtInfo->len  -= len;

	pdata = (ssv_type_u8*)pPKtInfo+pPKtInfo->hdr_offset;
#endif//#if USE_LWIP_PBUF

    OS_MutexUnLock(ssv_frame_mutex);
    return pdata;
}

ssv_type_u8* os_frame_get_data_addr(void *_frame)
{
    ssv_type_u8* pdata;
    OS_MutexLock(ssv_frame_mutex);
#if (CONFIG_USE_LWIP_PBUF==1)
    pdata = ((ssv_type_u8*)(((struct pbuf *)_frame)->payload));
#elif (USE_LINUX_SKB == 1)
    struct sk_buff *skb = (struct sk_buff *)_frame;
    pdata = (ssv_type_u8*)skb->data;
#else//#if USE_LWIP_PBUF 
    pdata = ((ssv_type_u8*)(_frame)+(((PKT_Info *)_frame)->hdr_offset));
#endif//#if USE_LWIP_PBUF 
    OS_MutexUnLock(ssv_frame_mutex);
    return pdata;
}

ssv_type_u32 os_frame_get_data_len(void *_frame)
{
    ssv_type_u32 len;
    OS_MutexLock(ssv_frame_mutex);
#if (CONFIG_USE_LWIP_PBUF==1)
    len = (((struct pbuf *)_frame)->tot_len);
#elif (USE_LINUX_SKB == 1)
    struct sk_buff *skb = (struct sk_buff *)_frame;
    len = skb->len;
#else//#if USE_LWIP_PBUF 
    len = (((PKT_Info *)_frame)->len);
#endif//#if USE_LWIP_PBUF 
    OS_MutexUnLock(ssv_frame_mutex);
    return len;
}

void os_frame_set_data_len(void *_frame, ssv_type_u32 _len)
{
    OS_MutexLock(ssv_frame_mutex);
#if (CONFIG_USE_LWIP_PBUF == 1)
    do{
        if(_len == 0) {abort();}
        ((struct pbuf *)_frame)->tot_len = _len;
        ((struct pbuf *)_frame)->len = _len;
    }while(0);
    
#elif (USE_LINUX_SKB == 1)
        struct sk_buff *skb = (struct sk_buff *)_frame;
        skb->len = _len;
#else//#if USE_LWIP_PBUF 
    (((PKT_Info *)_frame)->len = _len);
#endif//#if USE_LWIP_PBUF 
    OS_MutexUnLock(ssv_frame_mutex);
}

//----------------------------------------------

void os_frame_set_debug_flag(void *frame, ssv_type_u32 flag)
{
#if CONFIG_USE_LWIP_PBUF
#if CONFIG_MEMP_DEBUG    
    switch(flag)
    {
        case SSV_PBUF_DBG_FLAG_L2:
            flag = PBUF_DBG_FLAG_L2;
            break;
        case SSV_PBUF_DBG_FLAG_L2_TX_DRIVER:
            flag = PBUF_DBG_FLAG_L2_TX_DRIVER;
            break;
        case SSV_PBUF_DBG_FLAG_L2_CMDENG:
            flag = PBUF_DBG_FLAG_L2_CMDENG;
            break;
    }
    PBUF_DBG_FLAGS(((struct pbuf *)frame), flag);
#endif
#endif
}



