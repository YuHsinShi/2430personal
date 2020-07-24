#include <errno.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "string.h"
#include "stdio.h"
#include "protocol_wrapper.h"
#include "iot_socket.h"
#include "pthread.h"

#define MAX_PER_PACKET_BUFFER           1500

#define AYSNC_CHECK_SOCKET_GPIO         23
#define MAX_ASYNC_SOCKET_COUNT          8                    

//HOST Command
#define SOCKET_OPEN                     0x20000000
#define SOCKET_CLOSE                    0x20000001
#define SOCKET_BIND                     0x20000002
#define SOCKET_CONNECT                  0x20000003
#define SOCKET_GET_CONNECT_RESULT       0x20000004
#define SOCKET_ACCEPT                   0x20000005
#define SOCKET_GET_ACCEPT_RESULT        0x20000006
#define SOCKET_LISTEN                   0x20000007
#define SOCKET_SEND                     0x20000008
#define SOCKET_SEND_TO                  0x20000009
#define SOCKET_RECV                     0x2000000A
#define SOCKET_RECV_DATA                0x2000000B
#define SOCKET_RECV_FROM                0x2000000C
#define SOCKET_RECV_FROM_DATA           0x2000000D
#define SOCKET_SET_OPTION               0x2000000E
#define SOCKET_GET_OPTION               0x2000000F
#define SOCKET_GET_PEER_NAME            0x20000010
#define SOCKET_GET_SOCK_NAME            0x20000011
#define SOCKET_SHUTDOWN                 0x20000012
#define SOCKET_CHECK_STATUS             0x20000013
#define SOCKET_GET_SEND_RESULT          0x20000014
#define SOCKET_SELECT                   0x20000015
#define SOCKET_GET_SELECT_RESULT        0x20000016
#define SOCKET_IO_CTL                   0x20000017
#define SOCKET_F_CNTL                   0x20000018

struct lwip_time_val
{
    long tv_sec;
    long tv_usec;
};

typedef enum ASYNC_PROCESS_ID_TAG
{
    ASYNC_PROCESS_CONNECT = 0,
    ASYNC_PROCESS_ACCEPT,
    ASYNC_PROCESS_RECV,
    ASYNC_PROCESS_RECV_FROM,
    ASYNC_PROCESS_SEND,
    ASYNC_PROCESS_SELECT,
    LAST_ASYNC_PROCESS_ID
} ASYNC_PROCESS_ID;

typedef struct SOCKET_OPEN_REQUEST_TAG
{
    int                     domain;
    int                     type;
    int                     protocol;
} SOCKET_OPEN_REQUEST;

typedef struct SOCKET_OPEN_RESPONSE_TAG
{
    int                     result;
    int                     socketId;
}SOCKET_OPEN_RESPONSE;

typedef struct SOCKET_CLOSE_REQUEST_TAG
{
    int                     socketId;
}SOCKET_CLOSE_REQUEST;

typedef struct SOCKET_CLOSE_RESPONSE_TAG
{
    int     result;
    int     closeResult;
}SOCKET_CLOSE_RESPONSE;

typedef struct SOCKET_CONNECT_REQUEST_TAG
{
    int                     socketId;
    struct sockaddr_in      tSocketAddr;
    socklen_t               addrLen;
}SOCKET_CONNECT_REQUEST;

typedef struct SOCKET_ASYNC_RESULT_TAG
{
    int                     asyncEntryId;
} SOCKET_ASYNC_RESULT;

typedef struct SOCKET_ASYNC_RESULT_REQUEST_TAG
{
    int                     asyncEntryId;
} SOCKET_ASYNC_RESULT_REQUEST;

typedef struct SOCKET_CONNECT_RESULT_RESPONSE_TAG
{
    int                     result;
    int                     connectResult;
}SOCKET_CONNECT_RESULT_RESPONSE;

typedef struct SOCKET_BIND_REQUEST_TAG
{
    int                     socketId;
    struct sockaddr         tSocketAddr;
    socklen_t               addrLen;
}SOCKET_BIND_REQUEST;

typedef struct SOCKET_BIND_RESPONSE_TAG
{
    int                     result;
    int                     bindResult;
}SOCKET_BIND_RESPONSE;

typedef struct SOCKET_LISTEN_REQUEST_TAG
{
    int                     socketId;
    int                     backLog;
}SOCKET_LISTEN_REQUEST;

typedef struct SOCKET_LISTEN_RESPONSE_TAG
{
    int                     result;
    int                     listenResult;
}SOCKET_LISTEN_RESPONSE;

typedef struct SOCKET_ACCEPT_REQUEST_TAG
{
    int                     socketId;
}SOCKET_ACCEPT_REQUEST;

