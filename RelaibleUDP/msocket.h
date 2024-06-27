#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>



#define MAX_SEQ 15
#define MAX_SOCKET 25
#define SOCK_MTP 510
#define M_SOCKET_OFFEST 17
#define ENOBUFS 105
#define ENOTBOUND 106
// #define ENOMSG 107
#define T 5
#define p 0.5

#define min(a,b) ((a) < (b) ? (a) : (b))


// int GLOBERR;

typedef struct{
    int wndsize;
    int sequence[MAX_SEQ];
    int currframe;
}swnd;


typedef struct{
    int wndsize;
    int sequence[MAX_SEQ];
}rwnd;

typedef struct{
    int frame;
    int ack;
    int isbusy;
    char data[1024];
}buffer;

typedef struct{
    int size;
    int next;
    int nextframed;
    struct timeval lastsent;
    buffer data[5];
}sendmsgbuffer;

typedef struct{
    int insertloc;
    int fetchloc;
    int insertsize;
    int togetframe;
    int readframe;
    int fetchsize;
    buffer data[5];
}recvmsgbuffer;

typedef struct{
    int isfree;
    int isocketcreated;
    pid_t pid;
    int sockfd;
    char destIP[16];
    int destport;
    sendmsgbuffer sendbuffer;
    recvmsgbuffer receivebuffer;
    swnd sendwnd;
    rwnd recvwnd;
}mtp_socket;


// Functions


int m_socket(int domain, int type, int protocol);
int m_bind(int msock, const char *srcIP, int srcPort, const char *destIP, int destPort);
int m_sendto(int sockfd, const void *msg, int len, unsigned int flags,const struct sockaddr *to, socklen_t tolen);
int m_recvfrom(int sockfd, void *buf, int len, unsigned int flags,struct sockaddr *from, socklen_t fromlen);
void m_close(int sockfd);


void initializebuffers(mtp_socket *sock);
// Initializing the shared memory



// Remove these golbal variables

// int SMID;

// mtp_socket *SM;


typedef struct{
    int sockfd;
    char IP[16];
    int port;
    int errn;
} SOCK_INFO;


// Threads

// void *Rthreadfunc(void *arg);
// void *Sthreadfunc(void *arg);
// void * garbagecollector(void *arg);

// pthread_t Rthread, Sthread, Gthread;


// int sem1, sem2; // may be change in future


int dropMessage();