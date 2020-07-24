#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_mgr.h"
#include "scene.h"
#include "project.h"

static ITUSprite* audioPlayerSprite;
static ITUTrackBar* audioPlayerTrackBar;
static ITUProgressBar* audioPlayerProgressBar;
static ITUScrollMediaFileListBox* audioPlayerScrollMediaFileListBox;

static bool audioPlayerPlaying;
static int audioPlayerPageIndex;
static int audioPlayerFocusIndex;
static bool audioPlayerPlayingFinish;


static int AudioPlayerPlayCallback(int state)
{
	switch (state)
	{
	case AUDIOMGR_STATE_CALLBACK_PLAYING_FINISH:
		audioPlayerPlayingFinish = true;
		break;
	}
	return 0;
}

bool AudioPlayerOnEnter(ITUWidget* widget, char* param)
{
	StorageType storageType;
	int vol;

	if (!audioPlayerSprite)
	{
		audioPlayerSprite = ituSceneFindWidget(&theScene, "audioPlayerSprite");
		assert(audioPlayerSprite);

		audioPlayerTrackBar = ituSceneFindWidget(&theScene, "audioPlayerTrackBar");
		assert(audioPlayerTrackBar);

		audioPlayerProgressBar = ituSceneFindWidget(&theScene, "audioPlayerProgressBar");
		assert(audioPlayerProgressBar);

		audioPlayerScrollMediaFileListBox = ituSceneFindWidget(&theScene, "audioPlayerScrollMediaFileListBox");
		assert(audioPlayerScrollMediaFileListBox);

	}
	ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
	
	audioPlayerPageIndex = 1;
	audioPlayerFocusIndex = -1;

	storageType = StorageGetCurrType();
	ituMediaFileListSetPath(&audioPlayerScrollMediaFileListBox->mflistbox, StorageGetDrivePath(storageType));

	vol = AudioGetVolume();
	ituProgressBarSetValue(audioPlayerProgressBar, vol);
	ituTrackBarSetValue(audioPlayerTrackBar, vol);

	ituSpriteGoto(audioPlayerSprite, 0);
	audioPlayerPlaying = false;
	audioPlayerPlayingFinish = false;
    return true;
}

bool AudioPlayerOnLeave(ITUWidget* widget, char* param)
{
	ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
	ituListBoxSelect(listbox, -1);
	if (audioPlayerPlaying)
	{
		AudioStop();
		AudioResumeKeySound();
		audioPlayerPlaying = false;
	}
	ConfigSave();
    return true;
}

bool AudioPlayerVoiceUpButtonOnMouseUp(ITUWidget* widget, char* param)
{
	int vol;
	vol = AudioGetVolume();
	vol++;

	if (vol <= audioPlayerTrackBar->max)
	{
		ituTrackBarSetValue(audioPlayerTrackBar, vol);
		ituProgressBarSetValue(audioPlayerProgressBar, vol);

		if (audioPlayerPlaying)
			AudioSetVolume(vol);
		else
			AudioSetLevel(vol);
	}

    return true;
}

bool AudioPlayerVoiceDownButtonOnMouseUp(ITUWidget* widget, char* param)
{
	int vol;
	vol = AudioGetVolume();
	vol--;

	if (vol >= 0)
	{
		ituTrackBarSetValue(audioPlayerTrackBar, vol);
		ituProgressBarSetValue(audioPlayerProgressBar, vol);

		if (audioPlayerPlaying)
			AudioSetVolume(vol);
		else
			AudioSetLevel(vol);
	}
    return true;
}

