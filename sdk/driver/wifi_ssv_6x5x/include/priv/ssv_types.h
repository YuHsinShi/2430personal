/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_TYPES_H_
#define _SSV_TYPES_H_

#ifndef ssv_type_u8
#define ssv_type_u8 unsigned char
#endif

#ifndef ssv_type_u16
#define ssv_type_u16 unsigned short
#endif

#ifndef ssv_type_u32
#define ssv_type_u32 unsigned int
#endif

#ifndef ssv_type_u64
#define ssv_type_u64  unsigned long long
#endif

#ifndef ssv_type_s8
#define ssv_type_s8 signed char
#endif

#ifndef ssv_type_s16
#define ssv_type_s16 signed short
#endif

#ifndef ssv_type_s32
#define ssv_type_s32 signed int
#endif

#ifndef ssv_type_s32_t
#define ssv_type_s32_t signed long
#endif

#ifndef ssv_type_s64
#define ssv_type_s64  signed long long
#endif

#ifndef ssv_type_bool
#define ssv_type_bool   unsigned char
//#define bool            unsigned char
#endif

#ifndef ssv_type_size_t
#define ssv_type_size_t  unsigned int
#endif

typedef volatile unsigned int                SSV6XXX_REG;


typedef void (*TASK_FUNC)(void *);

#define __le16_ssv_type ssv_type_u16
#define __le32_ssv_type ssv_type_u32
#define __le64_ssv_type ssv_type_u64

#define __u8_ssv_type ssv_type_u8
#define __be16_ssv_type ssv_type_u16
#define __be32_ssv_type ssv_type_u32
#define __be64_ssv_type ssv_type_u64

#define le16_ssv_type ssv_type_u16
#define le32_ssv_type ssv_type_u32
#define le64_ssv_type ssv_type_u64

#define be16_ssv_type ssv_type_u16
#define be32_ssv_type ssv_type_u32
#define be64_ssv_type ssv_type_u64

#ifndef ssv_inline
#if defined(WIN32) && !defined(__cplusplus)
#define ssv_inline __inline
#endif
#endif

#define	ETHER_ADDR_LEN	6
#define	LLC_HEADER_LEN	6
#define	ETHER_TYPE_LEN	2
#define	ETHER_HDR_LEN	14
#define	ETHER_MAC_LEN	12		//802.3 DA+SA

#define OS_APIs
#define H_APIs
#define LIB_APIs
#define DRV_APIs

#ifndef NULL
#define NULL                        (void *)0
#endif

#ifndef true
#define true                        1
#endif

#ifndef false
#define false                       0
#endif

#ifndef TRUE
#define TRUE                        1
#endif

#ifndef FALSE
#define FALSE                       0
#endif

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif

#define SSV_ASSERT(x) \
{ \
    extern void ssv_halt (void); \
    if (!(x)) \
    { \
        LOG_PRINTF("Assert!! file: %s, function: %s, line: %d\n\t" #x, __FILE__, \
        	__FUNCTION__, __LINE__); \
        ssv_halt(); \
    } \
}

#define EMPTY

#define ASSERT_RET(x, ret) \
{ \
    extern void ssv_halt (void); \
    if (!(x)) \
    { \
        LOG_PRINTF("Assert!! file: %s, function: %s, line: %d\n\t" #x, __FILE__, \
        	__FUNCTION__, __LINE__); \
        ssv_halt(); \
        return ret; \
    } \
}



#ifndef ssv_assert
#define ssv_assert(x)                       SSV_ASSERT(x)
#endif

#define SSV_IS_EQUAL(a, b)                  ( (a) == (b) )

#ifdef SSV_SET_BIT
#undef  SSV_SET_BIT
#endif
#define SSV_SET_BIT(v, b)					( (v) |= (0x01<<b) )


#define SSV_CLEAR_BIT(v, b)			    	( (v) &= ~(0x01<<b) )
#define SSV_IS_BIT_SET(v, b)				( (v) & (0x01<<(b) ) )

#define ETH_ADDR_FORMAT                 "%02X:%02X:%02X:%02X:%02X:%02X"
#define ETH_ADDR(a)                     ((ETHER_ADDR *)(a))->addr[0], ((ETHER_ADDR *)(a))->addr[1], \
                                        ((ETHER_ADDR *)(a))->addr[2], ((ETHER_ADDR *)(a))->addr[3], \
                                        ((ETHER_ADDR *)(a))->addr[4], ((ETHER_ADDR *)(a))->addr[5]
#define KEY_32_FORMAT                   "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define KEY_16_FORMAT                   "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define KEY_8_FORMAT                    "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define KEY_32_VAL(m)                   (m)[0],(m)[1],(m)[2],(m)[3],(m)[4],(m)[5],(m)[6],(m)[7],(m)[8],(m)[9],(m)[10],(m)[11],(m)[12],(m)[13],(m)[14],(m)[15],(m)[16],(m)[18],(m)[18],(m)[19],(m)[20],(m)[21],(m)[22],(m)[23],(m)[24],(m)[25],(m)[26],(m)[27],(m)[28],(m)[29],(m)[30],(m)[31]
#define KEY_16_VAL(m)                   (m)[0],(m)[1],(m)[2],(m)[3],(m)[4],(m)[5],(m)[6],(m)[7],(m)[8],(m)[9],(m)[10],(m)[11],(m)[12],(m)[13],(m)[14],(m)[15]
#define KEY_8_VAL(m)                    (m)[0],(m)[1],(m)[2],(m)[3],(m)[4],(m)[5],(m)[6],(m)[7]


#ifndef BIT
#define BIT(nr)			(1UL << (nr))
#endif

#ifndef SIZE_1KB
#define		SIZE_1KB	(1024)
#endif

#ifndef SIZE_1MB
#define		SIZE_1MB	(1024 * SIZE_1KB)
#endif



#ifndef OFFSETOF
#define OFFSETOF(TYPE, MEMBER) ((ssv_type_size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#ifndef ARRAY_ELEM_SIZE
#define ARRAY_ELEM_SIZE(TYPE)   ((ssv_type_size_t)(&((TYPE *)100)[1]) - 100U)
#endif

typedef struct __timer_64_S {
    ssv_type_u32     lt;
    ssv_type_u32     ut;
} Time64_S;

typedef union {
        Time64_S   ts;
        ssv_type_u64        t;
} Time_T;

#endif /* _TYPES_H_ */

