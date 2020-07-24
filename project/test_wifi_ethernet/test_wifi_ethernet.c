#include <unistd.h>
#include "curl/curl.h"
#include "ite/itp.h"
#include "doorbell.h"


void* TestFunc(void *arg)
{        
    itpInit();

    ConfigInit();

#ifdef CFG_NET_ENABLE


    NetworkInit();
#endif // CFG_NET_ENABLE

    while(1)
    {
    /*    if(NetworkIsReady())
            break;
        else
            usleep(1000);*/
    }
    
    return NULL;
}
