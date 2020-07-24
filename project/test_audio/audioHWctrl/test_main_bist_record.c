#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "i2s/i2s.h" 
#include "test.h"
#include "wavheader.h"
#include "audio_ctrl.h"

#define ADC_MEMORY_SIZE (16*1024) 
//#define USB_DUMP 1

unsigned char *recdata;
uint32_t sizecount = 0; 

/**********************************************************/
#define I2S_REG_OUT_BIST            (0xD0100000 | 0x98)

void i2s_out_bist_pause(int bist)
{
    if(bist){
        ithWriteRegMaskA(I2S_REG_OUT_BIST, 0, 1<<0);
    }else{
        ithWriteRegMaskA(I2S_REG_OUT_BIST, 1, 1<<0);
    } 
}

void initAD(){
    /* init ADC */ 
    adc_buf = (uint8_t*)malloc(ADC_BUFFER_SIZE);
	memset((uint8_t*) adc_buf, 0, ADC_BUFFER_SIZE);
	memset((void*)&spec_adI, 0, sizeof(STRC_I2S_SPEC));
    spec_adI.channels					= 2;
    spec_adI.sample_rate				= 48000;
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
    spec_daI.sample_rate              = 48000;
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


void recvoice(){
    //uint16_t DAbsize=640;
    bool first = true;
    uint16_t bsize=audio_get_bsize_from_bps(&spec_daI);
    uint8_t *psrc=NULL;           
    psrc = (u8*)malloc(bsize);       
    recdata = (uint8_t*)malloc(ADC_MEMORY_SIZE);
    memset((uint8_t*) recdata, 0, ADC_MEMORY_SIZE);
  
    audio_pause_DA(0); 
    audio_pause_AD(0); 
    i2s_out_bist_pause(0);  
    //sizecount = ADC_MEMORY_SIZE/2;
    
    while(sizecount < ADC_MEMORY_SIZE){
        
        if((sizecount) >= ADC_MEMORY_SIZE) break;
            
        if(audio_RX_Data_Recv(&spec_adI,psrc,bsize)){
            if(first){
                first = false;
            }else{
                memcpy(recdata+sizecount,psrc,bsize);
                sizecount += bsize;
                audio_AD_wait_time(&spec_adI,bsize);
                printf(".");
            }
        }

    }
    printf("\n");
    i2s_out_bist_pause(1);
    audio_pause_AD(1); 
    audio_pause_DA(1); 
    if(psrc) free(psrc);
}  

void* TestFunc(void* arg)
{
    int fail=0;
#ifdef USB_DUMP
    char *filename = "a:/record.wav";
    wave_header_t wavehead;
    playdata *d;
#endif
    itpInit();
    sleep(1);  
    
    initDA();
    initAD();
#ifdef USB_DUMP    
    d = INITplaydata();
    openfilewb(d,filename);
#endif
    audio_DA_volume_set(10);//set rec 0~100 (faraday : level 60= 0db )
    audio_AD_volume_set(10);//set rec 0~100 (faraday : level 60= 0db )
    recvoice();
#ifdef USB_DUMP
      
    write_wav_header(d,&spec_adI,ADC_MEMORY_SIZE);
    fwrite(recdata,ADC_MEMORY_SIZE,1,d->fd);
 
    if(d->fd){
        fclose(d->fd);
    }
    
    printf("save file in a:/record.wav\n");
    printf("now check USB disk file\n");
#else
    {
        int i=0;
        int POS= ADC_MEMORY_SIZE/2;
        int length = 800;
        int halfL=0;
        int halfR=0;
        int countL=0;
        int countR=0;
        bool startL = false;
        bool startR = false;
        int tmpL =*((int16_t*)(recdata+POS  ));
        int tmpR =*((int16_t*)(recdata+POS+2));        
        
        for(i=POS;i<=POS+length;i+=4){
            int tmp ;
            //L channel start
            tmp = *((int16_t*)(recdata+i  ));
            if(tmp*tmpL < 0 && !startL) {
                startL=true;
            }
            if(startL){
                if(tmp*tmpL < 0 && countL>2) {
                    halfL++;
                    if( halfL == 2) {
                        halfL = 0;
                    if(countL != 48) {printf("L channel may fail countL=%d\n",countL);fail++;}
                        //printf("L countL = %d\n",countL);
                        countL = 0;
                    }
                }
                countL++;
            }
            tmpL=tmp;
            //L channel end
            //R channel start
            tmp = *((int16_t*)(recdata+i+2));
            if(tmp*tmpR < 0 && !startR) {
                startR=true;
            }
            if(startR){
                if(tmp*tmpR < 0 && countR>2) {
                    halfR++;
                    if( halfR == 2) {
                        halfR = 0;
                        if(countR != 48) {printf("R channel may fail countR=%d\n",countR); fail++;}
                        //printf("R countR = %d\n",countR);
                        countR = 0;
                    }
                }
                countR++;
            }
            tmpR=tmp;           
            // R channel end
        }
    }
    if(fail){
        printf("I2S test fail\n");
    }else{
        printf("I2S test pass\n");
    }
#endif
    return NULL;
}