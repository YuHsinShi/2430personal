#ifndef LINUX_OS_H
#define LINUX_OS_H

#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "ite/itp.h"
#include "ite/ith.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*----------------------------------------------------------*/
/* 
  * type related 
  */
typedef unsigned char		u8;
typedef unsigned char		__u8;
typedef unsigned short		u16;
typedef unsigned short		__u16;
typedef unsigned short		__le16;
typedef unsigned short		__be16;
typedef unsigned int		u32;
typedef unsigned int		__u32;
typedef unsigned int		__le32;
typedef unsigned int		__be32;
typedef signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed long long 	s64;
typedef unsigned long long	u64;
typedef unsigned int		uint;
typedef signed int			sint;

typedef unsigned int		gfp_t;
typedef unsigned int		dma_addr_t;
typedef long long	        loff_t;


/*----------------------------------------------------------*/
#ifndef min
#define min		ITH_MIN
#endif
#ifndef max
#define max		ITH_MAX
#endif

#ifndef ALIGN
#define ALIGN	ITH_ALIGN_UP
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((sizeof(a)) / (sizeof((a)[0])))
#endif

#ifndef BIT
#define BIT(nr)			(1UL << (nr))
#endif

#ifndef clamp
#define clamp(val, min, max) ({			\
	typeof(val) __val = (val);		\
	typeof(min) __min = (min);		\
	typeof(max) __max = (max);		\
	(void) (&__val == &__min);		\
	(void) (&__val == &__max);		\
	__val = __val < __min ? __min: __val;	\
	__val > __max ? __max: __val; })
#endif	

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#endif

/* is x a power of 2? */
#define is_power_of_2(x)	((x) != 0 && (((x) & ((x) - 1)) == 0))

/*----------------------------------------------------------*/
/* 
  * time related 
  */
#ifndef jiffies
#define jiffies		itpGetTickCount()
#endif

#ifndef time_after
#define time_after(a,b)   ((long)(b) - (long)(a) < 0)
#endif

#ifndef time_before
#define time_before(a,b)   time_after(b,a)
#endif

#ifndef msleep
#define msleep(x)					usleep(x*HZ)
#endif

#define schedule_timeout_uninterruptible(x)  msleep(x)

#ifndef udelay
#define udelay  ithDelay
#endif

#ifndef mdelay
#define mdelay(x)  ithDelay((x)*HZ)
#endif

#ifndef msecs_to_jiffies
#define msecs_to_jiffies(x)		(x)
#endif

#ifndef jiffies_to_msecs
#define jiffies_to_msecs(x)		(x)
#endif

#ifndef might_sleep
#define might_sleep()		do{ } while(0)
#endif

static inline int msleep_interruptible(u32 ms)  { msleep(ms); return 0; }

/* Parameters used to convert the timespec values: */
#define NSEC_PER_MSEC	1000000L

/*----------------------------------------------------------*/
/* 
  * memory related 
  */
#define GFP_DMA         0
#define GFP_ATOMIC    	1
#define GFP_KERNEL    	2
#define GFP_NOIO	    3

#ifndef kmalloc
//#define kmalloc(a,b)				malloc(a)
#define kmalloc(a,b)                (void*)memalign(64, ITH_ALIGN_UP(a, 32))
#endif
#ifndef kfree
#define kfree(x)					do { if((x)) free((void*)(x)); } while(0)
#endif
#ifndef __get_free_page
#define __get_free_page(b)      kmalloc(4096,b)
#endif
#ifndef free_page
#define free_page(x)            kfree(x)
#endif

static inline void *kzalloc(u32 size, int x)
{ 
    //void *tmp = malloc(size);
    void *tmp = (void*)memalign(64, ITH_ALIGN_UP(size, 32));
    if(tmp)
        memset(tmp, 0, size);
    return tmp;
}

/**
 * kcalloc - allocate memory for an array. The memory is set to zero.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{
	return kzalloc((n*size), flags);
}

static inline void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
        void *ret = kmalloc(len, gfp);
        if (ret)
                memcpy(ret, src, len);
        return ret;
}

#ifndef vmalloc
#define vmalloc(a)                (void*)memalign(64, ITH_ALIGN_UP(a, 32))
#endif
#ifndef vfree
#define vfree(x)					do { if((x)) free((x)); } while(0)
#endif

static inline void *vzalloc(u32 size)
{ 
    void *tmp = malloc(size);
    if(tmp)
        memset(tmp, 0, size);
    return tmp;
}


#define wmb()   ithFlushMemBuffer()
#define rmb()   do{ } while(0)
#define mmiowb() do{ } while(0)
#define smp_mb__before_atomic() do{ } while(0)

#ifndef __iomem
#define __iomem
#endif

/*----------------------------------------------------------*/
/* 
  * endian related 
  */
