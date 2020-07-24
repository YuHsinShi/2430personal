#include <sys/ioctl.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "lwip/ip.h"
#include "ping.h"
#include "curl/curl.h"
#include "ite/itp.h"


#define RX_BUF_LEN      (256+128)
#define RX_BUF_LEN_1    512

static pthread_t rx_task;
static char rxbuf[RX_BUF_LEN];
static char rxbuf_1[RX_BUF_LEN_1];
static int rxbuf_1_len;
static bool ready = false;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void* receive_task(void* arg)
{
    int len;

    while (ready == false) usleep(1000);

    for (;;) {
        len = read(ITP_DEVICE_USBOPTION, rxbuf, RX_BUF_LEN);
        pthread_mutex_lock(&mutex);
        if (len > 0) {
            if (len >= RX_BUF_LEN)
                printf("receive_task: len %d >= RX_BUF_LEN %d \n", len, RX_BUF_LEN);
            //rxbuf[len] = '\0';
            //printf("%s\n", rxbuf);
            printf("recv: \n");
            ithPrintVram8((uint32_t)rxbuf, len);
        }
        pthread_mutex_unlock(&mutex);

        if (len < 0) {
            //printf("receive_task: --....%d \n", len);
            while (!ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_IS_CONNECTED, NULL))
                sleep(1);
            // printf("receive_task: ++....\n");
        }
    }

end:
    return NULL;
}


void* TestFunc(void* arg)
{
    char *string;
    int retry, rc = 0;
    uint8_t *ok;

    itpInit();

    if (!rx_task)
        pthread_create(&rx_task, NULL, receive_task, NULL);

    /* wait usb serial ready */
    printf("wait usb option driver ready!\n");
    while (!ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_IS_CONNECTED, NULL)) {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }
    printf("usb option driver ready!\n");

    ioctl(ITP_DEVICE_USBOPTION, ITP_IOCTL_SET_MODE, (void*)ITP_USB_OPTION_IO_MODE);

    ready = true;

do_again:
    sleep(2);

    string = "01";
    printf("\n\n ==> send: %s \n\n", string);
    pthread_mutex_lock(&mutex);
    memset((void*)rxbuf_1, 0, RX_BUF_LEN_1);
    rxbuf_1_len = 0;
    pthread_mutex_unlock(&mutex);

    if (rc = write(ITP_DEVICE_USBOPTION, string, strlen(string)) < 0)
        goto end;

    goto do_again;

end:
    while (1)
        sleep(1);
}
