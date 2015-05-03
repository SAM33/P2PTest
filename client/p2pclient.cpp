#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "p2p.h"
#include "rowudpsocket.h"
#define MAXLENGTH 1024




void Error(char *s)
{
    printf("%s\n",s);
    exit(-1);
}

int active_p2pmain( struct sockaddr_in serveraddr , struct sockaddr_in targetaddr)
{
    printf("active_p2pmain\n");
    int socket_fd = rowsocket();
    if(socket_fd < 0)
        Error("socket failed");
    socklen_t  sizet = sizeof( targetaddr);
    char buf[MAXLENGTH];
    while(true)
    {
        bzero(buf,MAXLENGTH);
        sprintf(buf,"Hello P2P partner\n");
        printf("p2p target [%s:%d]\n",  inet_ntoa(targetaddr.sin_addr), ntohs(targetaddr.sin_port));
        ssize_t s = rowudpsendto(socket_fd, buf , strlen(buf), 0, &serveraddr,sizeof(serveraddr),&targetaddr,sizeof(targetaddr));
        if(s<0)
        {
            printf ("could not send datagram!!");
            continue;
        }
        else
        {
            printf("sendto p2p target : %s\n",buf);
        }
        sleep(5);
    }
}

int passive_p2pmain(int sock_fd , struct sockaddr_in serveraddr , struct sockaddr_in targetaddr)
{
    socklen_t  sizet = sizeof( targetaddr);
    char buf[MAXLENGTH];
    while(true)
    {
        bzero(buf,MAXLENGTH);
        printf("p2p target [%s:%d]\n",  inet_ntoa(targetaddr.sin_addr), ntohs(targetaddr.sin_port));
        ssize_t  r  =  recvfrom(sock_fd, buf, MAXLENGTH, 0, (struct sockaddr*)&targetaddr,  &sizet);
        if(r<0)
        {
            printf ("could not read datagram!!");
            continue;
        }
        else
        {
            printf("recvfrom p2p target : %s\n",buf);
        }
        sleep(5);
    }
}

int main(int argc , char * args[])
{
    char SelfUserID[20];
    char TargetUserID[20];
    bzero(SelfUserID,20);
    bzero(TargetUserID,20);
    
    char Server_Address[20];
    bzero(Server_Address,20);
    int Server_Port=0;
    bool ActiveMode = false;
    if(argc>=1+2+1)
    {
        strcpy(Server_Address,args[1]);
        Server_Port = atoi(args[2]);
        printf("Server Address : %s\n",Server_Address);
        printf("Server Port : %d\n",Server_Port);
        if(strcmp(args[3],"A")==0)
        {
            ActiveMode = true;
            printf("ActiveMode\n");
            
        }
        else if(strcmp(args[3],"P")==0)
        {
            printf("PassiveMode\n");
            ActiveMode = false;
        }
        else
        {
            printf("Useage : ./main_release <P2PServer Address> <P2PServer Port> <Attribute>\n");
            printf("Attribute : A/P A=active_p2client P=passive_p2pclient\n");
            return 0;
        }
        
    }
    else
    {
        printf("Useage : ./main_release <P2PServer Address> <P2PServer Port> <Attribute>\n");
        printf("Attribute : A/P A=active_p2client P=passive_p2pclient\n");
        return 0;
    }
    
    
    char buf[MAXLENGTH];
    struct sockaddr_in server_addr;
    int socket_fd =  socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0)
        Error("socket failed");
    else
        printf("socket ok\n");
    // inet_addr和inet_network函數都是用於將字符串形式轉換為整數形式用的，兩者區別很小，inet_addr返回的整數形式是網絡字節序，而inet_network返回的整數形式是主機字節序。
    // inet_aton函數和上面這倆小子的區別就是在於他認為255.255.255.255是有效的，他不會冤枉這個看似特殊的IP地址，inet_aton函數返回的是網絡字節序的IP地址。
    bzero ((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(Server_Address);
    server_addr.sin_port = htons(Server_Port);
    // socklen_t, which is an unsigned opaque integral type of length of at least 32 bits.
    // socklen_t   ---    unsigned int
    socklen_t  sizet = sizeof(server_addr);
    while (1)
    {
        struct request req;
        bzero((char*)&req,sizeof(struct request));
        
        if(ActiveMode)
        {
            strcpy(SelfUserID,"sam33");
            strcpy(TargetUserID,"jacket484");
            sprintf(req.myid,"%s",SelfUserID);
            req.mystate = UserState_Request;
            sprintf(req.targetid,"%s",TargetUserID);
        }
        else
        {
            strcpy(TargetUserID,"sam33");
            strcpy(SelfUserID,"jacket484");
            sprintf(req.myid,"%s",SelfUserID);
            req.mystate = UserState_Waiting;
        }
        ssize_t s = sendto(socket_fd, &req ,sizeof(struct request), 0, (struct sockaddr*)&server_addr, sizet);
        if(s<0)
        {
            printf ("could not send datagram!!");
            continue;
        }
        // ssize_t : singn ssize_t
        bzero(buf,MAXLENGTH);
        ssize_t  r  =  recvfrom(socket_fd, buf, MAXLENGTH, 0, (struct sockaddr*)&server_addr,  &sizet);
        if(r<0)
        {
            printf ("could not read datagram!!");
            continue;
        }
        if(r==sizeof(struct response))
        {
            struct response rsp;
            bzero((char *)&rsp,sizeof(struct response));
            memcpy((char*)&rsp,buf,sizeof(struct response));
            switch(rsp.targetstate)
            {
                case UserState_Offline:
                printf("UserState_Offline\n");
                break;
                case UserState_NotExist:
                printf("UserState_NotExist\n");
                break;
                case UserState_Running:
                char *address = inet_ntoa(rsp.targetaddr.sin_addr) ;
                int port = ntohs(rsp.targetaddr.sin_port);
                printf("prepare ok\n");
                printf("The user %s : \n" , rsp.targetid);
                printf("Address : %s \n", address);
                printf("Port        : %d \n", port);
                if(ActiveMode)
                    active_p2pmain(server_addr , rsp.targetaddr);
                else
                passive_p2pmain(socket_fd , server_addr , rsp.targetaddr);
                break;
            }
        }
        else
        {
            printf("Received data form server : \n");
            printf("%s\n", buf);
        }
        sleep(5);
    }
}
