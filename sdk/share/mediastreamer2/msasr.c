#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msasr.h"
static int asr_reset(MSFilter *f, void *arg);
#define MEASUREMENTS

typedef struct ASRstate{
    MSFilter *msF;
    MSBufferizer *Buf;
    int framesize;
    int samplerate;
    float vad_threshold;
    int vad_dur;
    float vad_cuttime;
    //int input_channel;
    //int output_channel;   
    bool_t threadstart;
    bool_t asrStart;
    bool_t bypass;
    ms_thread_t asr_thread;	   
    ms_mutex_t mutex;
    
}ASRstate;

typedef struct _asrStruct{
    int index;
    int rs;
    char* text;
    float score;
}asrStruct;

static inline int16_t saturate(int val) {
	return (val>32767) ? 32767 : ( (val<-32767) ? -32767 : val);
}

static int vad_detect(ASRstate *v , mblk_t *om){
	int result=1;
	int i;
	float acc = 0;
	float energy;
    int16_t *sample;
    static const float max_e = (32768* (1-0.3));
    int nsamples=(om->b_wptr-om->b_rptr)/2;
    //printf("nsamples = %d\n ",nsamples);
    
    for (sample=(int16_t*)om->b_rptr;sample<(int16_t*)om->b_wptr;++sample){
        acc += (*sample) * (*sample);
    }
    energy = (sqrt(acc / nsamples)+1) / max_e;
    
	if (energy > v->vad_threshold) {
        v->vad_dur = v->vad_cuttime;
	}
	else {
		if (v->vad_dur > 0) {
			v->vad_dur -= (nsamples * 1000) / v->samplerate;
		}else{        
            result=0;
        }
	}

    return result;

}

static pre_emphasise(mblk_t *m){
    int16_t *sample;
    static int16_t tmp[2] ={0};
	for (sample=(int16_t*)m->b_rptr;sample<(int16_t*)m->b_wptr;++sample){
        tmp[1] = tmp[0];
        tmp[0] = *sample;
        *sample = saturate(tmp[0] - (7*tmp[1])>>3);
    }
}

static void asr_init(MSFilter *f){
    ASRstate *s=(ASRstate *)ms_new(ASRstate,1);
    f->data=s;
    s->Buf =ms_bufferizer_new();
    s->framesize = 480;
    s->samplerate = 16000;
    s->vad_dur     = 0;
    s->vad_cuttime = 400;
    s->vad_threshold = 0.025;
    ms_mutex_init(&s->mutex,NULL);
    s->threadstart = FALSE;
    s->asrStart = TRUE;
    s->bypass = FALSE;
    printf("asr_init\n");   
}


static void *asr_thread(void *arg) {
    ASRstate *s=(ASRstate*)arg;
#ifdef MEASUREMENTS
	MSTimeSpec begin,end;
    uint32_t count=1;
    uint64_t diff=0;
#endif	
    int nbytes    = s->framesize;
    float score;
    int rs;
    const char *text;
    uint16_t PcmBuf1[480] = {0};
    //uint8_t *ref;
    while(s->threadstart){
        if(s->asrStart){
            int err1;
            ms_mutex_lock(&s->mutex);
            err1 = ms_bufferizer_read(s->Buf,PcmBuf1,nbytes*2);
            ms_mutex_unlock(&s->mutex);
            //while(ms_bufferizer_read(s->Buf,PcmBuf1,nbytes)){
            if(err1){
                #ifdef MEASUREMENTS
                ms_get_cur_time(&begin);
                #endif    
                rs = Wanson_ASR_Recog((short*)PcmBuf1, nbytes, &text, &score); 
                #ifdef MEASUREMENTS
                ms_get_cur_time(&end);
                count++;
                diff += (end.tv_sec-begin.tv_sec)*1000 + (end.tv_nsec-begin.tv_nsec)/1000000;
                if(count % 200==0){
                    printf("ASR 200 time %d bufsize=%d\n",(int)diff,ms_bufferizer_get_avail(s->Buf));
                    diff = 0;
                }                
                #endif
                if (rs == 1) {
                    asrStruct data;
                    data.rs=rs;
                    data.text=text;
                    data.score=score;
                    //ms_filter_notify_no_arg(s->msF,MS_ASR_SUCCESS);
                    ms_filter_notify(s->msF, MS_ASR_SUCCESS_ARG, (void*)&data);
                    //asr_reset(s->msF,NULL);
                    printf("ASR Result: %s\n", text);
                }
            }else{
                usleep(20000);
            }
        }else{
            usleep(20000);
        }
    }

    return NULL;
}

static void init_asr_thread(ASRstate *e){
    //ms_mutex_init(&e->hw_mutex,NULL);
    if (e->threadstart == FALSE){
        pthread_attr_t attr;
        struct sched_param param;
        Wanson_ASR_Init();
        e->threadstart=TRUE;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 25*1024);       
        param.sched_priority = sched_get_priority_min(0);
        pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&e->asr_thread, &attr, asr_thread, e);
    }
}

