/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _OS_PORTING_H_
#define _OS_PORTING_H_

#include <ssv_types.h>
#include <stdio.h>
#include <assert.h>
//#include <UCOS_II.H>

#define ssv_inline  inline
#define CHAR        char

/*============Task Priority===================*/
#define RX_ISR_PRIORITY    0 //No use for uCOS2
#define TMR_TASK_PRIORITY  18
#define MLME_TASK_PRIORITY 19
#define CMD_ENG_PRIORITY   20
#define WIFI_RX_PRIORITY   21
#define WIFI_TX_PRIORITY   22
#define TCPIP_PRIORITY     23
#define DHCPD_PRIORITY     24
#define NETAPP_PRIORITY    25
#define DEV_INIT_STACK_SIZE 64

#define NETAPP_PRIORITY_1  NETAPP_PRIORITY+1
#define NETAPP_PRIORITY_2  NETAPP_PRIORITY+2
#define NETAPP_PRIORITY_3  NETAPP_PRIORITY+3
#define NETAPP_PRIORITY_4  NETAPP_PRIORITY+4
#define NETMGR_PRIORITY    NETAPP_PRIORITY+5
#define TASK_END_PRIO      NETAPP_PRIORITY+6  //31

/*============Console setting===================*/
//#define PRINTF print_string
//#define stdout NULL
#define FFLUSH(x)
//extern void hal_print(CHAR *fmt, ...);
extern void hal_putchar(ssv_type_u8 ch);
extern ssv_type_u8 hal_getchar(void);

int ssv_printf(const char *format, ...);
#define hal_print       ssv_printf
#define ASSERT(expr)    assert(expr)
#ifndef udelay
#define udelay(t)       ((t/(portTICK_RATE_MS*1000))>0)?vTaskDelay(t/(portTICK_RATE_MS*1000)):(vTaskDelay(1))
#endif

/* ============Memory related============*/
#define GFP_DMA         0
#define GFP_ATOMIC    	1
#define GFP_KERNEL    	2

/*============Compiler setting===================*/
//#define ARM_ADS
/*#define STRUCT_PACKED __attribute__ ((packed))
#define UNION_PACKED __attribute__ ((packed))*/
//SSV PACK Definition
#define SSV_PACKED_STRUCT_BEGIN
#define SSV_PACKED_STRUCT
#define SSV_PACKED_STRUCT_END //__attribute__ ((packed))
#define SSV_PACKED_STRUCT_STRUCT  __attribute__ ((packed))
#define SSV_PACKED_STRUCT_FIELD

//LWIP PACK Definition
#define PACK_STRUCT_BEGIN   //#pragma pack(1)
#define PACK_STRUCT_END     //#pragma pack()
#if (CONFIG_USE_LWIP_PBUF==1)
#define PACK_STRUCT_STRUCT     __attribute__ ((packed))
#endif
#define PACK(__Declaration__) __Declaration__ PACK_STRUCT_STRUCT;
#define PACK_STRUCT_FIELD(x)    x
#define inline __inline
#define ALIGN_ARRAY(a) __attribute__ ((aligned(a)))
//SSV PACK Definition
#define SSV_PACKED_STRUCT_BEGIN
#define SSV_PACKED_STRUCT
#define SSV_PACKED_STRUCT_END //__attribute__ ((packed))
#define SSV_PACKED_STRUCT_STRUCT  __attribute__ ((packed))
#define SSV_PACKED_STRUCT_FIELD

/*============SSV-DRV setting===================*/
#define	CONFIG_RX_POLL      0
#define INTERFACE "sdio"

#define SDRV_INCLUDE_SPI    0
#define SDRV_INCLUDE_SDIO   1

#define SDRV_INCLUDE_SIM	0
#define SDRV_INCLUDE_UART	0
#define SDRV_INCLUDE_USB	0

#define SDIO_CARD_INT_TRIGGER
#define USE_LINUX_SKB       1

/*============Stack Size (unint: 16bytes)===================*/
#define TMR_TASK_STACK_SIZE  64
#define MLME_TASK_STACK_SIZE 0
#define CMD_ENG_STACK_SIZE   128
#define TCPIP_STACK_SIZE     128
#define DHCPD_STACK_SIZE     64

#define NETAPP1_STACK_SIZE    128
#define NETAPP2_STACK_SIZE    0
#define NETAPP3_STACK_SIZE    0
#define NETAPP4_STACK_SIZE    0
#define NETAPP5_STACK_SIZE    0

#define NETMGR_STACK_SIZE    128 //80

#define CLI_TASK_STACK_SIZE  64
#define RX_ISR_STACK_SIZE    0
#define WIFI_RX_STACK_SIZE   64
#define WIFI_TX_STACK_SIZE   64
#define PING_THREAD_STACK_SIZE 0 //16 , ping thread doesn't enable now, I set staic size is zero to reduce data size.

#define TOTAL_STACK_SIZE (TMR_TASK_STACK_SIZE+ \
                          MLME_TASK_STACK_SIZE+ \
                          CMD_ENG_STACK_SIZE+ \
                          TCPIP_STACK_SIZE+ \
                          DHCPD_STACK_SIZE+ \
                          NETAPP1_STACK_SIZE+ \
						  NETAPP2_STACK_SIZE+ \
						  NETAPP3_STACK_SIZE+ \
						  NETAPP4_STACK_SIZE+ \
						  NETAPP5_STACK_SIZE+ \
                          NETMGR_STACK_SIZE+ \
                          CLI_TASK_STACK_SIZE+ \
						  RX_ISR_STACK_SIZE+ \
						  WIFI_RX_STACK_SIZE+ \
                          WIFI_TX_STACK_SIZE+ \
                          PING_THREAD_STACK_SIZE)

/*============Memory========================*/
OS_APIs void *OS_MemAlloc( ssv_type_u32 size );
OS_APIs void __OS_MemFree( void *m );
OS_APIs void OS_MemSET(void *pdest, ssv_type_u8 byte, ssv_type_u32 size);
OS_APIs void OS_MemCPY(void *pdest, const void *psrc, ssv_type_u32 size);


/*============ Platform HW dependence ====================*/
void platform_dev_init(void);
void platform_read_firmware(void *d,void *s,ssv_type_u32 len);
void platform_ldo_en_pin_init(void);
void platform_ldo_en(ssv_type_bool en);
void platform_udelay(ssv_type_u32 us_delay);
void platform_read_firmware(void *d,void *s,ssv_type_u32 len);

#endif
