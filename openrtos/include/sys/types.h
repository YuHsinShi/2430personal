#ifndef ITP_SYS_TYPES_H
#define ITP_SYS_TYPES_H

#include_next <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __BSD_VISIBLE

#if ___int8_t_defined
typedef __uint8_t	u_int8_t;
#endif
#if ___int16_t_defined
typedef __uint16_t	u_int16_t;
#endif 
#if ___int32_t_defined
typedef __uint32_t	u_int32_t;
#endif
#if ___int64_t_defined
typedef __uint64_t	u_int64_t;
#endif

typedef unsigned long id_t;

#endif // __BSD_VISIBLE

#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_TYPES_H
