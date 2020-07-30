#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

#ifndef WIN32
#define ITU_PLAY_VIDEO_ON_BOOTING
#endif

static ITULayer* touchCalibrationLayer;
static ITUVideo *logoVideo = 0;
static ITULayer *mainMenuLayer = 0;

static void LogoVideoOnStop(ITUVideo* video)
{
	printf("LogoVideoOnStop LogoVideoOnStop LogoVideoOnStop\n");
	ituLayerGoto(mainMenuLayer);
}

bool LogoOnEnter(ITUWidget* widget, char* param)
{

	if (!logoVideo)
	{
		logoVideo = ituSceneFindWidget(&theScene, "logoVideo");
		assert(logoVideo);

		mainMenuLayer = ituSceneFindWidget(&theScene, "LayerMain");
		assert(mainMenuLayer);
	}

#ifdef ITU_PLAY_VIDEO_ON_BOOTING
	printf("ITU_PLAY_VIDEO_ON_BOOTING\n");

	ituVideoPlay(logoVideo, 0);
	ituVideoSetOnStop(logoVideo, LogoVideoOnStop);
#else
	ituLayerGoto(mainMenuLayer);
#endif
    return true;
}

void LogoReset(void)
{
    touchCalibrationLayer = NULL;
}