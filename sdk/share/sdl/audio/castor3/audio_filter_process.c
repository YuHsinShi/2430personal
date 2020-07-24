#include <assert.h>
#include <pthread.h>

#include "ring_buffer.h"
#include "audio_mgr.h"

typedef enum audio_state {
    ENUM_AUDIO_STATE_STOP = 0,
    ENUM_AUDIO_STATE_PLAYING,
} AUDIO_STATE;

typedef struct audio_process {
    int readIdx;
    AUDIO_STATE state;
    RingBuffer *soundRead;
    RingBuffer *soundWrite;
    uint8_t    *pOutBuff;
    pthread_t  *audioTxThread;
    pthread_t  *filterThread;
    pthread_mutex_t readMutex;
    pthread_mutex_t writeMutex;
    STRC_I2S_SPEC spec;
} AUDIO_PROCESS;

static AUDIO_PROCESS gAudioProcess;

static void *audio_txthread_func(void *arg);
static void *filter_thread_func(void *arg);

static inline void audio_filter_init()
{
    gAudioProcess.state = ENUM_AUDIO_STATE_PLAYING;
    gAudioProcess.readIdx = 0;
    pthread_mutex_init(&gAudioProcess.readMutex, NULL);
    pthread_mutex_init(&gAudioProcess.writeMutex, NULL);
    gAudioProcess.soundRead = WebRtc_CreateBuffer(128*1024, sizeof(char));
    gAudioProcess.soundWrite = WebRtc_CreateBuffer(128*1024, sizeof(char));
    gAudioProcess.filterThread = (pthread_t *)malloc(sizeof(pthread_t));
    gAudioProcess.audioTxThread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(gAudioProcess.filterThread, NULL, filter_thread_func, NULL);
    pthread_create(gAudioProcess.audioTxThread, NULL, audio_txthread_func, NULL);
}

static inline void audio_filter_uninit()
{
    if(gAudioProcess.state == ENUM_AUDIO_STATE_STOP) return;
    gAudioProcess.state = ENUM_AUDIO_STATE_STOP;
    pthread_join(*gAudioProcess.filterThread, NULL);
    pthread_join(*gAudioProcess.audioTxThread, NULL);
    WebRtc_FreeBuffer(gAudioProcess.soundWrite);
    WebRtc_FreeBuffer(gAudioProcess.soundRead);
    pthread_mutex_destroy(&gAudioProcess.readMutex);
    pthread_mutex_destroy(&gAudioProcess.writeMutex);
    free(gAudioProcess.filterThread);
    free(gAudioProcess.audioTxThread);
    free(gAudioProcess.pOutBuff);
    free(gAudioProcess.spec.base_i2s);
    // printf("audio_filter_uninit()!!!!! &&&&\n");
}

//=======soundread **=======

static inline void swap16_buff(uint16_t *buff, int len){
#define TOSHORT(n) ((((n)>>8)&0x00ff)+(((n)<<8)&0xff00))
    int i;
    for(i=0;i<len;i++){
        buff[i] = TOSHORT(buff[i]);
    }
}

static inline int get_pcmIdx()
{
#if (CFG_CHIP_FAMILY == 970)
    #define TOINT(n) ((((n)>>24)&0xff)+(((n)>>8)&0xff00)+(((n)<<8)&0xff0000)+(((n)<<24)&0xff000000))
#elif (CFG_CHIP_FAMILY == 9860)
    #define TOINT(n)   n
#endif
    unsigned int* pBuf;
    unsigned int nLength;
    int nTemp;
#if defined(__OPENRTOS__)

    pBuf = (unsigned char*)iteAudioGetAudioCodecAPIBuffer(&nLength);
    nTemp = TOINT(pBuf[0]);
#endif
    // printf("get_pcmIdx(%d)\n", nTemp);
    return nTemp;
}

