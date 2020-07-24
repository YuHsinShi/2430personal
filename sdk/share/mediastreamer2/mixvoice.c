
#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/dtmfgen.h"

#define max_file 10

typedef struct _Mixdata{
    unsigned char* pcm_buf;
    char mixfile[256];
    unsigned int mix_size;
    unsigned int end;
    float mixlevel;
    float baselevel;
    int repeatN;
    bool_t flag;
    bool_t fadingIN;
    bool_t fadingOUT;
    ms_mutex_t mutex;
}Mixdata;

static inline uint32_t swap32(uint32_t a)
{
	return ((a & 0xFF) << 24) | ((a & 0xFF00) << 8) | 
		((a & 0xFF0000) >> 8) | ((a & 0xFF000000) >> 24);
}

static inline int16_t saturate(int val) {
	return (val>32767) ? 32767 : ( (val<-32767) ? -32767 : val);
}

static int wav_file_open(MSFilter *f, void *arg){
    Mixdata *d=(Mixdata*)f->data;
    char *filepath=(char *)arg;  
    FILE *fp[max_file];
    const char *file_name;
    unsigned int wpidx=0;    
    int data_size[max_file];    
    int file_num=0;
    int i,j;

    ms_mutex_lock(&d->mutex);
    if(strcmp(d->mixfile,filepath)==0){
        d->flag = TRUE;
        d->fadingIN  =TRUE;
        d->fadingOUT =FALSE;
        d->baselevel= 1.0;
        ms_mutex_unlock(&d->mutex);
        return 0;
    }
    //d->mixfile = filepath;
    strcpy(d->mixfile, filepath);
    d->mix_size = 0;
    file_name = strtok(filepath, " ");
    while (file_name != NULL){
        char* buf;
        fp[file_num] = fopen(file_name,"rb");
        
        if( NULL == fp[file_num] ){
            printf( "[mixerror] %s open failure\n",file_name);
            ms_mutex_unlock(&d->mutex);
            return 0;
        }
        
        buf = malloc(50);
        fread(buf, 1, 44, fp[file_num]);
        data_size[file_num] = *((int*)&buf[40]);
        d->mix_size += data_size[file_num];
        file_name = strtok(NULL, " ");
        free(buf);
        file_num++;
    }
    if(d->pcm_buf) free(d->pcm_buf);
    d->pcm_buf = (unsigned char*)malloc(d->mix_size);
    for(j=0;j<file_num;j++){
        unsigned char* file_pcm;
        file_pcm = (unsigned char*)malloc(data_size[j]);
        fread(file_pcm, 1, data_size[j], fp[j]);
        memmove(d->pcm_buf+wpidx,file_pcm,data_size[j]);
        wpidx += data_size[j];
        free(file_pcm);
        fclose(fp[j]);
    }
    
    d->flag = TRUE;
    d->fadingIN  =TRUE;
    d->fadingOUT =FALSE;
    d->baselevel= 1.0;
    ms_mutex_unlock(&d->mutex);
    return 0;
} 

static void mix_voice_init(MSFilter *f){
    Mixdata *d=(Mixdata*)ms_new(Mixdata,1);
    d->flag =FALSE;
    d->fadingIN =FALSE;
    d->fadingOUT =FALSE;
    d->end = 0;
    d->mix_size = 0;
    d->pcm_buf = NULL;
    d->mixlevel = 1.0;
    d->baselevel= 1.0;
    d->repeatN  = 0;
    ms_mutex_init(&d->mutex,NULL);
    //d->mixfile = "null";
    f->data=d;
    
}

