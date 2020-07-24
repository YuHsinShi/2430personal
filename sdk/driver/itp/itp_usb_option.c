/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 * @version 1.0
 */
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/ite_usboption.h"

/* command mode NOT VERIFIED!!! */
//#define SUPPORT_AT_CMD

struct itp_option {
    uint32_t cmd_mode : 1;
    /* for I/O mode */
    uint32_t rx_waiting : 1;
    uint32_t rx_ready : 1;
    uint32_t closing : 1;

    uint8_t *rx_buf;
    uint32_t rx_buf_len;
    uint32_t rx_size;
    sem_t io_rx_sem;

    /* for command mode */
#if defined(SUPPORT_AT_CMD)
    ITPAtCmd *cmd;
    sem_t cmd_sem;
#endif
};

static struct itp_option ctxt;
/* rx tmp buffer for I/O mode */
#define RX_BUF_SIZE     4096
static uint8_t  rx_buf[RX_BUF_SIZE];
static uint32_t rx_len;
static pthread_mutex_t rx_mutex = PTHREAD_MUTEX_INITIALIZER;

#if defined(SUPPORT_AT_CMD)

#define AT_CMD_TIMEOUT      5000

static int UsbOptionAtCmd(ITPAtCmd *cmd)
{
    int rc = 0;

    if (cmd->cmd_len == 0) 
        cmd->cmd_len = strlen(cmd->cmd_buf);

    if (cmd->cmd_len > ITP_AT_CMD_LEN) {
        rc = ITP_AT_ERR_CMD_LEN;
        goto end;
    }

    ctxt.cmd = cmd;
    rc = iteUsbOptionWrite(cmd->cmd_buf, cmd->cmd_len);
    if (rc)
        goto end;

    if (cmd->flags & ITP_AT_WAIT_ACK) {
        rc = itpSemWaitTimeout(&ctxt.cmd_sem, (cmd->timeout ? cmd->timeout : AT_CMD_TIMEOUT));
        if (rc) {
            printf("usb_option: %s wait ack timeout \n", cmd->cmd_buf);
            rc = ITP_AT_ERR_CMD_TIMEOUT;
            goto end;
        }
    }

end:
    return rc;
}

static void AtcmdRxProcess(void *arg, const unsigned char *chars, size_t size)
{
    struct itp_option *ctxt = (struct itp_option*)arg;
    ITPAtCmd *cmd = ctxt->cmd;
    bool sem_done = false;

    if (cmd == NULL)
        return;

    printf("usb_option: rx size %d \n", size);
    printf("%s", chars);
    if (cmd->flags & ITP_AT_GET_RESP) {
        if (cmd->resp_len + (uint32_t)size <= ITP_AT_RESP_LEN) {
            strcpy(cmd->resp_buf + cmd->resp_len, chars);
            cmd->resp_len += (uint32_t)size;
        }
        else
            printf("usb_option: resp_buf not enough!!!\n");
    }

    if (cmd->flags & ITP_AT_WAIT_ACK) {
        if (strstr(chars, "OK") != NULL) {
            cmd->status = 0;
            sem_done = true;
        }
        else if (strstr(chars, "ERROR") != NULL) {
            cmd->status = ITP_AT_STS_ERROR;
            if (size > ITP_AT_ERR_LEN)
                printf("usb_option: [ERR] error string buffer size not enough! (%d > %d)", size, ITP_AT_ERR_LEN);
            strcpy(cmd->err_buf, chars);
            sem_done = true;
        }

        if (sem_done == true)
            goto end;
    }

end:
    if (sem_done == true)
        sem_post(&ctxt->cmd_sem);
    return;
}
#endif // SUPPORT_AT_CMD

static void _rx_cb(void *arg, unsigned char *chars, size_t size)
{
    //chars[size] = '\0';

    /* cmd mode */
    if (ctxt.cmd_mode) {
#if defined(SUPPORT_AT_CMD)
        AtcmdRxProcess(arg, chars, size);
#endif
    }
    /* I/O mode */
    else {
        pthread_mutex_lock(&rx_mutex);
        /* rx buffer is ready */
        if (ctxt.rx_waiting) {
            if (size > ctxt.rx_buf_len) {
                printf("usb_option: rx size:%d > buffer size:%d (line:%d)\n", size, ctxt.rx_buf_len, __LINE__);
                size = ctxt.rx_buf_len;
            }
            memcpy((void*)ctxt.rx_buf, (void*)chars, size);
            ctxt.rx_size = size;
            ctxt.rx_waiting = 0;
            pthread_mutex_unlock(&rx_mutex);
            sem_post(&ctxt.io_rx_sem);
            return;
        }
        /* rx buffer not ready, so copy to tmp buffer */
        else {
            //printf("usb_option: rx buffer not ready. %d \n", size);
            memcpy((void*)(rx_buf + rx_len), (void*)chars, size);
            rx_len += size;
            if (rx_len > RX_BUF_SIZE) {
                printf("usb_option: [err] rx tmp buffer size too small!!! ==> TODO!! \n");
                while (1);
            }
            ctxt.rx_ready = 1;
            pthread_mutex_unlock(&rx_mutex);
            return;
        }

    }
}

