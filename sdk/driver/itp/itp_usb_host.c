/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * PAL USB Host related code.
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <malloc.h>

struct usb_intf {
    int     type;
    void*   ctxt;  /* usb device's context that currently connected */
};

typedef struct
{
    int     index;
    bool    connected;
    struct usb_intf intf[USB_MAX_INTERFACE];
    int     sleep_mode:1;
    int     sleep_ready:1;
} ITPUsb;


#if defined(CFG_MSC_ENABLE)
    #include "itp_usb_msc.c"
#endif

#if defined(CFG_NET_WIFI)
    #include "ite/ite_wifi.h"
#endif


static ITPUsb itpUsb[2] = { { .index = USB0 }, { .index = USB1 } };

#if defined(CFG_USB_MOUSE)
extern bool usb_mouse_insert;
#endif

#if defined(CFG_USB_ECM)
static int socket_inited = 0;
#endif

extern caddr_t __heap_start__;
extern caddr_t __heap_end__;

#if 0
static void heapStatHandler(void)
{
#if 0
    struct mallinfo mi = mallinfo();
    unsigned long total = (unsigned int)&__heap_end__ - (unsigned int)&__heap_start__;

    // heap usage
    printf("HEAP newlib: usage=%d/%d(%d%%),addr=0x%X\n",
        mi.uordblks,
        total,
        (int)(100.0f * mi.uordblks / total),
        &__heap_start__);
#endif

#ifdef CFG_DBG_RMALLOC
    Rmalloc_stat(__FILE__);
#endif

}
#endif

static int _CheckUsbState(ITPUsb* itpusb)
{
    static USB_DEVICE_INFO device_info[USB_MAX_INTERFACE];
    static int usb_state = 0;
    int res, i;
    int idx = itpusb->sleep_mode ? (itpusb->index | 0x10) : itpusb->index;

    res = mmpUsbExCheckDeviceState(idx, &usb_state, device_info);
    if(!res)
    {
        if(USB_DEVICE_CONNECT(usb_state))
        {
            itpusb->connected = true;
            for (i = 0; i < USB_MAX_INTERFACE; i++)
            {
                if (!device_info[i].ctxt)
                    break;

                itpusb->intf[i].type = device_info[i].type;
                itpusb->intf[i].ctxt = device_info[i].ctxt;

                #if defined(CFG_UAS_ENABLE)
                UasConnect(itpusb, i);
                #endif

                #if defined(CFG_MSC_ENABLE)
                MscConnect(itpusb, i);
                #endif

                #if defined(CFG_NET_WIFI)
                WifiConnect(itpusb->intf[i].type);
                #endif

                #if defined(CFG_DEMOD_USB_INDEX)
                if (USB_DEVICE_DEMOD(itpusb->intf[i].type))
                    printf(" USB : Demodulator device is interted!! \n");
                #endif

                #if defined(CFG_USB_MOUSE)
                if (USB_DEVICE_MOUSE(itpusb->intf[i].type)) {
                    LOG_INFO " USB%d: usb mouse is interted!! \n", itpusb->index LOG_END
                    usb_mouse_insert = true;
                }
                #endif 

                #if defined(CFG_USB_KBD)
                if (USB_DEVICE_KBD(itpusb->intf[i].type))
                    LOG_INFO " USB%d: usb keyboard is interted!! \n", itpusb->index LOG_END
                #endif 

                #if defined(CFG_USB_OPTION)
                if (USB_DEVICE_OPTION(itpusb->intf[i].type)) {
                    LOG_INFO " USB%d: usb OPTION is interted!! \n", itpusb->index LOG_END
                    ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_ENABLE, NULL);
                }
                #endif 

                #if defined(CFG_USB_ECM)
                if (USB_DEVICE_ECM(itpusb->intf[i].type)) {
                    LOG_INFO " USB%d: usb ECM is interted!! \n", itpusb->index LOG_END
                    if (!socket_inited) {
                        itpRegisterDevice(ITP_DEVICE_SOCKET, &itpDeviceSocket);
                        ioctl(ITP_DEVICE_SOCKET, ITP_IOCTL_INIT, NULL);
                        socket_inited = 1;
                    } 
                    else { // for power sleep
                        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESUME, NULL);
                    }
                }
                #endif 
            }
        }
        if(USB_DEVICE_DISCONNECT(usb_state))
        {
            for (i = 0; i < USB_MAX_INTERFACE; i++)
            {
                if (itpusb->intf[i].ctxt == NULL)
                    break;

                #if defined(CFG_UAS_ENABLE)
                UasDisconnect(itpusb, i);
                #endif

                #if defined(CFG_MSC_ENABLE)
                MscDisconnect(itpusb, i);
                #endif

                #if defined(CFG_NET_WIFI)
                WifiDisconnect(itpusb->intf[i].type);
                #endif

                #if defined(CFG_DEMOD_USB_INDEX)
                if (USB_DEVICE_DEMOD(itpusb->intf[i].type))
                {
                    printf(" USB : Demodulator device is removed!! \n");
                }
                #endif

                #if defined(CFG_USB_MOUSE)
                if (USB_DEVICE_MOUSE(itpusb->intf[i].type)) {
                    LOG_INFO " USB%d: usb mouse is disconnected!! \n", itpusb->index LOG_END
                    usb_mouse_insert = false;
                }
                #endif 

                #if defined(CFG_USB_KBD)
                if (USB_DEVICE_KBD(itpusb->intf[i].type))
                    LOG_INFO " USB%d: usb keyboard is disconnected!! \n", itpusb->index LOG_END
                #endif 

                #if defined(CFG_USB_OPTION)
                if (USB_DEVICE_OPTION(itpusb->intf[i].type)) {
                    LOG_INFO " USB%d: usb OPTION is disconnected!! \n", itpusb->index LOG_END
                    //ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_DISABLE, NULL);
                }
                #endif 

                #if defined(CFG_USB_ECM)
                if (USB_DEVICE_ECM(itpusb->intf[i].type))
                    LOG_INFO " USB%d: usb ECM is disconnected!! \n", itpusb->index LOG_END
                #endif 

                itpusb->intf[i].ctxt = NULL;
                itpusb->intf[i].type = 0;
            }

            itpusb->connected = false;
            if (itpusb->sleep_mode)
                itpusb->sleep_ready = 1;

			//heapStatHandler();
        }
    }

end:
    return res;
}

static void* UsbHostDetectHandler(void* arg)
{
    while(1)
    {
    #if defined(CFG_USB0_ENABLE)
        if(_CheckUsbState(&itpUsb[USB0]))
            LOG_ERR " itp usb0 check state fail! \n" LOG_END
    #endif

    #if defined(CFG_USB1_ENABLE)
        if(_CheckUsbState(&itpUsb[USB1]))
            LOG_ERR " itp usb1 check state fail! \n" LOG_END
    #endif

        usleep(30*1000);
    }
}

static void UsbHostGetInfo(ITPUsbInfo* usbInfo)
{
    /* TODO: !!! multi-interface should check here !!! */
    usbInfo->ctxt = itpUsb[usbInfo->usbIndex].intf[0].ctxt;
    usbInfo->type = itpUsb[usbInfo->usbIndex].intf[0].type;
}
