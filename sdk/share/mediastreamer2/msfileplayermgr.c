#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msfileplayer.h"
#include "private.h"
#include "audio_mgr.h"
#include "ite/audio.h"

#if 0
#define DUMP_ITE
#include "ite/itp.h"
static char *Get_Storage_path(void)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = 0; i < ITP_MAX_DRIVE; i++)
    {
        driveStatus = &driveStatusTable[i];
		 if (driveStatus->disk >= ITP_DISK_MSC00 && driveStatus->disk <= ITP_DISK_MSC17)
        {
            if (driveStatus->avail )
            {
                printf("USB #%d inserted: %s\n", driveStatus->disk - ITP_DISK_MSC00, driveStatus->name[0]);
                return driveStatus->name[0];
            }
        }
    }
    return NULL;
} 
#endif

#define LOCAL_PLAYER_BUFFER_LEN  (64 * 1024)

static int audiomgr_close(MSFilter *f, void *arg);

typedef struct _AudiomgrData{
    void *fd;
    SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    MSPlayerState state;
    MSBufferizer buff;
    int rate;
    int bitsize;
    int nchannels;
    int loop_after;
    int playtime;
    int special_case;
    int nbytes;
    uint8_t *dac_buf;
    int dac_len;
    int RP;
    int WP;
    bool_t swap;
#ifdef DUMP_ITE
    queue_t out_q;
#endif

}AudiomgrData;

static int audiomgrCodecType(char* filename)
{
    char*                    ext;

    ext = strrchr(filename, '.');
    if (!ext)
    {
        printf("Invalid file name: %s\n", filename);
        return -1;
    }
    ext++;

    if (stricmp(ext, "wav") == 0)
    {
        return SMTK_AUDIO_WAV;     
    }
    else if (stricmp(ext, "mp3") == 0)
    {
        return SMTK_AUDIO_MP3;
    }
    else if (stricmp(ext, "wma") == 0)
    {
        return SMTK_AUDIO_WMA;
    }
    else if (stricmp(ext, "aac") == 0)
    {
        return SMTK_AUDIO_AAC;
    }
    else
    {
        printf("Unsupport file format: %s\n", ext);
        return -1;
    }
    return -1;
}

int audioCallbackFucntion(int nState)
{
    switch (nState)
    {
        case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
        printf("file play EOF");
        break;

        case AUDIOMGR_STATE_CALLBACK_GET_FINISH_NAME:
        printf("file : %s\n",smtkAudioMgrGetFinishName());
        break;
        
        default:
        break;
        
    }
}

static void audiomgr_init(MSFilter *f){
    AudiomgrData *d=ms_new(AudiomgrData,1);
    d->fd=NULL;
    d->state=MSPlayerClosed;
    d->swap=FALSE;
    d->rate=8000;
    d->bitsize = 16;
    d->nchannels=1;
    d->loop_after=-1; /*by default, don't loop*/
    d->playtime=0;
    d->special_case=0;
    d->nbytes = 320*2;
    ms_bufferizer_init(&d->buff);
    d->RP=0;
    d->WP=0;
#ifdef DUMP_ITE
    qinit(&d->out_q);
#endif
      
    f->data=d;
}


static int audiomgr_open(MSFilter *f, void *arg){
    AudiomgrData *d=(AudiomgrData*)f->data;
    const char *file=(const char*)arg;
    int pause = 1;
    int tmp=0, cnt=0;
    //SMTK_AUDIO_PARAM_NETWORK audiomgr_local;
    if(d->fd) {
        fclose(d->fd);
        d->fd=NULL;
    }
    
    d->fd = fopen(file, "rb");
    d->audiomgr_local.LocalRead   = fread;
    d->audiomgr_local.nReadLength = LOCAL_PLAYER_BUFFER_LEN;    
    d->audiomgr_local.bSeek       = 0;
    d->audiomgr_local.nM4A        = 0;
    d->audiomgr_local.bLocalPlay  = 1;
    d->audiomgr_local.pHandle   = d->fd;
    d->audiomgr_local.pFilename = file;
    d->audiomgr_local.audioMgrCallback = NULL;
    d->audiomgr_local.nType = audiomgrCodecType(file);
    //iteAudioSetAttrib(ITE_AUDIO_IS_PAUSE,&pause);
    iteAudioSetAttrib(ITE_AUDIO_I2S_INIT,&tmp);
    //printf("RP=%d WP=%d\n",I2S_DA32_GET_RP(),I2S_DA32_GET_WP());
    smtkAudioMgrPlayNetwork(&d->audiomgr_local);
    iteAudioSetMusicCodecDump(1);
    while(tmp==0){
        //audio_buffer_read_data();
        usleep(10000);
        iteAudioGetAttrib(ITE_AUDIO_I2S_INIT, &tmp);
        if(cnt++>100) return 0;
    }
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_CHANNEL,        &d->nchannels);
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_SAMPLE_RATE,    &d->rate);
    iteAudioGetAttrib(  ITE_AUDIO_I2S_PTR,                  &d->dac_buf);
    iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &d->dac_len);
  
    Castor3snd_reinit_for_diff_rate(d->rate,16,d->nchannels);
    //pause = 0;
    //iteAudioSetAttrib(ITE_AUDIO_IS_PAUSE,&pause);
    //i2s_pause_DAC(0);
    //iteAudioGetAttrib(  ITE_AUDIO_CODEC_SET_BUFFER_LENGTH,  &nI2SBufferLength);

    return 0;
}

