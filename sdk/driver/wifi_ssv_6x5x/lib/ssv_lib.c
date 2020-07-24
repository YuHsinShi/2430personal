/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#define SSV_LIB_C

#include <hw/config.h>
#include <pbuf.h>
#include <stdarg.h>
#include <hdr80211.h>
#include <ssv_drv.h>
#include <log.h>
#include <ssv_common.h>
#if (AP_MODE_ENABLE == 1)
#include <ap_info.h>
#endif
#include <ssv_devinfo.h>
#include <ssv_hal.h>
#include <ssv_hal_if.h>
#include "hctrl.h"
#include "ssv_dev.h"
#include <core/recover.h>
#if (CONFIG_HOST_PLATFORM == 0)
#include <uart/drv_uart.h>
#endif

#include "ssv_lib.h"
#include <ssv_timer.h>
#include <efuse.h>
#include <ssv_frame.h>

#define CONSOLE_UART   SSV6XXX_UART0

// return  -1 : fail
//        >=0 : ok
extern struct Host_cfg g_host_cfg;
LIB_APIs ssv_type_size_t ssv6xxx_strlen(const char *s);

void llist_head_init(struct ssv_llist_head *lhd)
{
    lhd->last = NULL;
    lhd->list = NULL;
    lhd->llen = 0;
}

struct ssv_llist *llist_pop(struct ssv_llist_head *lhd)
{
    struct ssv_llist *elm = lhd->list;
    if (elm != NULL)
    {
        lhd->list = elm->next;
        elm->next = NULL;
        lhd->llen--;
        return elm;
    }
    else
        return NULL;

}

void llist_push(struct ssv_llist_head *lhd, struct ssv_llist *new)
{
    if(lhd->list == NULL)
        lhd->list = new;
    else
        lhd->last->next = new;

    new->next = NULL;
    lhd->last = new;
    lhd->llen++;
}

void llist_push_head(struct ssv_llist_head *lhd, struct ssv_llist *old)
{
    if(lhd->list == NULL)
        lhd->list = old;
    else
    {
        old->next = lhd->list;
        lhd->list = old;
    }

    lhd->llen++;
}

ssv_type_u32 llist_l_len(struct ssv_llist_head *lhd)
{
    return lhd->llen;
}

struct ssv_llist *llist_pop_safe(struct ssv_llist_head *lhd, OsMutex *pmtx)
{
    struct ssv_llist *_list = NULL;
    OS_MutexLock(*pmtx);
    _list = llist_pop(lhd);
    OS_MutexUnLock(*pmtx);
    return _list;
}

void llist_push_safe(struct ssv_llist_head *lhd, struct ssv_llist *new, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    llist_push(lhd, new);
    OS_MutexUnLock(*pmtx);
}

void llist_push_head_safe(struct ssv_llist_head *lhd, struct ssv_llist *old, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    llist_push_head(lhd, old);
    OS_MutexUnLock(*pmtx);
}

ssv_type_u32 llist_l_len_safe(struct ssv_llist_head *lhd, OsMutex *pmtx)
{
    ssv_type_u32 len = 0;
    OS_MutexLock(*pmtx);
    len = llist_l_len(lhd);
    OS_MutexUnLock(*pmtx);
    return len;
}

void list_q_init(struct ssv_list_q *qhd)
{
    qhd->prev = (struct ssv_list_q *)qhd;
    qhd->next = (struct ssv_list_q *)qhd;
    qhd->qlen = 0;
}

void list_q_qtail(struct ssv_list_q *qhd, struct ssv_list_q *newq)
{
    struct ssv_list_q *next = qhd;
    struct ssv_list_q *prev = qhd->prev;

    newq->next = next;
    newq->prev = prev;
    next->prev = newq;
    prev->next = newq;
    qhd->qlen++;
}

void list_q_insert(struct ssv_list_q *qhd, struct ssv_list_q *prev, struct ssv_list_q *newq)
{
    struct ssv_list_q *next = prev->next;

    newq->next = next;
    newq->prev = prev;
    next->prev = newq;
    prev->next = newq;
    qhd->qlen++;
}

void list_q_remove(struct ssv_list_q *qhd,struct ssv_list_q *curt)
{
    struct ssv_list_q *next = curt->next;
    struct ssv_list_q *prev = curt->prev;

    prev->next = next;
    next->prev = prev;
    qhd->qlen--;
}

