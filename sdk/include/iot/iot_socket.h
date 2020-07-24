﻿/** @file
 * ITE IoT socket declaration.
 *
 * @author Steven Hsiao
 * @version 1.0
 * @date 2016
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef IOT_SOCKET_H
#define IOT_SOCKET_H

#ifdef CFG_NET_ENABLE
#include "lwip/sockets.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_NET_ENABLE
struct in_addr {
  uint32_t s_addr;
};

/* members are in network byte order */
struct sockaddr_in {
  uint8_t sin_len;
  uint8_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr_ll {
      unsigned short  sll_family;
      uint16_t        sll_protocol;
      int             sll_ifindex;
      unsigned short  sll_hatype;
      unsigned char   sll_pkttype;
      unsigned char   sll_halen;
      unsigned char   sll_addr[8];
  };

struct sockaddr {
  uint8_t sa_len;
  uint8_t sa_family;
  char sa_data[14];
};

/* If your port already typedef's socklen_t, define SOCKLEN_T_DEFINED
   to prevent this code from redefining it. */
#if !defined(socklen_t) && !defined(SOCKLEN_T_DEFINED)
typedef uint32_t socklen_t;
#endif

/** 0.0.0.0 */
#define INADDR_ANY          ((uint32_t)0x00000000UL)
/** 255.255.255.255 */
#define INADDR_BROADCAST    ((uint32_t)0xffffffffUL)


/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

/*
 * Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c)
 */
#define  SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define  SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define  SO_REUSEADDR   0x0004 /* Allow local address reuse */
#define  SO_KEEPALIVE   0x0008 /* keep connections alive */
#define  SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define  SO_BROADCAST   0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */
#define  SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define  SO_LINGER      0x0080 /* linger on close if data present */
#define  SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define  SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */

#define SO_DONTLINGER   ((int)(~SO_LINGER))

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF    0x1001    /* Unimplemented: send buffer size */
#define SO_RCVBUF    0x1002    /* receive buffer size */
#define SO_SNDLOWAT  0x1003    /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT  0x1004    /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO  0x1005    /* Unimplemented: send timeout */
#define SO_RCVTIMEO  0x1006    /* receive timeout */
#define SO_ERROR     0x1007    /* get error status and clear */
#define SO_TYPE      0x1008    /* get socket type */
#define SO_CONTIMEO  0x1009    /* Unimplemented: connect timeout */
#define SO_NO_CHECK  0x100a    /* don't create UDP checksum */


/*
 * Structure used for manipulating linger option.
 */
struct linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define  SOL_SOCKET  0xfff    /* options for socket level */


#define AF_UNSPEC       0
#define AF_INET         2
#define PF_INET         AF_INET
#define PF_UNSPEC       AF_UNSPEC
#define AF_PACKET       4
#define PF_PACKET       AF_PACKET

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#define IPPROTO_UDPLITE 136

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */


/*
 * Options for level IPPROTO_IP
 */
#define IP_TOS             1
#define IP_TTL             2

#if LWIP_TCP
/*
 * Options for level IPPROTO_TCP
 */
#define TCP_NODELAY    0x01    /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE  0x02    /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE   0x03    /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL  0x04    /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT    0x05    /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */
#endif /* LWIP_TCP */

#if LWIP_UDP && LWIP_UDPLITE
/*
 * Options for level IPPROTO_UDPLITE
 */
#define UDPLITE_SEND_CSCOV 0x01 /* sender checksum coverage */
#define UDPLITE_RECV_CSCOV 0x02 /* minimal receiver checksum coverage */
#endif /* LWIP_UDP && LWIP_UDPLITE*/


#if LWIP_IGMP
/*
 * Options and types for UDP multicast traffic handling
 */
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4
#define IP_MULTICAST_TTL   5
#define IP_MULTICAST_IF    6
#define IP_MULTICAST_LOOP  7

typedef struct ip_mreq {
    struct in_addr imr_multiaddr; /* IP multicast address of group */
    struct in_addr imr_interface; /* local IP address of interface */
} ip_mreq;
#endif /* LWIP_IGMP */

/*
 * The Type of Service provides an indication of the abstract
 * parameters of the quality of service desired.  These parameters are
 * to be used to guide the selection of the actual service parameters
 * when transmitting a datagram through a particular network.  Several
 * networks offer service precedence, which somehow treats high
 * precedence traffic as more important than other traffic (generally
 * by accepting only traffic above a certain precedence at time of high
 * load).  The major choice is a three way tradeoff between low-delay,
 * high-reliability, and high-throughput.
 * The use of the Delay, Throughput, and Reliability indications may
 * increase the cost (in some sense) of the service.  In many networks
 * better performance for one of these parameters is coupled with worse
 * performance on another.  Except for very unusual cases at most two
 * of these three indications should be set.
 */
#define IPTOS_TOS_MASK          0x1E
#define IPTOS_TOS(tos)          ((tos) & IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY          0x10
#define IPTOS_THROUGHPUT        0x08
#define IPTOS_RELIABILITY       0x04
#define IPTOS_LOWCOST           0x02
#define IPTOS_MINCOST           IPTOS_LOWCOST

/*
 * The Network Control precedence designation is intended to be used
 * within a network only.  The actual use and control of that
 * designation is up to each network. The Internetwork Control
 * designation is intended for use by gateway control originators only.
 * If the actual use of these precedence designations is of concern to
 * a particular network, it is the responsibility of that network to
 * control the access to, and use of, those precedence designations.
 */
#define IPTOS_PREC_MASK                 0xe0
#define IPTOS_PREC(tos)                ((tos) & IPTOS_PREC_MASK)
#define IPTOS_PREC_NETCONTROL           0xe0
#define IPTOS_PREC_INTERNETCONTROL      0xc0
#define IPTOS_PREC_CRITIC_ECP           0xa0
#define IPTOS_PREC_FLASHOVERRIDE        0x80
#define IPTOS_PREC_FLASH                0x60
#define IPTOS_PREC_IMMEDIATE            0x40
#define IPTOS_PREC_PRIORITY             0x20
#define IPTOS_PREC_ROUTINE              0x00


/*
 * Commands for ioctlsocket(),  taken from the BSD file fcntl.h.
 * lwip_ioctl only supports FIONREAD and FIONBIO, for now
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#if !defined(FIONREAD) || !defined(FIONBIO)
#define IOCPARM_MASK    0x7fU           /* parameters must be < 128 bytes */
#define IOC_VOID        0x20000000UL    /* no parameters */
#define IOC_OUT         0x40000000UL    /* copy out parameters */
#define IOC_IN          0x80000000UL    /* copy in parameters */
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                        /* 0x20000000 distinguishes new &
                                           old ioctl's */
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#endif /* !defined(FIONREAD) || !defined(FIONBIO) */

#ifndef FIONREAD
#define FIONREAD    _IOR('f', 127, unsigned long) /* get # bytes to read */
#endif
#ifndef FIONBIO
#define FIONBIO     _IOW('f', 126, unsigned long) /* set/clear non-blocking i/o */
#endif

/* Socket I/O Controls: unimplemented */
#ifndef SIOCSHIWAT
#define SIOCSHIWAT  _IOW('s',  0, unsigned long)  /* set high watermark */
#define SIOCGHIWAT  _IOR('s',  1, unsigned long)  /* get high watermark */
#define SIOCSLOWAT  _IOW('s',  2, unsigned long)  /* set low watermark */
#define SIOCGLOWAT  _IOR('s',  3, unsigned long)  /* get low watermark */
#define SIOCATMARK  _IOR('s',  7, unsigned long)  /* at oob mark? */
#endif

/* commands for fnctl */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* File status flags and file access modes for fnctl,
   these are bits in an int. */
#ifndef O_NONBLOCK
#define O_NONBLOCK  1 /* nonblocking I/O */
#endif
#ifndef O_NDELAY
#define O_NDELAY    1 /* same as O_NONBLOCK, for compatibility */
#endif

#ifndef SHUT_RD
  #define SHUT_RD   0
  #define SHUT_WR   1
  #define SHUT_RDWR 2
#endif

/* FD_SET used for lwip_select */
#ifndef FD_SET
  #undef  FD_SETSIZE
  /* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
  #define FD_SETSIZE    32
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

  typedef struct fd_set {
          unsigned char fd_bits [(FD_SETSIZE+7)/8];
        } fd_set;

#endif /* FD_SET */

#endif

#ifdef FD_SET
  #undef  FD_SETSIZE
  #undef FD_CLR
  #undef FD_ISSET
  #undef FD_SET
  #undef FD_ZERO
  #undef fd_set
  
