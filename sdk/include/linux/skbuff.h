/*
* Copyright (c) 2019 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
*
* @author Irene Lin
* @version 1.0
*/

#ifndef SKB_BUFF_H
#define SKB_BUFF_H

#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN	    0
#endif

#include "os.h"
#include "spinlock.h"
#include "netdevice.h"
#include "compiler.h"

struct sk_buff;

struct sk_buff_head {
    /* These two members must be first. */
    struct sk_buff	*next;
    struct sk_buff	*prev;

    u32		qlen;
    spinlock_t	lock;
};

struct sk_buff {
    struct sk_buff	*next;
    struct sk_buff	*prev;

    unsigned char		*tail;
    unsigned char		*end;
    unsigned char		*head;
    unsigned char		*data;

    unsigned int    len;
    unsigned int    data_len;

    struct net_device	*dev;
    char			cb[48] __aligned(8);

    __be16     protocol;  /* big-endian */
    unsigned int      ip_summed : 2;
    unsigned int      alloc_data_buf : 1;
};

struct net_device;

#define __netdev_alloc_skb  __netdev_alloc_skb_ite
#define alloc_skb(size, x)  __netdev_alloc_skb_ite(NULL, size, x)

struct sk_buff *__netdev_alloc_skb_ite(struct net_device *dev, 
    unsigned int bufsz, gfp_t gfp);

/**
 *	skb_reserve - adjust headroom
 *	@skb: buffer to alter
 *	@len: bytes to move
 *
 *	Increase the headroom of an empty &sk_buff by reducing the tail
 *	room. This is only allowed for an empty buffer.
 */
static inline void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
}

#define netdev_alloc_skb_ip_align(a,b)     __netdev_alloc_skb_ip_align(a,b,0) 
static inline struct sk_buff *__netdev_alloc_skb_ip_align(struct net_device *dev,
    unsigned int length, gfp_t gfp)
{
    struct sk_buff *skb = __netdev_alloc_skb(dev, length + NET_IP_ALIGN, gfp);

    if (NET_IP_ALIGN && skb)
        skb_reserve(skb, NET_IP_ALIGN);
    return skb;
}

#define kfree_skb       dev_kfree_skb_any
#define dev_kfree_skb	dev_kfree_skb_any
#define dev_consume_skb_any     dev_kfree_skb_any
static inline void dev_kfree_skb_any(struct sk_buff *skb)
{
    if (skb->alloc_data_buf)
        itpVmemFree((u32)skb->data);
    //printf("skb: free %p \n", skb);

    free(skb);
}

/*
*	Insert an sk_buff on a list.
*
*	The "__skb_xxxx()" functions are the non-atomic ones that
*	can only be called with interrupts disabled.
*/
static inline void __skb_insert(struct sk_buff *newsk,
        struct sk_buff *prev, struct sk_buff *next,
        struct sk_buff_head *list)
{
    newsk->next = next;
    newsk->prev = prev;
    next->prev = prev->next = newsk;
    list->qlen++;
}

static inline void __skb_queue_before(struct sk_buff_head *list,
                struct sk_buff *next,
                struct sk_buff *newsk)
{
    __skb_insert(newsk, next->prev, next, list);
}

/**
*	__skb_queue_tail - queue a buffer at the list tail
*	@list: list to use
*	@newsk: buffer to queue
*
*	Queue a buffer at the end of a list. This function takes no locks
*	and you must therefore hold required locks before calling it.
*
*	A buffer cannot be placed on two lists at the same time.
*/
static inline void __skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
    __skb_queue_before(list, (struct sk_buff *)list, newsk);
}

static inline void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
    unsigned long flags;

    spin_lock_irqsave(&list->lock, flags);
    __skb_queue_tail(list, newsk);
    spin_unlock_irqrestore(&list->lock, flags);
}

/*
* remove sk_buff from list. _Must_ be called atomically, and with
* the list known..
*/
static inline void __skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
    struct sk_buff *next, *prev;

    list->qlen--;
    next = skb->next;
    prev = skb->prev;
    skb->next = skb->prev = NULL;
    next->prev = prev;
    prev->next = next;
}

