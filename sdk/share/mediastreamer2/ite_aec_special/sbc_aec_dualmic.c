#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"
#include "ortp/b64.h"

#ifdef HAVE_CONFIG_H
#include "mediastreamer-config.h"
#endif

#ifdef WIN32
#include <malloc.h> /* for alloca */
#endif

#include "ite/audio.h"

//#define EC_MEASUREMENTS

#ifdef ENABLE_DUMP_AEC_DATA
#define EC_DUMP_ITE
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

typedef struct _AudioFlowController_{
    int target_samples;
    int total_samples;
    int current_pos;
    int current_dropped;
}AudioFlowController_;
 
void audio_flow_controller_init_(AudioFlowController_ *ctl){
    ctl->target_samples=0;
    ctl->total_samples=0;
    ctl->current_pos=0;
    ctl->current_dropped=0;
}
 
void audio_flow_controller_set_target_(AudioFlowController_ *ctl, int samples_to_drop, int total_samples){
    ctl->target_samples=samples_to_drop;
    ctl->total_samples=total_samples;
    ctl->current_pos=0;
    ctl->current_dropped=0;
}
 
static void discard_well_choosed_samples_(mblk_t *m, int nsamples, int todrop){
    int i;
    int16_t *samples=(int16_t*)m->b_rptr;
    int min_diff=32768;
    int pos=0;
 
 
#ifdef TWO_SAMPLES_CRITERIA
    for(i=0;i<nsamples-1;++i){
        int tmp=abs((int)samples[i]- (int)samples[i+1]);
#else
    for(i=0;i<nsamples-2;++i){
        int tmp=abs((int)samples[i]- (int)samples[i+1])+abs((int)samples[i+1]- (int)samples[i+2]);
#endif
        if (tmp<=min_diff){
            pos=i;
            min_diff=tmp;
        }
    }
    /*ms_message("min_diff=%i at pos %i",min_diff, pos);*/
#ifdef TWO_SAMPLES_CRITERIA
    memmove(samples+pos,samples+pos+1,(nsamples-pos-1)*2);
#else
    memmove(samples+pos+1,samples+pos+2,(nsamples-pos-2)*2);
#endif
 
    todrop--;
    m->b_wptr-=2;
    nsamples--;
    if (todrop>0){
        /*repeat the same process again*/
        discard_well_choosed_samples_(m,nsamples,todrop);
    }
}
 
mblk_t * audio_flow_controller_process_(AudioFlowController_ *ctl, mblk_t *m){
    if (ctl->total_samples>0 && ctl->target_samples>0){
        int nsamples=(m->b_wptr-m->b_rptr)/2;
        if (ctl->target_samples*16>ctl->total_samples){
            ms_warning("Too many samples to drop, dropping entire frames");
            m->b_wptr=m->b_rptr;
            ctl->current_pos+=nsamples;
        }else{
            int th_dropped;
            int todrop;
 
            ctl->current_pos+=nsamples;
            th_dropped=(ctl->target_samples*ctl->current_pos)/ctl->total_samples;
            todrop=th_dropped-ctl->current_dropped;
            if (todrop>0){
                if (todrop>nsamples) todrop=nsamples;
                discard_well_choosed_samples_(m,nsamples,todrop);
                /*ms_message("th_dropped=%i, current_dropped=%i, %i samples dropped.",th_dropped,ctl->current_dropped,todrop);*/
                ctl->current_dropped+=todrop;
            }
        }
        if (ctl->current_pos>=ctl->total_samples) ctl->target_samples=0;/*stop discarding*/
    }
    return m;
}
 
static const int flow_control_interval_ms=5000;
 
typedef struct SbcAECState{
    MSFilter *msF;
    MSBufferizer delayed_ref;
    MSBufferizer ref;
    MSBufferizer echo1;
    MSBufferizer echo2;
    MSBufferizer hw_ref;
    MSBufferizer hw_echo1;
    MSBufferizer hw_echo2;
 
    bool_t hw_start;
    ms_thread_t hw_thread;	
    pthread_t ICApid;
    pthread_mutex_t mxq;
        //ms_mutex_t hw_mutex;
        
    int framesize;
    int samplerate;
    int delay_ms;
    int AECframesize;
    int nominal_ref_samples;
    int min_ref_samples;
    AudioFlowController_ afc;
    bool_t echostarted;
    bool_t bypass_mode;
    bool_t using_zeroes;

#ifdef EC_DUMP_ITE
    queue_t echo1_copy_q;
    queue_t echo2_copy_q;
    queue_t ref_copy_q;
    queue_t clean_copy_q;
#endif
    ms_mutex_t mutex;
}SbcAECState;


static void sbc_aec_init(MSFilter *f){
    SbcAECState *s=(SbcAECState *)ms_new(SbcAECState,1);
 
    s->samplerate=CFG_AUDIO_SAMPLING_RATE;
    ms_bufferizer_init(&s->delayed_ref);
    ms_bufferizer_init(&s->echo1);
    ms_bufferizer_init(&s->echo2);
    ms_bufferizer_init(&s->ref);
    ms_bufferizer_init(&s->hw_ref);
    ms_bufferizer_init(&s->hw_echo1);
    ms_bufferizer_init(&s->hw_echo2);

    s->using_zeroes=FALSE;
    s->echostarted=FALSE;
    s->bypass_mode=FALSE;
    s->hw_start=FALSE;
    
    s->delay_ms=CFG_AEC_DELAY_MS;

#ifdef EC_DUMP_ITE
    qinit(&s->echo1_copy_q);
    qinit(&s->echo2_copy_q);
    qinit(&s->ref_copy_q);
    qinit(&s->clean_copy_q);
#endif
    ms_mutex_init(&s->mutex,NULL);
 
    f->data=s;
    iteAecCommand(AEC_CMD_INIT, 0, 0, 0, 0, &s->AECframesize);//AECframesize=144
    s->AECframesize = 144;
    printf("RISC version\n");
    s->framesize = s->AECframesize; //s->framesize can be any size, equal to AECframesize more efficient  

}
 
static void sbc_aec_uninit(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    ms_bufferizer_uninit(&s->delayed_ref);
    ms_mutex_destroy(&s->mutex);
    ms_free(s);
}

static void *hw_engine_thread(void *arg) {
    
    SbcAECState *s=(SbcAECState*)arg;

    int hw_nbytes = s->AECframesize*2;
    int nbytes    = s->framesize*2;
#ifdef EC_MEASUREMENTS
	MSTimeSpec begin,end;
    uint32_t count=1;
    uint64_t diff=0;
#endif	 

    while(s->hw_start){
        mblk_t *ref,*echo1,*echo2;
        int err1,err2,err3;
#ifdef EC_MEASUREMENTS
        ms_get_cur_time(&begin); 
#endif    
        ref=allocb(hw_nbytes, 0);
        echo1=allocb(hw_nbytes, 0);
        echo2=allocb(hw_nbytes, 0);
        ms_mutex_lock(&s->mutex);
        err1 = ms_bufferizer_read(&s->hw_echo1,echo1->b_wptr,hw_nbytes);
        err3 = ms_bufferizer_read(&s->hw_echo2,echo2->b_wptr,hw_nbytes);
        err2 = ms_bufferizer_read(&s->hw_ref,ref->b_wptr,hw_nbytes);
        ms_mutex_unlock(&s->mutex);

        if (err1 && err2 && err3) {
            mblk_t *oecho = allocb(hw_nbytes, 0);
            memset(oecho->b_wptr, 0, hw_nbytes);
            echo1->b_wptr+=hw_nbytes;
            echo2->b_wptr+=hw_nbytes;
            ref->b_wptr+=hw_nbytes;

            iteAecCommand(AEC_CMD_PROCESS,(unsigned int) echo1->b_rptr,(unsigned int) ref->b_rptr, (unsigned int) oecho->b_wptr,hw_nbytes, 0);
            
            memcpy(oecho->b_wptr,echo1->b_rptr,hw_nbytes);

            oecho->b_wptr += hw_nbytes;
            ms_queue_put(s->msF->outputs[1],oecho);
#ifdef EC_DUMP_ITE
            putq(&s->clean_copy_q, dupmsg(oecho));
#endif
#ifdef EC_MEASUREMENTS
            ms_get_cur_time(&end);
            count++;
            diff += (end.tv_sec-begin.tv_sec)*1000 + (end.tv_nsec-begin.tv_nsec)/1000000;
            if(count % 100==0){
                printf("AEC 100 time %d\n",(int)diff);
                diff = 0;
            }
#endif  
        }else{
           usleep(20000);
        }
        if (echo1) freemsg(echo1);
        if (echo2) freemsg(echo2);
        if (ref) freemsg(ref);
        
        if(s->hw_start==FALSE){
            break;
        }
    }

    return NULL;
}

static void sbc_aec_start_hwthread(SbcAECState *e){
    //ms_mutex_init(&e->hw_mutex,NULL);
    if (e->hw_start == FALSE){
        pthread_attr_t attr;
        struct sched_param param;
        e->hw_start=TRUE;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 32*1024);       
        param.sched_priority = sched_get_priority_min(0) + 1;
        pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&e->hw_thread, &attr, hw_engine_thread, e);
    }
}

