#include "ite/itu.h"


extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainButtonOnMouseUp(ITUWidget* widget, char* param);

extern bool UartCaptureOnEnter(ITUWidget* widget, char* param);
extern bool UartCaptureOnTimer(ITUWidget* widget, char* param);


ITUActionFunction actionFunctions[] =
{
	"MainOnEnter", MainOnEnter,
	"MainButtonOnMouseUp", MainButtonOnMouseUp,
	"UartCaptureOnTimer", UartCaptureOnTimer,
	"UartCaptureOnEnter", UartCaptureOnEnter,
    NULL, NULL
};
