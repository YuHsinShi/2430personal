#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40

static mainIndex = 0;
static ITUScene scene;
static ITULayer *finishLayer;
static ITUIcon *mainNullIcon;
//static ITUIcon *mainTmpIcon;
static ITUIcon *mainScaleCoverFlowIcon[5];;
static ITUSprite *mainSprite;
static ITUSprite *mainPlaySprite[5];
static ITUButton *mainNextButton;
static ITUButton *mainPlayButton;
static ITUButton *mainPauseButton;
static ITUScaleCoverFlow *mainScaleCoverFlow;

bool MainOnEnter(ITUWidget* widget, char* param)
{
	mainIndex = 0;
	if (!mainScaleCoverFlow)
	{
		mainScaleCoverFlow = ituSceneFindWidget(&scene, "mainScaleCoverFlow");
		assert(mainScaleCoverFlow);
		mainSprite = ituSceneFindWidget(&scene, "mainSprite");
		assert(mainSprite);
		mainPlaySprite[0] = ituSceneFindWidget(&scene, "main0Sprite");
		assert(mainPlaySprite[0]);
		mainPlaySprite[1] = ituSceneFindWidget(&scene, "main1Sprite");
		assert(mainPlaySprite[1]);
		mainPlaySprite[2] = ituSceneFindWidget(&scene, "main2Sprite");
		assert(mainPlaySprite[2]);
		mainPlaySprite[3] = ituSceneFindWidget(&scene, "main3Sprite");
		assert(mainPlaySprite[3]);
		mainPlaySprite[4] = ituSceneFindWidget(&scene, "main4Sprite");
		assert(mainPlaySprite[4]);

		mainNextButton = ituSceneFindWidget(&scene, "mainNextButton");
		assert(mainNextButton);

		mainPlayButton = ituSceneFindWidget(&scene, "mainPlayButton");
		assert(mainPlayButton);

		mainPauseButton = ituSceneFindWidget(&scene, "mainPauseButton");
		assert(mainPauseButton);

		mainNullIcon = ituSceneFindWidget(&scene, "mainNullIcon");
		assert(mainNullIcon);
		/*mainTmpIcon = ituSceneFindWidget(&scene, "mainTmpIcon");
		assert(mainTmpIcon);*/

		mainScaleCoverFlowIcon[0] = ituSceneFindWidget(&scene, "main0Icon");
		assert(mainScaleCoverFlowIcon[0]);
		mainScaleCoverFlowIcon[1] = ituSceneFindWidget(&scene, "main1Icon");
		assert(mainScaleCoverFlowIcon[1]);
		mainScaleCoverFlowIcon[2] = ituSceneFindWidget(&scene, "main2Icon");
		assert(mainScaleCoverFlowIcon[2]);
		mainScaleCoverFlowIcon[3] = ituSceneFindWidget(&scene, "main3Icon");
		assert(mainScaleCoverFlowIcon[3]);
		mainScaleCoverFlowIcon[4] = ituSceneFindWidget(&scene, "main4Icon");
		assert(mainScaleCoverFlowIcon[4]);

		finishLayer = ituSceneFindWidget(&scene, "finishLayer");
		assert(finishLayer);
	}
	// reset layer
	ituSpriteGoto(mainSprite, 0);
	ituCoverFlowGoto(mainScaleCoverFlow, 1);
	//ituIconLinkSurface(mainTmpIcon, mainScaleCoverFlowIcon[mainIndex]);
	ituIconLinkSurface(mainScaleCoverFlowIcon[mainIndex], mainNullIcon);
	ituSpritePlay(mainPlaySprite[mainIndex], -1);
	ituWidgetSetVisible(mainPauseButton, true);
	ituWidgetSetVisible(mainPlayButton, false);
	// reset layer end
	return true;
}

bool MainPlayButtonOnPress(ITUWidget* widget, char* param)
{
	ituSpritePlay(mainPlaySprite[mainIndex], -1);
	return true;
}

bool MainPauseButtonOnPress(ITUWidget* widget, char* param)
{
	ituSpriteStop(mainPlaySprite[mainIndex]);
	return true;
}

bool MainNextBtnOnPress(ITUWidget* widget, char* param)
{
	if (mainIndex != 4){
		//ituIconLinkSurface(mainTmpIcon, mainPlaySprite[mainIndex]->child);
		//ituIconLinkSurface(mainScaleCoverFlowIcon[mainIndex], mainTmpIcon);
		ituIconLinkSurface(mainScaleCoverFlowIcon[mainIndex], mainPlaySprite[mainIndex]->child);
		ituSpriteStop(mainPlaySprite[mainIndex]);
		ituWidgetSetVisible(mainPlaySprite[mainIndex], false);
		mainIndex++;
		ituWidgetSetVisible(mainNextButton, false);
		ituCoverFlowNext(mainScaleCoverFlow);
		ituWidgetSetVisible(mainPauseButton, true);
		ituWidgetSetVisible(mainPlayButton, false);
	}
	else
		ituLayerGoto(finishLayer);
	return true;
}

bool MainScaleCoverFlowOnChanged(ITUWidget* widget, char* param)
{
	//ituIconLinkSurface(mainTmpIcon, mainScaleCoverFlowIcon[mainIndex]);
	ituIconLinkSurface(mainScaleCoverFlowIcon[mainIndex], mainNullIcon);

	ituSpriteGoto(mainSprite, mainIndex);
	ituSpritePlay(mainPlaySprite[mainIndex], 0);
	ituWidgetSetVisible(mainNextButton, true);
	ituWidgetSetVisible(mainPlaySprite[mainIndex - 1], true);
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainPlayButtonOnPress", MainPlayButtonOnPress,
	"MainPauseButtonOnPress", MainPauseButtonOnPress,
	"MainNextBtnOnPress", MainNextBtnOnPress,
	"MainScaleCoverFlowOnChanged", MainScaleCoverFlowOnChanged,
	NULL, NULL,
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
	window = SDL_CreateWindow("UI Bread ScaleCoverflow",
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Cartoon_ScaleCoverflow.itu");

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