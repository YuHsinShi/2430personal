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
#define DUMP_ITE 

extern dmnr_state dmnr_config[1];
typedef struct MSstate{
    MSFilter *msF;
    MSBufferizer *dualBuf;
    int framesize;
    bool_t bypass_mode;
#ifdef DUMP_ITE    
    queue_t mic1_copy_q;
    queue_t mic2_copy_q;    
    queue_t o1_copy_q;
    queue_t o2_copy_q;     
#endif        
}MSstate;


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

static void dmnr_separate_init(MSFilter *f){
    MSstate *s=(MSstate *)ms_new(MSstate,1);
    f->data=s;
    s->dualBuf =ms_bufferizer_new();
    s->framesize = 128;
    Init_dmnr_hand_held(dmnr_config);
    s->bypass_mode = true;
    printf("dmnr_separate_init\n");
#ifdef DUMP_ITE    
    qinit(&s->mic1_copy_q);
    qinit(&s->mic2_copy_q); 
    qinit(&s->o1_copy_q);
    qinit(&s->o2_copy_q); 
#endif    
}

static void dmnr_separate_preprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
}


static void dmnr_separate_process(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    int nbytes=s->framesize*2;
  
    ms_bufferizer_put_from_queue(s->dualBuf,f->inputs[0]);
    
    while(ms_bufferizer_get_avail(s->dualBuf)>=nbytes*2){
        mblk_t *o1,*o2;
        mblk_t *im;
        mblk_t *p1,*p2;
        
        im=allocb(nbytes*2,0);
        ms_bufferizer_read(s->dualBuf,im->b_wptr,nbytes*2);
        im->b_wptr+=nbytes*2;
        
        o1=allocb(nbytes,0);
        o2=allocb(nbytes,0);
        p1=allocb(nbytes,0);
        p2=allocb(nbytes,0);

        for(;im->b_rptr<im->b_wptr;o1->b_wptr+=2,o2->b_wptr+=2){
            *((int16_t*)(o1->b_wptr))=(int)*(int16_t*)im->b_rptr;im->b_rptr+=2;
            *((int16_t*)(o2->b_wptr))=(int)*(int16_t*)im->b_rptr;im->b_rptr+=2;
        } 
#ifdef DUMP_ITE        
        putq(&s->mic1_copy_q, dupmsg(o1));
        putq(&s->mic2_copy_q, dupmsg(o2));       
#endif    
        dmnr_hand_held((short*)(o2->b_rptr),(short*)(o1->b_rptr),(short*)(p2->b_wptr),(short*)(p1->b_wptr),dmnr_config);
        p1->b_wptr+=nbytes;
        p2->b_wptr+=nbytes;
#ifdef DUMP_ITE        
        putq(&s->o1_copy_q, dupmsg(p1));
        putq(&s->o2_copy_q, dupmsg(p2));       
#endif       
        if(s->bypass_mode){
            ms_queue_put(f->outputs[0],o1);
            if(o2) freemsg(o2);
            if(p1) freemsg(p1);
            if(p2) freemsg(p2);
        }else{
            ms_queue_put(f->outputs[0],p1);
            if(o1) freemsg(o1);
            if(o2) freemsg(o2);
            if(p2) freemsg(p2);
        }
        if(im) freemsg(im);
    }
}

static void dmnr_separate_postprocess(MSFilter *f){
    MSstate *s=(MSstate*)f->data;
    
#ifdef DUMP_ITE
    FILE *mic1file;
    FILE *mic2file;
    FILE *o1file;
    FILE *o2file;
    mblk_t *mic1;
    mblk_t *mic2;
    mblk_t *o1;
    mblk_t *o2;
    int nbytes=s->framesize*2;
    static int index = 0;
    char *fname;
    char USBPATH = Get_Storage_path();
    if(USBPATH == NULL){
        flushq(&s->mic1_copy_q,0);
        flushq(&s->mic2_copy_q,0);
        flushq(&s->o1_copy_q,0);
        flushq(&s->o2_copy_q,0);
        printf("USB not insert \n");
        return ;
    }
    
    printf("save audio data in USB %c:/ \n",USBPATH);
    fname=ms_strdup_printf("%c:/mic1%03d.raw",USBPATH,index);
    mic1file=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/mic2%03d.raw",USBPATH,index);
    mic2file=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/o1%03d.raw",USBPATH,index);
    o1file=fopen(fname,"w");
    ms_free(fname);
    fname=ms_strdup_printf("%c:/o2%03d.raw",USBPATH,index);
    o2file=fopen(fname,"w");
    ms_free(fname);
    index++;
    while (1)
    {
        mic1=mic2=o1=o2=NULL;
        mic1=getq(&s->mic1_copy_q);
        mic2=getq(&s->mic2_copy_q);
        o1=getq(&s->o1_copy_q);
        o2=getq(&s->o2_copy_q);
        if (mic1 && mic2 && o1 && o2)
        {
            fwrite(mic1->b_rptr,nbytes,1,mic1file);
            freemsg(mic1);            
            fwrite(mic2->b_rptr,nbytes,1,mic2file);
            freemsg(mic2);
            fwrite(o1->b_rptr,nbytes,1,o1file);
            freemsg(o1);            
            fwrite(o2->b_rptr,nbytes,1,o2file);
            freemsg(o2);
        }
        else
        {
            flushq(&s->mic1_copy_q,0);
            flushq(&s->mic2_copy_q,0);
            flushq(&s->o1_copy_q,0);
            flushq(&s->o2_copy_q,0);
            fclose(mic1file);
            fclose(mic2file);
            fclose(o1file);
            fclose(o2file);
            break;
        }
    }
#endif

}

static void dmnr_separate_uninit(MSFilter *f){
    MSstate *s=(MSstate*)f->data;  
    ms_bufferizer_destroy(s->dualBuf);
    ms_free(s);
}

static int dmnr_separate_bypass_mode(MSFilter *f, void *arg){
	MSstate *s=(MSstate*)f->data;
	s->bypass_mode= *(int*)arg;
    return 0;
}

static MSFilterMethod dmnr_separate_methods[]={
    {MS_FILTER_SET_BYPASS_MODE    , dmnr_separate_bypass_mode},
};

#ifdef _MSC_VER

MSFilterDesc ms_separate_desc={
	MS_SEPARATE_ID,
	"MSSeparate",
	N_("A filter that separate one inputs to two output."),
	MS_FILTER_OTHER,
	NULL,
	1,
	1,
    dmnr_separate_init,
	dmnr_separate_preprocess,
	dmnr_separate_process,
    dmnr_separate_postprocess,
    dmnr_separate_uninit,
    dmnr_separate_methods
};

#else

MSFilterDesc ms_separate_desc={
	.id=MS_SEPARATE_ID,
	.name="MSSeparate",
	.text=N_("A filter that separate one inputs to two output."),
	.category=MS_FILTER_OTHER,
	.ninputs=1,
	.noutputs=1,
    .init=dmnr_separate_init,
    .preprocess=dmnr_separate_preprocess,
    .process=dmnr_separate_process,
    .postprocess=dmnr_separate_postprocess,
    .uninit=dmnr_separate_uninit,
    .methods=dmnr_separate_methods
};

#endif

MS_FILTER_DESC_EXPORT(ms_separate_desc)