#include <assert.h>
#include <malloc.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char waveName[] = "ITUWaveBackground";

void ituWaveBackgroundExit(ITUWidget* widget)
{
    ITUWaveBackground* wave = (ITUWaveBackground*) widget;
    assert(widget);
    ITU_ASSERT_THREAD();

    if (wave->waveMap)
    {
        free(wave->waveMap);
        wave->waveMap = NULL;
    }
    ituIconExit(widget);
}

bool ituWaveBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUIcon* icon = (ITUIcon*) widget;
    ITUWaveBackground* wave = (ITUWaveBackground*) widget;
    assert(wave);

    if (ev == ITU_EVENT_TIMER)
    {
        if (ituWidgetIsEnabled(widget) && wave->waveMap && icon->surf && wave->delayCount++ >= wave->delay)
        {
            ITURectangle* rect = &widget->rect;
            int x, y;

            if (wave->frame >= wave->totalframe - 1)
            {
                wave->frame = 0;
            }

            wave->xcount = (rect->width - wave->offsetx) * wave->xstep / rect->width;
            if (wave->xcount <= 0)
                wave->xcount = 1;

            wave->ycount = (rect->height - wave->offsety) * wave->ystep / rect->height;
            if (wave->ycount <= 0)
                wave->ycount = 1;

            for (x = 0; x < (wave->xstep + 1) * (wave->ystep + 1); x++)
            {
                ITUPoint* pt = &wave->waveMap[x];
                pt->x = INT_MAX;
                pt->y = INT_MAX;
            }

            for (y = 0; y < wave->ycount; y++)
            {
                for (x = 0; x < wave->xcount; x++)
                {
                    ITUPoint* pt0 = &wave->waveMap[y * (wave->xcount + 1) + x];
                    ITUPoint* pt1 = &wave->waveMap[y * (wave->xcount + 1) + (x + 1)];
                    ITUPoint* pt2 = &wave->waveMap[(y + 1) * (wave->xcount + 1) + (x + 1)];
                    ITUPoint* pt3 = &wave->waveMap[(y + 1) * (wave->xcount + 1) + x];

                    int diffx = (int)(wave->ampitude * sinf(2.0f * (float)M_PI * ((float)(wave->xcount - x) / (wave->xcount) + (float)wave->frame / wave->totalframe) + (float)M_PI));
                    int diffy = 0;

                    if (diffx < 0)
                        diffx = 0;

                    if (pt0->x == INT_MAX)
                        pt0->x = (x == 0) ? 0 : diffx;

                    if (pt0->y == INT_MAX)
                        pt0->y = (y == 0) ? 0 : diffy;

                    if (pt1->x == INT_MAX)
                        pt1->x = (x == wave->xcount - 1) ? 0 : diffx;

                    //printf("%d ", pt1->x);

                    pt1->y = pt0->y;
                    pt2->x = pt1->x;

                    if (pt2->y == INT_MAX)
                        pt2->y = (y == wave->ycount - 1) ? 0 : diffy;

                    pt3->x = pt0->x;
                    pt3->y = pt2->y;
                }
                //printf("\n");
            }
            wave->frame++;
            wave->delayCount = 0;
            result = true;
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (!wave->waveMap)
        {
            wave->waveMap = calloc(sizeof(ITUPoint), (wave->xstep+1) * (wave->ystep+1));
        }
        wave->xcount = wave->ycount = 0;
    }

    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);
    return result;
}

void ituWaveBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUWaveBackground* wave = (ITUWaveBackground*) widget;
    ITUIcon* icon = (ITUIcon*)widget;
    ITURectangle* rect = &widget->rect;
    int destx, desty, w, h, i, j;
    ITURectangle prevClip;
    assert(wave);
    assert(dest);

    if (wave->waveMap && wave->xstep > 0 && wave->ystep > 0 && icon->surf)
    {
        if (wave->xcount == 0 || wave->ycount == 0)
        {
            w = rect->width - wave->offsetx;
            h = rect->height - wave->offsety;
        }
        else
        {
            w = (int)ceilf((float)(rect->width - wave->offsetx) / wave->xcount);
            h = (int)ceilf((float)(rect->height - wave->offsety) / wave->ycount);
        }

        ituWidgetSetClipping(widget, dest, x, y, &prevClip);

        ituBitBlt(dest, rect->x + x, rect->y + y, rect->width, wave->offsety, icon->surf, 0, 0);
        ituBitBlt(dest, rect->x + x, rect->y + y + wave->offsety, wave->offsetx, rect->height - wave->offsety, icon->surf, 0, wave->offsety);

        if (w > 0 && h > 0)
        {
            destx = rect->x + x + wave->offsetx;
            desty = rect->y + y + wave->offsety;

            for (i = 0; i < wave->ycount; i++)
            {
                for (j = 0; j < wave->xcount; j++)
                {
                    ITUPoint* pt0 = &wave->waveMap[i * (wave->xcount + 1) + j];
                    ITUPoint* pt1 = &wave->waveMap[i * (wave->xcount + 1) + (j + 1)];
                    ITUPoint* pt2 = &wave->waveMap[(i + 1) * (wave->xcount + 1) + (j + 1)];
                    ITUPoint* pt3 = &wave->waveMap[(i + 1) * (wave->xcount + 1) + j];
                    int dx = j * w;
                    int dy = i * h;

                    //printf("(%d,%d) (%d,%d) (%d,%d) (%d,%d) \n", pt0->x, pt0->y, surf->width + pt1->x, pt1->y,  surf->width + pt2->x,  surf->height + pt2->y, pt3->x,  surf->height + pt3->y);
                    ituStretchBlt(dest, destx + dx + pt0->x - 1, desty + dy + pt0->y, w + pt2->x - pt0->x + 1, h + pt2->y - pt0->y, icon->surf, j*w + wave->offsetx, i*h + wave->offsety, w, h);
                }
            }
        }
        ituSurfaceSetClipping(dest, prevClip.x, prevClip.y, prevClip.width, prevClip.height);
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituWaveBackgroundInit(ITUWaveBackground* wave)
{
    assert(wave);
    ITU_ASSERT_THREAD();

    memset(wave, 0, sizeof (ITUWaveBackground));

    ituBackgroundInit(&wave->bg);

    ituWidgetSetType(wave, ITU_WAVEBACKGROUND);
    ituWidgetSetName(wave, waveName);
    ituWidgetSetExit(wave, ituWaveBackgroundExit);
    ituWidgetSetUpdate(wave, ituWaveBackgroundUpdate);
    ituWidgetSetDraw(wave, ituWaveBackgroundDraw);
}

void ituWaveBackgroundLoad(ITUWaveBackground* wave, uint32_t base)
{
    assert(wave);

    ituBackgroundLoad(&wave->bg, base);
    ituWidgetSetExit(wave, ituWaveBackgroundExit);
    ituWidgetSetUpdate(wave, ituWaveBackgroundUpdate);
    ituWidgetSetDraw(wave, ituWaveBackgroundDraw);
}
