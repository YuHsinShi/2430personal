#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define IconLinkSurface
#define GESTURE_THRESHOLD 40

#define WARM		0
#define ICE			1
#define COLD		2
#define HOT			3

ITUScene scene;
static ITUContainer *mainRMAnimationContainer = 0;
static ITUContainer *mainLMAnimationContainer = 0;
static ITUAnimation *mainRM0Animation = 0;
static ITUAnimation *mainRM1Animation = 0;
static ITUAnimation *mainRM2Animation = 0;
static ITUAnimation *mainRM3Animation = 0;
static ITUAnimation *mainLM0Animation = 0;
static ITUAnimation *mainLM1Animation = 0;
static ITUAnimation *mainLM2Animation = 0;
static ITUAnimation *mainLM3Animation = 0;
static ITUIcon *mainRM0Icon0 = 0;
static ITUIcon *mainRM0Icon1 = 0;
static ITUIcon *mainRM1Icon0 = 0;
static ITUIcon *mainRM1Icon1 = 0;
static ITUIcon *mainRM2Icon0 = 0;
static ITUIcon *mainRM2Icon1 = 0;
static ITUIcon *mainRM3Icon0 = 0;
static ITUIcon *mainRM3Icon1 = 0;
static ITUIcon *mainLM0Icon0 = 0;
static ITUIcon *mainLM0Icon1 = 0;
static ITUIcon *mainLM1Icon0 = 0;
static ITUIcon *mainLM1Icon1 = 0;
static ITUIcon *mainLM2Icon0 = 0;
static ITUIcon *mainLM2Icon1 = 0;
static ITUIcon *mainLM3Icon0 = 0;
static ITUIcon *mainLM3Icon1 = 0;
static ITUIcon *mainWarmMIcon[15];
static ITUIcon *mainHotMIcon[15];
static ITUIcon *mainColdMIcon[15];
static ITUIcon *mainIceMIcon[15];
static ITUText *mainText;

static ITUCoverFlow *mainCoverFlow;

static int currIndex = WARM;
static unsigned int frameCount;
static int frameIndex;

static void ResetIcon(int index, int frame)
{
	switch (index) {
	case WARM:
		ituIconLinkSurface(mainRM0Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainRM0Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainRM1Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainRM1Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainRM2Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainRM2Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainRM3Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainRM3Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainLM0Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainLM0Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainLM1Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainLM1Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainLM2Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainLM2Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainLM3Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainLM3Icon1, mainColdMIcon[frame]);
		break;
	case ICE:
		ituIconLinkSurface(mainRM0Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainRM0Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainRM1Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainRM1Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainRM2Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainRM2Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainRM3Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainRM3Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainLM0Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainLM0Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainLM1Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainLM1Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainLM2Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainLM2Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainLM3Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainLM3Icon1, mainHotMIcon[frame]);
		break;
	case COLD:
		ituIconLinkSurface(mainRM0Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainRM0Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainRM1Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainRM1Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainRM2Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainRM2Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainRM3Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainRM3Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainLM0Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainLM0Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainLM1Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainLM1Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainLM2Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainLM2Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainLM3Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainLM3Icon1, mainWarmMIcon[frame]);

		break;
	case HOT:
		ituIconLinkSurface(mainRM0Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainRM0Icon1, mainIceMIcon[frame]);

		ituIconLinkSurface(mainRM1Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainRM1Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainRM2Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainRM2Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainRM3Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainRM3Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainLM0Icon0, mainColdMIcon[frame]);
		ituIconLinkSurface(mainLM0Icon1, mainColdMIcon[frame]);

		ituIconLinkSurface(mainLM1Icon0, mainHotMIcon[frame]);
		ituIconLinkSurface(mainLM1Icon1, mainHotMIcon[frame]);

		ituIconLinkSurface(mainLM2Icon0, mainWarmMIcon[frame]);
		ituIconLinkSurface(mainLM2Icon1, mainWarmMIcon[frame]);

		ituIconLinkSurface(mainLM3Icon0, mainIceMIcon[frame]);
		ituIconLinkSurface(mainLM3Icon1, mainIceMIcon[frame]);

		break;
	}
}

