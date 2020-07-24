/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <ssv_hal.h>
#include <ssv_lib.h>
#include <log.h>
#include "../ssv_drv_if.h"

#include "sdio_port.h"
#include "sdio_def.h"
#include "../../core/txrx_task.h"
#include <ssv_devinfo.h>
#include <host_apis.h>
#include "sdio_delay_tuning.h"

extern DeviceInfo_st *gDeviceInfo;
extern struct Host_cfg g_host_cfg;
extern ssv_type_u8* hcirxAggtempbuf;

#define ALIGN_BYTE  4
#define MAX_DATA_BYTE  2400

#define sdio_if_align_size(size) (((size) + ALIGN_BYTE - 1) & ~(ALIGN_BYTE - 1))

ssv_type_u32 dataIOPort, regIOPort;

//#define SDIO_IF_CHECK

static void* sdio_data_align=NULL;


ssv_type_bool sdio_if_write_reg(ssv_type_u32 addr, ssv_type_u32 data);

static void _sdio_if_host_isr(void)
{
    //LOG_PRINTF("_sdio_if_host_isr\r\n");
    TXRXTask_Isr(INT_RX,is_truly_isr());
}

ssv_type_bool sdio_if_open(void)
{
    ssv_type_u32 dma_buf_size;
    if(g_host_cfg.hci_aggr_tx)
        dma_buf_size = MAX_HCI_AGGR_SIZE;
    else
        dma_buf_size = MAX_DATA_BYTE;
        
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    sdio_host_init(_sdio_if_host_isr);
    //sdio_data_align = sdio_allocate_dma_buffer(dma_buf_size);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    //SSV_ASSERT(sdio_data_align);
    return TRUE;
}

ssv_type_bool sdio_if_close(void)
{
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    //sdio_free_dma_buffer(sdio_data_align);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return TRUE;
}

ssv_type_bool sdio_if_init(void)
{
    //get dataIOPort(Accesee packet buffer & SRAM)
    ssv_type_u8 val=0;
    ssv_type_u8 err=0;

    dataIOPort = 0;
    #if 1
    if(false==sdio_readb_cmd52(REG_DATA_IO_PORT_0, &val)) 
    {
        err++;
    }
    else
    {
        dataIOPort = dataIOPort | (val<< (8*0));
    }

    if(false==sdio_readb_cmd52(REG_DATA_IO_PORT_1, &val)) 
    {
        err++;
    }
    else
    {
        dataIOPort = dataIOPort | (val << ( 8*1 ));
    }

    if(false==sdio_readb_cmd52(REG_DATA_IO_PORT_2, &val)) 
    {
        err++;
    }
    else
    {
        dataIOPort = dataIOPort | (val << ( 8*2 ));
    }
    #else
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_0) << ( 8*0 ));
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_1) << ( 8*1 ));
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_2) << ( 8*2 ));
    #endif

    //get regIOPort(Access register)
    regIOPort = 0;
    #if 1
    if(false==sdio_readb_cmd52(REG_REG_IO_PORT_0,&val)) 
    {
        err++;
    }
    else
    {
        regIOPort = regIOPort | (val << ( 8*0 ));
    }

    if(false==sdio_readb_cmd52(REG_REG_IO_PORT_1,&val)) 
    {
        err++;
    }
    else
    {
        regIOPort = regIOPort | (val << ( 8*1 ));
    }

    if(false==sdio_readb_cmd52(REG_REG_IO_PORT_2,&val)) 
    {
        err++;
    }
    else
    {
        regIOPort = regIOPort | (val << ( 8*2 ));
    }
    #else
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_0) << ( 8*0 ));
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_1) << ( 8*1 ));
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_2) << ( 8*2 ));
    #endif

    if(err!=0)
    {
        LOG_PRINTF("get dataIOPort and regIOPort fail\r\n");
    }

    if(!gDeviceInfo || gDeviceInfo->recovering != TRUE)
    {
        LOG_PRINTF("dataIOPort 0x%x regIOPort 0x%x\r\n",dataIOPort,regIOPort);
    }

    if (!sdio_writeb_cmd52(REG_INT_MASK, SDIO_REG_INT_MASK))
    {
        SDIO_FAIL(0, "sdio_write_byte(0x04, 0xfe)\r\n");
        err++;
    }

    if(!gDeviceInfo || gDeviceInfo->recovering != TRUE)
    {
        LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0xfe)\r\n");
    }
    // switch to normal mode
    // bit[1] , 0:normal mode, 1: Download mode
    if (!sdio_writeb_cmd52(REG_Fn1_STATUS, 0x00))
    {
        SDIO_FAIL_RET(0, "sdio_write_byte(0x0c, %02x\r)\n", 0x00);
    }

    if(!gDeviceInfo || gDeviceInfo->recovering != TRUE)
    {
        LOG_PRINTF("<sdio init> : switch to normal mode (0x0c, 0x00)\r\n");
    }


    if(!gDeviceInfo || gDeviceInfo->recovering != TRUE)
    {
        LOG_PRINTF("<sdio init> : success!! \r\n");
    }

    return (err==0)?true:false;

}

