/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#include "includes.h"

#include <stdarg.h>
#include <log.h>
#include <host_config.h>
#include "wpa_debug.h"
#include <ssv_lib.h>

#if (AP_MODE_ENABLE == 1)

#ifndef CONFIG_NO_WPA2



/**
 * wpa_printf - conditional printf
 * @level: priority level (MSG_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
 
#if SSV_LOG_DEBUG
extern ssv_type_u32 g_log_module;
extern ssv_type_u32 g_log_min_level;
#endif
static void _wpa_hexdump(ssv_type_u32 level, const char *title, const ssv_type_u8 *buf,
			 ssv_type_size_t len, int show)
{
    
#if SSV_LOG_DEBUG
	ssv_type_size_t i;

	if(level < g_log_min_level)
		return;

	if(!(LOG_L2_AP&g_log_module))
		return;


	hal_print("%s - hexdump(len=%lu):", title, (unsigned long) len);
	if (buf == NULL) {
		hal_print(" [NULL]");
	} else if (show) {
		for (i = 0; i < len; i++)
			hal_print(" %02x", buf[i]);
	} else {
		hal_print(" [REMOVED]");
	}
	hal_print("\n\r");
#endif    

}

void wpa_hexdump(ssv_type_u32 level, const char *title, const void *buf, ssv_type_size_t len)
{
	_wpa_hexdump(level, title, buf, len, 1);
}


void wpa_hexdump_key(ssv_type_u32 level, const char *title, const void *buf, ssv_type_size_t len)
{
	_wpa_hexdump(level, title, buf, len, 1);
}


static void _wpa_hexdump_ascii(ssv_type_u32 level, const char *title, const void *buf,
			       ssv_type_size_t len, int show)
{
#if SSV_LOG_DEBUG
	ssv_type_size_t i, llen;
	const ssv_type_u8 *pos = buf;
#define LINE_LEN	16

	if(level < g_log_min_level)
		return;

	if(!(LOG_L2_AP&g_log_module))
		return;


	if (!show) {
		hal_print("%s - hexdump_ascii(len=%lu): [REMOVED]\n",
		       title, (unsigned long) len);
		return;
	}
	if (buf == NULL) {
		hal_print("%s - hexdump_ascii(len=%lu): [NULL]\n",
		       title, (unsigned long) len);
		return;
	}
	hal_print("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
	while (len) {
		llen = len > LINE_LEN ? LINE_LEN : len;
		hal_print("    ");
		for (i = 0; i < llen; i++)
			hal_print(" %02x", pos[i]);
		for (i = llen; i < LINE_LEN; i++)
			hal_print("   ");
		hal_print("   ");
		for (i = 0; i < llen; i++) {
			if (ssv_isprint(pos[i]))
				hal_print("%c", pos[i]);
			else
				hal_print("_");
		}
		for (i = llen; i < LINE_LEN; i++)
			hal_print(" ");
		hal_print("\n");
		pos += llen;
		len -= llen;
	}
#endif
}


void wpa_hexdump_ascii(ssv_type_u32 level, const char *title, const void *buf,
		       ssv_type_size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, 1);
}


void wpa_hexdump_ascii_key(ssv_type_u32 level, const char *title, const void *buf,
			   ssv_type_size_t len)
{
	_wpa_hexdump_ascii(level, title, buf, len, 1);
}

#endif//CONFIG_NO_WPA2
#endif
