/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/



#include <ssv_types.h>
#include <msgevt.h>
#include <drv/ssv_drv.h>
#include <log.h>
#include <ssv_lib.h>
#include <host_apis.h>
#include <porting.h>
#include <ssv_devinfo.h>
#include <ssv_hal.h>
#include <ssv_frame.h>
#include <pbuf.h>
#include <txrx_task.h>


#if (CLI_ENABLE==1)
#include <cli.h>
#include "cli_cmd.h"
#include "cli_cmd_soc.h"
#include "cli_cmd_net.h"
#include "cli_cmd_wifi.h"
#include "cli_cmd_sim.h"
#include "cli_cmd_sys.h"
#include <ssv_dev.h>
#if CONFIG_MEMP_DEBUG
#include "lwip/memp.h"
#endif
#include <net_mgr.h>
#if NETAPP_SUPPORT
#include <net_app.h>
#endif
#include <recover.h>

void show_invalid_para(char * cmd_name)
{
    LOG_PRINTF("%s: invalid parameter \r\n",cmd_name);
}

/*---------------------------------- CMDs -----------------------------------*/
#ifdef __SSV_UNIX_SIM__
static void cmd_abort( ssv_type_s32 argc, char *argv[] )
{
    abort();
}

extern char SysProfBuf[256];
extern signed char SysProfSt;
static void cmd_exit( ssv_type_s32 argc, char *argv[] )
{
    if(SysProfSt == 1)
    {
        ssv_type_s16 i;

        LOG_PRINTF("*************System profiling result*************\n");
        for(i=0; i<256; i++)
            LOG_PRINTF("%c",SysProfBuf[i]);

        LOG_PRINTF("*************End of profiling result*************\n");
    }
    LOG_PRINTF("cmd_exit");
    reset_term();
}
#endif

#if CONFIG_STATUS_CHECK
extern ssv_type_u32 g_dump_tx;
#endif

static void cmd_dump( ssv_type_s32 argc, char *argv[] )
{
#if CONFIG_STATUS_CHECK
    if (ssv6xxx_strcmp(argv[1], "tx") == 0){
        if (ssv6xxx_strcmp(argv[2], "on") == 0){
            g_dump_tx = 1;
            LOG_PRINTF("tx on\r\n");
        }else{
            g_dump_tx = 0;
            LOG_PRINTF("tx off\r\n");
        }
    }
#endif//#if CONFIG_STATUS_CHECK



#if 0
    if (strcmp(argv[1], "all") == 0)
        dump_all = true;
    else dump_all = false;

    /* Dump PHY-Info: */
    if (strcmp(argv[1], "phyinfo")==0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_PHY_INFO_TBL, NULL, 0);

    }

    /* Dump Decision Table: */
    if (strcmp(argv[1], "decision") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_DECI_TBL, NULL, 0);

    }

    /* Dump STA MAC Address: */
    if (strcmp(argv[1], "sta-mac") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_STA_MAC, NULL, 0);
    }


    /* Dump STA BSSID: */
    if (strcmp(argv[1], "bssid") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_BSSID, NULL, 0);

    }

    /* Dump Ether Trap Table: */
    if (strcmp(argv[1], "ether-trap") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_ETHER_TRAP, NULL, 0);

    }

    /* Dump Flow Command Table: */
    if (strcmp(argv[1], "fcmd") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_FCMDS, NULL, 0);

    }

    /* Dump WSID Table: */
    if (strcmp(argv[1], "wsid") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_WSID_TBL, NULL, 0);

    }
#endif
}

#if 0
static void cmd_phy_config( s32 argc, char *argv[] )
{
	ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_PHY_ON, NULL, 0);
}

static void cmd_cal( s32 argc, char *argv[] )
{
	 u32 channel_index= 0;
	 channel_index=(u32)ssv6xxx_atoi(argv[1]);
	 ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_CAL, &channel_index, sizeof(u32));
}
#endif

#if CONFIG_STATUS_CHECK

extern void stats_display(void);
extern void stats_p(void);
extern void stats_m(void);
extern void SSV_PBUF_Status(void);

extern ssv_type_u32 g_l2_tx_packets;
extern ssv_type_u32 g_l2_tx_copy;
extern ssv_type_u32 g_l2_tx_late;
extern ssv_type_u32 g_notpool;

extern ssv_type_u32 g_heap_used;
extern ssv_type_u32 g_heap_max;
extern ssv_type_u32 g_tx_allocate_fail;

#if USE_ICOMM_LWIP

void dump_protocol(void){
#if USE_ICOMM_LWIP
    stats_p();
#endif
    LOG_PRINTF("\tl2 tx pakets[%d]\r\n\tl2_tx_copy[%d]\r\n\tl2_tx_late[%d]\r\n\tbuf_not_f_pool[%d]\r\n\ttx_allocate_fail[%d]\r\n\t",
                    g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool, g_tx_allocate_fail);
}



void dump_mem(void){
#if USE_ICOMM_LWIP
    stats_m();
#endif
#if (CONFIG_USE_LWIP_PBUF == 0)
    SSV_PBUF_Status();
#endif
    LOG_PRINTF("\tg_heap_used[%lu] g_heap_max[%lu]\r\n", g_heap_used, g_heap_max);

}

#if CONFIG_MEMP_DEBUG
extern void dump_mem_pool(memp_t type);
extern void dump_mem_pool_pbuf(void);
extern void dump_active_tcp_pcb(void);
#endif
void cmd_tcpip_status (ssv_type_s32 argc, char *argv[]){

    if(argc >=2){

        if (ssv6xxx_strcmp(argv[1], "p") == 0) {
            dump_protocol();
            return;
        }
        else if (ssv6xxx_strcmp(argv[1], "m") == 0) {
            dump_mem();
            return;
        }
#if CONFIG_MEMP_DEBUG
        else if(ssv6xxx_strcmp(argv[1], "mp") == 0){

            dump_mem_pool(ssv6xxx_atoi(argv[2]));
            return;
        }
        else if(ssv6xxx_strcmp(argv[1], "mpp") == 0){
            dump_mem_pool_pbuf();

             LOG_PRINTF("\tl2 tx pakets[%d]\r\n\tl2_tx_copy[%d]\r\n\tl2_tx_late[%d]\r\n\tbuf_not_f_pool[%d]\r\n\ttx_allocate_fail[%d]\r\n",
                    g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool, g_tx_allocate_fail);
            return;
        }
        else if (ssv6xxx_strcmp(argv[1],"pcb") == 0){
            dump_active_tcp_pcb();
            return;
        }

#endif//#if CONFIG_MEMP_DEBUG



    }

    dump_protocol();
    dump_mem();
}
#endif//CONFIG_STATUS_CHECK
#endif


