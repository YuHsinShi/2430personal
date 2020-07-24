#ifndef _ETHER_DEVICE_H
#define _ETHER_DEVICE_H

#include <stdlib.h>
#include "netdevice.h"

static inline void get_random_bytes(u8 *addr, int len)
{
    static u32 aa;
    int i;
    srand(clock()+(aa++));
    for (i = 0; i < len; i++)
        addr[i] = ((rand()+(aa++)) % 256);
}

/**
* eth_random_addr - Generate software assigned random Ethernet address
* @addr: Pointer to a six-byte array containing the Ethernet address
*
* Generate a random Ethernet address (MAC) that is not multicast
* and has the local assigned bit set.
*/
static inline void eth_random_addr(u8 *addr)
{
    get_random_bytes(addr, ETH_ALEN);
    addr[0] &= 0xfe;	/* clear multicast bit */
    addr[0] |= 0x02;	/* set local assignment bit (IEEE802) */
}

#define random_ether_addr(addr) eth_random_addr(addr)

/**
* is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
* @addr: Pointer to a six-byte array containing the Ethernet address
*
* Return true if the address is a multicast address.
* By definition the broadcast address is also a multicast address.
*/
static inline bool is_multicast_ether_addr(const u8 *addr)
{
    u32 a = *(const u32 *)addr;

#if BYTE_ORDER == LITTLE_ENDIAN
    return 0x01 & a;
#else
    return 0x01 & (a >> ((sizeof(a)* 8) - 8));
#endif
}

/**
* is_broadcast_ether_addr - Determine if the Ethernet address is broadcast
* @addr: Pointer to a six-byte array containing the Ethernet address
*
* Return true if the address is the broadcast address.
*
* Please note: addr must be aligned to u16.
*/
static inline bool is_broadcast_ether_addr(const u8 *addr)
{
    return (*(const u16 *)(addr + 0) &
        *(const u16 *)(addr + 2) &
        *(const u16 *)(addr + 4)) == 0xffff;
}


#endif /* _ETHER_DEVICE_H */
