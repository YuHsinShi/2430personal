#ifndef _WORKQUEUE_H_
#define _WORKQUEUE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <pthread.h>
#include <semaphore.h>

struct workqueue_struct {
    struct list_head    list;
    char *name;
    atomic_t            state;
#define WQ_STATE_INIT           0
#define WQ_STATE_RUNNING        1
#define WQ_STATE_WAIT_FLUSH     2
#define WQ_STATE_WAIT_DESTORY   3

    struct list_head    works;
    pthread_t           thread;
    spinlock_t          lock;
    sem_t               sem;
    sem_t               thread_complete;

    unsigned int next_work_id;
    unsigned int last_work_id;
    unsigned int waiting_work_id;
};


struct work_struct;
typedef void(*work_func_t)(struct work_struct *work);

struct work_struct {
    struct list_head list;
    work_func_t func;
    atomic_t    state;
#define WORK_STATE_IDLE             0
#define WORK_STATE_WAIT_DELAY       1
#define WORK_STATE_PENDING          2
#define WORK_STATE_RUNNING          3

    bool    wait_done;
    sem_t   wait_done_sem;
    unsigned int    id;
    struct workqueue_struct *wq;
};

#define WORK_SYNC_NONE              0  /* don't wait */
#define WORK_SYNC_RUN_DONE          1  /* wait current work done if running */
#define WORK_SYNC_PEND_DONE         2  /* wait current work done even if pending */


bool ite_work_remove(struct work_struct *work, int synced);
bool ite_schedule_work(struct work_struct *work);


#define INIT_WORK(_work, _func)						\
    do {    \
        INIT_LIST_HEAD(&(_work)->list);			\
        (_work)->func = (_func);				\
    } while (0)


#define schedule_work           ite_schedule_work
/**
* cancel_work_sync - cancel a work and wait for it to finish
* @work: the work to cancel
*
* Return:
* %true if @work was pending, %false otherwise.
*/
#define cancel_work_sync(work)  ite_work_remove(work, WORK_SYNC_RUN_DONE)
/**
* flush_work - wait for a work to finish executing the last queueing instance
* @work: the work to flush
*
* Return:
* %true if flush_work() waited for the work to finish execution,
* %false if it was already idle.
*/
#define flush_work(work)        ite_work_remove(work, WORK_SYNC_PEND_DONE)


#ifdef __cplusplus
}
#endif


#endif // _WORKQUEUE_H_