int audioFilterProcessSoundRead()
{
    int pcmIdx = get_pcmIdx();
    uint32_t bsize=0;
    unsigned char* rawBuf;
    int nBufferLength;
    iteAudioGetAttrib(  ITE_AUDIO_I2S_PTR,                  &rawBuf);
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nBufferLength);
    
    if(gAudioProcess.state == ENUM_AUDIO_STATE_STOP) return 0;
    
    if (gAudioProcess.readIdx <= pcmIdx)
    {
        bsize = pcmIdx - gAudioProcess.readIdx;
        if (bsize)
        {
            ithInvalidateDCacheRange(rawBuf + gAudioProcess.readIdx, bsize);
            while(WebRtc_available_write(gAudioProcess.soundRead)<bsize && gAudioProcess.state);
            pthread_mutex_lock(&gAudioProcess.readMutex);
            WebRtc_WriteBuffer(gAudioProcess.soundRead, rawBuf + gAudioProcess.readIdx, bsize);
            pthread_mutex_unlock(&gAudioProcess.readMutex);
        }
    }
    else
    {
        bsize = (nBufferLength - gAudioProcess.readIdx) + pcmIdx;
        if (bsize)
        {
            
            uint32_t szsec0 = nBufferLength - gAudioProcess.readIdx;
            uint32_t szsec1 = bsize - szsec0;
            if (szsec0)
            {
                ithInvalidateDCacheRange(rawBuf + gAudioProcess.readIdx, szsec0);
                while(WebRtc_available_write(gAudioProcess.soundRead)<szsec0 && gAudioProcess.state);
                pthread_mutex_lock(&gAudioProcess.readMutex);
                WebRtc_WriteBuffer(gAudioProcess.soundRead, rawBuf + gAudioProcess.readIdx, szsec0);
                pthread_mutex_unlock(&gAudioProcess.readMutex);
            }
            if (szsec1)
            {
                ithInvalidateDCacheRange(rawBuf, szsec1);
                while(WebRtc_available_write(gAudioProcess.soundRead)<szsec1 && gAudioProcess.state);
                pthread_mutex_lock(&gAudioProcess.readMutex);
                WebRtc_WriteBuffer(gAudioProcess.soundRead, rawBuf, szsec1);
                pthread_mutex_unlock(&gAudioProcess.readMutex);
            }
        }
    }
    gAudioProcess.readIdx = pcmIdx;
    return 0;
}

//=======soundread &&=======

//=======filter **=======
static inline void equalizer_filter()
{
#define POINTBYTES 512
    uint8_t buff[POINTBYTES] = {0};
    
    while(WebRtc_available_read(gAudioProcess.soundRead) >= POINTBYTES && gAudioProcess.state)
    {
        const uint8_t *out_ptr;
        pthread_mutex_lock(&gAudioProcess.readMutex);
        WebRtc_ReadBuffer(gAudioProcess.soundRead, (void **)&out_ptr, buff, POINTBYTES);
        pthread_mutex_unlock(&gAudioProcess.readMutex);
        if (out_ptr != buff)
        {
            memcpy(buff, out_ptr, POINTBYTES);
        }
#ifdef CFG_AUDIO_EQUALIZER_FILTER
        audioEqualizerProcess(buff, buff);
#endif
        while(WebRtc_available_write(gAudioProcess.soundWrite) < POINTBYTES && gAudioProcess.state);
        pthread_mutex_lock(&gAudioProcess.writeMutex);
        WebRtc_WriteBuffer(gAudioProcess.soundWrite, buff, POINTBYTES);
        pthread_mutex_unlock(&gAudioProcess.writeMutex);
    }
}

static void *filter_thread_func(void *arg)
{
    while (gAudioProcess.state == ENUM_AUDIO_STATE_PLAYING)
    {
        equalizer_filter();
    }
    // printf("filter_thread_func end &&&&&\n");
}

//=======filter &&=======

//=======soundwrite **=======

static inline void audio_tx_data_send(
    STRC_I2S_SPEC *i2s_spec,
    unsigned char *ptr,
    int size)
{
	uint32_t DA_w = I2S_DA32_GET_WP();

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
}