  #define FD_SETSIZE    64
  #define FD_SET(n, p)  ((p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
  #define FD_CLR(n, p)  ((p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
  #define FD_ISSET(n,p) ((p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
  #define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

  typedef struct fd_set {
          unsigned char fd_bits [(FD_SETSIZE+7)/8];
        } fd_set;
#endif

uint16_t IotSocketHtons(uint16_t n);

uint16_t IotSocketNtohs(uint16_t n);

uint32_t IotSocketHtonl(uint32_t n);

uint32_t IotSocketNtohl(uint32_t n);


int IotSocketOpenSock(int domain, int type, int protocol);

int IotSocketCloseSock(int s);

int IotSocketAccept(int s, struct sockaddr *addr, socklen_t *addrlen);

int IotSocketBind(int s, const struct sockaddr *name, socklen_t namelen);

int IotSocketConnect(int s, const struct sockaddr *name, socklen_t namelen);

int IotSocketListen(int s, int backlog);

int IotSocketRecv(int s, void *mem, size_t len, int flags);

int IotSocketRecvFrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);

//int IotSocketRead(int s, void *mem, size_t len);

int IotSocketSend(int s, const void *dataptr, size_t size, int flags);

int IotSocketSendTo(int s, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);

//int IotSocketWrite(int s, const void *data, size_t size);

