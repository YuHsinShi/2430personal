#include <assert.h>
#include <time.h>

#include "mediastreamer2/mssndcard.h"
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/msfileplayer.h"
#include "ite/itp.h"
//#include "openrtos/FreeRTOS.h"

#include "i2s/i2s.h"
#include "ite/audio.h"

#ifdef CFG_RISC2_OPUS_CODEC
#include "risc2/risc2_device.h"
#endif

#define I2S_AD_SET_RP I2S_AD32_SET_RP
#define I2S_AD_GET_RP I2S_AD32_GET_RP
#define I2S_AD_GET_WP I2S_AD32_GET_WP


MSFilter *ms_castor3snd_read_new(MSSndCard *card);
MSFilter *ms_castor3snd_write_new(MSSndCard *card);

static STRC_I2S_SPEC spec_da = {0};
static STRC_I2S_SPEC spec_ad = {0};

typedef struct Castor3SndData{
    char *pcmdev;
    char *mixdev;
    int   devid;

    int rate;
    int datalength;
    ms_thread_t txthread;
    ms_thread_t rxthread;
    ms_mutex_t ad_mutex;
	ms_mutex_t da_mutex;
    queue_t rq;
    MSBufferizer * bufferizer;
    bool_t read_started;
    bool_t write_started;
    bool_t write_EOF;
    bool_t stereo;
    
    STRC_I2S_SPEC spec;
    // DAC
    uint8_t *dac_buf;
    int dac_buf_len;
    int dac_vol;

    // ADC
    uint8_t *adc_buf;
    //uint32_t AECtxinitptr;
    int adc_buf_len;
    int adc_vol;
    uint32_t ts;
} Castor3SndData;


static void castor3snd_set_level(MSSndCard *card, MSSndCardMixerElem e, int percent)
{
    Castor3SndData *d=(Castor3SndData*)card->data;

    if (d->mixdev==NULL) return;
    switch(e){
        case MS_SND_CARD_MASTER:
            return;
        break;

        case MS_SND_CARD_PLAYBACK:
            //i2s_set_direct_volperc(percent);
			d->dac_vol = percent;
            return;
        break;

        case MS_SND_CARD_CAPTURE:
            //i2s_ADC_set_rec_volperc(percent);
            d->adc_vol = percent;
            return;
        break;

        default:
            ms_warning("castor3snd_card_set_level: unsupported command.");
            return;
    }
}

static int castor3snd_get_level(MSSndCard *card, MSSndCardMixerElem e)
{
    Castor3SndData *d=(Castor3SndData*)card->data;

    if (d->mixdev==NULL) return -1;
    switch(e){
        case MS_SND_CARD_MASTER:
            return 60;
        break;

        case MS_SND_CARD_PLAYBACK:
            return d->dac_vol;
        break;

        case MS_SND_CARD_CAPTURE:
            return d->adc_vol;
        break;

        default:
            ms_warning("castor3snd_card_get_level: unsupported command.");
            return -1;
    }
    return -1;
}

static void castor3snd_set_source(MSSndCard *card, MSSndCardCapture source)
{
    Castor3SndData *d=(Castor3SndData*)card->data;
    if (d->mixdev==NULL) return;

    switch(source){
        case MS_SND_CARD_MIC:
        break;
        case MS_SND_CARD_LINE:
        break;
    }
}

