/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <host_apis.h>
#include <log.h>
#include <drv/ssv_drv.h>
#include <ssv_dev.h>

#include "cli_cmd.h"
#include "cli_cmd_at.h"
#if (CLI_ENABLE==1)

const at_cmd_info atcmdicomm_info_tbl[] =
{
    {ATCMD_RADIO_RF_START,              At_RadioRFStart,                2},
    {ATCMD_RADIO_CHANNEL,               At_RadioChannel,                0},
    {ATCMD_RADIO_RF_RATE,               At_RadioRFRate,                 1},
    {ATCMD_RADIO_RF_STOP,               At_RadioRFStop,                 0},
    {ATCMD_RADIO_RF_RESET,              At_RadioRFReset,                0},
    {ATCMD_RADIO_RF_COUNT,              At_RadioRFCount,                1},
    {ATCMD_RADIO_RF_DUMP,               At_RadioRFDump,                 0},
    {ATCMD_RF_TABLE_RT,                 At_RfTableRT,                   18},
    {ATCMD_RF_TABLE_HT,                 At_RfTableHT,                   18},
    {ATCMD_RF_TABLE_LT,                 At_RfTableLT,                   18},
    {ATCMD_RF_5GTABLE_RT,               At_Rf5GTableRT,                 18},   
    {ATCMD_RF_5GTABLE_HT,               At_Rf5GTableHT,                 18},   
    {ATCMD_RF_5GTABLE_LT,               At_Rf5GTableLT,                 18},  
    {ATCMD_RADIO_RF_READ_TEMPCS,        At_RadioRFReadTempcs,           0},
    {ATCMD_RADIO_RF,                    At_ShowRfCommand,               0},
};

////RF AT Command
int At_ShowRfCommand (stParam *param)
{
    int i = 0;
    int num = sizeof(atcmdicomm_info_tbl)/sizeof(at_cmd_info);
    LOG_PRINTF("\r\n");
    for(i = 0;i<num-1;i++)
    {
        if( ssv6xxx_strncmp(atcmdicomm_info_tbl[i].atCmd, "AT+RF", ssv6xxx_strlen("AT+RF")) == 0 )
            LOG_PRINTF("%s\r\n", atcmdicomm_info_tbl[i].atCmd);
    }

    return SSV6XXX_SUCCESS;
}

extern void customer_extra_setting(void);
int At_RadioRFStart(stParam *param)
{	

    struct cfg_rf_tool rf_tool;
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_START;
    rf_tool.u.RF_START.count=ssv6xxx_atoi(param->argv[0]);
    rf_tool.u.RF_START.interval=ssv6xxx_atoi(param->argv[1]);    
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));
    return SSV6XXX_SUCCESS;
}
int At_RadioChannel(stParam *param)
{
    struct cfg_rf_tool rf_tool;
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_CHANNEL;
    rf_tool.u.RF_CHANNEL.channel=ssv6xxx_atoi(param->argv[0]);
    rf_tool.u.RF_CHANNEL.ch_bw=ssv6xxx_atoi(param->argv[1]);    
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));
    return SSV6XXX_SUCCESS;
}
int At_RadioRFRate(stParam *param)
{
    struct cfg_rf_tool rf_tool;
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_RATE;
    rf_tool.u.RF_RATE.rate=ssv6xxx_atoi(param->argv[0]);
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));
    return SSV6XXX_SUCCESS;
}


int At_RadioRFStop(stParam *param)
{

    struct cfg_rf_tool rf_tool;
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_STOP;
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));
    return SSV6XXX_SUCCESS;
}
int At_RadioRFReset(stParam *param)
{
    struct cfg_rf_tool rf_tool;
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_RESET;
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));
    return SSV6XXX_SUCCESS;
}
int At_RadioRFCount(stParam *param)
{

    int ret = SSV6XXX_SUCCESS;

    ssv_type_u32 err_rxpkt = 0;
    ssv_type_u32 total_rxpkt = 0;

    if(param->argc <1)
        return SSV6XXX_FAILED;

    LOG_PRINTF("\r\n");
    if( ssv6xxx_atoi(param->argv[0]) == 1 )
    {
        LOG_PRINTF("B Mode Count:\r\n");
        ssv_hal_get_b_mode_count(&total_rxpkt, &err_rxpkt);
    }
    else
    {
        LOG_PRINTF("G/N Mode Count:\r\n");
        ssv_hal_get_gn_mode_count(&total_rxpkt, &err_rxpkt);
    }
        
    LOG_PRINTF("crc count = '%ld'\r\n", err_rxpkt);
    LOG_PRINTF("total count = '%ld'\r\n", total_rxpkt);	

    return ret;
}

