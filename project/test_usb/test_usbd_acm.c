#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ite/itp.h"

#define BUF_LEN     (8*1024)
static char rbuf[BUF_LEN];

static void* TestLoopback(void* arg)
{
    int rlen, wlen, cnt=0;

wait:
    while (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0) {
        printf(".\n");
        cnt++;
        sleep(1);
#if 0 // test code for force enter usb device mode
        if (cnt == 10)
            ioctl(ITP_DEVICE_USB, ITP_IOCTL_ENABLE, (void *)ITP_USB_FORCE_DEVICE_MODE);
        if (cnt == 20)
            ioctl(ITP_DEVICE_USB, ITP_IOCTL_DISABLE, (void *)ITP_USB_FORCE_DEVICE_MODE);
#endif
    }

    printf("\n\nUSB Acm is ready...... ! \n\n");

    while (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        memset((void*)rbuf, 0x0, BUF_LEN);
        rlen = read(ITP_DEVICE_USBDACM, rbuf, BUF_LEN);
        if (rlen) {
            printf("read: %d \n", rlen);
            //printf("%s\n", rbuf);
            //sprintf(wbuf, "%s", rbuf);
            wlen = write(ITP_DEVICE_USBDACM, rbuf, rlen);
        }
        else {
            printf(" read: 0 \n");
            goto wait;
        }
    }
    goto wait;
    
}


static void* WriteFileToSd(void* arg)
{
    int rlen, wlen;
    char *line, *cmd, *filename;
    char buf[256], len;
    FILE* f;
    int file_size, read_size = 0, ret;

wait:
    while (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL) == 0) {
        printf(".\n");
        sleep(1);
    }

    printf("\n\nUSB Acm is ready...... ! \n\n");

    while (ioctl(ITP_DEVICE_USBDACM, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        printf("wait command..... \n\n");
        sprintf(buf, "\nwait command > send filename filesize \n", filename);
        len = strlen(buf);
        write(ITP_DEVICE_USBDACM, buf, len);

        /* get line */
        rlen = 0;
        memset((void*)rbuf, 0x0, BUF_LEN);
#if 1
        rlen = read(ITP_DEVICE_USBDACM, rbuf, BUF_LEN);
        line = strtok(rbuf, "\n");
#else
        do {
            rlen += read(ITP_DEVICE_USBDACM, rbuf+rlen, BUF_LEN-rlen);
            line = strtok(rbuf, "\n");
        } while (line == NULL);
#endif
        /* parse command */
        cmd = strtok(line, " ");
        filename = strtok(NULL, " ");
        file_size = atoi(strtok(NULL, "\0"));
        printf("cmd:%s, filename:%s (%d) \n", cmd, filename, file_size);

        if (strncmp(cmd, "send", 4) == 0) {
            sprintf(buf, "wait file:%s ......\n", filename);
            len = strlen(buf);
            write(ITP_DEVICE_USBDACM, buf, len);
        }
        else {
            sprintf(buf, "unknown command\n");
            len = strlen(buf);
            write(ITP_DEVICE_USBDACM, buf, len);
            continue;
        }

        /* wait internal storage is ready */
        {
            ITPDriveStatus* driveStatusTable;
            ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

            while (1)
            {
                ITPDriveStatus* driveStatus = &driveStatusTable[0];
                printf("drive A: avail=%d\n", driveStatus->avail);
                if (driveStatus->avail)
                    break;

                sleep(1);
            }
        }

        /* receive and writ to internal storage */
        sprintf(buf, "A:/%s", filename);
        f = fopen(buf, "wb");
        if (!f) {
            printf("open %s fail....\n", buf);
            continue;
        }
            
        read_size = 0;
        while (read_size < file_size) {
            memset((void*)rbuf, 0x0, BUF_LEN);
            rlen = read(ITP_DEVICE_USBDACM, rbuf, BUF_LEN);
            ret = fwrite(rbuf, 1, rlen, f);
            if (ret != rlen)
                printf("fwrite() %d != %d \n", ret, rlen);
            read_size += rlen;
            //printf(" read_size = %d \n", read_size);
        }
        ret = fclose(f);

        sprintf(buf, "file write done! \n", filename);
        len = strlen(buf);
        write(ITP_DEVICE_USBDACM, buf, len);
    }

    goto wait;

    return NULL;
}

static void TestDbg(void)
{
    int i;

    while (1)
    {
        for (i = 0; i < 99999; i++) {
            printf(" %d \n", i);
            sleep(1);
        }
        if (i == 99999)
            i++;
    }
}

void* TestFunc(void* arg)
{
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param param;

#if defined(CFG_TEST_ACM_DBG)
    TestDbg();
#else
    pthread_attr_init(&attr);
    param.sched_priority = 4;
    pthread_attr_setschedparam(&attr, &param);
#if defined(CFG_TEST_ACM_LOOPBACK)
    pthread_create(&thread, &attr, TestLoopback, NULL);
#elif defined(CFG_TEST_ACM_SEND_FILE)
    pthread_create(&thread, &attr, WriteFileToSd, NULL);
#else
    printf("unknow test...\n");
#endif
#endif

    while (1)
        sleep(1);

    return NULL;
}


