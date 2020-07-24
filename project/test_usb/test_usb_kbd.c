#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"

#include "kbd_scancode.c"


void* TestFunc(void* arg)
{
    for (;;)
    {
        ITPKeyboardEvent ev;
    
        if(read(ITP_DEVICE_USBKBD, &ev, sizeof(ITPKeyboardEvent)) == sizeof(ITPKeyboardEvent))
        {
            printf("kbd: (%d) %s - %s \r\n", 
                ev.code, 
                scancode_names[ev.code] ? scancode_names[ev.code] : " ",
                (ev.flags & ITP_KEYDOWN) ? "D" : "U");
        }

        usleep(33000);
    }
}


