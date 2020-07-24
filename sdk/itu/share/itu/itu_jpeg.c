#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "jpeglib.h"
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

#include "ite/itv.h" //test

#define min(x,y)	(x < y?x:y)
#define max(x,y)	(x > y?x:y)

#if defined(CFG_JPEG_HW_ENABLE)

    #include "jpg/ite_jpg.h"
    #include "isp/mmp_isp.h"
    #define MAX_JPEG_DECODE_SIZE 36000000
    #define JPEG_SOF_MARKER      0xFFC0
    #define JPEG_SOS_MARKER      0xFFDA
    #define JPEG_DHT_MARKER      0xFFC4
    #define JPEG_DRI_MARKER      0xFFDD
    #define JPEG_DQT_MARKER      0xFFDB
    #define JPEG_APP00_MARKER    0xFFE0
    #define JPEG_APP01_MARKER    0xFFE1
    #define JPEG_APP02_MARKER    0xFFE2
    #define JPEG_APP03_MARKER    0xFFE3
    #define JPEG_APP04_MARKER    0xFFE4
    #define JPEG_APP05_MARKER    0xFFE5
    #define JPEG_APP06_MARKER    0xFFE6
    #define JPEG_APP07_MARKER    0xFFE7
    #define JPEG_APP08_MARKER    0xFFE8
    #define JPEG_APP09_MARKER    0xFFE9
    #define JPEG_APP10_MARKER    0xFFEA
    #define JPEG_APP11_MARKER    0xFFEB
    #define JPEG_APP12_MARKER    0xFFEC
    #define JPEG_APP13_MARKER    0xFFED
    #define JPEG_APP14_MARKER    0xFFEE
    #define JPEG_APP15_MARKER    0xFFEF
    #define JPEG_COM_MARKER      0xFFFE

//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum DATA_COLOR_TYPE_TAG
{
    DATA_COLOR_YUV444,
    DATA_COLOR_YUV422,
    DATA_COLOR_YUV422R,
    DATA_COLOR_YUV420,
    DATA_COLOR_ARGB8888,
    DATA_COLOR_ARGB4444,
    DATA_COLOR_RGB565,
    DATA_COLOR_NV12,
    DATA_COLOR_NV21,

    DATA_COLOR_CNT,
} DATA_COLOR_TYPE;

typedef enum _ISP_ACT_CMD_TAG
{
    ISP_ACT_CMD_IDLE = 0,
    ISP_ACT_CMD_INIT,
    ISP_ACT_CMD_TERMINATE,
    ISP_ACT_CMD_PROC,
} ISP_ACT_CMD;

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _BASE_RECT_TAG
{
    int x;
    int y;
    int w;
    int h;
} BASE_RECT;

typedef struct _CLIP_WND_INFO_TAG
{
    int       bClipEnable;
    int       bClipOutside;
    int       clipWndId;
    BASE_RECT clipRect;
} CLIP_WND_INFO;

    #if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)
typedef struct tagFrameFuncInfo {
    void             *vramAddr;
    MMP_UINT         startX;
    MMP_UINT         startY;
    MMP_UINT         width;
    MMP_UINT         height;
    MMP_UINT         pitch;
    MMP_UINT32       linebytes;
    MMP_UINT32       bitstreambytes;
    MMP_UINT         colorKeyR;
    MMP_UINT         colorKeyG;
    MMP_UINT         colorKeyB;
    MMP_BOOL         EnAlphaBlend;
    MMP_UINT         constantAlpha;
    MMP_PIXEL_FORMAT format;
    MMP_UINT         uiBufferIndex;
} FrameFuncInfo; //For Jpeg With FrameFunction Use.
    #endif

//=============================================================================
//				  Global Data Definition
//=============================================================================
static ISP_DEVICE gIspDev;
extern ITUSurface *VideoSurf[3];

