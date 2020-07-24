#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "i2s/i2s.h"    

unsigned int DA_waitratio=8;
unsigned int AD_waitratio=8;

static inline uint32_t _get_DA_avail_size(uint32_t buflen){
    uint32_t rp = I2S_DA32_GET_RP();
    uint32_t wp = I2S_DA32_GET_WP();
    return (wp >= rp) ? (wp - rp):((buflen - rp) + wp);
}


static inline uint32_t _get_AD_avail_size(uint32_t buflen){
    uint32_t rp = I2S_AD32_GET_RP();
    uint32_t wp = I2S_AD32_GET_WP();
    return (wp >= rp) ? (wp - rp):((buflen - rp) + wp);
}

static inline void _wait_time_ctrl(int sample,int size,int waitRatio,int i2s_buffer_size){
    
    int SleepTime = 0 ;
    if     (                                 (sample <= i2s_buffer_size/4)){SleepTime=0;               }
    else if((sample >= i2s_buffer_size/4) && (sample <  i2s_buffer_size/2)){SleepTime=waitRatio*size  ;}
    else if((sample >= i2s_buffer_size/2) && (sample <3*i2s_buffer_size/4)){SleepTime=waitRatio*size*2;}
    else                                                                   {SleepTime=waitRatio*size*3;}     
    //printf("sample=%d waitRatio=%d size=%d SleepTime=%d\n",sample,waitRatio,size,SleepTime);
    usleep(SleepTime);
}

int _set_wait_ratio(int SampleRate,int Channels){
    int div = 1;
    int _ratio = 1;
    switch (SampleRate)
    {
        case 48000 : {div = 48; break;}
        case 44100 : {div = 32; break;}
        case 22050 : {div = 16; break;}
        case 11025 : {div =  8; break;}        
        case 32000 : {div = 32; break;}
        case 16000 : {div = 16; break;}
        case  8000 : {div =  8; break;}
        default:
        div = 16;
        break;        
    }
    switch (Channels)
    {
        case 1 : {div *= 1; break;}
        case 2 : {div *= 2; break;}
        default:
        div *= 1;
        break;        
    }
    
    
    _ratio = 1000/(div*2);
    printf("div = %d;\n",div);
    printf("_ratio = %d;\n",_ratio);
    return _ratio;
}

int audio_get_bsize_from_bps(STRC_I2S_SPEC *i2s_spec){
    int bps;
    int _size;
    bps = i2s_spec->sample_rate * i2s_spec->sample_size * i2s_spec->channels/1000;
    
    if     ((bps >= 1200)               ){_size =1600;}
    else if((bps >= 1024)&&(bps <  1200)){_size =1280;}
    else if((bps >=  512)&&(bps <  1024)){_size = 640;}
    else if((bps >=  256)&&(bps <   512)){_size = 320;}
    else                                 {_size = 320;} 
    printf("bps = %d _size = %d \n",bps,_size);
    return _size;
}

uint32_t audio_get_avail_size(uint32_t rp,uint32_t wp,uint32_t buflen){
    return (wp >= rp) ? ((buflen - wp) + rp): (rp - wp);
}

void audio_init_DA(STRC_I2S_SPEC *i2s_spec){
    i2s_init_DAC(i2s_spec);
    DA_waitratio = _set_wait_ratio(i2s_spec->sample_rate,i2s_spec->channels);
}

void audio_init_AD(STRC_I2S_SPEC *i2s_spec){
    i2s_init_ADC(i2s_spec);
    AD_waitratio = _set_wait_ratio(i2s_spec->sample_rate,i2s_spec->channels);
}

void audio_deinit_DA(){
    i2s_deinit_DAC();
}

void audio_deinit_AD(){
    i2s_deinit_ADC();
}

void audio_pause_DA(int pause){

    i2s_pause_DAC(pause);
}

void audio_pause_AD(int pause){
    i2s_pause_ADC(pause);
}

void audio_mute_DA(int mute){
    i2s_mute_DAC(mute);
}

void audio_mute_AD(int mute){
    i2s_mute_ADC(mute);
}

void audio_DA_volume_set(unsigned level){
    i2s_set_direct_volperc(level);//0~100;
}

void audio_AD_volume_set(unsigned level){
    i2s_ADC_set_rec_volperc(level);
}

