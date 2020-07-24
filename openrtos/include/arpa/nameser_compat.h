#ifndef _ARPA_NAMESER_COMPAT_
#define	_ARPA_NAMESER_COMPAT_

#define	__BIND		19950621	/* (DEAD) interface version stamp. */

#include <sys/param.h>

#if !defined(BYTE_ORDER) || \
    (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN && \
    BYTE_ORDER != PDP_ENDIAN)
	/* you must determine what the correct bit order is for
	 * your compiler - the next line is an intentional error
	 * which will force your compiles to bomb until you fix
	 * the above macros.
	 */
  error "Undefined or invalid BYTE_ORDER";
#endif

/*
 * Structure for query header.  The order of the fields is machine- and
 * compiler-dependent, depending on the byte/bit order and the layout
 * of bit fields.  We use bit fields only in int variables, as this
 * is all ANSI requires.  This requires a somewhat confusing rearrangement.
 */

typedef struct {
	unsigned	id :16;		/* query identification number */
#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
			/* fields in third byte */
	unsigned	rd :1;		/* recursion desired */
	unsigned	tc :1;		/* truncated message */
	unsigned	aa :1;		/* authoritive answer */
	unsigned	opcode :4;	/* purpose of message */
	unsigned	qr :1;		/* response flag */
			/* fields in fourth byte */
	unsigned	rcode :4;	/* response code */
	unsigned	cd: 1;		/* checking disabled by resolver */
	unsigned	ad: 1;		/* authentic data from named */
	unsigned	unused :1;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned	ra :1;		/* recursion available */
#endif
			/* remaining bytes */
	unsigned	qdcount :16;	/* number of question entries */
	unsigned	ancount :16;	/* number of answer entries */
	unsigned	nscount :16;	/* number of authority entries */
	unsigned	arcount :16;	/* number of resource entries */
} HEADER;

/*
* Define constants based on rfc883
*/
#define PACKETSZ	512		/* maximum packet size */
#define MAXDNAME	256		/* maximum domain name */
#define MAXCDNAME	255		/* maximum compressed domain name */
#define MAXLABEL	63		/* maximum length of domain label */
/* Number of bytes of fixed size data in query structure */
#define QFIXEDSZ	4
/* number of bytes of fixed size data in resource record */
#define RRFIXEDSZ	10

/*
* Internet nameserver port number
*/
#define NAMESERVER_PORT	53

/*
* Currently defined opcodes
*/
#define QUERY		0		/* standard query */
#define IQUERY		1		/* inverse query */
#define CQUERYM		2		/* completion query (multiple) */
#define CQUERYU		3		/* completion query (unique) */
/* non standard */
#define UPDATEA		100		/* add resource record */
#define UPDATED		101		/* delete resource record */
#define UPDATEM		102		/* modify resource record */
#define ZONEINIT	103		/* initial zone transfer */
#define ZONEREF		104		/* incremental zone referesh */

/*
* Currently defined response codes
*/
#define NOERROR		0		/* no error */
#define FORMERR		1		/* format error */
#define SERVFAIL	2		/* server failure */
#define NXDOMAIN	3		/* non existent domain */
#define NOTIMP		4		/* not implemented */
#define REFUSED		5		/* query refused */
/* non standard */
#define NOCHANGE	100		/* update failed to change db */

/*
* Type values for resources and queries
*/
#define T_A		1		/* host address */
#define T_NS		2		/* authoritative server */
#define T_MD		3		/* mail destination */
#define T_MF		4		/* mail forwarder */
#define T_CNAME		5		/* connonical name */
#define T_SOA		6		/* start of authority zone */
#define T_MB		7		/* mailbox domain name */
#define T_MG		8		/* mail group member */
#define T_MR		9		/* mail rename name */
#define T_NULL		10		/* null resource record */
#define T_WKS		11		/* well known service */
#define T_PTR		12		/* domain name pointer */
#define T_HINFO		13		/* host information */
#define T_MINFO		14		/* mailbox information */
#define T_MX		15		/* mail routing information */
/* non standard */
#define T_UINFO		100		/* user (finger) information */
#define T_UID		101		/* user ID */
#define T_GID		102		/* group ID */
/* Query type values which do not appear in resource records */
#define T_AXFR		252		/* transfer zone of authority */
#define T_MAILB		253		/* transfer mailbox records */
#define T_MAILA		254		/* transfer mail agent records */
#define T_ANY		255		/* wildcard match */

/*
* Values for class field
*/

#define C_IN		1		/* the arpa internet */
#define C_CHAOS		3		/* for chaos net at MIT */
/* Query class values which do not appear in resource records */
#define C_ANY		255		/* wildcard match */

#endif /* _ARPA_NAMESER_COMPAT_ */
