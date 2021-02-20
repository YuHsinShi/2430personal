#include <assert.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "project.h"

#ifndef _WIN32
	#define ITU_PLAY_VIDEO_ON_BOOTING
#endif

static ITUVideo *logoVideo = 0;
static ITULayer *mainLayer = 0;

static void LogoVideoOnStop(ITUVideo* video)
{
	ituLayerGoto(mainLayer);
}

bool LogoOnEnter(ITUWidget* widget, char* param)
{

	if (!logoVideo)
	{
		logoVideo = ituSceneFindWidget(&theScene, "logoVideo");
		assert(logoVideo);

		mainLayer = ituSceneFindWidget(&theScene, "mainLayer");
		assert(mainLayer);

	}

#ifdef ITU_PLAY_VIDEO_ON_BOOTING
	ituVideoPlay(logoVideo, 0);
	ituVideoSetOnStop(logoVideo, LogoVideoOnStop);
#else
	ituLayerGoto(mainLayer);
#endif
    return true;
}

void LogoReset(void)
{

}