//=============================================================================
//				  Private Function Definition
//=============================================================================
static void argb8888toyuv420(char *yuv_dst, char *argb_src, int width, int height)
{
    uint8_t*yuvBuf = yuv_dst;
	int nWidth = width;  
	int nHeight = height;  

	int i, j;  
	uint8_t *bufY = yuvBuf;  
	uint8_t *bufU = yuvBuf+nWidth*nHeight;	
	uint8_t *bufV = bufU+(nWidth*nHeight*1/4);	

	uint8_t *bufRGB;  
	unsigned char y,u,v,r,g,b;	
	if(NULL==argb_src)  
    {
		return;  
	}  
	for(j = 0; j < nHeight; j++)  
        {
		bufRGB = argb_src + nWidth * 4 * j; 
		for(i = 0; i < nWidth; i++)  
            {
			b=*(bufRGB++);	
			g=*(bufRGB++);	
			r=*(bufRGB++);
			bufRGB++; //alpha
			y=(unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;//16  
			v=(unsigned char)((-38 * r -74 * g + 112 * b + 128) >> 8) + 128;//128  
			u=(unsigned char)((112 * r -94 * g -18 * b + 128) >> 8) + 128;	
			*(bufY++) = max(0, min(y, 255));  

			if(j%2 == 0 && i%2 == 0)  
			{  
				if(u > 255)  
				{  
					u = 255;  
				}  
				if(u < 0)  
				{  
					u = 0;	
            }
				*(bufU++) = u;	
        }
        else
        {
				if(i%2 == 0)  
            {
					if(v > 255)  
					{  
						v = 255;  
					}  
					if(v < 0)  
					{  
						v = 0;	
					}  
					*(bufV++) = v;	
				}  
            }
        }
    }
}

static void rgb888toyuv420(char *yuv_dst, char *rgb_src, int width, int height)
{
	uint8_t*yuvBuf = yuv_dst;
	int nWidth = width;  
	int nHeight = height;  

	int i, j;  
	uint8_t *bufY = yuvBuf;  
	uint8_t *bufU = yuvBuf+nWidth*nHeight;	
	uint8_t *bufV = bufU+(nWidth*nHeight*1/4);	

	uint8_t *bufRGB;  
	unsigned char y,u,v,r,g,b;	
	if(NULL==rgb_src)  
    {
		return;  
	}  
	for(j = 0; j < nHeight; j++)  
        {
		bufRGB = rgb_src + nWidth * 3 * j; 
		for(i = 0; i < nWidth; i++)  
            {
			b=*(bufRGB++);	
			g=*(bufRGB++);	
			r=*(bufRGB++);	
			y=(unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;//16  
			v=(unsigned char)((-38 * r -74 * g + 112 * b + 128) >> 8) + 128;//128  
			u=(unsigned char)((112 * r -94 * g -18 * b + 128) >> 8) + 128;	
			*(bufY++) = max(0, min(y, 255));  

			if(j%2 == 0 && i%2 == 0)  
			{  
				if(u > 255)  
				{  
					u = 255;  
				}  
				if(u < 0)  
				{  
					u = 0;	
            }
				*(bufU++) = u;	
        }
        else
        {
				if(i%2 == 0)  
            {
					if(v > 255)  
					{  
						v = 255;  
					}  
					if(v < 0)  
					{  
						v = 0;	
					}  
					*(bufV++) = v;	
				}  
            }
        }
    }
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
void
set_isp_colorTrans(
    uint8_t         *srcAddr_rgby,
    uint8_t         *srcAddr_u,
    uint8_t         *srcAddr_v,
    DATA_COLOR_TYPE colorType,
    CLIP_WND_INFO   *clipInfo,
    BASE_RECT       *srcRect,
    BASE_RECT       *destRect,
    int             imgWidth,
    int             imgHeight,
    int             M2dPitch,
    uint16_t        *dest)
{
    int                 result = 0;
    int                 width = 0, height = 0;
    MMP_ISP_OUTPUT_INFO outInfo     = {0};
    MMP_ISP_SHARE       ispInput    = {0};
    #if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)
    MMP_ISP_CORE_INFO   ISPCOREINFO = { 0 };
    #endif

    //ispInput.width        = (imgWidth >> 3) << 3;  //srcRect->w;
    //ispInput.height       = (imgHeight >> 3) << 3; //srcRect->h;
    ispInput.width        = imgWidth;
    ispInput.height       = imgHeight;
    ispInput.isAdobe_CMYK = 0;

    switch (colorType)
    {
    case DATA_COLOR_YUV444:
    case DATA_COLOR_YUV422:
    case DATA_COLOR_YUV422R:
    case DATA_COLOR_YUV420:
        ispInput.addrY = (uint32_t)srcAddr_rgby;
        ispInput.addrU = (uint32_t)srcAddr_u;
        ispInput.addrV = (uint32_t)srcAddr_v;
        switch (colorType)
        {
        case DATA_COLOR_YUV444:
            ispInput.format  = MMP_ISP_IN_YUV444;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = srcRect->w;
            break;
        case DATA_COLOR_YUV422:
            ispInput.format  = MMP_ISP_IN_YUV422;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = (srcRect->w >> 1);
            break;
        case DATA_COLOR_YUV422R:
            ispInput.format  = MMP_ISP_IN_YUV422R;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = srcRect->w;
            break;
        case DATA_COLOR_YUV420:
            ispInput.format  = MMP_ISP_IN_YUV420;
            ispInput.pitchY  = srcRect->w;
            ispInput.pitchUv = (srcRect->w >> 1);
            break;
        }
        break;

    case DATA_COLOR_ARGB8888:
    case DATA_COLOR_ARGB4444:
        ispInput.addrY        = (uint32_t)dest;
        ispInput.format       = MMP_ISP_IN_RGB565;
        ispInput.pitchY       = (uint16_t)M2dPitch;
        ispInput.width        = width;
        ispInput.height       = height;
        ispInput.isAdobe_CMYK = 0;
        break;

    case DATA_COLOR_NV12:
    case DATA_COLOR_NV21:
        ispInput.addrY   = (uint32_t)srcAddr_rgby;
        ispInput.addrU   = (uint32_t)srcAddr_u;
        ispInput.pitchY  = srcRect->w;
        ispInput.pitchUv = srcRect->w;
        ispInput.format  = (colorType == DATA_COLOR_NV12) ? MMP_ISP_IN_NV12 : MMP_ISP_IN_NV21;
        break;

    case DATA_COLOR_RGB565:
        return;
    }

    #if (CFG_CHIP_FAMILY == 970 || CFG_CHIP_FAMILY == 9860)
    //mmpIspTerminate(&gIspDev);
    width  = destRect->w;     //dispWidth;
    height = destRect->h;     //dispHeight;

    result = mmpIspInitialize(&gIspDev, MMP_ISP_CORE_0);

    if (result)
        printf("mmpIspInitialize() error (0x%x) !!\n", result);

    // for VP1
    ISPCOREINFO.EnPreview   = false;
    ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;
    //end of for VP1.

    result                  = mmpIspSetCore(gIspDev, &ISPCOREINFO);
    if (result)
        printf("mmpIspSetCore() error (0x%x) !!\n", result);

    result = mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);
    if (result)
        printf("mmpIspSetMode() error (0x%x) !! \n", result);

    outInfo.startX   = 0;
    outInfo.startY   = 0;
    outInfo.width    = width;
    outInfo.height   = height;
    outInfo.addrRGB  = (uint32_t)dest;
    outInfo.pitchRGB = (uint16_t)M2dPitch;
    outInfo.format   = MMP_ISP_OUT_DITHER565A;

    mmpIspSetOutputWindow(gIspDev, &outInfo);
    mmpIspSetVideoWindow(gIspDev, 0, 0, outInfo.width, outInfo.height);

    if ((colorType == DATA_COLOR_ARGB8888) || (colorType == DATA_COLOR_ARGB4444))
    {
        int IFrm0Mode = 0;
        if (colorType == DATA_COLOR_ARGB8888)
            IFrm0Mode = 2;
        else if (colorType == DATA_COLOR_ARGB4444)
            IFrm0Mode = 1;

        mmpIspEnable(gIspDev, MMP_ISP_FRAME_FUNCTION_0);
        FrameFuncInfo FF0 = {0};
        FF0.vramAddr      = srcAddr_rgby;
        FF0.startX        = 0;
        FF0.startY        = 0;
        FF0.width         = srcRect->w;
        FF0.height        = srcRect->h;
        FF0.colorKeyR     = 100;
        FF0.colorKeyG     = 100;
        FF0.colorKeyB     = 100;
        FF0.EnAlphaBlend  = 1;
        FF0.constantAlpha = 0;
        if (IFrm0Mode == 0)
        {
            FF0.format = MMP_PIXEL_FORMAT_RGB565;
            FF0.pitch  = srcRect->w;
        }
        else if (IFrm0Mode == 1)
        {
            FF0.format = MMP_PIXEL_FORMAT_ARGB4444;
            FF0.pitch  = srcRect->w * 2;
        }
        else if (IFrm0Mode == 2)
        {
            FF0.format = MMP_PIXEL_FORMAT_ARGB8888;
            FF0.pitch  = srcRect->w * 4;
        }

        result = mmpIspSetFrameFunction(
            gIspDev,
            MMP_ISP_FRAME_FUNCTION_0,
            FF0.vramAddr,
            FF0.startX,
            FF0.startY,
            FF0.width,
            FF0.height,
            FF0.pitch,
            FF0.colorKeyR,
            FF0.colorKeyG,
            FF0.colorKeyB,
            FF0.EnAlphaBlend,
            FF0.constantAlpha,
            FF0.format,
            FF0.uiBufferIndex);
        if (result)
            printf("mmpIspSetFrameFunction() error (0x%x) !!\n", result);
    }

    #ifdef CFG_LCD_PQ_TUNING
        pthread_mutex_lock(&ISP_CORE_0_MUTEX);
    #endif

    result = mmpIspPlayImageProcess(gIspDev, &ispInput);
    if (result)
        printf("mmpIspPlayImageProcess() error (0x%x) !!\n", result);

    result = mmpIspWaitEngineIdle(gIspDev);
    if (result)
        printf("mmpIspWaitEngineIdle() error (0x%x) !!\n", result);

    #ifdef CFG_LCD_PQ_TUNING
        pthread_mutex_unlock(&ISP_CORE_0_MUTEX);
    #endif

	mmpIspTerminate(&gIspDev);
    #endif
}

int *ituJpegLoadEx(int width, int height, uint8_t *data, int size)
{
    ITUSurface        *surf = NULL;
    uint16_t          *dest = NULL;
    HJPG              *pHJpeg = 0;
    JPG_INIT_PARAM    initParam = {0};
    JPG_STREAM_INFO   inStreamInfo = {0};
    JPG_STREAM_INFO   outStreamInfo = {0};
    JPG_BUF_INFO      entropyBufInfo = {0};
    JPG_USER_INFO     jpgUserInfo = {0};
    JPG_ERR           result = JPG_ERR_OK;
    ITUColor          black = { 0, 0, 0, 0 };
    JPG_RECT          destRect = {0};
    CLIP_WND_INFO     clipInfo = {0};
    BASE_RECT         srcRect = {0};
    DATA_COLOR_TYPE   colorType = 0;
    uint32_t          real_width = 0, real_height = 0, real_height_ForTile = 0;
    uint32_t          imgWidth = 0, imgHeight = 0;
    uint32_t          SmallPicWidth = 1280, SmallPicHeight = 720;
    uint8_t           *pY = 0, *pStart = 0, *pCur = 0, *pEnd = 0;
    uint32_t          CurCount = 0, MarkerType = 0, GetMarkerLength = 0;
    uint8_t           *dbuf     = NULL;
    ITV_DBUF_PROPERTY dbufprop  = {0};
    int               new_index = 0;

    //malloc_stats();

    if (data[0] != 0xFF || data[1] != 0xD8)
    {
        printf("jpeg read stream fail,data[0]=0x%x,data[1]=0x%x\n", data[0], data[1]);
        return 0;
    }
    else
    {
        pStart = pCur = data;
        pEnd   = pCur + size;

        while ((pCur < pEnd))
        {
            MarkerType = (*(pCur) << 8 | *(pCur + 1));
            switch (MarkerType)
            {
            case JPEG_SOF_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                CurCount        = pCur - pStart;
                CurCount       += 3; //pass 3 byte
                imgHeight       = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 2;
                imgWidth        = pStart[CurCount] << 8 | pStart[CurCount + 1];
                pCur           += GetMarkerLength;
                break;
            case JPEG_SOS_MARKER:
            case JPEG_DHT_MARKER:
            case JPEG_DRI_MARKER:
            case JPEG_DQT_MARKER:
            case JPEG_APP00_MARKER:
            case JPEG_APP01_MARKER:
            case JPEG_APP02_MARKER:
            case JPEG_APP03_MARKER:
            case JPEG_APP04_MARKER:
            case JPEG_APP05_MARKER:
            case JPEG_APP06_MARKER:
            case JPEG_APP07_MARKER:
            case JPEG_APP08_MARKER:
            case JPEG_APP09_MARKER:
            case JPEG_APP10_MARKER:
            case JPEG_APP11_MARKER:
            case JPEG_APP12_MARKER:
            case JPEG_APP13_MARKER:
            case JPEG_APP14_MARKER:
            case JPEG_APP15_MARKER:
            case JPEG_COM_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                pCur           += GetMarkerLength;
                break;
            default:
                pCur++;
                break;
            }
        }
        pCur = pStart;
    }

    if (width == 0 || height == 0)
    {
        width  = (int)imgWidth;
        height = (int)imgHeight;
    }

    new_index = itv_get_vidSurf_index();
    while (new_index == -1)
    {
        //printf("wait to get new_index!\n");
        usleep(1000);
        new_index = itv_get_vidSurf_index();
    }

    switch (new_index)
    {
    case  0:
        new_index = 1;
        break;

    case  1:
    case -2:
        new_index = 0;
        break;
    }

    surf = VideoSurf[new_index];
    dest = (uint8_t *)ituLockSurface(surf, 0, 55, width, height);
    ituColorFill(surf, 0, 55, width, height, &black);

    if ((imgWidth * imgHeight >= MAX_JPEG_DECODE_SIZE) || imgWidth >= 4096 || imgHeight >= 4096)
    {
        printf("JPG not support this format\n");
        ituUnlockSurface(surf);
        return 0;
    }
    else if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        initParam.codecType                = JPG_CODEC_DEC_JPG_CMD;
        initParam.decType                  = JPG_DEC_PRIMARY;
        initParam.dispMode                 = JPG_DISP_CENTER;        //JPG_DISP_FIT;
        initParam.outColorSpace            = JPG_COLOR_SPACE_YUV420; //just a initial value. not really the picture`s  format.JPG CMD mode back will modify it.
        initParam.width                    = width;
        initParam.height                   = height;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        result                             = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("memory mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);

        real_width  = jpgUserInfo.real_width;
        real_height = jpgUserInfo.real_height;

        if (initParam.outColorSpace == JPG_COLOR_SPACE_RGB565)
        {
            outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
            outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
            outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
            outStreamInfo.validCompCnt          = 1;
        }
        else
        {
            pY = malloc(real_width * real_height * 3);
            //memset(pY, 0x0, real_width * real_height * 3);

            //Y
            outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY;
            outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
            outStreamInfo.jstream.mem[0].length = real_width * real_height;
            // U
            outStreamInfo.jstream.mem[1].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
            outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
            outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
            // V
            outStreamInfo.jstream.mem[2].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
            outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
            outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
            outStreamInfo.validCompCnt          = 3;
        }
    }
    else
    {
        initParam.codecType                = JPG_CODEC_DEC_JPG;
        initParam.decType                  = JPG_DEC_PRIMARY;
        initParam.outColorSpace            = JPG_COLOR_SPACE_RGB565;
        initParam.width                    = width;
        initParam.height                   = height;
        initParam.dispMode                 = JPG_DISP_CENTER;//JPG_DISP_FIT;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;

        result                             = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        if (result != JPG_ERR_OK)
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            goto end;
        }
        result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        /*
           printf("handshake mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);
         */
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
        outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
        outStreamInfo.validCompCnt          = 1;
    }
    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    result                     = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    if (pY)
    {
        ithFlushDCacheRange((void *)pY, real_width * real_height * 3);
        ithFlushMemBuffer();
    }

    result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        clipInfo.bClipEnable = 0;

        srcRect.w            = real_width;
        srcRect.h            = real_height;

        switch (jpgUserInfo.colorFormate)
        {
        case JPG_COLOR_SPACE_YUV411:    colorType  = DATA_COLOR_YUV422;  break;
        case JPG_COLOR_SPACE_YUV444:    colorType  = DATA_COLOR_YUV444;  break;
        case JPG_COLOR_SPACE_YUV422:    colorType  = DATA_COLOR_YUV422;  break;
        case JPG_COLOR_SPACE_YUV420:    colorType  = DATA_COLOR_YUV420;  break;
        case JPG_COLOR_SPACE_YUV422R:   colorType  = DATA_COLOR_YUV422R; break;
        case JPG_COLOR_SPACE_RGB565:    colorType  = DATA_COLOR_RGB565;  break;
        }

        set_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect,
            &destRect,
            imgWidth,
            imgHeight,
            surf->pitch,
            dest);
    }

    while ((dbuf = itv_get_dbuf_anchor()) == NULL)
    {
        //printf("wait to get dbuf!\n");
        usleep(1000);
    }

    {
        // ------------------------------------
        // Just through itv driver to Flip LCD ,both handshake mode or command trigger mode run this setting (when command trigger ,MMP_ISP_IN_RGB565 is not really format.).
        dbufprop.src_w    = 0;
        dbufprop.src_h    = 0;
        dbufprop.pitch_y  = 0;
        dbufprop.pitch_uv = 0;
        dbufprop.format   = MMP_ISP_IN_RGB565;
        dbufprop.ya       = 0;
        dbufprop.ua       = 0;
        dbufprop.va       = 0;
        dbufprop.bidx     = 0;
        //printf("dbufprop.ya=0x%x,dbufprop.ua=0x%x,dbufprop.va=0x%x,dbufprop.src_w=%d,dbufprop.src_h=%d,dbufprop.pitch_y=%d,dbufprop.pitch_uv=%d,dbufprop.format=%d\n",dbufprop.ya,dbufprop.ua,dbufprop.va,dbufprop.src_w,dbufprop.src_h,dbufprop.pitch_y,dbufprop.pitch_uv,dbufprop.format);
        itv_update_dbuf_anchor(&dbufprop);
    }

    if (pY) free(pY);

    //printf("jpeg decode end\n");
    ituUnlockSurface(surf);
    return 1;

