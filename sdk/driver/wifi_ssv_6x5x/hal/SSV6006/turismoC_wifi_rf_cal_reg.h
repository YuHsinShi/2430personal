/*
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

static ssv_cabrio_reg rf_calibration_result[]={
        /* Sx , 1*/       
        {0xCCB0A464,0},

        /* RX DC , 21*/
        //ADR_WF_DCOC_IDAC_Register1~ADR_WF_DCOC_IDAC_Register21
        {0xCCB0A494,0},            
        {0xCCB0A498,0},        
        {0xCCB0A49C,0},        
        {0xCCB0A4A0,0},        
        {0xCCB0A4A4,0},                
        {0xCCB0A4A8,0},                
        {0xCCB0A4AC,0},
        {0xCCB0A4B0,0},                
        {0xCCB0A4B4,0}, 
        {0xCCB0A4B8,0},                
        {0xCCB0A4BC,0},                
        {0xCCB0A4C0,0},                        
        {0xCCB0A4C4,0},
        {0xCCB0A4C8,0},                                
        {0xCCB0A4CC,0},                                
        {0xCCB0A4D0,0},                                        
        {0xCCB0A4D4,0},                                                
        {0xCCB0A4D8,0},                                                
        {0xCCB0A4DC,0},                                                
        {0xCCB0A4E0,0},        
        {0xCCB0A4E4,0},        
        //To Do
        //ADR_5G_DCOC_IDAC_Register1~ADR_5G_DCOC_IDAC_Register21
        //0xCCB0A5A8~0xCCB0A5F8

        /* RX RC , 2*/
        //ADR_WF_H20_RX_FILTER_REGISTER
        {0xCCB0A410,0},        
        //ADR_WF_H40_RX_FILTER_REGISTER
        {0XCCB0A414,0},        

        /* TX DC , 2*/    
        //ADR_WF_TX_DAC_REGISTER
        {0xCCB0A450,0},
        //ADR_5G_TX_DAC_REGISTER
        {0xCCB0A578,0},

        /* TX RX IQ ,5*/
        //ADR_TRX_IQ_COMP_2G
        {0xCCB0A820,0}, 
        //ADR_TRX_IQ_COMP_5G_0
        {0xCCB0A824,0}, 
        //ADR_TRX_IQ_COMP_5G_1
        {0xCCB0A828,0}, 
        //ADR_TRX_IQ_COMP_5G_2
        {0xCCB0A82C,0}, 
        //ADR_TRX_IQ_COMP_5G_3
        {0xCCB0A830,0}, 

        /* PADPD ,22*/
        //ADR_MODE_REGISTER
        {0xCCB0A400,0},
        //ADR_5G_TX_GAIN_PAFB_CONTROL
        {0xCCB0A634,0},
        //ADR_WIFI_PHY_COMMON_BB_SCALE_REG_0~ADR_WIFI_PHY_COMMON_BB_SCALE_REG_3
        {0xCCB0A180,0},        
        {0xCCB0A184,0},
        {0xCCB0A188,0},        
        {0xCCB0A18C,0},
        //ADR_WIFI_PADPD_2G_CONTROL_REG
        {0xCCB0AD1C,0},        
        //ADR_WIFI_PADPD_5G_BB_GAIN_REG        
        {0xCCB0ADA8,0},                
        //ADR_WIFI_PADPD_2G_BB_GAIN_REG        
        {0xCCB0ADAC,0},
        //ADR_WIFI_PADPD_2G_GAIN_REG0~ADR_WIFI_PADPD_2G_GAIN_REGC
        {0xCCB0AD20,0},
        {0xCCB0AD24,0},        
        {0xCCB0AD28,0},        
        //{0xCCB0AD2C,0}, //this register doesn't exist         
        {0xCCB0AD30,0},        
        {0xCCB0AD34,0},        
        {0xCCB0AD38,0},        
        {0xCCB0AD3C,0},        
        {0xCCB0AD40,0},        
        {0xCCB0AD44,0},        
        {0xCCB0AD48,0},        
        {0xCCB0AD4C,0},        
        {0xCCB0AD50,0},
        {0xCCB0AD54,0}        
        //ToDo: ADR_WIFI_PADPD_5100_GAIN_REG0~ADR_WIFI_PADPD_5100_GAIN_REGC
        //0xCCB0A900 ~ 0xCCB0A930
        
        //ToDo: ADR_WIFI_PADPD_5200_GAIN_REG0~ADR_WIFI_PADPD_5200_GAIN_REGC        
        //0xCCB0A980 ~ 0xCCB0A9B0
        
        //ToDo: ADR_WIFI_PADPD_5300_GAIN_REG0~ADR_WIFI_PADPD_5300_GAIN_REGC                
        //0xCCB0AA00 ~ 0xCCB0AA30        
        
        //ToDo: ADR_WIFI_PADPD_5400_GAIN_REG0~ADR_WIFI_PADPD_5400_GAIN_REGC       
        //0xCCB0AA80 ~ 0xCCB0AAB0        
        
};