typedef struct SOCKET_ACCEPT_RESULT_RESPONSE_TAG
{
    int                     result;
    int                     acceptResult;
    struct sockaddr         tRemoteAddr;
    socklen_t               addrLen;
}SOCKET_ACCEPT_RESULT_RESPONSE;

typedef struct SOCKET_STATUS_REQUEST_TAG
{
    int                     asyncEntryId;
}SOCKET_STATUS_REQUEST;

typedef struct SOCKET_STATUS_RESPONSE_TAG
{
    int                     result;
    int                     doneStatus;
    uint32_t                asyncPreReadData[MAX_ASYNC_SOCKET_COUNT];
}SOCKET_STATUS_RESPONSE;

typedef struct SOCKET_RECV_REQUEST_TAG
{
    int                     socketId;
    int                     recvSize;
    int                     flags;
}SOCKET_RECV_REQUEST;

typedef struct SOCKET_RECV_RESULT_REQUEST_TAG
{
    int                     asyncEntryId;
    int                     responseSize;
} SOCKET_RECV_RESULT_REQUEST;

typedef struct SOCKET_RECV_RESULT_RESPONSE_TAG
{
    int                     result;
    int                     recvResult;
    uint8_t                 pBuffer[MAX_PER_PACKET_BUFFER];
}SOCKET_RECV_RESULT_RESPONSE;

typedef struct SOCKET_RECV_FROM_REQUEST_TAG
{
    int                     socketId;
    int                     recvSize;
    int                     flags;
}SOCKET_RECV_FROM_REQUEST;

typedef struct SOCKET_RECV_FROM_RESULT_RESPONSE_TAG
{
    int                     result;
    int                     recvResult;
    uint8_t                 pBuffer[MAX_PER_PACKET_BUFFER];
    struct sockaddr_in      tFrom;
    socklen_t               fromLen;
}SOCKET_RECV_FROM_RESULT_RESPONSE;

typedef struct SOCKET_SEND_REQUEST_TAG
{
    int                     socketId;
    int                     sendSize;
    int                     flags;
}SOCKET_SEND_REQUEST;

typedef struct SOCKET_SEND_RESULT_RESPONSE_TAG
{
    int                     result;
    int                     sendResult;
}SOCKET_SEND_RESULT_RESPONSE;

typedef struct SOCKET_SEND_TO_REQUEST_TAG
{
    int                     socketId;
    int                     sendSize;
    int                     flags;
    struct sockaddr         tTo;
    socklen_t               toLen;
}SOCKET_SEND_TO_REQUEST;

typedef struct SOCKET_SEND_TO_RESPONSE_TAG
{
    int                     result;
    int                     sendResult;
}SOCKET_SEND_TO_RESPONSE;

typedef struct SOCKET_SELECT_REQUEST_TAG
{
    int                     maxFds;
    int                     bReadSet;
    fd_set                  tReadSet;
    int                     bWriteSet;
    fd_set                  tWriteSet;
    int                     bExceptSet;
    fd_set                  tExceptSet;
    struct lwip_time_val    tTimeout;
}SOCKET_SELECT_REQUEST;

typedef struct SOCKET_SELECT_RESULT_RESPONSE_TAG
{
    int                     result;
    fd_set                  tReadSet;
    fd_set                  tWriteSet;
    fd_set                  tExceptSet;
    int                     selectResult;
}SOCKET_SELECT_RESULT_RESPONSE;

typedef struct SOCKET_SHUTDOWN_REQUEST_TAG
{
    int                     socketId;
    int                     how;
}SOCKET_SHUTDOWN_REQUEST;

typedef struct SOCKET_SHUTDOWN_RESPONSE_TAG
{
    int                     result;
    int                     shutdownResult;
}SOCKET_SHUTDOWN_RESPONSE;

typedef struct SOCKET_GET_SOCK_NAME_REQUEST_TAG
{
    int                     socketId;
    int                     nameLen;
}SOCKET_GET_SOCK_NAME_REQUEST;

typedef struct SOCKET_GET_SOCK_NAME_RESPONSE_TAG
{
    int                     result;
    int                     getSockNameResult;
    struct sockaddr         tSockName;
    socklen_t               nameLen;
}SOCKET_GET_SOCK_NAME_RESPONSE;

typedef struct SOCKET_GET_PEER_NAME_REQUEST_TAG
{
    int                     socketId;
    int                     nameLen;
}SOCKET_GET_PEER_NAME_REQUEST;

