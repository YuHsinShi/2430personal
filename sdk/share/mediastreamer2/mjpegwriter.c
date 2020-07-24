/*
   mediastreamer2 library - modular sound and video processing and streaming
   Copyright (C) 2010  Belledonne Communications SARL <simon.morlat@linphone.org>

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

#ifdef HAVE_CONFIG_H
    #include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msmjpegwriter.h"
#include "mediastreamer2/msfilewriter.h"

#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"

//#include <sys/time.h>
//#include <sys/stat.h>
//#include <sys/statvfs.h>

//#include "../../include/jpg/ite_jpg.h"
//#include "mjpegdec_castor3.h"

//#include "stdio.h"
#include "ite/itp.h"
//#include "fat/api/api_fat.h"
//#include "ite/ite_sd.h"


#define DEF_BitStream_BUF_LENGTH           (256 << 10)

#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE  1
#endif

typedef struct {
    ms_mutex_t  mutex;
    FILE        *file;
    //AVCodec     *codec;
    int         Runstate;
    char        filepath[PATH_MAX];
} JpegWriter;

static void jpg_init(MSFilter *f)
{
    JpegWriter *s = ms_new0(JpegWriter, 1);
    printf("jpg_init\n");
    ms_mutex_init(&s->mutex,NULL); //Benson
    f->data = s;
}

static void jpg_uninit(MSFilter *f)
{
    JpegWriter *s = (JpegWriter *)f->data;
    printf("jpg_uninit\n");
    if (s->file != NULL)
    {
        fclose(s->file);
    }
    ms_mutex_destroy(&s->mutex); 
    ms_free(s);
    f->data = NULL; 
}

static void jpg_process(MSFilter *f)
{
    JpegWriter      *s             = (JpegWriter *)f->data;
    char            *filename      = s->filepath;//(char *)arg;
    mblk_t          *im            = NULL;

	uint8_t			*jpg_stream	   = NULL;
	uint32_t		jpg_length	   = 0;
#if 0
    YUV_FRAME		*picture       = NULL;

    HJPG            *pHJpeg        = 0;
    JPG_INIT_PARAM  initParam      = {0};
    JPG_STREAM_INFO inStreamInfo   = {0};
    JPG_STREAM_INFO outStreamInfo  = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo    = {0};
    uint8_t         *ya_out        = 0, *ua_out = 0, *va_out = 0; // address of YUV decoded video buffer
    uint8_t         *pSaveBuf      = 0; 
    uint32_t        src_w_out      = 0, src_h_out = 0;
    uint32_t        jpgEncSize     = 0;
    uint32_t        pitch_y   = 2048;
    uint32_t        pitch_uv  = 2048;
#endif    
    ms_mutex_lock(&s->mutex);
    if ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
    	jpg_stream = im->b_rptr;
		jpg_length = im->b_wptr - im->b_rptr;
    	
#if 0    
		picture       = (YUV_FRAME*)(im->b_rptr);

        src_w_out     = picture->width;
        src_h_out     = picture->height;
        ya_out        = picture->data[0];
        ua_out        = picture->data[1];
        va_out        = picture->data[2];
        pitch_y  = picture->linesize[0];
        pitch_uv = picture->linesize[1];
#endif		
    }
    else
    {
        ms_mutex_unlock(&s->mutex);
        return;
    }


    if (s->Runstate == TRUE)
    {
    	printf("xxxxxxxxxxxxxxx\n");
		
		mblk_t *CompressedData = NULL;
		CompressedData = allocb(DEF_BitStream_BUF_LENGTH+DEF_FileStream_Name_LENGTH,0);
        if (CompressedData == NULL)
        {
            freemsg(im);
            ms_mutex_unlock(&s->mutex);
            return;
        }
		strcpy(CompressedData->b_wptr, s->filepath);
        CompressedData->b_wptr += DEF_FileStream_Name_LENGTH;
		memcpy(CompressedData->b_wptr, jpg_stream, jpg_length);
		CompressedData->b_wptr += jpg_length;
        ms_queue_put(f->outputs[0], CompressedData);
		
#if 0
        mblk_t *CompressedData = NULL;

        if (!ms_queue_empty(f->outputs[0]))
        {
             if (im) freemsg(im);
             ms_mutex_unlock(&s->mutex);
             return;
        }
        
        // ------------------------------------------------------
        // encode
        initParam.codecType = JPG_CODEC_ENC_JPG;
        initParam.outColorSpace = JPG_COLOR_SPACE_YUV422;
        initParam.width         = src_w_out;
        initParam.height        = src_h_out;
        initParam.encQuality    = 70;//85;

        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType         = JPG_STREAM_IO_READ;
        inStreamInfo.streamType           = JPG_STREAM_MEM;
        // Y
        inStreamInfo.jstream.mem[0].pAddr = (uint8_t *)ya_out; //YUV_Save;
        inStreamInfo.jstream.mem[0].pitch = pitch_y;      // src_w_out;

        // U
        inStreamInfo.jstream.mem[1].pAddr = (uint8_t *)ua_out; //(inStreamInfo.jstream.mem[0].pAddr+pitch_y*src_h_out);
        inStreamInfo.jstream.mem[1].pitch = pitch_uv;     //src_w_out/2;

        // V
        inStreamInfo.jstream.mem[2].pAddr = (uint8_t *)va_out; //(inStreamInfo.jstream.mem[1].pAddr+pitch_y*src_h_out);
        inStreamInfo.jstream.mem[2].pitch = pitch_uv;     //src_w_out/2;

        inStreamInfo.validCompCnt         = 3;

#if 0
        if (filename)
        {
            outStreamInfo.streamType   = JPG_STREAM_FILE;
            outStreamInfo.jstream.path = (void *)filename;
        }
        outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
        outStreamInfo.jpg_reset_stream_info = 0; //  _reset_stream_info;
#else
        CompressedData = allocb(DEF_BitStream_BUF_LENGTH+DEF_FileStream_Name_LENGTH,0);
        if (CompressedData == NULL)
        {
            freemsg(im);
            ms_mutex_unlock(&s->mutex);
            return;
        }
        
        strcpy(CompressedData->b_wptr, s->filepath);
        CompressedData->b_wptr += DEF_FileStream_Name_LENGTH;      
           
        outStreamInfo.streamIOType       = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType         = JPG_STREAM_MEM;
        outStreamInfo.jpg_reset_stream_info =  0; //  _reset_stream_info;      
        outStreamInfo.jstream.mem[0].pAddr  = CompressedData->b_wptr; 
        outStreamInfo.jstream.mem[0].pitch  = pitch_y;
        outStreamInfo.jstream.mem[0].length = DEF_BitStream_BUF_LENGTH;
        outStreamInfo.validCompCnt = 1;   
        
#endif

        printf("\n\n\tencode input: Y=0x%x, u=0x%x, v=0x%x\n",
               inStreamInfo.jstream.mem[0].pAddr,
               inStreamInfo.jstream.mem[1].pAddr,
               inStreamInfo.jstream.mem[2].pAddr);

        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);

        iteJpg_Setup(pHJpeg, 0);

        iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize / 1024);

        CompressedData->b_wptr += jpgEncSize;
        ms_queue_put(f->outputs[0], CompressedData);
        iteJpg_DestroyHandle(&pHJpeg, 0);

#endif
        if (im)
            freemsg(im);
        s->Runstate = FALSE;
        ms_mutex_unlock(&s->mutex);
        return;
    }
    else
    {
        if (im)
            freemsg(im);
        ms_mutex_unlock(&s->mutex);
        return;
    }
}


static int take_snapshot(MSFilter *f, void *arg)
{
#if 1//def CFG_SD0_ENABLE
    JpegWriter *s = (JpegWriter *)f->data;  // it can using  ms_error to debug  -> ms_error("take_snapshot.");

    ms_mutex_lock(&s->mutex);
    s->Runstate = TRUE;
    strcpy(s->filepath, (char*)arg);
    ms_mutex_unlock(&s->mutex);
#endif
    return 1;
}
static void jpg_preprocess(MSFilter *f)
{
    JpegWriter *s                   = (JpegWriter *)f->data;
}

static MSFilterMethod jpg_methods[] = {
    {       MS_MJPEG_WRITER_TAKE_SNAPSHOT, take_snapshot },
    {       0, NULL}
};

#ifndef _MSC_VER

MSFilterDesc ms_mjpeg_writer_desc = {
    .id       = MS_MJPEG_WRITER_ID,
    .name     = "MSMjpegWriter",
    .text     = "Take a mjpeg video snapshot as jpg file",
    .category = MS_FILTER_ENCODER,
    .enc_fmt  = "Jpeg",
    .ninputs  = 1,
    .noutputs = 1,
    .init     = jpg_init,
    .process  = jpg_process,
    .uninit   = jpg_uninit,
    .methods  = jpg_methods
};
#else

MSFilterDesc ms_mjpeg_writer_desc = {
    MS_MJPEG_WRITER_ID,
    "MSMjpegWriter",
    "Take a mjpeg video snapshot as jpg file",
    MS_FILTER_ENCODER,
    "Jpeg",
    1,
    1,
    jpg_init,
    NULL,
    jpg_process,
    NULL,
    jpg_uninit,
    jpg_methods
};
#endif

MS_FILTER_DESC_EXPORT(ms_mjpeg_writer_desc)
