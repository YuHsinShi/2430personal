/* OPTIONAL: Pools to replace heap allocation
 * Optional: Pools can be used instead of the heap for mem_malloc. If
 * so, these should be defined here, in increasing order according to
 * the pool element size.
 *
 * LWIP_MALLOC_MEMPOOL(number_elements, element_size)
 */
#if MEM_USE_POOLS
LWIP_MALLOC_MEMPOOL_START
LWIP_MALLOC_MEMPOOL(32, 128)
LWIP_MALLOC_MEMPOOL(32, 256) //100
LWIP_MALLOC_MEMPOOL(50, 512)
LWIP_MALLOC_MEMPOOL(20, 1024)
#ifdef CFG_IPTV_TX
LWIP_MALLOC_MEMPOOL(2560, 1536)
#elif defined(CFG_NET_WIFI_VIDEO_DOORBELL)
LWIP_MALLOC_MEMPOOL(320, 1536) //200
#else
LWIP_MALLOC_MEMPOOL(32, 1536) //200
#endif
LWIP_MALLOC_MEMPOOL_END
#endif /* MEM_USE_POOLS */

/* Optional: Your custom pools can go here if you would like to use
 * lwIP's memory pools for anything else.
 */
LWIP_MEMPOOL(SYS_MBOX, 22, 100, "SYS_MBOX")