typedef struct SOCKET_GET_PEER_NAME_RESPONSE_TAG
{
    int                     result;
    int                     getPeerNameResult;
    struct sockaddr         tPeerName;
    socklen_t               nameLen;
}SOCKET_GET_PEER_NAME_RESPONSE;

typedef struct SOCKET_SET_OPTION_REQUEST_TAG
{
    int                     socketId;
    int                     level;
    int                     optName;
    uint8_t                 pOptionBuffer[64];
    socklen_t               optLen;
}SOCKET_SET_OPTION_REQUEST;

typedef struct SOCKET_SET_OPTION_RESPONSE_TAG
{
    int                     result;
    int                     setOptionResult;
}SOCKET_SET_OPTION_RESPONSE;

typedef struct SOCKET_GET_OPTION_REQUEST_TAG
{
    int                     socketId;
    int                     level;
    int                     optName;
    socklen_t               optLen;
}SOCKET_GET_OPTION_REQUEST;

typedef struct SOCKET_GET_OPTION_RESPONSE_TAG
{
    int                     result;
    int                     getOptionResult;
    uint8_t                 pOptionBuffer[64];
    socklen_t               optLen;
}SOCKET_GET_OPTION_RESPONSE;

typedef struct SOCKET_IO_CTL_REQUEST_TAG
{
    int                     socketId;
    int                     cmd;
    uint32_t                argv;
}SOCKET_IO_CTL_REQUEST;

typedef struct SOCKET_IO_CTL_RESPONSE_TAG
{
    int                     result;
    int                     ioCtlResult;
    uint32_t                argv;
}SOCKET_IO_CTL_RESPONSE;

typedef struct SOCKET_F_CNTL_REQUEST_TAG
{
    int                     socketId;
    int                     cmd;
    int                     val;
}SOCKET_F_CNTL_REQUEST;

typedef struct SOCKET_F_CNTL_RESPONSE_TAG
{
    int                     result;
    int                     fCntlResult;
}SOCKET_F_CNTL_RESPONSE;

typedef struct SOCKET_RECV_STATUS_REQUEST_TAG
{
    int                     asyncEntryId;
}SOCKET_RECV_STATUS_REQUEST;

typedef struct SOCKET_RECV_STATUS_RESPONSE_TAG
{
    int                     result;
    int                     recvSize;
}SOCKET_RECV_STATUS_RESPONSE;

static sem_t    gAsyncSocketSem[MAX_ASYNC_SOCKET_COUNT];
static int      gPreReadResult[MAX_ASYNC_SOCKET_COUNT] = { 0 };
static int      gWaitReadOut[MAX_ASYNC_SOCKET_COUNT] = { 0 };

#ifndef CFG_NET_ENABLE /*use lwip original function*/
uint16_t
htons(uint16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

uint16_t
ntohs(uint16_t n)
{
  return htons(n);
}

uint32_t
htonl(uint32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

uint32_t
ntohl(uint32_t n)
{
  return htonl(n);
}

/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */

int
inet_aton(const char *cp, struct in_addr *addr)
{
	u_long val, base, n;
	char c;
	u_long parts[4], *pp = parts;

	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, other=decimal.
		 */
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) + 
					(c + 10 - (islower(c) ? 'a' : 'A'));
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16-bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3 || val > 0xff)
				return (0);
			*pp++ = val, cp++;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && (!isascii(*cp) || !isspace(*cp)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}

const char	*inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (af == AF_INET)
    {
	    static const char fmt[] = "%u.%u.%u.%u";
	    char tmp[sizeof("255.255.255.255")];
        const u_char *s = (const u_char *)src;

	    if (sprintf(tmp, fmt, s[0], s[1], s[2], s[3]) > (int)size)
        {
		    errno = ENOSPC;
		    return NULL;
	    }
	    strcpy(dst, tmp);
	    return dst;
    }
    else
    {
        errno = EAFNOSUPPORT;
		return NULL;
	}
}

int
inet_pton(int af, const char *src, void *dst)
{
    if (af == AF_INET)
    {
	    static const char digits[] = "0123456789";
	    int saw_digit, octets, ch;
	    u_char tmp[4], *tp;

	    saw_digit = 0;
	    octets = 0;
	    *(tp = tmp) = 0;
	    while ((ch = *src++) != '\0') {
		    const char *pch;

		    if ((pch = strchr(digits, ch)) != NULL) {
			    u_int new = *tp * 10 + (pch - digits);

			    if (new > 255)
				    return (0);
			    *tp = new;
			    if (! saw_digit) {
				    if (++octets > 4)
					    return (0);
				    saw_digit = 1;
			    }
		    } else if (ch == '.' && saw_digit) {
			    if (octets == 4)
				    return (0);
			    *++tp = 0;
			    saw_digit = 0;
		    } else
			    return (0);
	    }
	    if (octets < 4)
		    return (0);

	    memcpy(dst, tmp, 4);
	    return (1);
    }
    else
    {
        errno = EAFNOSUPPORT;
		return -1;
	}
}



