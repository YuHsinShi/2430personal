/*
 * Copyright (c) 2019 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 *
 * @author Irene Lin
 * @version 1.0
 */

#ifndef NET_DEVICE_H
#define NET_DEVICE_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================
#include <net/if.h>
#include <linux/bitops.h>
#include <linux/if_ether.h>


//=============================================================================
//                              Constant Definition
//=============================================================================

#if 0
#define ETH_HLEN        14
#define ETH_DATA_LEN	    1500
#define ETH_ALEN        	6
#define ETH_ZLEN	        60		/* Min. octets in frame sans FCS */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */

#define ETH_P_802_2	0x0004		/* 802.2 frames 		*/
#define ETH_P_802_3_MIN	0x0600		/* If the value in the ethernet type is less than this value */
#endif

//=============================================================================
//                              Structure Definition
//=============================================================================
struct net_device_stats {
    unsigned int    	rx_errors;
    unsigned int    	rx_packets;
    unsigned long   	rx_bytes;
    unsigned int    	rx_length_errors;
    unsigned int    	rx_over_errors;
    unsigned int    	tx_packets;
    unsigned int    	tx_errors;
    unsigned long   	tx_bytes;
    unsigned int    	tx_dropped;
};

struct sk_buff;
struct net_device;

enum netdev_tx {
    __NETDEV_TX_MIN = INT_MIN,	/* make sure enum is signed */
    NETDEV_TX_OK = 0x00,	/* driver took care of packet */
    NETDEV_TX_BUSY = 0x10,	/* driver tx path was busy*/
};
typedef enum netdev_tx netdev_tx_t;

struct net_device_ops {
    int(*ndo_open)(struct net_device *dev);
    int(*ndo_stop)(struct net_device *dev);
    netdev_tx_t(*ndo_start_xmit)(struct sk_buff *skb, struct net_device *dev);
    void(*ndo_tx_timeout) (struct net_device *dev);
    void(*ndo_set_rx_mode)(struct net_device *dev);
    int(*ndo_change_mtu)(struct net_device *dev, int new_mtu);
    int(*ndo_set_mac_address)(struct net_device *dev, void *addr);
    int(*ndo_validate_addr)(struct net_device *dev);
};

struct net_device 
{
    char			        name[16];
    unsigned long		state;
#define __LINK_STATE_START  	(0x1 << 0)
#define __LINK_STATE_PRESENT  	(0x1 << 1)
#define __LINK_STATE_NOCARRIER  (0x1 << 2)
#define __LINK_STATE_LINKWATCH_PENDING  (0x1 << 3)
#define __LINK_STATE_DORMANT  	(0x1 << 4)
	
    int		            watchdog_timeo;
    unsigned short	    priv_flags;     /* private flags   */
#define TX_QUEUE_XOFF   0x1
    unsigned short	    flags;          /* interface flags   */
    void			        *priv;		    /* pointer to private data */
    unsigned char		dev_addr[ETH_ALEN];	/* set during bootup */
    const struct net_device_ops *netdev_ops;
    unsigned long       mc_count;  /* Multicast mac addresses count */
    unsigned char       *mc_list;
    unsigned int		    mtu;
    unsigned int		    min_mtu;
    unsigned int		    max_mtu;
    unsigned short		hard_header_len;

    struct net_device_stats stats;

    /* for lwip interface */
    void(*rx_cb)(void *arg, void *packet, int len);
    void*           rx_netif;
};

#if 0
/* This is an Ethernet frame header. */
struct ethhdr {
    unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
    unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
    //__be16		h_proto;		/* packet type ID field	*/
    unsigned short	h_proto;		/* packet type ID field	*/
} __attribute__((packed));
#endif

#ifndef htons
#define htons(x)        cpu_to_be16(x)
#endif

//=============================================================================
//                              Macro Definition
//=============================================================================
#define netif_start_queue		netif_tx_start_queue
#define netif_wake_queue        netif_tx_start_queue
#define netif_stop_queue		    netif_tx_queue_off
#define netif_tx_stop_queue		    netif_tx_queue_off
#define netif_tx_start_queue(dev)   ((dev)->priv_flags &= ~TX_QUEUE_XOFF)
#define netif_tx_queue_off(dev)     ((dev)->priv_flags |= TX_QUEUE_XOFF)
#define netif_tx_queue_ok(dev)      (!(((dev)->priv_flags) & TX_QUEUE_XOFF))
#define netif_queue_stopped(dev)	    (dev->priv_flags & TX_QUEUE_XOFF)

#define netif_start(dev)            ((dev)->state |= __LINK_STATE_START)
#define netif_running(dev)          ((dev)->state & __LINK_STATE_START)
#define netif_stop(dev)             ((dev)->state &= ~__LINK_STATE_START)

#define netif_device_attach(dev)    ((dev)->state |= __LINK_STATE_PRESENT)
#define netif_device_present(dev)   ((dev)->state & __LINK_STATE_PRESENT)

static inline void netif_device_detach(struct net_device *dev)
{
    if (test_and_clear_bit(__LINK_STATE_PRESENT, &dev->state) &&
        netif_running(dev)) {
        netif_tx_stop_queue(dev);
    }
}

#define netif_carrier_ok(dev)       (!(((dev)->state) & __LINK_STATE_NOCARRIER))
#define netif_carrier_on(dev)       ((dev)->state &= ~__LINK_STATE_NOCARRIER)
#define netif_carrier_off(dev)      ((dev)->state |= __LINK_STATE_NOCARRIER)


#define netdev_priv(dev)		    (dev->priv)
#define netdev_mc_count(dev)	    (dev->mc_count)
#define netdev_mc_empty(dev)    (dev->mc_count == 0)

#define SET_NETDEV_DEV(net, x)  do { } while(0)


#define net_ratelimit()         0
#define netif_msg_tx_err(dev)   1
#define netif_msg_rx_err(dev)   1
#define netif_trans_update(net)

/* Driver transmit return codes */
#define NETDEV_TX_MASK		0xf0


//=============================================================================
//							Funtion Declaration
//=============================================================================



#ifdef __cplusplus
}
#endif

#endif //NET_DEVICE_H
