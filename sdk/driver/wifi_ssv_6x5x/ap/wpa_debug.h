/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef WPA_DEBUG_H
#define WPA_DEBUG_H

//#include "wpabuf.h"
#include "log.h"
#include "stdarg.h"



/* Debugging function - conditional printf and hex dump. Driver wrappers can
 * use these for debugging purposes. */

//enum {
//	MSG_EXCESSIVE, MSG_MSGDUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR
//};




#define MSG_EXCESSIVE 	LOG_LEVEL_ALL
#define MSG_MSGDUMP 	LOG_LEVEL_ALL
#define MSG_DEBUG 		LOG_LEVEL_ALL
#define MSG_INFO 		LOG_LEVEL_ALL
#define MSG_WARNING 	LOG_LEVEL_WARNING
#define MSG_ERROR		LOG_LEVEL_SERIOUS				




#define wpa_printf(l,fmt,...)  LOG_DEBUGF(LOG_L2_AP,(fmt,##__VA_ARGS__))//do { } while (0)


/**
 * wpa_hexdump - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump.
 */
void wpa_hexdump(ssv_type_u32 level, const char *title, const void *buf, ssv_type_size_t len);

//static inline void wpa_hexdump_buf(int level, const char *title,
//				   const struct wpabuf *buf)
//{
//	wpa_hexdump(level, title, buf ? wpabuf_head(buf) : NULL,
//		    buf ? wpabuf_len(buf) : 0);
//}

/**
 * wpa_hexdump_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump. This works
 * like wpa_hexdump(), but by default, does not include secret keys (passwords,
 * etc.) in debug output.
 */
void wpa_hexdump_key(ssv_type_u32 level, const char *title, const void *buf, ssv_type_size_t len);

//static inline void wpa_hexdump_buf_key(int level, const char *title,
//				       const struct wpabuf *buf)
//{
//	wpa_hexdump_key(level, title, buf ? wpabuf_head(buf) : NULL,
//			buf ? wpabuf_len(buf) : 0);
//}

/**
 * wpa_hexdump_ascii - conditional hex dump
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown.
 */
void wpa_hexdump_ascii(ssv_type_u32 level, const char *title, const void *buf,
		       ssv_type_size_t len);

/**
 * wpa_hexdump_ascii_key - conditional hex dump, hide keys
 * @level: priority level (MSG_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown. This works like wpa_hexdump_ascii(), but by
 * default, does not include secret keys (passwords, etc.) in debug output.
 */
void wpa_hexdump_ascii_key(ssv_type_u32 level, const char *title, const void *buf,
			   ssv_type_size_t len);


#endif /* WPA_DEBUG_H */
