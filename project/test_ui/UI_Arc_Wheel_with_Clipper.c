#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itu.h"

#define GESTURE_THRESHOLD 40

#define LANG_DISPLAY_NUM	7

#ifndef ARCTABLE_H
#define ARCTABLE_H

static int arc_3[91] = {
	32767, 32609, 32441, 32263, 32075, 31878, 31670, 31453,
	31227, 30991, 30745, 30490, 30226, 29953, 29670, 29378,
	29078, 28769, 28451, 28124, 27788, 27445, 27092, 26732,
	26363, 25987, 25602, 25210, 24810, 24402, 23987, 23565,
	23136, 22699, 22256, 21805, 21349, 20885, 20415, 19940,
	19457, 18970, 18476, 17976, 17472, 16961, 16446, 15926,
	15401, 14871, 14336, 13798, 13255, 12708, 12157, 11602,
	11044, 10483, 9918, 9350, 8780, 8206, 7631, 7053,
	6473, 5890, 5306, 4721, 4134, 3546, 2956, 2366,
	1775, 1183, 592, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0 };

static int arc_5[91] = {
	32768, 32649, 32520, 32381, 32233, 32075, 31907, 31729,
	31541, 31344, 31138, 30922, 30696, 30461, 30217, 29964,
	29702, 29430, 29150, 28860, 28562, 28255, 27940, 27616,
	27283, 26943, 26594, 26237, 25872, 25499, 25118, 24730,
	24334, 23931, 23521, 23103, 22678, 22247, 21808, 21363,
	20912, 20454, 19990, 19519, 19043, 18561, 18073, 17580,
	17082, 16578, 16069, 15556, 15037, 14514, 13987, 13455,
	12920, 12380, 11836, 11289, 10739, 10185, 9628, 9068,
	8506, 7941, 7373, 6803, 6231, 5658, 5082, 4505,
	3926, 3347, 2766, 2185, 1602, 1020, 437, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0 };

static int arc_7[91] = {
	32768, 32672, 32566, 32451, 32326, 32190, 32045, 31891,
	31726, 31552, 31368, 31175, 30972, 30760, 30539, 30308,
	30068, 29818, 29560, 29293, 29016, 28731, 28437, 28135,
	27824, 27504, 27176, 26840, 26495, 26143, 25782, 25414,
	25038, 24654, 24263, 23865, 23459, 23046, 22626, 22199,
	21765, 21325, 20878, 20425, 19966, 19500, 19029, 18552,
	18069, 17580, 17087, 16588, 16084, 15575, 15061, 14543,
	14021, 13494, 12963, 12428, 11889, 11347, 10801, 10252,
	9699, 9144, 8586, 8026, 7463, 6897, 6330, 5761,
	5189, 4617, 4043, 3467, 2891, 2314, 1736, 1157,
	579, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0 };

#endif /* ARCTABLE_H */

#if (LANG_DISPLAY_NUM==3)
static int *arc_table = arc_3;
#elif (LANG_DISPLAY_NUM==5)
static int *arc_table = arc_5;
#elif (LANG_DISPLAY_NUM==7)
static int *arc_table = arc_7;
#endif

static int h, center, stepAngle = 180 / (LANG_DISPLAY_NUM + 1);
static char widgetName[30];
static ITUScene scene;
static ITUWheel *settingWheel = 0;
static ITUWidget* settingWheelChild[10] = {0};
static ITUBackground *settingLangBackground[10] = { 0 };
static ITUBackground *settingLangYellowBackground[10] = { 0 };
static ITUBackground *settingClippingBackground = 0;
static ITUBackground *settingLangAllBackground = 0;

bool SettingOnEnter(ITUWidget* widget, char* param)
{
	if (!settingWheel)
	{
		int i;
		settingWheel = ituSceneFindWidget(&scene, "settingWheel");
		assert(settingWheel);

		settingClippingBackground = ituSceneFindWidget(&scene, "settingClippingBackground");
		assert(settingClippingBackground);

		settingLangAllBackground = ituSceneFindWidget(&scene, "settingLangAllBackground");
		assert(settingLangAllBackground);

		for (i = 0; i < 10; i++){
			settingWheelChild[i] = (ITUWidget*)itcTreeGetChildAt(settingWheel, i);
			assert(settingWheelChild[i]);

			sprintf(widgetName, "settingLangBackground%d", i);
			settingLangBackground[i] = ituSceneFindWidget(&scene, widgetName);
			assert(settingLangBackground[i]);

			sprintf(widgetName, "settingLangYellowBackground%d", i);
			settingLangYellowBackground[i] = ituSceneFindWidget(&scene, widgetName);
			assert(settingLangYellowBackground[i]);
		}
		h = ituWidgetGetHeight(settingWheelChild[0]);
		center = h * ((ituWidgetGetHeight(settingLangAllBackground) / h) / 2);
	}
	return true;
}

bool SettingOnTimer(ITUWidget* widget, char* param)
{
	bool updated = false;
	if (ituWidgetUpdate(settingWheelChild[0], ITU_EVENT_LAYOUT, 0, 0, 0)) {
		int i;
		for (i = 0; i<10; i++) {
			// calculate arc height
			int y = ituWidgetGetY(settingWheelChild[i]);
			int theta = (y - center)*stepAngle / h;
			int arc_height;
			if (theta>90) theta = 90;
			if (theta<-90) theta = -90;
			arc_height = (arc_table[abs(theta)] * h) >> 15;
			if (arc_height) {
				ituWidgetSetVisible(settingLangBackground[i], true);
				ituWidgetSetHeight(settingLangBackground[i], arc_height);
				if (theta>0)
					ituWidgetSetY(settingLangBackground[i], y);
				else
					ituWidgetSetY(settingLangBackground[i], y + h - arc_height);
			}
			else
				ituWidgetSetVisible(settingLangBackground[i], false);
			// Yellow
			if (arc_height) {
				ituWidgetSetVisible(settingLangYellowBackground[i], true);
				ituWidgetSetHeight(settingLangYellowBackground[i], arc_height);
				if (theta>0)
					ituWidgetSetY(settingLangYellowBackground[i], y);
				else
					ituWidgetSetY(settingLangYellowBackground[i], y + h - arc_height);
			}
			else
				ituWidgetSetVisible(settingLangYellowBackground[i], false);
			// Yellow
		}
		updated = true;
	}
	return updated;
}

ITUActionFunction actionFunctions[] =
{
	"SettingOnEnter", SettingOnEnter,
	"SettingOnTimer", SettingOnTimer,
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
		800, 480, 0);
	if (!window)
	{
		printf("Couldn't create 800x480 window: %s\n",
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
	ituSceneLoadFile(&scene, CFG_PRIVATE_DRIVE ":/UI_Arc_Wheel_with_Clipper.itu");

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