extern int dump_rf_table(void);
int At_RadioRFDump(stParam *param)
{

    int ret = SSV6XXX_SUCCESS;	
    LOG_PRINTF("\r\n");
    dump_rf_table();     
    return ret;
}

extern struct st_rf_table def_rf_table;
int At_RfTableRT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
    LOG_PRINTF("\r\n");

    if(param->argc <1)
        return SSV6XXX_FAILED;
    
    //printf("arg1 = %s, arg2 = %s, ... , arg18 = %s\n", param->argv[0], param->argv[1], param->argv[17]);
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ATCMD_RF_TABLE_RT
                                                                                 , def_rf_table.rt_config.band_gain[0]
                                                                                 , def_rf_table.rt_config.band_gain[1]
                                                                                 , def_rf_table.rt_config.band_gain[2]
                                                                                 , def_rf_table.rt_config.band_gain[3]
                                                                                 , def_rf_table.rt_config.band_gain[4]
                                                                                 , def_rf_table.rt_config.band_gain[5]
                                                                                 , def_rf_table.rt_config.band_gain[6]
                                                                                 , def_rf_table.rt_config.freq_xi
                                                                                 , def_rf_table.rt_config.freq_xo
                                                                                 , def_rf_table.rt_config.ldo_rxafe
                                                                                 , def_rf_table.rt_config.ldo_dcdcv
                                                                                 , def_rf_table.rt_config.ldo_dldov
                                                                                 , def_rf_table.rt_config.pa_vcas1
                                                                                 , def_rf_table.rt_config.pa_vcas2
                                                                                 , def_rf_table.rt_config.pa_vcas3
                                                                                 , def_rf_table.rt_config.pa_bias
                                                                                 , def_rf_table.rt_config.pa_cap
                                                                                 , def_rf_table.rt_config.padpd_cali
                                                                                 );
    }
    else
    {
        struct st_tempe_table tempe_config;
        tempe_config.band_gain[0] = ssv6xxx_atoi(param->argv[0]);
        tempe_config.band_gain[1] = ssv6xxx_atoi(param->argv[1]);
        tempe_config.band_gain[2] = ssv6xxx_atoi(param->argv[2]);
        tempe_config.band_gain[3] = ssv6xxx_atoi(param->argv[3]);
        tempe_config.band_gain[4] = ssv6xxx_atoi(param->argv[4]);
        tempe_config.band_gain[5] = ssv6xxx_atoi(param->argv[5]);
        tempe_config.band_gain[6] = ssv6xxx_atoi(param->argv[6]);
        
        tempe_config.freq_xi = ssv6xxx_atoi(param->argv[7]);
        tempe_config.freq_xo = ssv6xxx_atoi(param->argv[8]);
        tempe_config.ldo_rxafe = ssv6xxx_atoi(param->argv[9]);
        tempe_config.ldo_dcdcv = ssv6xxx_atoi(param->argv[10]);
        tempe_config.ldo_dldov = ssv6xxx_atoi(param->argv[11]);

        tempe_config.pa_vcas1 = ssv6xxx_atoi(param->argv[12]);
        tempe_config.pa_vcas2 = ssv6xxx_atoi(param->argv[13]);
        tempe_config.pa_vcas3 = ssv6xxx_atoi(param->argv[14]);
        tempe_config.pa_bias = ssv6xxx_atoi(param->argv[15]);
        tempe_config.pa_cap = ssv6xxx_atoi(param->argv[16]);
        tempe_config.padpd_cali = ssv6xxx_atoi(param->argv[17]);

        OS_MemCPY(&def_rf_table.rt_config, &tempe_config, sizeof(tempe_config) );

    }
        
    return ret;
}      
int At_RfTableHT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
	LOG_PRINTF("\r\n");
    
    if(param->argc <1)
        return SSV6XXX_FAILED;
    
    //printf("arg1 = %s, arg2 = %s, ... , arg18 = %s\n", param->argv[0], param->argv[1], param->argv[17]);
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ATCMD_RF_TABLE_HT
                                                                                 , def_rf_table.ht_config.band_gain[0]
                                                                                 , def_rf_table.ht_config.band_gain[1]
                                                                                 , def_rf_table.ht_config.band_gain[2]
                                                                                 , def_rf_table.ht_config.band_gain[3]
                                                                                 , def_rf_table.ht_config.band_gain[4]
                                                                                 , def_rf_table.ht_config.band_gain[5]
                                                                                 , def_rf_table.ht_config.band_gain[6]
                                                                                 , def_rf_table.ht_config.freq_xi
                                                                                 , def_rf_table.ht_config.freq_xo
                                                                                 , def_rf_table.ht_config.ldo_rxafe
                                                                                 , def_rf_table.ht_config.ldo_dcdcv
                                                                                 , def_rf_table.ht_config.ldo_dldov
                                                                                 , def_rf_table.ht_config.pa_vcas1
                                                                                 , def_rf_table.ht_config.pa_vcas2
                                                                                 , def_rf_table.ht_config.pa_vcas3
                                                                                 , def_rf_table.ht_config.pa_bias
                                                                                 , def_rf_table.ht_config.pa_cap
                                                                                 , def_rf_table.ht_config.padpd_cali
                                                                                 );
    }
    else
    {
        struct st_tempe_table tempe_config;
        tempe_config.band_gain[0] = ssv6xxx_atoi(param->argv[0]);
        tempe_config.band_gain[1] = ssv6xxx_atoi(param->argv[1]);
        tempe_config.band_gain[2] = ssv6xxx_atoi(param->argv[2]);
        tempe_config.band_gain[3] = ssv6xxx_atoi(param->argv[3]);
        tempe_config.band_gain[4] = ssv6xxx_atoi(param->argv[4]);
        tempe_config.band_gain[5] = ssv6xxx_atoi(param->argv[5]);
        tempe_config.band_gain[6] = ssv6xxx_atoi(param->argv[6]);
        
        tempe_config.freq_xi = ssv6xxx_atoi(param->argv[7]);
        tempe_config.freq_xo = ssv6xxx_atoi(param->argv[8]);
        tempe_config.ldo_rxafe = ssv6xxx_atoi(param->argv[9]);
        tempe_config.ldo_dcdcv = ssv6xxx_atoi(param->argv[10]);
        tempe_config.ldo_dldov = ssv6xxx_atoi(param->argv[11]);

        tempe_config.pa_vcas1 = ssv6xxx_atoi(param->argv[12]);
        tempe_config.pa_vcas2 = ssv6xxx_atoi(param->argv[13]);
        tempe_config.pa_vcas3 = ssv6xxx_atoi(param->argv[14]);
        tempe_config.pa_bias = ssv6xxx_atoi(param->argv[15]);
        tempe_config.pa_cap = ssv6xxx_atoi(param->argv[16]);
        tempe_config.padpd_cali = ssv6xxx_atoi(param->argv[17]);

        OS_MemCPY(&def_rf_table.ht_config, &tempe_config, sizeof(tempe_config) );

    }
    return ret;
}      
int At_RfTableLT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
    LOG_PRINTF("\r\n");
    
    if(param->argc <1)
        return SSV6XXX_FAILED;
    
    //printf("arg1 = %s, arg2 = %s, ... , arg18 = %s\n", param->argv[0], param->argv[1], param->argv[17]);
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ATCMD_RF_TABLE_LT
                                                                                 , def_rf_table.lt_config.band_gain[0]
                                                                                 , def_rf_table.lt_config.band_gain[1]
                                                                                 , def_rf_table.lt_config.band_gain[2]
                                                                                 , def_rf_table.lt_config.band_gain[3]
                                                                                 , def_rf_table.lt_config.band_gain[4]
                                                                                 , def_rf_table.lt_config.band_gain[5]
                                                                                 , def_rf_table.lt_config.band_gain[6]
                                                                                 , def_rf_table.lt_config.freq_xi
                                                                                 , def_rf_table.lt_config.freq_xo
                                                                                 , def_rf_table.lt_config.ldo_rxafe
                                                                                 , def_rf_table.lt_config.ldo_dcdcv
                                                                                 , def_rf_table.lt_config.ldo_dldov
                                                                                 , def_rf_table.lt_config.pa_vcas1
                                                                                 , def_rf_table.lt_config.pa_vcas2
                                                                                 , def_rf_table.lt_config.pa_vcas3
                                                                                 , def_rf_table.lt_config.pa_bias
                                                                                 , def_rf_table.lt_config.pa_cap
                                                                                 , def_rf_table.lt_config.padpd_cali
                                                                                 );
    }
    else
    {
        struct st_tempe_table tempe_config;
        tempe_config.band_gain[0] = ssv6xxx_atoi(param->argv[0]);
        tempe_config.band_gain[1] = ssv6xxx_atoi(param->argv[1]);
        tempe_config.band_gain[2] = ssv6xxx_atoi(param->argv[2]);
        tempe_config.band_gain[3] = ssv6xxx_atoi(param->argv[3]);
        tempe_config.band_gain[4] = ssv6xxx_atoi(param->argv[4]);
        tempe_config.band_gain[5] = ssv6xxx_atoi(param->argv[5]);
        tempe_config.band_gain[6] = ssv6xxx_atoi(param->argv[6]);
        
        tempe_config.freq_xi = ssv6xxx_atoi(param->argv[7]);
        tempe_config.freq_xo = ssv6xxx_atoi(param->argv[8]);
        tempe_config.ldo_rxafe = ssv6xxx_atoi(param->argv[9]);
        tempe_config.ldo_dcdcv = ssv6xxx_atoi(param->argv[10]);
        tempe_config.ldo_dldov = ssv6xxx_atoi(param->argv[11]);

        tempe_config.pa_vcas1 = ssv6xxx_atoi(param->argv[12]);
        tempe_config.pa_vcas2 = ssv6xxx_atoi(param->argv[13]);
        tempe_config.pa_vcas3 = ssv6xxx_atoi(param->argv[14]);
        tempe_config.pa_bias = ssv6xxx_atoi(param->argv[15]);
        tempe_config.pa_cap = ssv6xxx_atoi(param->argv[16]);
        tempe_config.padpd_cali = ssv6xxx_atoi(param->argv[17]);

        OS_MemCPY(&def_rf_table.lt_config, &tempe_config, sizeof(tempe_config) );
    }
    return ret;
}   

