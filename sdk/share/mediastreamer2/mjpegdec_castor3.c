#include "mediastreamer2/msfilter.h"
#include "mediastreamer2/msticker.h"

#include "mjpegdec_castor3.h"
#include "jpg/ite_jpg.h"
#include "ite/itv.h"

typedef struct JPG_DECODER_TAG
{
    uint32_t	framePitchY;
    uint32_t	framePitchUV;
    uint32_t	frameWidth;
    uint32_t	frameHeight;
    uint32_t	frameBufCount;
	uint32_t    currDisplayFrameBufIndex;
    uint32_t	OutAddrY[2];
    uint32_t	OutAddrU[2];
    uint32_t	OutAddrV[2];
	uint8_t     *DisplayAddrY;
    uint8_t     *DisplayAddrU;
    uint8_t     *DisplayAddrV;
	JPG_COLOR_SPACE     colorFmt;
} JPG_DECODER;

// ite H/W jpg
static HJPG			 *pHJpeg = 0;
static JPG_DECODER    	*gptJPG_DECODER     = NULL;
static uint32_t Jbuf_vram_addr  	= 0;
static uint8_t* Jbuf_sys_addr   	= NULL;
static mblk_t * yuv_msg 			= NULL;
//static int frame_count = 0;

static mblk_t *ms_yuv_msg_alloc()
{
    int       size    = sizeof(YUV_FRAME);
    const int padding = 16;
    mblk_t    *msg    = allocb(size + padding, 0);
    msg->b_wptr += size;
    return msg;
}

static mblk_t *get_as_yuvmsg(YUV_FRAME *orig)
{
    if (!yuv_msg)
    {
        yuv_msg = ms_yuv_msg_alloc();
    }
    memcpy(yuv_msg->b_rptr, orig, sizeof(YUV_FRAME));
    return dupmsg(yuv_msg);
}

static void
_mjpeg_decode_display(
	void)
{
	uint32_t	   frame_width, frame_height, frame_PitchY, frame_PitchUV;
	
	frame_width   = gptJPG_DECODER->frameWidth;
	frame_height  = gptJPG_DECODER->frameHeight;
	frame_PitchY  = gptJPG_DECODER->framePitchY;
	frame_PitchUV = gptJPG_DECODER->framePitchUV;


	if (!Jbuf_sys_addr)
	{
		Jbuf_vram_addr = itpVmemAlignedAlloc(32,(frame_PitchY * frame_height * 4 ) ); //for YUV422
		if(!Jbuf_vram_addr) printf("Jbuf_sys_addr Alloc Buffer Fail!!\n");
		
		Jbuf_sys_addr = (uint8_t*) ithMapVram(Jbuf_vram_addr,(frame_PitchY * frame_height * 4 ) , ITH_VRAM_WRITE);
		gptJPG_DECODER->frameBufCount = 0;
		gptJPG_DECODER->currDisplayFrameBufIndex = 0;
	}

	if(!gptJPG_DECODER->frameBufCount)
	{
		gptJPG_DECODER->OutAddrY[0] = Jbuf_sys_addr;
		gptJPG_DECODER->OutAddrU[0] = gptJPG_DECODER->OutAddrY[0]  + (frame_PitchY * frame_height); 
		gptJPG_DECODER->OutAddrV[0] = gptJPG_DECODER->OutAddrU[0]  + (frame_PitchUV * frame_height);

		gptJPG_DECODER->OutAddrY[1] = gptJPG_DECODER->OutAddrV[0]  + (frame_PitchUV * frame_height);
		gptJPG_DECODER->OutAddrU[1] = gptJPG_DECODER->OutAddrY[1]  + (frame_PitchY * frame_height);
		gptJPG_DECODER->OutAddrV[1] = gptJPG_DECODER->OutAddrU[1]  + (frame_PitchUV * frame_height);
		gptJPG_DECODER->frameBufCount = 2;
	}


	if(gptJPG_DECODER->frameBufCount ==1)
			gptJPG_DECODER->currDisplayFrameBufIndex = 0;

	switch (gptJPG_DECODER->currDisplayFrameBufIndex)
    {
	    case 0:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[0];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[0];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[0];
	        break;

	    case 1:
	        gptJPG_DECODER->DisplayAddrY = gptJPG_DECODER->OutAddrY[1];
	        gptJPG_DECODER->DisplayAddrU = gptJPG_DECODER->OutAddrU[1];
	        gptJPG_DECODER->DisplayAddrV = gptJPG_DECODER->OutAddrV[1];
	        break;
    }

	if(gptJPG_DECODER->currDisplayFrameBufIndex >= 1) 	gptJPG_DECODER->currDisplayFrameBufIndex = 0;
		else gptJPG_DECODER->currDisplayFrameBufIndex++;	


}

