#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"

//#define ITU_PLAY_VIDEO_ON_BOOTING

static ITULayer* touchCalibrationLayer;
static ITUVideo *logoVideo = 0;
static ITULayer *mainMenuLayer = 0;

static void LogoVideoOnStop(ITUVideo* video)
{
	ituLayerGoto(mainMenuLayer);
}

bool LogoOnEnter(ITUWidget* widget, char* param)
{
    if (!touchCalibrationLayer)
    {
        touchCalibrationLayer = ituSceneFindWidget(&theScene, "touchCalibrationLayer");
        assert(touchCalibrationLayer);
    }
    if(theConfig.touch_calibration)
    {
		ituLayerGoto(touchCalibrationLayer);
        return false;
    }
	if (!logoVideo)
	{
		logoVideo = ituSceneFindWidget(&theScene, "logoVideo");
		assert(logoVideo);

		mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
		assert(mainMenuLayer);
	}

#ifdef ITU_PLAY_VIDEO_ON_BOOTING
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