/**
*	skb_peek - peek at the head of an &sk_buff_head
*	@list_: list to peek at
*
*	Peek an &sk_buff. Unlike most other operations you _MUST_
*	be careful with this one. A peek leaves the buffer on the
*	list and someone else may run off with it. You must hold
*	the appropriate locks or have a private queue to do this.
*
*	Returns %NULL for an empty list or a pointer to the head element.
*	The reference count is not incremented and the reference is therefore
*	volatile. Use with caution.
*/
static inline struct sk_buff *skb_peek(const struct sk_buff_head *list_)
{
    struct sk_buff *skb = list_->next;

    if (skb == (struct sk_buff *)list_)
        skb = NULL;
    return skb;
}

#define skb_dequeue     skb_dequeue_ite
#define skb_queue_purge skb_queue_purge_ite
/**
*	__skb_dequeue - remove from the head of the queue
*	@list: list to dequeue from
*
*	Remove the head of the list. This function does not take any locks
*	so must be used with appropriate locks held only. The head item is
*	returned or %NULL if the list is empty.
*/
static inline struct sk_buff *__skb_dequeue(struct sk_buff_head *list)
{
    struct sk_buff *skb = skb_peek(list);
    if (skb)
        __skb_unlink(skb, list);
    return skb;
}
struct sk_buff *skb_dequeue_ite(struct sk_buff_head *list);
void skb_queue_purge_ite(struct sk_buff_head *list);

static inline void skb_queue_head_init(struct sk_buff_head *list)
{
    spin_lock_init(&list->lock);
    list->prev = list->next = (struct sk_buff *)list;
    list->qlen = 0;
}

/**
*	skb_queue_empty - check if a queue is empty
*	@list: queue head
*
*	Returns true if the queue is empty, false otherwise.
*/
static inline int skb_queue_empty(const struct sk_buff_head *list)
{
    return list->next == (const struct sk_buff *) list;
}

#define skb_queue_walk(queue, skb) \
for (skb = (queue)->next;					\
    skb != (struct sk_buff *)(queue);				\
    skb = skb->next)

/**
*	skb_put - add data to a buffer
*	@skb: buffer to use
*	@len: amount of data to add
*
*	This function extends the used data area of the buffer. If this would
*	exceed the total buffer size the kernel will panic. A pointer to the
*	first byte of the extra data is returned.
*/
static inline void *skb_put(struct sk_buff *skb, unsigned int len)
{
    unsigned char *tmp = skb->tail;
    skb->tail += len;
    skb->len += len;
    return tmp;
}
#define __skb_put   skb_put

static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len)
{
    void *tmp = skb_put(skb, len);

    memset(tmp, 0, len);

    return tmp;
}

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
    unsigned int len)
{
    void *tmp = skb_put(skb, len);

    memcpy(tmp, data, len);

    return tmp;
}

/**
*	skb_tailroom - bytes at buffer end
*	@skb: buffer to check
*
*	Return the number of bytes of free space at the tail of an sk_buff
*/
static inline int skb_tailroom(const struct sk_buff *skb)
{
    return skb->end - skb->tail;
}

/**
* eth_proto_is_802_3 - Determine if a given Ethertype/length is a protocol
* @proto: Ethertype/length value to be tested
*
* Check that the value from the Ethertype/length field is a valid Ethertype.
*
* Return true if the valid is an 802.3 supported Ethertype.
*/
static inline bool eth_proto_is_802_3(__be16 proto)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* if CPU is little endian mask off bits representing LSB */
    proto &= htons(0xFF00);
#endif
    //printf("eth_proto_is_802_3(%X) %X >= %X ? \n", (u16)proto, (u16)htons(ETH_P_802_3_MIN));
    /* cast both to u16 and compare since LSB can be ignored */
    return (u16)proto >= (u16)htons(ETH_P_802_3_MIN);
}

static inline __be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
    const struct ethhdr *eth;

    eth = (struct ethhdr *)skb->data;

    if (eth_proto_is_802_3(eth->h_proto))
        return eth->h_proto;

    //printf("eth_type_trans() return htons(ETH_P_802_2) \n");
    /*
    *      Real 802.2 LLC
    */
    return htons(ETH_P_802_2);
}

#define skb_defer_rx_timestamp(skb)     0
#define skb_tx_timestamp(skb)



#endif //SKB_BUFF_H
