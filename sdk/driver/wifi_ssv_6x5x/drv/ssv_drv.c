/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <msgevt.h>
#include <cmd_def.h>
#include <log.h>
#include "ssv_drv.h"
#include "ssv_drv_if.h"
//#include <time.h>
#include <ssv_hal.h>
#include <ssv_lib.h>
#include "hctrl.h"
#include <txrx_hdl.h>

#define MAX_SSV6XXX_DRV     2
static ssv_type_s16                  s_drv_cnt;
static struct ssv6xxx_drv_ops*  s_drv_array[MAX_SSV6XXX_DRV];
static struct ssv6xxx_drv_ops   *s_drv_cur;

ssv_type_bool    ssv6xxx_drv_register(struct ssv6xxx_drv_ops *ssv_drv);
ssv_type_bool    ssv6xxx_drv_unregister(ssv_type_u16 i);

static ssv_type_bool _ssv6xxx_drv_started = false;
static OsSemaphore ssvdrv_rx_sphr;
ssv_type_u32 drv_trx_time=0; //ms

OsMutex drvMutex;




#ifndef __SSV_UNIX_SIM__
ssv_type_u32 free_tx_page=0;
ssv_type_u32 free_tx_id=0;
extern struct Host_cfg g_host_cfg;

#if 0
struct vif_tx_rcs vifTxRcs[MAX_VIF_NUM];
void update_tx_resource(void)
{
    ssv_hal_get_tx_resources((void*)&tx_rcs);
    if(gDeviceInfo->bMCC)
    {
        //LOG_PRINTF("1MCC rcs=%d,%d,T=%d\r\n",vifTxRcs[0].free_page,vifTxRcs[1].free_page,tx_rcs.free_page);
        if(tx_rcs.free_page >= (vifTxRcs[0].free_page+vifTxRcs[1].free_page))
        {
            tx_rcs.free_page -= (vifTxRcs[0].free_page+vifTxRcs[1].free_page);
        }
        while(tx_rcs.free_page)
        {            
            if(vifTxRcs[0].free_page < (g_host_cfg.tx_res_page/2))
            {
                vifTxRcs[0].free_page++;
                tx_rcs.free_page--;
            }

            if(vifTxRcs[1].free_page < (g_host_cfg.tx_res_page/2))
            {
                vifTxRcs[1].free_page++;
                tx_rcs.free_page--;
            }

            if((vifTxRcs[0].free_page+vifTxRcs[1].free_page) == g_host_cfg.tx_res_page)
                break;
        }
        //LOG_PRINTF("2MCC rcs=%d,%d,T=%d\r\n",vifTxRcs[0].free_page,vifTxRcs[1].free_page,tx_rcs.free_page);
    }
    else
    {
        vifTxRcs[0].free_page = 0;//tx_rcs.free_page;
        vifTxRcs[1].free_page = 0;
    }
}
#endif    