static void dec_init(MSFilter *f)
{
#ifndef _MSC_VER
	JPG_ERR            jpgRst     = JPG_ERR_OK;
    JPG_INIT_PARAM     initParam  = {0};

	if (NULL == gptJPG_DECODER)
		gptJPG_DECODER = (JPG_DECODER *)calloc(sizeof(char), sizeof(JPG_DECODER)); // for jpg engine
		
	initParam.codecType 	 = JPG_CODEC_DEC_MJPG;
	initParam.decType		 = JPG_DEC_PRIMARY;
	initParam.outColorSpace  = JPG_COLOR_SPACE_YUV420;

	initParam.dispMode		 = JPG_DISP_CENTER; 

	initParam.width 		 = 320;
	initParam.height		 = 240;

	iteJpg_CreateHandle(&pHJpeg, &initParam, 0);
#endif	
}

static void dec_preprocess(MSFilter *f)
{
	itv_set_pb_mode(1);
}

static void dec_process(MSFilter *f)
{
	YUV_FRAME orig = {0};
	mblk_t  *im;
	while ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
#ifndef _MSC_VER    
		JPG_ERR            jpgRst     = JPG_ERR_OK;
		JPG_RECT	       destRect   = {0};

		JPG_STREAM_INFO inStreamInfo   = {0};
        JPG_STREAM_INFO outStreamInfo  = {0};
        JPG_BUF_INFO    entropyBufInfo = {0};
        JPG_USER_INFO   jpgUserInfo    = {0};
				
        // ------------------------------------
        // set src type
        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jstream.mem[0].pAddr  = im->b_rptr;
        inStreamInfo.jstream.mem[0].length = im->b_wptr - im->b_rptr;
        inStreamInfo.validCompCnt          = 1;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);

        // ------------------------------------
        // parsing Header
        jpgRst = iteJpg_Parsing(pHJpeg, &entropyBufInfo,(void *)&destRect);
		
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }

        // ----------------------------------------
        // get output YUV plan buffer
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
     
		gptJPG_DECODER->frameHeight =  jpgUserInfo.real_height;
		gptJPG_DECODER->frameWidth  =  jpgUserInfo.real_width;
		gptJPG_DECODER->framePitchY  = jpgUserInfo.comp1Pitch;
		gptJPG_DECODER->framePitchUV = jpgUserInfo.comp23Pitch;
		gptJPG_DECODER->colorFmt 	 = jpgUserInfo.colorFormate;
		
        outStreamInfo.streamIOType         = JPG_STREAM_IO_WRITE;
        outStreamInfo.streamType           = JPG_STREAM_MEM;

		{
			_mjpeg_decode_display();

			outStreamInfo.jstream.mem[0].pAddr  = gptJPG_DECODER->DisplayAddrY; // get output buf;
	        outStreamInfo.jstream.mem[0].pitch  = gptJPG_DECODER->framePitchY;
	        outStreamInfo.jstream.mem[0].length = gptJPG_DECODER->framePitchY * gptJPG_DECODER->frameHeight;
	        // U
	        outStreamInfo.jstream.mem[1].pAddr  = gptJPG_DECODER->DisplayAddrU;
	        outStreamInfo.jstream.mem[1].pitch  = gptJPG_DECODER->framePitchUV;
	        outStreamInfo.jstream.mem[1].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight;
	        // V
	        outStreamInfo.jstream.mem[2].pAddr  = gptJPG_DECODER->DisplayAddrV;
	        outStreamInfo.jstream.mem[2].pitch  = gptJPG_DECODER->framePitchUV;
	        outStreamInfo.jstream.mem[2].length = gptJPG_DECODER->framePitchUV * gptJPG_DECODER->frameHeight;
		}
		
		//printf("\n\tY=0x%x, u=0x%x, v=0x%x\n",
		//			outStreamInfo.jstream.mem[0].pAddr,
		//			outStreamInfo.jstream.mem[1].pAddr,
		//			outStreamInfo.jstream.mem[2].pAddr);

        outStreamInfo.validCompCnt         = 3;
        jpgRst                             = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }

        // ------------------------------
        // setup jpg
        jpgRst = iteJpg_Setup(pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }

        // ------------------------------
        // fire H/W jpg
        jpgRst = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }
		
		iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0); 
	   	//printf("\n\tresult = %d\n", jpgUserInfo.status); 

        jpgRst = iteJpg_WaitIdle(pHJpeg, 0);
        if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }
        
		jpgRst = iteJpg_Reset(pHJpeg,0);
		if (jpgRst != JPG_ERR_OK)
        {
            printf(" err (0x%x) !! %s [%d]\n", jpgRst, __FILE__, __LINE__);
        }

		orig.width = gptJPG_DECODER->frameWidth;
		orig.height = gptJPG_DECODER->frameHeight;
		orig.data[0] = outStreamInfo.jstream.mem[0].pAddr;
		orig.data[1] = outStreamInfo.jstream.mem[1].pAddr;
		orig.data[2] = outStreamInfo.jstream.mem[2].pAddr;
		orig.linesize[0] = outStreamInfo.jstream.mem[0].pitch;
		orig.linesize[1] = outStreamInfo.jstream.mem[1].pitch;

		ms_queue_put(f->outputs[0], get_as_yuvmsg(&orig));
