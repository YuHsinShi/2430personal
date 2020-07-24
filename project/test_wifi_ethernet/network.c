#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
#include "lwip/ip.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "ite/itp.h"
#include "iniparser/iniparser.h"
#include "doorbell.h"
#include "wifiMgr.h"
#include "ite/itp.h"
#include <netdb.h>
#include "ping.h"
#include <errno.h>

#include "lwip/apps/mqtt.h"

#if 0
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <net/if.h>           // struct ifreq
#include <netdb.h>
#endif

#define DHCP_TIMEOUT_MSEC (60 * 1000) //60sec

static bool networkIsReady;
static int networkSocket;
static bool networkToReset, networkQuit;
static ITPEthernetInfo networkInfo;
static ITPEthernetSetting settingCopy;

//#define TEST_WIFI_DOWNLOAD
//#define LWIP_MQTT
//#define LWIP_MDNS
//#define LWIP_SNTP
//#define LWIP_TFTP

#ifdef TEST_WIFI_DOWNLOAD
#include "curl/curl.h"
#define HTTP_DOWNLOAD_ADDRESS  "http://192.168.191.104:23455/WAV/002-96000-24.wav"
#endif

#ifdef CFG_NET_WIFI
static WIFI_MGR_SETTING gWifiSetting;
#endif

static  struct timeval tvStart = {0, 0}, tvEnd = {0, 0};
// wifi init
static int gInit =0;

#if 0
// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define UDP_HDRLEN  8  // UDP header length, excludes data
#ifndef AI_CANONNAME
#define AI_CANONNAME 2
#endif
#define ETH_P_ALL  0x0003    /* Every packet (be careful!!!) */
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/

// Function prototypes
uint16_t checksum (uint16_t *, int);
uint16_t udp4_checksum (struct ip, struct udphdr, uint8_t *, int);
char *allocate_strmem (int);
uint8_t *allocate_ustrmem (int);
int *allocate_intmem (int);


int
pfpacket_test ()
{
  int i, status, datalen, frame_length, sd, bytes, *ip_flags;
  char *interface, *target, *src_ip, *dst_ip;
  struct ip iphdr;
  struct udphdr udphdr;
  uint8_t *data, *src_mac, *dst_mac, *ether_frame;
  struct addrinfo hints, *res;
  struct sockaddr_in *ipv4;
  struct sockaddr_ll device;
  struct ifreq ifr;
  void *tmp;
    struct sockaddr_in si;
   struct sockaddr_in to_addr;
    ITPWifiInfo wifiInfo;

  // Allocate memory for various arrays.
  src_mac = allocate_ustrmem (6);
  dst_mac = allocate_ustrmem (6);
  data = allocate_ustrmem (IP_MAXPACKET);
  ether_frame = allocate_ustrmem (IP_MAXPACKET);
  interface = allocate_strmem (40);
  target = allocate_strmem (40);
  src_ip = allocate_strmem (INET_ADDRSTRLEN);
  dst_ip = allocate_strmem (INET_ADDRSTRLEN);
  ip_flags = allocate_intmem (4);

  // Interface to send packet through.
  strcpy (interface, "eth0");

  // Submit request for a socket descriptor to look up interface.
  if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    printf ("socket() failed to get socket descriptor for using ioctl() ");
    //exit (EXIT_FAILURE);
  }
#if 0
  // Use ioctl() to look up interface name and get its MAC address.
  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
    printf ("ioctl() failed to get source MAC address ");
    //return (EXIT_FAILURE);
  }
  close (sd);
  // Copy source MAC address.
  memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
  
#endif
  // Copy source MAC address.
#ifdef CFG_NET_WIFI
   ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &wifiInfo);
#else  
   ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &networkInfo);
#endif
  memcpy(src_mac,&wifiInfo.hardwareAddress[0],6 * sizeof (uint8_t));
  
  // Report source MAC address to stdout.
  printf ("MAC address for interface is " );
  for (i=0; i<5; i++) {
    printf ("%02x:", src_mac[i]);
  }
  printf ("%02x\n", src_mac[5]);
usleep(10000);
#if 1
  // Find interface index from interface name and store index in
  // struct sockaddr_ll device, which will be used as an argument of sendto().
  memset (&device, 0, sizeof (device));
  if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
    printf ("if_nametoindex() failed to obtain interface index ");
    //exit (EXIT_FAILURE);
  }
  printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