bool AudioPlayerPreButtonOnMouseUp(ITUWidget* widget, char* param)
{
	ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
	ITUScrollText* item;

	if (listbox->focusIndex == -1)
	{
		ituListBoxGoto(listbox, audioPlayerPageIndex);
		ituListBoxSelect(listbox, audioPlayerFocusIndex);
	}

	item = ituMediaFileListPrev((ITUMediaFileListBox*)audioPlayerScrollMediaFileListBox);
	audioPlayerPageIndex = listbox->pageIndex;
	audioPlayerFocusIndex = listbox->focusIndex;
	if (item && audioPlayerPlaying)
	{
		char* filepath = item->tmpStr;

		AudioPauseKeySound();
		AudioPlayMusic(filepath, AudioPlayerPlayCallback);
		/*audioPlayerPageIndex = listbox->pageIndex;
		audioPlayerFocusIndex = listbox->focusIndex;*/
	}
    return true;
}

bool AudioPlayerNextButtonOnMouseUp(ITUWidget* widget, char* param)
{
	ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
	ITUScrollText* item;

	if (listbox->focusIndex == -1)
	{
		ituListBoxGoto(listbox, audioPlayerPageIndex);
		ituListBoxSelect(listbox, audioPlayerFocusIndex);
	}

	item = ituMediaFileListNext((ITUMediaFileListBox*)audioPlayerScrollMediaFileListBox);
	audioPlayerPageIndex = listbox->pageIndex;
	audioPlayerFocusIndex = listbox->focusIndex;
	if (item && audioPlayerPlaying)
	{
		char* filepath = item->tmpStr;

		AudioPauseKeySound();
		AudioPlayMusic(filepath, AudioPlayerPlayCallback);
		/*audioPlayerPageIndex = listbox->pageIndex;
		audioPlayerFocusIndex = listbox->focusIndex;*/
	}
    return true;
}

bool AudioPlayerPlayButtonOnMouseUp(ITUWidget* widget, char* param)
{
	ITUScrollText* item = ituMediaFileListPlay((ITUMediaFileListBox*)audioPlayerScrollMediaFileListBox);
	if (item)
	{
		char* filepath = item->tmpStr;
		ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
		AudioPauseKeySound();
		AudioPlayMusic(filepath, AudioPlayerPlayCallback);
		audioPlayerPlaying = true;
		audioPlayerPageIndex = listbox->pageIndex;
		audioPlayerFocusIndex = listbox->focusIndex;
	}
	

    return true;
}

bool AudioPlayerStopButtonOnMouseUp(ITUWidget* widget, char* param)
{
	if (audioPlayerPlaying)
	{
		AudioStop();
		AudioResumeKeySound();
		audioPlayerPlaying = false;
	}
    return true;
}

bool AudioPlayerTrackBarOnChanged(ITUWidget* widget, char* param)
{
	int vol;
	vol = atoi(param);

	if (audioPlayerPlaying)
		AudioSetVolume(vol);
	else
		AudioSetLevel(vol);

	return true;
}

bool AudioPlayerOnTimer(ITUWidget* widget, char* param)
{
	ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;

	if (audioPlayerPlaying)
	{
		ScreenSaverRefresh();
	}

	if (listbox->pageIndex == audioPlayerPageIndex && listbox->focusIndex == -1)
		ituListBoxSelect(listbox, audioPlayerFocusIndex);

	if (audioPlayerPlayingFinish)
	{
		ITUScrollText* item;

		if (listbox->focusIndex == -1)
		{
			ituListBoxGoto(listbox, audioPlayerPageIndex);
			ituListBoxSelect(listbox, audioPlayerFocusIndex);
		}
		item = ituMediaFileListNext((ITUMediaFileListBox*)audioPlayerScrollMediaFileListBox);
		if (item)
		{
			ITUListBox* listbox = (ITUListBox*)audioPlayerScrollMediaFileListBox;
			char* filepath = item->tmpStr;

			AudioPauseKeySound();
			AudioPlayMusic(filepath, AudioPlayerPlayCallback);
			audioPlayerPageIndex = listbox->pageIndex;
			audioPlayerFocusIndex = listbox->focusIndex;
		}
		else
		{
			AudioResumeKeySound();
			audioPlayerPlaying = false;
		}
		audioPlayerPlayingFinish = false;
	}
	return true;
}