ssv_type_u8 cmd_top_enable=0;
void cmd_top (ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 cpu_isr=0;
    ssv_type_u32 on=0;
    if(argc!=2){
        goto usage;
    }

    on=ssv6xxx_atoi(argv[1]);
    cpu_isr=OS_EnterCritical();
    cmd_top_enable=on;
    OS_ExitCritical(cpu_isr);
    return;

usage:
    LOG_PRINTF("usage\r\n");
    LOG_PRINTF("top 2  <--enable cpu usage log\r\n");
    LOG_PRINTF("top 1  <--enable task and cpu usage profiling\r\n");
    LOG_PRINTF("top 0  <--disable\r\n");
}


#if(BUS_TEST_MODE == 0)
typedef enum {
    EN_TX_START=0x36,
    EN_RX_START,
    EN_GOING,
    EN_END
}EN_BUS_THROUTHPUT_STATUS;

typedef struct{
    EN_BUS_THROUTHPUT_STATUS status;
    union{
        ssv_type_u32 TotalSize;
        ssv_type_u32 Size;
    }un;
}ST_BUS_THROUTHPUT;
//extern ssv_type_bool _wait_tx_resource(u32 frame_len);
ssv_type_u32 loop_times=0;
#define PER_SIZE 1460
#define TOTAL_SIZE (PER_SIZE*loop_times)
extern struct Host_cfg g_host_cfg;
static void _cmd_bus_tput_tx(void)
{

    HDR_HostEvent *evt=NULL;
    struct cfg_host_cmd *host_cmd=NULL;
    ssv_type_u8* frame=NULL;
    ST_BUS_THROUTHPUT *pstTput=NULL;
    ssv_type_s32 send_size=0;
    ssv_type_u32 start_tick=0;
    ssv_type_u32 end_tick=0;
    ssv_type_u32 seq_no=0;
    seq_no=0;

    /* =======================================================
                     Send start cmd
    =======================================================*/
    LOG_PRINTF("Send Start cmd\r\n");
    frame = os_frame_alloc(sizeof(ST_BUS_THROUTHPUT)+HOST_CMD_HDR_LEN,FALSE);
    host_cmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);
    host_cmd->c_type = ssv_hal_get_frame_type(SSV_FRM_HOST_CMD);
    host_cmd->h_cmd=SSV6XXX_HOST_CMD_BUS_THROUGHPUT_TEST ;
    host_cmd->len = HOST_CMD_HDR_LEN+sizeof(ST_BUS_THROUTHPUT);
    seq_no++;
    host_cmd->cmd_seq_no=seq_no;
    pstTput=(ST_BUS_THROUTHPUT *)host_cmd->un.dat8;
    pstTput->status=EN_TX_START;
    pstTput->un.TotalSize=TOTAL_SIZE;
    if(!g_host_cfg.hci_aggr_tx)
    {
#ifndef __SSV_UNIX_SIM__
        ssv6xxx_drv_wait_tx_resource(OS_FRAME_GET_DATA_LEN(frame));
#endif
        ssv6xxx_drv_send(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame));
        os_frame_free(frame);
    }
    else
    {
        //Send to tx driver task
        if(false == TXRXTask_FrameEnqueue(frame, 0))
        {
            LOG_PRINTF("[but t]snd cmd fail\r\n");
            os_frame_free(frame);
        }
    }

    /* =======================================================
                     Send Data
    =======================================================*/
    start_tick=OS_GetSysTick();
    send_size=TOTAL_SIZE;
    do{
        frame = os_frame_alloc(PER_SIZE,FALSE);
        if(frame)
        {
            seq_no++;
            host_cmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);
            host_cmd->c_type = ssv_hal_get_frame_type(SSV_FRM_HOST_CMD);
            host_cmd->h_cmd=SSV6XXX_HOST_CMD_BUS_THROUGHPUT_TEST ;
            host_cmd->len = PER_SIZE ;

            host_cmd->cmd_seq_no=seq_no;
            pstTput=(ST_BUS_THROUTHPUT *)host_cmd->un.dat8;
            pstTput->status=EN_GOING;
            pstTput->un.Size=PER_SIZE;
            if(!g_host_cfg.hci_aggr_tx)
            {
#ifndef __SSV_UNIX_SIM__
                ssv6xxx_drv_wait_tx_resource(OS_FRAME_GET_DATA_LEN(frame));
#endif
                ssv6xxx_drv_send(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame));
                os_frame_free(frame);
            }
            else
            {
                //Send to tx driver task
                //LOG_PRINTF("seq=%d\r\n",seq_no);
                if(false == TXRXTask_FrameEnqueue(frame, 0))
                {
                    LOG_PRINTF("[2but t]snd cmd fail\r\n");
                    os_frame_free(frame);
                }
            }
            send_size-=PER_SIZE;
            //LOG_PRINTF("done\r\n");
        }
        else
        {
            OS_TickDelay(1);
        }
    }while(send_size!=0);

    /* =======================================================
                     Get Status
    =======================================================*/
    //LOG_PRINTF("Get Status,seq_no=%d\r\n\r\n",seq_no);
    while((frame = os_frame_alloc(PER_SIZE,FALSE))==NULL){;};
    while(-1==ssv6xxx_drv_recv(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame))){;}
    evt=(HDR_HostEvent *)OS_FRAME_GET_DATA(frame);
    pstTput=(ST_BUS_THROUTHPUT *)evt->dat;
    os_frame_free(frame);
    end_tick=OS_GetSysTick();
    LOG_PRINTF("Test Done \r\n");
    LOG_PRINTF("TOTAL_SIZE=%d,Total Time=%d\r\n",TOTAL_SIZE,(end_tick-start_tick)*OS_TICK_RATE_MS);
    LOG_PRINTF("Throughput=%d KBps (%d Mbps) \r\n",(TOTAL_SIZE/((end_tick-start_tick)*OS_TICK_RATE_MS)),(((TOTAL_SIZE/((end_tick-start_tick)*OS_TICK_RATE_MS))*8)/1024));

}