static void sbc_aec_stop_hwthread(SbcAECState *e){
    e->hw_start=FALSE;
    //ms_mutex_destroy(e->hw_mutex);
    ms_thread_join(e->hw_thread,NULL);      
}
 
static void sbc_aec_preprocess(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    mblk_t *m;
    int delay_samples=0;
    
    s->echostarted=FALSE;
    delay_samples=s->delay_ms*s->samplerate/1000;
 
    /* fill with zeroes for the time of the delay*/
    m=allocb(delay_samples*2,0);
    memset(m->b_wptr,0,delay_samples*2);
    m->b_wptr+=delay_samples*2;
    ms_bufferizer_put(&s->delayed_ref,m);
    s->min_ref_samples=-1;
    s->nominal_ref_samples=delay_samples;
    audio_flow_controller_init_(&s->afc);
    s->msF = f;
    sbc_ICA_start_thread(s);
    sbc_aec_start_hwthread(s);
    //hw_engine_init();
    //hw_engine_link_filter(HW_EC_ID, f);
}
 
/*  inputs[0]= reference signal from far end (sent to soundcard)
 *  inputs[1]= near speech & echo1 signal    (read from soundcard) mic1
 *  inputs[1]= near speech & echo1 signal    (read from soundcard) mic2
 *  outputs[0]=  is a copy of inputs[0] to be sent to soundcard
 *  outputs[1]=  near end speech, echo1 removed - towards far end
*/
static void sbc_aec_process(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    int nbytes=s->framesize*2;
    mblk_t *refm;
    uint8_t *ref,*echo1,*echo2;
    if (s->bypass_mode) {
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            ms_queue_put(f->outputs[0],refm);
        }
        while((refm=ms_queue_get(f->inputs[1]))!=NULL){
            ms_queue_put(f->outputs[1],refm);
        }
        return;
    }
    
      
    if (f->inputs[0]!=NULL){
        if (s->echostarted){
            while((refm=ms_queue_get(f->inputs[0]))!=NULL){
                mblk_t *cp=dupmsg(audio_flow_controller_process_(&s->afc,refm));
                ms_mutex_lock(&s->mutex);
                ms_bufferizer_put(&s->delayed_ref,cp);
                ms_bufferizer_put(&s->ref,refm);
                ms_mutex_unlock(&s->mutex);
            }
        }else{
            ms_warning("Getting reference signal but no echo1 to synchronize on.");
            ms_queue_flush(f->inputs[0]);
        }
    }
 
    ms_bufferizer_put_from_queue(&s->echo1,f->inputs[1]);
    ms_bufferizer_put_from_queue(&s->echo2,f->inputs[2]);
 
    ref=(uint8_t*)alloca(nbytes);
    echo1=(uint8_t*)alloca(nbytes);
    echo2=(uint8_t*)alloca(nbytes);
    while ((ms_bufferizer_read(&s->echo1,echo1,nbytes)>=nbytes) && 
           (ms_bufferizer_read(&s->echo2,echo2,nbytes)>=nbytes)    ){
        int avail;
        int avail_samples;
        mblk_t *clean;
        mblk_t *echo1_to_ec,*echo2_to_ec;
        mblk_t *ref_to_ec;
 
        if (!s->echostarted) s->echostarted=TRUE;
        if ((avail=ms_bufferizer_get_avail(&s->delayed_ref))<((s->nominal_ref_samples*2)+nbytes)){
            /*we don't have enough to read in a reference signal buffer, inject silence instead*/
            refm=allocb(nbytes,0);
            memset(refm->b_wptr,0,nbytes);
            refm->b_wptr+=nbytes;
            ms_mutex_lock(&s->mutex);
            ms_bufferizer_put(&s->delayed_ref,refm);
            ms_queue_put(f->outputs[0],dupmsg(refm));
            ms_mutex_unlock(&s->mutex);
            if (!s->using_zeroes){
                ms_warning("Not enough ref samples, using zeroes");
                s->using_zeroes=TRUE;
            }
        }else{
            if (s->using_zeroes){
                ms_message("Samples are back.");
                s->using_zeroes=FALSE;
            }
            /* read from our no-delay buffer and output */
            refm=allocb(nbytes,0);
            if (ms_bufferizer_read(&s->ref,refm->b_wptr,nbytes)==0){
                ms_fatal("Should never happen");
            }
            refm->b_wptr+=nbytes;
            ms_queue_put(f->outputs[0],refm);
        }
 
        /*now read a valid buffer of delayed ref samples*/
        if (ms_bufferizer_read(&s->delayed_ref,ref,nbytes)==0){
            ms_fatal("Should never happen");
        }
        avail-=nbytes;
        avail_samples=avail/2;
        if (avail_samples<s->min_ref_samples || s->min_ref_samples==-1){
            s->min_ref_samples=avail_samples;
        }
 
        // put near-end and far-end data to queue
        echo1_to_ec = allocb(nbytes, 0);        
        memcpy(echo1_to_ec->b_wptr, echo1, nbytes);
        echo1_to_ec->b_wptr += nbytes;
        echo2_to_ec = allocb(nbytes, 0);        
        memcpy(echo2_to_ec->b_wptr, echo2, nbytes);
        echo2_to_ec->b_wptr += nbytes;
        ref_to_ec = allocb(nbytes, 0);
        memcpy(ref_to_ec->b_wptr, ref, nbytes);
        ref_to_ec->b_wptr += nbytes;
        
        ms_mutex_lock(&s->mutex);

        ms_bufferizer_put(&s->hw_echo1,echo1_to_ec);//put to buffer
        ms_bufferizer_put(&s->hw_echo2,echo2_to_ec);//put to buffer
        ms_bufferizer_put(&s->hw_ref,ref_to_ec);  //put to buffer    
#ifdef EC_DUMP_ITE
        putq(&s->echo1_copy_q, dupmsg(echo1_to_ec));
        putq(&s->echo2_copy_q, dupmsg(echo2_to_ec));
        putq(&s->ref_copy_q, dupmsg(ref_to_ec));
#endif
        ms_mutex_unlock(&s->mutex);  
    }
 
    /*verify our ref buffer does not become too big, meaning that we are receiving more samples than we are sending*/
    if (f->ticker->time % flow_control_interval_ms == 0 && s->min_ref_samples!=-1){
        int diff=s->min_ref_samples-s->nominal_ref_samples;
        if (diff>(nbytes/1)){
            int purge=diff-(nbytes/1);
            ms_warning("echo1 canceller: we are accumulating too much reference signal, need to throw out %i samples",purge);
            audio_flow_controller_set_target_(&s->afc,purge,(flow_control_interval_ms*s->samplerate)/1000);
        }
        s->min_ref_samples=-1;
    }
}
 
