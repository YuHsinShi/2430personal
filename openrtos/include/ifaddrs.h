#ifndef ITP_IFADDRS_H
#define ITP_IFADDRS_H

#ifdef __cplusplus
extern "C"
{
#endif

struct ifaddrs {
	struct ifaddrs  *ifa_next;
	char		*ifa_name;
	unsigned int	 ifa_flags;
	struct sockaddr	*ifa_addr;
	struct sockaddr	*ifa_netmask;

    union
    {
        /* At most one of the following two is valid.  If the IFF_BROADCAST
        bit is set in `ifa_flags', then `ifa_broadaddr' is valid.  If the
        IFF_POINTOPOINT bit is set, then `ifa_dstaddr' is valid.
        It is never the case that both these bits are set at once.  */
        struct sockaddr *ifu_broadaddr; /* Broadcast address of this interface. */
        struct sockaddr *ifu_dstaddr; /* Point-to-point destination address.  */
    } ifa_ifu;
    /* These very same macros are defined by <net/if.h> for `struct ifaddr'.
    So if they are defined already, the existing definitions will be fine.  */
# ifndef ifa_broadaddr
#  define ifa_broadaddr        ifa_ifu.ifu_broadaddr
# endif
# ifndef ifa_dstaddr
#  define ifa_dstaddr        ifa_ifu.ifu_dstaddr
# endif

	void		*ifa_data;
};

int getifaddrs(struct ifaddrs **ifap);
void freeifaddrs(struct ifaddrs *ifa);

#ifdef __cplusplus
}
#endif

#endif // ITP_IFADDRS_H