end:
    iteJpg_DestroyHandle(&pHJpeg, 0);
    ituUnlockSurface(surf);
    return 1;
}

int *ituJpegLoadFileEx(int width, int height, char *filepath)
{
    int         result;
    FILE        *f    = NULL;
    int         size  = 0;
    uint8_t     *data = NULL;
    struct stat sb;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    if (fstat(fileno(f), &sb) == -1)
        goto end;

    size = sb.st_size;

    data = malloc(size);
    if (!data)
        goto end;

    size   = fread(data, 1, size, f);
    result = ituJpegLoadEx(width, height, data, size);
end:
    free(data);

    if (f)
        fclose(f);

    return result;
}

ITUSurface *ituJpegLoad(int width, int height, uint8_t *data, int size, unsigned int flags)
{
    ITUSurface      *surf = NULL;
    uint16_t        *dest = NULL;
    HJPG            *pHJpeg = 0;
    JPG_INIT_PARAM  initParam = {0};
    JPG_STREAM_INFO inStreamInfo = {0};
    JPG_STREAM_INFO outStreamInfo = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo = {0};
    JPG_ERR         result = JPG_ERR_OK;
    ITUColor        black = { 0, 0, 0, 0 };
    uint8_t         *pY = 0;
    JPG_RECT        destRect = {0};
    CLIP_WND_INFO   clipInfo = {0};
    BASE_RECT       srcRect = {0};
    DATA_COLOR_TYPE colorType = 0;
    uint32_t        real_width = 0, real_height = 0, real_height_ForTile = 0;
    uint32_t        imgWidth = 0, imgHeight = 0;
    uint32_t        decWidth = 0, decHeight = 0;
    uint32_t        H_Samp = 0, V_Samp = 0, widthUnit = 0, heightUnit = 0;
    uint32_t        SmallPicWidth = 1280, SmallPicHeight = 720;
    uint8_t         *pStart = 0, *pCur = 0, *pEnd = 0;
    uint32_t        CurCount = 0, MarkerType = 0, GetMarkerLength = 0;

    //malloc_stats();
    if (data[0] != 0xFF || data[1] != 0xD8)
    {
        printf("jpeg read stream fail,data[0]=0x%x,data[1]=0x%x\n", data[0], data[1]);
        return NULL; //return NULL to tell AP, if JPEG reading fail!.
    }
    else
    {
        pStart = pCur = data;
        pEnd   = pCur + size;

        while ((pCur < pEnd))
        {
            MarkerType = (*(pCur) << 8 | *(pCur + 1));
            switch (MarkerType)
            {
            case JPEG_SOF_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                CurCount        = pCur - pStart;
                CurCount       += 3; //pass 3 byte
                imgHeight       = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 2;
                imgWidth        = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 4;
                H_Samp          = pStart[CurCount] >> 4;
                V_Samp          = pStart[CurCount] & 0xF;
                pCur           += GetMarkerLength;
                break;
            case JPEG_SOS_MARKER:
            case JPEG_DHT_MARKER:
            case JPEG_DRI_MARKER:
            case JPEG_DQT_MARKER:
            case JPEG_APP00_MARKER:
            case JPEG_APP01_MARKER:
            case JPEG_APP02_MARKER:
            case JPEG_APP03_MARKER:
            case JPEG_APP04_MARKER:
            case JPEG_APP05_MARKER:
            case JPEG_APP06_MARKER:
            case JPEG_APP07_MARKER:
            case JPEG_APP08_MARKER:
            case JPEG_APP09_MARKER:
            case JPEG_APP10_MARKER:
            case JPEG_APP11_MARKER:
            case JPEG_APP12_MARKER:
            case JPEG_APP13_MARKER:
            case JPEG_APP14_MARKER:
            case JPEG_APP15_MARKER:
            case JPEG_COM_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                pCur           += GetMarkerLength;
                break;
            default:
                pCur++;
                break;
            }
        }
        pCur = pStart;
    }

    #if 1
    widthUnit  = H_Samp << 3;
    heightUnit = V_Samp << 3;
    decWidth   = (imgWidth + (widthUnit - 1)) & ~(widthUnit - 1);
    decHeight  = (imgHeight + (heightUnit - 1)) & ~(heightUnit - 1);
    #endif
    if (width == 0 || height == 0)
    {
        width  = (int)decWidth;
        height = (int)decHeight;
    }

    surf = ituCreateSurface(width, height, 0, ITU_RGB565, NULL, 0);
    if (!surf)
    {
        printf("Jpeg Create Surface fail !!\n");
        return NULL;
    }
    dest = (uint16_t *)ituLockSurface(surf, 0, 0, width, height);
    ituColorFill(surf, 0, 0, width, height, &black);

    if ((imgWidth * imgHeight >= MAX_JPEG_DECODE_SIZE) || imgWidth >= 4096 || imgHeight >= 4096)
    {
        printf("JPG not support this format\n");
        ituUnlockSurface(surf);
        ituDestroySurface(surf);
        return NULL;
    }
    else if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        initParam.codecType = JPG_CODEC_DEC_JPG_CMD;
        initParam.decType   = JPG_DEC_PRIMARY;

        if (flags == ITU_FIT_TO_RECT)
            initParam.dispMode = JPG_DISP_FIT;
        else if (flags == ITU_CUT_BY_RECT)
            initParam.dispMode = JPG_DISP_CUT_BY_RECT;
        else
            initParam.dispMode = JPG_DISP_CENTER;

        if (surf->width == imgWidth && surf->height == imgHeight)
            initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;
        else
            initParam.outColorSpace = JPG_COLOR_SPACE_YUV420;

        initParam.width                    = width;
        initParam.height                   = height;
        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;
        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        result                             = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);

        /*
           printf("memory mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);
         */

        real_width  = jpgUserInfo.real_width;
        real_height = jpgUserInfo.real_height;

        if (initParam.outColorSpace == JPG_COLOR_SPACE_RGB565)
        {
            outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
            outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
            outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
            outStreamInfo.validCompCnt          = 1;
        }
        else
        {
            pY = malloc(real_width * real_height * 3);
            //memset(pY, 0x0, real_width * real_height * 3);

            //Y
            outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY;
            outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch;
            outStreamInfo.jstream.mem[0].length = real_width * real_height;
            // U
            outStreamInfo.jstream.mem[1].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
            outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
            outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
            // V
            outStreamInfo.jstream.mem[2].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
            outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
            outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
            outStreamInfo.validCompCnt          = 3;
        }
    }
    else
    {
        initParam.codecType     = JPG_CODEC_DEC_JPG;
        initParam.decType       = JPG_DEC_PRIMARY;
        initParam.outColorSpace = JPG_COLOR_SPACE_RGB565;
        initParam.width         = width;
        initParam.height        = height;

        if (flags == ITU_FIT_TO_RECT)
            initParam.dispMode = JPG_DISP_FIT;
        else if (flags == ITU_CUT_BY_RECT)
            initParam.dispMode = JPG_DISP_CUT_BY_RECT;
        else
            initParam.dispMode = JPG_DISP_CENTER;

        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

        inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
        inStreamInfo.streamType            = JPG_STREAM_MEM;
        inStreamInfo.jpg_reset_stream_info = 0;

        inStreamInfo.validCompCnt          = 1;
        inStreamInfo.jstream.mem[0].pAddr  = data;
        inStreamInfo.jstream.mem[0].length = size;

        result                             = iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);
        if (result != JPG_ERR_OK)
        {
            printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
            goto end;
        }
        result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
        if (result == JPG_ERR_JPROG_STREAM)
        {
            printf("JPG not support this format\n");
            goto end;
        }
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        /*
           printf("handshake mode  (%d, %d) %dx%d, dispMode=%d\r\n",
               jpgUserInfo.jpgRect.x, jpgUserInfo.jpgRect.y,
               jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
               initParam.dispMode);
         */
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)dest;            // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = surf->pitch;
        outStreamInfo.jstream.mem[0].length = surf->pitch * surf->height; //outStreamInfo.jstream.mem[0].pitch * jpgUserInfo.jpgRect.h;
        outStreamInfo.validCompCnt          = 1;
    }
    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    result                     = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    if (pY)
    {
        ithFlushDCacheRange((void *)pY, real_width * real_height * 3);
        ithFlushMemBuffer();
    }

    result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    if ((imgWidth * imgHeight) <= (SmallPicWidth * SmallPicHeight))
    {
        clipInfo.bClipEnable = 0;

        srcRect.w            = real_width;
        srcRect.h            = real_height;

        switch (jpgUserInfo.colorFormate)
        {
        case JPG_COLOR_SPACE_YUV411:    colorType  = DATA_COLOR_YUV422;  break;
        case JPG_COLOR_SPACE_YUV444:    colorType  = DATA_COLOR_YUV444;  break;
        case JPG_COLOR_SPACE_YUV422:    colorType  = DATA_COLOR_YUV422;  break;
        case JPG_COLOR_SPACE_YUV420:    colorType  = DATA_COLOR_YUV420;  break;
        case JPG_COLOR_SPACE_YUV422R:   colorType  = DATA_COLOR_YUV422R; break;
        case JPG_COLOR_SPACE_RGB565:    colorType  = DATA_COLOR_RGB565;  break;
        }

        set_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect,
            &destRect,
            imgWidth,
            imgHeight,
            surf->pitch,
            dest);
    }
    if (pY) free(pY);

    //printf("jpeg decode end\n");
    ituUnlockSurface(surf);
    return surf;