// mask rx/tx complete int
// 0: rx int
// 1: tx complete int
void sdio_if_irq_enable(void)
{
   sdio_host_enable_isr(1);

   #ifdef SDIO_CARD_INT_TRIGGER
   if (!sdio_writeb_cmd52(REG_INT_MASK, 0xff))
       SDIO_FAIL(0, "sdio_write_byte(0x04, 0xfe)\r\n");

   if (!sdio_writeb_cmd52(REG_INT_MASK, SDIO_REG_INT_MASK))
       SDIO_FAIL(0, "sdio_write_byte(0x04, 0xfe)\r\n");
   #endif
}

void sdio_if_irq_disable(void)
{
    sdio_host_enable_isr(0);
    
    #if 0
    if (!sdio_writeb_cmd52(REG_INT_MASK, 0xff))
        SDIO_FAIL(0, "sdio_write_byte(0x04, 0xff)\r\n");
    LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0xff)\r\n");
    #endif
}

ssv_type_size_t sdio_get_readsz(ssv_type_bool bAgg,ssv_type_size_t next_len)
{
    ssv_type_s32 ret;
    ssv_type_u8 val=0;
    ssv_type_u8 err=0;
    ssv_type_size_t rlen=0;
    if (!bAgg)
    {
        #if 1
        if(false==sdio_readb_cmd52(REG_CARD_PKT_LEN_0,&val)) 
        {
            err++;
        }
        else
        {
            rlen = (ssv_type_u32)(val);
        }

        if(false==sdio_readb_cmd52(REG_CARD_PKT_LEN_1,&val)) 
        {
            err++;
        }
        else
        {
            rlen |= ((ssv_type_u32)val<<0x8);
        }

        #else
        rlen = ((u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_0));
        rlen |= ((u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_1) << 0x8);
        #endif
    } else {

        if(!next_len)
        {
            /* For aggr mode, *size is next packet length */ 
            ret = sdio_read_cmd53(dataIOPort, (void*)&rlen, sizeof(ssv_type_size_t));
            if (ret == false) { 
                SDIO_ERROR("sdio read failed size ret[%d]\r\n",ret);
                rlen = 0;
            }
            rlen &= 0x0000ffff;
            if(!rlen)
            {
                LOG_PRINTF("zero len\r\n");
                
                #if 1
                if(false==sdio_readb_cmd52(REG_CARD_PKT_LEN_0,&val)) 
                {
                    err++;
                }
                else
                {
                    rlen = (ssv_type_u32)(val);
                }

                if(false==sdio_readb_cmd52(REG_CARD_PKT_LEN_1,&val))                 
                {
                    err++;
                }
                else
                {
                    rlen |= (ssv_type_u32)(val<<0x8);
                }
                #else
                rlen = (u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_0);
                rlen |= ((u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_1) << 0x8);
                #endif
                
                LOG_PRINTF("reg len=%d======================================================================================\r\n",rlen);
                OS_MsDelay(2000);
            }
        }
        else
        {
            return next_len;
        }
    }

    if(err!=0)
    {
        LOG_PRINTF("\33[31m sdio get rx size fail\33[0m\r\n");
    }

    return (err==0)?rlen:0;
}
ssv_type_bool _sdio_if_read_reg(ssv_type_u32 addr , ssv_type_u32 *data);