static void asr_stop_hwthread(ASRstate *e){
    e->threadstart=FALSE;
    //ms_mutex_destroy(e->hw_mutex);
    ms_thread_join(e->asr_thread,NULL);
    Wanson_ASR_Release();
}

static void asr_preprocess(MSFilter *f){
    ASRstate *s=(ASRstate*)f->data;
    s->msF = f;
    if(s->asrStart) init_asr_thread(s);
}


static void asr_process(MSFilter *f){
    ASRstate *s=(ASRstate*)f->data;
    mblk_t *refm;
    if (s->bypass){
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            ms_mutex_lock(&s->mutex);
            ms_bufferizer_put(s->Buf,dupmsg(refm));
            ms_mutex_unlock(&s->mutex);            

            ms_queue_put(f->outputs[0],dupmsg(refm));
            ms_queue_put(f->outputs[1],dupmsg(refm));
        }
    }
    
    if (s->asrStart) {
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            //pre_emphasise(refm);//emphasis data
            //if(vad_detect(s,refm)){
                ms_mutex_lock(&s->mutex);
                ms_bufferizer_put(s->Buf,dupmsg(refm));
                ms_mutex_unlock(&s->mutex);
            //}else{
            //    memset(refm->b_rptr,0,refm->b_wptr-refm->b_rptr);
            //}
            ms_queue_put(f->outputs[0],refm);

        }
    }else{
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            ms_queue_put(f->outputs[0],refm);
        }       
    }
}

static void asr_postprocess(MSFilter *f){
    ASRstate *s=(ASRstate*)f->data;
    if(s->threadstart) asr_stop_hwthread(s);
    ms_bufferizer_flush (s->Buf);
}

static void asr_uninit(MSFilter *f){
    ASRstate *s=(ASRstate*)f->data;  
    ms_mutex_destroy(&s->mutex);
    ms_bufferizer_destroy(s->Buf);
    ms_free(s);
}

static int asr_pause(MSFilter *f, void *arg){
	ASRstate *s=(ASRstate*)f->data;
    int status=*(int*)arg;
    if(status)
        s->asrStart=FALSE;
    else
        s->asrStart=TRUE;
    printf("s->asrStart (%d)\n",s->asrStart);
    ms_mutex_lock(&s->mutex);
    ms_bufferizer_flush (s->Buf);
    ms_mutex_unlock(&s->mutex);
	return 0;
}

static int asr_reset(MSFilter *f, void *arg){
	ASRstate *s=(ASRstate*)f->data;
    Wanson_ASR_Reset();
    ms_mutex_lock(&s->mutex);
    ms_bufferizer_flush (s->Buf);
    ms_mutex_unlock(&s->mutex);
	return 0;
}

static int asr_release(MSFilter *f, void *arg){
	ASRstate *s=(ASRstate*)f->data;
	Wanson_ASR_Release();
	return 0;
}

static int asr_init_thread(MSFilter *f, void *arg){
	ASRstate *s=(ASRstate*)f->data;
	if(!s->threadstart) init_asr_thread(s);
	return 0;
}

static int asr_bypass(MSFilter *f, void *arg){
	ASRstate *s=(ASRstate*)f->data;
    s->bypass=*(int*)arg;
    //s->asrStart=FALSE;
    //s->threadstart=FALSE;
    printf("asr by pass mode(%d)\n",s->bypass);
	return 0;
}

static MSFilterMethod methods[]={
	{	MS_ASR_PAUSE     ,	asr_pause		},
    {	MS_ASR_RESET     ,	asr_reset		},
    {	MS_ASR_RELEASE   ,	asr_release		},
    {	MS_ASR_INIT      ,	asr_init_thread },
    {	MS_ASR_BYPASS    ,	asr_bypass		},
	{	0			     ,	NULL			}
};

#ifdef _MSC_VER

MSFilterDesc ms_asr_desc={
	MS_ASR_ID,
	"MSAsr",
	N_("A filter that ASR."),
	MS_FILTER_OTHER,
	NULL,
	1,
	2,
    asr_init,
	asr_preprocess,
	asr_process,
    asr_postprocess,
    asr_uninit,
    methods,
};

#else

MSFilterDesc ms_asr_desc={
	.id=MS_ASR_ID,
	.name="MSAsr",
	.text=N_("A filter that ASR."),
	.category=MS_FILTER_OTHER,
	.ninputs=1,
	.noutputs=2,
    .init=asr_init,
    .preprocess=asr_preprocess,
    .process=asr_process,
    .postprocess=asr_postprocess,
    .uninit=asr_uninit,
    .methods=methods,
};

#endif

MS_FILTER_DESC_EXPORT(ms_asr_desc)