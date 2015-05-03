#ifndef _P2P_H_
#define _P2P_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#define UserState_NotExist   -1
#define UserState_Offline     0
#define UserState_Waiting     1
#define UserState_Request     2
#define UserState_Running     3

struct user
{
    char state;
    char id[10];
    char requestid[10];
    struct sockaddr_in addr;
};

struct request
{
    char mystate;
    char myid[10];
    char targetid[10];
};

struct response
{
    char targetstate;
    char targetid[10];
    struct sockaddr_in targetaddr;
};


#endif