static void _cmd_bus_tput_rx(void)
{

    HDR_HostEvent *evt=NULL;
    struct cfg_host_cmd *host_cmd=NULL;
    ssv_type_u8* frame=NULL;
    ST_BUS_THROUTHPUT *pstTput=NULL;
    ssv_type_u32 receive_size=0;
    ssv_type_s32 rlen=0;
    ssv_type_u32 start_tick=0,recv_tick=0;
    ssv_type_u32 end_tick=0;

    /* =======================================================
                     Send start cmd
    =======================================================*/
    LOG_PRINTF("Send Start cmd\r\n");
    frame = os_frame_alloc(sizeof(ST_BUS_THROUTHPUT)+HOST_CMD_HDR_LEN, FALSE);
    host_cmd = (struct cfg_host_cmd *)OS_FRAME_GET_DATA(frame);
    host_cmd->c_type = ssv_hal_get_frame_type(SSV_FRM_HOST_CMD);
    host_cmd->h_cmd=SSV6XXX_HOST_CMD_BUS_THROUGHPUT_TEST ;
    host_cmd->len = sizeof(ST_BUS_THROUTHPUT)+HOST_CMD_HDR_LEN;
    pstTput=(ST_BUS_THROUTHPUT *)host_cmd->un.dat8;
    pstTput->status=EN_RX_START;
    pstTput->un.TotalSize=TOTAL_SIZE;
#ifndef __SSV_UNIX_SIM__
    ssv6xxx_drv_wait_tx_resource(OS_FRAME_GET_DATA_LEN(frame));
#endif
    ssv6xxx_drv_send(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame));
    os_frame_free(frame);


    /* =======================================================
                     Get Data
    =======================================================*/
    start_tick=OS_GetSysTick();
    recv_tick = start_tick;
    do{
        frame = os_frame_alloc(PER_SIZE,FALSE);
        rlen=ssv6xxx_drv_recv(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame));
        if(-1!=rlen){
            evt=(HDR_HostEvent *)OS_FRAME_GET_DATA(frame);
            pstTput=(ST_BUS_THROUTHPUT *)evt->dat;
            receive_size+=PER_SIZE;
            recv_tick = OS_GetSysTick();
        }
        os_frame_free(frame);
        if (((OS_GetSysTick() - recv_tick) * OS_TICK_RATE_MS) > 10000)
        {
            LOG_PRINTF("Get Data timeout\r\n");
            goto END;
        }
    }while(receive_size!=TOTAL_SIZE);

    LOG_PRINTF("Get Data\r\n");

    /* =======================================================
                     Get Status
    =======================================================*/
    frame = os_frame_alloc(PER_SIZE,FALSE);
    recv_tick = OS_GetSysTick();
    while(-1==ssv6xxx_drv_recv(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame)))
    {
        if (((OS_GetSysTick() - recv_tick) * OS_TICK_RATE_MS) > 10000)
        {
            os_frame_free(frame);
            LOG_PRINTF("Get Status timeout\r\n");
            goto END;
        }
    }
    evt=(HDR_HostEvent *)OS_FRAME_GET_DATA(frame);
    pstTput=(ST_BUS_THROUTHPUT *)evt->dat;
    os_frame_free(frame);
    end_tick=OS_GetSysTick();
    LOG_PRINTF("Test Done\r\n");
    LOG_PRINTF("TOTAL_SIZE=%d,Total Time=%d\r\n",TOTAL_SIZE,(end_tick-start_tick)*OS_TICK_RATE_MS);
    if ((end_tick-start_tick) > 0)
    {
    LOG_PRINTF("Throughput=%d KBps (%d Mbps) \r\n",(TOTAL_SIZE/((end_tick-start_tick)*OS_TICK_RATE_MS)),((TOTAL_SIZE/((end_tick-start_tick)*OS_TICK_RATE_MS))*8)/1024);
    }
    else
    {
        LOG_PRINTF("Throughput>=%d KBps (%d Mbps) \r\n",(TOTAL_SIZE/(1*OS_TICK_RATE_MS)),((TOTAL_SIZE/(1*OS_TICK_RATE_MS))*8)/1024);
    }

END:
	return;
}
#endif

