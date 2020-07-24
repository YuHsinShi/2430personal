#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ortp/str_utils.h>
#include "ite/itc.h"
#include "ite/itp.h"
#include "ite/itv_iptv.h"
#include "libavutil/avstring.h"
#include "avformat.h"
//#include "../../../../project/multicast/iptv_userconfig.h"

#define CFG_IP_SECURITY_MODE

#define MULTICAST_PORT  5004
#define MULTICAST_GROUP "239.255.42.42"
#if 0
#define BUF_SIZE            (1472 * 1000) //(1316 * 1000)
#define WRITE_SIZE          1472 //1316
#define READ_SIZE           (1472 * 100) //(1316 * 100)
#else
#define BUF_SIZE            (((1280*1024)/1316)*1316) //(1316 * 20000)
#define WRITE_SIZE          1316
#define READ_SIZE           (1316*1000) // (1316 * 2000)
#define ETHMTU_SIZE			1500
#define DEC_ALIGN_SIZE      1312
#endif
//#define TX_BITRATE        (15 * 1024 * 1024 / 8)
#define PROBE_BUF_MAX       (1 << 16)

#define USE_ITC             1
#define MAX_THREAD_PRIORITY 1
#define STATISTIC_TASK      0

#define INIT_DROP_DATA_MS   0
/**
 * defines
 */
enum {
    READ_STOP = 0,
    READ_BEGIN,
    READ_PAUSE
};

typedef struct {
#if USE_ITC
    ITCBufferStream stream;
#endif
    int sock;
    struct sockaddr_in addr;
    int addrlen;
    struct ip_mreq mreq;
    unsigned long state;
    volatile uint64_t rbytes;
    volatile uint64_t cbytes;
//  pthread_mutex_t mutex;
} TSMulticastContext;

pthread_t g_ReadTask = 0;
/**
 * global variable
 */
#if STATISTIC_TASK
struct timeval g_tv_b, g_tv_s, g_tv_e;
pthread_t g_statd;
/**
 * function declaration
 */

static int ts_multicast_flush(void *priv_data);

/* Thread for statistic bitrate */
static void* StatTask(void *arg)
{
    URLContext *h = (URLContext*)arg;
    TSMulticastContext *s = h->priv_data;
    int eval_buf_usage = 0, accumulate = 0, counting_flag = 0;

    gettimeofday(&g_tv_b, NULL);
    gettimeofday(&g_tv_s, NULL);

    while (s->state)
    {
        uint64_t diff;
        float rbr = 0.0, cbr = 0.0;
        volatile int available = 0;
        gettimeofday(&g_tv_e, NULL);

        /* compute is milliseconds, result is seconds */
        diff = ((g_tv_e.tv_sec - g_tv_s.tv_sec) * 1000 + ((g_tv_e.tv_usec / 1000.0) - (g_tv_s.tv_usec / 1000.0))) / 1000.0;

        rbr = ((float)s->rbytes * 8) / diff;
        cbr = ((float)s->cbytes * 8) / diff;

        //av_log(h, AV_LOG_ERROR, "duration %llu sec recv bitrate %0.02f kbps, consume bitrate %0.02f kbps\n", diff, rbr / 1024, cbr / 1024);
        // used in plot
        available = itcStreamAvailable(&s->stream);
        printf("%lu   \t%0.02f   \t%0.02f   \t%d\n", (g_tv_e.tv_sec - g_tv_b.tv_sec), rbr / 1024, cbr / 1024, available);

        // reset param
        if (0) //(1)
        {
            gettimeofday(&g_tv_s, NULL);
//          pthread_mutex_lock(&s->mutex);
            s->rbytes = 0;
            s->cbytes = 0;
//          pthread_mutex_unlock(&s->mutex);
        }

        // FIXME: evaluate buffer usage
        // try to put this to other place, let this thread simply statistic bitrate
        accumulate += available;
        counting_flag++;
        if (counting_flag == 6)
        {
            eval_buf_usage = accumulate / counting_flag;
            if (eval_buf_usage > ((rbr/8)*5/10))
            {
                printf("TSMULTI# flush source buffer\n");
                //ts_multicast_flush(s);
                mtal_put_message(MSG_MCTL_CMD, MCTL_CMD_FLUSH);
            }
            counting_flag = 0;
            accumulate = 0;
        }

        // FIXME
        sleep(10); //sleep(1);
    }

    pthread_exit(NULL);
}
#endif