int IotSocketSelect(int maxfds, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);

int IotSocketGetSockName(int s, struct sockaddr *name, socklen_t* namelen);

int IotSocketGetPeerName(int s, struct sockaddr *name, socklen_t* namelen);

int IotSocketSetSockOpt(int s, int level, int optname, const void *optval, socklen_t optlen);

int IotSocketGetSockOpt(int s, int level, int optname, void *optval, socklen_t* optlen);

int IotSocketShutdown(int s, int how);

int IotSocketIoCtrl(int s, long cmd, void *argp);

void IotSocketInit(void);

//int IotSocketFcntl(int s, int cmd, int val);

#define accept(a,b,c)         IotSocketAccept(a,b,c)
#define bind(a,b,c)           IotSocketBind(a,b,c)
#define shutdown(a,b)         IotSocketShutdown(a,b)
#define closesocket(s)        IotSocketCloseSock(s)
#define connect(a,b,c)        IotSocketConnect(a,b,c)
#define getsockname(a,b,c)    IotSocketGetSockName(a,b,c)
#define getpeername(a,b,c)    IotSocketGetPeerName(a,b,c)
#define setsockopt(a,b,c,d,e) IotSocketSetSockOpt(a,b,c,d,e)
#define getsockopt(a,b,c,d,e) IotSocketGetSockOpt(a,b,c,d,e)
#define listen(a,b)           IotSocketListen(a,b)
#define recv(a,b,c,d)         IotSocketRecv(a,b,c,d)
#define recvfrom(a,b,c,d,e,f) IotSocketRecvFrom(a,b,c,d,e,f)
#define send(a,b,c,d)         IotSocketSend(a,b,c,d)
#define sendto(a,b,c,d,e,f)   IotSocketSendTo(a,b,c,d,e,f)
#define socket(a,b,c)         IotSocketOpenSock(a,b,c)
#define select(a,b,c,d,e)     IotSocketSelect(a,b,c,d,e)
#define ioctlsocket(a,b,c)    IotSocketIoCtrl(a,b,c)

#if 1
#define read(a,b,c)           IotSocketRead(a,b,c)
#define write(a,b,c)          IotSocketWrite(a,b,c)
#define close(s)              IotSocketCloseSock(s)
#define fcntl(a,b,c)          IotSocketFcntl(a,b,c)
#endif /* LWIP_POSIX_SOCKETS_IO_NAMES */

#ifdef __cplusplus
}
#endif

#endif /* IOT_SOCKET_H */
/** @} */ // end of IOT_SOCKET_H