int audio_TX_Data_Send(
    STRC_I2S_SPEC *i2s_spec,
    unsigned char *ptr,
    int size)
{
    uint32_t DA_r;
	uint32_t DA_w;
    uint32_t DA_free;
    uint32_t DA_data;
       
	DA_r = I2S_DA32_GET_RP();
	DA_w = I2S_DA32_GET_WP();

    DA_free = (DA_w >= DA_r) ? ((i2s_spec->buffer_size - DA_w) + DA_r): (DA_r - DA_w);
    //DA_data = i2s_spec->buffer_size - DA_free; 
    
    if(DA_free < size){
        printf("i2s buffer full set DA_w(%d) = DA_r(%d) (miss some data play)\n",DA_w,DA_r);
        I2S_DA32_SET_WP(DA_r);
        DA_w = DA_r;
    }
    
    if ((DA_w + size) > (uint32_t)i2s_spec->buffer_size){
        int szbuf = i2s_spec->buffer_size - DA_w;

        if (szbuf > 0)
        {
            memcpy(i2s_spec->base_i2s + DA_w, ptr, szbuf);
            ithFlushDCacheRange(i2s_spec->base_i2s+DA_w, szbuf);
        }
        DA_w = size - szbuf;
        memcpy(i2s_spec->base_i2s, ptr + szbuf, DA_w);
        ithFlushDCacheRange(i2s_spec->base_i2s, DA_w);
    }
    else
    {
        memcpy(i2s_spec->base_i2s + DA_w, ptr, size);
        ithFlushDCacheRange(i2s_spec->base_i2s+DA_w, size);
        DA_w += size;
    }
    
    if (DA_w == i2s_spec->buffer_size) DA_w = 0;

    I2S_DA32_SET_WP(DA_w);
    
    return 0;
}

int audio_RX_Data_Recv(
    STRC_I2S_SPEC *i2s_spec,
    unsigned char *ptr,
    int size)
{

	uint32_t AD_r = I2S_AD32_GET_RP();
    uint32_t AD_w = I2S_AD32_GET_WP();
    uint32_t AD_data;
   
    AD_data = (AD_w >= AD_r) ? (AD_w - AD_r):((i2s_spec->buffer_size - AD_r) + AD_w);
    if(AD_data < size) return 0;

    if (AD_r + size <= i2s_spec->buffer_size)
    {
        //bsize = AD_w - AD_r;  
        ithInvalidateDCacheRange(i2s_spec->base_i2s + AD_r, size);
        memcpy(ptr, i2s_spec->base_i2s + AD_r, size);
        AD_r += size;
    }
    else
    { // AD_r > AD_w
        uint32_t szsec0 = i2s_spec->buffer_size - AD_r;
        uint32_t szsec1 = size - szsec0;
        if (szsec0)
        {
            ithInvalidateDCacheRange(i2s_spec->base_i2s + AD_r, szsec0);
            memcpy(ptr, i2s_spec->base_i2s + AD_r, szsec0);
        }
        ithInvalidateDCacheRange(i2s_spec->base_i2s, szsec1);
        memcpy(ptr + szsec0, i2s_spec->base_i2s, szsec1);
        AD_r = szsec1;
    }
    
    I2S_AD32_SET_RP(AD_r);

    if(AD_r == i2s_spec->buffer_size ) AD_r=0;

    return 1;
}

int audio_HDMIRX_Data_Recv(
    STRC_I2S_SPEC *i2s_spec,
    unsigned char *ptr,
    int size ,int number)
{

	uint32_t AD_r = I2S_AD32_GET_HDMI_RP();
    uint32_t AD_w = I2S_AD32_GET_HDMI_WP();
    uint32_t AD_data;
   
    AD_data = (AD_w >= AD_r) ? (AD_w - AD_r):((i2s_spec->buffer_size - AD_r) + AD_w);
    if(AD_data < size) return 0;

    if (AD_r + size <= i2s_spec->buffer_size)
    {
        //bsize = AD_w - AD_r;
        ithInvalidateDCacheRange(i2s_spec->base_hdmi[number] + AD_r, size);
        memcpy(ptr, i2s_spec->base_hdmi[number] + AD_r, size);
        AD_r += size;    
    }
    else
    { // AD_r > AD_w
        uint32_t szsec0 = i2s_spec->buffer_size - AD_r;
        uint32_t szsec1 = size - szsec0;
        if (szsec0)
        {
            ithInvalidateDCacheRange(i2s_spec->base_hdmi[number] + AD_r, szsec0);
            memcpy(ptr, i2s_spec->base_hdmi[number] + AD_r, szsec0);
        }
        ithInvalidateDCacheRange(i2s_spec->base_hdmi[number], szsec1);
        memcpy(ptr + szsec0, i2s_spec->base_hdmi[number], szsec1);
        AD_r = szsec1;
    }
    
    I2S_AD32_SET_HDMI_RP(AD_r);

    if(AD_r == i2s_spec->buffer_size ) AD_r=0;

    return 1;
}

void audio_DA_wait_time(STRC_I2S_SPEC *i2s_spec,int size){        
    uint32_t DA_data = _get_DA_avail_size(i2s_spec->buffer_size);
    _wait_time_ctrl(DA_data,size,DA_waitratio,i2s_spec->buffer_size);
}

void audio_AD_wait_time(STRC_I2S_SPEC *i2s_spec,int size){  

    uint32_t AD_data = _get_AD_avail_size(i2s_spec->buffer_size); 
    uint32_t AD_free = i2s_spec->buffer_size - AD_data;
    _wait_time_ctrl(AD_free,size,AD_waitratio,i2s_spec->buffer_size);   
}