#define	P	0x20000000
#define	X	0x10000000
#define	CC	0x0F000000
#define	M	0x00800000
#define	PT	0x007F0000
#define	SN	0x0000FFFF

int GetRtpHeaderSize(uint8_t *RtpBuf)
{
	uint8_t headoffset = 12;
	uint32_t PacketFmt = 0x00000000;
	uint8_t CRC_num = 0;

	if(RtpBuf[0] == 0x47)
	{
		headoffset = 0;
		goto end;
	}
	PacketFmt = PacketFmt | ((RtpBuf[0] & 0x000000FF) << 24);
	PacketFmt = PacketFmt | ((RtpBuf[1] & 0x000000FF) << 16);
	PacketFmt = PacketFmt | ((RtpBuf[2] & 0x000000FF) << 8);
	PacketFmt = PacketFmt | (RtpBuf[3] & 0x000000FF);

	CRC_num = PacketFmt & CC;

	if(CRC_num)
	{
		headoffset += CRC_num<<2;
	}
end:
	return headoffset;
}

static void* ReadTask(void *arg)
{
    URLContext *h = (URLContext*)arg;
    TSMulticastContext *s = h->priv_data;
    int startState = 0;
    struct timeval startT, endT;
    
#if STATISTIC_TASK
    /* init statistic thread */
    pthread_create(&g_statd, NULL, StatTask, (void*)h);
#endif

    /* read data from lwip */
#if USE_ITC

    while (s->state)
    {
        int retv = -1, idx = 0, headsize = 0, writtensize;
        uint8_t buf[ETHMTU_SIZE] = {0};
		uint8_t *payload = buf;
        int bEncrypted = 0;
        int chkNoDataTimeOutCount = 0;
        // Thread safe issue
        while (s->state == READ_PAUSE)
        { 
            usleep(1000);
        }

        chkNoDataTimeOutCount = 1000;
        do
        {
            if (s->state == READ_STOP)
            {
                usleep(1000);
                break;
            }
            retv = recvfrom(s->sock, buf, ETHMTU_SIZE, MSG_DONTWAIT, (struct sockaddr*)&s->addr, &s->addrlen);
			//printf("retv = %d\n", retv);
            if (retv <= 0)
            {
                usleep(1000);
                if (--chkNoDataTimeOutCount <= 0)
                {
                    mtal_set_dataInputStatus(false);
                    chkNoDataTimeOutCount = 1000;
                }
            }
        } while (retv <= 0);

        if (s->state != READ_STOP)
        {
            mtal_set_dataInputStatus(true);
        }
        
        switch (startState)
        {
            case 0:
                gettimeofday(&startT, NULL);
                startState = 1;
            case 1:
                gettimeofday(&endT, NULL);
                if (itpTimevalDiff(&startT, &endT) > INIT_DROP_DATA_MS)
                {
                    startState = 2;
                    printf("drop %u ms init data\n", INIT_DROP_DATA_MS);
                }
                else
                {
                    continue;
                }
                break;
            case 2:
            default:
                break;
        }
        
		if(buf[0] != 0x47 && retv > WRITE_SIZE)
		{
			headsize = GetRtpHeaderSize(buf);
			retv -= headsize;
			payload += headsize;
		}

#ifdef CFG_IP_SECURITY_MODE
        if (payload[0] != 0x47 || payload[188] != 0x47 || payload[376] != 0x47)
        {
            bEncrypted = 1;
        }
#endif
        writtensize = 0;
        do
        {
            if (s->state == READ_STOP) break;
#ifdef CFG_IP_SECURITY_MODE
            if (writtensize == 0 && retv == WRITE_SIZE && bEncrypted)
            {
                mtal_decrypt_data_by_size(payload, DEC_ALIGN_SIZE);
            }
#endif
            //printf("payload[0] = 0x%x, payload[188] = 0x%x, payload[376] = 0x%x\n", payload[0], payload[188], payload[376]);
            if(payload[0] != 0x47 || payload[188] != 0x47)
            {
                printf("session key is not match with TX!\n");
            }
            else
            {
                writtensize += itcStreamWrite(&s->stream, &payload[writtensize], retv - writtensize);
            }
        } while (writtensize < retv);

        // calculate bit rate
//      pthread_mutex_lock(&s->mutex);
        s->rbytes += writtensize;
//      pthread_mutex_unlock(&s->mutex);
    }
#endif

#if STATISTIC_TASK
    pthread_join(g_statd, NULL);
#endif
    //pthread_exit(NULL);
}

