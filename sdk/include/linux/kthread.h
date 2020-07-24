#ifndef KTHREAD_H
#define KTHREAD_H

#include <pthread.h>
#include <linux/os.h>
#include "ite/itp.h"


struct task_struct {
    pthread_t   task;
    sem_t       sem;
    volatile uint32_t    stop;
	void *context;
	int (*threadfn)(void*);
};

static inline int wake_up_process(struct task_struct *task)
{
    if (task)
        sem_post(&task->sem);

	return 0;
}

static void *kthread_func(void *arg)
{
    struct task_struct *task = (struct task_struct *)arg;

	sem_wait(&task->sem);
	task->threadfn(task->context);
	return NULL;
}

static inline struct task_struct *kthread_create(int (*threadfn)(void*), void *data, char *name)
{
    struct task_struct *task;
    pthread_attr_t attr;
	struct sched_param param;

    task = kzalloc(sizeof(struct task_struct), 0);
    sem_init(&task->sem, 0, 0);
	task->context = data;
	task->threadfn = threadfn;

    pthread_attr_init(&attr);
	param.sched_priority = 2;
	pthread_attr_setschedparam(&attr, &param);
    pthread_create(&task->task, &attr, kthread_func, task);

    return task;
}

/** 
 * kthread_run - create and wake a thread.
 *
 * Description: Convenient wrapper for kthread_create() followed by
 * wake_up_process().  Returns the kthread or -1.
 */
static inline struct task_struct *kthread_run(int (*threadfn)(void*), void *data)
{
    struct task_struct *task = kthread_create(threadfn, data, NULL);
    if (task)
        wake_up_process(task);

    return task;
}

static inline bool kthread_should_stop(struct task_struct *task)
{
    if (task)
        return task->stop;
    else
        return 0;
}

static inline int schedule_timeout(struct task_struct *task, int timeout)
{
    if (task)
        return itpSemWaitTimeout(&task->sem, timeout);
    else {
        usleep(timeout * 1000);
        return 0;
    }
}

static inline int schedule(struct task_struct *task)
{
    if (task)
        return sem_wait(&task->sem);
	else
		usleep(1000);
}

static inline int kthread_stop(struct task_struct *task)
{
    task->stop = 1;
    wake_up_process(task);
    pthread_join(task->task, NULL);

    sem_destroy(&task->sem);
    kfree(task);

    return 0;
}

//=========== Importment !!!! ================
//   MUST CALL kthread_stop() to avoid memory leak!!!
//=========================================
#if defined(_WIN32)
#define kthread_creat(fn, data, fmt, ...)   kthread_creat(fn, data)
#define kthread_run(threadfn, data, namefmt, ...) kthread_run(threadfn, data)
#else
#define kthread_creat(fn, data, fmt, arg...)   kthread_creat(fn, (void*)data)
#define kthread_run(threadfn, data, namefmt, arg...) kthread_run(threadfn, (void*)data)
#endif



#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2

#define set_current_state(x)


#endif // KTHREAD_H