int At_Rf5GTableRT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
    struct st_tempe_5g_table tempe_config = {0};
        
    LOG_PRINTF("\n");
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d\r\n", ATCMD_RF_5GTABLE_RT, def_rf_table.rt_5g_config.bbscale_band0
                                                                                           , def_rf_table.rt_5g_config.bbscale_band1
                                                                                           , def_rf_table.rt_5g_config.bbscale_band2
                                                                                           , def_rf_table.rt_5g_config.bbscale_band3);
        return ret;
    }
    
    if(param->argc < 4)
        return SSV6XXX_FAILED;
     
    tempe_config.bbscale_band0 = ssv6xxx_atoi(param->argv[0]);
    tempe_config.bbscale_band1 = ssv6xxx_atoi(param->argv[1]);
    tempe_config.bbscale_band2 = ssv6xxx_atoi(param->argv[2]);
    tempe_config.bbscale_band3 = ssv6xxx_atoi(param->argv[3]);
    tempe_config.bias1 = def_rf_table.rt_5g_config.bias1;
    tempe_config.bias2 = def_rf_table.rt_5g_config.bias2;
        
    OS_MemCPY(&def_rf_table.rt_5g_config, &tempe_config, sizeof(tempe_config) );    
    return ret;    
}
int At_Rf5GTableHT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
    struct st_tempe_5g_table tempe_config = {0};
    
    LOG_PRINTF("\n");
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d\r\n", ATCMD_RF_5GTABLE_HT, def_rf_table.ht_5g_config.bbscale_band0
                                                                                           , def_rf_table.ht_5g_config.bbscale_band1
                                                                                           , def_rf_table.ht_5g_config.bbscale_band2
                                                                                           , def_rf_table.ht_5g_config.bbscale_band3);
        return ret;
    }
    
    if(param->argc < 4)
        return SSV6XXX_FAILED;
    
    tempe_config.bbscale_band0 = ssv6xxx_atoi(param->argv[0]);
    tempe_config.bbscale_band1 = ssv6xxx_atoi(param->argv[1]);
    tempe_config.bbscale_band2 = ssv6xxx_atoi(param->argv[2]);
    tempe_config.bbscale_band3 = ssv6xxx_atoi(param->argv[3]);
    tempe_config.bias1 = def_rf_table.ht_5g_config.bias1;
    tempe_config.bias2 = def_rf_table.ht_5g_config.bias2;


    OS_MemCPY(&def_rf_table.ht_5g_config, &tempe_config, sizeof(tempe_config) );    

    return ret;    
}
int At_Rf5GTableLT(stParam *param)
{
    int ret = SSV6XXX_SUCCESS;	
    struct st_tempe_5g_table tempe_config = {0};    
    LOG_PRINTF("\r\n");
    
    if(ssv6xxx_strcmp("?" , param->argv[0])==0)
    {
        LOG_PRINTF("%s=%d,%d,%d,%d\r\n", ATCMD_RF_5GTABLE_LT, def_rf_table.lt_5g_config.bbscale_band0
                                                                                          , def_rf_table.lt_5g_config.bbscale_band1
                                                                                          , def_rf_table.lt_5g_config.bbscale_band2
                                                                                          , def_rf_table.lt_5g_config.bbscale_band3);
        return ret;
    }
    
    if(param->argc < 4)
        return SSV6XXX_FAILED;
     
    tempe_config.bbscale_band0 = ssv6xxx_atoi(param->argv[0]);
    tempe_config.bbscale_band1 = ssv6xxx_atoi(param->argv[1]);
    tempe_config.bbscale_band2 = ssv6xxx_atoi(param->argv[2]);
    tempe_config.bbscale_band3 = ssv6xxx_atoi(param->argv[3]);
    tempe_config.bias1 = def_rf_table.lt_5g_config.bias1;
    tempe_config.bias2 = def_rf_table.lt_5g_config.bias2;

       
    OS_MemCPY(&def_rf_table.lt_5g_config, &tempe_config, sizeof(tempe_config) );    

    return ret; 
}