/**
 * flush source buffer itc
 */
static int ts_multicast_flush(void *priv_data)
{
    TSMulticastContext *s = priv_data;
    int size = -1;
    char *bufptr = NULL;
    // TODO: buffer evaluation should base on input source bit rate
    const int input_br = (20*1024*1024/8);
    const int remain = (((input_br *5/10) /188)*188); // half of 1 sec

    printf("%s# +\n", __func__);

    // thread safe issue
    s->state = READ_PAUSE;
    usleep(10 * 1000);

#if USE_ITC
    // flush itcStream
    size = itcStreamAvailable(&s->stream);
    if (size > remain)
    {
        int ret = -1;
        ret = itcStreamReadLock(&s->stream, &bufptr, size-remain);
        itcStreamReadUnlock(&s->stream, ret);
    }
#endif

    s->rbytes = s->cbytes = 0;
    s->state = READ_BEGIN;

#if STATISTIC_TASK
    // FIXME: to fit evaluate bitrate, try other methods
    gettimeofday(&g_tv_s, NULL);
#endif

    printf("%s# -\n", __func__);

    return 0;
}

#if 0
static int ts_multicast_setinfo(void *priv_data, SIT_SECTION_TABLE *section)
{
    TSMulticastContext *s = (TSMulticastContext*) priv_data;
    FF_COMMON_DEF *comm = (FF_COMMON_DEF*)&s->common;
    SIT_SECTION_TABLE *sec = &comm->stream;
    int bUpdate = 0;

    // FIXME:
    bUpdate = ((sec->skipframe != section->skipframe)
        | (sec->valid_resolution != section->valid_resolution)
        | (sec->input_lock != section->input_lock)
        | (sec->input_source != section->input_source)
        | (sec->width != section->width)
        | (sec->height != section->height)
        | (sec->encoder_info != section->encoder_info)
        | (sec->audio_codec != section->audio_codec)
        | (sec->sampling_rate != section->sampling_rate));

    if (bUpdate)
    {
#if 1
        printf("# original SourceInformationSection\n#{\n");
    printf("#\tskipframe=%X,\n#\tresolution=%X,\n#\tlock=%X,\n#\tsource=%X,\n#\twidth=%u, height=%u,\n#\tinfo=%u,\n#\taudio_codec=%X,\n#\tsample_rate=%u\n#}\n",
        sec->skipframe,
        sec->valid_resolution,
        sec->input_lock,
        sec->input_source,
        sec->width,
        sec->height,
        sec->encoder_info,
        sec->audio_codec,
        sec->sampling_rate);

        printf("# setting SourceInformationSection\n#{\n");
    printf("#\tskipframe=%X,\n#\tresolution=%X,\n#\tlock=%X,\n#\tsource=%X,\n#\twidth=%u, height=%u,\n#\tinfo=%u,\n#\taudio_codec=%X,\n#\tsample_rate=%u\n#}\n",
        section->skipframe,
        section->valid_resolution,
        section->input_lock,
        section->input_source,
        section->width,
        section->height,
        section->encoder_info,
        section->audio_codec,
        section->sampling_rate);
#endif

        // FIXME:
        sec->skipframe = section->skipframe;
        sec->valid_resolution = section->valid_resolution;
        sec->input_lock = section->input_lock;
        sec->input_source = section->input_source;
        sec->width = section->width;
        sec->height = section->height;
        sec->encoder_info = section->encoder_info;
        sec->audio_codec = section->audio_codec;
        sec->sampling_rate = section->sampling_rate;

        if (comm->reset_sdm)
            comm->reset_sdm(sec);
        else
            printf("#CB reset_sdm not exist\n");
    }

    return 0;
}
#endif