bool MainOnEnter(ITUWidget* widget, char* param)
{
	if (!mainRMAnimationContainer)
	{
		mainRMAnimationContainer = ituSceneFindWidget(&scene, "mainRMAnimationContainer");
		assert(mainRMAnimationContainer);

		mainLMAnimationContainer = ituSceneFindWidget(&scene, "mainLMAnimationContainer");
		assert(mainLMAnimationContainer);

		mainRM0Animation = ituSceneFindWidget(&scene, "mainRM0Animation");
		assert(mainRM0Animation);
		mainRM1Animation = ituSceneFindWidget(&scene, "mainRM1Animation");
		assert(mainRM1Animation);
		mainRM2Animation = ituSceneFindWidget(&scene, "mainRM2Animation");
		assert(mainRM2Animation);
		mainRM3Animation = ituSceneFindWidget(&scene, "mainRM3Animation");
		assert(mainRM3Animation);

		mainLM0Animation = ituSceneFindWidget(&scene, "mainLM0Animation");
		assert(mainLM0Animation);
		mainLM1Animation = ituSceneFindWidget(&scene, "mainLM1Animation");
		assert(mainLM1Animation);
		mainLM2Animation = ituSceneFindWidget(&scene, "mainLM2Animation");
		assert(mainLM2Animation);
		mainLM3Animation = ituSceneFindWidget(&scene, "mainLM3Animation");
		assert(mainLM3Animation);

		mainRM0Icon0 = ituSceneFindWidget(&scene, "mainRM0Icon0");
		assert(mainRM0Icon0);
		mainRM0Icon1 = ituSceneFindWidget(&scene, "mainRM0Icon1");
		assert(mainRM0Icon1);
		mainRM1Icon0 = ituSceneFindWidget(&scene, "mainRM1Icon0");
		assert(mainRM1Icon0);
		mainRM1Icon1 = ituSceneFindWidget(&scene, "mainRM1Icon1");
		assert(mainRM1Icon1);
		mainRM2Icon0 = ituSceneFindWidget(&scene, "mainRM2Icon0");
		assert(mainRM2Icon0);
		mainRM2Icon1 = ituSceneFindWidget(&scene, "mainRM2Icon1");
		assert(mainRM2Icon1);
		mainRM3Icon0 = ituSceneFindWidget(&scene, "mainRM3Icon0");
		assert(mainRM3Icon0);
		mainRM3Icon1 = ituSceneFindWidget(&scene, "mainRM3Icon1");
		assert(mainRM3Icon1);

		mainLM0Icon0 = ituSceneFindWidget(&scene, "mainLM0Icon0");
		assert(mainLM0Icon0);
		mainLM0Icon1 = ituSceneFindWidget(&scene, "mainLM0Icon1");
		assert(mainLM0Icon1);
		mainLM1Icon0 = ituSceneFindWidget(&scene, "mainLM1Icon0");
		assert(mainLM1Icon0);
		mainLM1Icon1 = ituSceneFindWidget(&scene, "mainLM1Icon1");
		assert(mainLM1Icon1);
		mainLM2Icon0 = ituSceneFindWidget(&scene, "mainLM2Icon0");
		assert(mainLM2Icon0);
		mainLM2Icon1 = ituSceneFindWidget(&scene, "mainLM2Icon1");
		assert(mainLM2Icon1);
		mainLM3Icon0 = ituSceneFindWidget(&scene, "mainLM3Icon0");
		assert(mainLM3Icon0);
		mainLM3Icon1 = ituSceneFindWidget(&scene, "mainLM3Icon1");
		assert(mainLM3Icon1);

		mainCoverFlow = ituSceneFindWidget(&scene, "mainCoverFlow");
		assert(mainCoverFlow);

		mainWarmMIcon[0] = ituSceneFindWidget(&scene, "mainWarmMIcon0");
		assert(mainWarmMIcon[0]);
		mainWarmMIcon[1] = ituSceneFindWidget(&scene, "mainWarmMIcon1");
		assert(mainWarmMIcon[1]);
		mainWarmMIcon[2] = ituSceneFindWidget(&scene, "mainWarmMIcon2");
		assert(mainWarmMIcon[2]);
		mainWarmMIcon[3] = ituSceneFindWidget(&scene, "mainWarmMIcon3");
		assert(mainWarmMIcon[3]);
		mainWarmMIcon[4] = ituSceneFindWidget(&scene, "mainWarmMIcon4");
		assert(mainWarmMIcon[4]);
		mainWarmMIcon[5] = ituSceneFindWidget(&scene, "mainWarmMIcon5");
		assert(mainWarmMIcon[5]);
		mainWarmMIcon[6] = ituSceneFindWidget(&scene, "mainWarmMIcon6");
		assert(mainWarmMIcon[6]);
		mainWarmMIcon[7] = ituSceneFindWidget(&scene, "mainWarmMIcon7");
		assert(mainWarmMIcon[7]);
		mainWarmMIcon[8] = ituSceneFindWidget(&scene, "mainWarmMIcon8");
		assert(mainWarmMIcon[8]);
		mainWarmMIcon[9] = ituSceneFindWidget(&scene, "mainWarmMIcon9");
		assert(mainWarmMIcon[9]);
		mainWarmMIcon[10] = ituSceneFindWidget(&scene, "mainWarmMIcon10");
		assert(mainWarmMIcon[10]);
		mainWarmMIcon[11] = ituSceneFindWidget(&scene, "mainWarmMIcon11");
		assert(mainWarmMIcon[11]);
		mainWarmMIcon[12] = ituSceneFindWidget(&scene, "mainWarmMIcon12");
		assert(mainWarmMIcon[12]);
		mainWarmMIcon[13] = ituSceneFindWidget(&scene, "mainWarmMIcon13");
		assert(mainWarmMIcon[13]);
		mainWarmMIcon[14] = ituSceneFindWidget(&scene, "mainWarmMIcon14");
		assert(mainWarmMIcon[14]);

		mainColdMIcon[0] = ituSceneFindWidget(&scene, "mainColdMIcon0");
		assert(mainColdMIcon[0]);
		mainColdMIcon[1] = ituSceneFindWidget(&scene, "mainColdMIcon1");
		assert(mainColdMIcon[1]);
		mainColdMIcon[2] = ituSceneFindWidget(&scene, "mainColdMIcon2");
		assert(mainColdMIcon[2]);
		mainColdMIcon[3] = ituSceneFindWidget(&scene, "mainColdMIcon3");
		assert(mainColdMIcon[3]);
		mainColdMIcon[4] = ituSceneFindWidget(&scene, "mainColdMIcon4");
		assert(mainColdMIcon[4]);
		mainColdMIcon[5] = ituSceneFindWidget(&scene, "mainColdMIcon5");
		assert(mainColdMIcon[5]);
		mainColdMIcon[6] = ituSceneFindWidget(&scene, "mainColdMIcon6");
		assert(mainColdMIcon[6]);
		mainColdMIcon[7] = ituSceneFindWidget(&scene, "mainColdMIcon7");
		assert(mainColdMIcon[7]);
		mainColdMIcon[8] = ituSceneFindWidget(&scene, "mainColdMIcon8");
		assert(mainColdMIcon[8]);
		mainColdMIcon[9] = ituSceneFindWidget(&scene, "mainColdMIcon9");
		assert(mainColdMIcon[9]);
		mainColdMIcon[10] = ituSceneFindWidget(&scene, "mainColdMIcon10");
		assert(mainColdMIcon[10]);
		mainColdMIcon[11] = ituSceneFindWidget(&scene, "mainColdMIcon11");
		assert(mainColdMIcon[11]);
		mainColdMIcon[12] = ituSceneFindWidget(&scene, "mainColdMIcon12");
		assert(mainColdMIcon[12]);
		mainColdMIcon[13] = ituSceneFindWidget(&scene, "mainColdMIcon13");
		assert(mainColdMIcon[13]);
		mainColdMIcon[14] = ituSceneFindWidget(&scene, "mainColdMIcon14");
		assert(mainColdMIcon[14]);

		mainIceMIcon[0] = ituSceneFindWidget(&scene, "mainIceMIcon0");
		assert(mainIceMIcon[0]);
		mainIceMIcon[1] = ituSceneFindWidget(&scene, "mainIceMIcon1");
		assert(mainIceMIcon[1]);
		mainIceMIcon[2] = ituSceneFindWidget(&scene, "mainIceMIcon2");
		assert(mainIceMIcon[2]);
		mainIceMIcon[3] = ituSceneFindWidget(&scene, "mainIceMIcon3");
		assert(mainIceMIcon[3]);
		mainIceMIcon[4] = ituSceneFindWidget(&scene, "mainIceMIcon4");
		assert(mainIceMIcon[4]);
		mainIceMIcon[5] = ituSceneFindWidget(&scene, "mainIceMIcon5");
		assert(mainIceMIcon[5]);
		mainIceMIcon[6] = ituSceneFindWidget(&scene, "mainIceMIcon6");
		assert(mainIceMIcon[6]);
		mainIceMIcon[7] = ituSceneFindWidget(&scene, "mainIceMIcon7");
		assert(mainIceMIcon[7]);
		mainIceMIcon[8] = ituSceneFindWidget(&scene, "mainIceMIcon8");
		assert(mainIceMIcon[8]);
		mainIceMIcon[9] = ituSceneFindWidget(&scene, "mainIceMIcon9");
		assert(mainIceMIcon[9]);
		mainIceMIcon[10] = ituSceneFindWidget(&scene, "mainIceMIcon10");
		assert(mainIceMIcon[10]);
		mainIceMIcon[11] = ituSceneFindWidget(&scene, "mainIceMIcon11");
		assert(mainIceMIcon[11]);
		mainIceMIcon[12] = ituSceneFindWidget(&scene, "mainIceMIcon12");
		assert(mainIceMIcon[12]);
		mainIceMIcon[13] = ituSceneFindWidget(&scene, "mainIceMIcon13");
		assert(mainIceMIcon[13]);
		mainIceMIcon[14] = ituSceneFindWidget(&scene, "mainIceMIcon14");
		assert(mainIceMIcon[14]);

		mainHotMIcon[0] = ituSceneFindWidget(&scene, "mainHotMIcon0");
		assert(mainHotMIcon[0]);
		mainHotMIcon[1] = ituSceneFindWidget(&scene, "mainHotMIcon1");
		assert(mainHotMIcon[1]);
		mainHotMIcon[2] = ituSceneFindWidget(&scene, "mainHotMIcon2");
		assert(mainHotMIcon[2]);
		mainHotMIcon[3] = ituSceneFindWidget(&scene, "mainHotMIcon3");
		assert(mainHotMIcon[3]);
		mainHotMIcon[4] = ituSceneFindWidget(&scene, "mainHotMIcon4");
		assert(mainHotMIcon[4]);
		mainHotMIcon[5] = ituSceneFindWidget(&scene, "mainHotMIcon5");
		assert(mainHotMIcon[5]);
		mainHotMIcon[6] = ituSceneFindWidget(&scene, "mainHotMIcon6");
		assert(mainHotMIcon[6]);
		mainHotMIcon[7] = ituSceneFindWidget(&scene, "mainHotMIcon7");
		assert(mainHotMIcon[7]);
		mainHotMIcon[8] = ituSceneFindWidget(&scene, "mainHotMIcon8");
		assert(mainHotMIcon[8]);
		mainHotMIcon[9] = ituSceneFindWidget(&scene, "mainHotMIcon9");
		assert(mainHotMIcon[9]);
		mainHotMIcon[10] = ituSceneFindWidget(&scene, "mainHotMIcon10");
		assert(mainHotMIcon[10]);
		mainHotMIcon[11] = ituSceneFindWidget(&scene, "mainHotMIcon11");
		assert(mainHotMIcon[11]);
		mainHotMIcon[12] = ituSceneFindWidget(&scene, "mainHotMIcon12");
		assert(mainHotMIcon[12]);
		mainHotMIcon[13] = ituSceneFindWidget(&scene, "mainHotMIcon13");
		assert(mainHotMIcon[13]);
		mainHotMIcon[14] = ituSceneFindWidget(&scene, "mainHotMIcon14");
		assert(mainHotMIcon[14]);

		mainText = ituSceneFindWidget(&scene, "mainText");
		assert(mainText);
	}

	frameCount = 0;
	frameIndex = 0;
	ResetIcon(currIndex, frameIndex);

	return true;
}

bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool updated = false;

	frameCount++;
	if (frameCount % 4 == 0) {
		frameIndex++;
		if (frameIndex>14) frameIndex = 0;
		ResetIcon(currIndex, frameIndex);
		updated = true;
	}

	return updated;
}

bool MainOnSlideRight(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(mainLMAnimationContainer, false);

	ituWidgetSetVisible(mainRMAnimationContainer, true);

	ituWidgetSetVisible(mainRM0Animation, true);

	ituAnimationPlay(mainRM0Animation, 0);
	ituAnimationPlay(mainRM1Animation, 0);
	ituAnimationPlay(mainRM2Animation, 0);
	ituAnimationPlay(mainRM3Animation, 0);
	return true;
}

bool MainOnSlideLeft(ITUWidget* widget, char* param)
{
	ituWidgetSetVisible(mainLMAnimationContainer, true);

	ituWidgetSetVisible(mainRMAnimationContainer, false);

	ituWidgetSetVisible(mainLM3Animation, true);

	ituAnimationPlay(mainLM0Animation, 0);
	ituAnimationPlay(mainLM1Animation, 0);
	ituAnimationPlay(mainLM2Animation, 0);
	ituAnimationPlay(mainLM3Animation, 0);
	return true;
}

bool MainLAnimationOnStopped(ITUWidget* widget, char* param)
{
	currIndex++;
	if (currIndex>3) currIndex = 0;
	ResetIcon(currIndex, frameIndex);

	ituWidgetSetVisible(mainLM3Animation, false);

	ituAnimationReset(mainLM0Animation);
	ituAnimationReset(mainLM1Animation);
	ituAnimationReset(mainLM2Animation);
	ituAnimationReset(mainLM3Animation);
	ituCoverFlowGoto(mainCoverFlow, currIndex);
	return true;
}

