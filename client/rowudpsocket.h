#ifndef _ROWUDPSOCKET_H_
#define _ROWUDPSOCKET_H_


#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PCKT_LEN 8192

int rowudpsendto(int sd, const void *data, size_t  datalength, int flags, const struct sockaddr_in *src_addr,  socklen_t src_addrlen, const struct sockaddr_in *dest_addr,  socklen_t dest_addrlen );
int rowsocket();



struct row_udphdr {
    u_short	source;		/* source port */
    u_short	dest;		/* destination port */
    short	len;		/* udp length */
    u_short	check;			/* udp checksum */
};

#endif
