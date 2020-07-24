#ifndef   ITP_SYS_SIGNAL_H
#define   ITP_SYS_SIGNAL_H

#include_next <sys/signal.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __BSD_VISIBLE

#define SA_RESTART   0x10000000 	/* Restart syscall on signal return */

#endif // __BSD_VISIBLE

#ifdef __cplusplus
}
#endif

#endif //   ITP_SYS_SIGNAL_H
