/*
 * basic_op.c
 *
 * Author: Hanxuan
 * Hint : separate i2s data 1 input (2 channel :left & right) to two independent output  
 */
#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"

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
    MSBufferizer *LBuf;
    MSBufferizer *RBuf;
    int framesize;
    //int input_channel;
    //int output_channel;    
    bool_t start;   
    ms_thread_t merge_thread;	   
    ms_mutex_t mutex;   
#ifdef DUMP_ITE
    queue_t spk_copy_q;
    queue_t lin_copy_q;
#endif
    
}MSstate;


static void channel_merge_init(MSFilter *f){
    MSstate *s=(MSstate *)ms_new(MSstate,1);
    f->data=s;
    s->LBuf =ms_bufferizer_new();
    s->RBuf =ms_bufferizer_new();
    s->framesize = 256*(CFG_AUDIO_SAMPLING_RATE/8000);
    ms_mutex_init(&s->mutex,NULL);
    s->start = FALSE;
    //s->input_channel = 1;
    //s->output_channel = 1;
    printf("channel_merge_init\n");  
#ifdef DUMP_ITE
    qinit(&s->spk_copy_q);
    qinit(&s->lin_copy_q);
#endif
    
}

static void *channel_merge_thread(void *arg) {
    MSstate *s=(MSstate*)arg;
    int nbytes    = s->framesize*2;

    while(s->start){
        int err1,err2;
        mblk_t *iL,*iR;
        iL=allocb(nbytes*2,0);
        iR=allocb(nbytes*2,0);
        
        ms_mutex_lock(&s->mutex);
        err1 = ms_bufferizer_read(s->LBuf,iL->b_wptr,nbytes*2);
        err2 = ms_bufferizer_read(s->RBuf,iR->b_wptr,nbytes*2);
        ms_mutex_unlock(&s->mutex);
        
        if(err1 && err2){
            mblk_t *o;
            o=allocb(nbytes*4,0);
            iL->b_wptr+=nbytes*2;
            iR->b_wptr+=nbytes*2;             
            for(;iL->b_rptr<iL->b_wptr;o->b_wptr+=4,iL->b_rptr+=2,iR->b_rptr+=2){
                *((int16_t*)(o->b_wptr+2))=(int)*(int16_t*)iL->b_rptr;
                *((int16_t*)(o->b_wptr  ))=(int)*(int16_t*)iR->b_rptr;
            }              
#ifdef DUMP_ITE
            putq(&s->spk_copy_q, dupmsg(iL));
            putq(&s->lin_copy_q, dupmsg(iR));
#endif
            ms_queue_put(s->msF->outputs[0],o);
        }else{
            usleep(20000);
        }
        
        if(iL) freemsg(iL);
        if(iR) freemsg(iR);
        
        if(s->start==FALSE) break;
        
    }

    return NULL;
}

static void channel_merge_init_thread(MSstate *e){
    //ms_mutex_init(&e->hw_mutex,NULL);
    if (e->start == FALSE){
        pthread_attr_t attr;
        struct sched_param param;
        e->start=TRUE;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 25*1024);       
        param.sched_priority = sched_get_priority_min(0);
        pthread_attr_setschedparam(&attr, &param);
        ms_thread_create(&e->merge_thread, &attr, channel_merge_thread, e);
    }
}

static void channel_merge_stop_hwthread(MSstate *e){
    e->start=FALSE;
    //ms_mutex_destroy(e->hw_mutex);
    ms_thread_join(e->merge_thread,NULL);      
}

static void channel_merge_preprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    s->msF = f;
    channel_merge_init_thread(s);
}

/*
input[0]:speaker data : play to local data
input[1]:Line out     : send to far end data
*/
static void channel_merge_process(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    int nbytes=s->framesize*2;
    
   /* if(s->input_channel == s->output_channel){
        mblk_t *refm;
        while((refm=ms_queue_get(f->inputs[0]))!=NULL){
            ms_queue_put(f->outputs[0],refm);
        }
        return;
    }
  */
    ms_mutex_lock(&s->mutex);
    ms_bufferizer_put_from_queue(s->LBuf,f->inputs[0]);
    ms_bufferizer_put_from_queue(s->RBuf,f->inputs[1]);
    ms_mutex_unlock(&s->mutex);
}

static void channel_merge_postprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    channel_merge_stop_hwthread(s);
    ms_bufferizer_flush (s->LBuf);
    ms_bufferizer_flush (s->RBuf);
    
#ifdef DUMP_ITE
    FILE *spkfile;
    FILE *linfile;
    mblk_t *spk;
    mblk_t *lin;
    int nbytes=s->framesize*2;
    static int index = 0;
    char *fname;
    char USBPATH = Get_Storage_path();
    if(USBPATH == NULL){
        flushq(&s->spk_copy_q,0);
        flushq(&s->lin_copy_q,0);
        printf("USB not insert \n");
        return ;
    }
    
    printf("save audio data in USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/spk%03d.raw",USBPATH,index);
    spkfile=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/lin%03d.raw",USBPATH,index);
    linfile=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        spk=lin=NULL;
        spk=getq(&s->spk_copy_q);
        lin=getq(&s->lin_copy_q);
        if (spk && lin)
        {
            fwrite(spk->b_rptr,nbytes,1,spkfile);
            freemsg(spk);            
            fwrite(lin->b_rptr,nbytes,1,linfile);
            freemsg(lin);
        }
        else
        {
            flushq(&s->spk_copy_q,0);
            flushq(&s->lin_copy_q,0);
            fclose(spkfile);
            fclose(linfile);
            break;
        }
    }
#endif
}

static void channel_merge_uninit(MSFilter *f){
    MSstate *s=(MSstate*)f->data;  
    ms_mutex_destroy(&s->mutex);
    ms_bufferizer_destroy(s->LBuf);
    ms_bufferizer_destroy(s->RBuf);
    ms_free(s);
}
/*
static int channel_merge_set_input_nchannel(MSFilter *f, void *arg){
	MSstate *s=(MSstate*)f->data;
	s->input_channel= *(int*)arg;
    printf("channel_merge_set_input_nchannel\n");
    return 0;
}

static int channel_merge_set_output_nchannel(MSFilter *f, void *arg){
	MSstate *s=(MSstate*)f->data;
	s->output_channel= *(int*)arg;
    printf("channel_merge_set_output_nchannel\n");
    return 0;
}

static MSFilterMethod channel_merge_methods[]={
    {MS_FILTER_SET_NCHANNELS       , channel_merge_set_input_nchannel },
    {MS_FILTER_SET_OUTPUT_NCHANNELS, channel_merge_set_output_nchannel}
};
*/
#ifdef _MSC_VER

MSFilterDesc ms_channel_merge_desc={
	MS_CHANNEL_MERGE_ID,
	"MSChannelMerge",
	N_("A filter that merge channel duplicate or separate."),
	MS_FILTER_OTHER,
	NULL,
	2,
	1,
    channel_merge_init,
	channel_merge_preprocess,
	channel_merge_process,
    channel_merge_postprocess,
    channel_merge_uninit,
    NULL
};

#else

MSFilterDesc ms_channel_merge_desc={
	.id=MS_CHANNEL_MERGE_ID,
	.name="MSChannelMerge",
	.text=N_("A filter that merge channel duplicate or separate."),
	.category=MS_FILTER_OTHER,
	.ninputs=2,
	.noutputs=1,
    .init=channel_merge_init,
    .preprocess=channel_merge_preprocess,
    .process=channel_merge_process,
    .postprocess=channel_merge_postprocess,
    .uninit=channel_merge_uninit
    //.methods=channel_merge_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_channel_merge_desc)