static void mix_voice_process(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    mblk_t *m;
    ms_mutex_lock(&d->mutex);
    if(d->flag){
        
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
            mblk_t *o;
            //msgpullup(m,-1);
            o=allocb(m->b_wptr-m->b_rptr,0);
            //mblk_meta_copy(m, o);
            if(d->fadingIN){
                //printf("size %d ",m->b_wptr-m->b_rptr);
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                    *((int16_t*)(o->b_wptr))=(d->baselevel)*((int)*(int16_t*)m->b_rptr);
                }
                d->baselevel-=0.005;
                if(d->baselevel<=1-d->mixlevel) {
                    d->baselevel=1-d->mixlevel;
                    d->fadingIN=FALSE;
                }
            }else if(!d->fadingIN && !d->fadingOUT){
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2,d->end+=2){
                    if(d->end < d->mix_size){//mixsound
                    //*((int16_t*)(o->b_wptr))=*((int16_t*)(d->pcm_buf+d->end));
                    *((int16_t*)(o->b_wptr))=saturate(*((int16_t*)(d->pcm_buf+d->end)))*(d->mixlevel)+(d->baselevel)*((int)*(int16_t*)m->b_rptr);
                    }else{//mixsound over
                            *((int16_t*)(o->b_wptr))=(d->baselevel)*((int)*(int16_t*)m->b_rptr);
                    }
                }
            }else{
                //printf("size %d ",m->b_wptr-m->b_rptr);
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                    *((int16_t*)(o->b_wptr))=(d->baselevel)*((int)*(int16_t*)m->b_rptr);
                }
                d->baselevel+=0.005;
                if(d->baselevel>=1) {
                    d->baselevel=1;
                    d->flag=FALSE;
                }                    
            }
            
            if(d->end >= d->mix_size){
                if(d->repeatN == 1){//mix 1 time
                    d->end=0;
                    d->fadingOUT=TRUE;                    
                }else if(d->repeatN < 0){//repeat mix
                    d->end=0;
                }else{
                    d->end=0;//mix mixlevel times
                    d->repeatN--;
                }
            };
            freemsg(m);
            ms_queue_put(f->outputs[0],o);
        }
        
    }else{
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
        ms_queue_put(f->outputs[0],m);
         }
    }
    ms_mutex_unlock(&d->mutex);
}
/*
static void mix_voice_process2(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    mblk_t *m;
    if(d->flag){
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
            mblk_t *o;
            //msgpullup(m,-1);
            o=allocb(m->b_wptr-m->b_rptr,0);
            //mblk_meta_copy(m, o);
            if(d->flag){
                if(d->fadingIN){
                    printf("size %d ",m->b_wptr-m->b_rptr);
                    for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                        *((int16_t*)(o->b_wptr))=(d->mixlevel)*((int)*(int16_t*)m->b_rptr);
                    }
                    d->mixlevel-=0.005;
                    if(d->mixlevel<=0) {
                        d->mixlevel=0;
                        d->fadingIN=FALSE;
                    }
                }else if(!d->fadingIN && !d->fadingOUT){
                    for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2,d->end+=2){
                        if(d->end < d->mix_size){//mixsound
                        *((int16_t*)(o->b_wptr))=*((int16_t*)(d->pcm_buf+d->end));
                        }else{//mixsound over
                                *((int16_t*)(o->b_wptr))=(int)*(int16_t*)m->b_rptr;
                        }
                    }
                }else{
                    printf("size %d ",m->b_wptr-m->b_rptr);
                    for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                        *((int16_t*)(o->b_wptr))=(d->mixlevel)*((int)*(int16_t*)m->b_rptr);
                    }
                    d->mixlevel+=0.005;
                    if(d->mixlevel>=1) {
                        d->mixlevel=1;
                        d->flag=FALSE;
                    }                    
                }
            }else{//no mixsound
                for(;m->b_rptr<m->b_wptr;m->b_rptr+=2,o->b_wptr+=2){
                    *((int16_t*)(o->b_wptr))=(int)*(int16_t*)m->b_rptr;
                }              
            }
            if(d->end > d->mix_size){
                d->end=0;
                d->fadingOUT=TRUE;
            };
            freemsg(m);
            ms_queue_put(f->outputs[0],o);
        }
    }else{
        while((m=ms_queue_get(f->inputs[0]))!=NULL){
        ms_queue_put(f->outputs[0],m);
         }
    }
    
}*/

static void mix_voice_postprocess(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    d->flag =FALSE;
    d->end = 0;
    d->mix_size = 0;
    d->repeatN = 0;
    free(d->pcm_buf);
    d->pcm_buf = NULL;
    ms_mutex_destroy(&d->mutex);
}

static void mix_voice_uninit(MSFilter *f){
    Mixdata *d=(Mixdata*)f->data;
    ms_free(d);
}

static int mix_set_level(MSFilter *f, void *arg){
    Mixdata *d=(Mixdata*)f->data;
    ms_mutex_lock(&d->mutex);
    d->mixlevel=*((double*)arg);
    ms_mutex_unlock(&d->mutex);  
    return 0;
}

static int mix_set_repeat(MSFilter *f, void *arg){
    Mixdata *d=(Mixdata*)f->data;
    if(d->flag) d->flag=FALSE;
    ms_mutex_lock(&d->mutex);
    d->repeatN=*((int*)arg);//repeatN=-1 :repeat forever ,0:NO repeat ,>0 repeat N times
    if(d->repeatN==0){
        d->end=0;
        d->fadingOUT=TRUE;
    }
    ms_mutex_unlock(&d->mutex);    
    return 0;    
}

 static MSFilterMethod mixvoice_methods[]={
    {   MS_WAV_FILE_OPEN        ,   wav_file_open},
    {   MS_MIXVOISE_SET_LEVEL   ,   mix_set_level},
    {   MS_MIXVOISE_SET_REPEAT  ,   mix_set_repeat},
    {   0               ,   NULL        }
}; 

#ifdef _MSC_VER

MSFilterDesc ms_mix_voice_desc={
    MS_MIXVOICE_ID,
    "MSMixVoice",
    N_("Mix the wave"),
    MS_FILTER_OTHER,
    NULL,
    1,
    1,
    mix_voice_init,
    NULL,
    mix_voice_process,
    mix_voice_postprocess,
    mix_voice_uninit,
    mixvoice_methods
};

#else

MSFilterDesc ms_mix_voice_desc={
    .id=MS_MIXVOICE_ID,
    .name="MSMixVoice",
    .text=N_("Mix the wave"),
    .category=MS_FILTER_OTHER,
    .ninputs=1,
    .noutputs=1,
    .init = mix_voice_init,
    .process = mix_voice_process,
    .uninit = mix_voice_uninit,
    .postprocess=mix_voice_postprocess,
    .methods=mixvoice_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_mix_voice_desc)