#if ((BUS_TEST_MODE == 1) && (SDRV_INCLUDE_SDIO == 1))
extern ssv_type_u32 dataIOPort,regIOPort;
extern ssv_type_bool sdio_if_init(void);
extern ssv_type_bool sdio_if_write_sram(ssv_type_u32 addr, ssv_type_u8 *data, ssv_type_u32 size);
extern ssv_type_u32 sdio_if_read_reg(ssv_type_u32 addr);
#endif
static void cmd_bus_tput(ssv_type_s32 argc, char *argv[])
{

#if (BUS_TEST_MODE == 0)
    if(argc!=3)
    {
        LOG_PRINTF("usage:\r\n");
        LOG_PRINTF("     bus -t [loop times]\r\n");
        LOG_PRINTF("     bus -r [loop times]\r\n");
        return;
    }
#else
    ssv_type_u8  w_buffer[192], r_buffer[192];
    ssv_type_u32 sram_addr = 0x00000000;
    ssv_type_u32 i, j, addr,temp;
#endif

#if ((BUS_TEST_MODE == 1) && (SDRV_INCLUDE_SDIO == 1))
    if(argc!=3)
    {
        LOG_PRINTF("usage:\r\n");
        LOG_PRINTF("     bus sdio [cmd52/cmd53]\r\n");/* bus sdio cmd52 / bus sdio cmd53 */
        return;
    }
#endif

#if ((BUS_TEST_MODE == 1) && (SDRV_INCLUDE_SPI == 1))
    if(argc!=2)
    {
        LOG_PRINTF("usage:\r\n");
        LOG_PRINTF("     bus spi\r\n");
        return;
    }
#endif

#if (BUS_TEST_MODE == 0)
    ssv6xxx_drv_irq_disable(false);
    ssv_hal_rf_disable();
    //cpu_sr=OS_EnterCritical();
    loop_times = ssv6xxx_atoi_base(argv[2], 10);
    if(ssv6xxx_strcmp(argv[1], "-t") == 0)
    {
        _cmd_bus_tput_tx();
    }

    if(ssv6xxx_strcmp(argv[1], "-r") == 0)
    {
        _cmd_bus_tput_rx();
    }
    //OS_ExitCritical(cpu_sr);
    ssv_hal_rf_enable();
    ssv6xxx_drv_irq_enable(false);
#endif

#if ((BUS_TEST_MODE == 1) && (SDRV_INCLUDE_SDIO == 1))
    if(ssv6xxx_strcmp(argv[1], "sdio") == 0)
    {
        if(ssv6xxx_strcmp(argv[2], "cmd52") == 0)
        {
            sdio_if_init();
            if((dataIOPort == 0x10000)&&(regIOPort == 0x10020))
            {
                LOG_PRINTF("Sdio cmd52 test ok ! \r\n");
            }
            else
            {
                LOG_PRINTF("Sdio cmd52 test error ! \r\n");
            }
        }

        if(ssv6xxx_strcmp(argv[2], "cmd53") == 0)
        {
            for(i=0; i<192; i+=4)
            {
               w_buffer[i]= 0xA5;
               w_buffer[i+1]= 0xA5;
               w_buffer[i+2]= 0x00;
               w_buffer[i+3]= 0xFF;
            }
            if(FALSE == sdio_if_write_sram(sram_addr,w_buffer,192))
            {
                LOG_PRINTF("Sdio cmd53 write error ! \r\n");
                return;
            }

            for (i=0, j=0, addr=sram_addr; i<48, j<192; i++, j+=4, addr+=4)
            {
                 temp = sdio_if_read_reg(addr);
                 r_buffer[j] = ((temp >> 0) & 0XFF);
                 r_buffer[j+1] = ((temp >> 8) & 0XFF);
                 r_buffer[j+2] = ((temp >> 16) & 0XFF);
                 r_buffer[j+3] = ((temp >> 24) & 0XFF);
            }
            for(i=0; i<192; i++)
            {
                if(w_buffer[i] != r_buffer[i])
                {
                    LOG_PRINTF("Sdio cmd53 read error, i = %d %02x<>%02x! \r\n", i, w_buffer[i], r_buffer[i]);
                    return;
                }
            }
            LOG_PRINTF("Sdio cmd53 test OK  ! \r\n");

        }
    }
#endif

#if ((BUS_TEST_MODE == 1) && (SDRV_INCLUDE_SPI == 1))
    if(ssv6xxx_strcmp(argv[1], "spi") == 0)
    {
        ssv_type_u32 *data = NULL;

        for(i=0; i<192; i+=4)
        {
           w_buffer[i]= 0xA5;
           w_buffer[i+1]= 0xA5;
           w_buffer[i+2]= 0x00;
           w_buffer[i+3]= 0xFF;
        }

        for (i = 0; i < 192; i++)
        {
            data = (ssv_type_u32 *)(&w_buffer[i*4]);
            if(FALSE==ssv6xxx_drv_write_reg(sram_addr, *data))
            {
                LOG_PRINTF("Spi test error ! \r\n");
                return;
            }
            data++;
            sram_addr += 4;
        }

        sram_addr = 0x00000000;
        for (i=0, j=0, addr=sram_addr; i<48, j<192; i++, j+=4, addr+=4)
        {
             temp = ssv6xxx_drv_read_reg(addr);
             r_buffer[j] = ((temp >> 0) & 0XFF);
             r_buffer[j+1] = ((temp >> 8) & 0XFF);
             r_buffer[j+2] = ((temp >> 16) & 0XFF);
             r_buffer[j+3] = ((temp >> 24) & 0XFF);
        }
        for(i=0; i<192; i++)
        {
            if(w_buffer[i] != r_buffer[i])
            {
                LOG_PRINTF("Spi read error ! \r\n");
                return;
            }
        }
        LOG_PRINTF("Spi test OK  ! \r\n");
    }
#endif
    return;
}

#if (CONFIG_BUS_LOOPBACK_TEST)
OsSemaphore busLoopBackSem;
void *pLoopBackOut=NULL;
void *pLoopBackIn=NULL;
//#define LOOPBACK_TEST_DATA_LEN (1460)
ssv_type_u32 LOOPBACK_TEST_DATA_LEN =1460;
void bus_loopback(int argc, char **argv)
{
    ssv_type_u32 loop=1;
    ssv_type_bool forever=FALSE;
    ssv_type_u32 count=0;
    if(argc>3){
        LOG_PRINTF("usage: bl 1 or bl 1 512\r\n");
        LOG_PRINTF("     bl [loop times][len]\r\n");
        LOG_PRINTF("     loop times: If set 0, it means forever\r\n");
        LOG_PRINTF("     len: default 1460 if no specify\r\n");
        return;
    }
    loop=ssv6xxx_atoi(argv[1]);
    if(argc>2)
    {
        LOOPBACK_TEST_DATA_LEN=ssv6xxx_atoi(argv[2]);
        if((!LOOPBACK_TEST_DATA_LEN)||(LOOPBACK_TEST_DATA_LEN>1460))
            LOOPBACK_TEST_DATA_LEN = 1460;
    }
    else
    {
        LOOPBACK_TEST_DATA_LEN = 1460;
    }

    if(loop==0) forever=TRUE;

    if(OS_FAILED==OS_SemInit(&busLoopBackSem,1,0)){
        LOG_PRINTF("Init Sem fail\r\n");
        return;
    }

    pLoopBackOut=os_frame_alloc(LOOPBACK_TEST_DATA_LEN,FALSE);
    if(pLoopBackOut==NULL){
        LOG_PRINTF("Allocate for output buf fail\r\n");
        return;
    }

    LOG_PRINTF("bl loop=%d, len=%d \r\n",loop,LOOPBACK_TEST_DATA_LEN);
    ssv6xxx_memset((void *)OS_FRAME_GET_DATA(pLoopBackOut),1,OS_FRAME_GET_DATA_LEN(pLoopBackOut));

    pLoopBackIn=os_frame_alloc(LOOPBACK_TEST_DATA_LEN,FALSE);
    if(pLoopBackIn==NULL){
        os_frame_free(pLoopBackOut);
        LOG_PRINTF("Allocate for input buf fail\r\n");
        return;
    }
    ssv6xxx_memset((void *)OS_FRAME_GET_DATA(pLoopBackIn),0,OS_FRAME_GET_DATA_LEN(pLoopBackIn));
    ssv_hal_rf_disable();
    while(loop||(forever==TRUE)){
        count++;
        LOG_PRINTF("get random \r\n");
       ssv_hal_gen_rand(OS_FRAME_GET_DATA(pLoopBackOut),OS_FRAME_GET_DATA_LEN(pLoopBackOut));

        *(ssv_type_u32 *)OS_FRAME_GET_DATA(pLoopBackOut)=count;
        LOG_PRINTF("Send Host Cmd\r\n");
        ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_BUS_LOOPBACK_TEST, (void *)OS_FRAME_GET_DATA(pLoopBackOut), OS_FRAME_GET_DATA_LEN(pLoopBackOut));
        LOG_PRINTF("Wait sem\r\n");
        if (OS_SemWait(busLoopBackSem, OS_MS2TICK(3000)) == OS_FAILED)
        {
            LOG_PRINTF("Wait Timeout\r\n");
            break;
        }

        LOG_PRINTF("Get sem\r\n");
        if(0!=ssv6xxx_memcmp((void *)OS_FRAME_GET_DATA(pLoopBackOut), (void *)OS_FRAME_GET_DATA(pLoopBackIn), OS_FRAME_GET_DATA_LEN(pLoopBackOut))){
            LOG_PRINTF("Dump output data. len=%d\r\n",OS_FRAME_GET_DATA_LEN(pLoopBackOut));
            hex_dump((void *)OS_FRAME_GET_DATA(pLoopBackOut), OS_FRAME_GET_DATA_LEN(pLoopBackOut));
            LOG_PRINTF("Dump input data. len=%d \r\n",OS_FRAME_GET_DATA_LEN(pLoopBackIn));
            hex_dump((void *)OS_FRAME_GET_DATA(pLoopBackIn), OS_FRAME_GET_DATA_LEN(pLoopBackIn));
            break;
        }else{
            LOG_PRINTF("Compare pass. count=%d\r\n",count);
        }
        if(forever==FALSE)
            loop--;
    }

    OS_SemDelete(busLoopBackSem);
    os_frame_free(pLoopBackOut);
    os_frame_free(pLoopBackIn);
    ssv_hal_rf_enable();
    return;
}
static void cmd_bus_loopback(ssv_type_s32 argc, char *argv[])
{
    #if NETAPP_SUPPORT
    net_app_run(argc, argv);
    #else
    bus_loopback(argc,argv);
    #endif
}
#endif

