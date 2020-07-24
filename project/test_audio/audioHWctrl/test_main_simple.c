#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "i2s/i2s.h"
#include "test.h"

#define _ALLOC_HW_BUFFER(size, flags) (uint8_t *)ithMapVram(itpVmemAlignedAlloc(32, (size)), size, (flags))
#define I2S_REG_OUT_BIST            (0xD0100000 | 0x98)

static unsigned char ring[] = {
//#include "testwav/opendoor_8000_16_1.hex"
//#include "testwav/tone/R1000HZ_L500HZ.hex"
#include "testwav/test_48K_16bit.hex"
};

void i2s_out_bist_pause(int bist)
{
    if(bist){
        ithWriteRegMaskA(I2S_REG_OUT_BIST, 0, 1<<0);
    }else{
        ithWriteRegMaskA(I2S_REG_OUT_BIST, 1, 1<<0);
    } 
}

/**********************************************************/

void initDA()
{
    /* init DAC */
    dac_buf = _ALLOC_HW_BUFFER(DAC_BUFFER_SIZE, ITH_VRAM_WRITE);
    memset((uint8_t *) dac_buf, 0, DAC_BUFFER_SIZE);
    ithFlushDCacheRange(dac_buf, DAC_BUFFER_SIZE);
    memset((void *)&spec_daI, 0, sizeof(STRC_I2S_SPEC));
    spec_daI.channels                 = 2;
    spec_daI.sample_rate              = 48000;
    spec_daI.buffer_size              = DAC_BUFFER_SIZE;
    spec_daI.is_big_endian            = 0;
    spec_daI.base_i2s                 = (uint8_t *) dac_buf;
    spec_daI.sample_size              = 16;
    spec_daI.num_hdmi_audio_buffer    = 1;
    spec_daI.is_dac_spdif_same_buffer = 1;
    spec_daI.enable_Speaker           = 1;
    spec_daI.enable_HeadPhone         = 1;
    spec_daI.postpone_audio_output    = 1;
    spec_daI.base_spdif               = (uint8_t *) dac_buf;
    //i2s_init_DAC(&spec_daI);
    audio_init_DA(&spec_daI);
}

void playring()
{
    uint8_t  *psrc     = NULL;
    uint16_t bsize     = audio_get_bsize_from_bps(&spec_daI);
    uint32_t dataSize  = sizeof(ring);
    uint32_t sizecount = 0;
    psrc = (u8 *)malloc(bsize);
    printf("play\n");
    audio_pause_DA(0);
    i2s_out_bist_pause(0);
    sleep(2);
    i2s_out_bist_pause(1);

    while (1)
    {
        if (sizecount + bsize < dataSize)
        {
            memcpy(psrc, ring + sizecount, bsize);
            sizecount += bsize;
        }
        else
        {
            if (sizecount < dataSize)
            {
                uint32_t szsec0 = dataSize - sizecount;
                uint32_t szsec1 = bsize - szsec0;
                memcpy(psrc, ring + sizecount, szsec0);
                sizecount += szsec0; // now :sizecount = dataSize;
                memset(psrc + szsec0, 0, szsec1);
                sizecount += szsec1;
            }
            sizecount = 0;
        }

        audio_TX_Data_Send(&spec_daI, psrc, bsize);
        audio_DA_wait_time(&spec_daI, bsize);
    }
}

void *TestFunc(void *arg)
{
    int i = 50;
    itpInit();
    initDA();
    printf("*******init DA********\n");
/*****************play sound**********************************************/
    while (1)
    {
        printf("vol = %d\n", i);
        audio_DA_volume_set(i);//set volume 0~100
        playring();
    }
/*************************************************************************/
    return NULL;
}