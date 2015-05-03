#include <stdio.h>
#include "p2p.h"
#define SERV_PORT 12345
#define MAXLENGTH 1024
#define MAXUSERS 10

struct user clients[MAXUSERS ];

void Error(char *s)
{
    printf("%s\n",s);
    exit(-1);
}

struct user * finduserbyid(char *id)
{
    for(int i=0 ; i<MAXUSERS ; i++)
       if(strcmp(clients[i].id,id)==0)
            return &clients[i];
    printf("finduserbyid : %s not found in database\n",id);
    return NULL;
}

struct user  * finduserbyrequestid(char *id)
{
    for(int i=0 ; i<MAXUSERS ; i++)
       if(strcmp(clients[i].requestid,id)==0)
            return &clients[i];
    printf("finduserbyrequestid : %s not found in database\n",id);
    return NULL;
}

void addaccount()
{
    sprintf(clients[0].id,"%s","sam33");
    clients[0].state = UserState_Offline;
    sprintf(clients[1].id,"%s","jacket484");
    clients[1].state = UserState_Offline;
}

void initp2p()
{
    for(int i=0 ; i<MAXUSERS ; i++)
        bzero(&clients[i],sizeof(struct user));
    addaccount();
}

bool response_clientrequest(struct response *userresponse , struct request reqmsg , struct sockaddr_in client_addr)
{
    //bzero(userresponse,sizeof(struct response));
    struct user * requester = finduserbyid(reqmsg.myid);
    if( requester == NULL)
    {
        return false;
    }
    else
    {
        printf("The user %s exist , allow access\n",requester->id);
        struct user * target ;
        switch (reqmsg.mystate)
        {
            case UserState_Waiting:
                printf("%s waiting\n",requester->id);
                printf("finduserbyrequestid(%s);\n",requester->id);
                target = finduserbyrequestid(requester->id);
                if(target == NULL)
                {
                     //目前沒影任何P2PClient端想要對requester連線
                    //A(1)
                    printf("SYSTEM : A(1)\n");
                    requester->state = UserState_Waiting;
                    requester->addr = client_addr;
                    userresponse->targetstate = UserState_NotExist;
                }
                else
                {
                    printf("result:%s\n",target->id);
                    switch(target->state)
                    {
                        //找到了想要與requester連線的主動端(且連線主動端還沒有requester的資訊)
                        case UserState_Request:
                        //B(2)
                        printf("SYSTEM : B(2)\n");
                        requester->state = UserState_Running;
                        memcpy((char*)&requester->addr,(char*)&client_addr,sizeof(sockaddr_in));
                        sprintf(userresponse->targetid,target->id);
                        memcpy((char*)&userresponse->targetaddr,(char*)&target->addr,sizeof(sockaddr_in));
                        userresponse->targetstate =  UserState_Running;
                        break;
                        //找到了想要與requester連線的主動端(且連線主動端已經有requester的資訊)  (被動端--主動端)
                        case UserState_Running:
                        //A(3)
                        printf("SYSTEM : A(3)\n");
                        requester->state = UserState_Running;
                        sprintf(userresponse->targetid,target->id);
                        memcpy((char*)&userresponse->targetaddr,(char*)&target->addr,sizeof(sockaddr_in));
                        userresponse->targetstate =  UserState_Running;
                        break;
                    }
                    break;
                }
                break;

            case UserState_Request:
                printf("%s searching the user : %s\n",requester->id , reqmsg.targetid);
                target = finduserbyid(reqmsg.targetid);
                if(target == NULL)
                {
                    userresponse->targetstate = UserState_NotExist;
                }
                else
                {
                    switch(target->state)
                    {
                        case UserState_Offline:
                        //B(1)
                        printf("SYSTEM : B(1)\n");
                        printf("%s save request(%s) to database\n",reqmsg.myid,reqmsg.targetid);
                        requester->state = UserState_Request;
                        requester->addr = client_addr;
                        sprintf(requester->requestid,"%s",reqmsg.targetid);
                        userresponse->targetstate = UserState_Offline;
                        break;
                        //找到了requester想連線的被動端(且被動端還沒有requester的資訊)
                        case UserState_Waiting:
                        printf("SYSTEM : A(2)\n");
                        //A(2)
                        requester->state = UserState_Running;
                        requester->addr = client_addr;
                        sprintf(requester->requestid,"%s",reqmsg.targetid);
                        sprintf(userresponse->targetid,target->id);
                        memcpy((char*)&userresponse->targetaddr,(char*)&target->addr,sizeof(sockaddr_in));
                        userresponse->targetstate =  UserState_Running;
                        break;
                        //找到了requester想連線的被動端(且被動端已經有requester的資訊)
                        case UserState_Running:
                        printf("SYSTEM : B(3)\n");
                        //B(3)
                        requester->state = UserState_Running;
                        sprintf(userresponse->targetid,target->id);
                        memcpy((char*)&userresponse->targetaddr,(char*)&target->addr,sizeof(sockaddr_in));
                        userresponse->targetstate =  UserState_Running;
                        char *address = inet_ntoa( userresponse->targetaddr.sin_addr) ;
                        int port = ntohs( userresponse->targetaddr.sin_port);
                        break;
                    }
                }
                break;

        }
        return true;
    }
    return false;
}

int main(void)
{
    char buf[MAXLENGTH];
    struct sockaddr_in myaddr; /* address of this service */
    struct sockaddr_in client_addr; /* address of client    */
    int socket_fd =  socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0)
        Error("socket failed");
    bzero ((char *)&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(SERV_PORT);
    int b = bind(socket_fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    if(b<0)
        Error("bind failed");
    printf("Server is ready to receive !!\n");
    // socklen_t, which is an unsigned opaque integral type of length of at least 32 bits.
    // socklen_t   ---    unsigned int
    socklen_t  sizet = sizeof(client_addr);
    initp2p();
    while (1)
    {
        // ssize_t : singn ssize_t
        ssize_t  r  =  recvfrom(socket_fd, buf, MAXLENGTH, 0, (struct sockaddr*)&client_addr,  &sizet);
        if(r<0)
        {
            printf ("could not read datagram!!");
            continue;
        }
        if(r==sizeof(struct request))
        {
            printf("Received request package form %s : %d\n",inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
            struct request req;
            bzero((char*)&req,sizeof(struct request));
            memcpy((char*)&req,buf,sizeof(struct request));
            struct response rsp;
            bzero((char*)&rsp,sizeof(struct response ));
            if(response_clientrequest(&rsp,req,client_addr))
            {
                int s = sendto(socket_fd,&rsp,sizeof(struct response),0,(struct sockaddr*)&client_addr,sizet);
                if(s<0)
                {
                    printf ("could not send datagram!!");
                    continue;
                }
            }
            else
            {
                bzero(buf,MAXLENGTH);
                sprintf(buf,"%s","permission deny\n");
                int s = sendto(socket_fd,buf,strlen(buf),0,(struct sockaddr*)&client_addr,sizet);
                if(s<0)
                {
                    printf ("could not send datagram!!");
                    continue;
                }
            }
        }
        else
        {
            printf("Received data form %s : %d\n",inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
            printf("%s\n", &buf);
            int s = sendto(socket_fd, &buf, r, 0, (struct sockaddr*)&client_addr, sizet);
            if(s<0)
            {
                printf ("could not send datagram!!");
                continue;
            }
        }
        printf("------------------------------------------\n");
    }
}