#if ((defined(SDRV_INCLUDE_SPI)) && (SDRV_INCLUDE_SPI))
extern ssv_type_u32 gSpiZeroLenCount;
extern ssv_type_u32 gSpiZeroLenRetryCount;
extern ssv_type_u32 gSpiReadCount;
//extern u32 RxTaskRetryCount[32];
static void cmd_spi_status(ssv_type_s32 argc, char *argv[])
{
    //int i=0;

    LOG_PRINTF("SPI total reading count is %1d\r\n",gSpiReadCount);
    LOG_PRINTF("The count of zero length is %d\r\n",gSpiZeroLenCount);
    LOG_PRINTF("The count of spi status double checking is %d\r\n",gSpiZeroLenRetryCount);
    #if 0
    for(i=0;i<32;i++){
        if(RxTaskRetryCount[i]!=0){
            LOG_PRINTF("RxTaskRetryCount[%d]: %d\r\n",i,RxTaskRetryCount[i]);
        }
    }
    #endif
    return;

}
#endif

#if ((defined(SDRV_INCLUDE_SDIO)) && (SDRV_INCLUDE_SDIO))
extern int ssv6xxx_sdio_tuning_initialization(void *func, const void *id);
static void cmd_sdio_tune(ssv_type_s32 argc, char *argv[])
{
    ssv6xxx_sdio_tuning_initialization(0,0);

}
#endif
static void cmd_dq_utility(ssv_type_s32 argc, char *argv[])
{
    struct cfg_dq_status dqStatus;
    struct cfg_dq_lock dqlock;
    if(argc==3){
        if(0==ssv6xxx_strcmp("status",argv[1])){
            dqStatus.wsid=ssv6xxx_atoi(argv[2]);
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_DQ_STATUS, &dqStatus, sizeof(struct cfg_dq_status));
            return;
        }
    }else if(argc==4){
        if(0==ssv6xxx_strcmp("lock",argv[1])){
            dqlock.wsid=ssv6xxx_atoi(argv[2]);
            dqlock.lock=ssv6xxx_atoi(argv[3]);
            LOG_PRINTF("dq[%d] %s\r\n",dqlock.wsid,((dqlock.lock==TRUE)?"LOCK":"UNLOCK"));
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_DQ_LOCK, &dqlock, sizeof(struct cfg_dq_lock));
            return;
        }

    }
   LOG_PRINTF("Usage: \r\n");
   LOG_PRINTF("     dq status [wsid] \r\n");
   LOG_PRINTF("     dq lock [wsid] [0:unlock, 1:lock] \r\n");
   return;
}

