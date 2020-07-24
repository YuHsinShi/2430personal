#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40
#define PART_INTEVAL 30

static ITUScene scene;
static ITUCoverFlow *menu4CoverFlow = 0;
static ITUContainer *menu4Container[4] = { 0 };
static ITUBackground *menu4Background[3][4] = { 0 };
static int org_position[3][4];
static bool moving = false;
static int moving_cnt = 0;
static int prev_pos[3] = { 0 };
static char widgetName[30];

bool MenuOnEnter(ITUWidget* widget, char* param)
{
	int i, j;
	if (!menu4CoverFlow)
	{
		menu4CoverFlow = ituSceneFindWidget(&scene, "menu4CoverFlow");
		assert(menu4CoverFlow);

		for (i = 0; i < 3; i++){
			sprintf(widgetName, "menu4Container%d", i);
			menu4Container[i] = ituSceneFindWidget(&scene, widgetName);
			assert(menu4Container[i]);
			for (j = 0; j < 4; j++){
				sprintf(widgetName, "menu4Background%d_%d", i, j);
				menu4Background[i][j] = ituSceneFindWidget(&scene, widgetName);
				assert(menu4Background[i][j]);

				org_position[i][j] = ituWidgetGetX(menu4Background[i][j]);
			}
			prev_pos[i] = ituWidgetGetX(menu4Container[i]);
		}
	}
	return true;
}

bool MenuOnTimer(ITUWidget* widget, char* param)
{
	int i, j;
	int cur_pos, diff, offset;
	bool updated = false;
	if (ituWidgetGetX(menu4Container[1])>0 && ituWidgetGetX(menu4Container[1])<1024) {
		cur_pos = ituWidgetGetX(menu4Container[1]);
		diff = cur_pos - prev_pos[1];
		if (diff>0) {
			for (i = 0; i<4; i++) {
				ituWidgetSetX(menu4Background[0][i], org_position[0][i] - PART_INTEVAL * (4 - i));
				ituWidgetSetX(menu4Background[1][i], org_position[1][i]);
			}
		}
		else if (diff<0) {
			for (i = 0; i<4; i++) {
				ituWidgetSetX(menu4Background[0][i], org_position[0][i]);
				ituWidgetSetX(menu4Background[1][i], org_position[1][i] + PART_INTEVAL * (i + 1));
			}
		}

		prev_pos[1] = cur_pos;
		moving = true;
		updated = true;
		moving_cnt = 0;
	}
	else if (ituWidgetGetX(menu4Container[2])>0 && ituWidgetGetX(menu4Container[2])<1024) {
		cur_pos = ituWidgetGetX(menu4Container[2]);
		diff = cur_pos - prev_pos[2];
		if (diff>0) {
			for (i = 0; i<4; i++) {
				ituWidgetSetX(menu4Background[1][i], org_position[1][i] - PART_INTEVAL * (4 - i));
				ituWidgetSetX(menu4Background[2][i], org_position[2][i]);
			}
		}
		else if (diff<0) {
			for (i = 0; i<4; i++) {
				ituWidgetSetX(menu4Background[1][i], org_position[1][i]);
				ituWidgetSetX(menu4Background[2][i], org_position[2][i] + PART_INTEVAL * (i + 1));
			}
		}
		prev_pos[2] = cur_pos;
		moving = true;
		updated = true;
		moving_cnt = 0;
	}
	else {
		if (moving == true) {
			if (moving_cnt<3) {
				for (i = 0; i<3; i++) {
					int tmp = ituWidgetGetX(menu4Background[i][0]);
					if (org_position[i][0] != ituWidgetGetX(menu4Background[i][0]))
						break;
				}
				if (i<3) {
					for (j = 0; j<4; j++) {
						int pos = ituWidgetGetX(menu4Background[i][j]);
						ituWidgetSetX(menu4Background[i][j], org_position[i][j] + ((pos - org_position[i][j]) >> 1));
					}
				}
			}
			else {
				for (i = 0; i<3; i++) {
					for (j = 0; j<4; j++) {
						ituWidgetSetX(menu4Background[i][j], org_position[i][j]);
					}
					prev_pos[i] = ituWidgetGetX(menu4Container[i]);
				}
				moving = false;
			}
			moving_cnt++;
			updated = true;
		}
		else {
			updated = false;
		}
	}

	return updated;
}

ITUActionFunction actionFunctions[] =
{
	"MenuOnEnter", MenuOnEnter,
	"MenuOnTimer", MenuOnTimer,
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Coverflow_Part_MyFavorite.itu");

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