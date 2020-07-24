#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

static ITUScene scene;

static ITUOscilloscope* osc = NULL;
static ITUText* text1 = NULL;
static ITUText* text2 = NULL;

int SDL_main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event ev;
    int done, delay;
    uint32_t tick, lasttick, dblclk;
    float width, w, value1, value2;
    char buf[64];

    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }
    window = SDL_CreateWindow("ITU Curve Test",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 600, 0);
    if (!window)
    {
        printf("Couldn't create 800x600 window: %s\n",
                SDL_GetError());
        SDL_Quit();
        exit(2);
    }

    // init itu
    ituLcdInit();

#ifdef CFG_M2D_ENABLE
    ituM2dInit();
#else
    ituSWInit();
#endif

    ituFtInit();
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/DroidSans.ttf", ITU_GLYPH_8BPP);

    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/oscilloscope.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey     = SDLK_UP;
    scene.downKey   = SDLK_DOWN;

    osc = (ITUOscilloscope*)ituSceneFindWidget(&scene, "Oscilloscope1");
    width = (float)ituWidgetGetWidth(osc);
    w = width / osc->resolutionX;

    text1 = (ITUText*)ituSceneFindWidget(&scene, "Text1");
    ituWidgetSetX(text1, (int)(width + w * 1));

    text2 = (ITUText*)ituSceneFindWidget(&scene, "Text2");
    ituWidgetSetX(text2, (int)(width + w * 10));

    /* Watch keystrokes */
    done = lasttick = dblclk = 0;
    while (!done)
    {
        tick = SDL_GetTicks();

        if (tick - lasttick >= 1000)
        {
            int value = rand() % osc->resolutionY;

            
            ituOscilloscopeInput(osc, value);

            ituWidgetSetX(text1, (int)(width - w * osc->offsetX + w * 1));
            ituWidgetSetX(text2, (int)(width - w * osc->offsetX + w * 10));

            if (osc->offsetX == 1)
            {
                value1 = (float)value;
                sprintf(buf, "%0.2f", value1);
                ituTextSetString(text1, buf);
            }
            else if (osc->offsetX == 10)
            {
                value2 = (float)value;
                sprintf(buf, "%0.2f", value2);
                ituTextSetString(text2, buf);
            }
            printf("value=%d\n", value);
            lasttick = tick;
        }

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_KEYDOWN:
                ituSceneUpdate(&scene, ITU_EVENT_KEYDOWN, ev.key.keysym.sym, 0, 0);
                switch (ev.key.keysym.sym)
                {
                case SDLK_TAB:
                    ituSceneFocusNext(&scene);
                    break;
                }
                break;

            case SDL_KEYUP:
                ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    uint32_t t = SDL_GetTicks();
                    if (t - dblclk <= 300)
                    {
                        ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = 0;
                    }
                    else
                    {
                        ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
                        dblclk = t;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
                break;

            case SDL_QUIT:
                done = 1;
                break;
            }
        }

        if (ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0))
        {
            ituSceneDraw(&scene, ituGetDisplaySurface());
            ituFlip(ituGetDisplaySurface());
        }

        delay = 33 - (SDL_GetTicks() - tick);
        if (delay > 0)
        {
            SDL_Delay(delay);
        }
    }

    SDL_Quit();
    return (0);
}
