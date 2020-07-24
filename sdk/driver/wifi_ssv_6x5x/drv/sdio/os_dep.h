/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __OS_DEP_H__
#define __OS_DEP_H__

#if 1
#include <rtos.h>
#include <sdio_def.h>
#else
#include <linux/version.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/delay.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
#include <linux/printk.h>
#else
#include <linux/kernel.h>
#endif
#endif

#define OS_WRAPPER_LOG	                            LOG_PRINTF

#define OS_WRAPPER_MEM_ALLOC(size, flag)			OS_MemAlloc(size)
#define OS_WRAPPER_MEM_FREE(p)					    OS_MemFree(p)

//typedef struct sdio_func					OS_WRAPPER_SDIO_FUNC;
//typedef struct sdio_device_id					OS_WRAPPER_SDIO_DEVICE_ID;
#define OS_WRAPPER_SDIO_FUNC void
#define OS_WRAPPER_SDIO_DEVICE_ID void

#define MMC_BUS_WIDTH_1 SDIO_BUS_1_BIT
#define MMC_BUS_WIDTH_4 SDIO_BUS_4_BIT

#define OS_WRAPPER_SDIO_SET_CLK(func, hz)			        sdio_set_clk(hz)
#define OS_WRAPPER_SDIO_SET_BUS_WIDTH(func, width)		    NULL//sdio_set_bwidth(width)
#define OS_WRAPPER_SDIO_HOST_LOCK(func)				         //sdio_claim_host(NULL)
#define OS_WRAPPER_SDIO_HOST_UNLOCK(func)			         //sdio_release_host(NULL)
#define OS_WRAPPER_SDIO_CMD52_RD(func, addr, pvalue)        ((TRUE==sdio_readb_cmd52(addr, pvalue))?0:-1)
#define OS_WRAPPER_SDIO_CMD52_WR(func, value, addr, ret)	((TRUE==sdio_writeb_cmd52(addr, value))?0:-1)
#define OS_WRAPPER_SDIO_CMD53_RD(func, dst, addr, cnt)		((TRUE==sdio_read_cmd53(addr,(void*)dst, cnt))?0:-1)//sdio_if_recv_raw_data(dst,addr,cnt) //((sdio_if_recv_data(dst, cnt)>0)?0:-1)
#define OS_WRAPPER_SDIO_CMD53_WR(func, addr, src, cnt)		((TRUE==sdio_write_cmd53(addr,(void*)src, cnt))?0:-1)//sdio_if_wirte_raw_data(addr,src,cnt)//((1==sdio_if_write_data(src, cnt))?0:-1)
#define OS_WRAPPER_SDIO_ALIGN_SIZE(func, size)			    size //sdio_align_size(NULL, size)
#define OS_WRAPPER_SDIO_SET_PRIV_DRV_DATA(func, glue)	    NULL//sdio_set_drvdata(NULL, glue)

#endif /* __OS_DEP_H__ */