static int audiomgr_start(MSFilter *f, void *arg){
    AudiomgrData *d=(AudiomgrData*)f->data;
    if (d->state==MSPlayerPaused)
        d->state = MSPlayerPlaying;
    else{
        d->state = MSdummyPlaying;
        printf("MSdummyPlaying scilent\n");
    }
    return 0;
}

static int audiomgr_stop(MSFilter *f, void *arg){
    AudiomgrData *d=(AudiomgrData*)f->data;
    
    return 0;
}

static int audiomgr_close(MSFilter *f, void *arg){
    AudiomgrData *d=(AudiomgrData*)f->data;
    d->state=MSPlayerClosed; 
    if (d->fd)
    {
#ifdef __OPENRTOS__
        smtkAudioMgrQuickStop();
#endif
        printf("%s %d\n",__FUNCTION__,__LINE__);
        fclose(d->fd);
        d->fd=NULL;
        castor3snd_reinit_for_video_memo_play();
    }
    return 0;
}

static void swap_bytes(unsigned char *bytes, int len){
    int i;
    unsigned char tmp;
    for(i=0;i<len;i+=2){
        tmp=bytes[i];
        bytes[i]=bytes[i+1];
        bytes[i+1]=tmp;
    }
}

static int audiomgr_loop(MSFilter *f, void *arg){
    AudiomgrData *d=(AudiomgrData*)f->data;
    d->loop_after=*((int*)arg);
    if(d->loop_after==-1)
        smtkAudioMgrSetMode(SMTK_AUDIO_NORMAL);
    else
        smtkAudioMgrSetMode(SMTK_AUDIO_REPEAT);  
    return 0;
}

static mblk_t *audiomgr_get_data(AudiomgrData *d){
    
        mblk_t *rm = NULL;
        uint32_t bsize = 0;
        uint32_t AD_r;
        uint32_t AD_w;
        uint8_t *dac_buf=d->dac_buf;
        int dac_len=d->dac_len;
        AD_r=d->RP;
        AD_w=d->WP;
        if (AD_r <= AD_w)
        {
            bsize = AD_w - AD_r;
            if (bsize)
            {
                rm = allocb(bsize, 0);
                ithInvalidateDCacheRange(dac_buf + AD_r, bsize);
                memcpy(rm->b_wptr, dac_buf + AD_r, bsize);
                rm->b_wptr += bsize;
                AD_r += bsize;
            }
        }
        else
        { // AD_r > AD_w
            bsize = (dac_len - AD_r) + AD_w;
            if (bsize)
            {
                
                uint32_t szsec0 = dac_len - AD_r;
                uint32_t szsec1 = bsize - szsec0;
                rm = allocb(bsize, 0);
                if (szsec0)
                {
                    ithInvalidateDCacheRange(dac_buf + AD_r, szsec0);
                    memcpy(rm->b_wptr, dac_buf + AD_r, szsec0);
                }
                ithInvalidateDCacheRange(dac_buf, szsec1);
                memcpy(rm->b_wptr + szsec0, dac_buf, szsec1);
                rm->b_wptr += bsize;
                AD_r = szsec1;
            }
        }
    return rm;
}

static void audiomgr_preprocess(MSFilter *f){
    AudiomgrData *d=(AudiomgrData*)f->data;
    d->state = MSPlayerPlaying;
}