end:
    iteJpg_DestroyHandle(&pHJpeg, 0);
    ituUnlockSurface(surf);
    ituDestroySurface(surf);
    return NULL;
}

ITUSurface *ituJpegAlphaLoad(int width, int height, uint8_t *alpha, uint8_t *data, int size)
{
    ITUSurface      *surf = NULL;
    uint16_t        *dest = NULL;
    HJPG            *pHJpeg = 0;
    JPG_INIT_PARAM  initParam = {0};
    JPG_STREAM_INFO inStreamInfo = {0};
    JPG_STREAM_INFO outStreamInfo = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo = {0};
    JPG_ERR         result = JPG_ERR_OK;
    ITUColor        black = { 0, 0, 0, 0 };
    uint8_t         *pY = 0;
    JPG_RECT        destRect = {0};
    CLIP_WND_INFO   clipInfo = {0};
    BASE_RECT       srcRect = {0};
    DATA_COLOR_TYPE colorType = 0;
    uint32_t        real_width = 0, real_height = 0, real_height_ForTile = 0;
    uint32_t        imgWidth = 0, imgHeight = 0;
    uint32_t        SmallPicWidth = 800, SmallPicHeight = 480;
    uint8_t         *pStart = 0, *pCur = 0, *pEnd = 0;
    uint32_t        CurCount = 0, MarkerType = 0, GetMarkerLength = 0;
    uint8_t         *WriteBuf     = NULL;
    uint8_t         *mappedSysRam = NULL;

    //malloc_stats();
    if (data[0] != 0xFF || data[1] != 0xD8)
    {
        printf("jpeg read stream fail,data[0]=0x%x,data[1]=0x%x\n", data[0], data[1]);
        return NULL; //return NULL to tell AP, if JPEG reading fail!.
    }
    else
    {
        pStart = pCur = data;
        pEnd   = pCur + size;

        while ((pCur < pEnd))
        {
            MarkerType = (*(pCur) << 8 | *(pCur + 1));
            switch (MarkerType)
            {
            case JPEG_SOF_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                CurCount        = pCur - pStart;
                CurCount       += 3; //pass 3 byte
                imgHeight       = pStart[CurCount] << 8 | pStart[CurCount + 1];
                CurCount       += 2;
                imgWidth        = pStart[CurCount] << 8 | pStart[CurCount + 1];
                pCur           += GetMarkerLength;
                break;
            case JPEG_SOS_MARKER:
            case JPEG_DHT_MARKER:
            case JPEG_DRI_MARKER:
            case JPEG_DQT_MARKER:
            case JPEG_APP00_MARKER:
            case JPEG_APP01_MARKER:
            case JPEG_APP02_MARKER:
            case JPEG_APP03_MARKER:
            case JPEG_APP04_MARKER:
            case JPEG_APP05_MARKER:
            case JPEG_APP06_MARKER:
            case JPEG_APP07_MARKER:
            case JPEG_APP08_MARKER:
            case JPEG_APP09_MARKER:
            case JPEG_APP10_MARKER:
            case JPEG_APP11_MARKER:
            case JPEG_APP12_MARKER:
            case JPEG_APP13_MARKER:
            case JPEG_APP14_MARKER:
            case JPEG_APP15_MARKER:
            case JPEG_COM_MARKER:
                pCur           += 2;
                GetMarkerLength = (*(pCur) << 8 | *(pCur + 1));
                pCur           += GetMarkerLength;
                break;
            default:
                pCur++;
                break;
            }
        }
        pCur = pStart;
    }

    if (width == 0 || height == 0)
    {
        width  = (int)imgWidth;
        height = (int)imgHeight;
    }

    surf = ituCreateSurface(width, height, 0, ITU_ARGB8888, NULL, 0);
    if (!surf)
    {
        printf("Jpeg Create Surface fail !!\n");
        return NULL;
    }
    dest = (uint16_t *)ituLockSurface(surf, 0, 0, width, height);

    if ((imgWidth * imgHeight >= MAX_JPEG_DECODE_SIZE) || imgWidth >= 4096 || imgHeight >= 4096)
    {
        printf("JPG not support this format\n");
        ituUnlockSurface(surf);
        ituDestroySurface(surf);
        return NULL;
    }
    else
    {
        initParam.codecType     = JPG_CODEC_DEC_JPG_CMD;    //JPG_CODEC_DEC_JPG;
        initParam.decType       = JPG_DEC_PRIMARY;          //JPG_DEC_SMALL_THUMB; //JPG_DEC_PRIMARY;
        initParam.outColorSpace = JPG_COLOR_SPACE_ARGB8888; //JPG_COLOR_SPACE_ARGB4444;//JPG_COLOR_SPACE_ARGB8888;
        initParam.width         = width;
        initParam.height        = height;
        initParam.dispMode      = JPG_DISP_CENTER;
        if (initParam.outColorSpace == JPG_COLOR_SPACE_ARGB4444 || initParam.outColorSpace == JPG_COLOR_SPACE_ARGB8888)
        {
            initParam.alphaPlane.bEnConstAlpha = false;
            if (initParam.alphaPlane.bEnConstAlpha)
            {
                initParam.alphaPlane.ConstAlpha = 128; //0 ~ 255
            }
            else
            {
                initParam.alphaPlane.AlphaPlaneAddr  = alpha; //alpha plane address
                initParam.alphaPlane.AlphaPlanePitch = width;
            }
        }
    }

    iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType          = JPG_STREAM_IO_READ;
    inStreamInfo.streamType            = JPG_STREAM_MEM;
    inStreamInfo.jstream.mem[0].pAddr  = data;
    inStreamInfo.jstream.mem[0].length = size;
    inStreamInfo.validCompCnt          = 1;

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);

    result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
    if (result == JPG_ERR_JPROG_STREAM)
    {
        printf("JPG not support this format\n");
        goto end;
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    printf("  disp(%ux%u), dispMode=%d, real(%ux%u), img(%ux%u), slice=%u, pitch(%u, %u)\r\n",
           jpgUserInfo.jpgRect.w, jpgUserInfo.jpgRect.h,
           initParam.dispMode,
           jpgUserInfo.real_width, jpgUserInfo.real_height,
           jpgUserInfo.imgWidth, jpgUserInfo.imgHeight,
           jpgUserInfo.slice_num,
           jpgUserInfo.comp1Pitch, jpgUserInfo.comp23Pitch);

    real_width                 = jpgUserInfo.real_width;
    real_height                = jpgUserInfo.real_height;

    outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    pY                         = malloc(real_width * real_height * 4); //forARGB8888 format.
    //memset(pY, 0x0, real_width * real_height * 4);

    // Y
    switch (initParam.outColorSpace)
    {
    case JPG_COLOR_SPACE_ARGB4444:
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY;        // get output buf;
        outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch * 2;
        outStreamInfo.jstream.mem[0].length = real_width * real_height * 2;
        break;

    case JPG_COLOR_SPACE_ARGB8888:
        outStreamInfo.jstream.mem[0].pAddr  = (uint8_t *)pY;
        outStreamInfo.jstream.mem[0].pitch  = jpgUserInfo.comp1Pitch * 4;
        outStreamInfo.jstream.mem[0].length = real_width * real_height * 4;
        break;
    }

    // U
    outStreamInfo.jstream.mem[1].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[0].pAddr + outStreamInfo.jstream.mem[0].length);
    outStreamInfo.jstream.mem[1].pitch  = jpgUserInfo.comp23Pitch;
    outStreamInfo.jstream.mem[1].length = outStreamInfo.jstream.mem[1].pitch * real_height;
    // V
    outStreamInfo.jstream.mem[2].pAddr  = (uint8_t *)(outStreamInfo.jstream.mem[1].pAddr + outStreamInfo.jstream.mem[1].length);
    outStreamInfo.jstream.mem[2].pitch  = jpgUserInfo.comp23Pitch;
    outStreamInfo.jstream.mem[2].length = outStreamInfo.jstream.mem[2].pitch * real_height;
    outStreamInfo.validCompCnt          = 3;

    result                              = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    WriteBuf     = (uint8_t *)itpVmemAlloc((real_width * real_height * 3 ));
    mappedSysRam = ithMapVram((uint32_t)WriteBuf, (real_width * real_height * 3), ITH_VRAM_WRITE);
	if (pY)
	{
		memcpy(mappedSysRam, pY, (real_width * real_height * 3));
		free(pY);
	}
    ithUnmapVram((void *)mappedSysRam, (real_width * real_height * 3 ));
    ithFlushDCacheRange((void *)mappedSysRam, (real_width * real_height * 3 ));
    ithFlushMemBuffer();

    result = iteJpg_Process(pHJpeg, &entropyBufInfo, 0, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
        goto end;
    }

    if (WriteBuf)
    {
        itpVmemFree((uint32_t)WriteBuf);
    }

    iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

    {
        CLIP_WND_INFO   clipInfo  = {0};
        BASE_RECT       srcRect   = {0};
        DATA_COLOR_TYPE colorType = 0;

        clipInfo.bClipEnable = 0;

        srcRect.w            = real_width;
        srcRect.h            = real_height;

        //printf("  ** jpgUserInfo.colorFormate=0x%x\n", jpgUserInfo.colorFormate);
        switch (jpgUserInfo.colorFormate)
        {
        case JPG_COLOR_SPACE_YUV444:    colorType   = DATA_COLOR_YUV444;   break;
        case JPG_COLOR_SPACE_YUV422:    colorType   = DATA_COLOR_YUV422;   break;
        case JPG_COLOR_SPACE_YUV420:    colorType   = DATA_COLOR_YUV420;   break;
        case JPG_COLOR_SPACE_YUV422R:   colorType   = DATA_COLOR_YUV422R;  break;
        case JPG_COLOR_SPACE_RGB565:    colorType   = DATA_COLOR_RGB565;   break;
        case JPG_COLOR_SPACE_ARGB8888:  colorType   = DATA_COLOR_ARGB8888; break;
        case JPG_COLOR_SPACE_ARGB4444:  colorType   = DATA_COLOR_ARGB4444; break;
        }

        set_isp_colorTrans(
            outStreamInfo.jstream.mem[0].pAddr,
            outStreamInfo.jstream.mem[1].pAddr,
            outStreamInfo.jstream.mem[2].pAddr,
            colorType,
            &clipInfo,
            &srcRect,
            &destRect,
            imgWidth,
            imgHeight,
            surf->pitch,
            dest);
    }

    //if (pY) free(pY);
    ituUnlockSurface(surf);
    return surf;

