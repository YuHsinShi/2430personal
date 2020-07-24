#include <stdio.h>
#include <pthread.h>
#include <linux/os.h>
#include <linux/list.h>
#include <linux/hrtimer.h>
#include <linux/spinlock.h>
#include "ite/itp.h"

#define HRTIMER_DBG     0
#define HRTIMER_ERR     1

#if defined(_WIN32)

#if HRTIMER_DBG
#define hrt_dbg(string, ...)    do { printf("[HRT][DBG] "); printf(string, __VA_ARGS__); } while (0)
#else
#define hrt_dbg(string, ...)
#endif

#if HRTIMER_ERR
#define hrt_err(string, ...)    do { printf("[HRT][ERR] "); printf(string, __VA_ARGS__); } while (0)
#else
#define hrt_err(string, ...)
#endif

#else // #if defined(_WIN32)

#if HRTIMER_DBG
#define hrt_dbg(string, args...)    do { ithPrintf("[HRT][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define hrt_dbg(string, args...)
#endif

#if HRTIMER_ERR
#define hrt_err(string, args...)    do { ithPrintf("[HRT][ERR] "); ithPrintf(string, ## args); } while (0)
#else
#define hrt_err(string, args...)
#endif

#endif // #if defined(_WIN32)


struct hrtimer_ctrl {
    struct list_head    list;  /* hrtimer list */
    sem_t               sem;
    sem_t               thread_ready;
    spinlock_t          lock;
    pthread_t           thread;
};

static struct hrtimer_ctrl g_hrtimer_ctrl;
static struct hrtimer_ctrl *hrtimer_c;

#if defined(WIN32)

#undef list_for_each_entry

#define list_for_each_entry(pos, head, member)				\
for (pos = list_entry((head)->next, struct hrtimer, member);	\
    &pos->member != (head);	\
    pos = list_entry(pos->member.next, struct hrtimer, member))

#endif

static void* hrtimer_thread(void* arg)
{
    struct hrtimer_ctrl *ctrl = (struct hrtimer_ctrl *)arg;
    int empty = 1, restart, do_del;
    struct hrtimer *timer;
    uint32_t    now;

    sem_post(&ctrl->thread_ready);

do_work:
    empty = list_empty(&ctrl->list);
    if (empty)
        sem_wait(&ctrl->sem);

    spin_lock(ctrl->lock);

    do_del = 0;
    now = jiffies;

    list_for_each_entry(timer, &ctrl->list, list) {
        if (timer) {
            if (timer->state == HRTIMER_STATE_ENQUEUED) {
                if (((timer->start_time < now) && (timer->expires <= now) && (timer->start_time < timer->expires)) ||
                    ((timer->start_time > now) && ((timer->start_time - now) >= HR_TIMER_MAX_TIMEOUT_VAL) &&
                    ((timer->expires > timer->start_time) ||
                    ((timer->expires < timer->start_time) && (now >= timer->expires))))) 
                {
                    if (timer->function) {
                        timer->state = HRTIMER_STATE_INACTIVE;
                        //hrt_dbg(" %p **!\n", timer);
                        hrt_dbg("(0x%08X) %p **! (0x%08X, 0x%08X)\n", now, timer, timer->start_time, timer->interval);
                        restart = timer->function(timer);
                        if (restart == HRTIMER_RESTART)
                            timer->state = HRTIMER_STATE_ENQUEUED;
                        else
                            do_del = 1;
                    }
                    else {
                        hrt_err("timer %p NULL function! \n", timer);
                        continue;
                    }
                }
            }
        }
    }

    // delete after one round
    if (do_del) {
        struct hrtimer *to_del;

        for (timer = list_entry((&ctrl->list)->next, struct hrtimer, list);
            &timer->list != (&ctrl->list); 
            ) {
            if (timer->state == HRTIMER_STATE_INACTIVE) {
                //hrt_dbg(" %p -- after **! \n", timer);
                hrt_dbg("(0x%08X) %p -- after **! (0x%08X, 0x%08X)\n", now, timer, timer->start_time, timer->interval);
                to_del = timer;
                timer = list_entry(timer->list.next, struct hrtimer, list);
                list_del(&to_del->list);
            }
            else
                timer = list_entry(timer->list.next, struct hrtimer, list);
        }
    }

    spin_unlock(ctrl->lock);

    usleep(1000);

    goto do_work;
}

