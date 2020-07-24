#ifndef ITE_WAIT_H
#define ITE_WAIT_H

#include "list.h"
#include "ite/ith.h"
#include "ite/itp.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct __wait_queue_head {
	volatile int is_wait; 
	sem_t sem;
    spinlock_t  lock;
    struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;
#define init_waitqueue_head(q)	    do { (q)->is_wait = 0; spin_lock_init(&(q)->lock); INIT_LIST_HEAD(&(q)->task_list); } while (0)


struct __wait_queue {
    void *private;
    struct list_head task_list;
};
typedef struct __wait_queue wait_queue_t;

#define __WAITQUEUE_INITIALIZER(name, tsk) {				\
    .private = (void*)tsk, \
    .task_list = { NULL, NULL } }

#define DECLARE_WAITQUEUE(name, tsk)					\
    wait_queue_t name = __WAITQUEUE_INITIALIZER(name, tsk)

#define __add_wait_queue(head, new)     do { list_add(&new->task_list, &head->task_list); } while(0)
static inline void add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
{
    unsigned long flags;

    spin_lock_irqsave(&q->lock, flags);
    __add_wait_queue(q, wait);
    spin_unlock_irqrestore(&q->lock, flags);
}

#define __remove_wait_queue(head, old)  do { list_del(&old->task_list); } while(0)
static inline void remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait)
{
    unsigned long flags;

    spin_lock_irqsave(&q->lock, flags);
    __remove_wait_queue(q, wait);
    spin_unlock_irqrestore(&q->lock, flags);
}

static inline int waitqueue_active(wait_queue_head_t *q)
{
    return !list_empty(&q->task_list);
}

#define wake_up_all     wake_up

#define wake_up(x)      \
    do {        \
        if ((x)->is_wait)  \
            sem_post(&(x)->sem); \
    } while(0); 
#define wake_up_interruptible(x)	wake_up(x)

/**
 * wait_event_timeout - sleep until a condition gets true or a timeout elapses
 * @wq: the waitqueue to wait on
 * @condition: a C expression for the event to wait for
 * @timeout: timeout, in jiffies
 * @interval: in ms
 *
 * The process is put to sleep until the
 * @condition evaluates to true. The @condition is checked each time
 * the waitqueue @wq is woken up.
 *
 * wake_up() has to be called after changing any variable that could
 * change the result of the wait condition.
 *
 * The function returns 0 if the @timeout elapsed, or the remaining
 * jiffies (at least 1) if the @condition evaluated to %true before
 * the @timeout elapsed.
 */
#define _wait_event_timeout(wq, condition, timeout, interval)			\
({									\
	long __ret = timeout;						\
	if(!condition) { \
    sem_init(&wq.sem, 0, 0);    \
    wq.is_wait = 1;    \
    do {    \
        if (!(condition))	{			\
	            if (itpSemWaitTimeout(&wq.sem, interval)) {	\
	                __ret -= interval;  \
	                if (((int)__ret)<=0/* && (interval>=10)*/) \
	                { __ret=0; /*ithPrintf("%s:%d wait_event_timeout(%d) \n", __FILE__, __LINE__, timeout);*/ break; } \
            }                                       \
        } else                                          \
            break;  \
    } while(1); \
    wq.is_wait = 0;    \
    sem_destroy(&wq.sem);   \
	}  \
	__ret;								\
})

#define wait_event(wq, condition)                   _wait_event_timeout((wq), (condition), 200*1000*1000, 10)
#define wait_event_timeout(wq, condition, timeout)  _wait_event_timeout((wq), (condition), timeout, 10)
#define wait_event_interruptible_timeout(wq, condition, timeout)	 wait_event_timeout(wq, condition, timeout)


#ifdef __cplusplus
}
#endif

#endif // ITE_WAIT_H