ssv_type_s32 sdio_if_recv_data(ssv_type_u8 *dat, ssv_type_size_t len)
{
    ssv_type_size_t rlen;
    ssv_type_s32 ret;
    ssv_type_u8 int_status;
    
    ssv_type_u8 *data_align;
    
    if ((len >= MAX_DATA_BYTE)&&((!g_host_cfg.hci_rx_aggr)||(!hcirxAggtempbuf)))
    {
        SDIO_ERROR("sdio recv data len < %d, len = %d\r\n", MAX_DATA_BYTE, len);
        return 0;
    }
    
    if ((unsigned int)dat & 3)
    {
        data_align = sdio_data_align;
    }
    else
    {
        data_align = dat;
    }
    
    #if 1
    if(false==sdio_readb_cmd52(REG_INT_STATUS,&int_status))
    {
        LOG_PRINTF("\33[31m Get SDIO int status fail \33[0m\r\n");
        return -1;
    }
    #else
    int_status = sdio_readb_cmd52(REG_INT_STATUS);
    #endif
    if (!(int_status & 0x01))
    {
        return -1;
    }
    
    //rlen = (u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_0);
    //rlen |= ((u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_1) << 0x8);
    rlen = sdio_get_readsz(g_host_cfg.hci_rx_aggr,len);
    //LOG_PRINTF("rlen=%d,len=%d\r\n",rlen,len);
    //rlen=(rlen<len)?rlen:len;
    if((rlen > len)&&(!g_host_cfg.hci_rx_aggr))
    {
        void* tmp_addr=NULL;
        tmp_addr = (ssv_type_u8*)OS_MemAlloc(rlen+64);
        if(tmp_addr)
            ret = sdio_read_cmd53(dataIOPort,(void*)tmp_addr, sdio_if_align_size(rlen));
        else
            SSV_ASSERT(0);
        OS_MemCPY((void *)data_align,(void*)tmp_addr,len);
        OS_MemFree(tmp_addr);
    }
    else
    {
        if (rlen > 0)
        {
            ret = sdio_read_cmd53(dataIOPort,(void*)data_align, sdio_if_align_size(rlen));
        }
        else
        {
            return -1;
        }
    }
        
    if (ret == false) 
    {
        LOG_PRINTF("sdio_if_recv_data : rlen = %d fail\r\n", rlen);
        return -1;
    }
#ifndef SDIO_NO_NEED_ALIGN
    if ((unsigned int)dat & 3)
    {
        ssv6xxx_memcpy(dat, data_align, rlen);
    }
#endif 
    
    return rlen;
}

ssv_type_s32	sdio_if_write_data(void *dat, ssv_type_size_t len)
{
    ssv_type_s32 ret = 0;
    int retry = 10;

    if ((g_host_cfg.hci_aggr_tx==0) && (len >= MAX_DATA_BYTE))
    {
        SDIO_ERROR("sdio write data len < %d, len = %d\r\n", MAX_DATA_BYTE, len);
        return 0;
    }
    if ((unsigned int)dat & 3)
    {
        ssv6xxx_memcpy(sdio_data_align, dat, len);
        dat = sdio_data_align;
    }
    
    do
    {
        ret = sdio_write_cmd53(dataIOPort,dat,sdio_if_align_size(len));
    }
    while ((ret==FALSE) && (--retry));

    return ret;
}


ssv_type_bool _sdio_if_read_reg(ssv_type_u32 addr , ssv_type_u32 *data)    
{
	ssv_type_u8      in_buf[4], out_buf[4];
	ssv_type_u32     in_len, out_len;
    ssv_type_bool    ret;

	in_len	= 4;
	ssv6xxx_memset((void *)in_buf, 0x00, in_len);

	in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
	in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
	in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
	in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);

    do{
        ret = sdio_write_cmd53(regIOPort, in_buf, in_len);
        if (ret == false)
        {
            break;
        }

        out_len = 4;
        ssv6xxx_memset((void *)out_buf, 0x00, out_len);
        ret = sdio_read_cmd53(regIOPort, out_buf, out_len);        
        if (ret == false)
        {
            break;
        }
        
        *data  = 0;
        *data |= ((out_buf[0] & 0xff)<<( 0  ));
        *data |= ((out_buf[1] & 0xff)<<( 8  ));
        *data |= ((out_buf[2] & 0xff)<<( 16 ));
        *data |= ((out_buf[3] & 0xff)<<( 24 ));
        
        SDIO_TRACE("%-20s : 0x%08x, 0x%08x\r\n", "READ_IF_REG", addr, *data);
        
        return TRUE;
    }while(0);
    
    SDIO_ERROR("%-20s : 0x%08x, 0x%08x\r\n", "READ_IF_REG", addr, *data);
    
	return FALSE;
}

