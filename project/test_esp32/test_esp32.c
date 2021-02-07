#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ite/ite_esp32_sdio_at.h"

static void DoScan()
{
#define BUF_SIZE    (2048+1024)
    struct esp32_cmd at_cmd;
    struct esp32_data data= { 0 };
    int rc;

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CWLAP\r\n");
    at_cmd.flags = (ESP32_DATA_READ | ESP32_WAIT_ACK);
    at_cmd.timeout = 5000;
    data.buf = (uint8_t*)malloc(BUF_SIZE);
    memset((void*)data.buf, 0x0, BUF_SIZE);
    data.length = BUF_SIZE;
    data.header_len = 7;  // "+CWLAP:"
    at_cmd.data = (void*)&data;

    esp32_sdio_at_lock();
    rc = esp32_at_cmd(&at_cmd);
    esp32_sdio_at_unlock();
    if (rc)
        goto end;

    if (at_cmd.status)
        printf("scan status: %d \n", at_cmd.status);

    printf("%s \n", data.buf);

end:
    free(data.buf);
}

static sem_t sem_download_ready;

static void http_download_cb(uint8_t *buf, uint32_t len)
{
    static uint32_t file_size;
    static uint32_t total_rsize;
    static uint32_t cnt;

    if (!file_size) {
        uint8_t *len_start, *len_end;
        if ((len_start = strstr(buf, "Content-Length: ")) != NULL) {
            uint8_t c_len[10] = "";
            len_start += 16;
            len_end = strstr(len_start, "\r\n");
            memcpy(c_len, len_start, len_end - len_start);
            file_size = atoi(c_len);
            printf("file size: %d \n", file_size);
            total_rsize = 0;
            cnt = 0;
        }
        else {
            printf("can't find \"Content-Length\"");
            while (1);
        }
    }
    else {
        total_rsize += len;
        if ((cnt++ % 200) == 0)
            printf("read: %d \n", total_rsize);
        if (total_rsize == file_size) {
            printf("download finish! \n");
            esp32_at_cmd_ipd_end();
            file_size = 0;
            sem_post(&sem_download_ready);
        }
    }
}

static void DoHttpDownload(void)
{
    int rc;
    struct esp32_cmd at_cmd;

    sem_init(&sem_download_ready, 0, 0);

    esp32_sdio_at_lock();

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CIPSTART=\"TCP\",\"192.168.191.102\",80\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;  // wait "OK" or "ERROR"
    at_cmd.timeout = 5000;
    rc = esp32_at_cmd(&at_cmd);
    if (rc || at_cmd.status) {
        printf("TCP start fail! \n");
        goto end;
    }

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CIPSEND=30\r\n");  // next HTTP cmd's length
    at_cmd.flags = ESP32_WAIT_FOR_SEND;  // wait '>'
    esp32_at_cmd(&at_cmd);

    /* 
       It will return:
       +IPD,324:HTTP/1.1 200 OK
       Content-Type: application/octet-stream
       Content-Length: 56268228
       Accept-Ranges: bytes
       Server: HFS 2.3m
       Set-Cookie: HFS_SID_=0.480718132574111; path=/; HttpOnly
       ETag: 6B790226D4090AA0EFADF56707EF27FA
       Last-Modified: Thu, 16 Jun 2016 02:08:49 GMT
       Content-Disposition: attachment; filename="test.rar";
    */
	memset(&at_cmd, 0, sizeof(at_cmd));
	strcpy(at_cmd.cmd_buf, "GET /ITEPKG04.PKG HTTP/1.1\r\n\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;  // wait "SEND OK"
    at_cmd.ipd_cb = http_download_cb;
    esp32_at_cmd(&at_cmd);


    sem_wait(&sem_download_ready);
    sem_destroy(&sem_download_ready);

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CIPCLOSE\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;  // wait OK
    esp32_at_cmd(&at_cmd);

end:
    esp32_sdio_at_unlock();
}

void* TestFunc(void* arg)
{
    struct esp32_cmd at_cmd;
    struct esp32_data data;
#define TEST_BUF_SIZE   128
    uint8_t buf[TEST_BUF_SIZE] = { 0 };

    itpInit();

    do {
        usleep(1000);
    } while (esp32_sdio_at_is_ready() == 0);

    esp32_sdio_at_lock();

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+GMR\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;
    esp32_at_cmd(&at_cmd);

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CWMODE=1\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;
    esp32_at_cmd(&at_cmd);

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CWMODE?\r\n");
	memset((void*)&data, 0x0, sizeof(struct esp32_data));
	at_cmd.flags = (ESP32_DATA_READ | ESP32_WAIT_ACK);
	data.buf = (uint8_t*)buf;
	memset((void*)data.buf, 0x0, TEST_BUF_SIZE);
	data.length = TEST_BUF_SIZE;
	data.header_len = 8;  // "+CWMODE:"
	at_cmd.data = (void*)&data;
	esp32_at_cmd(&at_cmd);
	printf("return from driver: %s \n", data.buf);

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;
    at_cmd.timeout = 10000;
    esp32_at_cmd(&at_cmd);
    if (at_cmd.status) {
        printf("join AP fail! \n");
        while (1);
    }

	memset(&at_cmd, 0, sizeof(at_cmd));
    strcpy(at_cmd.cmd_buf, "AT+CIFSR\r\n");
    at_cmd.flags = ESP32_WAIT_ACK;
    esp32_at_cmd(&at_cmd);

    esp32_sdio_at_unlock();

    DoScan(&at_cmd);
    DoHttpDownload();

    while (1) 
        sleep(1);
}