#ifndef __le16_to_cpu
#define __le16_to_cpu   le16_to_cpu
#endif
#ifndef __be16_to_cpu
#define __be16_to_cpu   be16_to_cpu
#endif
#ifndef __le32_to_cpu
#define __le32_to_cpu	le32_to_cpu
#endif
#ifndef be16_to_cpup
#define be16_to_cpup(x) be16_to_cpu((*(x)))
#endif
#ifndef le16_to_cpup
#define le16_to_cpup(x) le16_to_cpu((*(x)))
#endif
#ifndef le32_to_cpup
#define le32_to_cpup(x) le32_to_cpu((*(x)))
#endif
#ifndef __cpu_to_le16
#define __cpu_to_le16	cpu_to_le16
#endif
#ifndef __cpu_to_be16
#define __cpu_to_be16	cpu_to_be16
#endif

/*----------------------------------------------------------*/
/* 
  * mutex related 
  */
struct mutex {
    pthread_mutex_t mutex;
};
#define mutex_init(x)       pthread_mutex_init(&(x)->mutex, NULL)
#define mutex_destroy(x)    pthread_mutex_destroy(&(x)->mutex)
#define mutex_lock(x)       pthread_mutex_lock(&(x)->mutex)
#define mutex_trylock(x)    pthread_mutex_trylock(&(x)->mutex)
#define mutex_unlock(x)     pthread_mutex_unlock(&(x)->mutex)
#define DEFINE_MUTEX(mutexname) \
	struct mutex mutexname = { .mutex = PTHREAD_MUTEX_INITIALIZER }

/*----------------------------------------------------------*/
/* 
  * others
  */
#define	MAX_SCHEDULE_TIMEOUT	((long)(~0UL>>1))

#if defined(WIN32)
#define _PRINTF     printf
#else
#define _PRINTF     ithPrintf
#endif

#define KERN_EMERG	 "0"	/* system is unusable */
#define KERN_ALERT	 "1"	/* action must be taken immediately */
#define KERN_CRIT	 "2"	/* critical conditions */
#define KERN_ERR	 "3"	/* error conditions */
#define KERN_WARNING	 "4"	/* warning conditions */
#define KERN_NOTICE	 "5"	/* normal but significant condition */
#define KERN_INFO	 "6"	/* informational */
#define KERN_DEBUG	 "7"	/* debug-level messages */
#define KERN_DEFAULT	 "d"	/* the default kernel loglevel */


#ifndef printk
#define printk  _PRINTF
#endif

#define BUG_ON(x)	do { if(x) _PRINTF(" %s:%d => BUG?? \n", __FILE__, __LINE__); } while(0)
#define BUG()       do { _PRINTF(" %s:%d => BUG!! \n", __FILE__, __LINE__); } while(0)

#ifndef likely
#define likely(x)	(x)
#endif
#ifndef unlikely
#define unlikely(x)	(x)
#endif
#define in_interrupt()	(ithGetCpuMode() != ITH_CPU_SYS)

#if defined(WIN32)
#define WARN_ON(x) 			do { if(x) _PRINTF(" %s:%s():%d => WARN_ON?? \n", __FILE__, __FUNCTION__, __LINE__); } while(0)
#else
#define WARN_ON(condition)							\
	({								\
		int __ret_warn_on = !!(condition);			\
		if (unlikely(__ret_warn_on))				\
			_PRINTF(" %s:%s():%d => WARN_ON?? \n", __FILE__, __FUNCTION__, __LINE__);		\
		unlikely(__ret_warn_on);				\
	})
#endif

#define WARN_ON_ONCE(x) 	WARN_ON(x)	

#define EXPORT_SYMBOL_GPL(x)
#define EXPORT_SYMBOL(x)

#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)
#define MODULE_LICENSE(x)
#define MODULE_VERSION(x)

#define module_init(x)
#define module_exit(x)


#ifndef __devinit
#define __devinit
#endif
#ifndef __devexit
#define __devexit
#endif
#ifndef __devexit_p
#define __devexit_p(func)   func
#endif
#ifndef __exit
#define __exit
#endif



#ifdef __cplusplus
}
#endif

#endif // LINUX_OS_H