struct ssv_list_q *list_q_deq(struct ssv_list_q *qhd)
{
    struct ssv_list_q *next, *prev;
    struct ssv_list_q *elm = qhd->next;

    if((qhd->qlen > 0) && (elm != NULL))
    {
        qhd->qlen--;
        next        = elm->next;
        prev        = elm->prev;
        elm->next   = NULL;
        elm->prev   = NULL;
        next->prev  = prev;
        prev->next  = next;

        return elm;
    }else{
        return NULL;
    }
}
unsigned int list_q_len(struct ssv_list_q *qhd)
{
    return qhd->qlen;
}

ssv_type_u32 list_q_len_safe(struct ssv_list_q *q, OsMutex *pmtx)
{
    ssv_type_u32 len = 0;
    OS_MutexLock(*pmtx);
    len = q->qlen;
    OS_MutexUnLock(*pmtx);
    return len;
}

void list_q_qtail_safe(struct ssv_list_q *qhd, struct ssv_list_q *newq, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_qtail(qhd, newq);
    OS_MutexUnLock(*pmtx);
}

struct ssv_list_q *list_q_deq_safe(struct ssv_list_q *qhd, OsMutex *pmtx)
{
    struct ssv_list_q *_list = NULL;
    OS_MutexLock(*pmtx);
    _list = list_q_deq(qhd);
    OS_MutexUnLock(*pmtx);
    return _list;
}

void list_q_insert_safe(struct ssv_list_q *qhd, struct ssv_list_q *prev, struct ssv_list_q *newq, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_insert(qhd, prev, newq);
    OS_MutexUnLock(*pmtx);
}

void list_q_remove_safe(struct ssv_list_q *qhd,struct ssv_list_q *curt, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_remove(qhd, curt);
    OS_MutexUnLock(*pmtx);
}

LIB_APIs ssv_type_s32 ssv6xxx_strrpos(const char *str, char delimiter)
{
	const char *p;

	for (p = (str + ssv6xxx_strlen(str)) - 1; (ssv_type_s32)p>=(ssv_type_s32)str; p--)
	{
		if (*p == delimiter)
			return ((ssv_type_s32)p - (ssv_type_s32)str);
	}

	return -1;	// find no matching delimiter

}

static ssv_inline int ssvislower(int c)
{
	return c >= 'a' && c <= 'z';
}

static ssv_inline int ssvisupper(int c)
{
	return c >= 'A' && c <= 'Z';
}

static ssv_inline int ssvisalpha(int c)
{
	return ssvislower(c) || ssvisupper(c);
}

