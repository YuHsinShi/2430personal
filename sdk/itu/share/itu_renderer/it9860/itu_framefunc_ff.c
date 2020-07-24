#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "itu_cfg.h"
#include "ite/ith.h"
#include "ite/itu.h"
#include "ite/itp.h"

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
#define SURF_WIDTH 600
#define SURF_HEIGHT 400
#define SURF_PITCH 600 * 4
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
//                              Static Data Definition
//=============================================================================
static ITUSurface *gITUSurface[MAX_UI_BUFFER_COUNT];
static uint32_t    g_ui_buff_addr[MAX_UI_BUFFER_COUNT];
static bool        g_inited;
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
        	gITUSurface[i] = ituCreateSurface(width, height, pitch, ITU_ARGB8888, g_ui_buff_addr[i], ITU_STATIC);
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

void ituFrameFuncDrawStr(int gui_x, int gui_y, int gui_w, int gui_h, int font_w, int font_h, char *string)
{
	int             i          = 0;
    ITV_UI_PROPERTY ui_prop    = {0};
    uint32_t        ui_buf_ptr = 0;

    ITUColor        color_r    = {0, 255, 0, 0};

	itv_ff_enable(0, true);
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

    ituColorFill(gITUSurface[i], 0, 0, gui_w, gui_h, &color_r);
    ituFtSetFontStyle(0);
    ituFtSetFontSize(font_w, font_h);
    ituFtDrawText(gITUSurface[i], 0, 0, string);

#if defined(CFG_M2D_ENABLE)
    gfxwaitEngineIdle();
#endif
    ui_prop.startX       = gui_x;
    ui_prop.startY       = gui_y;
    ui_prop.width        = gui_w;
    ui_prop.height       = gui_h;
    ui_prop.pitch        = SURF_PITCH;
    ui_prop.colorKeyR    = 0xFF;
    ui_prop.colorKeyG    = 0xFF;
    ui_prop.colorKeyB    = 0xFF;
    ui_prop.EnAlphaBlend = true;
    ui_prop.constAlpha   = 0xFF;
	printf("YC: x = %d, y = %d, w = %d, h = %d\n", ui_prop.startX, ui_prop.startY, ui_prop.width, ui_prop.height);
    itv_update_uibuf_anchor(0, &ui_prop);
}

void ituFrameFuncDrawMultiStr(int gui_x, int gui_y, int gui_w, int gui_h, int font_w, int font_h, char *str1, char *str2, char *str3, char *str4, char *str5, char *str6)
{
	int             i          = 0;
    ITV_UI_PROPERTY ui_prop    = {0};
    uint32_t        ui_buf_ptr = 0;
	ITUColor        color_1    = {255, 255, 0, 0};
	ITUColor        color_2    = {255, 0, 0, 0};

	itv_ff_enable(0, true);
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

	ituColorFill(gITUSurface[i], 0, 0, gui_w, gui_h, &color_1);
	ituColorFill(gITUSurface[i], 5, 5, gui_w - 10, gui_h - 10, &color_2);

    ituFtSetFontStyle(0);
    ituFtSetFontSize(font_w, font_h);
	if(str1)
    	ituFtDrawText(gITUSurface[i], 10, 10, str1);
	if(str2)
		ituFtDrawText(gITUSurface[i], 10, 10 + font_h, str2);
	if(str3)
		ituFtDrawText(gITUSurface[i], 10, 10 + font_h*2, str3);
	if(str4)
		ituFtDrawText(gITUSurface[i], 10, 10 + font_h*3, str4);
	if(str5)
		ituFtDrawText(gITUSurface[i], 10, 10 + font_h*4, str5);
	if(str6)
		ituFtDrawText(gITUSurface[i], 10, 10 + font_h*5, str6);

#if defined(CFG_M2D_ENABLE)
    gfxwaitEngineIdle();
#endif
    ui_prop.startX       = gui_x;
    ui_prop.startY       = gui_y;
    ui_prop.width        = gui_w;
    ui_prop.height       = gui_h;
    ui_prop.pitch        = SURF_PITCH;
    ui_prop.colorKeyR    = 0xFF;
    ui_prop.colorKeyG    = 0xFF;
    ui_prop.colorKeyB    = 0xFF;
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
#if (CFG_CHIP_FAMILY == 9860)
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
#if (CFG_CHIP_FAMILY == 9860)
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
#endif
}
#endif // CFG_VIDEO_ENABLE