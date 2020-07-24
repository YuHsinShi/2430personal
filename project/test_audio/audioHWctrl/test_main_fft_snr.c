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
#include "fft/fft.h"
#include <math.h>

//#define USB_DUMP 1

#define ADC_MEMORY_SIZE (16*1024) 
#define INPUTTONE  1125
#define SRATE      48000
#define THRESHHOLD 0.007

static unsigned char ring[] = {
#include "testwav/48K_1125Hz.hex"
//#include "testwav/32K_1000Hz.hex"
//#include "testwav/16K_1000Hz.hex"
//#include "testwav/8K_1000Hz.hex"
};

extern complex_of_N_FFT data_of_N_FFT[N_FFT];
extern ElemType SIN_TABLE_of_N_FFT[Npart4_of_N_FFT+1];

unsigned char *recdata;

/**********************************************************/

static void dac_write_reg(unsigned char reg_addr, unsigned int value)
{
    ithWriteRegA(0xD0100000|reg_addr, value);
}

static unsigned int dac_read_reg(unsigned char reg_addr)
{
    unsigned int value = ithReadRegA(0xD0100000|reg_addr);
    return value;
}

static inline void dac_writeRegMask(unsigned char addr, unsigned int data, unsigned int mask)
{
    dac_write_reg(addr, (dac_read_reg(addr) & ~mask) | (data & mask));
}

void initAD(){
    /* init ADC */ 
    adc_buf = (uint8_t*)malloc(ADC_BUFFER_SIZE);
	memset((uint8_t*) adc_buf, 0, ADC_BUFFER_SIZE);
	memset((void*)&spec_adI, 0, sizeof(STRC_I2S_SPEC));
    spec_adI.channels					= 2;
    spec_adI.sample_rate				= SRATE;
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
    spec_daI.sample_rate              = SRATE;
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
    uint8_t  *rpsrc     = NULL;
    uint16_t rbsize     = audio_get_bsize_from_bps(&spec_daI);
    uint32_t dataSize  = sizeof(ring);
    uint32_t rsizecount = 0;
    uint32_t sizecount = 0; 
    rpsrc = (u8 *)malloc(rbsize);
    
    psrc = (u8*)malloc(bsize);       
    recdata = (uint8_t*)malloc(ADC_MEMORY_SIZE);
    memset((uint8_t*) recdata, 0, ADC_MEMORY_SIZE);
  
    audio_pause_DA(0); 
    audio_pause_AD(0); 
    //sizecount = ADC_MEMORY_SIZE/2;
    
    while(sizecount < ADC_MEMORY_SIZE){
        
        if (rsizecount + rbsize < dataSize)
        {
            memcpy(rpsrc, ring + rsizecount, rbsize);
            rsizecount += rbsize;
        }
        else
        {
            if (rsizecount < dataSize)
            {
                uint32_t szsec0 = dataSize - rsizecount;
                uint32_t szsec1 = rbsize - szsec0;
                memcpy(rpsrc, ring + rsizecount, szsec0);
                rsizecount += szsec0; // now :rsizecount = dataSize;
                memset(rpsrc + szsec0, 0, szsec1);
                rsizecount += szsec1;
            }
            rsizecount = 0;
        }

        audio_TX_Data_Send(&spec_daI, rpsrc, rbsize);
        audio_DA_wait_time(&spec_daI, rbsize);
        
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
    
    initDA();
    initAD();

#ifdef USB_DUMP   
    sleep(2);   
    d = INITplaydata();
    openfilewb(d,filename);
#endif
    dac_writeRegMask(0xD4,0x0 << 25,0x3 << 25);//line in +0db
    dac_writeRegMask(0xE4, 0x61<<24 | 0x61<<16, 0x3F<<24 | 0x3F<<16);//ADC digtal gain +0db
    audio_DA_volume_set(86);//set HP  0~100 (faraday : level 86= 0db )
    audio_AD_volume_set(60);//set rec 0~100 (faraday : level 60= 0db )
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
        //FFT measure
        int i=0;
        int POS= ADC_MEMORY_SIZE/2;
        int NumberF=floor((float)(INPUTTONE*N_FFT)/SRATE+0.5);
        float sumL = 0;
        float sumR = 0;
        float SNRL = 0;
        float SNRR = 0;
        printf("*********************info**********************\n");
        printf("INPUTTONE = %d, SRATE = %d,\n",INPUTTONE,SRATE,N_FFT);
        printf("N_FFT = %d, NumberF = %d \n",N_FFT,NumberF);
        printf("***********************************************\n");
        Init_FFT();	
        for(i=0;i<=N_FFT;i++){
            int tmp;
            float pp;
            tmp = *((int16_t*)(recdata+POS+i*4));
            pp = (float)tmp/32767.0;
            data_of_N_FFT[i].real = pp;
            data_of_N_FFT[i].imag = 0;
        }

        FFT();
        //for(i=NumberF-1; i<NumberF+2; i++)
        //    {printf("[%d %.2f(Hz) %f ]",i,i*((float)SRATE/N_FFT),FFT_RESULT(i));}        
        //printf("\n");
        for(i=1; i<Npart2_of_N_FFT;i++)
        {
            if(i!=NumberF && FFT_RESULT(i)>=THRESHHOLD) sumL+=FFT_RESULT(i);
        }
        SNRL=20*log(FFT_RESULT(NumberF)/sumL);

        printf("Left SNR = 20*log((%f)/(%f)) %f\n",FFT_RESULT(NumberF),sumL,SNRL);

        for(i=0;i<=N_FFT;i++){
            int tmp;
            float pp;
            tmp = *((int16_t*)(recdata+POS+i*4+2));
            pp = (float)tmp/32767.0;
            data_of_N_FFT[i].real = pp;
            data_of_N_FFT[i].imag = 0;
        }

        FFT();
        
        //for(i=NumberF-1; i<NumberF+2; i++)
        //    {printf("[%d %.2f(Hz) %f ]",i,i*((float)SRATE/N_FFT),FFT_RESULT(i));} 
        //printf("\n");
        for(i=1; i<Npart2_of_N_FFT;i++)
        {
            if(i!=NumberF && FFT_RESULT(i)>=THRESHHOLD) sumR+=FFT_RESULT(i);
        }
        SNRR=20*log(FFT_RESULT(NumberF)/sumR);

        printf("Right SNR = 20*log((%f)/(%f)) %f\n",FFT_RESULT(NumberF),sumR,SNRR);        
     
        if(SNRL>100) printf("left channel pass\n");
        else printf("left channel fail\n");
        
        if(SNRR>100) printf("right channel pass\n");
        else printf("right channel fail\n");

    }

#endif
    return NULL;
}