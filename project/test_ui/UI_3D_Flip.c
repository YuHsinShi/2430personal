#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define IconLinkSurface
#define GESTURE_THRESHOLD 40

ITUScene scene;

static ITUIcon *mainDnNxt0Icon = 0;
static ITUIcon *mainDnNxt1Icon = 0;
static ITUIcon *mainDnPre0Icon = 0;
static ITUIcon *mainDnPre1Icon = 0;
static ITUIcon *mainUpNxt0Icon = 0;
static ITUIcon *mainUpNxt1Icon = 0;
static ITUIcon *mainUpPre0Icon = 0;
static ITUIcon *mainUpPre1Icon = 0;
static ITUIcon *mainLfNxt0Icon = 0;
static ITUIcon *mainLfNxt1Icon = 0;
static ITUIcon *mainLfPre0Icon = 0;
static ITUIcon *mainLfPre1Icon = 0;
static ITUIcon *mainRtNxt0Icon = 0;
static ITUIcon *mainRtNxt1Icon = 0;
static ITUIcon *mainRtPre0Icon = 0;
static ITUIcon *mainRtPre1Icon = 0;

static ITUContainer *mainUpContainer = 0;
static ITUContainer *mainDnContainer = 0;
static ITUContainer *mainLfContainer = 0;
static ITUContainer *mainRtContainer = 0;
static ITUAnimation *mainU0Animation = 0;
static ITUAnimation *mainU1Animation = 0;
static ITUAnimation *mainD0Animation = 0;
static ITUAnimation *mainD1Animation = 0;
static ITUAnimation *mainL0Animation = 0;
static ITUAnimation *mainL1Animation = 0;
static ITUAnimation *mainR0Animation = 0;
static ITUAnimation *mainR1Animation = 0;
static ITUCoverFlow *mainVCoverFlow = 0;

static ITUIcon *mainIcon[4][4] = { 0 };

static int VerIndex = 1;
static int VerCur;
static int HerIndex = 1;
static int HerCur;