static void castor3snd_init(MSSndCard *card)
{
    Castor3SndData *d=(Castor3SndData*)ms_new0(Castor3SndData,1);

    d->pcmdev=NULL;
    d->mixdev=NULL;
    d->read_started=FALSE;
    d->write_started=FALSE;
    d->rate=CFG_AUDIO_SAMPLING_RATE;
    d->stereo=FALSE;
    d->datalength = -1;
    d->write_EOF = FALSE;
    qinit(&d->rq);
    d->bufferizer=ms_bufferizer_new();
    ms_mutex_init(&d->ad_mutex,NULL);
	ms_mutex_init(&d->da_mutex,NULL);
    d->ts=0;
    card->data=d;

#ifndef AEC_RUN_IN_ARM
    iteAudioOpenEngine(ITE_SBC_CODEC);
#endif

#ifdef CFG_RISC2_OPUS_CODEC
    int risc2EngineType = RISC2_OPUS_CODEC;
    ioctl(ITP_DEVICE_RISC2, ITP_IOCTL_RISC2_SWITCH_ENG, &risc2EngineType);
    ioctl(ITP_DEVICE_RISC2, ITP_IOCTL_INIT, NULL);
#endif

    /* init DAC */
    d->dac_buf_len = 128 * 1024;
    d->dac_buf = (uint8_t*)malloc(d->dac_buf_len);
    memset(d->dac_buf, 0, d->dac_buf_len);
    assert(d->dac_buf);
    memset((void*)&spec_da, 0, sizeof(STRC_I2S_SPEC));
    spec_da.channels                 = d->stereo? 2: 1;
    spec_da.sample_rate              = d->rate;
    spec_da.buffer_size              = d->dac_buf_len;
	spec_da.is_big_endian            = 0;
    spec_da.base_i2s                 = d->dac_buf;

    spec_da.sample_size              = 16;
    spec_da.num_hdmi_audio_buffer    = 1;
    spec_da.is_dac_spdif_same_buffer = 1;

    spec_da.base_hdmi[0]             = d->dac_buf;
    spec_da.base_hdmi[1]             = d->dac_buf;
    spec_da.base_hdmi[2]             = d->dac_buf;
    spec_da.base_hdmi[3]             = d->dac_buf;
    spec_da.base_spdif               = d->dac_buf;

    spec_da.enable_Speaker           = 1;
    spec_da.enable_HeadPhone         = 1;
    spec_da.postpone_audio_output    = 1;

    i2s_init_DAC(&spec_da);
    
    
    /* init ADC */
    d->adc_buf_len = 64 * 1024;
    d->adc_buf = (uint8_t*)malloc(d->adc_buf_len);
    memset(d->adc_buf, 0, d->adc_buf_len);
    assert(d->adc_buf);

    memset((void*)&spec_ad, 0, sizeof(STRC_I2S_SPEC));
    /* ADC Spec */    
//#if defined(ENABLE_DUAL_MIC_ICA_SEPARATION) || defined(ENABLE_DMNR_HAND_HELD_SEPARATION)
//    spec_ad.channels      = 2;
//#else
    spec_ad.channels      = 1;
//#endif
    spec_ad.sample_rate   = d->rate;
    spec_ad.buffer_size   = d->adc_buf_len;

	spec_ad.is_big_endian = 0;
    spec_ad.base_i2s      = d->adc_buf;

    spec_ad.sample_size   = 16;
    spec_ad.record_mode   = 1;
    
    spec_ad.from_MIC_IN   = 1;


    i2s_init_ADC(&spec_ad);
    i2s_pause_ADC(1);
    
}

static void castor3snd_uninit(MSSndCard *card)
{
    Castor3SndData *d=(Castor3SndData*)card->data;
    
    if (d==NULL)
        return;
    if (d->pcmdev!=NULL) ms_free(d->pcmdev);
    if (d->mixdev!=NULL) ms_free(d->mixdev);
    ms_bufferizer_destroy(d->bufferizer);
    flushq(&d->rq,0);

    ms_mutex_destroy(&d->ad_mutex);
	ms_mutex_destroy(&d->da_mutex);

    /* hook for Castor3 I2S */
    if (d->adc_buf != NULL) {
        free(d->adc_buf);
        d->adc_buf = NULL;
    }
    d->adc_buf_len = 0;

    if (d->dac_buf != NULL) {
        free(d->dac_buf);
        d->dac_buf = NULL;
    }
    d->dac_buf_len = 0;

    ms_free(d);
}

static void castor3snd_detect(MSSndCardManager *m);
static MSSndCard *castor3snd_dup(MSSndCard *obj);

MSSndCardDesc castor3snd_card_desc={
    "CASTOR3SND",
    castor3snd_detect,
    castor3snd_init,
    castor3snd_set_level,
    castor3snd_get_level,
    castor3snd_set_source,
    NULL,
    NULL,
    ms_castor3snd_read_new,
    ms_castor3snd_write_new,
    NULL,
    castor3snd_uninit,
    castor3snd_dup
};

static  MSSndCard *castor3snd_dup(MSSndCard *obj){
    MSSndCard *card=ms_snd_card_new(&castor3snd_card_desc);
    Castor3SndData *dcard=(Castor3SndData*)card->data;
    Castor3SndData *dobj=(Castor3SndData*)obj->data;
    dcard->pcmdev=ms_strdup(dobj->pcmdev);
    dcard->mixdev=ms_strdup(dobj->mixdev);
    dcard->devid=dobj->devid;
    card->name=ms_strdup(obj->name);
    //printf("CAUTION: called castor3snd_dup\n");
    return card;
}