/*
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 */
uint32_t
inet_addr(const char *cp)
{
	struct in_addr val;

	if (inet_aton(cp, &val))
		return (val.s_addr);
	return 0;
}
#endif

static void *_IotAsyncSocketCheckTask(void *arg)
{
    int i = 0;
    for (i = 0; i < MAX_ASYNC_SOCKET_COUNT; i++)
    {
        sem_init(&gAsyncSocketSem[i], 0, 0);
    }
    printf("Inited socket check task...\n");
    for (;;)
    {
        SOCKET_STATUS_REQUEST           tStatusRequest;
        SOCKET_STATUS_RESPONSE          tStatusResponse;

        while (!ithGpioGet(AYSNC_CHECK_SOCKET_GPIO))
        {
            usleep(100);
        }
        memset(&tStatusResponse, 0x0, sizeof(SOCKET_STATUS_RESPONSE));
        IotProtocolRead(SOCKET_CHECK_STATUS, (uint8_t*)&tStatusRequest, sizeof(tStatusRequest), (uint8_t*)&tStatusResponse, sizeof(tStatusResponse));
        if (tStatusResponse.result == 0x12345678)
        {
            for (i = 0; i < MAX_ASYNC_SOCKET_COUNT; i++)
            {
                if (tStatusResponse.doneStatus & (0x1 << i))
                {
                    if (gWaitReadOut[i] == 0)
                    {
                        gPreReadResult[i] = tStatusResponse.asyncPreReadData[i];
                        gWaitReadOut[i] = 1;
                        sem_post(&gAsyncSocketSem[i]);
                    }
                }
            }

            if (tStatusResponse.doneStatus)
            {
                usleep(1000);
            }
        }
    }
    return NULL;
}

static void _IotWaitAsyncSocketDone(int asyncId)
{
    sem_wait(&gAsyncSocketSem[asyncId]);
}


int IotSocketOpenSock(int domain, int type, int protocol)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_OPEN_REQUEST  tOpenRequest = { 0 };
    SOCKET_OPEN_RESPONSE tOpenResponse = { 0 };

    tOpenRequest.domain = domain;
    tOpenRequest.type = type;
    tOpenRequest.protocol = protocol;
    IotProtocolRead(SOCKET_OPEN, (uint8_t*)&tOpenRequest, sizeof(tOpenRequest), (uint8_t*)&tOpenResponse, sizeof(tOpenResponse));
    return tOpenResponse.socketId;
}


int IotSocketCloseSock(int s)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_CLOSE_REQUEST tCloseRequest = { 0 };
    SOCKET_CLOSE_RESPONSE tCloseResponse = { 0 };
    tCloseRequest.socketId = s;
    IotProtocolRead(SOCKET_CLOSE, (uint8_t*)&tCloseRequest, sizeof(tCloseRequest), (uint8_t*)&tCloseResponse, sizeof(tCloseResponse));
    return tCloseResponse.closeResult;
}

int IotSocketBind(int s, const struct sockaddr *name, socklen_t namelen)
{
	//printf("+++ %s(%s) +++\n", __func__, name->sa_data);
    SOCKET_BIND_REQUEST  tBindRequest = { 0 };
    SOCKET_BIND_RESPONSE tBindResponse = { 0 };
    
    tBindRequest.socketId = s;
    memcpy(&tBindRequest.tSocketAddr, name, namelen);
    tBindRequest.addrLen = namelen;
    IotProtocolRead(SOCKET_BIND, (uint8_t*)&tBindRequest, sizeof(tBindRequest), (uint8_t*)&tBindResponse, sizeof(tBindResponse));
    return tBindResponse.bindResult;
}

int IotSocketListen(int s, int backlog)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_LISTEN_REQUEST  tListenRequest = { 0 };
    SOCKET_LISTEN_RESPONSE tListenResponse = { 0 };
    
    tListenRequest.socketId = s;
    tListenRequest.backLog = backlog;
    IotProtocolRead(SOCKET_LISTEN, (uint8_t*)&tListenRequest, sizeof(tListenRequest), (uint8_t*)&tListenResponse, sizeof(tListenResponse));
    return tListenResponse.listenResult;
}

