#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "ite/itp.h"

#include "jpg/ite_jpg.h"
#include "isp/mmp_isp.h"

#if defined(CFG_M2D_ENABLE)
    #include "gfx/gfx.h"
#endif

#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
    #include "ite/itv.h"
    #include "ith/ith_video.h"
#endif
#include "itu_private.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_UI_BUFFER_COUNT 2
#define SURF_WIDTH 1920
#define SURF_HEIGHT 1080
#define SURF_PITCH 1920 * 2
//#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
//    #define FRAME_BUFFER_COUNT 4
//#endif

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================
//typedef struct
//{
//    ITUSurface      surf;
//#if defined(CFG_M2D_ENABLE)
//    MMP_M2D_SURFACE m2dSurf;
//#endif
//} M2dSurface;

//=============================================================================
//                              Extern Reference
//=============================================================================
extern bool gbDisplayFieldMode;

//=============================================================================
//                              Static Data Definition
//=============================================================================
static ISP_DEVICE gIspDev;
static ITUSurface *gITUSurface[MAX_UI_BUFFER_COUNT];
static uint32_t    g_ui_buff_addr[MAX_UI_BUFFER_COUNT];
static bool        g_inited;
static uint8_t     *pY = NULL;
//static ITURotation g_curr_rotation;

//=============================================================================
//                              Private Function Declaration
//=============================================================================
#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
static void VideoInit(void);
static void VideoExit(void);
#endif // CFG_VIDEO_ENABLE

//=============================================================================
//                              Public Function Definition
//=============================================================================
void ituFrameFuncInit(void)
{
#ifndef CFG_FFMPEG_H264_SW
    M2dSurface *screenSurf;
    int        width;
    int        height;
    int        pitch;
#endif

    if (g_inited)
        return;

	if(!pY)
		pY = malloc(1920 * 1080 * 3);
#ifndef CFG_FFMPEG_H264_SW

    width         = SURF_WIDTH;
    height        = SURF_HEIGHT;
    pitch         = SURF_PITCH;

    VideoInit();
    // TODO: IMPLEMENT
    if (!g_ui_buff_addr[0])
    {
        int i;
        int size = pitch * height * MAX_UI_BUFFER_COUNT;
        g_ui_buff_addr[0] = itpVmemAlloc(size);

        assert(g_ui_buff_addr[0]);
        itv_ff_setup_base(0, 0, (uint8_t *)g_ui_buff_addr[0]);
        for (i = 1; i < MAX_UI_BUFFER_COUNT; ++i)
        {
            g_ui_buff_addr[i] = g_ui_buff_addr[i - 1] + pitch * height;
            itv_ff_setup_base(0, i, (uint8_t *)g_ui_buff_addr[i]);
        }
    #ifdef _DEBUG
        for (i = 0; i < MAX_UI_BUFFER_COUNT; ++i)
            printf("frame func(%d, %X)\n", i, g_ui_buff_addr[i]);
    #endif

		for (i = 0; i < MAX_UI_BUFFER_COUNT; ++i)
        	gITUSurface[i] = ituCreateSurface(width, height, pitch, ITU_RGB565, g_ui_buff_addr[i], ITU_STATIC);
    }

    itv_ff_enable(0, true);
#endif

    g_inited       = true;
}

void ituFrameFuncExit(void)
{
	int i;
    if (!g_inited)
        return;

#ifndef CFG_FFMPEG_H264_SW
	//itv_flush_uibuf(0);
    itv_ff_enable(0, false);

    for (i = 0; i < MAX_UI_BUFFER_COUNT; i++)
        ituDestroySurface(gITUSurface[i]);

    VideoExit();
	
    if (g_ui_buff_addr[0])
    {
        itpVmemFree(g_ui_buff_addr[0]);
        memset(g_ui_buff_addr, 0, sizeof(g_ui_buff_addr));
    }
#endif
    g_inited = false;
}

int ituFrameFuncGetSurfWidth()
{
	return SURF_WIDTH;
}

int ituFrameFuncGetSurfHeight()
{
	return SURF_HEIGHT;
}

void ituFrameFuncColorFill(int rect_x, int rect_y, int rect_w, int rect_h, int alpha, int red, int green, int blue)
{
	int             i          = 0;
	uint32_t        ui_buf_ptr = 0;
	ITUColor        color;

	color.alpha = alpha;
	color.red = red;
	color.green = green;
	color.blue = blue;

	ui_buf_ptr = (uint32_t)itv_get_uibuf_anchor(0);

	if (!ui_buf_ptr) return;

	for (i = 0; i < MAX_UI_BUFFER_COUNT; i++)
        if (gITUSurface[i]->addr == ui_buf_ptr)
            break;

	if (i == MAX_UI_BUFFER_COUNT)
    {
        printf("gITUSurface addr fail ...\n");
        return;
    }

	ituColorFill(gITUSurface[i], rect_x, rect_y, rect_w, rect_h, &color);
}

