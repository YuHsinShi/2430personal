/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <stdarg.h>
#include <rtos.h>
#include "log.h"
#include <ssv_lib.h>
//#include <lib-impl.h>
#include <porting.h>

//SIM platform

#include <host_apis.h>

_os_mutex		g_log_mutex;
ssv_type_u32				g_dbg_mutex;
char s_prn_buf[PRN_BUF_MAX];

#ifdef __SSV_UNIX_SIM__
log_prnf_cfg_st		g_log_prnf_cfg;
log_out_desc_st		g_log_out;

// global variable for acceleration
//char s_acc_buf[ACC_BUF_MAX] = {0};
char s_prn_tmp[PRN_BUF_MAX];
char s_tag_lvl[PRN_TAG_MAX];
char s_tag_mod[PRN_TAG_MAX];
char s_tag_fl[PRN_TAG_MAX];


const char*	g_log_lvl_tag[LOG_LEVEL_OFF] = {
	""     ,		// LOG_LEVEL_ON
	"TRACE",		// LOG_LEVEL_TRACE
	"DEBUG",		// LOG_LEVEL_DEBUG
	"INFO" ,		// LOG_LEVEL_INFO
	"WARN" ,		// LOG_LEVEL_WARN
	"FAIL" ,		// LOG_LEVEL_FAIL
	"ERROR",		// LOG_LEVEL_ERROR
	"FATAL",		// LOG_LEVEL_FATAL
	// LOG_LEVEL_OFF
};

const char*	g_log_mod_tag[LOG_MODULE_ALL] = {
	""      ,		// LOG_MODULE_EMPTY
	"mrx"   ,		// LOG_MODULE_MRX		
	"mtx"   ,		// LOG_MODULE_MTX		
	"edca"  ,		// LOG_MODULE_EDCA		
	"pbuf"  ,		// LOG_MODULE_PBUF		
	"l3l4"  ,		// LOG_MODULE_L3L4		
	"mgmt"  ,		// LOG_MODULE_MGMT		
	"frag"  ,		// LOG_MODULE_FRAG		
	"defrag",		// LOG_MODULE_DEFRAG	
	"mlme"  ,		// LOG_MODULE_MLME
	"cmd"   ,		// LOG_MODULE_CMD
	"wpa"   ,		// LOG_MODULE_WPA
	"main"  ,       // LOG_MODULE_MAIN
	// LOG_MODULE_ALL	
};

/* ============================================================================
		static local functions on host/soc side
============================================================================ */

#if (SSV_LOG_DEBUG)
static ssv_inline const char *_extract_filename(const char *str)
{
#if (defined WIN32)
	char	ch = '\\';
//#elif (defined __SSV_UNIX_SIM__)
#else
	char	ch = '/';
#endif
	ssv_type_u32		p = (ssv_type_u32)ssv6xxx_strrpos(str, ch);
	if (p>0)
		return (str + p + 1);
	
	return "";
}
#endif

void	_log_out_dst_open_dump(ssv_type_u8 dst_open)
{
	log_printf("dst_open : 0x%02x ", dst_open);
	LOG_TAG_SUPPRESS_ON();
	if (dst_open & LOG_OUT_HOST_TERM)		log_printf("(LOG_OUT_HOST_TERM) ");
	//if (dst_open & LOG_OUT_HOST_FILE)		log_printf("(LOG_OUT_HOST_FILE) ");
	//if (dst_open & LOG_OUT_SOC_TERM)		log_printf("(LOG_OUT_SOC_TERM) ");
	if (dst_open & LOG_OUT_SOC_HOST_TERM)	log_printf("(LOG_OUT_SOC_HOST_TERM) ");
	//if (dst_open & LOG_OUT_SOC_HOST_FILE)	log_printf("(LOG_OUT_SOC_HOST_FILE) ");
	LOG_TAG_SUPPRESS_OFF();
	log_printf("\n\r");
}

