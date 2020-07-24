#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/itu.h"
#include "SDL/SDL.h"

#include "qrencode.h"
#include "Qrenc.h"


//    -------------------------------------------------------
//    DEFines
//    -------------------------------------------------------

#define QRCODE_TEXT                 "User:ITE\nIC_TYPE:970\nhttp://www.ite.com.tw"       //Text to encode into QRCode
#define OUT_FILE                    "A:/output.png"                                      //Output file name
#define QRCODE_SIZE                 8                                                    //Show Output size 
//#define SAVE_PNG                               1                                                                                      //save png or not 

//#pragma pack(pop)
//    -------------------------------------------------------

static ITUScene scene;

typedef struct Rect
{
    int x;
    int y;
    int size; //size*size rect
} Rect;
/*===================================
fill color into Rectangle  color = 1 black , color = 0 white
====================================*/
static void Fill_Rect(uint16_t* dest, int w, Rect rect, bool color)
{
    int i, j = 0;

    for(i = rect.x; i < rect.x + rect.size; i++)
    {
        for(j = rect.y; j < rect.y + rect.size; j++)
        {
            if(color)
                dest[i + j * w] = ITH_RGB565(0, 0, 0);//black
            else
                dest[i + j * w] = ITH_RGB565(255, 255, 255);//white        
        }
    }
}
/*===================================
fill color into Rectangle  color = 1 black , color = 0 white
====================================*/
static void Fill_Rect_32(uint32_t* dest, int w, Rect rect, bool color)
{
    int i, j = 0;

    for(i = rect.x; i < rect.x + rect.size; i++)
    {
        for(j = rect.y; j < rect.y + rect.size; j++)
        {
            if(color)
                dest[i + j * w] = ITH_ARGB8888(255,0, 0, 0);
            else
                dest[i + j * w] = ITH_ARGB8888(255,255, 255, 255);         
        }
    }
}

/*===================================
Direct show QR encdoe result 
You can call Draw_QRcode to display the QR Code on icon
format :RGB565 or ARGB8888
====================================*/
static void Draw_QRcode(QRcode * content, ITUIcon* icon, ITUPixelFormat format)
{
    ITUSurface* surf = NULL;
    int size = QRCODE_SIZE; // scale up size
    int x,y,w,h;
    int scale_up_w;
    Rect rect;
    w = h = (content->width);
    scale_up_w = w * size;
    surf = ituCreateSurface(scale_up_w, scale_up_w, 0, format, NULL, 0);
    
    if(format == ITU_RGB565)
    {
        uint16_t* dest = (uint16_t*)ituLockSurface(surf, 0, 0, scale_up_w, scale_up_w);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            for (x = 0; x < w; x++)
            {
    			rect.x = x * size;
    			rect.y = y * size;
                rect.size = size;
                Fill_Rect(dest, scale_up_w, rect, (content->data[x + y * w] & 0x1) );
            }
        }
    }
    else
    {
        uint32_t* dest = (uint32_t*)ituLockSurface(surf, 0, 0, scale_up_w, scale_up_w);
        assert(dest);

        for (y = 0; y < h; y++)
        {
            for (x = 0; x < w; x++)
            {
    			rect.x = x * size;
    			rect.y = y * size;
                rect.size = size;
                Fill_Rect_32(dest, scale_up_w, rect, (content->data[x + y * w] & 0x1) );
            }
        }    
    }
    ituUnlockSurface(surf);

    icon->loadedSurf= surf;
    icon->widget.flags |= ITU_LOADED;
    free(content);
}

static bool OnPress(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    static char msg[256];
    ITUTextBox* txtbox = (ITUTextBox*)ituSceneFindWidget(&scene, "TextBox");
    ITUIcon* icon = (ITUIcon*)ituSceneFindWidget(&scene, "Icon");
    if (icon)
    {
        ITUButton* btn = (ITUButton*)widget;

        if ((ev == ITU_EVENT_KEYDOWN && arg1 == SDLK_RETURN))
        {
            return true;
        }
        else if (ev == ITU_EVENT_MOUSEDOWN)
        {
            if (!strcmp(widget->name, "BtnClear"))
            {
                strcpy(msg, "QR Clear");
                ituTextSetString(txtbox, msg);
                ituIconReleaseSurface(icon);
            }
            else
            {
                strcpy(msg, QRCODE_TEXT);
                ituTextSetString(txtbox, msg);
                #ifdef SAVE_PNG
                ituIconLoadPngFile(icon, OUT_FILE);
                #else
                Draw_QRcode(encode(QRCODE_TEXT, strlen(QRCODE_TEXT)), icon, ITU_RGB565);
                #endif
            }
            return true;
        }
        else if (ev == ITU_EVENT_MOUSEUP)
        {
            if (ituButtonIsPressed(btn))
            {
                return true;
            }
        }
    }
    return false;
}

//    -------------------------------------------------------
//    Main
//    -------------------------------------------------------

void* TestFunc(void* arg)
{
    // SDL start
    SDL_Window *window;
    SDL_Event ev;
    int done, delay;
    uint32_t tick, dblclk;
    ITUWidget* widget;
    
    itpInit();

    // wait mouting USB storage
#ifndef _WIN32
    sleep(3);
#endif

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return NULL;
    }
    window = SDL_CreateWindow("ITU Button Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1024, 600, 0);
    if (!window)
    {
        printf("Couldn't create 1024x600 window: %s\n",
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
    ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/WenQuanYiMicroHeiMono.ttf", ITU_GLYPH_8BPP);

    // load itu file
    tick = SDL_GetTicks();

    ituSceneInit(&scene, NULL);
    ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/qrencode.itu");

    printf("loading time: %dms\n", SDL_GetTicks() - tick);

    scene.upKey = SDLK_UP;
    scene.downKey = SDLK_DOWN;

    // customize button behavior
    widget = ituSceneFindWidget(&scene, "BtnShow");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);

    widget = ituSceneFindWidget(&scene, "BtnClear");
    if (widget)
        ituWidgetSetOnPress(widget, OnPress);
    

    // Encode input string ,then export output.png to private drive
#ifdef SAVE_PNG
    qrencode(QRCODE_TEXT, strlen(QRCODE_TEXT), OUT_FILE);
#endif
    // customize button behavior

    /* Watch keystrokes */
    done = dblclk = 0;
    while (!done)
    {
        tick = SDL_GetTicks();

        while (SDL_PollEvent(&ev))
        {
            switch (ev.type)
            {
            case SDL_FINGERDOWN:
                printf("\nOuch, you touch me at: (%d, %d)", ev.tfinger.x, ev.tfinger.y);
                ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
                switch (ev.key.keysym.sym)
                {
                case SDLK_TAB:
                    ituSceneFocusNext(&scene);
                    break;
                }
                break;

            case SDL_FINGERUP:
                ituSceneUpdate(&scene, ITU_EVENT_KEYUP, ev.key.keysym.sym, 0, 0);
                break;

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
    // SDL end

    return NULL;
}