static MSSndCard *castor3snd_card_new(const char *pcmdev, const char *mixdev, int id){
    MSSndCard *card=ms_snd_card_new(&castor3snd_card_desc);
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->pcmdev=ms_strdup(pcmdev);
    d->mixdev=ms_strdup(mixdev);
    card->name=ms_strdup(pcmdev);
    d->devid=id;    
    return card;
}

// this function will be called while booting
static void castor3snd_detect(MSSndCardManager *m){
    char pcmdev[1024];
    char mixdev[1024];
    MSSndCard *card;
    snprintf(pcmdev,sizeof(pcmdev),"%s","default");
    snprintf(mixdev,sizeof(mixdev),"%s","default");
    card=castor3snd_card_new(pcmdev,mixdev, 1);
    ms_snd_card_manager_add_card(m,card);    
    //printf("CAUTION: called castor3snd_detect\n");
}

static void *castor3snd_txthread(void *p)
{
    MSSndCard *card=(MSSndCard*)p;
    Castor3SndData *d=(Castor3SndData*)card->data;
    int bsize;
    uint8_t *wtmpbuff=NULL;
    int totaldata = 0;
    int DAcount = 0;
    uint32_t DA_r_pre;
    uint32_t waitcount = 3 ;
    bool the_first_write = TRUE;
    d->rate = spec_da.sample_rate;

    bsize=(320*d->rate*spec_da.channels)/8000;// 8K call & wav play

    //i2s_pause_DAC(0);
    I2S_DA32_SET_WP(I2S_DA32_GET_RP());
    DA_r_pre = I2S_DA32_GET_RP();
    i2s_set_direct_volperc(d->dac_vol);

    wtmpbuff=(uint8_t*)malloc(bsize);

    while (d->write_started)
    {
        uint32_t DA_r, DA_w, DA_free;
        
        while (d->write_started)
        {
            DA_r = I2S_DA32_GET_RP();
            DA_w = I2S_DA32_GET_WP();            
            DA_free = (DA_w >= DA_r) ? ((d->dac_buf_len - DA_w) + DA_r): (DA_r - DA_w);

            if (!d->read_started && DA_free < (d->dac_buf_len/4)){//workaround 
                usleep(10000);//when DA_free almost full add sleep time avoid audio miss
                continue;
            }
            
            if (DA_free >= (uint32_t)bsize)
                break;

            printf("i2s full, Never be happened\n");
            I2S_DA32_SET_WP(I2S_DA32_GET_RP());  
        }

        // printf("%d\n",ms_bufferizer_get_avail(d->bufferizer));
        if(ms_bufferizer_get_avail(d->bufferizer)<bsize)
        {
            if (d->read_started)
            {
                usleep(1000); // for calling case
            }
            else
            {
                usleep(10000); // for playing case, wait for data readed from file
            }
            continue;
        }
        ms_mutex_lock(&d->da_mutex);
        ms_bufferizer_read(d->bufferizer,wtmpbuff,bsize);
        ms_mutex_unlock(&d->da_mutex);
        if (d->write_started == FALSE)
            break;

        //ms_mutex_lock(&d->da_mutex);

        //printf("bsize=%d dac_buf_len=%d DA_r=%d DA_w=%d DA_free=%d\n", bsize, d->dac_buf_len, DA_r, DA_w, DA_free);

        /* write to sound device! */
        if ((DA_w + bsize) > (uint32_t)d->dac_buf_len)
        {
            int szbuf = d->dac_buf_len - DA_w;

            if (szbuf > 0)
            {
                memcpy(d->dac_buf + DA_w, wtmpbuff, szbuf);
                ithFlushDCacheRange((void*)(d->dac_buf+DA_w), szbuf);
            }
            DA_w = bsize - szbuf;
            memcpy(d->dac_buf, wtmpbuff + szbuf, DA_w);
            ithFlushDCacheRange((void*)(d->dac_buf), DA_w);
        }
        else
        {
            memcpy(d->dac_buf + DA_w, wtmpbuff, bsize);
            ithFlushDCacheRange((void*)(d->dac_buf+DA_w), bsize);
            DA_w += bsize;
        }
        
        if (DA_w == d->dac_buf_len)
            DA_w = 0;

        I2S_DA32_SET_WP(DA_w);

        if(d->datalength != -1){
            int shift;
            DA_r = I2S_DA32_GET_RP();
            shift = (DA_r >= DA_r_pre) ? (DA_r-DA_r_pre): (DA_r + d->dac_buf_len-DA_r_pre);
            totaldata += shift; 
            DA_r_pre = DA_r;
            if(totaldata >= d->datalength ){//play EOF 
                d->datalength = -1;
                d->write_EOF = TRUE;
            }
        }
        
        if(DAcount < d->dac_buf_len){//when DAC buffer full around we clear dac_buf prevent abnormal sound (bee sound)
            DAcount += bsize;
        }else{
            DA_r = I2S_DA32_GET_RP();
            DA_w = I2S_DA32_GET_WP();
            if(DA_w >= DA_r){
                memset(d->dac_buf,0,DA_r);
                memset(d->dac_buf+DA_w,0,d->dac_buf_len-DA_w);
            }else{
                memset(d->dac_buf+DA_w,0,DA_r-DA_w);
            }
            DAcount = 0;
        }
        
        if (waitcount>0){
            waitcount--;
            if(waitcount==0) i2s_pause_DAC(0);
        }else{    
            if (d->read_started){
                // for calling case, do nothing because of too much sleep makes more delays
            }else{
                // for playing case, add sleep time here to avoid filling too much data into i2s dac buffer shortly
                // sleep 20000 will get broken sound
                usleep(10000);
            }
        }
        //ms_mutex_unlock(&d->da_mutex);
    }

    /* close sound card */
    //ms_error("Shutting down sound device (input-output: %i) (notplayed: %i)", d->stat_input - d->stat_output, d->stat_notplayed);   
    free(wtmpbuff);
    i2s_pause_DAC(1);


    return NULL;
}