void	_log_out_dst_cur_dump(ssv_type_u8 dst_cur)
{
	log_printf("dst_cur  : 0x%02x ", g_log_out.dst_cur);
	LOG_TAG_SUPPRESS_ON();
	if (dst_cur & LOG_OUT_HOST_TERM)		log_printf("(LOG_OUT_HOST_TERM) ");
	//if (dst_cur & LOG_OUT_HOST_FILE)		log_printf("(LOG_OUT_HOST_FILE) ");
	//if (dst_cur & LOG_OUT_SOC_TERM)			log_printf("(LOG_OUT_SOC_TERM) ");
	if (dst_cur & LOG_OUT_SOC_HOST_TERM)	log_printf("(LOG_OUT_SOC_HOST_TERM) ");
	//if (dst_cur & LOG_OUT_SOC_HOST_FILE)	log_printf("(LOG_OUT_SOC_HOST_FILE) ");
	LOG_TAG_SUPPRESS_OFF();
	log_printf("\n\r");
}
#endif

/* ============================================================================
		static local functions on host side
============================================================================ */





/* ============================================================================
		exported functions on host/soc side
============================================================================ */





void LOG_init(ssv_type_bool tag_level, ssv_type_bool tag_mod, ssv_type_u32 level, ssv_type_u32 mod_mask, ssv_type_bool fileline)
{
	LOG_MUTEX_INIT();

#ifdef __SSV_UNIX_SIM__
	ssv6xxx_memset((void *)&g_log_prnf_cfg, 0, sizeof(log_prnf_cfg_st));
	g_log_prnf_cfg.prn_tag_lvl	= true;
	g_log_prnf_cfg.prn_tag_mod	= true;
	g_log_prnf_cfg.prn_tag_sprs	= false;
	g_log_prnf_cfg.chk_tag_sprs	= false;

	g_log_prnf_cfg.prn_tag_lvl	= tag_level;
	g_log_prnf_cfg.prn_tag_mod	= tag_mod;
	g_log_prnf_cfg.lvl			= level;

	mod_mask |= LOG_MODULE_MASK(LOG_MODULE_EMPTY);	// by default, enable LOG_MODULE_EMPTY
	g_log_prnf_cfg.mod			= mod_mask;
	g_log_prnf_cfg.fl			= fileline;
#endif
	// init frm buf
}

#ifdef __SSV_UNIX_SIM__
void LOG_out_desc_dump(void)
{
	log_printf("< log output descriptor >\n\r");
	_log_out_dst_open_dump(g_log_out.dst_open);
	_log_out_dst_cur_dump(g_log_out.dst_cur);

	log_printf("path     : %s\n\r",	    g_log_out.path);
	log_printf("fp       : 0x%08x\n\r", (ssv_type_u32)g_log_out.fp);

	return;
}


ssv_type_bool	_sim_out_dst_open(ssv_type_u8 _dst, const ssv_type_u8 *_path)
{
	SSV_ASSERT((_dst == LOG_OUT_HOST_TERM));
	if (_dst == LOG_OUT_HOST_TERM)
	{
		if (LOG_OUT_DST_IS_OPEN(LOG_OUT_HOST_TERM))
			return false;
		
		LOG_OUT_DST_OPEN(LOG_OUT_HOST_TERM);
		return true;
	}

	/*if (_dst == LOG_OUT_HOST_FILE)
	{
		if (LOG_OUT_DST_IS_OPEN(LOG_OUT_HOST_FILE))
			return false;

		if (g_log_out.fp != NULL)
		{
			fclose((FILE *)g_log_out.fp);	// try to fclose() fp
			g_log_out.fp = NULL;
		}
		if ((g_log_out.fp = (void *)fopen((const char *)_path, LOG_OUT_FILE_MODE)) == NULL)
		{
			LOG_FAIL("fail to fopen() %s\n", _path);
			return false;
		}
		strcpy((char *)(g_log_out.path), (const char *)_path);
		LOG_TRACE("%s(): fopen() %s\n", __FUNCTION__, _path);

		LOG_OUT_DST_OPEN(LOG_OUT_HOST_FILE);
		return true;
	}
       */
	// should never reach here!!
	return false;
}

