#include "ite/itu.h"

extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool mainStallsAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainMusicAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainBeamAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainReTestButtonOnPress(ITUWidget* widget, char* param);
extern bool MainNextBGButtonOnPress(ITUWidget* widget, char* param);
extern bool MainOnLeave(ITUWidget* widget, char* param);


extern bool DigitalTypeOnEnter(ITUWidget* widget, char* param);
extern bool DigitalTypeOnTimer(ITUWidget* widget, char* param);
extern bool DigitalTypeOnLeave(ITUWidget* widget, char* param);
extern bool DigitalTypeChangeUIBtOnMouseUp(ITUWidget* widget, char* param);
extern bool DigitalTypeTempAnOnStopped(ITUWidget* widget, char* param);
extern bool DigitalTypInfoUpSpriteOnStopped(ITUWidget* widget, char* param);
extern bool DigitalTypInfoDownSpriteOnStopped(ITUWidget* widget, char* param);
extern bool DigitalTypeInfoAnOnStopped(ITUWidget* widget, char* param);
extern bool DigitalTypePlayBtOnMouseUp(ITUWidget* widget, char* param);




extern bool digitalSpeedButtonOnPress(ITUWidget* widget, char* param);
extern bool digitalDelayButtonOnPress(ITUWidget* widget, char* param);




ITUActionFunction actionFunctions[] =
{
    "MainOnEnter", MainOnEnter,
    "MainOnTimer", MainOnTimer,
    "mainStallsAnimationOnStop", mainStallsAnimationOnStop,
    "MainMusicAnimationOnStop", MainMusicAnimationOnStop,
    "MainBeamAnimationOnStop", MainBeamAnimationOnStop,
    "MainReTestButtonOnPress", MainReTestButtonOnPress,
    "MainNextBGButtonOnPress", MainNextBGButtonOnPress,
	"MainOnLeave", MainOnLeave,

	"DigitalTypeOnEnter", DigitalTypeOnEnter,
	"DigitalTypeOnTimer", DigitalTypeOnTimer,
	"DigitalTypeOnLeave", DigitalTypeOnLeave,
	"DigitalTypeChangeUIBtOnMouseUp", DigitalTypeChangeUIBtOnMouseUp,
	"DigitalTypeTempAnOnStopped", DigitalTypeTempAnOnStopped,
	"DigitalTypInfoUpSpriteOnStopped", DigitalTypInfoUpSpriteOnStopped,
	"DigitalTypInfoDownSpriteOnStopped", DigitalTypInfoDownSpriteOnStopped,
	"DigitalTypeInfoAnOnStopped", DigitalTypeInfoAnOnStopped,
	"DigitalTypePlayBtOnMouseUp", DigitalTypePlayBtOnMouseUp,
	"digitalSpeedButtonOnPress", digitalSpeedButtonOnPress,
	"digitalDelayButtonOnPress", digitalDelayButtonOnPress,

    NULL, NULL
};
