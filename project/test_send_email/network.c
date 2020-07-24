#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "doorbell.h"
#include "ite/itp.h"
#include "curl/curl.h"

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static bool networkIsReady;
static int networkSocket;
static bool networkToReset, networkQuit;
static ITPEthernetInfo networkInfo;
static ITPEthernetSetting settingCopy;


static  struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
// wifi init
static int gInit =0;

/* This is a simple example showing how to send mail using libcurl's SMTP
 * capabilities. For an example of using the multi interface please see
 * smtp-multi.c.
 *
 * Note that this example requires libcurl 7.20.0 or above.
 */
#define MAIL_SERVER "smtp://smtp.gmail.com:587"
#define USERNAME  "nicktestXXXX@gmail.com"
#define PASSWORD "XXXXXXXXXXXX"

#define FROM    "<XXXXXXXX@gmail.com>"
#define TO      "<XXXXXXXX@ite.com.tw>"
#define CC      "<XXXXXXXX@gmail.com>"

static const char *payload_text[] = {
  "Date: Mon, 30 Nov 2015 21:55:40 +1100\r\n",
  "To: " TO "\r\n",
  "From: " FROM "(Example User)\r\n",
  "Cc: " CC "(Another example User)\r\n",
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
  "rfcpedant.example.org>\r\n",
  "Subject: SMTP example message\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */
  "The body of the message starts here.\r\n",
  "\r\n",
  "´�ail µo«H\r\n",
  NULL
};

struct upload_status {
  int lines_read;
};

static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;

  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = payload_text[upload_ctx->lines_read];

  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;

    return len;
  }

  return 0;
}

int send_mail(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;

  upload_ctx.lines_read = 0;
printf("send_mail \n");
  curl = curl_easy_init();
  if(curl) {

   curl_easy_setopt(curl, CURLOPT_URL, MAIL_SERVER);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
    recipients = curl_slist_append(recipients, TO);
    recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);    
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


    /* Send the message */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed %d: %s\n",res,
              curl_easy_strerror(res));

    /* Free the list of recipients */
    curl_slist_free_all(recipients);

    /* curl won't send the QUIT command until you call cleanup, so you should
     * be able to re-use this connection for additional messages (setting
     * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
     * curl_easy_perform() again. It may not be a good idea to keep the
     * connection open for a very long time though (more than a few minutes
     * may result in the server timing out the connection), and you do want to
     * clean up in the end.
     */
    curl_easy_cleanup(curl);
  }

  return (int)res;
}


#if 1
/* Dynamic address change to Static address. */
void NetworkDynamicToStatic(void)
{
	//DHCP is enabled last time, but DHCP is disabled now.
	if ((theConfig.dhcp == 0) && (theConfig.dhcp_ipaddr != NULL)){

		if (!strcmp(theConfig.default_ipaddr, theConfig.ipaddr) ||
				!strcmp(theConfig.dhcp_ipaddr, theConfig.ipaddr))
			strcpy(theConfig.ipaddr, theConfig.default_ipaddr);

	}
}

void ResetAddr(uint8_t* eth_setting, char* config_addr){
	char *saveptr, *pToken = NULL;
	int count = 0;

	pToken = strtok_r(config_addr, ".", &saveptr);
	eth_setting[count] = atoi(pToken);

	while(count < 3){
		count++;

		pToken = strtok_r(NULL, ".", &saveptr);
		eth_setting[count] = atoi(pToken);
	}
}