ssv_type_u32 sdio_if_read_reg(ssv_type_u32 addr)
{
    ssv_type_u32 val=0;

    if (!_sdio_if_read_reg(addr, &val))
    {
        return 0;
    }

    return val;
}

ssv_type_bool sdio_if_write_reg(ssv_type_u32 addr, ssv_type_u32 data)
{
    ssv_type_u8      in_buf[8];
    ssv_type_u32     in_len;
    ssv_type_bool    ret;

    in_len	= 8;
    in_buf[0] = ((ssv_type_u8)(addr >> 0) & 0xff);
    in_buf[1] = ((ssv_type_u8)(addr >> 8) & 0xff);
    in_buf[2] = ((ssv_type_u8)(addr >> 16) & 0xff);
    in_buf[3] = ((ssv_type_u8)(addr >> 24) & 0xff);

    in_buf[4] = ((ssv_type_u8)(data >> 0) & 0xff);
    in_buf[5] = ((ssv_type_u8)(data >> 8) & 0xff);
    in_buf[6] = ((ssv_type_u8)(data >> 16) & 0xff);
    in_buf[7] = ((ssv_type_u8)(data >> 24) & 0xff);

    ret = sdio_write_cmd53(regIOPort, in_buf, in_len);

    SDIO_TRACE("%-20s : 0x%08x, 0x%08x\r\n", "WRITE_IF_REG", addr, data);
    
    if(ret)
    {
        #ifdef SDIO_IF_CHECK
        ssv_type_u32 value;
        value = sdio_if_read_reg(addr);

        if (value != data)
        {
            LOG_PRINTF("!!!! addr = %08x, %08x<>%08x\r\n", addr, data, value);
        }
        #endif
        
        return TRUE;
    }

    SDIO_ERROR("%-20s : 0x%08x, 0x%08x\r\n", "WRITE_IF_REG", addr, data);

    return FALSE;
}

ssv_type_bool sdio_if_write_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size)
{
    ssv_type_s32 ret;
    // Set SDIO DMA start address
    sdio_if_write_reg(0xc0000860, addr);

    // Set data path to DMA to SRAM
    if (TRUE == sdio_writeb_cmd52(REG_Fn1_STATUS, 0x2))
    {
        ret = sdio_if_write_data(data, size);

        // Set data path back to packet
        sdio_writeb_cmd52(REG_Fn1_STATUS, 0x0);
        if(ret > 0)
            return TRUE;
    }

    return FALSE;
}// end of - sdio_write_sram -

