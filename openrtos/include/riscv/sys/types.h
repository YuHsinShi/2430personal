#ifndef ITP_RISCV_SYS_TYPES_H
#define ITP_RISCV_SYS_TYPES_H

#include_next <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __machine_fsblkcnt_t_defined
typedef __uint64_t fsblkcnt_t;
#endif

#ifndef __machine_fsfilcnt_t_defined
typedef __uint32_t fsfilcnt_t;
#endif    

#ifndef __machine_id_t_defined
typedef __uint32_t id_t;
#endif

#ifdef __cplusplus
}
#endif

#endif // ITP_RISCV_SYS_TYPES_H