/* only for I/O mode */
static int UsbOptionRead(int file, char *ptr, int len, void *info)
{
    if (!iteUsbOptionIsReady())
        return -111;
    if (ctxt.closing)
        return -222;
    if (ctxt.cmd_mode) 
        return -333;

    pthread_mutex_lock(&rx_mutex);

    if (ctxt.rx_ready) {
        int copy_size = rx_len;
        if (rx_len > len) {
            printf("usb_option: rx size:%d > buffer size:%d (line:%d)\n", rx_len, len, __LINE__);
            while (1);
        }
        memcpy((void*)ptr, (void*)rx_buf, rx_len);
        ctxt.rx_ready = 0;
        rx_len = 0;

        pthread_mutex_unlock(&rx_mutex);
        return copy_size;
    }

    ctxt.rx_buf = (uint8_t *)ptr;
    ctxt.rx_buf_len = (uint32_t)len;
    ctxt.rx_waiting = 1;
    ctxt.rx_size = 0;

    pthread_mutex_unlock(&rx_mutex);

    sem_wait(&ctxt.io_rx_sem);

    if (ctxt.closing)
        return -2;

    return ctxt.rx_size;
}

/* only for I/O mode */
static int UsbOptionWrite(int file, char *ptr, int len, void *info)
{
    if (!iteUsbOptionIsReady())
        return -111;
    if (ctxt.closing)
        return -222;
    if (ctxt.cmd_mode) 
        return -333;

    return iteUsbOptionWrite(ptr, len);
}

static int UsbOptionIoctl(int file, unsigned long request, void* ptr, void* info)
{
    switch (request)
    {
    case ITP_IOCTL_ENABLE:
        memset((void*)&ctxt, 0, sizeof(ctxt));
        sem_init(&ctxt.io_rx_sem, 0, 0);
        #if defined(SUPPORT_AT_CMD)
        sem_init(&ctxt.cmd_sem, 0, 0);
        #endif
        iteUsbOptionOpen(_rx_cb, (void*)&ctxt);
        break;

    case ITP_IOCTL_DISABLE:
        if (!ctxt.closing) {
            int val;
            ctxt.closing = 1;
            iteUsbOptionClose();
            rx_len = 0;
            //sem_getvalue(&ctxt.io_rx_sem, &val);
            //printf("io_rx_sem: val %d a\n", val);
            sem_post(&ctxt.io_rx_sem);
            do {
                usleep(1000);
                sem_getvalue(&ctxt.io_rx_sem, &val);
                //printf("io_rx_sem: val %d b\n", val);
            } while (val);
            sem_destroy(&ctxt.io_rx_sem);

            #if defined(SUPPORT_AT_CMD)
            sem_post(&ctxt.cmd_sem);
            do {
                usleep(1000);
                sem_getvalue(&ctxt.cmd_sem, &val);
            } while (val);
            sem_destroy(&ctxt.cmd_sem);
            #endif
        }
        break;
		
    case ITP_IOCTL_IS_CONNECTED:
        if (ctxt.closing)
            return 0;
        return iteUsbOptionIsReady();

    case ITP_IOCTL_SET_MODE:
        if (((int)ptr) == ITP_USB_OPTION_CMD_MODE) {
            //ctxt.cmd_mode = 1;
            printf("usb_option: currently not support command mode!!!!!\n");
            return -1;
        }
        if (((int)ptr) == ITP_USB_OPTION_IO_MODE)
            ctxt.cmd_mode = 0;
        break;

#if defined(SUPPORT_AT_CMD)
    case ITP_IOCTL_AT_CMD:
        return UsbOptionAtCmd((ITPAtCmd*)ptr);
        break;
#endif

    default:
        errno = (ITP_DEVICE_USBOPTION << ITP_DEVICE_ERRNO_BIT) | 1;
        return -1;
    }
    return 0;
}


const ITPDevice itpDeviceUsbOption =
{
    ":usb_option",
    itpOpenDefault,
    itpCloseDefault,
    UsbOptionRead,
    UsbOptionWrite,
    itpLseekDefault,
    UsbOptionIoctl,
    NULL
};