ssv_type_u32 sdio_if_write_fw_to_sram(ssv_type_u8* fw_bin, ssv_type_u32 fw_bin_len, ssv_type_u32 block_size)
{
    ssv_type_u8   *fw_buffer = NULL;
    ssv_type_u8   *fw_ptr = fw_bin;
    ssv_type_u32  sram_addr = 0x00000000;
    ssv_type_u32  bin_len=fw_bin_len;
    ssv_type_u32  i=0,len=0, checksum=0, temp=0;
    ssv_type_u32  j=0;

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    fw_buffer = (ssv_type_u8 *)OS_MemAlloc(block_size);
    if (fw_buffer == NULL) {
        SDRV_FAIL("%s(): Failed to allocate buffer for firmware.\r\n",__FUNCTION__);
        return 0;
    }

    while(bin_len > 0){
        len=(bin_len >= block_size)?block_size:bin_len;
        bin_len -= len;
        if(len!=block_size){
            ssv6xxx_memset((void *)fw_buffer, 0xA5, block_size);
        }
        platform_read_firmware(fw_buffer,fw_ptr,len);
        fw_ptr += len;

        SDRV_DEBUG("%s(): read len=0x%x,sram_addr=0x%x\r\n",__FUNCTION__,len,sram_addr);

        if(FALSE== sdio_if_write_sram(sram_addr,fw_buffer,block_size)) goto SDIO_ERROR_RETURN;
        #ifdef SDIO_IF_CHECK
        if (1)
        {
            ssv_type_u32 data32 = 0, reg32 = 0;
            for (i = 0; i < block_size / 4; i ++)
            {
                data32 = *((ssv_type_u32 *)(&fw_buffer[i * 4]));
                reg32 = sdio_if_read_reg(sram_addr + i * 4);
                if (reg32 != data32)
                {
                    SDIO_ERROR("%-20s : 0x%08x, 0x%08x<>0x%08x\r\n", "sdio_if_write_sram", sram_addr + i * 4, data32, reg32);
                    return FALSE;
                }
            }
		}
        #endif
        for (i = 0; i < (block_size)/4; i++) /* force 1024 bytes a set. */
        {
            temp = *((ssv_type_u32 *)(&fw_buffer[i*4]));
            checksum += temp;
        }
		sram_addr += block_size;
        j++;
#if 0        
        if(gDeviceInfo->recovering != TRUE)
        {
            LOG_PRINTF("* ",__FUNCTION__);
            if( j% 16 == 0)
                LOG_PRINTF("\r\n",__FUNCTION__);
        }
#endif        
    }// while(bin_len > 0){
    //LOG_PRINTF("\r\n",__FUNCTION__);
    SDRV_DEBUG("%s(): checksum = 0x%x\r\n",__FUNCTION__, checksum);
    OS_MemFree(fw_buffer);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return checksum;
SDIO_ERROR_RETURN:
    SDRV_INFO("\r\n",__FUNCTION__);
    OS_MemFree(fw_buffer);
    SDRV_ERROR("%s(): FAIL\r\n",__FUNCTION__);
    return 0;
}



ssv_type_s32 sdio_if_start(void)
{
    return TRUE;
}

ssv_type_s32 sdio_if_stop(void)
{
    return TRUE;
}

ssv_type_bool sdio_if_wakeup_wifi(ssv_type_bool sw)
{
    ssv_type_bool ret = 0;
    if(sw)
    {
        //SET_TX_SEG(0xBEBEBEBE);
        sdio_writeb_cmd52(REG_OUTPUT_TIMING_REG,(0xF0|SDIO_DEF_OUTPUT_TIMING));
        ret = sdio_writeb_cmd52(REG_PMU_WAKEUP, 0x1);
    }
    else
    {
        ret = sdio_writeb_cmd52(REG_PMU_WAKEUP, 0x0);
    }

     return ret;
}

ssv_type_bool sdio_if_detect_card(void)
{
    return sdio_host_detect_card();
}

ssv_type_bool sdio_if_writeb(ssv_type_u8 func,ssv_type_u32 addr, ssv_type_u8 data)
{
    ssv_type_bool ret = 0;
    ret = sdio_writeb_cmd52(BASE_SDIO+addr, data);

    return ret;
}
ssv_type_u32 sdio_if_readb(ssv_type_u8 func,ssv_type_u32 addr)
{
    ssv_type_u8 data=0;
    if(false==sdio_readb_cmd52(BASE_SDIO+addr,&data))
        return 0;
    else
    return data;
}

const struct ssv6xxx_drv_ops	g_drv_sdio =
{
    DRV_NAME_SDIO,
    sdio_if_open, //Not implement "sdio_allocate_dma_buffer" in drv/sdio/
    sdio_if_close, //Not implement "sdio_allocate_dma_buffer" in drv/sdio/
    sdio_if_init,
    sdio_if_recv_data,
    sdio_if_write_data,
    NULL,
    NULL,
    NULL,
    NULL,
    sdio_if_write_sram,
    NULL,
    sdio_if_write_reg,
    sdio_if_read_reg,
    sdio_if_writeb,
    sdio_if_readb,
    sdio_if_write_fw_to_sram,
    sdio_if_start,
    sdio_if_stop,
    sdio_if_irq_enable,
    sdio_if_irq_disable,
    sdio_if_wakeup_wifi,
    sdio_if_detect_card,
};