static void cmd_recovery(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u8 vif_idx;
    if(netmgr_wifi_check_sta(&vif_idx)){
        LOG_PRINTF("STA recover\r\n");
        ssv6xxx_wifi_sta_recover(vif_idx);
    }else if(netmgr_wifi_check_ap(&vif_idx)){
        LOG_PRINTF("AP recover\r\n");
        ssv6xxx_wifi_ap_recover(vif_idx);
    }else{
        LOG_PRINTF("Wrong mode\r\n");
    }
}
#if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
extern ssv_type_bool dynamic_cci_update;
static void cmd_dynamic_cci_update(ssv_type_s32 argc, char *argv[])
{
    if(argc!=2)
    {
        goto USAGE;
    }

    if(0==ssv6xxx_strcmp("1",argv[1])){
        dynamic_cci_update=TRUE;
        LOG_PRINTF("Enable the dynamic cci update\r\n");
    }
    if(0==ssv6xxx_strcmp("0",argv[1])){
        dynamic_cci_update=FALSE;
        LOG_PRINTF("Disable the dynamic cci update\r\n");
    }
    return ;
USAGE:
    LOG_PRINTF("Usage:\r\n");
    LOG_PRINTF("     dcu 1\r\n");
    LOG_PRINTF("     dcu 0\r\n");
    return ;
}
#endif
extern struct Host_cfg g_host_cfg;
static void cmd_cfg(ssv_type_s32 argc, char *argv[])
{

    if (ssv6xxx_strcmp(argv[1], "ts")==0) {
        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_tx_task_sleep(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_tx_task_sleep(0);
            }
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ts 1\r\n");
            LOG_PRINTF("     cfg ts 0\r\n");
        }

    } else if (ssv6xxx_strcmp(argv[1], "ts_tick")==0) {
        if (argc >= 3)
        {
           ssv6xxx_wifi_set_tx_task_sleep_tick(ssv6xxx_atoi(argv[2]));
           return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ts_tick [tick]\r\n");
        }

    } else if (ssv6xxx_strcmp(argv[1], "tx_retry_cnt")==0) {
        if (argc >= 3)
        {
           ssv6xxx_wifi_set_tx_task_retry_cnt(ssv6xxx_atoi(argv[2]));
           return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg tx_retry_cnt [cnt]\r\n");
        }

    }else if (ssv6xxx_strcmp(argv[1], "erp")==0) {

        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_ap_erp(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_ap_erp(0);
            }
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg erp 1\r\n");
            LOG_PRINTF("     cfg erp 0\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "resp")==0) {

        if (argc >= 4)
        {
            ssv_type_u8 txp = ssv6xxx_atoi(argv[2]);
            ssv_type_u8 rxp = ssv6xxx_atoi(argv[3]);

            ssv6xxx_wifi_set_trx_res_page(txp,rxp);

            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg respage txp rxp\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "spmb")==0) {

        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2])){
                ssv6xxx_wifi_set_ap_short_preamble(1);
            }
            if(0==ssv6xxx_strcmp("0",argv[2])){
                ssv6xxx_wifi_set_ap_short_preamble(0);
            }

            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg spmb 0\r\n");
            LOG_PRINTF("     cfg spmb 1\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "txpwr")==0) {

            if (argc >= 3)
            {
                ssv_type_u8 pwr_mode = ssv6xxx_atoi(argv[2]);
                ssv6xxx_wifi_set_tx_pwr_mode(pwr_mode);
                return ;
            }
            else
            {
                LOG_PRINTF("Usage:\r\n");
                LOG_PRINTF("     cfg txpwr 0/1/2\r\n");
            }

    }else if (ssv6xxx_strcmp(argv[1], "rxsrate")==0) {

        if (argc >= 4)
        {
            ssv_type_u16 rx_bas_srate = (ssv_type_u16)ssv6xxx_atoi(argv[2]);
            ssv_type_u16 rx_mcs_srate = (ssv_type_u16)ssv6xxx_atoi(argv[3]);
            ssv6xxx_wifi_set_ap_rx_support_rate(rx_bas_srate,rx_mcs_srate);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg rxsrate basrate(0xfff) mcsrate(0xfff)\r\n");
        }

    }else if (ssv6xxx_strcmp(argv[1], "volt")==0) {

        if (argc >= 3)
        {
            ssv_type_u32 volt_mode = ssv6xxx_atoi(argv[2]);
            ssv6xxx_set_voltage_mode(volt_mode);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:0=LDO, 1=DCDC\r\n");
            LOG_PRINTF("     cfg volt 0/1\r\n");
        }
    }else if(ssv6xxx_strcmp(argv[1], "5G")==0){
        if (argc >= 3)
        {
            ssv_type_bool rf_5g_band = ssv6xxx_atoi(argv[2]);
            ssv6xxx_wifi_enable_5g_band(rf_5g_band);
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg 5G 0\r\n");
            LOG_PRINTF("     cfg 5G 1\r\n");
        }
    }else if(ssv6xxx_strcmp(argv[1], "ap_no_dfs")==0){
        if (argc >= 3)
        {
            ssv_type_u8 ap_no_dfs = ssv6xxx_atoi(argv[2]);
            ssv6xxx_wifi_set_ap_no_dfs(ap_no_dfs);
            return;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ap_no_dfs 0\r\n");
            LOG_PRINTF("     cfg ap_no_dfs 1\r\n");
        }

    }else if (ssv6xxx_strcmp(argv[1], "hciagg")==0){
        if (argc >= 4)
        {
            ssv_type_u8 value;
            if(0==ssv6xxx_strcmp("t",argv[2])){
                value = ssv6xxx_atoi(argv[3]);
                ssv6xxx_wifi_set_hci_aggr(SSV_HCI_TX,value);
                 g_host_cfg.hci_aggr_tx = value;
            }
            if(0==ssv6xxx_strcmp("r",argv[2])){
                value = ssv6xxx_atoi(argv[3]);
                ssv6xxx_wifi_set_hci_aggr(SSV_HCI_RX,value);
                 g_host_cfg.hci_rx_aggr = value;
            }
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg hciagg t/r 0/1\r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "no_bcn_to")==0){
        ssv6xxx_wifi_set_sta_no_bcn_timeout(ssv6xxx_atoi(argv[2]));
    }else if(ssv6xxx_strcmp(argv[1], "ap_bw")==0){
        if (argc >= 3)
        {
            if(ssv6xxx_strcmp(argv[2], "0")==0)
            {
                ssv6xxx_wifi_set_ap_bw(AP_HT20_ONLY);
            }
            else if(ssv6xxx_strcmp(argv[2], "1")==0)
            {
                ssv6xxx_wifi_set_ap_bw(AP_HT40_ONLY);
            }
            else
            {
                ssv6xxx_wifi_set_ap_bw(AP_HT2040);
            }
            return;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg ap_bw [0/1/2]\r\n");
            LOG_PRINTF("     0: HT20 only\r\n");
            LOG_PRINTF("     1: HT40 only\r\n");
            LOG_PRINTF("     2: HT2040   \r\n");
        }
    }else if (ssv6xxx_strcmp(argv[1], "krack")==0){

        if (argc >= 3)
        {
            if(ssv6xxx_strcmp(argv[2], "0")==0)
            {
                ssv6xxx_wifi_krack_patch(FALSE);
            }
            else
            {
                ssv6xxx_wifi_krack_patch(TRUE);
            }
            return;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     cfg krach [1/0]\r\n");
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "show")==0) {
            LOG_PRINTF("TX rate mask=0x%x\r\n",g_host_cfg.rate_mask);
            LOG_PRINTF("TX rc per U=%d, D=%d\r\n",g_host_cfg.upgrade_per,g_host_cfg.downgrade_per);
            LOG_PRINTF("TX rc resent=%d\r\n",g_host_cfg.resent_fail_report);
            LOG_PRINTF("TX rc direct down=%d\r\n",g_host_cfg.direct_rc_down);
            LOG_PRINTF("TX rc drate endian=%d\r\n",g_host_cfg.rc_drate_endian);
            LOG_PRINTF("TX pwr mode = %d\r\n",g_host_cfg.tx_power_mode);
            LOG_PRINTF("TX task sleep = %d\r\n",g_host_cfg.tx_sleep);
            LOG_PRINTF("TX task sleep tick = %d\r\n",g_host_cfg.tx_sleep_tick);
            LOG_PRINTF("TX task retry cnt = %d\r\n",g_host_cfg.tx_retry_cnt);
            LOG_PRINTF("AP erp = %d\r\n",g_host_cfg.erp);
            LOG_PRINTF("AP b mode short preamble = %d\r\n",g_host_cfg.b_short_preamble);
            LOG_PRINTF("Resource page T=%d, R=%d\r\n",g_host_cfg.tx_res_page,g_host_cfg.rx_res_page);
            LOG_PRINTF("Host pbuf num = %d\r\n",g_host_cfg.pool_size);
            LOG_PRINTF("Host sec pbuf num = %d\r\n",g_host_cfg.pool_sec_size);
            LOG_PRINTF("Support ht = %d\r\n",g_host_cfg.support_ht);
            LOG_PRINTF("Support RF band = %s\r\n",(ssv6xxx_wifi_support_5g_band()?"5G":"2G"));
            LOG_PRINTF("AP no DFS = %d\r\n",g_host_cfg.ap_no_dfs);
            LOG_PRINTF("AP Bandwidth= %d\r\n", g_host_cfg.ap_bw);
            LOG_PRINTF("STA no bcn timeout = %d(ms)\r\n",g_host_cfg.sta_no_bcn_timeout*500);
            LOG_PRINTF("hci_aggr_tx = %d\r\n",g_host_cfg.hci_aggr_tx);
            LOG_PRINTF("hci_rx_aggr = %d\r\n",g_host_cfg.hci_rx_aggr);
            LOG_PRINTF("ampdu_tx = %d\r\n",g_host_cfg.ampdu_tx_enable);
            LOG_PRINTF("amdpu_rx = %d\r\n",g_host_cfg.ampdu_rx_enable);
            LOG_PRINTF("krack patch = %d\r\n",g_host_cfg.krack_patch);
    }
    return ;
}

