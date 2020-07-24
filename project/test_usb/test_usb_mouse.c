#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"


void* TestFunc(void* arg)
{
    for (;;)
    {
        ITPMouseEvent ev;
    
        if(read(ITP_DEVICE_USBMOUSE, &ev, sizeof(ITPMouseEvent)) == sizeof(ITPMouseEvent))
        {
            printf("mouse: (%d, %d, %d) %s%s%s%s %s%s%s%s \r\n", 
                ev.x, ev.y, ev.wheel,
                (ev.flags & ITP_MOUSE_LBTN_DOWN) ? "L-D " : "",
                (ev.flags & ITP_MOUSE_RBTN_DOWN) ? "R-D " : "",
                (ev.flags & ITP_MOUSE_MBTN_DOWN) ? "M-D " : "",
                (ev.flags & ITP_MOUSE_SBTN_DOWN) ? "S-D " : "",
                (ev.flags & ITP_MOUSE_LBTN_UP) ? "L-U " : "",
                (ev.flags & ITP_MOUSE_RBTN_UP) ? "R-U " : "",
                (ev.flags & ITP_MOUSE_MBTN_UP) ? "M-U " : "",
                (ev.flags & ITP_MOUSE_SBTN_UP) ? "S-U " : "" );
        }

        usleep(33000);
    }
}