bool MainOnEnter(ITUWidget* widget, char* param)
{
	if (!mainDnNxt0Icon)
	{
		mainDnNxt0Icon = ituSceneFindWidget(&scene, "mainDnNxt0Icon");
		assert(mainDnNxt0Icon);
		mainDnNxt1Icon = ituSceneFindWidget(&scene, "mainDnNxt1Icon");
		assert(mainDnNxt1Icon);
		mainDnPre0Icon = ituSceneFindWidget(&scene, "mainDnPre0Icon");
		assert(mainDnPre0Icon);
		mainDnPre1Icon = ituSceneFindWidget(&scene, "mainDnPre1Icon");
		assert(mainDnPre1Icon);
		mainUpNxt0Icon = ituSceneFindWidget(&scene, "mainUpNxt0Icon");
		assert(mainUpNxt0Icon);
		mainUpNxt1Icon = ituSceneFindWidget(&scene, "mainUpNxt1Icon");
		assert(mainUpNxt1Icon);
		mainUpPre0Icon = ituSceneFindWidget(&scene, "mainUpPre0Icon");
		assert(mainUpPre0Icon);
		mainUpPre1Icon = ituSceneFindWidget(&scene, "mainUpPre1Icon");
		assert(mainUpPre1Icon);
		mainLfNxt0Icon = ituSceneFindWidget(&scene, "mainLfNxt0Icon");
		assert(mainLfNxt0Icon);
		mainLfNxt1Icon = ituSceneFindWidget(&scene, "mainLfNxt1Icon");
		assert(mainLfNxt1Icon);
		mainLfPre0Icon = ituSceneFindWidget(&scene, "mainLfPre0Icon");
		assert(mainLfPre0Icon);
		mainLfPre1Icon = ituSceneFindWidget(&scene, "mainLfPre1Icon");
		assert(mainLfPre1Icon);
		mainRtNxt0Icon = ituSceneFindWidget(&scene, "mainRtNxt0Icon");
		assert(mainRtNxt0Icon);
		mainRtNxt1Icon = ituSceneFindWidget(&scene, "mainRtNxt1Icon");
		assert(mainRtNxt1Icon);
		mainRtPre0Icon = ituSceneFindWidget(&scene, "mainRtPre0Icon");
		assert(mainRtPre0Icon);
		mainRtPre1Icon = ituSceneFindWidget(&scene, "mainRtPre1Icon");
		assert(mainRtPre1Icon);

		mainUpContainer = ituSceneFindWidget(&scene, "mainUpContainer");
		assert(mainUpContainer);
		mainDnContainer = ituSceneFindWidget(&scene, "mainDnContainer");
		assert(mainDnContainer);
		mainLfContainer = ituSceneFindWidget(&scene, "mainLfContainer");
		assert(mainLfContainer);
		mainRtContainer = ituSceneFindWidget(&scene, "mainRtContainer");
		assert(mainRtContainer);
		mainU0Animation = ituSceneFindWidget(&scene, "mainU0Animation");
		assert(mainU0Animation);
		mainU1Animation = ituSceneFindWidget(&scene, "mainU1Animation");
		assert(mainU1Animation);
		mainD0Animation = ituSceneFindWidget(&scene, "mainD0Animation");
		assert(mainD0Animation);
		mainD1Animation = ituSceneFindWidget(&scene, "mainD1Animation");
		assert(mainD1Animation);
		mainL0Animation = ituSceneFindWidget(&scene, "mainL0Animation");
		assert(mainL0Animation);
		mainL1Animation = ituSceneFindWidget(&scene, "mainL1Animation");
		assert(mainL1Animation);
		mainR0Animation = ituSceneFindWidget(&scene, "mainR0Animation");
		assert(mainR0Animation);
		mainR1Animation = ituSceneFindWidget(&scene, "mainR1Animation");
		assert(mainR1Animation);
		mainVCoverFlow = ituSceneFindWidget(&scene, "mainVCoverFlow");
		assert(mainVCoverFlow);

		mainIcon[0][0] = ituSceneFindWidget(&scene, "main0-0Icon");
		assert(mainIcon[0][0]);
		mainIcon[0][1] = ituSceneFindWidget(&scene, "main0-1Icon");
		assert(mainIcon[0][1]);
		mainIcon[0][2] = ituSceneFindWidget(&scene, "main0-2Icon");
		assert(mainIcon[0][2]);
		mainIcon[0][3] = ituSceneFindWidget(&scene, "main0-3Icon");
		assert(mainIcon[0][3]);

		mainIcon[1][0] = ituSceneFindWidget(&scene, "main1-0Icon");
		assert(mainIcon[1][0]);
		mainIcon[1][1] = ituSceneFindWidget(&scene, "main1-1Icon");
		assert(mainIcon[1][1]);
		mainIcon[1][2] = ituSceneFindWidget(&scene, "main1-2Icon");
		assert(mainIcon[1][2]);
		mainIcon[1][3] = ituSceneFindWidget(&scene, "main1-3Icon");
		assert(mainIcon[1][3]);

		mainIcon[2][0] = ituSceneFindWidget(&scene, "main2-0Icon");
		assert(mainIcon[2][0]);
		mainIcon[2][1] = ituSceneFindWidget(&scene, "main2-1Icon");
		assert(mainIcon[2][1]);
		mainIcon[2][2] = ituSceneFindWidget(&scene, "main2-2Icon");
		assert(mainIcon[2][2]);
		mainIcon[2][3] = ituSceneFindWidget(&scene, "main2-3Icon");
		assert(mainIcon[2][3]);

		mainIcon[3][0] = ituSceneFindWidget(&scene, "main3-0Icon");
		assert(mainIcon[3][0]);
		mainIcon[3][1] = ituSceneFindWidget(&scene, "main3-1Icon");
		assert(mainIcon[3][1]);
		mainIcon[3][2] = ituSceneFindWidget(&scene, "main3-2Icon");
		assert(mainIcon[3][2]);
		mainIcon[3][3] = ituSceneFindWidget(&scene, "main3-3Icon");
		assert(mainIcon[3][3]);
	}
    return true;
}

static void LeftPlay(int Cur)
{
	ituWidgetSetVisible(mainUpContainer, false);
	ituWidgetSetVisible(mainRtContainer, false);
	ituWidgetSetVisible(mainDnContainer, false);
	ituWidgetSetVisible(mainLfContainer, true);

#ifdef IconLinkSurface
	ituIconLinkSurface(mainLfPre0Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainLfPre1Icon, mainIcon[HerIndex][VerIndex]);

	VerIndex = 1;
	ituCoverFlowGoto(mainVCoverFlow, 1);

	ituIconLinkSurface(mainLfNxt0Icon, mainIcon[Cur][VerIndex]);
	ituIconLinkSurface(mainLfNxt1Icon, mainIcon[Cur][VerIndex]);

	ituAnimationPlay(mainL0Animation, 0);
	ituAnimationPlay(mainL1Animation, 0);
#else
	char buf[64];
#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainLfPre0Icon, buf);
	ituIconLoadJpegFileSync(mainLfPre1Icon, buf);

	VerIndex = 1;
	ituCoverFlowGoto(mainVCoverFlow, 1);

#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", Cur, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", Cur, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainLfNxt0Icon, buf);
	ituIconLoadJpegFileSync(mainLfNxt1Icon, buf);
	ituAnimationPlay(mainL0Animation, 0);
	ituAnimationPlay(mainL1Animation, 0);
#endif
}