bool MainRAnimationOnStopped(ITUWidget* widget, char* param)
{
	currIndex--;
	if (currIndex<0) currIndex = 3;
	ResetIcon(currIndex, frameIndex);

	ituWidgetSetVisible(mainRM0Animation, false);

	ituAnimationReset(mainRM0Animation);
	ituAnimationReset(mainRM1Animation);
	ituAnimationReset(mainRM2Animation);
	ituAnimationReset(mainRM3Animation);

	ituCoverFlowGoto(mainCoverFlow, currIndex);
	return true;
}

bool MainButtonMouseUp(ITUWidget* widget, char* param)
{
	int select = mainCoverFlow->focusIndex;
	switch (select)
	{
	case 0:
		ituTextSetString(mainText, "Lion");
		break;
	case 1:
		ituTextSetString(mainText, "Penguin");
		break;
	case 2:
		ituTextSetString(mainText, "Donkey");
		break;
	case 3:
		ituTextSetString(mainText, "Balloon");
		break;
	default:
		break;
	}
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainOnTimer", MainOnTimer,
	"MainOnSlideRight", MainOnSlideRight,
	"MainOnSlideLeft", MainOnSlideLeft,
	"MainLAnimationOnStopped", MainLAnimationOnStopped,
	"MainRAnimationOnStopped", MainRAnimationOnStopped,
	"MainButtonMouseUp", MainButtonMouseUp,
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
	ituFtLoadFont(0, CFG_PRIVATE_DRIVE ":/font/wqyMicroHei.ttf", ITU_GLYPH_8BPP);

	// load itu file
	tick = SDL_GetTicks();

	ituSceneInit(&scene, NULL);
	ituSceneSetFunctionTable(&scene, actionFunctions);
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Animation_Switching.itu");

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