#include "rowudpsocket.h"

uint16_t checksum (uint16_t *addr, int len)
{
    int count = len;
    register uint32_t sum = 0;
    uint16_t answer = 0;
    // Sum up 2-byte values until none or only one byte left.
    while (count > 1)
    {
        sum += *(addr++);
        count -= 2;
    }
    // Add left-over byte, if any.
    if (count > 0)
    {
        sum += *(uint8_t *) addr;
    }
    // Fold 32-bit sum into 16 bits; we lose information by doing this,
    // increasing the chances of a collision.
    // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
    while (sum >> 16)
    {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    // Checksum is one's compliment of sum.
    answer = ~sum;
    return (answer);
}



uint16_t udp4_checksum (struct ip iphdr, struct row_udphdr udphdr, uint8_t *payload, int payloadlen)
{
    char buf[IP_MAXPACKET];
    char *ptr;
    int chksumlen = 0;
    int i;
    ptr = &buf[0];  // ptr points to beginning of buffer buf
    // Copy source IP address into buf (32 bits)
    memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
    ptr += sizeof (iphdr.ip_src.s_addr);
    chksumlen += sizeof (iphdr.ip_src.s_addr);
    // Copy destination IP address into buf (32 bits)
    memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
    ptr += sizeof (iphdr.ip_dst.s_addr);
    chksumlen += sizeof (iphdr.ip_dst.s_addr);
    // Copy zero field to buf (8 bits)
    *ptr = 0; ptr++;
    chksumlen += 1;
    // Copy transport layer protocol to buf (8 bits)
    memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
    ptr += sizeof (iphdr.ip_p);
    chksumlen += sizeof (iphdr.ip_p);
    // Copy UDP length to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);
    // Copy UDP source port to buf (16 bits)
    memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
    ptr += sizeof (udphdr.source);
    chksumlen += sizeof (udphdr.source);
    // Copy UDP destination port to buf (16 bits)
    memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
    ptr += sizeof (udphdr.dest);
    chksumlen += sizeof (udphdr.dest);
    // Copy UDP length again to buf (16 bits)
    memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
    ptr += sizeof (udphdr.len);
    chksumlen += sizeof (udphdr.len);
    // Copy UDP checksum to buf (16 bits)
    // Zero, since we don't know it yet
    *ptr = 0; ptr++;
    *ptr = 0; ptr++;
    chksumlen += 2;
    // Copy payload to buf
    memcpy (ptr, payload, payloadlen);
    ptr += payloadlen;
    chksumlen += payloadlen;
    // Pad to the next 16-bit boundary
    for (i=0; i<payloadlen%2; i++, ptr++)
    {
        *ptr = 0;
        ptr++;
        chksumlen++;
    }
    return checksum ((uint16_t *) buf, chksumlen);
}

// Source IP, source port, target IP, target port from the command line arguments
int rowsocket()
{
    return socket (AF_INET, SOCK_RAW, IPPROTO_RAW);
}

int rowudpsendto(int sd, const void *data, size_t  datalength, int flags, const struct sockaddr_in *src_addr,  socklen_t src_addrlen, const struct sockaddr_in *dest_addr,  socklen_t dest_addrlen )
{
    struct row_udphdr udphdr;
    struct ip iphdr;
    unsigned short packageid = 12345;
    iphdr.ip_hl = sizeof(struct ip) / 4;  //The number of 32-bit words in the header
    iphdr.ip_v = 4;
    iphdr.ip_tos = 0;
    iphdr.ip_len = sizeof(struct ip)+sizeof(struct row_udphdr)+datalength;
    iphdr.ip_id = htons(packageid);
    unsigned int ip_flag[4];
    ip_flag[0] = 0;
    ip_flag[1] = 0;
    ip_flag[2] = 0;
    ip_flag[3] = 0;
    iphdr.ip_off = htons( (ip_flag[0]<<15)+(ip_flag[1]<<14)+(ip_flag[2]<<13)+(ip_flag[3]) );
    iphdr.ip_ttl = 255;
    iphdr.ip_p = IPPROTO_UDP;
    //超重要觀念,struct ip中的ip_srv為struct in_addr ,和struct sockaddr_in中的欄位sin_addr的型別struct in_addr為同一個
    memcpy(&(iphdr.ip_src), &(src_addr->sin_addr), sizeof(struct in_addr) );
    //IP地址轉換函數，可以在將IP地址在“點分十進制”和“二進制整數”之間轉換，而且，inet_pton和inet_ntop這2個函數能夠處理ipv4和ipv6。算是比較新的函數了。
    //int result = inet_pton(AF_INET,SRC_ADDR,&(iphdr.ip_src));
    //超重要觀念,struct ip中的ip_srv為struct in_addr ,和struct sockaddr_in中的欄位sin_addr的型別struct in_addr為同一個
    memcpy(&(iphdr.ip_dst), &(dest_addr->sin_addr), sizeof(struct in_addr) );
    //int result = inet_pton(AF_INET,DEST_ADDR,&(iphdr.ip_dst));
    //checksum
    iphdr.ip_sum = checksum((uint16_t *)&iphdr, sizeof(struct ip));
    //超重要觀念,struct udphdr中的source為u16即unsigned short和struct sockaddr_in中的欄位sin_port的型別unsigned short 為同一個
    udphdr.source = src_addr->sin_port;
    //udphdr.source = htons(SRC_PORT)
    //超重要觀念,struct udphdr中的source為u16即unsigned short和struct sockaddr_in中的欄位sin_port的型別unsigned short 為同一個
    udphdr.dest = dest_addr->sin_port;;
    //udphdr.dest = htons(DEST_PORT);
    udphdr.len = htons(sizeof(struct row_udphdr) + datalength);
    udphdr.check = udp4_checksum(iphdr , udphdr , (uint8_t*)data , datalength);
    char packet[IP_MAXPACKET];
    bzero(packet,IP_MAXPACKET);
    memcpy(packet,&iphdr,sizeof(struct ip));
    memcpy(packet+sizeof(struct ip),&udphdr,sizeof(struct row_udphdr));
    memcpy(packet+sizeof(struct ip)+sizeof(struct row_udphdr),data,datalength);
    struct sockaddr_in sinaddr;
    bzero((char *)&sinaddr,sizeof(struct sockaddr_in));
    sinaddr.sin_family = AF_INET;
    sinaddr.sin_addr.s_addr = iphdr.ip_dst.s_addr;
    size_t totallength = sizeof(struct ip)+sizeof(struct row_udphdr)+datalength;
    int s = sendto( sd, packet, totallength, flags, (struct sockaddr *)&sinaddr, sizeof(struct sockaddr) );
    return s;
}