static void *castor3snd_rxthread(void *p)
{
    MSSndCard *card = (MSSndCard*)p;
    Castor3SndData *d = (Castor3SndData*)card->data;
	uint32_t AD_r,AD_w;
    int discardN = 1 ;
    I2S_AD_SET_RP(I2S_AD_GET_WP());

    i2s_pause_ADC(0);
    i2s_ADC_set_rec_volperc(d->adc_vol);

    while (d->read_started)
    {
        mblk_t *rm = NULL;
        uint32_t bsize = 0;
        uint32_t sleep_time = 10000;

		AD_r = I2S_AD_GET_RP();
        AD_w = I2S_AD_GET_WP();        

        if (AD_r <= AD_w)
        {
            bsize = AD_w - AD_r;
            if (bsize)
            {
                //printf("AD_r %u, AD_w %u bsize %u\n", AD_r, AD_w, bsize);
                rm = allocb(bsize, 0);
                ithInvalidateDCacheRange(d->adc_buf + AD_r, bsize);
                memcpy(rm->b_wptr, d->adc_buf + AD_r, bsize);
                rm->b_wptr += bsize;
                AD_r += bsize;
				I2S_AD_SET_RP(AD_r);
            }
        }
        else
        { // AD_r > AD_w
            bsize = (d->adc_buf_len - AD_r) + AD_w;
            if (bsize)
            {
                //printf("AD_r %u, AD_w %u bsize %u adc_buf_len %u\n", AD_r, AD_w, bsize, d->adc_buf_len);
                uint32_t szsec0 = d->adc_buf_len - AD_r;
                uint32_t szsec1 = bsize - szsec0;
                rm = allocb(bsize, 0);
                if (szsec0)
                {
                    ithInvalidateDCacheRange(d->adc_buf + AD_r, szsec0);
                    memcpy(rm->b_wptr, d->adc_buf + AD_r, szsec0);
                }
                ithInvalidateDCacheRange(d->adc_buf, szsec1);
                memcpy(rm->b_wptr + szsec0, d->adc_buf, szsec1);
                rm->b_wptr += bsize;
                AD_r = szsec1;

				I2S_AD_SET_RP(AD_r);

            }
        }
        if (rm)
        {
            if(discardN>0){//set 0 for init frame(discard impluse data)
                memset(rm->b_rptr,0,bsize);
                discardN--;
            }
            if (d->read_started == FALSE)
            {
                freeb(rm);
                break;
            }
            ms_mutex_lock(&d->ad_mutex);
            putq(&d->rq, rm);
            ms_mutex_unlock(&d->ad_mutex);
            //d->stat_input++;
            rm = NULL;
        }

        usleep(sleep_time);    
    }

    i2s_pause_ADC(1);

    return NULL;
}

