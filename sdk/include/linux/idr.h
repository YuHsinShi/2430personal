#ifndef IDR_H
#define IDR_H

#include "bitops.h"

struct idr {
    uint32_t    bitmap;  /* maximum number is 32 */
    spinlock_t	lock;
    void *ctxt[32];
};

#define DEFINE_IDR(name)    	struct idr name = { 0 }

static inline int idr_alloc(struct idr *idr, void *ptr, int start, int end, gfp_t gfp_mask)
{
    int i;
    unsigned long flags;
    int _end = (end < 32) ? end : 32;

    if (start > _end) {
        ithPrintf("idr_alloc: start %d > _end %d ===> TODO!! \n", start, end);
        return -1;
    }

    spin_lock_irqsave(&idr->lock, flags);
    for (i = start; i < _end; i++) {
		if (!test_bit(i, &idr->bitmap)) {
            idr->ctxt[i] = ptr;
			set_bit(i, &idr->bitmap);
			break;
    	}
    }
    spin_unlock_irqrestore(&idr->lock, flags);

    if (i >= _end)
        return -99;

    return i;
}

static inline void idr_remove(struct idr *idr, int id)
{
    unsigned long flags;

    spin_lock_irqsave(&idr->lock, flags);
    idr->ctxt[id] = NULL;
	clear_bit(id, &idr->bitmap);
    spin_unlock_irqrestore(&idr->lock, flags);
}

static inline void *idr_find(struct idr *idr, int id)
{
    return idr->ctxt[id];
}

static inline void idr_destroy(struct idr *idr)
{
}

#endif // IDR_H
