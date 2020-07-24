#ifndef ITP_SYS_SELECT_H
#define ITP_SYS_SELECT_H

#ifdef __BSD_VISIBLE

#include_next <sys/select.h>

#else

#include <sys/types.h>
#include <sys/time.h>

#endif // __BSD_VISIBLE

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __BSD_VISIBLE

extern int itpSocketSelect(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
                struct timeval *timeout);

#undef select
#define select(a,b,c,d,e)     itpSocketSelect(a,b,c,d,e)

#endif // !__BSD_VISIBLE

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_SELECT_H