ssv_type_bool	_sim_out_dst_close(ssv_type_u8 _dst)
{
	SSV_ASSERT((_dst == LOG_OUT_HOST_TERM));
	if (_dst == LOG_OUT_HOST_TERM)
	{
		if (LOG_OUT_DST_IS_OPEN(LOG_OUT_HOST_TERM))
		{
			LOG_OUT_DST_CUR_OFF(LOG_OUT_HOST_TERM);
			LOG_OUT_DST_CLOSE(LOG_OUT_HOST_TERM);
		}
		return true;
	}

	/*if (_dst == LOG_OUT_HOST_FILE)
	{
		if (LOG_OUT_DST_IS_OPEN(LOG_OUT_HOST_FILE))
		{
			LOG_TRACE("%s(): fclose() %s, fp = 0x%08x\n\r", __FUNCTION__, g_log_out.path, g_log_out.fp);
			memset(g_log_out.path, 0, LOG_MAX_PATH);
			fclose((FILE *)g_log_out.fp);
			g_log_out.fp = NULL;
			
			LOG_OUT_DST_CUR_OFF(LOG_OUT_HOST_FILE);
			LOG_OUT_DST_CLOSE(LOG_OUT_HOST_FILE);
		}
		return true;
	}*/
	// should never reach here!!
	return false;
}

ssv_type_bool	_sim_out_dst_turn_on(ssv_type_u8 _dst)
{
	SSV_ASSERT((_dst == LOG_OUT_HOST_TERM));
	if (_dst == LOG_OUT_HOST_TERM)
	{
		if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_TERM) == false)
			LOG_OUT_DST_CUR_ON(LOG_OUT_HOST_TERM);

		return true;
	}
	/*if (_dst == LOG_OUT_HOST_FILE)
	{
		if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_FILE) == false)
		{
			if (g_log_out.fp == NULL)
			{
				LOG_ERROR("%s(): g_log_out.fp = NULL!", __FUNCTION__);
				return false;
			}
			LOG_OUT_DST_CUR_ON(LOG_OUT_HOST_FILE);
			return true;
		}
		return true;
	}*/
	// should never reach here!!
	return false;
}

ssv_type_bool	_sim_out_dst_turn_off(ssv_type_u8 _dst)
{
	SSV_ASSERT((_dst == LOG_OUT_HOST_TERM));
	if (_dst == LOG_OUT_HOST_TERM)
	{
		if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_TERM))
			LOG_OUT_DST_CUR_OFF(LOG_OUT_HOST_TERM);
		return true;
	}
	/*if (_dst == LOG_OUT_HOST_FILE)
	{
		if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_FILE))
		{
			if (g_log_out.fp == NULL)
			{
				LOG_ERROR("%s(): g_log_out.fp = NULL!", __FUNCTION__);
				return false;
			}
			LOG_OUT_DST_CUR_OFF(LOG_OUT_HOST_FILE);
			return true;
		}
		return true;
	}*/
	// should never reach here!!
	return false;
}
#endif

/* ============================================================================
		exported functions on host side
============================================================================ */