int At_RadioRFReadTempcs(stParam *param)
{
    //ssv_type_u32 value=0;
    struct cfg_rf_tool rf_tool;
    
    OS_MemSET((void *)&rf_tool,0,sizeof(struct cfg_rf_tool));
    rf_tool.cmd=EN_RF_TOOL_RF_READ_TEMPERATURE;
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_RF_TOOL, &rf_tool, sizeof(struct cfg_rf_tool));

    return SSV6XXX_SUCCESS;
    //LOG_PRINTF("temperature = %d\r\n", value)
    //return SSV6XXX_SUCCESS;
}


int rf_tool_read_evt_handler (void *data)
{
    struct cfg_rf_tool_rsp *rf_tool_rsp = (struct cfg_rf_tool_rsp *)data;

    switch(rf_tool_rsp->evt)
    {
        case EN_RF_TOOL_EVT_RF_READ_TEMPERATURE:
            LOG_PRINTF("temperature = %d\r\n", rf_tool_rsp->u.RF_READ_TEMPERATURE.val);
            break;
        default:
            break;
    }
    return SSV6XXX_SUCCESS;
}

void At_RespOK (char* pStr)
{
	char szResp[64] = {0};
	int len = ssv6xxx_strlen(pStr);
	if( *(pStr+len-1) == '?' && *(pStr+len-2) == '=' )
		len-=2;
	else if( *(pStr+len-1) == '=' )
		len-=1;
	OS_MemCPY(szResp, pStr, len);
	//printf("%s=OK\n", szResp);
    LOG_PRINTF("%s=", szResp);
    LOG_PRINTF("OK\r\n");	
  
}
int parseBuff2Param(char* bufCmd, stParam* pParam, ssv_type_u8 maxargu)
{
	int buflen, i;
	const char delimiters = ',';

    buflen = ssv6xxx_strlen (bufCmd);
	if (ssv6xxx_strlen (bufCmd) == 0) 
		return SSV6XXX_FAILED;

    if(maxargu == 0)
        maxargu = MAX_ARGUMENT;

    //LOG_PRINTF("bufcmd=%s\r\n",bufCmd);
    pParam->argc = 1;
    pParam->argv[pParam->argc - 1] = &bufCmd[0];
    i = 0;
    while(pParam->argc < maxargu)
    {
        for(; i < buflen ; i++)
        {
            if(bufCmd[i] == delimiters)
            {
                bufCmd[i] = 0;
                break;
            }
        }
        if(i == buflen)
			break;
        if(bufCmd[i + 1] != 0)
        {
            pParam->argc += 1;
            pParam->argv[pParam->argc - 1] = &bufCmd[i + 1];
        }
        else
            break;
    }
    
	return 0;
}

