#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainButtonOnMouseUp(ITUWidget* widget, char* param);

extern bool UartCaptureOnEnter(ITUWidget* widget, char* param);
extern bool UartCaptureOnTimer(ITUWidget* widget, char* param);
extern bool UartCaptureOnLeave(ITUWidget* widget, char* param);


extern bool PowerOnEnter(ITUWidget* widget, char* param);
extern bool PowerOnTimer(ITUWidget* widget, char* param);
extern bool PowerOnLeave(ITUWidget* widget, char* param);

extern bool SettingOnEnter(ITUWidget* widget, char* param);
extern bool Setting_next(ITUWidget* widget, char* param);
extern bool Setting_prev(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainButtonOnMouseUp", MainButtonOnMouseUp,
	"UartCaptureOnTimer", UartCaptureOnTimer,
	"UartCaptureOnEnter", UartCaptureOnEnter,
	"UartCaptureOnLeave", UartCaptureOnLeave,
	
	"PowerOnEnter", PowerOnEnter,
	"PowerOnTimer", PowerOnTimer,
	"PowerOnLeave", PowerOnLeave,

	"SettingOnEnter", SettingOnEnter,
	"Setting_next", Setting_next,
	"Setting_prev", Setting_prev,
    NULL, NULL
};