static void *audio_txthread_func(void *arg)
{
    I2S_DA32_SET_WP(I2S_DA32_GET_RP());
    int bsize=(640*gAudioProcess.spec.sample_rate*gAudioProcess.spec.channels)/8000;
    gAudioProcess.pOutBuff=(uint8_t*)malloc(bsize);

    while (gAudioProcess.state == ENUM_AUDIO_STATE_PLAYING)
    {
        uint32_t DA_r, DA_w, DA_free;
        DA_r = I2S_DA32_GET_RP();
        DA_w = I2S_DA32_GET_WP();            
        DA_free = (DA_w >= DA_r) ? ((gAudioProcess.spec.buffer_size - DA_w) + DA_r): (DA_r - DA_w);

        if (DA_free < (gAudioProcess.spec.buffer_size/8) && DA_free < (uint32_t)bsize){//workaround 
            usleep(5000);//when DA_free almost full add sleep time avoid audio miss
            continue;
        }
        
        // printf("%d\n",WebRtc_available_read(gAudioProcess.soundWrite));
        if(WebRtc_available_read(gAudioProcess.soundWrite)<bsize)
        {
            usleep(5000); // for playing case, wait for data readed from file
            continue;
        }
        if (gAudioProcess.state == ENUM_AUDIO_STATE_STOP) break;
        const uint8_t *out_ptr;
        pthread_mutex_lock(&gAudioProcess.writeMutex);
        WebRtc_ReadBuffer(gAudioProcess.soundWrite,(void **)&out_ptr,gAudioProcess.pOutBuff,bsize);
        pthread_mutex_unlock(&gAudioProcess.writeMutex);
        if (out_ptr != gAudioProcess.pOutBuff)
        {
            memcpy(gAudioProcess.pOutBuff, out_ptr, bsize);
        }
        
        audio_tx_data_send(&gAudioProcess.spec, gAudioProcess.pOutBuff, bsize);
        usleep(5000);
    }
    // printf("audio_txthread_func end &&&&&\n");
}

//=======soundwrite &&=======

void audioFilterProcessInit(int nChannels, int nSampeRate)
{
    int nBufferLength;
    // printf("audioFilterProcessInit(%d)(%d)\n", nChannels, nSampeRate);

    /* init I2S */
    if(i2s_get_DA_running())
        i2s_deinit_DAC();
    nBufferLength = 128*1024;
    uint8_t* dac_buf = (uint8_t*)malloc(nBufferLength);
    assert(dac_buf);
    memset(dac_buf, 0, nBufferLength);
    
    memset(&gAudioProcess.spec,0,sizeof(gAudioProcess.spec));
    gAudioProcess.spec.channels                 = nChannels;
    gAudioProcess.spec.sample_rate              = nSampeRate;
    gAudioProcess.spec.buffer_size              = nBufferLength;
    gAudioProcess.spec.is_big_endian            = 0;
    gAudioProcess.spec.base_i2s                 = dac_buf;

    gAudioProcess.spec.enable_Speaker          = 1;
    gAudioProcess.spec.sample_size              = 16;
    gAudioProcess.spec.num_hdmi_audio_buffer    = 1;
    gAudioProcess.spec.is_dac_spdif_same_buffer = 1;
    gAudioProcess.spec.base_hdmi[0]             = dac_buf;
    gAudioProcess.spec.base_hdmi[1]             = dac_buf;
    gAudioProcess.spec.base_hdmi[2]             = dac_buf;
    gAudioProcess.spec.base_hdmi[3]             = dac_buf;
    gAudioProcess.spec.base_spdif               = dac_buf;
    i2s_init_DAC(&gAudioProcess.spec);
    
    if(gAudioProcess.state == ENUM_AUDIO_STATE_PLAYING)
        audio_filter_uninit();
    audio_filter_init();
#ifdef CFG_AUDIO_EQUALIZER_FILTER
    audioEqualizerInit(nSampeRate);
#endif
}

void audioFilterProcessExit()
{
    iteAudioSetMusicCodecDump(0);
    audio_filter_uninit();
}

void audioFilterProcessEnable()
{
    iteAudioSetMusicCodecDump(1);
}

void audioFilterProcessDisable()
{
    iteAudioSetMusicCodecDump(0);
}