#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40

static ITUScene scene;
static ITUSprite *mainLSprite = 0;
static ITUSprite *mainRSprite = 0;
static ITUSprite *mainLStateSprite = 0;
static ITUSprite *mainRStateSprite = 0;
static ITUSprite *mainLSprite2 = 0;
static ITUSprite *mainLSprite4 = 0;
static ITUSprite *mainRSprite2 = 0;
static ITUSprite *mainRSprite4 = 0;
static ITUAnimation *mainLAnimation0 = 0;
static ITUAnimation *mainLAnimation1 = 0;
static ITUAnimation *mainLAnimation3 = 0;
static ITUAnimation *mainRAnimation0 = 0;
static ITUAnimation *mainRAnimation1 = 0;
static ITUAnimation *mainRAnimation3 = 0;
static ITUAnimation *mainLStateAnimation0 = 0;
static ITUAnimation *mainLStateAnimation1 = 0;
static ITUAnimation *mainLStateAnimation2 = 0;
static ITUAnimation *mainLStateAnimation3 = 0;
static ITUAnimation *mainLStateAnimation4 = 0;
static ITUAnimation *mainRStateAnimation0 = 0;
static ITUAnimation *mainRStateAnimation1 = 0;
static ITUAnimation *mainRStateAnimation2 = 0;
static ITUAnimation *mainRStateAnimation3 = 0;
static ITUAnimation *mainRStateAnimation4 = 0;
static ITUButton *mainLButton = 0;
static ITUButton *mainRButton = 0;

static int LStatus = 0;
static int RStatus = 0;

bool MainOnEnter(ITUWidget* widget, char* param)
{
	if (!mainLSprite)
	{
		mainLSprite = ituSceneFindWidget(&scene, "mainLSprite");
		assert(mainLSprite);

		mainRSprite = ituSceneFindWidget(&scene, "mainRSprite");
		assert(mainRSprite);

		mainLStateSprite = ituSceneFindWidget(&scene, "mainLStateSprite");
		assert(mainLStateSprite);

		mainRStateSprite = ituSceneFindWidget(&scene, "mainRStateSprite");
		assert(mainRStateSprite);

		mainLSprite2 = ituSceneFindWidget(&scene, "mainLSprite2");
		assert(mainLSprite2);

		mainLSprite4 = ituSceneFindWidget(&scene, "mainLSprite4");
		assert(mainLSprite4);

		mainRSprite2 = ituSceneFindWidget(&scene, "mainRSprite2");
		assert(mainRSprite2);

		mainRSprite4 = ituSceneFindWidget(&scene, "mainRSprite4");
		assert(mainRSprite4);

		mainLAnimation0 = ituSceneFindWidget(&scene, "mainLAnimation0");
		assert(mainLAnimation0);

		mainLAnimation1 = ituSceneFindWidget(&scene, "mainLAnimation1");
		assert(mainLAnimation1);

		mainLAnimation3 = ituSceneFindWidget(&scene, "mainLAnimation3");
		assert(mainLAnimation3);

		mainRAnimation0 = ituSceneFindWidget(&scene, "mainRAnimation0");
		assert(mainRAnimation0);

		mainRAnimation1 = ituSceneFindWidget(&scene, "mainRAnimation1");
		assert(mainRAnimation1);

		mainRAnimation3 = ituSceneFindWidget(&scene, "mainRAnimation3");
		assert(mainRAnimation3);

		mainLStateAnimation0 = ituSceneFindWidget(&scene, "mainLStateAnimation0");
		assert(mainLStateAnimation0);

		mainLStateAnimation1 = ituSceneFindWidget(&scene, "mainLStateAnimation1");
		assert(mainLStateAnimation1);

		mainLStateAnimation2 = ituSceneFindWidget(&scene, "mainLStateAnimation2");
		assert(mainLStateAnimation2);

		mainLStateAnimation3 = ituSceneFindWidget(&scene, "mainLStateAnimation3");
		assert(mainLStateAnimation3);

		mainLStateAnimation4 = ituSceneFindWidget(&scene, "mainLStateAnimation4");
		assert(mainLStateAnimation4);

		mainRStateAnimation0 = ituSceneFindWidget(&scene, "mainRStateAnimation0");
		assert(mainRStateAnimation0);

		mainRStateAnimation1 = ituSceneFindWidget(&scene, "mainRStateAnimation1");
		assert(mainRStateAnimation1);

		mainRStateAnimation2 = ituSceneFindWidget(&scene, "mainRStateAnimation2");
		assert(mainRStateAnimation2);

		mainRStateAnimation3 = ituSceneFindWidget(&scene, "mainRStateAnimation3");
		assert(mainRStateAnimation3);

		mainRStateAnimation4 = ituSceneFindWidget(&scene, "mainRStateAnimation4");
		assert(mainRStateAnimation4);

		mainLButton = ituSceneFindWidget(&scene, "mainLButton");
		assert(mainLButton);

		mainRButton = ituSceneFindWidget(&scene, "mainRButton");
		assert(mainRButton);
	}
    return true;
}

