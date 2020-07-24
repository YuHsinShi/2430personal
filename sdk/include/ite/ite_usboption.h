
#ifndef ITE_USB_OPTION_H
#define ITE_USB_OPTION_H

#include "ite_acm.h"

#ifdef __cplusplus
extern "C" {
#endif


int iteUsbSerialRegister(void);
int iteUsbOptionRegister(void);

#define iteUsbOptionIsReady()          iteAcmTtyIsReadyEx(TTY_USB_MODEM)

/* cb: rx callback function */
#define iteUsbOptionOpen(cb, arg)      iteAcmTtyOpenEx(TTY_USB_MODEM, cb, arg)

#define iteUsbOptionClose()            iteAcmTtyCloseEx(TTY_USB_MODEM)

#define iteUsbOptionWrite(buf, len)    iteAcmTtyWriteEx(TTY_USB_MODEM, buf, len)

#define iteUsbOptionGetWriteRoom()     iteAcmTtyGetWriteRoomEx(TTY_USB_MODEM)


#ifdef __cplusplus
}
#endif

#endif // ITE_USB_OPTION_H