#endif
  // Set destination MAC address: you need to fill these out
  dst_mac[0] = 0x20;
  dst_mac[1] = 0xcf;
  dst_mac[2] = 0x30;
  dst_mac[3] = 0xe1;
  dst_mac[4] = 0x24;
  dst_mac[5] = 0x44;

  // Source IPv4 address: you need to fill this out
  strcpy (src_ip, "192.168.190.9");

  // Destination URL or IPv4 address: you need to fill this out
  strcpy (target, "192.168.190.2");

  // Fill out hints for getaddrinfo().
  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = hints.ai_flags | AI_CANONNAME;


  // Resolve target using getaddrinfo().
  if ((status = getaddrinfo (target, NULL, &hints, &res)) != 0) {
    printf("getaddrinfo \n");

    fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
    //exit (EXIT_FAILURE);
  }
  ipv4 = (struct sockaddr_in *) res->ai_addr;
  tmp = &(ipv4->sin_addr);


  
  if (inet_ntop (AF_INET, tmp, dst_ip, INET_ADDRSTRLEN) == NULL) {
    status = 0;
    printf("inet_ntop \n");    
    fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror (status));
    //exit (EXIT_FAILURE);
  }

  freeaddrinfo (res);

  // Fill out sockaddr_ll.
  device.sll_family = AF_PACKET;
  memcpy (device.sll_addr, src_mac, 6 * sizeof (uint8_t));
  device.sll_halen = 6;

  // UDP data
  datalen = 4;
  data[0] = 'T';
  data[1] = 'e';
  data[2] = 's';
  data[3] = 't';

  // IPv4 header

  // IPv4 header length (4 bits): Number of 32-bit words in header = 5
  iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);

  // Internet Protocol version (4 bits): IPv4
  iphdr.ip_v = 4;

  // Type of service (8 bits)
  iphdr.ip_tos = 0;

  // Total length of datagram (16 bits): IP header + UDP header + datalen
  iphdr.ip_len = htons (IP4_HDRLEN + UDP_HDRLEN + datalen);

  // ID sequence number (16 bits): unused, since single datagram
  iphdr.ip_id = htons (0);

  // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

  // Zero (1 bit)
  ip_flags[0] = 0;

  // Do not fragment flag (1 bit)
  ip_flags[1] = 0;

  // More fragments following flag (1 bit)
  ip_flags[2] = 0;

  // Fragmentation offset (13 bits)
  ip_flags[3] = 0;

  iphdr.ip_off = htons ((ip_flags[0] << 15)
                      + (ip_flags[1] << 14)
                      + (ip_flags[2] << 13)
                      +  ip_flags[3]);

  // Time-to-Live (8 bits): default to maximum value
  iphdr.ip_ttl = 255;

  // Transport layer protocol (8 bits): 17 for UDP
  iphdr.ip_p = IPPROTO_UDP;

  // Source IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, src_ip, &(iphdr.ip_src))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    //exit (EXIT_FAILURE);
  }

  // Destination IPv4 address (32 bits)
  if ((status = inet_pton (AF_INET, dst_ip, &(iphdr.ip_dst))) != 1) {
    fprintf (stderr, "inet_pton() failed.\nError message: %s", strerror (status));
    //exit (EXIT_FAILURE);
  }

  // IPv4 header checksum (16 bits): set to 0 when calculating checksum
  iphdr.ip_sum = 0;
  iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

  // UDP header

  // Source port number (16 bits): pick a number
  udphdr.source = htons (4950);

  // Destination port number (16 bits): pick a number
  udphdr.dest = htons (4950);

  // Length of UDP datagram (16 bits): UDP header + UDP data
  udphdr.len = htons (UDP_HDRLEN + datalen);


  // UDP checksum (16 bits)
  udphdr.check = udp4_checksum (iphdr, udphdr, data, datalen);

  // Fill out ethernet frame header.

  // Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (IP header + UDP header + UDP data)
  frame_length = 6 + 6 + 2 + IP4_HDRLEN + UDP_HDRLEN + datalen;

  // Destination and Source MAC addresses
  memcpy (ether_frame, dst_mac, 6 * sizeof (uint8_t));
  memcpy (ether_frame + 6, src_mac, 6 * sizeof (uint8_t));

  // Next is ethernet type code (ETH_P_IP for IPv4).
  // http://www.iana.org/assignments/ethernet-numbers
  ether_frame[12] = ETH_P_IP / 256;
  ether_frame[13] = ETH_P_IP % 256;

  // Next is ethernet frame data (IPv4 header + UDP header + UDP data).

  // IPv4 header
  memcpy (ether_frame + ETH_HDRLEN, &iphdr, IP4_HDRLEN * sizeof (uint8_t));

  // UDP header
  memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN, &udphdr, UDP_HDRLEN * sizeof (uint8_t));

  // UDP data
  memcpy (ether_frame + ETH_HDRLEN + IP4_HDRLEN + UDP_HDRLEN, data, datalen * sizeof (uint8_t));

  // Submit request for a raw socket descriptor.
  if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    printf ("socket() failed ");
    //exit (EXIT_FAILURE);
  }