static void ResetEthernet(void)
{
    ITPEthernetSetting setting[CFG_NET_ETHERNET_COUNT];
    unsigned long      mscnt = 0;
    char               buf[16];
    int                i;

    for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++){
        memset(&setting[i], 0, sizeof (ITPEthernetSetting));

        setting[i].index = i;
    }


    printf("ResetEthernet %d ,%s ,%s ,%s \n",theConfig.dhcp,theConfig.ipaddr,theConfig.netmask,theConfig.gw);

    // dhcp
    setting[0].dhcp   = theConfig.dhcp;

    // autoip
    setting[0].autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    ResetAddr(&setting[0].ipaddr,   &buf);

    // netmask
    strcpy(buf, theConfig.netmask);
    ResetAddr(&setting[0].netmask,  &buf);

    // gateway
    strcpy(buf, theConfig.gw);
    ResetAddr(&setting[0].gw,       &buf);

    if (theConfig.dhcp)
        theConfig.dhcp_ipaddr_writen = false;

	sleep(1); //wait for eXosip_init ready

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);

    printf("Wait ethernet cable to plugin...\n");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        mscnt += 1000;
        putchar('.');
        fflush(stdout);

	
    }


    printf("\nWait DHCP settings");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        usleep(100000);
        mscnt += 100;

        putchar('.');
        fflush(stdout);

        if (mscnt >= DHCP_TIMEOUT_MSEC)
        {
            printf("\nDHCP timeout, use default settings\n");
            setting[CFG_NET_ETHERNET_COUNT - 1].dhcp = setting[CFG_NET_ETHERNET_COUNT - 1].autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting[CFG_NET_ETHERNET_COUNT - 1]);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char *ip;

        for (i = 0; i < CFG_NET_ETHERNET_COUNT; i++)
        {
            networkInfo.index = i;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &networkInfo);
            ip         = ipaddr_ntoa((const ip_addr_t *)&networkInfo.address);

            printf("IP address[%d]: %s\n", i, ip);
        }

        //if (setting[0].dhcp)
            //DeviceInfoInitByDhcp(ip);

        networkIsReady = true;
    }

}

#else

static void ResetEthernet(void)
{
    ITPEthernetSetting setting;
    ITPEthernetInfo info;
    unsigned long mscnt = 0;
    char buf[16], *saveptr;

    memset(&setting, 0, sizeof (ITPEthernetSetting));

    setting.index = 0;
printf("ResetEthernet %d ,%s ,%s ,%s \n",theConfig.dhcp,theConfig.ipaddr,theConfig.netmask,theConfig.gw);
    // dhcp
    setting.dhcp = 0;

    // autoip
    setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));

    ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);

    printf("Wait ethernet cable to plugin");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL))
    {
        sleep(1);
        putchar('.');
        fflush(stdout);
    }

    printf("\nWait DHCP settings");
    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        usleep(100000);
        mscnt += 100;

        putchar('.');
        fflush(stdout);

        if (mscnt >= DHCP_TIMEOUT_MSEC)
        {
            printf("\nDHCP timeout, use default settings\n");
            setting.dhcp = setting.autoip = 0;
            ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET, &setting);
            break;
        }
    }
    puts("");

    if (ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
    {
        char ip[16] = {0};

        info.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &info);
        ipaddr_ntoa_r((const ip_addr_t*)&info.address, ip, sizeof(ip));

        printf("IP address: %s\n", ip);

        networkIsReady = true;
    }
}
#endif

bool NetworkIsReady(void)
{
    return networkIsReady;
}


static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
    int nTemp;
#endif

#ifdef CFG_NET_ETHERNET
    ResetEthernet();
#endif


    send_mail();

    while (!networkQuit)
    {


#ifdef CFG_NET_ETHERNET
        networkIsReady = ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_CONNECTED, NULL);
#endif

        if (networkToReset)
        {

#ifdef CFG_NET_ETHERNET
            ResetEthernet();
#endif

#ifdef CFG_UDP_HEARTBEAT
            NetworkResetHeartbeat();
#endif
            networkToReset = false;
        }

        sleep(1);
    }
    return NULL;
}

void NetworkInit(void)
{
    pthread_t task;

    networkIsReady = false;
    networkToReset = false;
    networkSocket = -1;
    networkQuit             = false;
    
    pthread_create(&task, NULL, NetworkTask, NULL);
}

void NetworkReset(void)
{
    networkToReset  = true;
}
