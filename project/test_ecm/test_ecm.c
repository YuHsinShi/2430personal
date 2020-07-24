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

#define DHCP_TIMWOUT   (60*1000*1000)

int dhcp_ready = 0;

static int NetworkInit(void)
{
    ITPEthernetSetting setting = { 0 };
    ITPEthernetInfo info = { 0 };
    uint32_t dhcp_timeout = DHCP_TIMWOUT;
    char ip[16] = { 0 };

    /* wait link up */
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL)) {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }
    printf("Network connected.. \n");

    /* start dhcp to get ip */
    setting.index = 0;
    setting.dhcp = 1;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);

    /* wait for get ip ready */
    printf("Wait DHCP ready...\n");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL)) {
        usleep(100 * 1000);
        dhcp_timeout -= 100;
        if (!(dhcp_timeout % 1000)) {
            putchar('*');
            fflush(stdout);
        }

        if (dhcp_timeout == 0) {
            printf("[ERR] dhcp timeout................\n");
            return -1;
        }
    }
    printf("Network DHCP ready!\n");

    dhcp_ready = 1;

    /* get IP address */
    info.index = 0;
    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
    ipaddr_ntoa_r((const ip_addr_t*)&info.address, ip, sizeof(ip));
    printf("\n\nGet IP: %s \n\n", ip);

    return 0;
}

#if defined(CFG_TEST_PING)

static void DoPing(void)
{
    //printf("\n\nping: 172.217.160,78 \n\n");
    //ping_set_target("172.217.160.78"); // google(172.217.160.78)
    printf("\n\nping: %s \n\n", CFG_PING_IP);
    ping_set_target(CFG_PING_IP);
    ping_init();
}

#endif // #if defined(CFG_TEST_PING)

#if defined(CFG_TEST_HTTP)

//#define HTTP_URL "http://soc.ite.com.tw/index.php/instruction/file/SDK%20Training%20Video/101A_iTE_SDK_Learning%252Emov"
#define HTTP_URL	CFG_HTTP_URL

static uint32_t http_file_size;
static uint32_t http_down_size;

/* get length from header */
static size_t HttpGetHeader(void *buffer, size_t size, size_t nmemb, void *stream)
{
    int     length;
    char    *pch;

    (void *)stream;

    pch = strstr(buffer, "Content-Length:");
    if (pch) {
        sscanf(pch + 16, "%d", &length);
        if (length >= 0) {
            printf("\n\n========================================\n\n");
            printf("\n\nHttp download: content length = %d bytes \n\n", length);
            http_file_size = length;
        }
    }

    return (size * nmemb);
}

static size_t HttpWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    size_t s;
    static uint32_t cnt = 0;

    (void *)stream;
    (void *)buffer;

    s = size * nmemb;

    http_down_size += s;
    if (!(cnt++ % 100)) {
        putchar('#');
        fflush(stdout);
    }
    if (http_down_size == http_file_size)
        printf("\n\nHttp download finish!! \n\n");

    return s;
}

static void DoHttpDownload(void)
{
    CURL *curl;
    CURLcode res;
    uint32_t startTime, download_time, bit_rate;

    printf("\n\nDo Http Download....\n\n");

    http_down_size = 0;
    http_file_size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        printf("curl_easy_init() fail! \n");
        goto end;
    }
    curl_easy_setopt(curl, CURLOPT_URL, HTTP_URL);
    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    /* get header */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HttpGetHeader);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, NULL);
    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    startTime = itpGetTickCount();
    res = curl_easy_perform(curl);
    if (res)
        printf("curl_easy_perform() err: %s \n", curl_easy_strerror(res));

    download_time = itpGetTickDuration(startTime); // ms
    bit_rate = (((double)(http_file_size * 8 )/ 1024) / download_time) * 1000;
    printf("Download time: %u ms (%d kbps)\n", download_time, bit_rate);
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();

end:
    return;
}

#endif

extern int Dial_io_mode_L718(void);
extern int Dial_io_mode_N720(void);
extern int Dial_io_mode_Air720SL(void);

#if defined(CFG_NET_L718_4G)
#define Dial    Dial_io_mode_L718
#elif defined(CFG_NET_N720_4G)
#define Dial    Dial_io_mode_N720
#elif defined(CFG_NET_AIR720SL_4G)
#define Dial    Dial_io_mode_Air720SL
#endif

void* TestFunc(void* arg)
{
    int rc;

    itpInit();

#if defined(CFG_USB_OPTION)
    rc = Dial();
    if (rc) {
        printf("Dial fail! \n");
        while (1);
    }
#endif

#if defined(CFG_USB_ECM)
    rc = NetworkInit();
    if (rc) {
        printf("NetworkInit() fail! \n");
        while (1);
    }

#if defined(CFG_TEST_PING)
    DoPing();
#elif defined(CFG_TEST_HTTP)
    while (1)
        DoHttpDownload();
#endif
#endif // #if defined(CFG_USB_ECM)

    while (1)
        sleep(1);
}