printf("sendto sd %d ,frame length %d \n",sd,frame_length);
usleep(10000);

	  {
		  unsigned char *data = (unsigned char*)ether_frame;
		  int i;
 
		  for(i=0; i<frame_length; i++)
		  {
			  if(!(i%0x10))
				  printf("\n");
			  printf("%02x ", data[i]);
		  }
		  printf("\n\n");
	  }

  // Send ethernet frame to socket.
  
  if ((bytes = sendto (sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
    printf ("sendto() failed");
    //exit (EXIT_FAILURE);
  }
printf("sendto  \n");
usleep(10000);

#if 0
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(4951);
    to_addr.sin_addr.s_addr = inet_addr("192.168.190.2");	
    do {
        if (sendto(sd, "#", 1, 0, &to_addr, sizeof(to_addr) ) < 0) {
            printf("[Rtpproxy] sock %d Could not send datagram!!\n",sd); 
        } else {
           printf("[Rtpproxy]send datagram!!\n"); 
        }
            usleep(2000000);
    } while (1);

#endif    

  // Close socket descriptor.
  close (sd);

  // Free allocated memory.
  free (src_mac);
  free (dst_mac);
  free (data);
  free (ether_frame);
  free (interface);
  free (target);
  free (src_ip);
  free (dst_ip);
  free (ip_flags);

  return (EXIT_SUCCESS);
}


// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t
checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t
udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
{
  char buf[IP_MAXPACKET];
  char *ptr;
  int chksumlen = 0;
  int i;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);

  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);

  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;

  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);

  // Copy UDP length to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP source port to buf (16 bits)
  memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
  ptr += sizeof (udphdr.source);
  chksumlen += sizeof (udphdr.source);

  // Copy UDP destination port to buf (16 bits)
  memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
  ptr += sizeof (udphdr.dest);
  chksumlen += sizeof (udphdr.dest);

  // Copy UDP length again to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  // Copy payload to buf
  memcpy (ptr, payload, payloadlen);
  ptr += payloadlen;
  chksumlen += payloadlen;

  // Pad to the next 16-bit boundary
  for (i=0; i<payloadlen%2; i++, ptr++) {
    *ptr = 0;
    ptr++;
    chksumlen++;
  }

  return checksum ((uint16_t *) buf, chksumlen);
}

// Allocate memory for an array of chars.
char *
allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    //exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    //exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *
allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    //exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    //exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *
allocate_intmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
    //exit (EXIT_FAILURE);
  }

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    //exit (EXIT_FAILURE);
  }
}

#endif


#ifdef TEST_WIFI_DOWNLOAD
static int gnTest =-1;

struct FtpBuf
{
    uint8_t* buf;
    uint32_t pos;
};

static size_t FtpWrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpBuf* out = (struct FtpBuf*)stream;
    size_t s;
/*
    LOG_DEBUG "FtpWrite(0x%X,%d,%d,0x%X)\n", buffer, size, nmemb, stream LOG_END
*/
    // assert(out->buf);

    s = size * nmemb;
    memcpy(&out->buf[out->pos], buffer, s);
    out->pos += s;
    if (out->pos>510000){

        printf("FtpWrite(0x%X,%d,%d,0x%X)\n, buffer, size, nmemb, stream");
        out->pos = 0;
    }

    return s;
}

static int  httpDownload()
{
    CURL *curl;
    CURLcode res;
    struct FtpBuf ftpBuf;

    //InitNetwork();
    printf("[httpDownload] download : start %s\r\n");

    ftpBuf.buf = malloc(0x200000);
    ftpBuf.pos = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        printf("curl_easy_init() fail.\n");
        goto error;
    }

    curl_easy_setopt(curl, CURLOPT_URL, HTTP_DOWNLOAD_ADDRESS);
    //curl_easy_setopt(curl, CURLOPT_USERPWD, FTP_USER_NAME_PASSWORD);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FtpWrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpBuf);

#ifndef NDEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (CURLE_OK != res)
    {
        printf("curl fail: %d\n", res);
        goto error;
    }

    curl_global_cleanup();

    if(ftpBuf.buf){
        free(ftpBuf.buf);
        ftpBuf.buf = NULL;
    }
    return NULL;


    error:
    curl_global_cleanup();
    if(ftpBuf.buf){
        free(ftpBuf.buf);
        ftpBuf.buf = NULL;
    }
    return NULL;

}

static int TestHTTPDownload()
{
    int nRet;
    int bIsAvail;
    do {
    nRet = wifiMgr_is_wifi_available(&bIsAvail);
    if (bIsAvail){
        httpDownload();
    }
    usleep(2000000);
    usleep(2000000);
    printf("ready to download \n\n");
    usleep(2000000);
    } while (1);

}

