/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <host_apis.h>
#include "cli_cmd_sys.h"
#include "txrx_task.h"
#include "ssv_drv.h"

#if (CLI_ENABLE==1)
char CmdEngModeString[3][8] = {"STOP", "RUNNING", "EXIT"};
#ifdef __SSV_UNIX_SIM__
char SysProfBuf[256];
signed char SysProfSt = -1;
char g_SysTaskStack[1024];
#endif
void usage(void)
{
    LOG_PRINTF("Usage:\r\n");
    LOG_PRINTF("      sys cmdeng st\r\n");
    LOG_PRINTF("      sys soc st\r\n");
    LOG_PRINTF("      sys msg\r\n");
    LOG_PRINTF("      sys reset\r\n");
    LOG_PRINTF("      sys txq\r\n");
	LOG_PRINTF("      sys log\r\n");
#ifdef __SSV_UNIX_SIM__
    LOG_PRINTF("      sys profile\r\n");
#endif
}


void log_usage(void)
{
    LOG_PRINTF("Usage:\r\n");
    LOG_PRINTF("      sys log show\r\n");
    LOG_PRINTF("      sys log reset\r\n");
    LOG_PRINTF("      sys log lev [0|1|2|3]\r\n");
    LOG_PRINTF("      sys log mod [add|rem] [module name]\r\n");
	LOG_PRINTF("      module name = [all|mem|socket|api|tcp|udp|ip|other|data|ap|sta|httpd|dhcpd|netmgr|cmdeng|txrx|sconfig]\r\n");
}


static void GetSocStatus(void)
{
	 ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_SOC_STATUS, NULL, 0);
}


#if SSV_LOG_DEBUG

extern ssv_type_u32 g_log_module;
extern ssv_type_u32 g_log_min_level;

static void ShowLog(void)
{
	 LOG_PRINTF("g_log_module=%08x \r\n", g_log_module);
     LOG_PRINTF("g_log_min_level=%08x \r\n", g_log_min_level);
}
#endif