static int ite_init_hrtimer(void)
{
    int res;
    pthread_attr_t attr;
    struct sched_param param;

    hrtimer_c = &g_hrtimer_ctrl;
    INIT_LIST_HEAD(&hrtimer_c->list);
    sem_init(&hrtimer_c->sem, 0, 0);
    sem_init(&hrtimer_c->thread_ready, 0, 0);
    spin_lock_init(&hrtimer_c->lock);

    pthread_attr_init(&attr);
    param.sched_priority = 4;
    pthread_attr_setschedparam(&attr, &param);
    res = pthread_create(&hrtimer_c->thread, &attr, hrtimer_thread, (void*)hrtimer_c);
    if (res) {
        hrt_err("create hrtimer_thread fail! \n\n");
        goto err;
    }
    sem_wait(&hrtimer_c->thread_ready);
    sem_destroy(&hrtimer_c->thread_ready);

    hrt_dbg("hrtimer_thread created ready! \n");

err:
    return res;
}


void ite_hrtimer_init(struct hrtimer *timer, int clock_id, enum hrtimer_mode mode)
{
    if (!hrtimer_c) {
        if (ite_init_hrtimer())
            hrt_err("ite_init_hrtimer() fail! \n");
    }

    memset(timer, 0, sizeof(struct hrtimer));
    timer->mode = mode;
    INIT_LIST_HEAD(&timer->list);
}

/**
* hrtimer_start - (re)start an hrtimer
* @timer:	the timer to be added
* @tim:	expiry time   (ns)
* @mode:	timer mode: absolute (HRTIMER_MODE_ABS) or
*		relative (HRTIMER_MODE_REL)
*/
void ite_hrtimer_start(struct hrtimer *timer, uint32_t tim,
    const enum hrtimer_mode mode)
{
    int empty;

    spin_lock(&hrtimer_c->lock);

    /* Remove an active timer from the queue: */
    if (timer->state == HRTIMER_STATE_ENQUEUED) {
        list_del(&timer->list);
        //hrt_dbg(" %p --! t:%d\n", timer, timer->expires);
        hrt_dbg("(0x%08X) %p --! t:%d (0x%08X, 0x%08X)\n", jiffies, timer, timer->expires, timer->start_time, timer->interval);
    }

    if (mode & HRTIMER_MODE_REL) {
        timer->start_time = jiffies;
        timer->interval = tim / 1000;
        tim = timer->start_time + tim / 1000;
    }
    else {
        timer->start_time = jiffies;
        timer->interval = tim / 1000 - timer->start_time;
        tim = tim / 1000;
    }

    empty = list_empty(&hrtimer_c->list);

    timer->expires = tim;
    timer->state = HRTIMER_STATE_ENQUEUED;
    list_add_tail(&timer->list, &hrtimer_c->list);
    //hrt_dbg(" %p ++! t:%d\n", timer, timer->expires);
    hrt_dbg("(0x%08X) %p(0x%X) ++! t:%X\n", timer->start_time, timer, timer->interval, timer->expires);

    spin_unlock(&hrtimer_c->lock);

    if (empty)
        sem_post(&hrtimer_c->sem);
}

/**
* hrtimer_try_to_cancel - try to deactivate a timer
* @timer:	hrtimer to stop
*
* Returns:
*
*  *  0 when the timer was not active
*  *  1 when the timer was active
*/
int ite_hrtimer_try_to_cancel(struct hrtimer *timer)
{
    if (timer->state == HRTIMER_STATE_INACTIVE)
        return 0;

    spin_lock(&hrtimer_c->lock);

    timer->state = HRTIMER_STATE_INACTIVE;
    list_del(&timer->list);
    //hrt_dbg(" %p --\n", timer);
    hrt_dbg("(0x%08X) %p --\n", jiffies, timer);

    spin_unlock(&hrtimer_c->lock);

    return 1;
}

/**
* hrtimer_cancel - cancel a timer and wait for the handler to finish.
* @timer:	the timer to be cancelled
*
* Returns:
*  0 when the timer was not active
*  1 when the timer was active
*/
int ite_hrtimer_cancel(struct hrtimer *timer)
{
    return hrtimer_try_to_cancel(timer);
}