static void sbc_aec_postprocess(MSFilter *f){
    SbcAECState *s=(SbcAECState*)f->data;
    
    sbc_aec_stop_hwthread(s);
    sbc_ICA_stop_thread(s);

    ms_bufferizer_flush (&s->delayed_ref);
    ms_bufferizer_flush (&s->echo1);
    ms_bufferizer_flush (&s->ref);
    ms_bufferizer_flush (&s->hw_ref);
    ms_bufferizer_flush (&s->hw_echo1);
    ms_bufferizer_flush (&s->hw_echo2);

#ifdef EC_DUMP_ITE
    FILE *echo1file;
    FILE *echo2file;
    FILE *reffile;
    FILE *cleanfile;
    mblk_t *echo1;
    mblk_t *ref;
    mblk_t *clean;
    int nbytes=s->framesize*2;
    static int index = 0;
    char *fname;
    char USBPATH = Get_Storage_path();
    if(USBPATH == NULL){
        flushq(&s->echo1_copy_q,0);
        flushq(&s->echo2_copy_q,0);
        flushq(&s->ref_copy_q,0);
        flushq(&s->clean_copy_q,0);
        printf("USB not insert \n");
        return ;
    }
    
    printf("save audio data in USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/echo1%03d.raw",USBPATH,index);
    echo1file=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/echo2%03d.raw",USBPATH,index);
    echo2file=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/ref%03d.raw",USBPATH,index);
    reffile=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/clean%03d.raw",USBPATH,index);
    cleanfile=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        echo1=ref=NULL;
        echo1=getq(&s->echo1_copy_q);
        echo2=getq(&s->echo2_copy_q);
        ref=getq(&s->ref_copy_q);
        clean=getq(&s->clean_copy_q);
        if (echo1 && ref && clean)
        {
            fwrite(echo1->b_rptr,nbytes,1,echo1file);
            freemsg(echo1);
            fwrite(echo2->b_rptr,nbytes,1,echo2file);
            freemsg(echo2);    
            fwrite(ref->b_rptr,nbytes,1,reffile);
            freemsg(ref);
            fwrite(clean->b_rptr,nbytes,1,cleanfile);
            freemsg(clean);
        }
        else
        {
            flushq(&s->echo1_copy_q,0);
            flushq(&s->echo2_copy_q,0);
            flushq(&s->ref_copy_q,0);
            flushq(&s->clean_copy_q,0);
            fclose(echo1file);
            fclose(echo2file);
            fclose(reffile);
            fclose(cleanfile);
            break;
        }
    }
