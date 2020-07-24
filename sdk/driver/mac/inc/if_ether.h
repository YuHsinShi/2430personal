
#ifndef MAC_IF_ETHER_H
#define MAC_IF_ETHER_H


#define ETH_ALEN	6		/* Octets in one ethernet addr	 */

#define ETH_HLEN	14		/* Total octets in header.	 */
#define ETH_ZLEN	60		/* Min. octets in frame sans FCS */
#define ETH_DATA_LEN	1500		/* Max. octets in payload	 */
#define ETH_FRAME_LEN	1514		/* Max. octets in frame sans FCS */
#define ETH_FCS_LEN	4		/* Octets in the FCS		 */

#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/
#define ETH_P_IPV6	0x86DD		/* IPv6 over bluebook		*/

#define ETH_P_802_3_MIN	0x0600

#define ETH_P_802_2	0x0004		/* 802.2 frames 		*/


struct ethhdr {
    unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
    unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
    //__be16		h_proto;		/* packet type ID field	*/
    unsigned short	h_proto;		/* packet type ID field	*/
} __attribute__((packed));


#endif /* MAC_IF_ETHER_H */
