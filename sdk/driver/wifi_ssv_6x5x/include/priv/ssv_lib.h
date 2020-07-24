/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SSV_LIB_H_
#define _SSV_LIB_H_

#include <host_config.h>
#include <rtos.h>

#if (CONFIG_HOST_PLATFORM == 0)
#include <stdarg.h>
#endif

struct ssv_llist {
    struct ssv_llist   *next;
};


struct ssv_llist_head {
    struct ssv_llist   *list;
    struct ssv_llist   *last;
    ssv_type_u32 llen;
};

struct ssv_list_q {
    struct ssv_list_q   *next;
    struct ssv_list_q   *prev;
    unsigned int    qlen;
};

#define ssv_in_range(c, lo, up)  ((ssv_type_u8)c >= lo && (ssv_type_u8)c <= up)
#define ssv_isprint(c)           ssv_in_range(c, 0x20, 0x7f)
#define ssv_isdigit(c)           ssv_in_range(c, '0', '9')
#define ssv_isxdigit(c)          (ssv_isdigit(c) || ssv_in_range(c, 'a', 'f') || ssv_in_range(c, 'A', 'F'))
#define ssv_islower(c)           ssv_in_range(c, 'a', 'z')
#define ssv_isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')

void llist_head_init(struct ssv_llist_head *lhd);
struct ssv_llist *llist_pop(struct ssv_llist_head *lhd);
void llist_push(struct ssv_llist_head *lhd, struct ssv_llist *new);
ssv_type_u32 llist_l_len(struct ssv_llist_head *lhd);
struct ssv_llist *llist_pop_safe(struct ssv_llist_head *lhd, OsMutex *pmtx);
void llist_push_safe(struct ssv_llist_head *lhd, struct ssv_llist *new, OsMutex *pmtx);
void llist_push_head_safe(struct ssv_llist_head *lhd, struct ssv_llist *old, OsMutex *pmtx);
ssv_type_u32 llist_l_len_safe(struct ssv_llist_head *lhd, OsMutex *pmtx);


void list_q_init(struct ssv_list_q *qhd);
void list_q_qtail(struct ssv_list_q *qhd, struct ssv_list_q *newq);
struct ssv_list_q *list_q_deq(struct ssv_list_q *qhd);
unsigned int list_q_len(struct ssv_list_q *qhd);
void list_q_insert(struct ssv_list_q *qhd, struct ssv_list_q *prev, struct ssv_list_q *newq);
void list_q_remove(struct ssv_list_q *qhd,struct ssv_list_q *curt);

void list_q_qtail_safe(struct ssv_list_q *qhd, struct ssv_list_q *newq, OsMutex *pmtx);
struct ssv_list_q *list_q_deq_safe(struct ssv_list_q *qhd, OsMutex *pmtx);
ssv_type_u32 list_q_len_safe(struct ssv_list_q *qhd, OsMutex *pmtx);
void list_q_insert_safe(struct ssv_list_q *qhd, struct ssv_list_q *prev, struct ssv_list_q *newq, OsMutex *pmtx);
void list_q_remove_safe(struct ssv_list_q *qhd,struct ssv_list_q *curt, OsMutex *pmtx);

unsigned long ssv_strtoul(const char *nptr, char **endptr, int base);
char *ssv_strtok(char * str, const char * delim);
LIB_APIs ssv_type_u32 ssv6xxx_atoi_base( const char *s, ssv_type_u32 base );
LIB_APIs ssv_type_s32 ssv6xxx_atoi( const char *s );
LIB_APIs ssv_type_s32 ssv6xxx_str_tolower(char *s);
LIB_APIs ssv_type_s32 ssv6xxx_str_toupper(char *s);

LIB_APIs ssv_type_s32 ssv6xxx_strrpos(const char *str, char delimiter);

#if (CONFIG_HOST_PLATFORM == 1)
ssv_type_u64 ssv6xxx_64atoi( char *s );
#endif



LIB_APIs char ssv6xxx_toupper(char ch);
LIB_APIs char ssv6xxx_tolower(char ch);

