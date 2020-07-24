#ifndef	ITE_WIFI_NDIS_H
#define	ITE_WIFI_NDIS_H

#include "if.h" // An implementation of the TCP/IP protocol suite.
#include "iw_handler.h"

#if 0
/* interface flags (netdevice->flags). */
#define IFF_UP          0x1             /* interface is up              */
#define IFF_BROADCAST   0x2             /* broadcast address valid      */
#define IFF_DEBUG       0x4             /* turn on debugging            */
#define IFF_LOOPBACK    0x8             /* is a loopback net            */
#define IFF_POINTOPOINT 0x10            /* interface is has p-p link    */
#define IFF_NOTRAILERS  0x20            /* avoid use of trailers        */
#define IFF_RUNNING     0x40            /* resources allocated          */
#define IFF_NOARP       0x80            /* no ARP protocol              */
#define IFF_PROMISC     0x100           /* receive all packets          */
#define IFF_ALLMULTI    0x200           /* receive all multicast packets*/
#define IFF_MASTER      0x400           /* master of a load balancer    */
#define IFF_SLAVE       0x800           /* slave of a load balancer     */
#define IFF_MULTICAST   0x1000          /* Supports multicast           */

#define IFF_VOLATILE    (IFF_LOOPBACK|IFF_POINTOPOINT|IFF_BROADCAST|IFF_MASTER|IFF_SLAVE|IFF_RUNNING)
#endif

#define IFF_PORTSEL     0x2000          /* can set media type           */
#define IFF_AUTOMEDIA   0x4000          /* auto media select active     */
#define IFF_DYNAMIC     0x8000          /* dialup device with changing addresses*/

#define	__LINK_STATE_XOFF 				(1 << 0)
#define	__LINK_STATE_START				(1 << 1)
#define	__LINK_STATE_PRESENT			(1 << 2)
#define	__LINK_STATE_SCHED				(1 << 3)
#define	__LINK_STATE_NOCARRIER			(1 << 4)
#define	__LINK_STATE_RX_SCHED			(1 << 5)
#define	__LINK_STATE_LINKWATCH_PENDING	(1 << 6)

#define netif_start_queue(dev)		    (dev->state &= ~__LINK_STATE_XOFF)
#define netif_stop_queue(dev)		    (dev->state |= __LINK_STATE_XOFF)
#define netif_queue_stopped(dev)		(dev->state & __LINK_STATE_XOFF)
#define netif_wake_queue(dev)		    (dev->state &= ~__LINK_STATE_XOFF)
#define netif_carrier_on(dev)			(dev->state &= ~__LINK_STATE_NOCARRIER)
#define netif_carrier_off(dev)			(dev->state |= __LINK_STATE_NOCARRIER)
#define netif_running(dev)              (dev->state & __LINK_STATE_START)


struct net_device *smNetAlloc(int sizeof_priv);
void* smNetdev_priv(struct net_device *dev);
void smNetFree(struct net_device*);
int smNetOpen(struct net_device *dev, input_fn input, void *arg);
int smNetClose(struct net_device *dev);
void smNetTransmit(struct net_device *dev, struct sk_buff *skb);
void smNetReceive(struct sk_buff *skb);
int smNetIOCtrl(struct net_device *dev, void* ptr, int cmd);
int smNetCtrl(struct net_device *dev, struct net_device_config *netDeviceConfig);
int smNetInfo(struct net_device *dev, struct net_device_info *netDeviceInfo);
struct net_device* smNetGetDevice(void);
#endif