extern ssv6xxx_result CmdEng_GetStatus(void *stp);
struct CliCmdEng_st
{
    ssv_type_u32 mode;
    ssv_type_u32 BlkCmdNum;
    ssv_type_bool BlkCmdIn;
};
void msg_evt_show(void);
void cmd_sys(ssv_type_s32 argc, char *argv[])
{
    if ( (argc == 2) && (ssv6xxx_strcmp(argv[1], "cmdeng") == 0))
    {
        if(ssv6xxx_strcmp(argv[2], "st") == 0)
        {
            struct CliCmdEng_st st;
            OS_MemSET(&st, 0, sizeof(struct CliCmdEng_st));

            CmdEng_GetStatus((void*)&st);
            LOG_PRINTF("Mode:%s , BlockCmd:%s, %d Cmds In pendingQ\r\n", CmdEngModeString[st.mode], (st.BlkCmdIn == true)?"YES":"NO", st.BlkCmdNum);
        }
    }
    else if( (argc == 2) &&(ssv6xxx_strcmp(argv[1], "msg") == 0))
    {
        msg_evt_show();
    }
    else if( (argc == 2) &&(ssv6xxx_strcmp(argv[1], "reset") == 0))
    {
        ssv6xxx_sw_reset(SW_RESET_SYS_ALL);
    }
    else if ( (argc == 2) &&(ssv6xxx_strcmp(argv[1], "trx") == 0))
        TXRXTask_ShowSt();
    else if ( (argc == 2) &&(ssv6xxx_strcmp(argv[1], "irq") == 0))
    {
        ssv6xxx_drv_irq_disable(false);
        OS_TickDelay(1);
        ssv6xxx_drv_irq_enable(false);
    }
    else if ( (argc == 3) && (ssv6xxx_strcmp(argv[1], "soc") == 0) )
    {
        if(ssv6xxx_strcmp(argv[2], "st") == 0)
        {
            GetSocStatus();
        }
    }
#ifdef __SSV_UNIX_SIM__
    else if ((argc == 2) && (ssv6xxx_strcmp(argv[1],"profile") == 0) )
    {
    	ssv_type_s32 ret = OS_SysProfiling(&SysProfBuf);
    	if(ret == OS_SUCCESS)
    	{
            SysProfSt = 1;
            LOG_PRINTF("Enable system profiling\n");
    	}
        else
            LOG_PRINTF("Fail to enable system profiling\n");
    }
    else if((argc == 2)&&(ssv6xxx_strcmp(argv[1],"stack") == 0)){
        vTaskList( g_SysTaskStack );
        LOG_PRINTF("%s \r\n", g_SysTaskStack);
    }
#endif
	else if ((argc >= 2) && (ssv6xxx_strcmp(argv[1],"log") == 0)) {
        ssv_type_bool add=true;
        ssv_type_u32 module_bit = 0;


        if ((argc == 3) && (ssv6xxx_strcmp(argv[2],"show") == 0 )){
            ShowLog();
            return;
        }
        
#if SSV_LOG_DEBUG
        else if ((argc == 3) && (ssv6xxx_strcmp(argv[2],"reset") == 0 )){
            g_log_module = CONFIG_LOG_MODULE;
		    g_log_min_level = CONFIG_LOG_LEVEL;
            ShowLog();

            return;
        }else if ((argc == 4)&&(ssv6xxx_strcmp(argv[2],"lev") == 0 )){
             g_log_min_level = ssv6xxx_atoi(argv[3]);
             ShowLog();

            return;
        }else if ((argc == 5)&&(ssv6xxx_strcmp(argv[2],"mod") == 0)){

            //operation
            if ((ssv6xxx_strcmp(argv[3],"add") == 0 ))
                add =true;

            if ((ssv6xxx_strcmp(argv[3],"rem") == 0 ))
                add =false;


            //module
            if ((ssv6xxx_strcmp(argv[4],"all") == 0 ))
                module_bit = LOG_ALL_MODULES;
            else if((ssv6xxx_strcmp(argv[4],"mem") == 0 ))
                module_bit = LOG_MEM;
            else if((ssv6xxx_strcmp(argv[4],"socket") == 0 ))
                module_bit = LOG_L3_SOCKET;
            else if((ssv6xxx_strcmp(argv[4],"api") == 0 ))
                module_bit = LOG_L3_API;
            else if((ssv6xxx_strcmp(argv[4],"tcp") == 0 ))
                module_bit = LOG_L3_TCP;
            else if((ssv6xxx_strcmp(argv[4],"udp") == 0 ))
                module_bit = LOG_L3_UDP;
            else if((ssv6xxx_strcmp(argv[4],"ip") == 0 ))
                module_bit = LOG_L3_IP;
            else if((ssv6xxx_strcmp(argv[4],"other") == 0 ))
                module_bit = LOG_L3_OTHER_PROTO;
            else if((ssv6xxx_strcmp(argv[4],"data") == 0 ))
                module_bit = LOG_L2_DATA;
            else if((ssv6xxx_strcmp(argv[4],"ap") == 0 ))
                module_bit = LOG_L2_AP;
            else if((ssv6xxx_strcmp(argv[4],"sta") == 0 ))
                module_bit = LOG_L2_STA;
            else if((ssv6xxx_strcmp(argv[4],"httpd") == 0 ))
                module_bit = LOG_L4_HTTPD;
            else if((ssv6xxx_strcmp(argv[4],"dhcpd") == 0 ))
                module_bit = LOG_L4_DHCPD;
            else if((ssv6xxx_strcmp(argv[4],"netmgr") == 0 ))
                module_bit = LOG_L4_NETMGR;
            else if((ssv6xxx_strcmp(argv[4],"cmdeng") == 0 ))
                module_bit = LOG_CMDENG;
            else if((ssv6xxx_strcmp(argv[4],"txrx") == 0 ))
                module_bit = LOG_TXRX;
            else if((ssv6xxx_strcmp(argv[4],"sconfig") == 0 ))
                module_bit = LOG_SCONFIG;
#endif            
            else
            {;}


            if( true == add){
                //ADD module
                g_log_module |= module_bit;
            }else{

                //Remove module
                g_log_module &= (~module_bit);
            }
            ShowLog();
            return;
        }else
        {;}


        log_usage();



	}
    else
    {
        LOG_PRINTF("Invalid Command\r\n");
        usage();
    }
}
#endif