#if USE_ICOMM_LWIP
extern ssv_type_u8 g_lwip_tcp_ignore_cwnd;
static void cmd_lwip(ssv_type_s32 argc, char *argv[])
{
    if (ssv6xxx_strcmp(argv[1], "icwnd")==0)
    {
        if (argc >= 3)
        {
            if(0==ssv6xxx_strcmp("1",argv[2]))
            {
                g_lwip_tcp_ignore_cwnd=1;
            }

            if(0==ssv6xxx_strcmp("0",argv[2]))
            {
                g_lwip_tcp_ignore_cwnd=0;
            }
            return ;
        }
        else
        {
            LOG_PRINTF("Usage:\r\n");
            LOG_PRINTF("     lwip icwnd 1\r\n");
            LOG_PRINTF("     lwip icwnd 0\r\n");
        }
    }
    else if (ssv6xxx_strcmp(argv[1], "default")==0)
    {
        LOG_PRINTF("default if:%s\r\n",argv[2]);
        netdev_set_default(argv[2]);
    }
    else if (ssv6xxx_strcmp(argv[1], "setip")==0)
    {
     //lwip setip[1] vif_idx[2] update_dhcped[3] ip[4] netmask[5] gw[6] dns[7]
        ipinfo info;
        ssv_type_u8 vif_idx = ssv6xxx_atoi(argv[2]);
        ssv_type_bool update_dhcpd = ssv6xxx_atoi(argv[3]);


        LOG_PRINTF("vif_idx:%d\r\n",vif_idx);
        LOG_PRINTF("update_dhcpd:%d\r\n",update_dhcpd);
        LOG_PRINTF("ip:%s\r\n",argv[4]);
        LOG_PRINTF("netmask:%s\r\n",argv[5]);
        LOG_PRINTF("gateway:%s\r\n",argv[6]);
        LOG_PRINTF("dns:%s\r\n",argv[7]);

        info.ipv4 = netstack_inet_addr(argv[4]);
        info.netmask = netstack_inet_addr(argv[5]);
        info.gateway = netstack_inet_addr(argv[6]);
        info.dns = netstack_inet_addr(argv[7]);

        netmgr_ipinfo_set(vif_idx, &info, update_dhcpd);
    }
}
#endif

static void cmd_temperature(ssv_type_s32 argc, char *argv[])
{
   ssv_type_u8 sar_code;
   signed char temperature;
   ssv_hal_get_temperature(&sar_code,&temperature);
   LOG_PRINTF("sar code=%d, temp. = %d\r\n",sar_code, temperature);
   return ;
}

#if (CONFIG_BUS_LOOPBACK_TEST)
static void cmd_reg_loop_back(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 rvalue=0;

    ssv_type_u32 addr=0;
    ssv_type_u32 value=0;
    ssv_type_u32 loop=0;
    ssv_type_u32 i=0;
    ssv_type_u8  forever=0;
    if(argc==4)
    {
        addr=(ssv_type_u32)ssv6xxx_atoi_base(argv[1],16);
        value=(ssv_type_u32)ssv6xxx_atoi_base(argv[2],16);
        loop=(ssv_type_u32)ssv6xxx_atoi_base(argv[3],16);
        if(loop==0)
        {
            forever=1;
            LOG_PRINTF("addr=0x%x, value=0x%x, loop=infinite\r\n",addr,value);
        }
        else
        {
            LOG_PRINTF("addr=0x%x, value=0x%x, loop=0x%x\r\n",addr,value,loop);
        }

        while(loop||forever)
        {
            i++;
            ssv6xxx_drv_write_reg(addr,value);
            rvalue=ssv6xxx_drv_read_reg(addr);

            if(rvalue!=value)
            {
                LOG_PRINTF("%dth test fail, rvalue=%x, value=%x\r\n",i, rvalue,value);
                break;
            }
            LOG_PRINTF("%dth ok,rvalue=%x, value=%x\r\n",i, rvalue,value);
            ssv6xxx_drv_write_reg(addr,value>>1);
            loop--;
        }
    }
    else
    {
        LOG_PRINTF("rl [address] [value] [loop times]\r\n");
        LOG_PRINTF("loop times: if set 0, it means forever\r\n");
        LOG_PRINTF("PS: C0000A10 is a dummy registers\r\n");
    }
    return;
}
#endif

static void cmd_read_hw_queue(ssv_type_s32 argc, char *argv[])
{
    ssv_hal_read_hw_queue();
    return;
}
void eco_cb(ssv_type_u32 nEvtId, void *data, ssv_type_s32 len, void* priv)
{
    if(SOC_EVT_PS_WAKENED==nEvtId)
    {
        LOG_PRINTF("WIFI Quit ECO mode\r\n");
    }

    return;
}