// 1. always print out, skip the influence of 'level' & 'module' filter.
// 2. always print out file & line info.
void ssv6xxx_fatal(ssv_type_u32 m, const char *func_name, ssv_type_u32 line, const char *fmt, ...)
{
#if (SSV_LOG_DEBUG)
	va_list args;
	char	*p = (char *)s_prn_buf;

	OS_MemSET(s_prn_buf, 0, sizeof(s_prn_buf));

	LOG_MUTEX_LOCK();
#ifdef __SSV_UNIX_SIM__
	sprintf(s_tag_lvl, "[%-5s] ", g_log_lvl_tag[LOG_LEVEL_FATAL]);
	if (ssv6xxx_strcmp(g_log_mod_tag[m], "") != 0)
		sprintf(s_tag_mod, "<%-5s> ", g_log_mod_tag[m]);

	p = (char *)s_prn_buf + ssv6xxx_strlen(s_prn_buf);
#endif
	va_start(args, fmt);
	vsprintf(p, fmt, args);
	va_end(args);

	// note : fatal printf will always print string to terminal
	p = (char *)s_prn_buf + ssv6xxx_strlen(s_prn_buf);
	ssv6xxx_strcat(p, "\nprogram halt!!!");
	hal_print("%s@%s,%d\r\n", s_prn_buf,func_name,line);
	//_prnf_soc_host_term(s_prn_buf);

	//if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_FILE))
	//{
	//	fprintf((FILE *)g_log_out.fp, "%s", s_prn_buf);
	//	fflush((FILE *)g_log_out.fp);
	//}
	LOG_MUTEX_UNLOCK();
	ssv_halt();
#endif
	return;
}

int ssv6xxx_vsnprintf(char *out, ssv_type_size_t size, const char *format, va_list args)
{
	return vsnprintf(out, (int)(out + size - 1), format, args );
}


// the device's trace functions output only the identifiers instead of entire strings
void ssv6xxx_printf(ssv_type_u32 n, ssv_type_u32 m, const char *file, ssv_type_u32 line, const char *fmt, ...)
{    
#if (SSV_LOG_DEBUG)
	va_list args;
	char	*p = (char *)s_prn_buf;
	int result = 0;

	LOG_MUTEX_LOCK();
	//assert(0 <= (n) && (n) <= LOG_LEVEL_FATAL);
#ifdef __SSV_UNIX_SIM__
	if ((n < g_log_prnf_cfg.lvl) || ((g_log_prnf_cfg.mod & LOG_MODULE_MASK(m)) == 0))
	{
		LOG_MUTEX_UNLOCK();
		return;
	}
	ssv6xxx_memset((void *)s_prn_buf, 0, PRN_BUF_MAX);
	ssv6xxx_memset((void *)s_prn_tmp, 0, PRN_BUF_MAX);
	ssv6xxx_memset((void *)s_tag_lvl, 0, PRN_TAG_MAX);
	ssv6xxx_memset((void *)s_tag_mod, 0, PRN_TAG_MAX);
	ssv6xxx_memset((void *)s_tag_fl,  0, PRN_TAG_MAX);

	if (!g_log_prnf_cfg.prn_tag_sprs)
	{
		if (g_log_prnf_cfg.prn_tag_lvl)
		{
			if (ssv6xxx_strcmp(g_log_lvl_tag[n], "") != 0)
				sprintf(s_tag_lvl, "[%-5s] ", g_log_lvl_tag[n]);
		}
		if (g_log_prnf_cfg.prn_tag_mod)
		{
			if (ssv6xxx_strcmp(g_log_mod_tag[m], "") != 0)
				sprintf(s_tag_mod, "<%-5s> ", g_log_mod_tag[m]);
		}
		if (g_log_prnf_cfg.fl)
			sprintf(s_tag_fl, "(%s #%4d) ", _extract_filename(file), line);
	}
	sprintf(s_prn_buf, "%s%s%s", s_tag_lvl, s_tag_mod, s_tag_fl);
	p += ssv6xxx_strlen(s_prn_buf);
#endif	
	va_start(args, fmt);	
	result = ssv6xxx_vsnprintf(p, PRN_BUF_MAX, fmt, args);
	va_end(args);
	//if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_TERM))
    hal_print("%s", s_prn_buf);
	if(result > PRN_BUF_MAX)
		hal_print("Error : print size is bigger than PRN_BUF_MAX");
    FFLUSH(0);
	//if (LOG_OUT_DST_IS_CUR_ON(LOG_OUT_HOST_FILE))
	//{
	//	fprintf((FILE *)g_log_out.fp, "%s", s_prn_buf);
	//	fflush((FILE *)g_log_out.fp);
	//}
	LOG_MUTEX_UNLOCK();
#endif
}

 


