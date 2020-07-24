/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#if defined(HAVE_CONFIG_H)
#include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"

#include "type_def.h"
#include "aecm_core.h"
#include "basic_op.h"
#include "hd_aec.h"
#include "rfft_256.h"
#include "dmnr_held.h"

static ARG argc;
static int Cntr=0;
extern ica_state ica_config[1];
extern dmnr_state dmnr_config[1];
typedef struct ICAState{
    MSFilter *msF;
    MSBufferizer *dualBuf;
    int ICAframesize;

    pthread_t ICApid;
    pthread_mutex_t mxq;
        
}ICAState;

static void ICA_INIT(int *ICAframesize){
    ICA_SeparateInit(&ica_config[0]);
    Init_dmnr_hand_held(dmnr_config);
    *ICAframesize = 128;
    Cntr+=1;
}

static void ICA_UNINIT(void){
    ICA_SeparateDestroy(&ica_config[0]);   
}

static void ICA_start_thread(ICAState *e){
    int err;
    pthread_attr_t tattrICA;
    struct sched_param param;
    pthread_mutex_init(&e->mxq, NULL);
    pthread_mutex_lock(&e->mxq);
    argc.ica_config = &ica_config[0]; 
    argc.mx = &e->mxq;

    pthread_attr_init(&tattrICA);
    pthread_attr_setstacksize(&tattrICA, 32*1024);
    param.sched_priority = sched_get_priority_min(0);
    pthread_attr_setschedparam(&tattrICA, &param);
    err = pthread_create(&e->ICApid, &tattrICA, pca_ica_thread_func, &argc);
}

static void sbc_ICA_stop_thread(ICAState *e){
    pthread_mutex_unlock(&e->mxq);
    pthread_join(e->ICApid, NULL);
    pthread_mutex_destroy(&e->mxq);    
}

static void ica_separate_init(MSFilter *f){
    ICAState *s=(ICAState *)ms_new(ICAState,0);
    f->data=s;
    s->dualBuf =ms_bufferizer_new();
    ICA_INIT(&s->ICAframesize);
}

static void ica_separate_preprocess(MSFilter *f){

    ICAState *s=(ICAState*)f->data;
    ICA_start_thread(s);
}


static void ica_separate_process(MSFilter *f){
    ICAState *s=(ICAState*)f->data;
    int nbytes=s->ICAframesize*2;
  
    ms_bufferizer_put_from_queue(s->dualBuf,f->inputs[0]);
    
    while(ms_bufferizer_get_avail(s->dualBuf)>=nbytes*2){
        mblk_t *o1,*o2;
        mblk_t *im;
        
        im=allocb(nbytes*2,0);
        ms_bufferizer_read(s->dualBuf,im->b_wptr,nbytes*2);
        im->b_wptr+=nbytes*2;
        
        o1=allocb(nbytes,0);
        o2=allocb(nbytes,0);

        for(;im->b_rptr<im->b_wptr;o1->b_wptr+=2,o2->b_wptr+=2){
            *((int16_t*)(o1->b_wptr))=(int)*(int16_t*)im->b_rptr;im->b_rptr+=2;
            *((int16_t*)(o2->b_wptr))=(int)*(int16_t*)im->b_rptr;im->b_rptr+=2;
        }
        
        if(Cntr%2==0)
        //ICA_Separation((short*)(o1->b_rptr), (short*)(o2->b_rptr), (short*)(o1->b_rptr), (short*)(o2->b_rptr), &ica_config[0]);
        dmnr_hand_held((short*)(o2->b_rptr),(short*)(o1->b_rptr),(short*)(o2->b_rptr),(short*)(o1->b_rptr),dmnr_config);
        ms_queue_put(f->outputs[0],o1);
        if(o2) freemsg(o2);
        if(im) freemsg(im);
    }
}

static void ica_separate_postprocess(MSFilter *f){
    ICAState *s=(ICAState*)f->data;
    sbc_ICA_stop_thread(s);

}

static void ica_separate_uninit(MSFilter *f){
    ICAState *s=(ICAState*)f->data;  
    ICA_UNINIT();
    ms_bufferizer_destroy(s->dualBuf);
    ms_free(s);
}


#ifdef _MSC_VER

MSFilterDesc ms_separate_desc={
	MS_SEPARATE_ID,
	"MSSeparate",
	N_("A filter that separate one inputs to two output."),
	MS_FILTER_OTHER,
	NULL,
	1,
	1,
    ica_separate_init,
	ica_separate_preprocess,
	ica_separate_process,
    ica_separate_postprocess,
    ica_separate_uninit,
    NULL
};

#else

MSFilterDesc ms_separate_desc={
	.id=MS_SEPARATE_ID,
	.name="MSSeparate",
	.text=N_("A filter that separate one inputs to two output."),
	.category=MS_FILTER_OTHER,
	.ninputs=1,
	.noutputs=1,
    .init=ica_separate_init,
    .preprocess=ica_separate_preprocess,
    .process=ica_separate_process,
    .postprocess=ica_separate_postprocess,
    .uninit=ica_separate_uninit,
};

#endif

MS_FILTER_DESC_EXPORT(ms_separate_desc)