extern struct vif_flow_ctrl g_vif_fctl[MAX_VIF_NUM];
TX_STATE ssv6xxx_drv_tx_resource_enough(ssv_type_u32 frame_len, ssv_type_u16 frame_cnt, ssv_type_u16 q_idx)
{
    ssv_type_u32 page_count=0;
    ssv_type_u32 rty_cnt ;//= g_vif_fctl[q_idx].retry_cnt;//g_host_cfg.tx_retry_cnt;
//    u32 rty_cnt= g_host_cfg.tx_retry_cnt;
    ssv_type_bool need_check_tx_src=FALSE;
    TX_STATE ret;
    ssv_type_u8 edca_idx = 1;
    ssv_type_u16* cur_TxPage;

    if(gDeviceInfo->bMCC)
    {
        edca_idx++;
        rty_cnt= g_host_cfg.tx_retry_cnt;
        //q_idx &= 0x01;
        //cur_TxPage = &vifTxRcs[q_idx].free_page;
        cur_TxPage = &tx_rcs.free_page;
        //LOG_PRINTF("mcc q=%d, tx_page=%d\r\n",q_idx,*cur_TxPage);
    }
    else
    {
        cur_TxPage = &tx_rcs.free_page;
        if((q_idx == 0) || (q_idx == 1))
        {
            rty_cnt= g_vif_fctl[q_idx].retry_cnt;
        
            if(g_vif_fctl[q_idx].priority)
                edca_idx++;
        }
        else
        {                
            rty_cnt= g_host_cfg.tx_retry_cnt;
        }
    }

    page_count=ssv_hal_bytes_to_pages(frame_len);

    do
    {
        OS_MutexLock(txsrcMutex);
        if ((*cur_TxPage < page_count) || (tx_rcs.free_id == 0)|| (tx_rcs.free_space< frame_cnt))
        {
            need_check_tx_src = TRUE;
            ret = KEEP_TRY;
        }
        else
	    {
			need_check_tx_src = FALSE;
            ret = GO_AHEAD;
            
#if 1
            if(tx_rcs.edca_fifo_cnt[edca_idx] > 13)
            {
                ssv_hal_get_tx_resources((void*)&tx_rcs);
                //update_tx_resource();
				if(g_vif_fctl[q_idx].priority){
	                if(tx_rcs.edca_fifo_cnt[edca_idx] > 10)
	                    ret = NEXT_ONE;
                }else{
					if(tx_rcs.edca_fifo_cnt[edca_idx] > 7)
	                    ret = NEXT_ONE;
                }
            }
#endif           
            if(ret == GO_AHEAD)
            {
                if(*cur_TxPage < (g_host_cfg.tx_res_page/(3+gDeviceInfo->bMCC)))
                    ssv_hal_get_tx_resources((void*)&tx_rcs);
                    //update_tx_resource();
                
                *cur_TxPage -= page_count;
                tx_rcs.free_id --;
                tx_rcs.free_space-=frame_cnt;

            }

        }
        OS_MutexUnLock(txsrcMutex);

        if(need_check_tx_src)
        {
            if(rty_cnt > 0){
                rty_cnt--;

                //update_tx_resource
                OS_MutexLock(txsrcMutex);
                ssv_hal_get_tx_resources((void*)&tx_rcs);
                //update_tx_resource();
                OS_MutexUnLock(txsrcMutex);
#if 1                
                if(tx_rcs.edca_fifo_cnt[edca_idx] > 9)
                {
                    //LOG_PRINTF("Fk!!(%d)\r\n",tx_rcs.edca_fifo_cnt[q_idx+1]);
                    ret = PAUSE_TX;
                    break;
                }
#endif
            }
            else
            {
                LOG_DEBUGF(LOG_TXRX, ("qid(%d),page%d,id=%d, space= %d, pg_cnt = %d, frm len =%d,MCC=%d\r\n",q_idx,
                                      *cur_TxPage, tx_rcs.free_id,tx_rcs.free_space, page_count, frame_len,gDeviceInfo->bMCC));        
                if(gDeviceInfo->bMCC){
                    ret = PAUSE_TX;
                }
                else{
                    ret = KEEP_TRY;
                }
                break;
            }
        }
    }while(need_check_tx_src);

    return ret;
}

ssv_type_bool ssv6xxx_drv_wait_tx_resource(ssv_type_u32 frame_len)
{
    ssv_type_u8 times=0;
    //struct ssv6xxx_hci_txq_info *pInfo=NULL;
    //ssv_type_u32 regVal;

    while(GO_AHEAD!=ssv6xxx_drv_tx_resource_enough(frame_len,1,0xFFFF))
    {
//        LOG_PRINTF("wait tx resource\n");
        if(times>=1){
            OS_TickDelay(1);
        }

        //update_tx_resource
        OS_MutexLock(txsrcMutex);
        ssv_hal_get_tx_resources((void*)&tx_rcs);
        OS_MutexUnLock(txsrcMutex);

        times++;
    }
    return TRUE;
}

#endif//#ifndef __SSV_UNIX_SIM__

