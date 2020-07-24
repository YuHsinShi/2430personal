#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif
#ifdef CFG_BUILD_ASR
#define RUN_IN_ARMLITE
#endif

#ifdef RUN_IN_ARMLITE
    #include "mediastreamer2/msfilter.h"
    #include "ite/audio.h"
#else
    #include "mediastreamer2/msfilter.h"
    #include "type_def.h"
    #include "aecm_core.h"
    #include "basic_op.h"
    #include "hd_aec.h"
    #include "rfft_256.h"
#endif


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

typedef struct MSstate{
    MSFilter *msF;
    MSBufferizer *Buf;
    int framesize;
    int samplerate;
    ms_mutex_t mutex;   
#ifdef DUMP_ITE
    queue_t in_copy_q;
    queue_t out_copy_q;
#endif
    
}MSstate;

static inline int16_t saturate(int val) {
	return (val>32767) ? 32767 : ( (val<-32767) ? -32767 : val);
}

static void filter_denoise_init(MSFilter *f){
    MSstate *s=(MSstate *)ms_new(MSstate,1);
    f->data=s;
    s->Buf =ms_bufferizer_new();
    s->samplerate = CFG_AUDIO_SAMPLING_RATE;
    s->framesize = 128;
    ms_mutex_init(&s->mutex,NULL);
#ifdef DUMP_ITE
    qinit(&s->in_copy_q);
    qinit(&s->out_copy_q);
#endif
#ifdef RUN_IN_ARMLITE
    iteAudioOpenEngine(ITE_SBC_CODEC);
#endif   
}

static void filter_denoise_stop_hwthread(MSstate *e){
    //ms_thread_join(e->merge_thread,NULL);      
}

static void filter_denoise_preprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    #ifdef RUN_IN_ARMLITE
    printf("denoise in ARM-LITE\n");
    iteAecCommand(NR_CMD_INIT, 0, 0, 0, 0, 0);   
    #else 
    NR_Create(&anr_config[0], 1);
    #endif
    s->msF = f;
    //filter_denoise_init_thread(s);
}

/*
input[0]:speaker data : play to local data
input[1]:Line out     : send to far end data
*/
static void filter_denoise_process(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    int nbytes=s->framesize*2;
    
    ms_mutex_lock(&s->mutex);
    ms_bufferizer_put_from_queue(s->Buf,f->inputs[0]);
    ms_mutex_unlock(&s->mutex);
    
    while((ms_bufferizer_get_avail(s->Buf)>=nbytes)){
        mblk_t *in,*out;
        in =allocb(nbytes,0);
        out=allocb(nbytes,0);          
        ms_bufferizer_read(s->Buf,in->b_wptr,nbytes);
        in->b_wptr+=nbytes;
        {
            #ifdef RUN_IN_ARMLITE
            iteAecCommand(NR_CMD_PROCESS,(unsigned int) in->b_rptr,0,(unsigned int) out->b_wptr,nbytes,0);    
            #else
            Overlap_Add((Word16*)in->b_rptr,(Word16*)out->b_wptr,&anr_config[0]);
            #endif
            
            //memcpy(out->b_wptr,in->b_rptr,nbytes*2);
            
#ifdef DUMP_ITE
            putq(&s->in_copy_q, dupmsg(in ));
            putq(&s->out_copy_q, dupmsg(out));
#endif      
            out->b_wptr+=nbytes;
            ms_queue_put(f->outputs[0],out);
        }
        if(in) freemsg(in);
    }
}

static void filter_denoise_postprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    //filter_denoise_stop_hwthread(s);
    ms_bufferizer_flush (s->Buf);
    
#ifdef DUMP_ITE
    FILE *infile;
    FILE *outfile;
    mblk_t *in_dump;
    mblk_t *out_dump;
    int nbytes=s->framesize*2;
    static int index = 0;
    char *fname;
    char USBPATH = Get_Storage_path();
    if(USBPATH == NULL){
        flushq(&s->in_copy_q,0);
        flushq(&s->out_copy_q,0);
        printf("USB not insert \n");
        return ;
    }
    
    printf("save audio data infile USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/in_dump%03d.raw",USBPATH,index);
    infile=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/out_dump%03d.raw",USBPATH,index);
    outfile=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        in_dump=out_dump=NULL;
        in_dump=getq(&s->in_copy_q);
        out_dump=getq(&s->out_copy_q);
        if (in_dump && out_dump)
        {
            fwrite(in_dump->b_rptr,nbytes,1,infile);
            freemsg(in_dump);            
            fwrite(out_dump->b_rptr,nbytes,1,outfile);
            freemsg(out_dump);
        }
        else
        {
            flushq(&s->in_copy_q,0);
            flushq(&s->out_copy_q,0);
            fclose(infile);
            fclose(outfile);
            break;
        }
    }
#endif
}

static void filter_denoise_uninit(MSFilter *f){
    MSstate *s=(MSstate*)f->data;  
    ms_mutex_destroy(&s->mutex);
    ms_bufferizer_destroy(s->Buf);
    ms_free(s);
}

static int set_sample_rate(MSFilter *f, void *arg){
	MSstate *s=(MSstate*)f->data; 
    s->samplerate=*((int*)arg);

    return 0;
}

static int set_max_energy(MSFilter *f, void *arg){
	MSstate *s=(MSstate*)f->data; 

	return 0;
}

static MSFilterMethod filter_denoise_methods[]={
    {    MS_FILTER_SET_SAMPLE_RATE  , set_sample_rate   },
    {	 MS_FILTER_SET_MAX_GAIN     , set_max_energy    },
	{	0			                , NULL            }
};

#ifdef _MSC_VER

MSFilterDesc ms_filter_denoise_desc={
	MS_FILTER_DENOISE_ID,
	"MSFilterDenoise",
	N_("A filter that do something ."),
	MS_FILTER_OTHER,
	NULL,
	1,
	1,
    filter_denoise_init,
	filter_denoise_preprocess,
	filter_denoise_process,
    filter_denoise_postprocess,
    filter_denoise_uninit,
    filter_denoise_methods,
};

#else

MSFilterDesc ms_filter_denoise_desc={
	.id=MS_FILTER_DENOISE_ID,
	.name="MSFilterDenoise",
	.text=N_("A filter that do something ."),
	.category=MS_FILTER_OTHER,
	.ninputs=1,
	.noutputs=1,
    .init=filter_denoise_init,
    .preprocess=filter_denoise_preprocess,
    .process=filter_denoise_process,
    .postprocess=filter_denoise_postprocess,
    .uninit=filter_denoise_uninit,
    .methods=filter_denoise_methods,
};

#endif

MS_FILTER_DESC_EXPORT(ms_filter_denoise_desc)
