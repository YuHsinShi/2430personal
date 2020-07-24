
//=============================================================================
//							LOG
//=============================================================================
#define DEV_DBG     0
#define DEV_ERR     1
#define DEV_INFO    0
#define NETDEV_ERR  1
#define NETDEV_DBG  0
#define NETDEV_INFO 0
#define NETIF_INFO  0
#define NETIF_DBG   0
#define NETIF_ERR   1

#if DEV_DBG
#define dev_dbg(ddev, string, args...)    do { ithPrintf("[ECM][dev_dbg] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_dbg(ddev, string, args...)
#endif

#if DEV_ERR
#define dev_err(ddev, string, args...)    do { ithPrintf("[ECM][dev_err] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_err(ddev, string, args...)
#endif

#if DEV_INFO
#define dev_info(ddev, string, args...)    do { ithPrintf("[ECM][dev_info] "); ithPrintf(string, ## args); } while (0)
#else
#define dev_info(ddev, string, args...)
#endif

#if NETDEV_ERR
#define netdev_err(netdev, string, args...)    do { ithPrintf("[ECM][netdev_err] "); ithPrintf(string, ## args); } while (0)
#else
#define netdev_err(netdev, string, args...)
#endif

#if NETDEV_DBG
#define netdev_dbg(netdev, string, args...)    do { ithPrintf("[ECM][netdev_dbg] "); ithPrintf(string, ## args); } while (0)
#else
#define netdev_dbg(netdev, string, args...)
#endif

#if NETDEV_INFO
#define netdev_info(netdev, string, args...)    do { ithPrintf("[ECM][netdev_info] "); ithPrintf(string, ## args); } while (0)
#else
#define netdev_info(netdev, string, args...)
#endif

#define ifup    	"ifup:"
#define _timer   	"timer:"
#define ifdown  	"ifdown:"
#define rx_status   "rx_status:"
#define rx_err  	"rx_err:"
#define tx_err  	"tx_err:"
#define _probe   	"probe:"
#define tx_queued   "tx_queued:"
#define _link		"link:"

#if NETIF_INFO
#define netif_info(dev, xx, net, string, args...)    do { ithPrintf("[ECM][netif_info]%s ", xx); ithPrintf(string, ## args); } while (0)
#else
#define netif_info(dev, xx, net, string, args...)
#endif

#if NETIF_DBG
#define netif_dbg(dev, xx, net, string, args...)    do { ithPrintf("[ECM][netif_dbg]%s ", xx); ithPrintf(string, ## args); } while (0)
#else
#define netif_dbg(dev, xx, net, string, args...)
#endif

#if NETIF_ERR
#define netif_err(dev, xx, net, string, args...)    do { ithPrintf("[ECM][netif_err]%s ", xx); ithPrintf(string, ## args); } while (0)
#else
#define netif_err(dev, xx, net, string, args...)
#endif

#define check_result(rc) do { if (rc) ithPrintf("[ECM][ERR][%s] res = %d(0x%08X) \n", __FUNCTION__, rc, rc); } while (0)


