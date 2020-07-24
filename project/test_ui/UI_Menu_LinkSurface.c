#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40

static int mainIndex = 1;
static int focusLast = 1;
static ITUScene scene;
static ITUIcon *mainIcon[10][2] = { 0, 0 };
static ITUIcon *mainVIcon[3][2] = { 0, 0 };
static ITUCoverFlow *mainVCoverFlow[3] = { 0 };

bool MainOnEnter(ITUWidget* widget, char* param)
{
	int i,j;
	if (!mainIcon[0][0])
	{
		mainIcon[0][0] = ituSceneFindWidget(&scene, "mainIcon00");
		assert(mainIcon[0][0]);

		mainIcon[1][0] = ituSceneFindWidget(&scene, "mainIcon10");
		assert(mainIcon[1][0]);

		mainIcon[2][0] = ituSceneFindWidget(&scene, "mainIcon20");
		assert(mainIcon[2][0]);

		mainIcon[2][1] = ituSceneFindWidget(&scene, "mainIcon21");
		assert(mainIcon[2][1]);

		mainIcon[3][0] = ituSceneFindWidget(&scene, "mainIcon30");
		assert(mainIcon[3][0]);

		mainIcon[3][1] = ituSceneFindWidget(&scene, "mainIcon31");
		assert(mainIcon[3][1]);

		mainIcon[4][0] = ituSceneFindWidget(&scene, "mainIcon40");
		assert(mainIcon[4][0]);

		mainIcon[5][0] = ituSceneFindWidget(&scene, "mainIcon50");
		assert(mainIcon[5][0]);

		mainIcon[6][0] = ituSceneFindWidget(&scene, "mainIcon60");
		assert(mainIcon[6][0]);

		mainIcon[7][0] = ituSceneFindWidget(&scene, "mainIcon70");
		assert(mainIcon[7][0]);

		mainIcon[7][1] = ituSceneFindWidget(&scene, "mainIcon71");
		assert(mainIcon[7][1]);

		mainIcon[8][0] = ituSceneFindWidget(&scene, "mainIcon80");
		assert(mainIcon[8][0]);

		mainIcon[9][0] = ituSceneFindWidget(&scene, "mainIcon90");
		assert(mainIcon[9][0]);

		mainVIcon[0][0] = ituSceneFindWidget(&scene, "mainV0Icon0");
		assert(mainVIcon[0][0]);

		mainVIcon[0][1] = ituSceneFindWidget(&scene, "mainV0Icon1");
		assert(mainVIcon[0][1]);

		mainVIcon[1][0] = ituSceneFindWidget(&scene, "mainV1Icon0");
		assert(mainVIcon[1][0]);

		mainVIcon[1][1] = ituSceneFindWidget(&scene, "mainV1Icon1");
		assert(mainVIcon[1][1]);

		mainVIcon[2][0] = ituSceneFindWidget(&scene, "mainV2Icon0");
		assert(mainVIcon[2][0]);

		mainVIcon[2][1] = ituSceneFindWidget(&scene, "mainV2Icon1");
		assert(mainVIcon[2][1]);

		mainVCoverFlow[0] = ituSceneFindWidget(&scene, "mainV0CoverFlow");
		assert(mainVCoverFlow[0]);

		mainVCoverFlow[1] = ituSceneFindWidget(&scene, "mainV1CoverFlow");
		assert(mainVCoverFlow[1]);

		mainVCoverFlow[2] = ituSceneFindWidget(&scene, "mainV2CoverFlow");
		assert(mainVCoverFlow[2]);
	}
	for (i = 0; i < 3; i++){
		for (j = 0; j < 2; j++){
			if (!mainIcon[i][j]){
				ituWidgetSetVisible(mainVIcon[i][j], false);
				break;
			}
			ituWidgetSetVisible(mainVIcon[i][j], true);
			ituIconLinkSurface(mainVIcon[i][j], mainIcon[i][j]);
		}
	}
    return true;
}

bool MainHCoverFlowOnChange(ITUWidget* widget, char* param)
{
	int i, focusNext, indexNext;
	int focusIndex = atoi(param);
	if (((focusIndex - focusLast) % 2 > 0) || (focusIndex - focusLast == -2)) // shift right
	{
		printf("\nRight!");
		mainIndex = (mainIndex + 1) % 10;
		indexNext = (mainIndex + 1) % 10;
		focusNext = (focusIndex + 1) % 3;
	}
	else
	{
		printf("\nLeft!");
		mainIndex = (mainIndex + 9) % 10;
		indexNext = (mainIndex + 9) % 10;
		focusNext = (focusIndex + 2) % 3;
	}
	for (i = 0; i < 2; i++){
		if (!mainIcon[indexNext][i]){
			ituWidgetSetVisible(mainVIcon[focusNext][i], false);
			break;
		}
		ituWidgetSetVisible(mainVIcon[focusNext][i], true);
		ituIconLinkSurface(mainVIcon[focusNext][i], mainIcon[indexNext][i]);
	}
	for (i = 0; i < 3; i++)
		ituCoverFlowGoto(mainVCoverFlow[i], 0);
	focusLast = focusIndex;
	return true;
}

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainHCoverFlowOnChange", MainHCoverFlowOnChange,
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Menu_LinkSurface.itu");

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