#if (defined(CONFIG_CMD_BUS_TEST))&&(CONFIG_CMD_BUS_TEST==1)
extern OsMsgQ spi_qevt;
#endif
void  CmdEng_RxHdlData(void *frame);
ssv_type_u32 ssv6xxx_drv_get_handle()
{
    ssv_type_u32 retVal=0;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(-1, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->handle == NULL)
    {
        SDRV_WARN("%s() : NO handle() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return -1;
    }
    OS_MutexLock(drvMutex);
    retVal=s_drv_cur->handle();
    OS_MutexUnLock(drvMutex);
    return retVal;
}

ssv_type_bool ssv6xxx_drv_ack_int()
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->ack_int == NULL)
    {
        SDRV_WARN("%s() : NO ack_int() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->ack_int();
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_register(struct ssv6xxx_drv_ops *ssv_drv)
{
    ssv_type_u16 i;

    if (s_drv_cnt == MAX_SSV6XXX_DRV)
    {
        SDRV_ERROR("%s s_drv_cnt = MAX_SSV6XXX_DRV!\r\n",__FUNCTION__);
        return false;
    }
    SDRV_TRACE("%s() <= : 0x%08x, %s\r\n", __FUNCTION__, ssv_drv, ssv_drv->name);

    // find empty slot in array
    for (i = 0; i < MAX_SSV6XXX_DRV; i++)
    {
        if (s_drv_array[i] == NULL)
        {
            s_drv_array[i] = ssv_drv;
            s_drv_cnt++;
            SDRV_TRACE("%s() => : ok! s_drv_cnt = %d, i = %d, ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, s_drv_cnt, i, ssv_drv, ssv_drv->name);
            return TRUE;
        }
    }

    /* never reach here! */
    SDRV_FATAL("%s should never reach here!\r\n",__FUNCTION__);
    return FALSE;
}

ssv_type_bool ssv6xxx_drv_unregister(ssv_type_u16 i)
{
    if (s_drv_cnt == 0)
    {
        SDRV_WARN("%s() : s_drv_cnt = 0, return true!\r\n", __FUNCTION__);
        return TRUE;
    }
    SDRV_TRACE("%s() <= : i = %d, 0x%08x, %s\r\n", __FUNCTION__, i, s_drv_array[i], s_drv_array[i]->name);

    // find matching slot in array
    s_drv_array[i] = 0;
    s_drv_cnt--;
    SDRV_TRACE("%s() => : s_drv_cnt = %d\r\n", __FUNCTION__, s_drv_cnt);
    return TRUE;
}

void ssv6xxx_drv_list(void)
{
    ssv_type_u16 i;

    LOG_PRINTF("available in ssv driver module : %d\r\n",__LINE__);
    for (i = 0; i < s_drv_cnt; i++)
    {
        if (s_drv_cur == s_drv_array[i])
        {
            LOG_PRINTF("%-10s : 0x%08x (selected)\r\n", s_drv_array[i]->name, (unsigned int)s_drv_array[i]);
        }
        else
        {
            LOG_PRINTF("%-10s : 0x%08x\r\n", s_drv_array[i]->name, (unsigned int)s_drv_array[i]);
        }
    }
    LOG_PRINTF("%-10s = %d\r\n", "TOTAL", s_drv_cnt);
}

ssv_type_bool ssv6xxx_drv_module_init(void)
{
    SDRV_TRACE("%s() <=\r\n", __FUNCTION__);

    s_drv_cnt = 0;
    OS_MemSET(s_drv_array, 0x00, MAX_SSV6XXX_DRV * sizeof(struct ssv6xxx_drv_ops *));
    s_drv_cur = 0;

    // register each driver
//#if (SDRV_INCLUDE_SIM)
//	ssv6xxx_drv_register(&g_drv_sim);
//#endif


#if ((defined(SDRV_INCLUDE_SDIO)) && (SDRV_INCLUDE_SDIO))
    ssv6xxx_drv_register((struct ssv6xxx_drv_ops *)&g_drv_sdio);
#endif

#if (defined _WIN32)
#if ((defined(SDRV_INCLUDE_UART)) && (SDRV_INCLUDE_UART))
    ssv6xxx_drv_register((struct ssv6xxx_drv_ops *)&g_drv_uart);
#endif

#if ((defined(SDRV_INCLUDE_USB)) && (SDRV_INCLUDE_USB))
    ssv6xxx_drv_register((struct ssv6xxx_drv_ops *)&g_drv_usb);
#endif
#endif /* _WIN32 */

#if ((defined(SDRV_INCLUDE_SPI)) && (SDRV_INCLUDE_SPI))
    ssv6xxx_drv_register((struct ssv6xxx_drv_ops *)&g_drv_spi);
#endif

    if(OS_SUCCESS != OS_MutexInit(&drvMutex,"drvMutex"))
        return FALSE;
    if(OS_SUCCESS != OS_MutexInit(&txsrcMutex,"txsrcMutex"))
        return FALSE;

    return TRUE;

}

// ssv_drv module release
void ssv6xxx_drv_module_release(void)
{
    ssv_type_s16 i, tmp;

    SDRV_TRACE("%s() <= : s_drv_cnt = %d\r\n", __FUNCTION__, s_drv_cnt);
    // close each driver & unregister
    tmp = s_drv_cnt;
    for (i = 0; i < tmp; i++)
    {
        SDRV_TRACE("s_drv_array[%d] = 0x%08x\r\n", i, (unsigned int)s_drv_array[i]);
        if (s_drv_array[i] != NULL)
        {
            if (s_drv_array[i]->close != NULL)
                s_drv_array[i]->close();

            if (!ssv6xxx_drv_unregister((ssv_type_u16)i))
                SDRV_WARN("ssv6xxx_drv_unregister(%d) fail!\r\n", i);
        }
    }

    s_drv_cur = 0;
    OS_MutexDelete(drvMutex);
    OS_MutexDelete(txsrcMutex);
    SDRV_TRACE("%s() =>\r\n", __FUNCTION__);
    return;
}

ssv_type_bool ssv6xxx_drv_select(char name[32])
{
    ssv_type_u16 i;
    ssv_type_bool bRet;
    struct ssv6xxx_drv_ops *drv_target;

    SDRV_TRACE("%s() <= : name = %s, s_drv_cnt = %d, s_drv_cur = (0x%08x)\r\n", __FUNCTION__, name, s_drv_cnt, (unsigned int)s_drv_cur);

    if (s_drv_cnt == 0)
		SDRV_FAIL("%s s_drv_cnt = 0\r\n",__FUNCTION__);

    // find the matching ssv_drv
    drv_target = 0;
    for (i = 0; i < s_drv_cnt; i++)
    {
        if (ssv6xxx_strcmp(name, s_drv_array[i]->name) == 0)
        {
            drv_target = s_drv_array[i];
            break;
        }
    }

    if (drv_target == 0)
    {
	    LOG_PRINTF("ssv driver '%s' is NOT available now!\r\n", name);
        // ssv6xxx_drv_list();
        return FALSE;
    }
    // if the target drv = current drv, just return
    if (drv_target == s_drv_cur)
    {
	    LOG_PRINTF("ssv drv '%s' is already in selection.\r\n", drv_target->name);
        // ssv6xxx_drv_list();
        return TRUE;
    }

    // try to open the target ssv_drv
    bRet = FALSE;
    if (drv_target->open != NULL)
    {
        if ((bRet = drv_target->open()) == false)
		SDRV_FAIL("open() fail! in s_drv_cur (0x%08x, %s)\r\n", drv_target, drv_target->name);
    }
    else
    {
        bRet = true; // regard it as success
	    SDRV_WARN("open() = NULL in s_drv_cur (0x%08x, %s)\r\n", drv_target, drv_target->name);
    }
    // if target drv open() fail, return
    if (bRet == FALSE)
    {
        // ssv6xxx_drv_list();
        return FALSE;
    }
    // init the target drv, alloc memory in drv open
    bRet = FALSE;
    if (drv_target->init != NULL)
    {
        if ((bRet = drv_target->init()) == FALSE)
		SDRV_FAIL("init() fail! in drv (0x%08x, %s)\r\n", drv_target, drv_target->name);
    }
    else
    {
        bRet = TRUE; // regard it as success
		SDRV_WARN("init() = NULL in drv (0x%08x, %s)\r\n", drv_target, drv_target->name);
    }
    // if target drv init() fail, return
    if (bRet == FALSE)
    {
        // ssv6xxx_drv_list();
        return FALSE;
    }

    s_drv_cur = drv_target;
    SDRV_DEBUG("select drv -> %-10s : 0x%08x\r\n", s_drv_cur->name, (unsigned int)s_drv_cur);
    // ssv6xxx_drv_list();
    return TRUE;
}

ssv_type_bool ssv6xxx_drv_open(void)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    ssv_assert(s_drv_cur->open != NULL);

    OS_MutexLock(drvMutex);
    ret=s_drv_cur->open();
    OS_MutexUnLock(drvMutex);

    return ret;
}

ssv_type_bool ssv6xxx_drv_close(void)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    ssv_assert(s_drv_cur->close != NULL);

    OS_MutexLock(drvMutex);
    ret=s_drv_cur->close();
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_init(void)
{
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    ssv_assert(s_drv_cur->init != NULL);

    return (s_drv_cur->init());
}

ssv_type_s32 ssv6xxx_drv_recv(ssv_type_u8 *dat, ssv_type_size_t len)
{
    ssv_type_s32 retVal=0;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(-1, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->recv == NULL)
    {
        SDRV_WARN("%s() : NO recv() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return -1;
    }
    OS_MutexLock(drvMutex);
    retVal=s_drv_cur->recv(dat, len);
    if(retVal>0)
        drv_trx_time = OS_TICK2MS(OS_GetSysTick());
    OS_MutexUnLock(drvMutex);
    return retVal;
}

ssv_type_bool ssv6xxx_drv_get_name(char name[32])
{
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    ssv6xxx_strcpy(name, s_drv_cur->name);
    return TRUE;
}

ssv_type_bool ssv6xxx_drv_ioctl(ssv_type_u32 ctl_code,
                            void *in_buf, ssv_type_size_t in_size,
                            void *out_buf, ssv_type_size_t out_size,
                            ssv_type_size_t *bytes_ret)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->ioctl == NULL)
    {
        SDRV_WARN("%s() : NO ioctl() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->ioctl(ctl_code, in_buf, in_size, out_buf, out_size, bytes_ret);
    OS_MutexUnLock(drvMutex);
    return ret;
}

#if CONFIG_STATUS_CHECK
extern ssv_type_u32 g_dump_tx;
extern void _packetdump(const char *title, const ssv_type_u8 *buf,
                             ssv_type_size_t len);

#endif

ssv_type_s32 ssv6xxx_drv_send(void *dat, ssv_type_size_t len)
{
    ssv_type_s32 retVal=0;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(-1, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->send == NULL)
    {
        SDRV_WARN("%s() : NO send() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return -1;
    }


#if CONFIG_STATUS_CHECK
    if(g_dump_tx)
        _packetdump("ssv6xxx_drv_send", dat, len);
#endif

    OS_MutexLock(drvMutex);
    retVal=s_drv_cur->send(dat, len);
    if(retVal>0)
        drv_trx_time = OS_TICK2MS(OS_GetSysTick());
    OS_MutexUnLock(drvMutex);

    return retVal;
}

ssv_type_u32 ssv6xxx_drv_read_reg(ssv_type_u32 addr)
{
    ssv_type_u32 retVal=0;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(-1, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->read_reg == NULL)
    {
        SDRV_WARN("%s() : NO read_reg() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return -1;
    }
    
    OS_MutexLock(drvMutex);
    retVal=s_drv_cur->read_reg(addr);
    OS_MutexUnLock(drvMutex);
    return retVal;
}

ssv_type_bool ssv6xxx_drv_write_reg(ssv_type_u32 addr, ssv_type_u32 data)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->write_reg == NULL)
    {
        SDRV_WARN("%s() : NO write_reg() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->write_reg(addr, data);
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_set_reg(ssv_type_u32 _REG_,ssv_type_u32 _VAL_,ssv_type_u32 _SHIFT_, ssv_type_u32 _MASK_)
{
    ssv_type_u32 regVal;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);


    regVal= ssv6xxx_drv_read_reg(_REG_);
    regVal =((((_VAL_) << _SHIFT_) & ~_MASK_) | (regVal & _MASK_));
    if(TRUE==ssv6xxx_drv_write_reg(_REG_, regVal))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
ssv_type_bool ssv6xxx_drv_write_byte(ssv_type_u32 addr, ssv_type_u32 data)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->write_byte == NULL)
    {
        SDRV_WARN("%s() : NO write_reg() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->write_byte(1,addr, data);
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_read_byte(ssv_type_u32 addr)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->read_byte == NULL)
    {
        SDRV_WARN("%s() : NO read_byte() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->read_byte(1,addr);
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_write_sram (ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->write_sram == NULL)
    {
        SDRV_WARN("%s() : NO write_sram() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->write_sram(addr, data, size);
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_bool ssv6xxx_drv_read_sram (ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size)
{
    ssv_type_bool ret=TRUE;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->read_sram == NULL)
    {
        SDRV_WARN("%s() : NO read_sram() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    OS_MutexLock(drvMutex);
    ret=s_drv_cur->read_sram(addr, data, size);
    OS_MutexUnLock(drvMutex);
    return ret;
}

ssv_type_u32 ssv6xxx_drv_write_fw_to_sram(ssv_type_u8 *fw_bin, ssv_type_u32 fw_bin_len, ssv_type_u32 block_size)
{
    ssv_type_u32 checkSum=0;
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if (s_drv_cur->write_fw_to_sram== NULL)
    {
        SDRV_WARN("%s() : NO read_sram() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return 0;
    }
    OS_MutexLock(drvMutex);
    checkSum=s_drv_cur->write_fw_to_sram(fw_bin, fw_bin_len,block_size);
    OS_MutexUnLock(drvMutex);
    return checkSum;
}

ssv_type_bool ssv6xxx_drv_start (void)
{
#ifdef __linux__
    if ((_ssv6xxx_drv_started != TRUE) && (s_drv_cur->start != NULL))
    {
        // if sdio, use ioctl to start the HCI
        s_drv_cur->start();
    }
#endif
    _ssv6xxx_drv_started = TRUE;
    return TRUE;
}

ssv_type_bool ssv6xxx_drv_stop (void)
{
#ifdef __linux__
    if ((_ssv6xxx_drv_started != FALSE) && (s_drv_cur->stop != NULL))
    {
        s_drv_cur->stop();
    }
#endif
    _ssv6xxx_drv_started = FALSE;
    return TRUE;
}

static ssv_type_bool _ssv6xxx_irq_st = false;

ssv_type_bool ssv6xxx_drv_irq_enable(ssv_type_bool is_isr)
{
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);
    if(!is_isr)
        OS_MutexLock(drvMutex);
    if (s_drv_cur->ssv_irq_enable== NULL)
    {        
        if(!is_isr)
            OS_MutexUnLock(drvMutex);
        SDRV_WARN("%s() : NO read_sram() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    //LOG_PRINTF("%s,%d\r\n",__func__,is_isr);
    s_drv_cur->ssv_irq_enable();
    _ssv6xxx_irq_st = true;
    if(!is_isr)
        OS_MutexUnLock(drvMutex);
    return TRUE;

}

ssv_type_bool ssv6xxx_drv_irq_disable(ssv_type_bool is_isr)
{
    if (s_drv_cur == 0)
        SDRV_FAIL_RET(FALSE, "%s s_drv_cur = 0\r\n",__FUNCTION__);

    if(!is_isr)
        OS_MutexLock(drvMutex);
    if (s_drv_cur->ssv_irq_disable== NULL)
    {
        if(!is_isr)
            OS_MutexUnLock(drvMutex);
        SDRV_WARN("%s() : NO read_sram() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, (unsigned int)s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    //LOG_PRINTF("%s,%d\r\n",__func__,is_isr);
    s_drv_cur->ssv_irq_disable();
    _ssv6xxx_irq_st = false;
    if(!is_isr)
        OS_MutexUnLock(drvMutex);
    return TRUE;

}

ssv_type_bool ssv6xxx_drv_irq_status(void)
{
    return _ssv6xxx_irq_st;
}

ssv_type_u32 ssv6xxx_drv_get_TRX_time_stamp(void)
{
    ssv_type_u32 last_trx_time;
    
    //OS_MutexLock(drvMutex);
    last_trx_time =  drv_trx_time;
    //OS_MutexUnLock(drvMutex);
    return last_trx_time;
}

ssv_type_bool ssv6xxx_drv_wakeup_wifi(ssv_type_bool sw)
{
    ssv_type_bool ret;

    if (s_drv_cur->wakeup_wifi== NULL)
    {
        SDRV_WARN("%s() : NO wakeup_wifi() in ssv_drv = (0x%08x, %s)\r\n", __FUNCTION__, s_drv_cur, s_drv_cur->name);
        return FALSE;
    }
    
    //ssv6xxx_drv_irq_disable(false);
    {
        //u32 val;
        OS_MutexLock(drvMutex);
        //val = OS_EnterCritical();
        //LOG_PRINTF("%s,sw=%d\r\n",__func__,sw);
        ret = s_drv_cur->wakeup_wifi(sw);
        //OS_ExitCritical(val);
        OS_MutexUnLock(drvMutex);
    }
    //ssv6xxx_drv_irq_enable(false);
    return ret;
}
ssv_type_bool ssv6xxx_drv_detect_card(void)
{
    ssv_type_bool ret=FALSE;
    
    if (s_drv_cur == 0)
        SDRV_FAIL("%s s_drv_cur = 0\r\n",__FUNCTION__);

    if(s_drv_cur->detect_card != NULL)
    {
        OS_MutexLock(drvMutex);
        ret = s_drv_cur->detect_card();
        ret=s_drv_cur->open();
        //ret=s_drv_cur->init();
        OS_MutexUnLock(drvMutex);
    }

    return ret;
}