static int ts_multicast_close(URLContext *h)
{
    TSMulticastContext *s = h->priv_data;
    s->state = READ_STOP;

    av_log(h, AV_LOG_INFO, "close+\n");
    // close thread (if any) and socket
#if USE_ITC
    pthread_join(g_ReadTask, NULL);
    usleep(10*1000);
    g_ReadTask = 0;
    itcStreamClose(&s->stream);
#endif

    if (s->addr.sin_family == AF_INET)
    {
        if (setsockopt(s->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &s->mreq, sizeof(s->mreq)) < 0)
        {
            av_log(h, AV_LOG_ERROR, "setsockopt(IP_DROP_MEMBERSHIP: %s\n", strerror(errno));
            return -1;
        }
    }

    closesocket(s->sock);

    s->rbytes = 0; // production packet in bytes
    s->cbytes = 0; // consume packet in bytes

//  pthread_mutex_destroy(&s->mutex);
    memset(s, NULL, sizeof(TSMulticastContext));

    mtal_pb_set_livesrc(0);

    av_log(h, AV_LOG_INFO, "close-\n");
    return 0;
}

static int ts_multicast_open(URLContext *h, const char *filename, int flags)
{
    //extern IPTV_USER_CONFIG IptvUserConfig;
    struct timeval startTime, curTime;
    gettimeofday(&startTime, NULL);

    TSMulticastContext *s = h->priv_data;

    av_log(h, AV_LOG_INFO, "open+ %s\n", filename);
    if (g_ReadTask)
    {
        printf("re-entry open... close previous first\n");
        ts_multicast_close(h);
        usleep(500000);
    }    
    mtal_pb_set_livesrc(1);

    while (!ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_IS_AVAIL, NULL))
        usleep(100000);

    h->is_streamed = 1;
//  h->max_packet_size = WRITE_SIZE;

    s->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s->sock < 0)
    {
        av_log(h, AV_LOG_FATAL, "ENET not available\n");
        return -1;
    }
    av_log(h, AV_LOG_INFO, "socket handler %d\n", s->sock);
    bzero((char *)&s->addr, sizeof(s->addr));
    s->addr.sin_family = AF_INET;
    s->addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s->addr.sin_port = htons(MULTICAST_PORT);
    s->addrlen = sizeof(s->addr);

    if (bind(s->sock, (struct sockaddr *) &s->addr, sizeof(s->addr)) < 0)
    {
        av_log(h, AV_LOG_FATAL, "Socket bind failied\n");
        return -1;
    }
    //s->mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    if (!memcmp(filename, "tsmulti://@", sizeof("tsmulti://@") - 1))
    {
        char multiCastAddr[32] = { 0 };
        char* pBuffer = &filename[sizeof("tsmulti://@") - 1];
        char* pos = strchr(pBuffer, ':');
        if (pos != NULL)
        {
            memcpy(multiCastAddr, pBuffer, (int)(pos - pBuffer));
            
            av_log(h, AV_LOG_INFO, "Open source: %s\n", multiCastAddr);
            s->mreq.imr_multiaddr.s_addr = inet_addr(multiCastAddr);
            s->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        }
#if 0		
        else
        {
            s->mreq.imr_multiaddr.s_addr = inet_addr(IptvUserConfigGetMulticastGroup(&IptvUserConfig));
            s->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        }
#endif		
    }
#if 0	
    else
    {
        s->mreq.imr_multiaddr.s_addr = inet_addr(IptvUserConfigGetMulticastGroup(&IptvUserConfig));
        s->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    }
#endif    
    if (setsockopt(s->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &s->mreq, sizeof(s->mreq)) < 0)
    {
        av_log(h, AV_LOG_FATAL, "setsockopt(IP_ADD_MEMBERSHIP): %s\n", strerror(errno));
        return -1;
    }

//  pthread_mutex_init(&s->mutex, NULL);
    s->rbytes = s->cbytes = 0;
    s->state = READ_BEGIN;

#if USE_ITC
    if (itcBufferStreamOpen(&s->stream, BUF_SIZE))
    {
        av_log(h, AV_LOG_ERROR, "itcBufferStreamOpen fail\n");
        return -1;
    }	
#if MAX_THREAD_PRIORITY
    {
        pthread_attr_t attr;
        struct sched_param param;
        int thread_policy;
        int rev, min_prior, max_prior;

        rev = pthread_attr_init(&attr);
        rev = pthread_attr_getschedparam(&attr, &param);
        min_prior = sched_get_priority_min(SCHED_RR);
        max_prior = sched_get_priority_max(SCHED_RR);
        param.sched_priority = max_prior;
        rev = pthread_attr_setschedparam(&attr, &param);
        pthread_create(&g_ReadTask, &attr, ReadTask, (void*)h);
    }
#else
    pthread_create(&g_ReadTask, NULL, ReadTask, (void*)h);