bool MainRButtonOnPress(ITUWidget* widget, char* param)
{
	switch ((++RStatus) % 6)
	{
	case 0:
		ituSpriteGoto(mainRSprite, 0);
		ituSpriteGoto(mainRStateSprite, 0);
		RStatus = 0;
		break;
	case 1:
		ituSpriteGoto(mainRSprite, 1);
		ituAnimationPlay(mainRAnimation0, 0);
		ituSpriteGoto(mainRStateSprite, 1);
		ituAnimationPlay(mainRStateAnimation0, 0);
		break;
	case 2:
		ituSpriteGoto(mainRSprite, 2);
		ituAnimationPlay(mainRAnimation1, 0);
		ituSpriteGoto(mainRStateSprite, 2);
		ituAnimationPlay(mainRStateAnimation1, 0);
		ituWidgetSetVisible(mainRButton, false);
		RStatus++;
		break;
	case 4:
		ituSpriteGoto(mainRSprite, 4);
		ituAnimationPlay(mainRAnimation3, 0);
		ituSpriteGoto(mainRStateSprite, 4);
		ituAnimationPlay(mainRStateAnimation3, 0);
		ituWidgetSetVisible(mainRButton, false);
		RStatus++;
		break;
	default:
		break;
	}
	return true;
}

bool MainLButtonOnPress(ITUWidget* widget, char* param)
{
	switch ((++LStatus) % 6)
	{
	case 0:
		ituSpriteGoto(mainLSprite, 0);
		ituSpriteGoto(mainLStateSprite, 0);
		LStatus = 0;
		break;
	case 1:
		ituSpriteGoto(mainLSprite, 1);
		ituAnimationPlay(mainLAnimation0, 0);
		ituSpriteGoto(mainLStateSprite, 1);
		ituAnimationPlay(mainLStateAnimation0, 0);
		break;
	case 2:
		ituSpriteGoto(mainLSprite, 2);
		ituAnimationPlay(mainLAnimation1, 0);
		ituSpriteGoto(mainLStateSprite, 2);
		ituAnimationPlay(mainLStateAnimation1, 0);
		ituWidgetSetVisible(mainLButton, false);
		LStatus++;
		break;
	case 4:
		ituSpriteGoto(mainLSprite, 4);
		ituAnimationPlay(mainLAnimation3, 0);
		ituSpriteGoto(mainLStateSprite, 4);
		ituAnimationPlay(mainLStateAnimation3, 0);
		ituWidgetSetVisible(mainLButton, false);
		LStatus++;
	default:
		break;
	}
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainRButtonOnPress", MainRButtonOnPress,
	"MainLButtonOnPress", MainLButtonOnPress,
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
	window = SDL_CreateWindow("UI Laundry",
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Laundry.itu");

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