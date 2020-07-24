#include <pthread.h>
#include <errno.h>
#include "lwip/sockets.h"
#include "lwip/ip.h"

///////////////////////////////////////////
#define SENDER_PORT_NUM 6001
#define SERVER_PORT_NUM 12345
#define SERVER_IP_ADDRESS "192.168.190.102"
///////////////////////////////////////////

int network_tcp_client_main(void)
{
	int socket_fd, connect_fd, data_count=0;
	int recv_timeout;
	int keep_alive_opt = 1, tcp_keepidle = 120, tcp_keepintvl = 10, tcp_keepcnt = 5;
	char * pbuffer = "OK";
	struct sockaddr_in sa, ra;
	struct timeval timeout={5,0}; //5 sec

	int recv_data; char data_buffer[80];
	/* Creates an TCP socket (SOCK_STREAM) with Internet Protocol Family (PF_INET).
	  * Protocol family and Address family related. For example PF_INET Protocol Family and AF_INET family are coupled.
	  */

	socket_fd = lwip_socket(PF_INET, SOCK_STREAM, 0);
	//fcntl(socket_fd, F_SETFL, O_NONBLOCK); //set non-blocking socket

	if ( socket_fd < 0 )
	{

		printf("socket call failed");
		return 0;
	}

	/*memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(SENDER_IP_ADDR);
	sa.sin_port = htons(SENDER_PORT_NUM);*/


	/* Bind the TCP socket to the port SENDER_PORT_NUM and to the current
	* machines IP address (Its defined by SENDER_IP_ADDR).
	* Once bind is successful for UDP sockets application can operate
	* on the socket descriptor for sending or receiving data.
	*/
	/*if (lwip_bind(socket_fd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) == -1)
	{
		printf("Bind to Port Number %d ,IP address %s failed\n", SENDER_PORT_NUM, SENDER_IP_ADDR);
		lwip_close(socket_fd);
		return 1;
	}*/
	/* Receiver connects to server ip-address. */

	memset(&ra, 0, sizeof(struct sockaddr_in));
	ra.sin_family = AF_INET;
	ra.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	ra.sin_port = htons(SERVER_PORT_NUM);

	sleep(5);

	connect_fd = lwip_connect(socket_fd, (struct sockaddr*)&ra, sizeof(ra));

	if(connect_fd < 0)
	{
		printf("connect failed: %d \n", connect_fd);
		lwip_close(socket_fd);
		return 0;
	}

	//recv_timeout = lwip_setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	lwip_setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keep_alive_opt, sizeof(keep_alive_opt));
	lwip_setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&tcp_keepidle, sizeof(tcp_keepidle));
	lwip_setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&tcp_keepintvl, sizeof(tcp_keepintvl));
	lwip_setsockopt(socket_fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&tcp_keepcnt, sizeof(tcp_keepcnt));

	while (1){
        recv_data = lwip_recv(socket_fd, data_buffer, sizeof(data_buffer), 0);
		++data_count;

		if(recv_data == -1 && errno == EAGAIN)
		{
		    printf("timeout\n");
		}

	    if(recv_data > 0)
        {
            data_buffer[recv_data] = '\0';
            printf("received data[%d bytes][%d]: %s \n", recv_data, data_count, data_buffer);

			lwip_send(socket_fd, pbuffer, strlen(pbuffer), 0);
		}else{
		    printf("recv failed \n");
		    lwip_close(socket_fd);
		    return 0;
	    }
	}
}
