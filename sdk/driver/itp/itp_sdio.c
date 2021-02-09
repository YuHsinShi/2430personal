/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL SDIO functions.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <errno.h>
#include <string.h>
#include "itp_cfg.h"
#include "ite/ite_sd.h"
#if defined(CFG_NET_WIFI_SDIO_NGPL) || defined (CFG_NET_WIFI_SDIO_SSV)
#include "sys/ioctl.h"
#include "ite/ite_wifi.h"
#endif
#if defined(CFG_ESP32_SDIO_AT)
#include "ite/ite_esp32_sdio_at.h"
#endif

static SD_CARD_INFO sdio_card_info[2];

static void sdio_connect(int sdc_idx, SD_CARD_INFO *card_info)
{
    int i;

    if (card_info->type < SD_TYPE_SDIO)
        return;

    memcpy((void*)&sdio_card_info[sdc_idx], (void*)card_info, sizeof(SD_CARD_INFO));

    for (i = 0; i < card_info->sdio_drv_num; i++) {
        switch (card_info->sdio_info[i].func_type) {
        case SDIO_AMEBA:
            printf(" SDIO %d: found Ameba SDIO driver! \n", sdc_idx);
            break;
#ifdef CFG_ESP32_SDIO_AT
		case SDIO_ESP32_WROOM_32_AT:
            esp32_sdio_at_set_ready();
			printf(" SDIO %d: found ESP32 SDIO driver! \n", sdc_idx);
			break;
#endif
        default:
            break;
        }
    }
}

static void sdio_disconnect(int sdc_idx)
{
    int i;
    SD_CARD_INFO *card_info = &sdio_card_info[sdc_idx];

    for (i = 0; i < card_info->sdio_drv_num; i++) {
        switch (card_info->sdio_info[i].func_type) {
        case SDIO_AMEBA:
            printf(" SDIO %d: remove Ameba SDIO driver! \n", sdc_idx);
            break;
#ifdef CFG_ESP32_SDIO_AT
		case SDIO_ESP32_WROOM_32_AT:
            printf(" SDIO %d: remove ESP32 SDIO driver! \n", sdc_idx);
			break;
#endif

        default:
            break;
        }
    }

    memset((void*)card_info, 0x0, sizeof(SD_CARD_INFO));
}

#if defined(CFG_NET_AMEBA_SDIO)
extern int iteAmebaSdioWifiRegister(void);
#endif

static int driver_registered;

static int sdio_init(void)
{

    int rc = 0;
    SD_CARD_INFO card_info = { 0 };
    if (!driver_registered) {
        #if defined(CFG_NET_AMEBA_SDIO)
        iteAmebaSdioWifiRegister();
        #endif
		#if defined(CFG_ESP32_SDIO_AT)
        esp32_sdio_at_register();
		#endif
        driver_registered = 1;
    }
#if defined(CFG_NET_WIFI_SDIO_NGPL)
    mmpRtlWifiDriverRegister();
#elif defined (CFG_NET_WIFI_SDIO_SSV)
    mmpSsvWifiDriverRegister();
#endif

    #if defined(CFG_SDIO0_STATIC)
    rc = iteSdcInitialize(SD_0, &card_info);
    if (rc)
        printf("iteSdcInitialize(SD_0) fail! \n");
    sdio_connect(SD_0, &card_info);
    #endif

    #if defined(CFG_SDIO1_STATIC)
    rc = iteSdcInitialize(SD_1, &card_info);
    if (rc)
        printf("iteSdcInitialize(SD_1) fail! \n");
    sdio_connect(SD_1, &card_info);
    #endif

    return rc;
}

static int sdio_exit(void)
{
    int rc = 0;

#if defined(CFG_NET_WIFI_SDIO_NGPL) || defined (CFG_NET_WIFI_SDIO_SSV)
    ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_EXIT, NULL);
#endif

    #if defined(CFG_SDIO0_STATIC)
    rc = iteSdcTerminate(SD_0);
    if (rc)
        printf("iteSdcTerminate(SD_0) fail! \n");
    sdio_disconnect(SD_0);
    #endif

    #if defined(CFG_SDIO1_STATIC)
    rc = iteSdcTerminate(SD_1);
    if (rc)
        printf("iteSdcTerminate(SD_1) fail! \n");
    sdio_disconnect(SD_1);
    #endif

    return rc;
}

static int SdioIoctl(int file, unsigned long request, void* ptr, void* info)
{
    int rc;

    switch (request)
    {
    case ITP_IOCTL_INIT:
		/*
		#define GPIO_WIFI_EN  46
		//#define GPIO_SD2  46

		//ithGpioSetOut(GPIO_SD2);
		//ithGpioSetMode(GPIO_SD2, ITH_GPIO_MODE0);
		//ithGpioClear(GPIO_SD2);



		ithGpioSetOut(GPIO_WIFI_EN);
		ithGpioSetMode(GPIO_WIFI_EN, ITH_GPIO_MODE0);
		ithGpioClear(GPIO_WIFI_EN);
		usleep(50*1000);		
		ithGpioSet(GPIO_WIFI_EN);		
		usleep(50*1000);	
*/




		
        rc = sdio_init();
        if (rc)
            return -1;
        break;

    case ITP_IOCTL_EXIT:
        rc = sdio_exit();
        if (rc)
            return -1;
        break;

    case ITP_IOCTL_ON:
        {
            SD_CARD_INFO *card_info = (SD_CARD_INFO *)ptr;
            sdio_connect(card_info->sdc_idx, card_info);
        }
        break;

    case ITP_IOCTL_OFF:
        sdio_disconnect((int)ptr);
        break;

    default:
        return -1;
    }
    return 0;
}

const ITPDevice itpDeviceSdio =
{
    ":sdio",
    itpOpenDefault,
    itpCloseDefault,
    itpReadDefault,
    itpWriteDefault,
    itpLseekDefault,
    SdioIoctl,
    NULL
};