end:
    if (pY) free(pY);
    iteJpg_DestroyHandle(&pHJpeg, 0);
    ituUnlockSurface(surf);
    ituDestroySurface(surf);
    return NULL;
}

ITUSurface *ituJpegLoadFile(int width, int height, char *filepath, unsigned int flags)
{
    ITUSurface  *surf = NULL;
    FILE        *f    = NULL;
    int         size  = 0;
    uint8_t     *data = NULL;
    struct stat sb;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    if (fstat(fileno(f), &sb) == -1)
        goto end;

    size = sb.st_size;

    data = malloc(size);
    if (!data)
        goto end;

    size = fread(data, 1, size, f);
    printf("+ituJpegLoad(%d,%d,0x%X,%d)\n", width, height, data, size);
    surf = ituJpegLoad(width, height, data, size, flags);
    printf("-ituJpegLoad\n");
end:
    free(data);

    if (f)
        fclose(f);

    return surf;
}

void ituJpegSaveFile(ITUSurface *surf, char *filepath)
{
    uint8_t *src      = NULL;
    uint8_t *yuv_data = NULL;
    src = ituLockSurface(surf, 0, 0, surf->width, surf->height);
    assert(src);

    if (surf == ituGetDisplaySurface())
    {
        uint32_t addr;

        switch (ithLcdGetFlip())
        {
        case 0:
            addr = ithLcdGetBaseAddrA();
            break;

        case 1:
            addr = ithLcdGetBaseAddrB();
            break;

        default:
            addr = ithLcdGetBaseAddrC();
            break;
        }
        src = ithMapVram(addr, surf->lockSize, ITH_VRAM_READ);
    }

    yuv_data = (char *) malloc(surf->width * surf->height * 3 / 2);

    if (surf->format == ITU_ARGB8888)
    {
        argb8888toyuv420(yuv_data, src, surf->width, surf->height);
    }
    else if (surf->format == ITU_RGB565)
    {
        int     h;
        int     size    = surf->width * surf->height * 3;
        uint8_t *rgb888 = malloc(size);
        for (h = 0; h < surf->height; h++)
        {
            int     i, j;
            uint8_t *ptr = src + surf->width * 2 * h;

            // color trasform from RGB565 to RGB888
            for (i = (surf->width - 1) * 2, j = (surf->width - 1) * 3; i >= 0 && j >= 0; i -= 2, j -= 3)
            {
                rgb888[surf->width * h * 3 + j + 0] = ((ptr[i + 1]) & 0xf8) + ((ptr[i + 1] >> 5) & 0x07);
                rgb888[surf->width * h * 3 + j + 1] = ((ptr[i + 0] >> 3) & 0x1c) + ((ptr[i + 1] << 5) & 0xe0) + ((ptr[i + 1] >> 1) & 0x3);
                rgb888[surf->width * h * 3 + j + 2] = ((ptr[i + 0] << 3) & 0xf8) + ((ptr[i + 0] >> 2) & 0x07);
            }
        }

        rgb888toyuv420(yuv_data, rgb888, surf->width, surf->height);
        free(rgb888);
    }

    if (yuv_data)
    {
        HJPG            *pHJpeg = 0;
        JPG_INIT_PARAM  initParam = {0};
        JPG_STREAM_INFO inStreamInfo = {0};
        JPG_STREAM_INFO outStreamInfo = {0};
        JPG_BUF_INFO    entropyBufInfo = {0};
        JPG_USER_INFO   jpgUserInfo = {0};
        uint32_t        jpgEncSize = 0;

        unsigned char   *pAddr_y = 0, *pAddr_u = 0, *pAddr_v = 0;
        pAddr_y                           = yuv_data;
        pAddr_u                           = yuv_data + (surf->width * surf->height);
        pAddr_v                           = pAddr_u + (surf->width * surf->height / 4);

        initParam.codecType               = JPG_CODEC_ENC_JPG;
        initParam.outColorSpace           = JPG_COLOR_SPACE_YUV420;

        initParam.width                   = surf->width;
        initParam.height                  = surf->height;

        initParam.encQuality              = 70;//85;

        iteJpg_CreateHandle(&pHJpeg, &initParam, 0);
        inStreamInfo.streamIOType         = JPG_STREAM_IO_READ;
        inStreamInfo.streamType           = JPG_STREAM_MEM;
        // Y
        inStreamInfo.jstream.mem[0].pAddr = (uint8_t *)pAddr_y; //YUV_Save;
        inStreamInfo.jstream.mem[0].pitch = surf->width;        // src_w_out;

        // U
        inStreamInfo.jstream.mem[1].pAddr = (uint8_t *)pAddr_u; //(inStreamInfo.jstream.mem[0].pAddr+H264_pitch_y*src_h_out);
        inStreamInfo.jstream.mem[1].pitch = surf->width / 2;    //src_w_out/2;

        // V
        inStreamInfo.jstream.mem[2].pAddr = (uint8_t *)pAddr_v; //(inStreamInfo.jstream.mem[1].pAddr+H264_pitch_y*src_h_out);
        inStreamInfo.jstream.mem[2].pitch = surf->width / 2;    //src_w_out/2;

        inStreamInfo.validCompCnt         = 3;

        if (filepath)
        {
            outStreamInfo.streamType   = JPG_STREAM_FILE;
            outStreamInfo.jstream.path = (void *)filepath;
        }
        outStreamInfo.streamIOType          = JPG_STREAM_IO_WRITE;
        outStreamInfo.jpg_reset_stream_info = 0; //  _reset_stream_info;

        iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, &outStreamInfo, 0);
        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);

        iteJpg_Setup(pHJpeg, 0);

        iteJpg_Process(pHJpeg, &entropyBufInfo, &jpgEncSize, 0);

        iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
        printf("\n\tresult = %d, encode size = %f KB\n", jpgUserInfo.status, (float)jpgEncSize / 1024);

        iteJpg_DestroyHandle(&pHJpeg, 0);

        free(yuv_data);
    }

    ituUnlockSurface(surf);
}

