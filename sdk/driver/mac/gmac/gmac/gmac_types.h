/*
 * Copyright (c) 2017 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as Gigabit Ethernet error code header file.
 *
 * @author Irene Lin
 */

#ifndef GMAC_TYPES_H
#define GMAC_TYPES_H

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned int		__le32;
typedef unsigned short		__be16;
typedef unsigned int		gfp_t;
typedef unsigned int		__be32;
typedef unsigned int		dma_addr_t;
typedef unsigned long long	u64;
typedef signed int			s32;

#define jiffies		itpGetTickCount()

static inline int copy_from_user(void *des, const void *src, int len) {
    memcpy(des, src, len);
    return 0;
}

static inline int copy_to_user(void *des, void *src, int len) {
    memcpy(des, src, len);
    return 0;
}

static inline void *vzalloc(u32 size)
{
    void *tmp = malloc(size);
    if (tmp)
        memset(tmp, 0, size);
    return tmp;
}
#define vfree(x)					do { if((x)) free((x)); } while(0)

static inline void *kzalloc(u32 size, int x)
{
    void *tmp = (void*)memalign(64, ITH_ALIGN_UP(size, 32));
    if (tmp)
        memset(tmp, 0, size);
    return tmp;
}
#define kfree(x)					do { if((x)) free((void*)(x)); } while(0)

static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{
    return kzalloc((n*size), flags);
}

#define min_t(type, x, y) ({			\
    type __min1 = (x);			\
    type __min2 = (y);			\
    __min1 < __min2 ? __min1 : __min2; })

#define time_after(a,b)   ((long)(b) - (long)(a) < 0)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define BUG_ON(x)	do { if(x) ithPrintf(" %s:%d => BUG?? \n", __FILE__, __LINE__); } while(0)

#define WARN_ON(condition)							\
({ \
    int __ret_warn_on = !!(condition);			\
    if (unlikely(__ret_warn_on))				\
        ithPrintf(" %s:%s():%d => WARN_ON?? \n", __FILE__, __FUNCTION__, __LINE__);		\
    unlikely(__ret_warn_on);				\
})

#define udelay  ithDelay
#define min		ITH_MIN
#define max		ITH_MAX
#define ALIGN	ITH_ALIGN_UP
#define likely(x)	(x)
#define unlikely(x)	(x)

#define GFP_ATOMIC    	1
#define GFP_KERNEL    	2


#endif
