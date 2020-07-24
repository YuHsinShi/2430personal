#include <stdio.h>
#include <pthread.h>
#include <linux/os.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include "ite/itp.h"

#ifndef UINT_MAX
#define UINT_MAX	    (~0U)
#endif

#define WQ_DBG     0
#define WQ_ERR     1

#if defined(_WIN32)

#if WQ_DBG
#define wq_dbg(string, ...)    do { ithPrintf("[WQ][DBG] "); ithPrintf(string, __VA_ARGS__); } while (0)
#else
#define wq_dbg(string, ...)
#endif

#if WQ_ERR
#define wq_err(string, ...)    do { ithPrintf("[WQ][ERR] "); ithPrintf(string, __VA_ARGS__); } while (0)
#else
#define wq_err(string, ...)
#endif

#else // #if defined(_WIN32)

#if WQ_DBG
#define wq_dbg(string, args...)    do { ithPrintf("[WQ][DBG] "); ithPrintf(string, ## args); } while (0)
#else
#define wq_dbg(string, args...)
#endif

#if WQ_ERR
#define wq_err(string, args...)    do { ithPrintf("[WQ][ERR] "); ithPrintf(string, ## args); } while (0)
#else
#define wq_err(string, args...)
#endif

#endif // #if defined(_WIN32)


static LIST_HEAD(workqueue_head);
static struct workqueue_struct  *system_wq = NULL;
static pthread_mutex_t  wq_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* __wq_thread(void* arg)
{
    struct workqueue_struct *wq = (struct workqueue_struct *)arg;
    struct work_struct *work;
    int empty;

    atomic_set(&wq->state, WQ_STATE_RUNNING);
    sem_post(&wq->thread_complete);

do_work:
    spin_lock(&wq->lock);
    empty = list_empty(&wq->works);
    spin_unlock(&wq->lock);

    if (empty)
        sem_wait(&wq->sem);

    wq_dbg("wakeup wq-%s state %d, last:%d, next:%d, wait:%d \n",
        wq->name, wq->state, wq->last_work_id, wq->next_work_id, wq->waiting_work_id);

    spin_lock(&wq->lock);
    work = list_first_entry_or_null(&wq->works, struct work_struct, list);
    if (work) {
        list_del(&work->list);
        work->wq = NULL;
        wq->last_work_id = work->id;
    }
    spin_unlock(&wq->lock);

    if (work) {
        atomic_set(&work->state, WORK_STATE_RUNNING);
        wq_dbg("work-%d +\n", wq->last_work_id);
        work->func(work);
        wq_dbg("work-%d -\n", wq->last_work_id);
        atomic_set(&work->state, WORK_STATE_IDLE);

        if (work->wait_done)
            sem_post(&work->wait_done_sem);
    }

    goto do_work;
}

static struct workqueue_struct  *ite_init_wq(char *name)
{
    int res;
    struct workqueue_struct *wq;
    pthread_attr_t attr;

    wq = kzalloc(sizeof(*wq), 0);
    if (!wq)
        goto err;

    wq->name = name;
    atomic_set(&wq->state, WQ_STATE_INIT);
    INIT_LIST_HEAD(&wq->list);
    INIT_LIST_HEAD(&wq->works);
    spin_lock_init(&wq->lock);
    sem_init(&wq->sem, 0, 0);
    sem_init(&wq->thread_complete, 0, 0);
    wq->last_work_id = 0;
    wq->next_work_id = wq->last_work_id + 1;
    wq->waiting_work_id = UINT_MAX;

    pthread_attr_init(&attr);
    res = pthread_create(&wq->thread, &attr, __wq_thread, (void*)wq);
    if (res) {
        wq_err("create wq: %s fail! \n\n", name);
        goto err;
    }
    sem_wait(&wq->thread_complete);
    sem_destroy(&wq->thread_complete);

    wq_dbg("wq-%s created success \n", wq->name);

    list_add_tail(&wq->list, &workqueue_head);

	return wq;

err:
    if (wq)
        free(wq);
    return NULL;
}

static bool ite_queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
    int empty;

    spin_lock(&wq->lock);

    if (atomic_read(&work->state) == WORK_STATE_PENDING) {
        wq_dbg("work %p already in wq-%s \n", work, wq->name);
        return false;
    }

    if (atomic_read(&wq->state) == WQ_STATE_WAIT_FLUSH)
        wq_dbg("wq-%s: is flushing, next id: %d \n", wq->name, wq->next_work_id);

    work->id = wq->next_work_id++;
    work->wq = wq;
    atomic_set(&work->state, WORK_STATE_PENDING);

    empty = list_empty(&wq->works);
    list_add_tail(&work->list, &wq->works);
    wq_dbg("insert work %p id %d to %s, func %p \n", work, work->id, wq->name, work->func);

    spin_unlock(&wq->lock);

    if (empty)
        sem_post(&wq->sem);
    
    return true;
}

/**
* schedule_work - put work task in global workqueue
* @work: job to be done
*
* Returns %false if @work was already on the kernel-global workqueue and
* %true otherwise.
*/
bool ite_schedule_work(struct work_struct *work)
{
    int ret = 0;

    pthread_mutex_lock(&wq_mutex);
    if (!system_wq) {
        system_wq = ite_init_wq("system_wq");
        if (!system_wq) {
            printf("wq: create system workqueue fail!\n");
			pthread_mutex_unlock(&wq_mutex);
            return false;
        }
    }
    pthread_mutex_unlock(&wq_mutex);

    return ite_queue_work(system_wq, work);
}

static void __work_wait_done(struct work_struct *work)
{
    wq_dbg("wait work %p done \n", work);

    work->wait_done = true;
    sem_init(&work->wait_done_sem, 0, 0);
    spin_unlock(&work->wq->lock);

    sem_wait(&work->wait_done_sem);

    work->wait_done = false;
    sem_destroy(&work->wait_done_sem);

    wq_dbg("work %p done!\n", work);
}

bool ite_work_remove(struct work_struct *work, int synced)
{
    bool ret = false;
    int state;

    struct workqueue_struct *wq = work->wq;

    spin_lock(&wq->lock);
    state = atomic_read(&work->state);

    wq_dbg("rmove work %p id %d from wq-%s, state %d synced %d \n",
        work, work->id, wq->name, state, synced);

    if (state == WORK_STATE_PENDING) {
        if (synced == WORK_SYNC_RUN_DONE) {
            list_del(&work->list);
            wq_dbg("work %p was deleted from wq-%s. \n", work, wq->name);
            spin_unlock(&wq->lock);
            ret = true;
        }
        else if (WORK_SYNC_PEND_DONE){
            __work_wait_done(work);
            ret = true;
        }
    }
    else if (state == WORK_STATE_RUNNING) {
        if (synced == WORK_SYNC_RUN_DONE) {
            __work_wait_done(work);
            ret = false;
        }
        else if (synced == WORK_SYNC_PEND_DONE){
            __work_wait_done(work);
            ret = true;
        }
    }
    else {
        spin_unlock(&wq->lock);

        if (state == WORK_STATE_IDLE)
            ret = false;   /* current work is already idle */
        else
            wq_err("%s(%d) : error state %d \n", __func__, __LINE__, state);
    }

    return ret;
}