#ifndef ULONG_MAX
#define LONG_MAX 0x7FFFFFFFL
#define LONG_MIN ((long) 0x80000000L)
#define ULONG_MAX 0xFFFFFFFFUL
#endif
unsigned long ssv_strtoul(const char *nptr, char **endptr, int base)
{
	const char *s;
	unsigned long acc, cutoff;
	int c;
	int neg, any, cutlim;
	/*
	 * See strtol for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = (unsigned char) *s++;
	} while (ssv_isspace(c));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X')) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	cutoff = ULONG_MAX / (unsigned long)base;
	cutlim = ULONG_MAX % (unsigned long)base;
	for (acc = 0, any = 0;; c = (unsigned char) *s++) {
		if (ssv_isdigit(c))
			c -= '0';
		else if (ssvisalpha(c))
			c -= ssvisupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0)
			continue;
		if (acc > cutoff || (acc == cutoff && c > cutlim)) {
			any = -1;
			acc = ULONG_MAX;
			//errno = ERANGE;
		} else {
			any = 1;
			acc *= (unsigned long)base;
			acc += c;
		}
	}
	if (neg && any > 0)
		acc = -acc;
	if (endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return (acc);
}

LIB_APIs ssv_type_s32 ssv6xxx_str_toupper(char *s)
{
	while (*s)
	{
		*s = ssv6xxx_toupper(*s);
		s++;
	}
	return 0;
}

LIB_APIs ssv_type_s32 ssv6xxx_str_tolower(char *s)
{
	while (*s)
	{
		*s = ssv6xxx_tolower(*s);
		s++;
	}
	return 0;
}

LIB_APIs ssv_type_u32 ssv6xxx_atoi_base( const char *s, ssv_type_u32 base )
{
    ssv_type_u32  idx, upbound=base-1;
    ssv_type_u32  value = 0, v;

    while( (v = (ssv_type_u8)*s) != 0 ) {
        idx = v - '0';
        if ( idx > 10 && base==16 ) {
            idx = (v >= 'a') ? (v - 'a') : (v - 'A');
            idx += 10;
        }
        if ( idx > upbound )
            break;
        value = value * base + idx;
        s++;
    }

    return value;
}

LIB_APIs ssv_type_s32 ssv6xxx_atoi( const char *s )
{
    ssv_type_u32 neg=0, value, base=10;

    if ( *s=='0' ) {
        switch (*++s) {
        case 'x':
        case 'X': base = 16; break;
        case 'b':
        case 'B': base = 2; break;
        default: return 0;
        }
        s++;
    }
    else if ( *s=='-' ) {
        neg = 1;
        s++;
    }

    value = ssv6xxx_atoi_base(s, base);

    if ( neg==1 )
        return -(ssv_type_s32)value;
    return (ssv_type_s32)value;

}


#if (CONFIG_HOST_PLATFORM == 1)
ssv_type_u64 ssv6xxx_64atoi( char *s )
{
    ssv_type_u8 bchar='A', idx, upbound=9;
    ssv_type_u32 neg=0, value=0, base=10;

    if ( *s=='0' ) {
        switch (*++s) {
                case 'x': bchar = 'a';
                case 'X': base = 16; upbound = 15; break;
                case 'b':
                case 'B': base = 2; upbound = 1; break;
                default: return 0;
        }
        s++;
    }
    else if ( *s=='-' ) {
        neg = 1;
        s++;
    }

    while( *s ) {
        idx = (ssv_type_u8)*s - '0';
        if ( base==16 && (*s>=bchar) && (*s<=(bchar+5)) )
        {
                idx = (ssv_type_u8)10 + (ssv_type_u8)*s - bchar;
        }
        if ( idx > upbound )
        {
                break;
        }
        value = value * base + idx;
        s++;
    }

    if ( neg==1 )
        return -(ssv_type_s32)value;
    return (ssv_type_u64)value;

}
#endif




LIB_APIs char ssv6xxx_toupper(char ch)
{
	if (('a' <= ch) && (ch <= 'z'))
		return ('A' + (ch - 'a'));

	// else, make the original ch unchanged
	return ch;
}

LIB_APIs char ssv6xxx_tolower(char ch)
{
	if (('A' <= ch) && (ch <= 'Z'))
		return ('a' + (ch - 'A'));

	// else, make the original ch unchanged
	return ch;
}

LIB_APIs ssv_type_s32 ssv6xxx_isupper(char ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

LIB_APIs ssv_type_s32 ssv6xxx_strcmp( const char *s0, const char *s1 )
{
    ssv_type_s32 c1, c2;

    do {
        c1 = (ssv_type_u8) *s0++;
        c2 = (ssv_type_u8) *s1++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

LIB_APIs ssv_type_s32 ssv6xxx_strncmp ( const char * s1, const char * s2, ssv_type_size_t n)
{
  if ( !n )
      return(0);

  while (--n && *s1 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return( *s1 - *s2 );
}

LIB_APIs char *ssv6xxx_strcat(char *s, const char *append)
{
    char *save = s;

    while (*s) { s++; }
    while ((*s++ = *append++) != 0) { }
    return(save);
}

LIB_APIs char *ssv6xxx_strncat(char *s, const char *append, ssv_type_size_t n)
{
     char* save = s;
     while(*s){ s++; }
     while((n--)&&((*s++ = *append++) != 0)){}
     *s='\0';
     return(save);
}

/*Not considering the case of memory overlap*/
LIB_APIs char *ssv6xxx_strcpy(char *dst, const char *src)
{
    char *ret = dst;
    ssv_assert(dst != NULL);
    ssv_assert(src != NULL);


    while((* dst++ = * src++) != '\0')
        ;
    return ret;
}

LIB_APIs char *ssv6xxx_strncpy(char *dst, const char *src, ssv_type_size_t n)
{
    register char *d = dst;
    register const char *s = src;

    if (n != 0) {
        do {
            if ((*d++ = *s++) == 0) {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dst);
}


LIB_APIs ssv_type_size_t ssv6xxx_strlen(const char *s)
{
    const char *ptr = s;
    while (*ptr) ptr++;
    return (ssv_type_size_t)ptr-(ssv_type_size_t)s;
}

LIB_APIs char * ssv6xxx_strchr(const char * s, char c)
{
    const char * p = s;
    while(*p != c && *p)
        p++;
    return (*p=='\0' ? NULL : (char *)p);
}

LIB_APIs char *ssv6xxx_strstr(const char *haystack, const char *needle)
{
    extern char *strstr(const char *haystack, const char *needle);
    return strstr(haystack, needle);
}

#if 0
size_t ssv_strspn(const char *s1, const char *s2)
{
    size_t ret=0;
    while(*s1 && ssv6xxx_strchr(s2,*s1++))
        ret++;
    LOG_PRINTF("%s,%d\r\n",__func__,ret);
    return ret;    
}

char *ssv_strtok(char * str, const char * delim)
{
    static char* p=0;
    if(str)
        p=str;
    else if(!p)
        return 0;
    str=p+ssv_strspn(p,delim);
    LOG_PRINTF("%s,str=%s\r\n",__func__,str);
    p=str+ssv_strspn(str,delim);
    LOG_PRINTF("%s,p=%s\r\n",__func__,p);
    if(p==str)
        return p=0;
    p = *p ? *p=0,p+1 : 0;
    return str;
}
#endif
LIB_APIs void *ssv6xxx_memset(void *s, ssv_type_s32 c, ssv_type_size_t n)
{
    if ( NULL != s ) {
		ssv_type_u8 * ps= (ssv_type_u8 *)s;
		const ssv_type_u8 * pes= ps+n;
        while( ps != pes )
			*(ps++) = (ssv_type_u8) c;
    }
    return s;
}


LIB_APIs void *ssv6xxx_memcpy(void *dest, const void *src, ssv_type_size_t n)
{
    ssv_type_u8 *d = dest;
    const ssv_type_u8 *s = src;

    while (n-- > 0)
      *d++ = *s++;
    return dest;
}


LIB_APIs ssv_type_s32 ssv6xxx_memcmp(const void *s1, const void *s2, ssv_type_size_t n)
{
    const ssv_type_u8 *u1 = (const ssv_type_u8 *)s1, *u2 = (const ssv_type_u8 *)s2;

    while (n--) {
        ssv_type_s32 d = *u1++ - *u2++;
        if (d != 0)
            return d;
    }
    /*
    for ( ; n-- ; s1++, s2++) {
        u1 = *(u8 *)s1;
        u2 = *(u8 *)s2;
        if (u1 != u2)
            return (u1-u2);
    } */
    return 0;
}

#if 0


//extern s32 gOsInFatal;
LIB_APIs void fatal_printf(const char *format, ...)
{

#if 0
   va_list args;


//   gOsInFatal = 1;
   /*lint -save -e530 */
   va_start( args, format );
   /*lint -restore */
   ret = print( 0, 0, format, args );
   va_end(args);
#endif
//    printf(format, ...);


}
#endif

#if 0
LIB_APIs void ssv6xxx_printf(const char *format, ...)
{
   va_list args;

   /*lint -save -e530 */
   va_start( args, format );
   /*lint -restore */

    printf(format, args);
//   ret = print( 0, 0, format, args );
   va_end(args);
}


LIB_APIs void ssv6xxx_snprintf(char *out, size_t size, const char *format, ...)
{
#if 0
    va_list args;
    s32     ret;
    /*lint -save -e530 */
    va_start( args, format ); /*lint -restore */
    ret = print( out, (out + size - 1), format, args );
    va_end(args);
#endif
}

extern int vsnprintf(char* str, size_t size, const char* format, va_list ap);

LIB_APIs void ssv6xxx_vsnprintf(char *out, size_t size, const char *format, va_list args)
{
	return vsnprintf(out, (out + size - 1), format, args );

}
#endif

//LIB_APIs s32 putstr (const char *s, size_t len)
//{
//    return  printstr(0, 0, s, len);
//}


//LIB_APIs s32 snputstr (char *out, size_t size, const char *s, size_t len)
//{
//    return  printstr( &out, (out + size - 1), s, len);
//}


//#endif


#if (CLI_ENABLE==1 && CONFIG_HOST_PLATFORM==0)
LIB_APIs ssv_type_s32 kbhit(void)
{
    return drv_uart_rx_ready(CONSOLE_UART);
}


LIB_APIs ssv_type_s32 getch(void)
{
    return drv_uart_rx(CONSOLE_UART);
}


LIB_APIs ssv_type_s32 putchar(ssv_type_s32 ch)
{
    return drv_uart_tx(CONSOLE_UART, ch);
}
#endif


#if 0
LIB_APIs void ssv6xxx_raw_dump(u8 *data, s32 len)
{
	ssv6xxx_raw_dump_ex(data, len, true, 10, 10, 16, LOG_LEVEL_ON, LOG_MODULE_EMPTY);
	return;
}


LIB_APIs ssv_type_bool ssv6xxx_raw_dump_ex(u8 *data, s32 len, ssv_type_bool with_addr, u8 addr_radix, s8 line_cols, u8 radix, u32 log_level, u32 log_module)
{
    s32 i;

	// check input parameters
	if ((addr_radix != 10) && (addr_radix != 16))
	{
		LOG_ERROR("%s(): invalid value 'addr_radix' = %d\n\r", __FUNCTION__, addr_radix);
		return false;
	}
	if ((line_cols != 8) && (line_cols != 10) && (line_cols != 16) && (line_cols != -1))
	{
		LOG_ERROR("%s(): invalid value 'line_cols' = %d\n\r", __FUNCTION__, line_cols);
		return false;
	}
	if ((radix != 10) && (radix != 16))
	{
		LOG_ERROR("%s(): invalid value 'radix' = %d\n\r", __FUNCTION__, radix);
		return false;
	}

	if (len == 0)	return true;

	// if ONLY have one line
	if (line_cols == -1)
	{
		LOG_TAG_SUPPRESS_ON();
		// only print addr heading at one time
		if ((with_addr == true))
		{
			if      (addr_radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%08d: ", 0);
			else if (addr_radix == 16)	LOG_PRINTF_LM(log_level, log_module, "0x%08x: ", 0);
		}

		for (i=0; i<len; i++)
		{
			// print data
			if	    (radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%4d ",  data[i]);
			else if (radix == 16)	LOG_PRINTF_LM(log_level, log_module, "%02x ", data[i]);
		}
		LOG_PRINTF_LM(log_level, log_module, "\n\r");
		LOG_TAG_SUPPRESS_OFF();
		return true;
	}

	// normal case
	LOG_TAG_SUPPRESS_ON();
    for (i=0; i<len; i++)
	{
		// print addr heading
		if ((with_addr == true) && (i % line_cols) == 0)
		{
			if      (addr_radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%08d: ", i);
			else if (addr_radix == 16)	LOG_PRINTF_LM(log_level, log_module, "0x%08x: ", i);
		}
		// print data
		if	    (radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%4d ",  data[i]);
		else if (radix == 16)	LOG_PRINTF_LM(log_level, log_module, "%02x ", data[i]);
		// print newline
        if (((i+1) % line_cols) == 0)
            LOG_PRINTF_LM(log_level, log_module, "\n\r");
    }
    LOG_PRINTF_LM(log_level, log_module, "\n\r");
	LOG_TAG_SUPPRESS_OFF();
	return true;
}
#endif

#define ONE_RAW 16
void _packetdump(const char *title, const ssv_type_u8 *buf,
                             ssv_type_size_t len)
{
    ssv_type_size_t i;
    LOG_DEBUGF(LOG_L2_DATA, ("%s - hexdump(len=%d):\r\n    ", title, len));



    if (buf == NULL) {
        LOG_DEBUGF(LOG_L2_DATA, (" [NULL]"));
    }else{


        for (i = 0; i < ONE_RAW; i++)
            LOG_DEBUGF(LOG_L2_DATA, ("%02X ", i));

        LOG_DEBUGF(LOG_L2_DATA,("\r\n---\r\n00|"));


        for (i = 0; i < len; i++){
            LOG_DEBUGF(LOG_L2_DATA,(" %02x", buf[i]));
            if((i+1)%ONE_RAW ==0)
                LOG_DEBUGF(LOG_L2_DATA,("\r\n%02x|", (i+1)));
        }
    }
    LOG_DEBUGF(LOG_L2_DATA,("\r\n-----------------------------\r\n"));
}



void pkt_dump_txinfo(void *p)
{
    ssv_hal_dump_txinfo(p);
	return;
}

void pkt_dump_rxinfo(void *p)
{
    ssv_hal_dump_rxinfo(p);
	return;
}


LIB_APIs void hex_dump (const void *addr, ssv_type_u32 size)
{
    ssv_type_u32 i, j;
    const ssv_type_u32 *data = (const ssv_type_u32 *)addr;

#if (defined(__SSV_UNIX_SIM__) && __SSV_UNIX_SIM__)
    LOG_TAG_SUPPRESS_ON();
#endif
    LOG_PRINTF("        ");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("       %02X", i*sizeof(ssv_type_u32));

    LOG_PRINTF("\r\n--------");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("+--------");

    for (i = 0; i < size; i+= 8)
    {
        LOG_PRINTF("\r\n%08X:%08X", (ssv_type_s32)data, data[0]);
        for (j = 1; j < 8; j++)
        {
            LOG_PRINTF(" %08X", data[j]);
        }
        data = &data[8];
    }
    LOG_PRINTF("\r\n");
#if (defined(__SSV_UNIX_SIM__) && __SSV_UNIX_SIM__)
    LOG_TAG_SUPPRESS_OFF();
#endif
    return;
}

LIB_APIs void ssv_halt(void)
{
#if (defined(__SSV_UNIX_SIM__) && __SSV_UNIX_SIM__)
    abort();
//	system("pause");
//	exit(EXIT_FAILURE);
#else
	/*lint -save -e716 */
    while (1) ;
	/*lint -restore */
#endif
}

extern ssv_type_s32 _ssv6xxx_wifi_ioctl_Ext(ssv_type_u32 cmd_id, void *data, ssv_type_u32 len, ssv_type_bool blocking,const ssv_type_bool mutexLock);

const chip_def_S chip_list[] = {
      //{"SSV6051Q",SSV6051Q},
      //{"SSV6051Z",SSV6051Z},
      //{"SSV6030P",SSV6030P},
      //{"SSV6052Q",SSV6052Q},
      //{"SSV6006B",SSV6006B},
      //{"SSV6006C",SSV6006C},
      {"SV6155P",SV6155P,CHIP_ID_SV6155P},
      {"SV6156P",SV6156P,CHIP_ID_SV6156P},
      {"SV6166P",SV6166P,CHIP_ID_SV6166P},
      {"SV6167Q",SV6167Q,CHIP_ID_SV6167Q},
      {"SV6166F",SV6166F,CHIP_ID_SV6166F},
      {"SV6166M",SV6166M,CHIP_ID_SV6166M},
      {"SV6255P",SV6255P,CHIP_ID_SV6255P},
      {"SV6256P",SV6256P,CHIP_ID_SV6256P},
      {"SV6266P",SV6266P,CHIP_ID_SV6266P},
      {"SV6267Q",SV6267Q,CHIP_ID_SV6267Q},
      {"SV6266F",SV6266F,CHIP_ID_SV6266F},
      {"SV6266M",SV6266M,CHIP_ID_SV6266M},          
      {"SV6166FS",SV6166FS,CHIP_ID_SV6166FS},            
      {"SV6151P",SV6151P,CHIP_ID_SV6151P},                  
      {"SV6152P",SV6152P,CHIP_ID_SV6152P},                        
    };
chip_def_S* chip_sel;
//Check efuse chip id. 
ssv6xxx_result check_efuse_chip_id(void)
{
    ssv_type_u32 efuse_chip_id;
    ssv_type_u32 _chip_list;
    int i;

    efuse_chip_id = read_chip_id();
    efuse_chip_id &= 0xFF000000;
    //LOG_PRINTF("efuse_chip_id = 0x%x\r\n",efuse_chip_id);
    if(efuse_chip_id)
    {        
        for(i=0;i<32;i++)
        {
            if(CONFIG_CHIP_ID & (1<<i))
            {
                _chip_list=chip_list[i].eid;
								_chip_list &= 0xFF000000;
								if(_chip_list == efuse_chip_id)
                {
                    if((efuse_chip_id==(CHIP_ID_SV6167Q&0xFF000000)) || (efuse_chip_id==(CHIP_ID_SV6267Q&0xFF000000)))
                    {
                        g_host_cfg.DoDPD = FALSE;
                    }
                    else
                    {
                        g_host_cfg.DoDPD = FALSE;
                    }
                    return SSV6XXX_SUCCESS;
                }
            }
        }
        //if(i>=32)
        {
            LOG_PRINTF("\33[31m[Error] Wi-fi CHIP ID mismatch, efuse_chip_id = %x\33[0m", efuse_chip_id);
            ssv_hal_get_chip_name();
            LOG_PRINTF("\r\n");
            //while(1);
            return SSV6XXX_WRONG_CHIP_ID;
        }
    }
    else
    {
        if(g_host_cfg.usePA)
            g_host_cfg.DoDPD = FALSE;
        else
            g_host_cfg.DoDPD = TRUE;
        return SSV6XXX_SUCCESS;
    }

#if 0    
    if(
#if (CONFIG_CHIP_ID == SV6266P)
       (efuse_chip_id == CHIP_ID_SV6266P) ||
#elif (CONFIG_CHIP_ID == SV6167Q)
       (efuse_chip_id == CHIP_ID_SV6167Q) ||
#elif (CONFIG_CHIP_ID == SV6267Q)
       (efuse_chip_id == CHIP_ID_SV6267Q) ||
#else
# error "Please redefine CONFIG_CHIP_ID!!"
#endif
       (efuse_chip_id == 0))
    {
        if((efuse_chip_id==CHIP_ID_SV6167Q)||(efuse_chip_id==CHIP_ID_SV6267Q))
        {
            g_host_cfg.DoDPD = FALSE;
        }
        else
        {
            g_host_cfg.DoDPD = TRUE;
        }
        return SSV6XXX_SUCCESS;
    }
    else
    {
        LOG_PRINTF("\33[31m[Error] Wi-fi CHIP ID mismatch iComm %s!\33[0m\r\n",ssv_hal_get_chip_name());
        //while(1);
        return SSV6XXX_WRONG_CHIP_ID;
    }
#endif    
}
extern const char *ssv_version;
extern const char *ssv_date;
extern const char *rlsversion;

int ssv6xxx_init_mac(void* vif)
{
    ssv_vif* vf=(ssv_vif*)vif;
    if(gDeviceInfo->recovering != TRUE)
    {
    	LOG_DEBUG("\33[35mRELEASE VERSION: %s     SW VERSION: %s\r\nBUILD DATE: %s\33[0m\r\n",rlsversion,ssv_version, ssv_date);
    }
    return ssv_hal_init_mac(vf->self_mac);
}

int ssv6xxx_init_sta_mac(ssv_type_u32 wifi_mode)
{
    return ssv_hal_init_sta_mac(wifi_mode);
}

int ssv6xxx_init_ap_mac(void* vif)
{
#if (AP_MODE_ENABLE == 1)
    ssv_vif* vf=(ssv_vif*)vif;
    ssv6xxx_sec_type sec_type=gDeviceInfo->APInfo->sec_type;

    if(0==ssv_hal_init_ap_mac(vf->self_mac,gDeviceInfo->APInfo->nCurrentChannel))
    {
        //if((sec_type==SSV6XXX_SEC_WEP_40)||(sec_type==SSV6XXX_SEC_WEP_104))
        //{
        //    ssv_hal_ap_wep_setting(sec_type,gDeviceInfo->APInfo->password,vf->idx);
        //}
        return 0;
    }
    else
    {
        return -1;
    }
#else
    return -1;
#endif
}

int ssv6xxx_promiscuous_enable(void)
{
   return ssv_hal_promiscuous_enable();
}

int ssv6xxx_promiscuous_disable(void)
{
   return ssv_hal_promiscuous_disable();
}


ssv_type_u32 g_hw_enable = false;
extern void check_watchdog_timer(void *data1, void *data2);
extern struct task_info_st g_host_task_info[];
extern ssv_type_s32 g_watchdog_check_time;
extern void timer_sta_reorder_release(void* data1, void* data2);

void ssv6xxx_HW_disable(void)
{
	g_hw_enable = FALSE;
#if(RECOVER_ENABLE == 1)
   ssv_hal_watchdog_disable();
#if(RECOVER_MECHANISM == 1)
    os_cancel_timer(check_watchdog_timer,(ssv_type_u32)NULL,(ssv_type_u32)NULL);
#endif //#if(RECOVER_MECHANISM == 1)
#else
   ssv_hal_watchdog_disable();
#endif //#if(RECOVER_ENABLE == 1)
    //os_cancel_timer(timer_sta_reorder_release,(u32)NULL,(u32)NULL);

    ssv6xxx_drv_irq_disable(false);
#if((CONFIG_CHIP_ID!=SSV6006B)&&(CONFIG_CHIP_ID!=SSV6006C))
    ssv_hal_rf_disable();
#endif
    //Disable MCU
    ssv_hal_mcu_disable();

    return;


}
void ssv6xxx_HW_enable(void)
{
#if((CONFIG_CHIP_ID!=SSV6006B)&&(CONFIG_CHIP_ID!=SSV6006C))
    ssv_hal_rf_enable();
#endif
    //Enable MCU, it's duplicate, load fw has already set this bit
    ssv_hal_mcu_enable();

    ssv6xxx_drv_irq_enable(false);
#if(RECOVER_ENABLE == 1)
    ssv_hal_watchdog_enable();
#if(RECOVER_MECHANISM == 1)
    os_create_timer(IPC_CHECK_TIMER, check_watchdog_timer, NULL, NULL, (void*)TIMEOUT_TASK);
#endif //#if(RECOVER_MECHANISM == 1)
#endif //#if(RECOVER_ENABLE == 1)
    //os_create_timer(HT_RX_REORDER_BUF_TIMEOUT,timer_sta_reorder_release,NULL,NULL,(void*)TIMEOUT_TASK);

    g_hw_enable = TRUE;
    return;


}

ssv_type_bool ssv6xxx_download_fw(ssv_type_u8 *bin, ssv_type_u32 len)
{
    if(0==MAC_LOAD_FW(bin,len))
        return TRUE;
    else
        return FALSE;
}

void ssv6xxx_set_wakeup_bb_gpio(ssv_type_bool hi_lo, ssv_type_u32 pin)
{
    LOG_PRINTF("Set pin%d=%d\r\n",pin,hi_lo);
    ssv_hal_set_gpio_output(hi_lo, pin);
}

#if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
void ssv6xxx_sta_mode_disconnect(void *Info)
{
    struct StaInfo* SInfo = (struct StaInfo *)Info;
    OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    SInfo->status=DISCONNECT;
    #if(SW_8023TO80211==1)
    OS_MemSET(SInfo->seq_ctl,0,sizeof(SInfo->seq_ctl));
    #endif
    OS_MemSET(SInfo->joincfg, 0, sizeof(struct cfg_join_request));
    OS_MemSET(SInfo->joincfg_backup, 0, sizeof(struct cfg_join_request));
    SInfo->rssi = 0;
    gDeviceInfo->cci_current_level=0;
    gDeviceInfo->cci_current_gate=0;
    ssv_hal_update_cci_setting(MAX_CCI_SENSITIVE);
    OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);

}

void ssv6xxx_sta_mode_connect(void *Info)
{
    struct StaInfo* SInfo = (struct StaInfo *)Info;
    OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    SInfo->status = CONNECT;
    OS_MemCPY(SInfo->joincfg, SInfo->joincfg_backup, sizeof(struct cfg_join_request));
    OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
}
#endif

void tmr_set_vif_mcc_slice(void *data1, void *data2)
{
    ssv_vif* vif = (ssv_vif*)data1;
    ssv_type_u32 t_slice;
    if(!vif)
        return;
    if(data2)
        t_slice = (ssv_type_u32)data2;
    else
        t_slice = CHAN_SWITCH_TIMER_TIME;
    //LOG_PRINTF("tmr vif_%d, slice=%d\r\n",vif->idx,t_slice);
    ssv6xxx_wifi_set_mcc_time_slice(vif->idx, t_slice);
}

ssv_type_u8* ssv6xxx_host_rx_data_get_data_ptr(void *rxpkt)
{
    ssv_type_u16 nOffset;
    nOffset=(ssv_type_u32)ssv_hal_get_rxpkt_data_ptr(rxpkt)-(ssv_type_u32)rxpkt;
    return (((ssv_type_u8*)rxpkt)+nOffset);
}

void os_msg_free(void *msg)
{
	msg_evt_free((MsgEvent *)msg);
}

void* os_msg_alloc()
{
	return msg_evt_alloc();
}

ssv_type_s32 os_msg_send(void* msg, void *pBuffer)
{
    ssv_type_s32 res;
	MsgEvent *MsgEv = (MsgEvent *)msg;
	/* Fill MsgEvent Content: */
	MsgEv->MsgType  = MEVT_PKT_BUF;
	MsgEv->MsgData  = (ssv_type_u32)pBuffer;
	MsgEv->MsgData1 = 0;
	MsgEv->MsgData2 = 0;
	MsgEv->MsgData3 = 0;

    os_frame_set_debug_flag(pBuffer,SSV_PBUF_DBG_FLAG_L2_CMDENG);
	res = msg_evt_post(MBOX_CMD_ENGINE, MsgEv);
	
	return res;
}