int IotSocketAccept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_ACCEPT_REQUEST           tAcceptRequest = { 0 };
    SOCKET_ASYNC_RESULT             tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST     tResultRequest = { 0 };
    SOCKET_ACCEPT_RESULT_RESPONSE   tResultResponse = { 0 };

    tAcceptRequest.socketId = s;
    
    IotProtocolRead(SOCKET_ACCEPT, (uint8_t*)&tAcceptRequest, sizeof(tAcceptRequest), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));
    
    _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);

    tResultRequest.asyncEntryId = tAsyncResult.asyncEntryId;
    IotProtocolRead(SOCKET_GET_ACCEPT_RESULT, (uint8_t*)&tResultRequest, sizeof(tResultRequest), (uint8_t*)&tResultResponse, sizeof(tResultResponse));

    if (addr && addrlen)
    {
        memcpy(addr, &tResultResponse.tRemoteAddr, sizeof(struct sockaddr));
        *addrlen = tResultResponse.addrLen;
    }
    
    gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
    return tResultResponse.acceptResult;
}

int IotSocketConnect(int s, const struct sockaddr *name, socklen_t namelen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_CONNECT_REQUEST          tConnectRequest = { 0 };
    SOCKET_ASYNC_RESULT             tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST     tResultRequest = { 0 };
    SOCKET_CONNECT_RESULT_RESPONSE  tResultResponse = { 0 };

    tConnectRequest.socketId = s;
    memcpy(&tConnectRequest.tSocketAddr, (void*)name, namelen);
    tConnectRequest.addrLen = namelen;
    IotProtocolRead(SOCKET_CONNECT, (uint8_t*)&tConnectRequest, sizeof(tConnectRequest), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));

    _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);

    tResultRequest.asyncEntryId = tAsyncResult.asyncEntryId;
    IotProtocolRead(SOCKET_GET_CONNECT_RESULT, (uint8_t*)&tResultRequest, sizeof(tResultRequest), (uint8_t*)&tResultResponse, sizeof(tResultResponse));
    gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
    return tResultResponse.connectResult;
}

int IotSocketRecv(int s, void *mem, size_t len, int flags)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_RECV_REQUEST             tRecvRequest = { 0 };
    SOCKET_ASYNC_RESULT             tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST     tResultRequest = { 0 };
    SOCKET_RECV_RESULT_RESPONSE     tResultResponse = { 0 };

    int                             remainSize = len;
    int                             totalRecvSize = 0;
    uint8_t*                        pBuffer = mem;

    while (remainSize > 0)
    {
        int requestSize = remainSize;
        if (remainSize >= MAX_PER_PACKET_BUFFER)
        {
            requestSize = MAX_PER_PACKET_BUFFER;
        }
    
        tRecvRequest.socketId = s;
        tRecvRequest.recvSize = requestSize;
        tRecvRequest.flags = flags;
        IotProtocolRead(SOCKET_RECV, (uint8_t*)&tRecvRequest,  sizeof(tRecvRequest), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));

        _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);
        if (gPreReadResult[tAsyncResult.asyncEntryId] <= 0)
        {
            gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
            return gPreReadResult[tAsyncResult.asyncEntryId];
        }

        tResultRequest.asyncEntryId = tAsyncResult.asyncEntryId;
        IotProtocolRead(SOCKET_RECV_DATA, (uint8_t*)&tResultRequest, sizeof(tResultRequest), (uint8_t*)&tResultResponse, sizeof(tResultResponse));
        gWaitReadOut[tAsyncResult.asyncEntryId] = 0;

        if (tResultResponse.recvResult > 0)
        {
            memcpy(&pBuffer[totalRecvSize], tResultResponse.pBuffer, tResultResponse.recvResult);
            totalRecvSize += tResultResponse.recvResult;
        }
        if (tResultResponse.recvResult < requestSize)
        {
            return totalRecvSize;
        }
        remainSize -= tResultResponse.recvResult;
    }
    return totalRecvSize;
}

int IotSocketRecvFrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_RECV_FROM_REQUEST            tRecvFromRequest = { 0 };
    SOCKET_ASYNC_RESULT                 tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST         tResultRequest = { 0 };
    SOCKET_RECV_FROM_RESULT_RESPONSE    tResultResponse = { 0 };

    int                                 remainSize = len;
    int                                 totalRecvSize = 0;
    uint8_t*                            pBuffer = mem;

    while (remainSize > 0)
    {
        int requestSize = remainSize;
        if (remainSize >= MAX_PER_PACKET_BUFFER)
        {
            requestSize = MAX_PER_PACKET_BUFFER;
        }

        tRecvFromRequest.socketId = s;
        tRecvFromRequest.recvSize = requestSize;
        tRecvFromRequest.flags = flags;
        IotProtocolRead(SOCKET_RECV_FROM, (uint8_t*)&tRecvFromRequest,  sizeof(tRecvFromRequest), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));

        _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);
        if (gPreReadResult[tAsyncResult.asyncEntryId] <= 0)
        {
            gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
            return gPreReadResult[tAsyncResult.asyncEntryId];
        }
        
        tResultRequest.asyncEntryId = tAsyncResult.asyncEntryId;       
        IotProtocolRead(SOCKET_RECV_FROM_DATA, (uint8_t*)&tResultRequest, sizeof(tResultRequest), (uint8_t*)&tResultResponse, sizeof(tResultResponse));
        gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
        if (tResultResponse.recvResult > 0)
        {
            memcpy(from, &tResultResponse.tFrom, sizeof(struct sockaddr));
            memcpy(&pBuffer[totalRecvSize], tResultResponse.pBuffer, tResultResponse.recvResult);
            *fromlen = tResultResponse.fromLen;
            totalRecvSize += tResultResponse.recvResult;
        }
        if (tResultResponse.recvResult < requestSize)
        {
            return totalRecvSize;
        }
        remainSize -= tResultResponse.recvResult;
    }
    return totalRecvSize;
}

int IotSocketRead(int s, void *mem, size_t len)
{
  return IotSocketRecvFrom(s, mem, len, 0, NULL, NULL);
}

int IotSocketSend(int s, const void *dataptr, size_t size, int flags)
{
    SOCKET_SEND_REQUEST*                ptSendRequest = NULL;
    SOCKET_ASYNC_RESULT                 tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST         tResultRequest = { 0 };
    SOCKET_SEND_RESULT_RESPONSE         tResultResponse = { 0 };

    int                                 headerSize = sizeof(SOCKET_SEND_REQUEST);
    uint8_t                             pOutBuffer[2048] = { 0 };

    int                                 remainSize = size;
    int                                 totalSendSize = 0;
    uint8_t*                            pBuffer = (uint8_t*) dataptr;
    
    while (remainSize > 0)
    {
        int requestSize = remainSize;
        if (remainSize >= MAX_PER_PACKET_BUFFER)
        {
            requestSize = MAX_PER_PACKET_BUFFER;
        }
        ptSendRequest = (SOCKET_SEND_REQUEST*) pOutBuffer;
        ptSendRequest->socketId = s;
        memcpy(&pOutBuffer[headerSize], &pBuffer[totalSendSize], requestSize);
        ptSendRequest->sendSize = requestSize;
        ptSendRequest->flags = flags;

        IotProtocolRead(SOCKET_SEND, pOutBuffer,  (headerSize + requestSize), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));

        _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);

        if (gPreReadResult[tAsyncResult.asyncEntryId] >= 0)
        {
            totalSendSize += gPreReadResult[tAsyncResult.asyncEntryId];
        }
        if (gPreReadResult[tAsyncResult.asyncEntryId] < requestSize)
        {
            return totalSendSize;
        }
        remainSize -= gPreReadResult[tAsyncResult.asyncEntryId];
        gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
    }
    return totalSendSize;
}

int IotSocketWrite(int s, const void *data, size_t size)
{
  return IotSocketSend(s, data, size, 0);
}

int IotSocketSendTo(int s, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_SEND_TO_REQUEST*      ptSendToRequest = NULL;
    SOCKET_SEND_TO_RESPONSE      tSendToResponse = { 0 };

    int                          remainSize = size;
    int                          totalSendSize = 0;
    int                          headerSize = sizeof(SOCKET_SEND_TO_REQUEST);
    uint8_t*                     pBuffer = (uint8_t*) data;
    uint8_t                      pOutBuffer[2048] = { 0 };

    while (remainSize > 0)
    {
        int requestSize = remainSize;
        if (remainSize >= MAX_PER_PACKET_BUFFER)
        {
            requestSize = MAX_PER_PACKET_BUFFER;
        }
        ptSendToRequest = (SOCKET_SEND_TO_REQUEST*) pOutBuffer;
        ptSendToRequest->socketId = s;
        memcpy(&pOutBuffer[headerSize], &pBuffer[totalSendSize], requestSize);
        ptSendToRequest->sendSize = requestSize;
        ptSendToRequest->flags = flags;
        memcpy(&ptSendToRequest->tTo, to, sizeof(struct sockaddr));
        ptSendToRequest->toLen = tolen;

        IotProtocolRead(SOCKET_SEND_TO, pOutBuffer, (headerSize + requestSize), (uint8_t*)&tSendToResponse, 0);
        tSendToResponse.sendResult = size;

        if (tSendToResponse.sendResult >= 0)
        {
            totalSendSize += tSendToResponse.sendResult;
        }
        if (tSendToResponse.sendResult < requestSize)
        {
            return totalSendSize;
        }
        remainSize -= tSendToResponse.sendResult;
    }
    return totalSendSize;
}