static void audiomgr_process(MSFilter *f){
    AudiomgrData *d=(AudiomgrData*)f->data;
    //printf("audiomgr_process %d\n",d->fd);
    ms_filter_lock(f);
    //audio_buffer_read_data();
    if(d->state==MSPlayerPlaying){
        //usleep(50000);
        //audio_buffer_read_data();
        //printf("index=%d\n",smtkAudioMgrGetPcmIdx());
        
        d->WP=smtkAudioMgrGetPcmIdx();
        if(d->RP!=d->WP){
            //mblk_t *out = audiomgr_get_data(d->RP,d->WP);
            int nbytes=d->nbytes;
            //ms_queue_put(f->outputs[0],audiomgr_get_data(d));
            
            #if 1
            ms_bufferizer_put(&d->buff,audiomgr_get_data(d));
            while(ms_bufferizer_get_avail(&d->buff)>=nbytes){
                mblk_t *out;
                out=allocb(nbytes,0);
                ms_bufferizer_read(&d->buff,out->b_wptr,nbytes);
                out->b_wptr+=nbytes;
                ms_queue_put(f->outputs[0],out);
                #ifdef DUMP_ITE
                putq(&d->out_q, out);
                #endif
            }
            #endif
            //I2S_DA32_SET_WP(smtkAudioMgrGetPcmIdx());
            d->RP=d->WP;
        }
    }
    if(d->state==MSPlayerEof){
        sleep(2);
        d->state=MSPlayerPlaying;
    }
    if(d->state == MSdummyPlaying){

    }

    ms_filter_unlock(f);

}

static void audiomgr_postprocess(MSFilter *f){
    AudiomgrData *d=(AudiomgrData*)f->data;
#ifdef DUMP_ITE
    FILE *outfile;
    mblk_t *out;
    int nbytes=d->nbytes;
    static int index = 0;
    char *fname;
    char USBPATH = 'E';//Get_Storage_path();
    if(USBPATH == NULL){
        flushq(&d->out_q,0);
        printf("USB not insert \n");
        return ;
    }    
    printf("save audio data in USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/out%03d.raw",USBPATH,index);
    outfile=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        out=NULL;
        out=getq(&d->out_q);
        if (out)
        {
            fwrite(out->b_rptr,nbytes,1,outfile);
            freemsg(out);            
        }
        else
        {
            flushq(&d->out_q,0);
            fclose(outfile);
            break;
        }
    }
#endif
    
    printf("%s %d\n",__FUNCTION__,__LINE__);
}

static void audiomgr_uninit(MSFilter *f){
    AudiomgrData *d=(AudiomgrData*)f->data;
    audiomgr_close(f,NULL);
    ms_bufferizer_uninit(&d->buff);
    iteAudioSetMusicCodecDump(0);
    printf("%s %d\n",__FUNCTION__,__LINE__);
    ms_free(d);
}

static int get_rate(MSFilter *f, void *arg)
{
    AudiomgrData *d=(AudiomgrData*)f->data;
    *(int*)arg = d->rate;
    return 0;
}

static int get_nchannels(MSFilter *f, void *arg)
{
    AudiomgrData *d=(AudiomgrData*)f->data;
    *(int*)arg = d->nchannels;
    return 0;
}

static MSFilterMethod audiomgr_methods[]={
    {   MS_AUDIOMGR_OPEN,    audiomgr_open },
    {   MS_AUDIOMGR_START,   audiomgr_start    },
    {   MS_AUDIOMGR_STOP,    audiomgr_stop },
    {   MS_AUDIOMGR_CLOSE,   audiomgr_close    },
    {   MS_AUDIOMGR_SET_LOOP,    audiomgr_loop },
    {   MS_FILTER_GET_SAMPLE_RATE,    get_rate },
    {   MS_FILTER_GET_NCHANNELS,    get_nchannels },
    {   0,          NULL        }
};

#ifdef WIN32

MSFilterDesc ms_file_audiomgr_desc={
    MS_FILE_AUDIOMGR_ID,
    "MSFileAudioMgr",
    N_("Raw files and wav reader"),
    MS_FILTER_OTHER,
    NULL,
    0,
    1,
    audiomgr_init,
    audiomgr_preprocess,
    audiomgr_process,
    audiomgr_postprocess,
    audiomgr_uninit,
    audiomgr_methods
};

#else

MSFilterDesc ms_file_audiomgr_desc={
    .id=MS_FILE_AUDIOMGR_ID,
    .name="MSFileAudioMgr",
    .text=N_("Raw files and wav reader"),
    .category=MS_FILTER_OTHER,
    .ninputs=0,
    .noutputs=1,
    .init=audiomgr_init,
    .preprocess=audiomgr_preprocess,
    .process=audiomgr_process,
    .postprocess=audiomgr_postprocess,
    .uninit=audiomgr_uninit,
    .methods=audiomgr_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_file_audiomgr_desc)
