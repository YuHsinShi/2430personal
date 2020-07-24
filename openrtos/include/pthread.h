#ifndef ITP_PTHREAD_H
#define ITP_PTHREAD_H

#include_next<pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern int itpPthreadCreate(pthread_t* pthread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg, const char* name);

#ifndef pthread_create
#define pthread_create(pthread, attr, start_routine, arg) itpPthreadCreate((pthread), (attr), (start_routine), (arg), #start_routine)
#endif

pthread_t pthread_self(void);
int pthread_getschedparam(pthread_t pthread, int* policy, struct sched_param* param);
int pthread_setschedparam(pthread_t pthread, int policy, struct sched_param* param);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param);

#ifdef __cplusplus
}
#endif

#endif // ITP_PTHREAD_H
