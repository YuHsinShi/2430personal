/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <porting.h>
#include <ssv_common.h>
#include <host_config.h>
#include <log.h>

//default table
struct st_rf_table def_rf_table ={
        /* rt_config */  
        { {11, 11, 11, 11, 11, 11, 11} , 0x97, 0x97, 7, 9, 10, 7, 7, 7, 7, 7, 0}, 
        /* ht_config */
        { {11, 11, 11, 11, 11, 11, 11} , 0x97, 0x97, 7, 9, 10, 7, 7, 7, 7, 7, 0}, 
        /* lt_config */
        { {11, 11, 11, 11, 11, 11, 11} , 0x97, 0x97, 7, 9, 10, 7, 7, 7, 7, 7, 0}, 
        /* rf_gain */
        4,
        /* rate_gain_b */ 
        2, 
        /* rate_config_g */
        {13, 11, 9, 7}, 
        /* rate_config_20n */
        {13, 11, 9, 7}, 
        /* rate_config_40n */
        {13, 11, 9, 7},
        /* low_boundarty */
        35,
        /* high_boundary */ 
        90,
        /* boot flag*/
        EN_NOT_FIRST_BOOT,
        /* work mode */ 
        EN_WORK_NOMAL,
        /* rt_5g_config */
        { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
        /* ht_5g_config */
        { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
        /* lt_5g_config */
        { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
        /* band_f0_threshold */
        0x141E,
        /* band_f1_threshold */ 
        0x157C, 
        /* band_f2_threshold */ 
        0x1644
    };         

struct st_rf_table *p_rf_table=(struct st_rf_table *)&def_rf_table;
int dump_rf_table(void)
{
    LOG_PRINTF("===================\r\n");
    LOG_PRINTF("RT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->rt_config.band_gain[0], p_rf_table->rt_config.band_gain[1], p_rf_table->rt_config.band_gain[2], p_rf_table->rt_config.band_gain[3], p_rf_table->rt_config.band_gain[4], p_rf_table->rt_config.band_gain[5], p_rf_table->rt_config.band_gain[6],
                p_rf_table->rt_config.freq_xi, p_rf_table->rt_config.freq_xo,
                p_rf_table->rt_config.ldo_rxafe, p_rf_table->rt_config.ldo_dcdcv, p_rf_table->rt_config.ldo_dldov,
                p_rf_table->rt_config.pa_vcas1, p_rf_table->rt_config.pa_vcas2, p_rf_table->rt_config.pa_vcas3, p_rf_table->rt_config.pa_bias, p_rf_table->rt_config.pa_cap,
                p_rf_table->rt_config.padpd_cali
                );
    LOG_PRINTF("HT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->ht_config.band_gain[0], p_rf_table->ht_config.band_gain[1], p_rf_table->ht_config.band_gain[2], p_rf_table->ht_config.band_gain[3], p_rf_table->ht_config.band_gain[4], p_rf_table->ht_config.band_gain[5], p_rf_table->ht_config.band_gain[6],
                p_rf_table->ht_config.freq_xi, p_rf_table->ht_config.freq_xo,
                p_rf_table->ht_config.ldo_rxafe, p_rf_table->ht_config.ldo_dcdcv, p_rf_table->ht_config.ldo_dldov,
                p_rf_table->ht_config.pa_vcas1, p_rf_table->ht_config.pa_vcas2, p_rf_table->ht_config.pa_vcas3, p_rf_table->ht_config.pa_bias, p_rf_table->ht_config.pa_cap,
                p_rf_table->ht_config.padpd_cali
                );
    LOG_PRINTF("LT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->lt_config.band_gain[0], p_rf_table->lt_config.band_gain[1], p_rf_table->lt_config.band_gain[2], p_rf_table->lt_config.band_gain[3], p_rf_table->lt_config.band_gain[4], p_rf_table->lt_config.band_gain[5], p_rf_table->lt_config.band_gain[6],
                p_rf_table->lt_config.freq_xi, p_rf_table->lt_config.freq_xo,
                p_rf_table->lt_config.ldo_rxafe, p_rf_table->lt_config.ldo_dcdcv, p_rf_table->lt_config.ldo_dldov,
                p_rf_table->lt_config.pa_vcas1, p_rf_table->lt_config.pa_vcas2, p_rf_table->lt_config.pa_vcas3, p_rf_table->lt_config.pa_bias, p_rf_table->lt_config.pa_cap,
                p_rf_table->lt_config.padpd_cali
                );    
    LOG_PRINTF("===================\r\n");
    LOG_PRINTF("RF Gain:\t %d\r\n", p_rf_table->rf_gain);
    LOG_PRINTF("B Rate Gain:\t %2d\r\n", p_rf_table->rate_gain_b);
    LOG_PRINTF("G Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_g.rate1, p_rf_table->rate_config_g.rate2, p_rf_table->rate_config_g.rate3, p_rf_table->rate_config_g.rate4 );
    LOG_PRINTF("20N Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_20n.rate1, p_rf_table->rate_config_20n.rate2, p_rf_table->rate_config_20n.rate3, p_rf_table->rate_config_20n.rate4 );
    LOG_PRINTF("40N Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_40n.rate1, p_rf_table->rate_config_40n.rate2, p_rf_table->rate_config_40n.rate3, p_rf_table->rate_config_40n.rate4 );
    LOG_PRINTF("Temperature Boundary:\t %d, %d\r\n", p_rf_table->low_boundary, p_rf_table->high_boundary);
    LOG_PRINTF("===================\r\n");       
    LOG_PRINTF("5G RT Config:\t %2d %2d %2d %2d 0x%lx 0x%lx\r\n", 
                p_rf_table->rt_5g_config.bbscale_band0, p_rf_table->rt_5g_config.bbscale_band1, p_rf_table->rt_5g_config.bbscale_band2, p_rf_table->rt_5g_config.bbscale_band3,
                p_rf_table->rt_5g_config.bias1, p_rf_table->rt_5g_config.bias2
                );
    LOG_PRINTF("5G HT Config:\t %2d %2d %2d %2d 0x%lx 0x%lx\r\n", 
                p_rf_table->ht_5g_config.bbscale_band0, p_rf_table->ht_5g_config.bbscale_band1, p_rf_table->ht_5g_config.bbscale_band2, p_rf_table->ht_5g_config.bbscale_band3,
                p_rf_table->ht_5g_config.bias1, p_rf_table->ht_5g_config.bias2
                );
    LOG_PRINTF("5G LT Config:\t %2d %2d %2d %2d 0x%lx 0x%lx\r\n", 
                p_rf_table->lt_5g_config.bbscale_band0, p_rf_table->lt_5g_config.bbscale_band1, p_rf_table->lt_5g_config.bbscale_band2, p_rf_table->lt_5g_config.bbscale_band3,
                p_rf_table->lt_5g_config.bias1, p_rf_table->lt_5g_config.bias2
                );    
    LOG_PRINTF("5G Band Threshold:\t %4d %4d %4d\r\n", p_rf_table->band_f0_threshold, p_rf_table->band_f1_threshold, p_rf_table->band_f2_threshold);    
    LOG_PRINTF("===================\r\n");       
    return 0;
}



typedef struct ssv_reg_st {
    ssv_type_u32 address;
    ssv_type_u32 data;
} ssv_reg;

const ssv_reg customer_extra_table[]={
    //addrr      value
    {0xCCB0A424, 0x57444427},
    {0xCCB0A400, 0x39000300},
};

void customer_extra_setting(void)
{
#if (USE_EXT_PA==1)
    //OS_MsDelay(1000);
    //LOG_PRINTF("%s,size = %d\r\n",__func__,sizeof(customer_extra_table));
    //MAC_REG_SET_BITS((0xCCB0A424),((0x02)<<(4)),(0xffffff0f));
    //MAC_REG_WRITE(0xCCB0A400,0x39000300);
    ssv6xxx_wifi_customer_setting((void*)customer_extra_table, sizeof(customer_extra_table));
#endif
}

void customer_setting_rftable(int is_5g)
{
    if (is_5g)
    {
        def_rf_table.rt_config.padpd_cali = 1;
        def_rf_table.ht_config.padpd_cali = 1;
        def_rf_table.lt_config.padpd_cali = 1;
    }
    else
    {
        def_rf_table.rt_config.padpd_cali = 0;
        def_rf_table.ht_config.padpd_cali = 0;
        def_rf_table.lt_config.padpd_cali = 0;
    }
}