static void RightPlay(int Cur)
{
	ituWidgetSetVisible(mainLfContainer, false);
	ituWidgetSetVisible(mainUpContainer, false);
	ituWidgetSetVisible(mainDnContainer, false);
	ituWidgetSetVisible(mainRtContainer, true);

#ifdef IconLinkSurface
	ituIconLinkSurface(mainRtPre0Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainRtPre1Icon, mainIcon[HerIndex][VerIndex]);

	VerIndex = 1;
	ituCoverFlowGoto(mainVCoverFlow, 1);

	ituIconLinkSurface(mainRtNxt0Icon, mainIcon[Cur][VerIndex]);
	ituIconLinkSurface(mainRtNxt1Icon, mainIcon[Cur][VerIndex]);

	ituAnimationPlay(mainR0Animation, 0);
	ituAnimationPlay(mainR1Animation, 0);
#else
	char buf[64];
#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainRtPre0Icon, buf);
	ituIconLoadJpegFileSync(mainRtPre1Icon, buf);
	VerIndex = 1;
	ituCoverFlowGoto(mainVCoverFlow, 1);
#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", Cur, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", Cur, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainRtNxt0Icon, buf);
	ituIconLoadJpegFileSync(mainRtNxt1Icon, buf);
	ituAnimationPlay(mainR0Animation, 0);
	ituAnimationPlay(mainR1Animation, 0);
#endif
}

static void UpPlay(int Cur)
{
	ituWidgetSetVisible(mainLfContainer, false);
	ituWidgetSetVisible(mainRtContainer, false);
	ituWidgetSetVisible(mainDnContainer, false);
	ituWidgetSetVisible(mainUpContainer, true);
#ifdef IconLinkSurface
	ituIconLinkSurface(mainUpPre0Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainUpPre1Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainUpNxt0Icon, mainIcon[HerIndex][Cur]);
	ituIconLinkSurface(mainUpNxt1Icon, mainIcon[HerIndex][Cur]);
	ituAnimationPlay(mainU0Animation, 0);
	ituAnimationPlay(mainU1Animation, 0);
#else
#ifdef WIN32
	char buf[64];
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainUpPre0Icon, buf);
	ituIconLoadJpegFileSync(mainUpPre1Icon, buf);
#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, Cur);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, Cur);
#endif
	ituIconLoadJpegFileSync(mainUpNxt0Icon, buf);
	ituIconLoadJpegFileSync(mainUpNxt1Icon, buf);
	ituAnimationPlay(mainU0Animation, 0);
	ituAnimationPlay(mainU1Animation, 0);
#endif
}

static void DownPlay(int Cur)
{
	ituWidgetSetVisible(mainLfContainer, false);
	ituWidgetSetVisible(mainRtContainer, false);
	ituWidgetSetVisible(mainUpContainer, false);
	ituWidgetSetVisible(mainDnContainer, true);
#ifdef IconLinkSurface
	ituIconLinkSurface(mainDnPre0Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainDnPre1Icon, mainIcon[HerIndex][VerIndex]);
	ituIconLinkSurface(mainDnNxt0Icon, mainIcon[HerIndex][Cur]);
	ituIconLinkSurface(mainDnNxt1Icon, mainIcon[HerIndex][Cur]);
	ituAnimationPlay(mainD0Animation, 0);
	ituAnimationPlay(mainD1Animation, 0);
#else
#ifdef WIN32
	char buf[64];
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, VerIndex);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, VerIndex);
#endif
	ituIconLoadJpegFileSync(mainDnPre0Icon, buf);
	ituIconLoadJpegFileSync(mainDnPre1Icon, buf);
#ifdef WIN32
	sprintf(buf, "b:/pic/%d_%d.jpg", HerIndex, Cur);
#else
	sprintf(buf, "A:/pic/%d_%d.jpg", HerIndex, Cur);
#endif
	ituIconLoadJpegFileSync(mainDnNxt0Icon, buf);
	ituIconLoadJpegFileSync(mainDnNxt1Icon, buf);
	ituAnimationPlay(mainD0Animation, 0);
	ituAnimationPlay(mainD1Animation, 0);
#endif
}

bool MainVCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	int result;
	VerCur = atoi(param);
	result = VerCur - VerIndex;
	if (result == 0)
		return true;
	if ((result) < 0){
		if (result == -3) UpPlay(VerCur);
		else DownPlay(VerCur);			
	}
	else{
		if (result == 3) DownPlay(VerCur);
		else UpPlay(VerCur);
	}
	VerIndex = VerCur;
	return true;
}

bool MainHCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	int result;
	HerCur = atoi(param);
	result = HerCur - HerIndex;
	if (result == 0)
		return true;
	if ((result) < 0){
		if (result == -3) LeftPlay(HerCur);
		else RightPlay(HerCur);
	}	
	else{
		if (result == 3) RightPlay(HerCur);
		else LeftPlay(HerCur);
	}
	HerIndex = HerCur;
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainVCoverFlowOnChanged", MainVCoverFlowOnChanged,
	"MainHCoverFlowOnChanged", MainHCoverFlowOnChanged,
	NULL, NULL
};