int At_Parser (char *buff, int len)
{
	int i = 0;
	int	nRet = SSV6XXX_FAILED;
    char cmd[MAX_CMD_LEN], operat = 0; //cmd len 32 should be enough
    stParam param;

#if 0
	OS_MemSET(atcmd_buffer,0 , CLI_BUFFER_SIZE+1);
	OS_MemCPY(atcmd_buffer, buff, len);
	buff = atcmd_buffer;
#endif

#if 1
    buff[len] = 0x0;  //chomp!! replace \r\n with null string
    len++;
#endif

//    printf("buff=%s,len=%d\n",buff,len);

    OS_MemSET(&param, 0, sizeof(stParam));
    if( (1==len) && (buff[0]=='\r' || buff[0]=='\n')){
        nRet = SSV6XXX_SUCCESS;
        goto exit;
    }
    
	if (0 == len) {
		//printf (ATRSP_ERROR, ERROR_INVALID_PARAMETER);
		return SSV6XXX_FAILED;
	}


    for(i = 0; i < MAX_CMD_LEN; i++)
    {
        if(buff[i] == 0 || buff[i] == '=' || buff[i] == ' ')
        {
            OS_MemCPY(cmd, buff, i);
            operat = buff[i];
            cmd[i] = 0;
            break;
        }
    }

    for(i = 0; i < sizeof(atcmdicomm_info_tbl)/sizeof(at_cmd_info); i++)
    {
#if 0    
        printf("atCmd = %s\n", atcmdicomm_info_tbl[i].atCmd);
        printf("buff=%s\n",buff);
        printf("cmd=%s\n",cmd);
        printf("strlen=%d\n",strlen(atcmdicomm_info_tbl[i].atCmd));
#endif
    
        //if(strncmp(atcmdicomm_info_tbl[i].atCmd, cmd, strlen(atcmdicomm_info_tbl[i].atCmd)) == 0)    
        if(ssv6xxx_strcmp(atcmdicomm_info_tbl[i].atCmd, cmd) == 0)
        {
            if(operat != 0)
                parseBuff2Param(buff + ssv6xxx_strlen(atcmdicomm_info_tbl[i].atCmd) + 1, &param, atcmdicomm_info_tbl[i].maxargu);

            nRet = atcmdicomm_info_tbl[i].pfHandle(&param);
            goto exit_rsp;
        }
    }

exit_rsp:
	if (SSV6XXX_SUCCESS > nRet){
		//atcmdprintf (ATRSP_ERROR, nRet);
		LOG_PRINTF(ATRSP_ERROR, nRet);
    }else if(SSV6XXX_SUCCESS == nRet)
        At_RespOK(cmd);

exit:
	return nRet;
}

#endif

