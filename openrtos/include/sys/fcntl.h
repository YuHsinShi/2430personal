#ifndef ITP_SYS_FCNTL_H
#define ITP_SYS_FCNTL_H

#include_next <sys/fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __BSD_VISIBLE

extern int itpSocketFcntl(int fd, int cmd, int arg);

#undef fcntl
#define fcntl itpSocketFcntl

#endif // !__BSD_VISIBLE

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_FCNTL_H