static void castor3snd_start_r(MSSndCard *card){
    Castor3SndData *d=(Castor3SndData*)card->data;
    if (d->read_started==FALSE){
        pthread_attr_t attr;
        //struct sched_param param;
        d->read_started=TRUE;
        pthread_attr_init(&attr);
        //param.sched_priority = sched_get_priority_min(0) + 1;
        //pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&d->rxthread, &attr, castor3snd_rxthread, card);
    }
}

static void castor3snd_stop_r(MSSndCard *card){
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->read_started=FALSE;
    ms_thread_join(d->rxthread,NULL);
}

static void castor3snd_start_w(MSSndCard *card){
    Castor3SndData *d=(Castor3SndData*)card->data;
    if (d->write_started==FALSE){
        pthread_attr_t attr;
        //struct sched_param param;
        d->write_started=TRUE;
        pthread_attr_init(&attr);
        //param.sched_priority = sched_get_priority_min(0) + 1;
        //pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&d->txthread, &attr, castor3snd_txthread, card);
    }
}

static void castor3snd_stop_w(MSSndCard *card){
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->write_started=FALSE;
    ms_thread_join(d->txthread,NULL);
}

static mblk_t *castor3snd_get(MSSndCard *card){
    Castor3SndData *d=(Castor3SndData*)card->data;
    mblk_t *m;
    ms_mutex_lock(&d->ad_mutex);
    m=getq(&d->rq);
    ms_mutex_unlock(&d->ad_mutex);
    return m;
}

static void castor3snd_put(MSSndCard *card, mblk_t *m){
    Castor3SndData *d=(Castor3SndData*)card->data;
    ms_mutex_lock(&d->da_mutex);
    ms_bufferizer_put(d->bufferizer,m);
    ms_mutex_unlock(&d->da_mutex);
}

static void castor3snd_read_preprocess(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
//printf("### [%s] preprocess\n", f->desc->name);
    d->ts=0;
    castor3snd_start_r(card);
    //ms_ticker_set_time_func(f->ticker,castor3snd_get_cur_time,card->data);
}

static void castor3snd_read_postprocess(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
//printf("### [%s] postprocess\n", f->desc->name);
    //ms_ticker_set_time_func(f->ticker,NULL,NULL);
    castor3snd_stop_r(card);
    d->ts=0;
    memset(d->adc_buf, 0, d->adc_buf_len);
    flushq(&d->rq,0);
}

static void castor3snd_read_process(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
    mblk_t *m;
    int nbytes;
//printf("### [%s] process\n", f->desc->name);
    while((m=castor3snd_get(card))!=NULL){
        nbytes=(m->b_wptr-m->b_rptr)/2;
        mblk_set_timestamp_info(m,d->ts);
        d->ts+=nbytes;
        ms_queue_put(f->outputs[0],m);
    }
}

static void castor3snd_write_preprocess(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
//printf("### [%s] preprocess\n", f->desc->name);
    castor3snd_start_w(card);
}

static void castor3snd_write_postprocess(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
//printf("### [%s] postprocess\n", f->desc->name);
    castor3snd_stop_w(card);
    ms_mutex_lock(&d->da_mutex);
    ms_bufferizer_flush(d->bufferizer);
    memset(d->dac_buf, 0, d->dac_buf_len);
    ms_mutex_unlock(&d->da_mutex);
    d->datalength = -1;
    d->write_EOF = FALSE;
}

static void castor3snd_write_process(MSFilter *f){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
    mblk_t *m;
//printf("### [%s] process\n", f->desc->name);
    while((m=ms_queue_get(f->inputs[0]))!=NULL){
        castor3snd_put(card,m);
    }
    if(d->write_EOF){
        ms_filter_notify_no_arg(f,MS_FILE_PLAYER_EOF);
        d->write_EOF = FALSE ;//play EOF reset to FALSE;
    }
}

static int set_rate(MSFilter *f, void *arg){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->rate=*((int*)arg);
//printf("### [%s] set sample rate %d\n", f->desc->name, d->rate);
    return 0;
}

static int set_nchannels(MSFilter *f, void *arg){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->stereo=(*((int*)arg)==2);
//printf("### [%s] set channels %d\n", f->desc->name, *(int*)arg);
    return 0;
}

