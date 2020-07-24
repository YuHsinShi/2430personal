#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "i2s/i2s.h" 
#include "test.h"

#define ADC_MEMORY_SIZE (64*1024) 

unsigned char *recdata;
uint32_t sizecount = 0; 
/**********************************************************/


void initAD(){
    /* init ADC */ 
    adc_buf = (uint8_t*)malloc(ADC_BUFFER_SIZE);
	memset((uint8_t*) adc_buf, 0, ADC_BUFFER_SIZE);
	memset((void*)&spec_adI, 0, sizeof(STRC_I2S_SPEC));
    spec_adI.channels					= 1;
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
    spec_daI.channels                 = 1;
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
    i2s_pause_DAC(1);
}

void playring(){

	uint8_t *psrc=NULL;
	uint16_t bsize=audio_get_bsize_from_bps(&spec_daI);
    psrc = (u8*)malloc(bsize);
    sizecount = 0;
    printf("play\n");
    audio_pause_DA(0); 
    while(1){
        if(sizecount + bsize < ADC_MEMORY_SIZE){
            memcpy(psrc,recdata+sizecount, bsize);
            sizecount += bsize;
        }else{
            if(sizecount < ADC_MEMORY_SIZE){
                uint32_t szsec0 = ADC_MEMORY_SIZE - sizecount;
                uint32_t szsec1 = bsize - szsec0;
                memcpy(psrc,recdata+sizecount, szsec0);
                sizecount += szsec0;
                memset(psrc+szsec0,0,szsec1);
                sizecount += szsec1;
            }
            sizecount = 0;
        }
        audio_TX_Data_Send(&spec_daI,psrc,bsize);
        audio_DA_wait_time(&spec_daI,bsize);

    }
}

void recvoice(){
    uint16_t bsize=audio_get_bsize_from_bps(&spec_adI);
    uint8_t *psrc=NULL;           
    psrc = (u8*)malloc(bsize);
    recdata = (uint8_t*)malloc(ADC_MEMORY_SIZE);    
    audio_pause_AD(0);
    printf("record");
    while(sizecount < ADC_MEMORY_SIZE){
        
        if((sizecount+bsize) > ADC_MEMORY_SIZE) break;
            
        if(audio_RX_Data_Recv(&spec_adI,psrc,bsize)){
            memcpy(recdata+sizecount,psrc,bsize);
            sizecount += bsize;
        }
        
        audio_AD_wait_time(&spec_adI,bsize);
        
        printf(".");
    }
    printf("record end\n");
    audio_pause_AD(1);
}  

void* TestFunc(void* arg)
{
    int i=50;
    itpInit();
    printf("*******init DA********\n");
    printf("remove usb-to-spi tool for init DAC \n");//9070
    initDA();
    printf("*******init AD********\n");
    initAD();
    printf("*******load ring********\n");
/******************rec sound and play shond*******************************/ 
   printf("rec voice test\n");
   i2s_ADC_set_rec_volperc(80);
   printf("recording....");
   recvoice();
   printf("play rec sound\n");
   
   i2s_set_direct_volperc(80);//set volume 0~100
   playring();
    
/*******************end***************************************************/    
    return NULL;
}