void ituFrameFuncDrawStr(int font_x, int font_y, int font_w, int font_h, char *string)
{
	int             i          = 0;
    ITV_UI_PROPERTY ui_prop    = {0};
    uint32_t        ui_buf_ptr = 0;

	ui_buf_ptr = (uint32_t)itv_get_uibuf_anchor(0);

    if (!ui_buf_ptr) return;

    for (i = 0; i < MAX_UI_BUFFER_COUNT; i++)
        if (gITUSurface[i]->addr == ui_buf_ptr)
            break;

    if (i == MAX_UI_BUFFER_COUNT)
    {
        printf("gITUSurface addr fail ...\n");
        return;
    }

    //ituColorFill(gITUSurface[i], 0, 0, gui_w, gui_h, &color_r);
    ituFtSetFontStyle(0);
    ituFtSetFontSize(font_w, font_h);
    ituFtDrawText(gITUSurface[i], font_x, font_y, string);	
}

void ituFrameFuncDrawJpeg(int gui_x, int gui_y, int gui_w, int gui_h, char *jpg_data, int jpg_size)
{
	int             i          = 0;
    uint32_t        ui_buf_ptr = 0;
	uint8_t        *dest = NULL;
	HJPG            *pHJpeg = 0;
    JPG_INIT_PARAM  initParam = {0};
    JPG_STREAM_INFO inStreamInfo = {0};
    JPG_STREAM_INFO outStreamInfo = {0};
    JPG_BUF_INFO    entropyBufInfo = {0};
    JPG_USER_INFO   jpgUserInfo = {0};
    JPG_ERR         result = JPG_ERR_OK;
	JPG_RECT        destRect = {0};

	uint32_t        real_width = 0, real_height = 0;
	uint32_t        img_width = 0, img_height = 0;
	//uint8_t         *pY = 0;
	
	ui_buf_ptr = (uint32_t)itv_get_uibuf_anchor(0);

    if (!ui_buf_ptr) return;

    for (i = 0; i < MAX_UI_BUFFER_COUNT; i++)
        if (gITUSurface[i]->addr == ui_buf_ptr)
            break;

    if (i == MAX_UI_BUFFER_COUNT)
    {
        printf("gITUSurface addr fail ...\n");
        return;
    }

	dest = (uint16_t *)ituLockSurface(gITUSurface[i], 0, 0, 0, 0);

	initParam.codecType     = JPG_CODEC_DEC_JPG_CMD;    //JPG_CODEC_DEC_JPG;
    initParam.decType       = JPG_DEC_PRIMARY;          //JPG_DEC_SMALL_THUMB; //JPG_DEC_PRIMARY;
    initParam.outColorSpace = JPG_COLOR_SPACE_YUV420; //JPG_COLOR_SPACE_ARGB4444;//JPG_COLOR_SPACE_ARGB8888;
    initParam.width         = gui_w;
    initParam.height        = gui_h;
    initParam.dispMode      = JPG_DISP_CENTER;

	iteJpg_CreateHandle(&pHJpeg, &initParam, 0);

    inStreamInfo.streamIOType	= JPG_STREAM_IO_READ;
    inStreamInfo.streamType     = JPG_STREAM_MEM;
	inStreamInfo.validCompCnt           = 1;
    inStreamInfo.jstream.mem[0].pAddr  	= jpg_data;
    inStreamInfo.jstream.mem[0].length  = jpg_size;

    iteJpg_SetStreamInfo(pHJpeg, &inStreamInfo, 0, 0);

    result = iteJpg_Parsing(pHJpeg, &entropyBufInfo, (void *)&destRect);
    if (result == JPG_ERR_JPROG_STREAM)
    {
        printf("JPG not support this format\n");
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

	img_width				   = jpgUserInfo.imgWidth;
	img_height				   = jpgUserInfo.imgHeight;

	outStreamInfo.streamIOType = JPG_STREAM_IO_WRITE;
    outStreamInfo.streamType   = JPG_STREAM_MEM;

    //pY = malloc(real_width * real_height * 3);
	memset(pY, 0x0, real_width * real_height * 3);
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

	result                              = iteJpg_SetStreamInfo(pHJpeg, 0, &outStreamInfo, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
    }

    result = iteJpg_Setup(pHJpeg, 0);
    if (result != JPG_ERR_OK)
    {
        printf(" jpeg err ! %s [%d]\r\n", __FILE__, __LINE__);
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
    }

	iteJpg_GetStatus(pHJpeg, &jpgUserInfo, 0);
    //printf("\n\tresult = %d\n", jpgUserInfo.status);

    iteJpg_DestroyHandle(&pHJpeg, 0);

	{
		MMP_ISP_OUTPUT_INFO outInfo     = {0};
	    MMP_ISP_SHARE       ispInput    = {0};
	    MMP_ISP_CORE_INFO   ISPCOREINFO = {0};

		printf("jpgUserInfo.colorFormate = %d\n", jpgUserInfo.colorFormate);
		switch (jpgUserInfo.colorFormate)
        {
	        case JPG_COLOR_SPACE_YUV444:    ispInput.format = MMP_ISP_IN_YUV444; break;
	        case JPG_COLOR_SPACE_YUV422:    ispInput.format = MMP_ISP_IN_YUV422; break;
	        case JPG_COLOR_SPACE_YUV420:    ispInput.format = MMP_ISP_IN_YUV420; break;
	        case JPG_COLOR_SPACE_YUV422R:   ispInput.format = MMP_ISP_IN_YUV422R; break;
        }

		ispInput.width = img_width;
		ispInput.height = img_height;
		ispInput.isAdobe_CMYK = 0;
		ispInput.addrY = (uint32_t)outStreamInfo.jstream.mem[0].pAddr;
		ispInput.addrU = (uint32_t)outStreamInfo.jstream.mem[1].pAddr;
		ispInput.addrV = (uint32_t)outStreamInfo.jstream.mem[2].pAddr;
		ispInput.pitchY = outStreamInfo.jstream.mem[0].pitch;
		ispInput.pitchUv = outStreamInfo.jstream.mem[1].pitch;

		mmpIspInitialize(&gIspDev, MMP_ISP_CORE_1);
		ISPCOREINFO.EnPreview   = false;
    	ISPCOREINFO.PreScaleSel = MMP_ISP_PRESCALE_NORMAL;

		mmpIspSetCore(gIspDev, &ISPCOREINFO);
		mmpIspSetMode(gIspDev, MMP_ISP_MODE_TRANSFORM);

		outInfo.startX    = gui_x;
	    outInfo.startY    = gui_y;
	    outInfo.width     = gui_w;
	    outInfo.height    = gui_h;
	    outInfo.addrRGB   = (uint32_t)dest;
	    outInfo.pitchRGB  = SURF_PITCH;//gui_w*4;
	    outInfo.format    = MMP_ISP_OUT_RGB565;

	    mmpIspSetOutputWindow(gIspDev, &outInfo);
	    mmpIspSetVideoWindow(gIspDev, 0, 0, outInfo.width, outInfo.height);
		mmpIspPlayImageProcess(gIspDev, &ispInput);

        if (!gbDisplayFieldMode)
        {
		    mmpIspWaitEngineIdle(gIspDev);
        }
        else
        {
            uint32_t VP_field_status = ithReadRegA(0xd0300434);
            uint32_t timeout = 30 * 10; // 30ms

            while ((!(VP_field_status & 1 << 16) || !(VP_field_status & 1 << 17) ||
                    !(VP_field_status & 1 << 18) || !(VP_field_status & 1 << 19) || !(VP_field_status & 1 << 21)) && timeout--)
                usleep(100);

            if (!timeout) printf("JPEG scaling timeout (VP timeout)\n");
        }

        mmpIspResetEngine(gIspDev);
		mmpIspTerminate(&gIspDev);

#if 0
		// fill jpg alpha value
		{
			int j, k;
			for(j = 0; j < outInfo.height; j++)
			{
				for(k = 0; k < outInfo.width * 4; k++)
				{
					if(k % 4 == 3)
						dest[(outInfo.startY + j) * SURF_PITCH + outInfo.startX * 4 + k] = 0xFF;
				}	
			}
		}
#endif		
		ithFlushDCacheRange((void *) dest, SURF_PITCH * SURF_HEIGHT);

#if 0
		FILE *fp1 = fopen("E:/iptv.rgb", "wb");

		printf("w = %d, h = %d, pitch = %d\n", outInfo.width, outInfo.height, outInfo.pitchRGB);
        fwrite(outInfo.addrRGB, 1, outInfo.height*outInfo.pitchRGB, fp1);
        fclose(fp1);
#endif		
    }

	//if (pY) free(pY);
    ituUnlockSurface(gITUSurface[i]);	
}

void ituFrameFuncFlip(int gui_x, int gui_y, int gui_w, int gui_h)
{
	ITV_UI_PROPERTY ui_prop    = {0};
#if defined(CFG_M2D_ENABLE)    
	gfxwaitEngineIdle();
#endif
	ui_prop.startX       = gui_x;
    ui_prop.startY       = gui_y;
    ui_prop.width        = gui_w;
    ui_prop.height       = gui_h;
    ui_prop.pitch        = SURF_PITCH;
    ui_prop.colorKeyR    = 0x88;
    ui_prop.colorKeyG    = 0x88;
    ui_prop.colorKeyB    = 0x88;
    ui_prop.EnAlphaBlend = true;
    ui_prop.constAlpha   = 0xFF;
    itv_update_uibuf_anchor(0, &ui_prop);
}

//=============================================================================
//                              Private Function Definition
//=============================================================================
#if defined(CFG_VIDEO_ENABLE) && !defined(CFG_FFMPEG_H264_SW)
static void
VideoInit(
    void)
{
#if (CFG_CHIP_FAMILY == 960)
    ithVideoInit(NULL);
#endif
    itv_init();
}

static void
VideoExit(
    void)
{
    /* release dbuf & itv */
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
#if (CFG_CHIP_FAMILY == 960)	
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
#endif	
}	
#endif // CFG_VIDEO_ENABLE