#else
ITUSurface *ituJpegLoad(int width, int height, uint8_t *data, int size, unsigned int flags)
{
    uint8_t                       *src = NULL;
    uint16_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;

    assert(data);
    assert(size > 0);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    jpeg_read_header(&cinfo, TRUE);

    if (width && height)
    {
        cinfo.scale_num   = 1;
        cinfo.scale_denom = ITH_MAX(cinfo.image_width / width, cinfo.image_height / height);
        if (cinfo.scale_denom == 0)
            cinfo.scale_denom = 1;
    }

    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components);
    if (!src)
        goto end;

    y = 0;

    if (width == 0 || height == 0)
    {
        w = (int)cinfo.output_width;
        h = (int)cinfo.output_height;
    }
    else
    {
        w = width < (int)cinfo.output_width ? width : cinfo.output_width;
        h = height < (int)cinfo.output_height ? height : cinfo.output_height;
    }

    surf = ituCreateSurface(w, h, 0, ITU_RGB565, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint16_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    while ((int)cinfo.output_scanline < h)
    {
        jpeg_read_scanlines(&cinfo, &src, 1);

        for (x = 0; x < w; x++)
        {
            dest[x + y * w] = ITH_RGB565(src[x * 3], src[x * 3 + 1], src[x * 3 + 2]);
        }
        y++;
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    return surf;
}

ITUSurface *ituJpegAlphaLoad(int width, int height, uint8_t *alpha, uint8_t *data, int size)
{
    uint8_t                       *src = NULL;
    uint32_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;

    assert(data);
    assert(size > 0);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components);
    if (!src)
        goto end;

    y    = 0;
    w    = width;
    h    = height;

    surf = ituCreateSurface(w, h, 0, ITU_ARGB8888, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint32_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    while ((int)cinfo.output_scanline < h)
    {
        jpeg_read_scanlines(&cinfo, &src, 1);

        for (x = 0; x < w; x++)
        {
            dest[x + y * w] = ITH_ARGB8888(*alpha++, src[x * 3], src[x * 3 + 1], src[x * 3 + 2]);
        }
        y++;
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    return surf;
}

ITUSurface *ituJpegLoadFile(int width, int height, char *filepath, unsigned int flags)
{
    uint8_t                       *src = NULL;
    uint16_t                      *dest = NULL;
    ITUSurface                    *surf = NULL;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr         jerr;
    int                           w, h, x, y;
    FILE                          *f = NULL;

    assert(filepath);

    f = fopen(filepath, "rb");
    if (!f)
        goto end;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);

    if (width && height)
    {
        cinfo.scale_num   = 1;
        cinfo.scale_denom = ITH_MAX(cinfo.image_width / width, cinfo.image_height / height);
        if (cinfo.scale_denom == 0)
            cinfo.scale_denom = 1;
    }

    jpeg_start_decompress(&cinfo);

    if (cinfo.output_components != 3)
        goto end;

    src = malloc(cinfo.output_width * cinfo.output_components * cinfo.output_height);
    if (!src)
        goto end;

    while (cinfo.output_scanline < cinfo.output_height)
    {
        uint8_t *rowp[1];
        rowp[0] = src + cinfo.output_scanline * cinfo.output_width * cinfo.output_components;
        jpeg_read_scanlines(&cinfo, rowp, 1);
    }

    if (width == 0 || height == 0)
    {
        w = (int)cinfo.output_width;
        h = (int)cinfo.output_height;
    }
    else
    {
        w = width < (int)cinfo.output_width ? width : cinfo.output_width;
        h = height < (int)cinfo.output_height ? height : cinfo.output_height;
    }

    surf = ituCreateSurface(w, h, 0, ITU_RGB565, NULL, 0);
    if (!surf)
        goto end;

    dest = (uint16_t *)ituLockSurface(surf, 0, 0, w, h);
    assert(dest);

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            int xx    = x * cinfo.output_width / w;
            int yy    = y * cinfo.output_height / h;
            int index = cinfo.output_width * yy + xx;
            dest[w * y + x] = ITH_RGB565(src[index * 3], src[index * 3 + 1], src[index * 3 + 2]);
        }
    }

    jpeg_destroy_decompress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (src)
        free(src);

    if (f)
        fclose(f);

    return surf;
}