static void cmd_eco_mode(ssv_type_s32 argc, char *argv[])
{
    if(argc!=2)
    {
        LOG_PRINTF("eco [1/0]\r\n");
        LOG_PRINTF("1: run eco mode\r\n");
        LOG_PRINTF("0: quit eco mode\r\n");
        return;
    }

    ssv6xxx_wifi_reg_evt_cb(eco_cb,NULL);

    if(0==ssv6xxx_strcmp("1",argv[1]))
    {
        if(FALSE==ssv6xxx_wifi_set_eco_mode())
        {
            LOG_PRINTF("Run ECO mode fail\r\n");
        }
    }

    if(0==ssv6xxx_strcmp("0",argv[1]))
    {
        if(FALSE==ssv6xxx_wifi_wakeup())
        {
            LOG_PRINTF("Wakeup WIFI fail\r\n");
        }
    }

    return;
}
//gpio pin high/low
static void cmd_gpio(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 pin;
    ssv_type_u32 hi_lo;
    if(argc!=3)
    {
        LOG_PRINTF("gpio pin high/low\r\n");
        return;
    }


    pin=ssv6xxx_atoi(argv[1]);
    hi_lo=ssv6xxx_atoi(argv[2]);
    ssv6xxx_set_wakeup_bb_gpio(hi_lo,pin); //Wakeup BB low

    return;
}

static void cmd_setch(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u32 pin;
    //ssv_type_u32 hi_lo;
    //if(argc!=3)
    //{
    //    LOG_PRINTF("gpio pin high/low\r\n");
    //    return;
    //}


    pin=ssv6xxx_atoi(argv[1]);
    //hi_lo=ssv6xxx_atoi(argv[2]);
    ssv6xxx_wifi_set_channel(0 , pin,SSV6XXX_HWM_STA);

    return;
}
static void cmd_set_mcc_slice(ssv_type_s32 argc, char *argv[])
{
    ssv_type_u16 pin;
    ssv_type_u16 hi_lo;


    pin=ssv6xxx_atoi(argv[1]);
    hi_lo=ssv6xxx_atoi(argv[2]);
    ssv6xxx_wifi_set_mcc_time_slice(pin,hi_lo);

    return;
}


CLICmds gCliCmdTable[] =
{
    #ifdef __SSV_UNIX_SIM__
    { "exit",       cmd_exit,           "Terminate uWifi system."           },
    { "abort",       cmd_abort,           "force abort"           },
    #endif
//	{ "sdrv",       cmd_sdrv,           "SSV driver module commands."       },
    { "ifconfig",   cmd_ifconfig,       "Network interface configuration"   },
//    { "route",      cmd_route,          "Routing table manipulation"        },
#if(PING_SUPPORT==1)
    { "ping",       cmd_ping,           "ping"                              },
#endif
#ifdef __APP_TTCP__
    { "ttcp",       cmd_ttcp,           "ttcp"                              },
#endif
#if (IPERF3_ENABLE == 1)
      { "iperf3",      cmd_iperf3,         "throughput testing via tcp or udp"},
#endif
#if (NETAPP_SUPPORT == 1)
    { "netapp",     cmd_net_app,        "Network application utilities"     },
#endif
    { "netmgr",     cmd_net_mgr,        "Network Management utilities"     },

    { "iw",         cmd_iw,             "Wireless utility"                  },
    { "ctl",       cmd_ctl,           "wi-fi interface control (AP/station on or off)"       },
    { "r",          cmd_read,           "Read SoC"                          },
    { "w",          cmd_write,          "Write SoC"                         },

   // { "pattern",    cmd_pattern,        "Pattern mode"                      },

//	{ "setnetif",   cmd_setnetif,		"SetNetIf Ip Mask Gateway Mac"		},
	//{ "socket-c",   cmd_createsocketclient, "Create Socket client and set server addreass" },
//	{ "52w",        cmd_write_52,          "Write 52 dommand"               },
//	{ "52r",        cmd_read_52,          "Write 52 dommand"               },

#ifdef __AP_DEBUG__
   // { "ap"		,   cmd_ap				, "list ap mode info." 				},
   // { "test"		,cmd_test			, "send test frame"					},
#endif
#if USE_ICOMM_LWIP
    { "dump",       cmd_dump,           "Do dump."                          },
#endif
//	{ "phy-on",		cmd_phy_config,		"Load PHY configuration"			},
//    { "cal",        cmd_cal,			"Calibration Process"				},
//    { "phy",        Cmd_Phy,            "phy" },
//    { "delba",  	cmd_wifi_delba,		"DELBA"		                        },

#if CONFIG_STATUS_CHECK
    { "s",     cmd_tcpip_status,    "dump tcp/ip status"                },
#endif
    { "sys",       cmd_sys,           "Components info"       },
#if(OS_TASK_STAT_EN==1)
    {"top",cmd_top,"task profile"},
#endif
    {"bus",         cmd_bus_tput, "Bus throughput Test or verify" },
#if ((defined(SDRV_INCLUDE_SPI)) && (SDRV_INCLUDE_SPI))
    {"spi", cmd_spi_status,"check spi status"},
#endif
#if ((defined(SDRV_INCLUDE_SDIO)) && (SDRV_INCLUDE_SDIO))
    {"sdio", cmd_sdio_tune,"tune sdio parameter"},
#endif
    {"dq", cmd_dq_utility, "data queue utility" },
#if (CONFIG_BUS_LOOPBACK_TEST)
    {"bl", cmd_bus_loopback,"bus loopback test"},
    {"rl", cmd_reg_loop_back,"register loopback testing"},
#endif
    {"rc", cmd_rc,"Set phy rate upper bound"},
    {"recover",cmd_recovery,"Do sta recover"},
    {"mib",cmd_mib,"Show fw recovery count and timer interrupt count"},
    {"ampdu",cmd_ampdu,"ampdu tx related commands"},
    #if(ENABLE_DYNAMIC_RX_SENSITIVE==1)
    {"dcu",cmd_dynamic_cci_update,"dynamic update cci parameter on STA mode"},
    #endif
    {"cfg",cmd_cfg,"change host cfg setting"},
#if USE_ICOMM_LWIP
    {"lwip",cmd_lwip,"change the lwip setting"},
#endif
    {"tmpt",cmd_temperature,"get temperature"},
    {"q",cmd_read_hw_queue, "read hw queue"},
    {"eco",cmd_eco_mode,"eco mode testing"},
    {"gpio",cmd_gpio,"set gpio to output high/low"},
    {"ch",cmd_setch,"set gpio to output high/low"},
    {"mcc",cmd_set_mcc_slice,"set set_mcc_slice"},
    { NULL, NULL, NULL },
};
#endif
