#include <unistd.h>
#include "bootloader.h"
#include "config.h"
#include "ite/cli.h"

void DoUsbDeviceCommands(void)
{
    cliInit();

    for (;;)
        sleep(1000);
}