#endif
    	freemsg(im);
#if 0		
		frame_count++;
		if(frame_count == 10)
		{
			FILE *fp = fopen("E:/saved_ajb.yuv", "wb");

		    fwrite(outStreamInfo.jstream.mem[0].pAddr, 1, outStreamInfo.jstream.mem[0].length, fp);
			fwrite(outStreamInfo.jstream.mem[1].pAddr, 1, outStreamInfo.jstream.mem[1].length, fp);
			fwrite(outStreamInfo.jstream.mem[2].pAddr, 1, outStreamInfo.jstream.mem[2].length, fp);
			fclose(fp);

			printf("xxxxxxxxxxxxxxxxxx write file end xxxxxxxxxxxxxxxxxxxxxxxxxx\n");
		}
#endif
	}
}

static int dec_postprocess(MSFilter *f)
{
	itv_set_pb_mode(0);
}

static void dec_uninit(MSFilter *f)
{
#ifndef _MSC_VER
	JPG_ERR            result     = JPG_ERR_OK;

    iteJpg_DestroyHandle(&pHJpeg, 0);
	pHJpeg =0;
	if (Jbuf_sys_addr)
    {
        itpVmemFree(Jbuf_vram_addr);
        Jbuf_sys_addr  = NULL;
        Jbuf_vram_addr = 0;
    }
	if (gptJPG_DECODER)
	{
        free(gptJPG_DECODER);
    	gptJPG_DECODER = NULL;
	}
#endif	
}

#ifndef _MSC_VER

MSFilterDesc ms_mjpeg_dec_desc = {
    .id          = MS_MJPEG_DEC_ID,
    .name        = "MSMJPEGDec",
    .text        = "A MJPEG decoder",
    .category    = MS_FILTER_DECODER,
    .enc_fmt     = "MJPEG",
    .ninputs     =                                         1,
    .noutputs    =                                         1,
    //.noutputs=0,
    .init        = dec_init,
    .preprocess  = dec_preprocess,
    .process     = dec_process,
    .postprocess = dec_postprocess,
    .uninit      = dec_uninit
};

#else

MSFilterDesc ms_mjpeg_dec_desc = {
    MS_MJPEG_DEC_ID,
    "MSMJPEGDec",
    "A MJPEG decoder",
    MS_FILTER_DECODER,
    "MJPEG",
    1,
    1,
    dec_init,
    dec_preprocess,
    dec_process,
    dec_postprocess,
    dec_uninit
};

#endif

MS_FILTER_DESC_EXPORT(ms_mjpeg_dec_desc)

