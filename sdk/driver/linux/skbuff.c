/*
* Copyright (c) 2019 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
*
* @author Irene Lin
* @version 1.0
*/
#include <stdio.h>
#include <linux/skbuff.h>


struct sk_buff *__netdev_alloc_skb_ite(struct net_device *dev,
    unsigned int bufsz, gfp_t gfp)
{
    struct sk_buff *skb;
    u8 *data = NULL;

    skb = malloc(sizeof(struct sk_buff));
    if (!skb)
        goto out;

    memset(skb, 0, sizeof(struct sk_buff));
    if (bufsz) {
        data = (u8*)itpVmemAlloc(bufsz);   /** already 64-bytes alignment */
        if (!data)
            goto alloc_fail;

		skb->head = data;
		skb->data = data;
		skb->tail = data;
		skb->end  = data;
        skb->alloc_data_buf = 1;
    }
    //printf("skb: alloc %p (%d) buf: %p \n", skb, bufsz, data);
out:
    if (!skb)
        printf("alloc skb fail!!!\n");
    return skb;
alloc_fail:
    free(skb);
    goto out;
}

struct sk_buff *skb_dequeue_ite(struct sk_buff_head *list)
{
    unsigned long flags;
    struct sk_buff *result;

    spin_lock_irqsave(&list->lock, flags);
    result = __skb_dequeue(list);
    spin_unlock_irqrestore(&list->lock, flags);
    return result;
}

/**
*	skb_queue_purge - empty a list
*	@list: list to empty
*
*	Delete all buffers on an &sk_buff list. Each buffer is removed from
*	the list and one reference dropped. This function takes the list
*	lock and is atomic with respect to other list locking functions.
*/
void skb_queue_purge_ite(struct sk_buff_head *list)
{
    struct sk_buff *skb;
    while ((skb = skb_dequeue(list)) != NULL)
        kfree_skb(skb);
}