static int createHttpThread()
{
    pthread_t task;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, (255 * 1024));
    pthread_create(&task, &attr, TestHTTPDownload, NULL);
    return 0;

}


#endif

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



#ifdef CFG_NET_WIFI
static int wifiCallbackFucntion(int nState)
{
    switch (nState)
    {
        case WIFIMGR_STATE_CALLBACK_CONNECTION_FINISH:
            printf("[Indoor]WifiCallback connection finish \n");
            //WebServerInit();
#if defined(CFG_NET_ETHERNET) && defined(CFG_NET_WIFI)
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_RESET_DEFAULT, NULL);
#endif

#ifdef TEST_WIFI_DOWNLOAD
            createHttpThread();
#endif

//ping_set_target("192.168.190.4");
//ping_init();
#ifdef LWIP_MQTT
sleep(6);
mqtt_example_init();
#endif

#ifdef LWIP_MDNS
mdns_example_init();
#endif


#ifdef LWIP_SNTP
sntp_example_init();
#endif

#ifdef LWIP_TFTP
tftp_example_init_server();
#endif

        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_DISCONNECT_30S:
            printf("[Indoor]WifiCallback connection disconnect 30s \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_RECONNECTION:
            printf("[Indoor]WifiCallback connection reconnection \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_TEMP_DISCONNECT:
            printf("[Indoor]WifiCallback connection temp disconnect \n");
        break;

        case WIFIMGR_STATE_CALLBACK_CLIENT_MODE_CONNECTING_FAIL:
            printf("[Indoor]WifiCallback connecting fail, please check ssid,password,secmode \n");
        break;

        default:
            printf("[Indoor]WifiCallback unknown %d state  \n",nState);
        break;

    }


}


static void ResetWifi(void)
{
//    ITPEthernetSetting setting;
    char buf[16], *saveptr;

    memset(&gWifiSetting.setting, 0, sizeof (ITPEthernetSetting));

    gWifiSetting.setting.index = 0;

    // dhcp
    gWifiSetting.setting.dhcp = 1;

    // autoip
    gWifiSetting.setting.autoip = 0;

    // ipaddr
    strcpy(buf, theConfig.ipaddr);
    gWifiSetting.setting.ipaddr[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.ipaddr[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.ipaddr[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // netmask
    strcpy(buf, theConfig.netmask);
    gWifiSetting.setting.netmask[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.netmask[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.netmask[3] = atoi(strtok_r(NULL, " ", &saveptr));

    // gateway
    strcpy(buf, theConfig.gw);
    gWifiSetting.setting.gw[0] = atoi(strtok_r(buf, ".", &saveptr));
    gWifiSetting.setting.gw[1] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[2] = atoi(strtok_r(NULL, ".", &saveptr));
    gWifiSetting.setting.gw[3] = atoi(strtok_r(NULL, " ", &saveptr));
}
#endif


static void* NetworkTask(void* arg)
{
#ifdef CFG_NET_WIFI
    int nTemp;
#endif

#ifdef CFG_NET_ETHERNET
    ResetEthernet();
#endif

#ifdef TEST_HTTPS_DOWNLOAD
    CurlGetHttpsExample();
#endif
printf("NetworkTask ------------- \n");


    while (!networkQuit)
    {
#ifdef CFG_NET_WIFI
        gettimeofday(&tvEnd, NULL);

        nTemp = itpTimevalDiff(&tvStart, &tvEnd);
        if (nTemp > 5000 && gInit == 0)
        {
            printf("init wifi \n");

            nTemp = wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
            
            gInit = 1;
        }
        else if (gInit == 1)
        {
            networkIsReady = wifiMgr_is_wifi_available(&nTemp);
            networkIsReady = (bool)nTemp;
        }
#endif


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

#ifdef CFG_NET_WIFI
    snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ssid);
    snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.password);
#ifdef CFG_NET_WIFI_SDIO_NGPL
     gWifiSetting.secumode = WifiMgr_Secu_ITE_To_8189F(theConfig.secumode);
     if (theConfig.wifi_mode == WIFI_SOFTAP){
        snprintf(gWifiSetting.ssid , WIFI_SSID_MAXLEN, theConfig.ap_ssid);
        snprintf(gWifiSetting.password, WIFI_PASSWORD_MAXLEN, theConfig.ap_password);
     }
#else
    snprintf(gWifiSetting.secumode, WIFI_SECUMODE_MAXLEN, theConfig.secumode);
#endif
#endif

    gWifiSetting.wifiCallback = wifiCallbackFucntion;
    

    gettimeofday(&tvStart, NULL);

    pthread_create(&task, NULL, NetworkTask, NULL);
}

void NetworkReset(void)
{
    networkToReset  = true;
}
