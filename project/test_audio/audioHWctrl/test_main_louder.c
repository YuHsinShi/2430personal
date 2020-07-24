#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "i2s/i2s.h" 
#include "test.h"
#include "audio_ctrl.h"

/**********************************************************/


void initAD(){
    /* init ADC */ 
    adc_buf = (uint8_t*)malloc(ADC_BUFFER_SIZE);
	memset((uint8_t*) adc_buf, 0, ADC_BUFFER_SIZE);
	memset((void*)&spec_adI, 0, sizeof(STRC_I2S_SPEC));
    spec_adI.channels					= 2;
    spec_adI.sample_rate				= 8000;
    spec_adI.buffer_size				= ADC_BUFFER_SIZE;
    spec_adI.is_big_endian				= 0;
	spec_adI.base_i2s				    = (uint8_t*) adc_buf;	
    spec_adI.sample_size				= 16;
    spec_adI.record_mode				= 1;
    spec_adI.from_LineIN				= 0;
    spec_adI.from_MIC_IN				= 1;
    //i2s_init_ADC(&spec_adI);
    audio_init_AD(&spec_adI);
    audio_pause_AD(1);
}

void initDA(){
	/* init DAC */
    dac_buf = (uint8_t*)malloc(DAC_BUFFER_SIZE);
	memset((uint8_t*) dac_buf, 0, DAC_BUFFER_SIZE);
	memset((void*)&spec_daI, 0, sizeof(STRC_I2S_SPEC));
    spec_daI.channels                 = 2;
    spec_daI.sample_rate              = 8000;
    spec_daI.buffer_size              = DAC_BUFFER_SIZE;
    spec_daI.is_big_endian            = 0;
    spec_daI.base_i2s                 = (uint8_t*) dac_buf;
    spec_daI.sample_size              = 16;
    spec_daI.num_hdmi_audio_buffer    = 0;
    spec_daI.is_dac_spdif_same_buffer = 1;
    spec_daI.enable_Speaker           = 1;
    spec_daI.enable_HeadPhone         = 1;
    spec_daI.postpone_audio_output    = 1;
    spec_daI.base_spdif                = (uint8_t*) dac_buf;
    //i2s_init_DAC(&spec_daI);
    audio_init_DA(&spec_daI);
    audio_pause_DA(1);
}


void louder(){
    //uint16_t DAbsize=640;
    uint16_t bsize=audio_get_bsize_from_bps(&spec_daI);
    uint8_t *psrc=NULL;           
    psrc = (u8*)malloc(bsize);        
    audio_pause_DA(0); 
    audio_pause_AD(0);      
    while(1){      
        
        if(audio_RX_Data_Recv(&spec_adI,psrc,bsize))
            audio_TX_Data_Send(&spec_daI,psrc,bsize);
        
        //audio_AD_wait_time(&spec_adI,bsize);
    }
    if(psrc) free(psrc);
}  

void* TestFunc(void* arg)
{
    itpInit();
    initDA();
    initAD();
    //GPIO_switch_set(TELEPHONETUBE);
    //GPIO_switch_set(HEADSET);    
    
/******************rec sound and play shond*******************************/ 

    audio_AD_volume_set(60);//set rec 0~100 (faraday : level 60= 0db )
    audio_DA_volume_set(100);//set volume 0~100 (faraday : level 100= 0db )
    louder();
    
/*******************end***************************************************/    
    return NULL;
}