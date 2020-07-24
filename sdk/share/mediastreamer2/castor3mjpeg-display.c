#ifdef HAVE_CONFIG_H
    #include "mediastreamer-config.h"
#endif

#include "mediastreamer2/msfilter.h"

/* hack for RMI display */
#include "ite/itp.h"
#include "ite/itv.h"
#include "isp/mmp_isp.h"
#include "mjpegdec_castor3.h"

static void castor3_display_init(MSFilter  *f)
{
}

static void castor3_display_uninit(MSFilter *f)
{
}

static void castor3_display_preprocess(MSFilter *f)
{
}

static void castor3_display_process(MSFilter *f)
{
    mblk_t            *im      = NULL;
	YUV_FRAME		  *picture = NULL;

    ITV_DBUF_PROPERTY dispProp = {0};

    while ((im = ms_queue_get(f->inputs[0])) != NULL)
    {
        uint8_t *dbuf = NULL;
        picture = im->b_rptr;
        dbuf    = itv_get_dbuf_anchor();
        if (dbuf != NULL)
        {
            uint32_t col, row, bytespp, pitch;
            int      bidx = im->reserved1;

            dispProp.src_w    = picture->width;
            dispProp.src_h    = picture->height;
            dispProp.ya       = picture->data[0];
            dispProp.ua       = picture->data[1];
            dispProp.va       = picture->data[2];
            dispProp.pitch_y  = picture->linesize[0];
            dispProp.pitch_uv = picture->linesize[1];			
            dispProp.bidx     = bidx;
            dispProp.format   = MMP_ISP_IN_YUV422;

            itv_update_dbuf_anchor(&dispProp);
            freemsg(im);
            break;
        }
        else
        {
            freemsg(im);
            usleep(1000);
        }
    }

end:

    if (f->inputs[0] != NULL)
        ms_queue_flush(f->inputs[0]);
    if (f->inputs[1] != NULL)
        ms_queue_flush(f->inputs[1]);
}

#ifdef _MSC_VER

MSFilterDesc ms_castor3mjpeg_display_desc = {
    MS_CASTOR3MJPEG_DISPLAY_ID,
    "MSCastor3MjpegDisplay",
    N_("A mjpeg video display based on Castor3 platform"),
    MS_FILTER_OTHER,
    NULL,
    2,
    0,
    castor3_display_init,
    castor3_display_preprocess,
    castor3_display_process,
    NULL,
    castor3_display_uninit
};

#else

MSFilterDesc ms_castor3mjpeg_display_desc = {
    .id         = MS_CASTOR3MJPEG_DISPLAY_ID,
    .name       = "MSCastor3MjpegDisplay",
    .text       = N_("A mjpeg vide display based on Castor3 api"),
    .category   = MS_FILTER_OTHER,
    .ninputs    = 2,
    .noutputs   = 0,
    .init       = castor3_display_init,
    .preprocess = castor3_display_preprocess,
    .process    = castor3_display_process,
    .uninit     = castor3_display_uninit
};

#endif

MS_FILTER_DESC_EXPORT(ms_castor3mjpeg_display_desc)