void ituJpegSaveFile(ITUSurface *surf, char *filepath)
{
    FILE                        *fp = NULL;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
    JSAMPROW                    row_pointer[1];
    JSAMPLE                     *image_buffer = NULL;
    int                         h;
    uint8_t                     *src          = NULL;

    fp = fopen(filepath, "wb");
    if (!fp)
    {
        LOG_ERR "open %s fail.\n", filepath LOG_END
        goto end;
    }

    image_buffer = malloc(surf->width * surf->height * 3);
    if (!image_buffer)
        goto end;

    cinfo.err              = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width      = surf->width;
    cinfo.image_height     = surf->height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    src = ituLockSurface(surf, 0, 0, surf->width, surf->height);
    assert(src);

    if (surf == ituGetDisplaySurface())
    {
        uint32_t addr;

        switch (ithLcdGetFlip())
        {
        case 0:
            addr = ithLcdGetBaseAddrA();
            break;

        case 1:
            addr = ithLcdGetBaseAddrB();
            break;

        default:
            addr = ithLcdGetBaseAddrC();
            break;
        }
        src = ithMapVram(addr, surf->lockSize, ITH_VRAM_READ);
    }

    if (surf->format == ITU_ARGB8888)
    {
        for (h = 0; h < surf->height; h++)
        {
            int     i, j;
            uint8_t *ptr = src + surf->width * 4 * h;

            // color trasform from ARGB8888 to RGB888
            for (i = (surf->width - 1) * 4, j = (surf->width - 1) * 3; i >= 0 && j >= 0; i -= 4, j -= 3)
            {
                image_buffer[surf->width * h * 3 + j + 0] = ptr[i + 2];
                image_buffer[surf->width * h * 3 + j + 1] = ptr[i + 1];
                image_buffer[surf->width * h * 3 + j + 2] = ptr[i + 0];
            }
        }
    }
    else if (surf->format == ITU_RGB565)
    {
        for (h = 0; h < surf->height; h++)
        {
            int     i, j;
            uint8_t *ptr = src + surf->width * 2 * h;

            // color trasform from RGB565 to RGB888
            for (i = (surf->width - 1) * 2, j = (surf->width - 1) * 3; i >= 0 && j >= 0; i -= 2, j -= 3)
            {
                image_buffer[surf->width * h * 3 + j + 0] = ((ptr[i + 1]) & 0xf8) + ((ptr[i + 1] >> 5) & 0x07);
                image_buffer[surf->width * h * 3 + j + 1] = ((ptr[i + 0] >> 3) & 0x1c) + ((ptr[i + 1] << 5) & 0xe0) + ((ptr[i + 1] >> 1) & 0x3);
                image_buffer[surf->width * h * 3 + j + 2] = ((ptr[i + 0] << 3) & 0xf8) + ((ptr[i + 0] >> 2) & 0x07);
            }
        }
    }

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &image_buffer[cinfo.next_scanline * surf->width * 3];
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    ituUnlockSurface(surf);

end:
    if (image_buffer)
        free(image_buffer);

    if (fp)
        fclose(fp);
}

#endif // !defined(_WIN32) && defined(CFG_JPEG_HW_ENABLE)
