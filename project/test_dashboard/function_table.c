#include "ite/itu.h"

extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool mainStallsAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainMusicAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainBeamAnimationOnStop(ITUWidget* widget, char* param);
extern bool MainReTestButtonOnPress(ITUWidget* widget, char* param);
extern bool MainNextBGButtonOnPress(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "MainOnEnter", MainOnEnter,
    "MainOnTimer", MainOnTimer,
    "mainStallsAnimationOnStop", mainStallsAnimationOnStop,
    "MainMusicAnimationOnStop", MainMusicAnimationOnStop,
    "MainBeamAnimationOnStop", MainBeamAnimationOnStop,
    "MainReTestButtonOnPress", MainReTestButtonOnPress,
    "MainNextBGButtonOnPress", MainNextBGButtonOnPress,
    NULL, NULL
};
