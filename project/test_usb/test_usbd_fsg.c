#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"



void* TestFunc(void* arg)
{
    int fsg_connected = 0;

    for (;;)
    {
        if (fsg_connected) {
            if (!ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_IS_CONNECTED, NULL)) {
                printf("usb fsg leave.... \n\n");
                fsg_connected = 0;
            }
        }
        else {
            if (ioctl(ITP_DEVICE_USBDFSG, ITP_IOCTL_IS_CONNECTED, NULL)) {
                printf("\n\nusb fsg enter.... \n");
                fsg_connected = 1;
            }
        }
        usleep(66000);
    }
}


