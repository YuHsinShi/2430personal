#include "ite/ith.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>


struct net_device *usbd_ncm_netdev = NULL;

#define ERR_USBD_NCM_NO_NETDEV               0x00002001
#define ERR_USBD_NCM_CARRIER_OFF             0x00002002
#define ERR_USBD_NCM_OPEN_FAIL               0x00002003
#define ERR_USBD_NCM_TX_ALLOC_SDK_FAIL       0x00002004

#define check_result(rc) do { if (rc) printf("[%s] res = %d(0x%08X) \n", __FUNCTION__, rc, rc); } while (0)


int iteUsbdNcmOpen(uint8_t* mac_addr, void(*func)(void *arg, void *packet, int len), void* arg)
{
    int ret;
    struct net_device *dev = usbd_ncm_netdev;
    const struct net_device_ops *ops;
printf("iteUsbdNcmOpen() \n");
    if (!dev) {
        ret = ERR_USBD_NCM_NO_NETDEV;
        goto end;
    }

    if (dev->flags & IFF_UP)
        return 0;

    if (!netif_carrier_ok(dev)) {
        ret = ERR_USBD_NCM_CARRIER_OFF;
        goto end;
    }

    if (mac_addr)
        memcpy(mac_addr, dev->dev_addr, 6);

    netif_start(dev);

    dev->rx_netif = arg;
    dev->rx_cb = func;

    ops = dev->netdev_ops;
    ret = ops->ndo_open(dev);
    if (ret) {
        netif_stop(dev);
        printf("%s():open fail: 0xX \n", __func__, ret);
        ret = ERR_USBD_NCM_OPEN_FAIL;
    }
    else {
        dev->flags |= IFF_UP;
        //ops->ndo_set_rx_mode(dev);
    }

end:
    check_result(ret);
    return ret;
}

int iteUsbdNcmStop(void)
{
    int ret = 0;
    struct net_device *dev = usbd_ncm_netdev;

    if (!dev) {
        ret = ERR_USBD_NCM_NO_NETDEV;
        goto end;
    }

    if (dev->flags & IFF_UP) {
        const struct net_device_ops *ops = dev->netdev_ops;

        netif_stop(dev);
        ops->ndo_stop(dev);
        dev->flags &= ~IFF_UP;
    }

end:
    check_result(ret);
    return ret;
}

int iteUsbdNcmSend(void* packet, uint32_t len)
{
    int ret = 0;
    struct net_device *dev = usbd_ncm_netdev;
    const struct net_device_ops *ops;
    struct sk_buff *skb;

    if (!dev) {
        ret = ERR_USBD_NCM_NO_NETDEV;
        goto end;
    }

    if (!netif_running(dev) || !netif_carrier_ok(dev))
        return 0;

    ops = dev->netdev_ops;

    /* wait for tx queue available */
    while (!netif_tx_queue_ok(dev)) {
        printf("&\n");
        udelay(50);
    }

    skb = __netdev_alloc_skb(dev, 0, 0);
    if (!skb) {
        ret = ERR_USBD_NCM_TX_ALLOC_SDK_FAIL;
        goto end;
    }
    skb->data = packet;
    skb->len = len;
    if (skb->len < ETH_ZLEN)
        skb->len = ETH_ZLEN;

    ret = ops->ndo_start_xmit(skb, dev);
    while (ret == NETDEV_TX_BUSY) {
        printf("@\n");
        udelay(50);
    }

end:
    check_result(ret);
    return ret;
}

int iteUsbdNcmGetLink(void)
{
    struct net_device *dev = usbd_ncm_netdev;

    if (!dev)
        return 0;

    return netif_carrier_ok(dev) ? 1 : 0;

}

int iteUsbdNcmSetRxMode(int flags)
{
    int ret = 0;
    struct net_device *dev = usbd_ncm_netdev;

    if (!dev) {
        ret = ERR_USBD_NCM_NO_NETDEV;
        goto end;
    }

    dev->flags &= ~(IFF_PROMISC | IFF_ALLMULTI);
    dev->flags |= flags;
    if (dev->netdev_ops->ndo_set_rx_mode)
        dev->netdev_ops->ndo_set_rx_mode(dev);
    else
        return -1;

end:
    return ret;
}