static int set_datalength(MSFilter *f, void *arg){
    MSSndCard *card=(MSSndCard*)f->data;
    Castor3SndData *d=(Castor3SndData*)card->data;
    d->datalength = *((int*)arg) ;
//    printf("### [%s] set datalength = %d\n", f->desc->name, d->datalength);
    return 0;
}

static int get_rate(MSFilter *f, void *arg)
{
    MSSndCard *card = (MSSndCard*)f->data;
    Castor3SndData *d = (Castor3SndData*)card->data;
    *(int*)arg = d->rate;
    return 0;
}

static int get_nchannels(MSFilter *f, void *arg)
{
    MSSndCard *card = (MSSndCard*)f->data;
    Castor3SndData *d = (Castor3SndData*)card->data;
    *(int*)arg = d->stereo? 2: 1;
    return 0;
}

static int set_useaec(MSFilter *f, void *arg){
    
    printf("Please disable config option [Echo Cancellation With Filter]\n");
    printf("NOTE:In this case echo cancellation is not working\n");
    return 0;
}

static MSFilterMethod castor3snd_methods[]={
    {    MS_FILTER_SET_SAMPLE_RATE  , set_rate      },
    {    MS_FILTER_SET_NCHANNELS    , set_nchannels },
    {    MS_FILTER_GET_SAMPLE_RATE  , get_rate      },
    {    MS_FILTER_GET_NCHANNELS    , get_nchannels },
    {    MS_FILTER_SET_DATALENGTH   , set_datalength},
    {    MS_FILTER_SET_USEAEC       , set_useaec    },
    {    0                          , NULL          }
};

MSFilterDesc castor3snd_read_desc={
    MS_CASTOR3SND_READ_ID,
    "MSCastor3SndRead",
    "Sound capture filter for Windows Sound drivers",
    MS_FILTER_OTHER,
    NULL,
    0,
    1,
    NULL,
    castor3snd_read_preprocess,
    castor3snd_read_process,
    castor3snd_read_postprocess,
    NULL,
    castor3snd_methods
};

MSFilterDesc castor3snd_write_desc={
    MS_CASTOR3SND_WRITE_ID,
    "MSCastor3SndWrite",
    "Sound playback filter for Castor3 Sound drivers",
    MS_FILTER_OTHER,
    NULL,
    1,
    0,
    NULL,
    castor3snd_write_preprocess,
    castor3snd_write_process,
    castor3snd_write_postprocess,
    NULL,
    castor3snd_methods
};

MSFilter *ms_castor3snd_read_new(MSSndCard *card){
    MSFilter *f=ms_filter_new_from_desc(&castor3snd_read_desc);
    f->data=card;
    return f;
}

MSFilter *ms_castor3snd_write_new(MSSndCard *card){
    MSFilter *f=ms_filter_new_from_desc(&castor3snd_write_desc);
    f->data=card;
    return f;
}

MS_FILTER_DESC_EXPORT(castor3snd_read_desc)
MS_FILTER_DESC_EXPORT(castor3snd_write_desc)

void castor3snd_deinit_for_video_memo_play(void)
{
    spec_da.sample_rate = 0;
    spec_da.sample_size = 0;        
    i2s_deinit_ADC();
    i2s_deinit_DAC();
}
void castor3snd_reinit_for_video_memo_play(void)
{
#ifndef AEC_RUN_IN_ARM
    iteAudioOpenEngine(ITE_SBC_CODEC);
#endif
    //i2s_init_DAC(&spec_da);
    //i2s_init_ADC(&spec_ad);
    //i2s_pause_ADC(1);
}

void Castor3snd_reinit_for_diff_rate(int rate,int bitsize,int channel)
{
    if (spec_da.sample_rate == rate && spec_da.sample_size == bitsize && spec_da.channels == channel && 
        i2s_get_DA_running() && i2s_get_AD_running())
        return ;

    i2s_deinit_ADC();
    i2s_deinit_DAC();
    spec_da.sample_rate = spec_ad.sample_rate = rate ;
    spec_da.sample_size = bitsize ;
    spec_da.channels = channel;
    //spec_ad.channels = channel;
    // channel bit etc. can be changed here;
    i2s_init_DAC(&spec_da);
    i2s_init_ADC(&spec_ad);
    i2s_pause_ADC(1);
    

}