int IotSocketSelect(int maxfds, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_SELECT_REQUEST               tSelectRequest = { 0 };
    SOCKET_ASYNC_RESULT                 tAsyncResult = { 0 };
    SOCKET_ASYNC_RESULT_REQUEST         tResultRequest = { 0 };
    SOCKET_SELECT_RESULT_RESPONSE       tResultResponse = { 0 };

    tSelectRequest.maxFds = maxfds;
    if (readset)
    {
        memcpy(&tSelectRequest.tReadSet, readset, sizeof(fd_set));
        tSelectRequest.bReadSet = 1;
    }
    if (writeset)
    {
        memcpy(&tSelectRequest.tWriteSet, writeset, sizeof(fd_set));
        tSelectRequest.bWriteSet = 1;
    }
    if (exceptset)
    {
        memcpy(&tSelectRequest.tExceptSet, exceptset, sizeof(fd_set));
        tSelectRequest.bExceptSet = 1;
    }
    tSelectRequest.tTimeout.tv_sec = (long) timeout->tv_sec;
    tSelectRequest.tTimeout.tv_usec = (long) timeout->tv_usec;

    IotProtocolRead(SOCKET_SELECT, (uint8_t*)&tSelectRequest,  sizeof(tSelectRequest), (uint8_t*)&tAsyncResult, sizeof(tAsyncResult));
    _IotWaitAsyncSocketDone(tAsyncResult.asyncEntryId);
    if (gPreReadResult[tAsyncResult.asyncEntryId] <= 0)
    {
        if (gPreReadResult[tAsyncResult.asyncEntryId] < 0)
        {
            printf("asyncId: %d, maxfds: %d, val: %d\n", tAsyncResult.asyncEntryId, maxfds, gPreReadResult[tAsyncResult.asyncEntryId]);
            goto read_sel;
        }
        if (readset)
        {
            memset(readset, 0x0, sizeof(fd_set));
        }
        if (writeset)
        {
            memset(writeset, 0x0, sizeof(fd_set));
        }
        if (exceptset)
        {
            memset(exceptset, 0x0, sizeof(fd_set));
        }
        gWaitReadOut[tAsyncResult.asyncEntryId] = 0;
        return 0;
    }
read_sel:
    tResultRequest.asyncEntryId = tAsyncResult.asyncEntryId;
    IotProtocolRead(SOCKET_GET_SELECT_RESULT, (uint8_t*)&tResultRequest, sizeof(tResultRequest), (uint8_t*)&tResultResponse, sizeof(tResultResponse));
    gWaitReadOut[tAsyncResult.asyncEntryId] = 0;

    if (readset)
    {
        memcpy(readset, &tResultResponse.tReadSet, sizeof(fd_set));
    }
    if (writeset)
    {
        memcpy(writeset, &tResultResponse.tWriteSet, sizeof(fd_set));
    }
    if (exceptset)
    {
        memcpy(exceptset, &tResultResponse.tExceptSet, sizeof(fd_set));
    }
    return tResultResponse.selectResult;
}

int IotSocketGetSockName(int s, struct sockaddr *name, socklen_t* namelen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_GET_SOCK_NAME_REQUEST  tSockNameRequest = { 0 };
    SOCKET_GET_SOCK_NAME_RESPONSE tSockNameResponse = { 0 };

    tSockNameRequest.socketId = s;
    tSockNameRequest.nameLen = *namelen;
    IotProtocolRead(SOCKET_GET_SOCK_NAME, (uint8_t*)&tSockNameRequest, sizeof(tSockNameRequest), (uint8_t*)&tSockNameResponse, sizeof(tSockNameResponse));
    memcpy(name, &tSockNameResponse.tSockName, tSockNameResponse.nameLen);
    *namelen = tSockNameResponse.nameLen;
    return tSockNameResponse.getSockNameResult;
}

int IotSocketGetPeerName(int s, struct sockaddr *name, socklen_t* namelen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_GET_PEER_NAME_REQUEST  tPeerNameRequest = { 0 };
    SOCKET_GET_PEER_NAME_RESPONSE tPeerNameResponse = { 0 };

    tPeerNameRequest.socketId = s;
    tPeerNameRequest.nameLen = *namelen;
    IotProtocolRead(SOCKET_GET_PEER_NAME, (uint8_t*)&tPeerNameRequest, sizeof(tPeerNameRequest), (uint8_t*)&tPeerNameResponse, sizeof(tPeerNameResponse));
    memcpy(name, &tPeerNameResponse.tPeerName, tPeerNameResponse.nameLen);
    *namelen = tPeerNameResponse.nameLen;
    return tPeerNameResponse.getPeerNameResult;
}