#endif

    // Set ffmpeg component to itv
    {
        FF_COMPONENT comp1, *comp = &comp1;
        comp->component_prop = (void*) s;
        comp->flush = ts_multicast_flush;
        mtal_set_component(ITV_SRC_COMPONENT, comp);
    }

    // FIXME: preserved 1 MBytes
    //while(s->rbytes < PROBE_BUF_MAX)
    while(1)
    {
        gettimeofday(&curTime, NULL);
        if(itpTimevalDiff(&startTime, &curTime) > 2*1000)
        {
            ts_multicast_close(h);
            //mtal_set_switch_flag(0);
            return -1;
        }
        if(itcStreamAvailable(&s->stream) < PROBE_BUF_MAX)
            usleep(100*1000);
        else
            break;
    }
#endif
    av_log(h, AV_LOG_INFO, "open- %s, avasize: %u\n", filename, itcStreamAvailable(&s->stream));
    return 0;
}

/**
 * itc buffer stream read mechanism
 *
 * 1. read pointer < write pointer
 * |------|------------|---------|
 *        r            w
 * read len = (w - r)
 *
 * 2. read pointer >= write pointer
 * |------|------------|---------|
 *        w            r         size
 * read len = (size - r)
 *
 * hint: becareful r=w case
 * 1. read catch pointer
 * 2. write catch read
 */
static int ts_multicast_read(URLContext *h, unsigned char *buf, int size)
{
    TSMulticastContext *s = h->priv_data;
    uint8_t *bufptr = NULL;
    int retv = 0, len = size;
    int retryCount = 300;
//av_log(h, AV_LOG_INFO, "read+ %d ret %d\n", size, retv);

#if USE_ITC
retry_lock:
    // magic number, size = 32768
    // input bitrate 18 Mb ~= 2 MB
    // 32768 / 2359296 ~= 0.014 sec
    if(mtal_get_switch_flag())
    {
        return 0;
    }

    while (((retv = itcStreamAvailable(&s->stream)) == 0) && retryCount > 0)
    {
        //printf("itc# req %d, avl %d\n", size, retv);
        usleep(1000);
        retryCount--;
    }
    if (retv < size)
    {
        len = retv;
    }

#if 1
    retv = itcStreamRead(&s->stream, buf, len);
#else
    retv = itcStreamReadLock(&s->stream, &bufptr, size);
    if (retv > 0)
    {
        memcpy(buf, bufptr, retv);
    }
    else if (retv == 0)
    {
        //av_log(h, AV_LOG_INFO, "itc# read nothing\n");
        itcStreamReadUnlock(&s->stream, retv);
        usleep(1000);
        goto retry_lock;
    }
    else
    {
        if (retv == ITC_LOCK_FAIL)
        {
            av_log(h, AV_LOG_ERROR, "itc# lock fail\n");
            goto retry_lock;
        }
        av_log(h, AV_LOG_ERROR, "itc# read fail %d\n", retv);
    }
    itcStreamReadUnlock(&s->stream, retv);
#endif
    // statistic consume rate
//  pthread_mutex_lock(&s->mutex);
    s->cbytes += retv;
//  pthread_mutex_unlock(&s->mutex);
#else
    while (len)
    {
        retv = recvfrom(s->sock, buf, len, 0, (struct sockaddr*)&s->addr, &s->addrlen);
        if (retv > 0)
        {
            buf += retv;
            len -= retv;
        }
    }
    retv = size;
#endif

    //av_log(h, AV_LOG_INFO, "read- %d ret %d\n", size, retv);
    return retv;
}

static int ts_multicast_get_handle(URLContext *h)
{
    TSMulticastContext *s = h->priv_data;
    return s->sock;
}

static int ts_multicast_check(URLContext *h, int mask)
{
    TSMulticastContext *s = h->priv_data;
    av_log(h, AV_LOG_INFO, "%s:%d mask %d\n", __func__, __LINE__, mask);
    return 0;
}

URLProtocol ff_tsmulti_protocol = {
    .name           = "tsmulti",
    .url_open       = ts_multicast_open,
    .url_read       = ts_multicast_read,
    .url_write      = NULL,
    .url_seek       = NULL,
    .url_close      = ts_multicast_close,
    .url_read_pause     = NULL,
    .url_read_seek      = NULL,
    .url_get_file_handle    = ts_multicast_get_handle,
    .url_check      = ts_multicast_check,
    .priv_data_size     = sizeof(TSMulticastContext),
};