int SDL_main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Event ev;
	int done, delay, lastx, lasty;
	uint32_t tick, dblclk, lasttick, mouseDownTick;

	// wait mouting USB storage
#ifndef _WIN32
	sleep(5);
#endif

	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}
	window = SDL_CreateWindow("UI Hexahedron Flip",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1024, 600, 0);
	if (!window)
	{
		printf("Couldn't create 1024x60 window: %s\n",
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
	ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/wqyMicroHei.ttf", ITU_GLYPH_8BPP);

	// load itu file
	tick = SDL_GetTicks();

	ituSceneInit(&scene, NULL);
	ituSceneSetFunctionTable(&scene, actionFunctions);
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_3D_Flip.itu");

	printf("loading time: %dms\n", SDL_GetTicks() - tick);

	scene.leftKey = SDLK_LEFT;
	scene.upKey = SDLK_UP;
	scene.rightKey = SDLK_RIGHT;
	scene.downKey = SDLK_DOWN;

	/* Watch keystrokes */
	done = dblclk = lasttick = lastx = lasty = mouseDownTick = 0;
	while (!done)
	{
		bool result = false;

		tick = SDL_GetTicks();

		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_MOUSEMOTION:
				result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, ev.button.button, ev.button.x, ev.button.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				mouseDownTick = SDL_GetTicks();
				if (mouseDownTick - dblclk <= 300)
				{
					result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, ev.button.button, ev.button.x, ev.button.y);
					dblclk = mouseDownTick = 0;
				}
				else
				{
					result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, ev.button.button, ev.button.x, ev.button.y);
					dblclk = mouseDownTick;
					lastx = ev.button.x;
					lasty = ev.button.y;
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if (SDL_GetTicks() - dblclk <= 300)
				{
					int xdiff = abs(ev.button.x - lastx);
					int ydiff = abs(ev.button.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD)
					{
						if (ev.button.x > lastx)
						{
							printf("mouse: slide to right\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to left\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.button.x, ev.button.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.button.y > lasty)
						{
							printf("mouse: slide to down\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.button.x, ev.button.y);
						}
						else
						{
							printf("mouse: slide to up\n");
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.button.x, ev.button.y);
						}
					}
				}
				result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, ev.button.button, ev.button.x, ev.button.y);
				mouseDownTick = 0;
				break;

			case SDL_FINGERMOTION:
				printf("touch: move %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEMOVE, 1, ev.tfinger.x, ev.tfinger.y);
				break;

			case SDL_FINGERDOWN:
				printf("touch: down %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				{
					mouseDownTick = SDL_GetTicks();
					if (mouseDownTick - dblclk <= 300)
					{
						result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOUBLECLICK, 1, ev.tfinger.x, ev.tfinger.y);
						dblclk = mouseDownTick = 0;
					}
					else
					{
						result = ituSceneUpdate(&scene, ITU_EVENT_MOUSEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
						dblclk = mouseDownTick;
						lastx = ev.tfinger.x;
						lasty = ev.tfinger.y;
					}
				}
				break;

			case SDL_FINGERUP:
				printf("touch: up %d, %d\n", ev.tfinger.x, ev.tfinger.y);
				if (SDL_GetTicks() - dblclk <= 300)
				{
					int xdiff = abs(ev.tfinger.x - lastx);
					int ydiff = abs(ev.tfinger.y - lasty);

					if (xdiff >= GESTURE_THRESHOLD)
					{
						if (ev.tfinger.x > lastx)
						{
							printf("touch: slide to right %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDERIGHT, 1, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to left %d %d\n", ev.button.x, ev.button.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDELEFT, 1, ev.tfinger.x, ev.tfinger.y);
						}
					}
					else if (ydiff >= GESTURE_THRESHOLD)
					{
						if (ev.tfinger.y > lasty)
						{
							printf("touch: slide to down %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEDOWN, 1, ev.tfinger.x, ev.tfinger.y);
						}
						else
						{
							printf("touch: slide to up %d %d\n", ev.tfinger.x, ev.tfinger.y);
							result |= ituSceneUpdate(&scene, ITU_EVENT_TOUCHSLIDEUP, 1, ev.tfinger.x, ev.tfinger.y);
						}
					}
				}
				result |= ituSceneUpdate(&scene, ITU_EVENT_MOUSEUP, 1, ev.tfinger.x, ev.tfinger.y);
				mouseDownTick = 0;
				break;

			case SDL_QUIT:
				done = 1;
				break;
			}
		}

		result |= ituSceneUpdate(&scene, ITU_EVENT_TIMER, 0, 0, 0);
		if (result)
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