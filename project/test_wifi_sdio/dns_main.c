#include "test_config.h"

#if test_dns
void dns_resolve(void)
{
	char* argv[] = {"showip", "www.google.com"};

	dns_resolve_main(ITH_COUNT_OF(argv), argv);
}

int dns_resolve_main(int argc, char *argv[])
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[46];

	if (argc != 2) {
		fprintf(stderr,"usage: showip hostname\n");
		return 1;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = lwip_getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
		//lwip_freeaddrinfo(res);
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("\n\nIP addresses for %s:\n\n", argv[1]);

	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;

		if (p->ai_family == AF_INET) { // IPv4
		  struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		  addr = &(ipv4->sin_addr);
		  ipver = "IPv4";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n\n", ipver, ipstr);
	}

	lwip_freeaddrinfo(res);

	return 0;
}
#endif

#if test_ping
void ping_main(void)
{
	printf("====>ping test\n");
	ping_set_target("172.217.160.78"); //midea(101.37.128.72), google(172.217.160.78)
	ping_init();
}
#endif
