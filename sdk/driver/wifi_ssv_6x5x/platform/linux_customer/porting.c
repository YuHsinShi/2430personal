/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <rtos.h>
#include <porting.h>
#include <ssv_lib.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/os.h>
#define APP_INCLUDE_WIFI_FW

/*=============console==================*/
#define PRINT_BUF_SIZE                  512
static CHAR print_buf[PRINT_BUF_SIZE];

int ssv_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int n = vprintf(format, args);
	va_end(args);
	return n;
}

/*void hal_print(CHAR *fmt, ...)
{
    va_list v_list;
    ssv_type_s32 ret;

	{
		CHAR *pt;
	    va_start(v_list, fmt);
	    ret = vsnprintf(print_buf, PRINT_BUF_SIZE, fmt, v_list);
		if(ret < 0)
		{
			return;
		}
	    //vsprintf(print_buf, fmt, v_list);
	    va_end(v_list);

	    print_buf[PRINT_BUF_SIZE-1] = 0;
	    pt = print_buf;
	    while (*pt) {
            drv_uart_data_send(*pt);
			pt++;
		}
	}
}*/

void hal_putchar(ssv_type_u8 ch)
{
    //Not implement
    //drv_uart_data_send(ch);
}

ssv_type_u8 hal_getchar(void)
{
    return 0;
    /*ssv_type_u8 data=0;

    if(STATUS_OK == drv_uart_data_get(&data))
        return data;
    else
        return 0;*/

}

/*=============Memory==================*/

OS_APIs void *OS_MemAlloc( ssv_type_u32 size )
{
    void *ptr = NULL;
	ptr = kzalloc(size, GFP_KERNEL);
	return ptr;
}


/**
 *  We do not recommend using OS_MemFree() API
 *  because we do not want to support memory
 *  management mechanism in embedded system.
 */
OS_APIs void __OS_MemFree( void *m )
{
    /**
        *  Platform depedent code. Please rewrite
        *  this piece of code for different system.
        */
    kfree(m);
}

void OS_MemCPY(void *pdest, const void *psrc, ssv_type_u32 size)
{
    memcpy(pdest, psrc, size);
}

void OS_MemSET(void *pdest, ssv_type_u8 byte, ssv_type_u32 size)
{
    memset(pdest, byte, size);
}

//=====================Platform LDO EN ping setting=======================

void platform_ldo_en_pin_init(void)
{

}

void platform_ldo_en(ssv_type_bool en)
{

}

//=====================find fw to download=======================
#if 1
#include <firmware/ssv6200_uart_bin.h>
ssv_type_bool platform_download_firmware(void)
{
    //LOG_PRINTF("bin size =%d\r\n",sizeof(ssv6200_uart_bin));
    return ssv6xxx_download_fw((ssv_type_u8 *)ssv6200_uart_bin,sizeof(ssv6200_uart_bin));

#else  //this option is to set fw.bin as resource to open. please check resource_wifi_fw.s
void platform_download_firmware(void)
{
    u32 fw_size;
    extern u8 *RES_WIFI_FW_START;
    extern u8 *RES_WIFI_END;

    fw_size = ((u32)&RES_WIFI_END) - ((u32)&RES_WIFI_FW_START);
    LOG_PRINTF("fw_size=%d\r\n",fw_size);
    ssv6xxx_download_fw((u8 *)&RES_WIFI_FW_START,fw_size);//??? u8* bin
#endif
}
void platform_read_firmware(void *d,void *s,ssv_type_u32 len)
{
    OS_MemCPY(d,s,len);
}

void platform_udelay(ssv_type_u32 us_delay)
{
    udelay(us_delay);
}

