/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _AP_TX_H_
#define _AP_TX_H_

#include <ssv_types.h>
#include <ssv_lib.h>

#include "common/defs.h"



struct APStaInfo;



#define AP_TX_UNICAST				BIT(1)

//#define AP_TX_PS_BUFFERED	BIT(2)
#define AP_TX_DATA					BIT(2)

//802.11 frame belong to native AP
#define AP_TX_NATIVE_AP_FRAME		BIT(3)

//Trigger frame/ps poll frame
#define AP_TX_POLL_RESPONSE			BIT(4)




struct cfg_host_txreq0;




struct ap_tx_desp {
	/* These two members must be first. */
	//struct ap_tx_desp *prev;
	//struct ap_tx_desp *next;
	struct ssv_list_q  list;
    
	ssv_type_u32 *frame;								//tx frame pointer
	struct cfg_host_txreq0 *host_txreq0;	//tx descriptor pointer
	struct APStaInfo  *sta;

	ssv_type_u8 *data;								//Pointer to 802.11/802.3 header

	ssv_type_u16 nDataLen;							//Pointer to 802.11/802.3 frame len	
	ssv_type_u8 priority;
	
	ssv_type_u32 flags;
	ssv_type_u32 jiffies;	
} ;


//struct ap_tx_desp_head { 
	/* These two members must be first. */
//	struct ap_tx_desp *prev;
//	struct ap_tx_desp *next;


//	u8 len;
//};


//void ap_tx_desp_head_init(struct list_q *list);
struct ap_tx_desp *ap_tx_desp_dequeue(struct ssv_list_q *list);
//void skb_queue_tail(struct ap_tx_desp_head *list, struct ap_tx_desp *new);



void ap_release_tx_desp(struct ap_tx_desp *tx_desp);





static ssv_inline int ap_tx_desp_queue_empty(const struct ssv_list_q *list, OsMutex *pmtx )
{
    int result;
    OS_MutexLock(*pmtx);
    result = (const struct ssv_list_q *)list->next == list;
    OS_MutexUnLock(*pmtx);

	return result;
}







void ap_free_host_txreq(struct cfg_host_txreq0 **host_txreq0);
//u32 ap_tx_desp_queue_len(const struct list_q *list_);
//void ap_tx_desp_head_init(struct list_q *list);
struct ap_tx_desp *ap_tx_desp_peek(const struct ssv_list_q *list_);
//struct ap_tx_desp *ap_tx_desp_dequeue(struct ap_tx_desp_head *list);
//static inline void ap_tx_desp_insert(struct ap_tx_desp *new,
//struct ap_tx_desp *prev, struct ap_tx_desp *next,
//struct ap_tx_desp_head *list);
//void ap_tx_desp_queue_tail(struct list_q *list, struct ap_tx_desp *new_ap_tx_desp);











#endif /* _AP_TX_H_ */