int IotSocketSetSockOpt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_SET_OPTION_REQUEST  tSetOptionRequest = { 0 };
    SOCKET_SET_OPTION_RESPONSE tSetOptionResponse = { 0 };

    tSetOptionRequest.socketId = s;
    tSetOptionRequest.level = level;
    tSetOptionRequest.optName = optname;
    memcpy(tSetOptionRequest.pOptionBuffer, optval, optlen);
    tSetOptionRequest.optLen = optlen;
    IotProtocolRead(SOCKET_SET_OPTION, (uint8_t*)&tSetOptionRequest, sizeof(tSetOptionRequest), (uint8_t*)&tSetOptionResponse, sizeof(tSetOptionResponse));
    return tSetOptionResponse.setOptionResult;
}

int IotSocketGetSockOpt(int s, int level, int optname, void *optval, socklen_t* optlen)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_GET_OPTION_REQUEST  tGetOptionRequest = { 0 };
    SOCKET_GET_OPTION_RESPONSE tGetOptionResponse = { 0 };

    tGetOptionRequest.socketId = s;
    tGetOptionRequest.level = level;
    tGetOptionRequest.optName = optname;
    tGetOptionRequest.optLen = *optlen;
    IotProtocolRead(SOCKET_GET_OPTION, (uint8_t*)&tGetOptionRequest, sizeof(tGetOptionRequest), (uint8_t*)&tGetOptionResponse, sizeof(tGetOptionResponse));
    memcpy(optval, tGetOptionResponse.pOptionBuffer, tGetOptionResponse.optLen);
    *optlen = tGetOptionResponse.optLen;
    return tGetOptionResponse.getOptionResult;
}

int IotSocketShutdown(int s, int how)
{
    SOCKET_SHUTDOWN_REQUEST  tShutdownRequest = { 0 };
    SOCKET_SHUTDOWN_RESPONSE tShutdownResponse = { 0 };
    
    tShutdownRequest.socketId = s;
    tShutdownRequest.how = how;
    IotProtocolRead(SOCKET_BIND, (uint8_t*)&tShutdownRequest, sizeof(tShutdownRequest), (uint8_t*)&tShutdownResponse, sizeof(tShutdownResponse));
    return tShutdownResponse.shutdownResult;
}

int IotSocketIoCtrl(int s, long cmd, void *argp)
{
	//printf("+++ %s +++\n", __func__);
    SOCKET_IO_CTL_REQUEST  tIoCtlRequest = { 0 };
    SOCKET_IO_CTL_RESPONSE tIoCtlResponse = { 0 };
    
    tIoCtlRequest.socketId = s;
    tIoCtlRequest.cmd = cmd;
    tIoCtlRequest.argv = *(uint32_t*)argp;
    IotProtocolRead(SOCKET_IO_CTL, (uint8_t*)&tIoCtlRequest, sizeof(tIoCtlRequest), (uint8_t*)&tIoCtlResponse, sizeof(tIoCtlResponse));
    *(uint32_t*)argp = tIoCtlResponse.argv;
    return tIoCtlResponse.ioCtlResult;
}
int IotSocketFcntl(int s, int cmd, int val)
{
	//printf("IOT to be defined!!!\n");
    SOCKET_F_CNTL_REQUEST  tFCntlRequest = { 0 };
    SOCKET_IO_CTL_RESPONSE tFCntlResponse = { 0 };
    
    tFCntlRequest.socketId = s;
    tFCntlRequest.cmd = cmd;
    if (cmd == 0x4)
    {
        if (val |= O_NONBLOCK)
        {
            tFCntlRequest.val = 1;
        }
        else
        {
            tFCntlRequest.val = val;
        }
    }

    IotProtocolRead(SOCKET_F_CNTL, (uint8_t*)&tFCntlRequest, sizeof(tFCntlRequest), (uint8_t*)&tFCntlResponse, sizeof(tFCntlResponse));
    return tFCntlResponse.ioCtlResult;
}

void IotSocketInit(void)
{
    pthread_t       task;
    pthread_attr_t  attr;
    //Slave
    ithGpioSetMode(AYSNC_CHECK_SOCKET_GPIO, ITH_GPIO_MODE0);
    ithGpioSetIn(AYSNC_CHECK_SOCKET_GPIO);
    
    pthread_attr_init(&attr);
    pthread_create(&task, &attr, _IotAsyncSocketCheckTask, NULL);
}
