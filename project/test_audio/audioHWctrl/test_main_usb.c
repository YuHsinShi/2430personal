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

#define Loop_test
//#define Volume_test
/**********************************************************/


void initDA(wave_header_t wheader){
	/* init DAC */
    dac_buf = (uint8_t*)malloc(DAC_BUFFER_SIZE);
	memset((uint8_t*) dac_buf, 0, DAC_BUFFER_SIZE);
	memset((void*)&spec_daI, 0, sizeof(STRC_I2S_SPEC));
    spec_daI.channels                 = wheader.format_chunk.channel;
    spec_daI.sample_rate              = wheader.format_chunk.rate;
    spec_daI.buffer_size              = DAC_BUFFER_SIZE;
    spec_daI.is_big_endian            = 0;
    spec_daI.base_i2s                 = (uint8_t*) dac_buf;
    spec_daI.sample_size              = wheader.format_chunk.bitpspl;
    spec_daI.num_hdmi_audio_buffer    = 1;
    spec_daI.is_dac_spdif_same_buffer = 1;
    spec_daI.enable_Speaker           = 1;
    spec_daI.enable_HeadPhone         = 1;
    spec_daI.postpone_audio_output    = 1;
    spec_daI.base_spdif                = (uint8_t*) dac_buf;
    //i2s_init_DAC(&spec_daI);
    audio_init_DA(&spec_daI);
}

void playring(playdata *d,bool loop){

	u32 DA_r;
	u32 DA_w;
	u32 DA_free;
    u32 DA_r_pre;
	u8 *psrc=NULL;
	//int bsize=256;
    int bsize = audio_get_bsize_from_bps(&spec_daI);
    
    int err;
    int TotalPlayedData = 0;
    audio_pause_DA(0);
    I2S_DA32_SET_WP(I2S_DA32_GET_RP());
    DA_r_pre = I2S_DA32_GET_RP();
    psrc = (unsigned char *)malloc(bsize);
    while(1){       

        err = fread(psrc,1,bsize,d->fd);
        if(err>=0){
            if(err!=0){
                if(err<bsize){
                    memset(psrc+err,0,bsize);
                }
            }
            if(err<bsize){
                   fseek(d->fd, d->hsize, SEEK_SET);
            }  
        }
        
        audio_TX_Data_Send(&spec_daI,psrc,bsize);
        audio_DA_wait_time(&spec_daI,bsize);
                
        if(!loop){
            int shift;
            DA_r = I2S_DA32_GET_RP();
            shift = (DA_r >= DA_r_pre) ? (DA_r-DA_r_pre): (DA_r + DAC_BUFFER_SIZE -DA_r_pre);
            TotalPlayedData += shift; 
            DA_r_pre = DA_r;

            if(d->playlenth < TotalPlayedData)
                break;      
        }
        
    }
    free(psrc);
    audio_pause_DA(1);
}

#if 1
void* TestFunc(void* arg)
{
    char *filename = "a:/test.wav";
    bool loop = true; //LOOP play ,
    wave_header_t wavehead;
    playdata *d;
    itpInit();
    sleep(1);

    d = INITplaydata();
    openfile(d,filename);
    get_wav_header(d,&wavehead);
    
    initDA(wavehead);
    audio_DA_volume_set(50);//set volume 0~100 (level 100 = 0db gain)
    playring(d,loop);
    
    
    if(d->fd){
        fclose(d->fd);
    }

    return NULL;
}
#endif

#if 0
void* TestFunc(void* arg)
{
    char *filename[10] = {"a:full/test.wav",
                          "a:full/ring48K.wav",
                          "a:full/ring44K.wav",
                          "a:full/ring32K.wav",
                          "a:full/ring24K.wav",
                          "a:full/ring22K.wav",
                          "a:full/ring16K.wav",
                          "a:full/ring12K.wav",
                          "a:full/ring11K.wav",
                          "a:full/ring8K.wav",
    };
    bool loop = false; //LOOP play ,
    int i;
    itpInit();
    sleep(1);

    for(i=1;i<10;i++){
        wave_header_t wavehead;
        
        playdata *d = (playdata*)INITplaydata();
        
        openfile(d,filename[i]);
        get_wav_header(d,&wavehead);   
        initDA(wavehead);
        audio_DA_volume_set(i*10);//set volume 0~100 (level 100 = 0db gain)
        printf("level = %d\n",i*10);
        
       /* if(i%5==0)      GPIO_switch_set(DOUBLE_TELETUBE);
        else if(i%5==1) GPIO_switch_set(HEADSET);
        else if(i%5==2) GPIO_switch_set(TELEPHONETUBE);
        else if(i%5==3) GPIO_switch_set(HANDFREE);
        else if(i%5==4) GPIO_switch_set(DOUBLE_HEADSET);
       */ 
        playring(d,loop);
        
        DEINITplaydata(d);
        audio_deinit_DA();
        sleep(1);
    }
    
    printf("end\n");
    return NULL;
}
#endif