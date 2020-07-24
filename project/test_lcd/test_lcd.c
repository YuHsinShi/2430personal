#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"

void* TestFunc(void* arg)
{
    uint16_t* addr;
    uint32_t col, row, x, y, i = 0;
    const uint16_t colors[] = { ITH_RGB565(255, 0, 0), ITH_RGB565(0, 255, 0), ITH_RGB565(0, 0, 255) };

    itpInit();

#ifdef CFG_LCD_MULTIPLE
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_RESET, (void*)0);
#endif
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_RESET, NULL);

    addr = (uint16_t*) ithLcdGetBaseAddrA();
    col = ithLcdGetPitch() / 2;
    row = ithLcdGetHeight();


    ////set reg 0x0020 as 0x81000000 for test mode
    //// wait for 0x81000000 become 0x01000000
    //ithWriteRegA(ITH_LCD_BASE + 0x0020, 0x81000000);
    //ithWriteRegMaskA(ITH_LCD_BASE + 0x0020, (0x1 << 25), (0x1 << 25));

    //while (ithReadRegA(ITH_LCD_BASE + 0x0020) & 0x80000000)
    //{
    //    ithDelay(1000);
    //}

#if 0
    //open AXI bus
    ithWriteRegMaskA(ITH_LCD_BASE + 0x0000, (0x1 << 19), (0x1 << 19));
#endif

    for (;;)
    {
#if 1
        uint16_t* base = ithMapVram((uint32_t) addr, ithLcdGetPitch() * ithLcdGetHeight(), ITH_VRAM_WRITE);
        uint16_t color = colors[i++ % ITH_COUNT_OF(colors)];
        uint16_t* ptr = base;

        for (y = 0; y < row; y++)
            for (x = 0; x < col; x++)
                *ptr++ = color;
        
        ithFlushDCacheRange(base, row * col * 2);
        sleep(1);
        printf("go=%d\n", i);
#else
        uint32_t	writeData = 0;
        uint32_t	readData = 0;

        writeData = 0x12345678;

        for (y = 0; y < 256000; y++)
        {
            ithWriteRegA(0xC8000000 + (y * 4), writeData);
            //ithWriteRegA(0xC8000000 + (y * 4), writeData);
            usleep(1);
        }

        usleep(10);

        for (y = 0; y < 256000; y++)
        {
            usleep(10);
            readData = ithReadRegA(0xC8000000 + (y * 4));

            if (readData != writeData)
            {
                printf("reg:0x%x readData:0x%x, writeData:0x%x\n", 0xC8000000 + (y * 4), readData, writeData);
                printf("....................data compare fail.\n");
                while (1);
            }
         }

         count++;
         printf(".........success(%d).\n", count);

#endif        
    }
    return NULL;
}

