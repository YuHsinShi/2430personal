/*
* Copyright (c) 2019 ITE Tech. Inc. All Rights Reserved.
*/
/** @file
*  CDC-ECM extern API implementation.
*
* @author Irene Lin
* @version 1.0
*/
#include "ite/ith.h"
#include "ite/itp.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "debug.h"

struct net_device *g_netdev = NULL;

#define ERR_ECM_NO_NETDEV               0x00001001
#define ERR_ECM_DEV_NOT_PRESENT         0x00001002
#define ERR_ECM_OPEN_FAIL               0x00001003
#define ERR_ECM_TX_ALLOC_SDK_FAIL       0x00001004

int iteEcmOpen(uint8_t* mac_addr, void(*func)(void *arg, void *packet, int len), void* arg)
{
    int ret;
    struct net_device *dev = g_netdev;
    const struct net_device_ops *ops;

    if (!dev) {
        ret = ERR_ECM_NO_NETDEV;
        goto end;
    }

    if (dev->flags & IFF_UP)
        return 0;

    if (!netif_device_present(dev)) {
        ret = ERR_ECM_DEV_NOT_PRESENT;
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
        printf("ecm open fail: 0xX \n", ret);
        ret = ERR_ECM_OPEN_FAIL;
    }
    else {
        dev->flags |= IFF_UP;
        //ops->ndo_set_rx_mode(dev);
    }

end:
    check_result(ret);
    return ret;
}

int iteEcmStop(void)
{
    int ret = 0;
    struct net_device *dev = g_netdev;

    if (!dev) {
        ret = ERR_ECM_NO_NETDEV;
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

int iteEcmSend(void* packet, uint32_t len)
{
    int ret = 0;
    struct net_device *dev = g_netdev;
    const struct net_device_ops *ops;
    struct sk_buff *skb;

    if (!dev) {
        ret = ERR_ECM_NO_NETDEV;
        goto end;
    }

    if (!netif_running(dev) || !netif_device_present(dev))
        return 0;

    ops = dev->netdev_ops;

    /* wait for tx queue available */
    while (!netif_tx_queue_ok(dev)) {
        printf("&\n");
        udelay(50);
    }

    skb = __netdev_alloc_skb(dev, 0, 0);
    if (!skb) {
        ret = ERR_ECM_TX_ALLOC_SDK_FAIL;
        goto end;
    }
    skb->data = packet;
    skb->len = len;
    if (skb->len < ETH_ZLEN)
        skb->len = ETH_ZLEN;

    ret = ops->ndo_start_xmit(skb, dev);

end:
    check_result(ret);
    return ret;
}

int iteEcmGetLink(void)
{
    struct net_device *dev = g_netdev;

    if (!dev)
        return 0;

    return netif_carrier_ok(dev) ? 1 : 0;
}

int iteEcmSetRxMode(int flags)
{
    int ret = 0;
    struct net_device *dev = g_netdev;

    if (!dev) {
        ret = ERR_ECM_NO_NETDEV;
        goto end;
    }

    dev->flags &= ~(IFF_PROMISC | IFF_ALLMULTI);
    dev->flags |= flags;
    dev->netdev_ops->ndo_set_rx_mode(dev);

end:
    return ret;
}