#endif
}
 
static int echo_canceller_set_delay(MSFilter *f, void *arg){
	SbcAECState *s=(SbcAECState*)f->data;
	s->delay_ms= *(int*)arg;
    return 0;
}
 
static MSFilterMethod sbc_aec_methods[]={
    {MS_ECHO_CANCELLER_SET_DELAY        , echo_canceller_set_delay},
};
 
#ifdef _MSC_VER
 
MSFilterDesc ms_sbc_aec_desc={
    MS_SBC_AEC_ID,
    "MSSbcAEC",
    N_("echo canceller using sbc aec"),
    MS_FILTER_OTHER,
    NULL,
    2,
    2,
    sbc_aec_init,
    sbc_aec_preprocess,
    sbc_aec_process,
    sbc_aec_postprocess,
    sbc_aec_uninit,
    sbc_aec_methods
};
 
#else
 
MSFilterDesc ms_sbc_aec_desc={
    .id=MS_SBC_AEC_ID,
    .name="MSSbcAEC",
    .text=N_("echo canceller using sbc aec"),
    .category=MS_FILTER_OTHER,
    .ninputs=2,
    .noutputs=2,
    .init=sbc_aec_init,
    .preprocess=sbc_aec_preprocess,
    .process=sbc_aec_process,
    .postprocess=sbc_aec_postprocess,
    .uninit=sbc_aec_uninit,
    .methods=sbc_aec_methods
};
 
#endif
 
MS_FILTER_DESC_EXPORT(ms_sbc_aec_desc)