LIB_APIs ssv_type_s32 ssv6xxx_isupper(char ch);
LIB_APIs ssv_type_s32 ssv6xxx_strcmp( const char *s0, const char *s1 );
LIB_APIs char *ssv6xxx_strcat(char *s, const char *append);
LIB_APIs char *ssv6xxx_strncat(char *s, const char *append, ssv_type_size_t n);
LIB_APIs char *ssv6xxx_strncpy(char *dst, const char *src, ssv_type_size_t n);
LIB_APIs ssv_type_size_t ssv6xxx_strlen(const char *s);
LIB_APIs char *ssv6xxx_strcpy(char *dst, const char *src);
LIB_APIs ssv_type_s32 ssv6xxx_strncmp ( const char * s1, const char * s2, ssv_type_size_t n);
LIB_APIs void *ssv6xxx_memset(void *s, ssv_type_s32 c, ssv_type_size_t n);
LIB_APIs void *ssv6xxx_memcpy(void *dest, const void *src, ssv_type_size_t n);
LIB_APIs ssv_type_s32 ssv6xxx_memcmp(const void *s1, const void *s2, ssv_type_size_t n);
LIB_APIs char * ssv6xxx_strchr(const char * s, char c);


#if 0
LIB_APIs void ssv6xxx_vsnprintf(char *out, size_t size, const char *format, va_list args);
LIB_APIs void ssv6xxx_snprintf(char *out, size_t size, const char *format, ...);
LIB_APIs void ssv6xxx_printf(const char *format, ...);
#endif

//LIB_APIs s32 putstr(const char *str, size_t size);
//LIB_APIs s32 snputstr(char *out, size_t size, const char *str, size_t len);
//LIB_APIs void fatal_printf(const char *format, ...);


#if (CLI_ENABLE==1 && CONFIG_HOST_PLATFORM==0)
LIB_APIs ssv_type_s32 kbhit(void);
LIB_APIs ssv_type_s32 getch(void);
LIB_APIs ssv_type_s32 putchar(ssv_type_s32 ch);
#endif

#if 0
LIB_APIs void ssv6xxx_raw_dump(u8 *data, s32 len);

// with_addr : (true) -> will print address head "xxxxxxxx : " in begining of each line
// addr_radix: 10 (digial)  -> "00000171 : "
//		     : 16 (hex)		-> "000000ab : "
// line_cols : 8, 10, 16, -1 (just print all in one line)
// radix     : 10 (digital) ->  171 (max num is 255)
//			   16 (hex)		-> 0xab
// log_level : log level  pass to LOG_PRINTF_LM()
// log_module: log module pass to LOG_PRINTF_LM()
//
LIB_APIs ssv_type_bool ssv6xxx_raw_dump_ex(u8 *data, s32 len, ssv_type_bool with_addr, u8 addr_radix, s8 line_cols, u8 radix, u32 log_level, u32 log_module);
#endif

LIB_APIs void hex_dump(const void *addr, ssv_type_u32 size);

void _packetdump(const char *title, const ssv_type_u8 *buf,
                             ssv_type_size_t len);
LIB_APIs void ssv_halt(void);
//=============================================================
void ssv6xxx_HW_enable(void);
void ssv6xxx_HW_disable(void);
int ssv6xxx_init_mac(void* vif);
int ssv6xxx_init_sta_mac(ssv_type_u32 wifi_mode);
int ssv6xxx_init_ap_mac(void* vif);
ssv_type_bool ssv6xxx_download_fw(ssv_type_u8*, ssv_type_u32);
void ssv6xxx_set_wakeup_bb_gpio(ssv_type_bool hi_lo, ssv_type_u32 pin);
int ssv6xxx_promiscuous_enable(void);
int ssv6xxx_promiscuous_disable(void);
ssv_type_u8* ssv6xxx_host_rx_data_get_data_ptr(void *rxpkt);

#if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
void ssv6xxx_sta_mode_disconnect(void *Info);
void ssv6xxx_sta_mode_connect(void *Info);
#endif
void tmr_set_vif_mcc_slice(void *data1, void *data2);
void  os_msg_free(void *msg);
void* os_msg_alloc(void);
ssv_type_s32   os_msg_send(void* msg, void *pBuffer);

#endif /* _SSV_LIB_H_ */
