#include "msocket.h"



int m_socket(int domain, int type, int protocol){
    if(type != SOCK_MTP){
        printf("Error: Invalid socket type\n");
        exit(EXIT_FAILURE);
    }

    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);

    int sem1key = ftok(".", 97);
    int sem1 = semget(sem1key, 1, shmget_flags);
    if(sem1 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct sembuf signal_op = {0, 1, 0};
    struct sembuf wait_op = {0, -1, 0};

    int sem2key = ftok(".", 96);
    int sem2 = semget(sem2key, 1, shmget_flags);
    if(sem2 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sem3key = ftok(".", 95);
    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    int smsockinfokey = ftok(".", 99);
    int sockinfoID = shmget(smsockinfokey, sizeof(SOCK_INFO), shmget_flags);
    SOCK_INFO *sockinfo = (SOCK_INFO *)shmat(sockinfoID, 0, 0);


    int m_sockid = -1;
    // Check in shared memory if there is a socket available
    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(int i=0; i<MAX_SOCKET; i++){
        if(SM[i].isfree == 1){
            m_sockid = i;
            SM[i].isfree = 0;
            // SM[i].isocketcreated = 0;
            SM[i].pid = getpid();
            break;
        }
    }

    if(m_sockid == -1){
        //GLOBERR = ENOBUFS;
        return -1;
    }

    sockinfo->sockfd = 0;
    sockinfo->port = 0;
    sockinfo->errn = 0;
    sockinfo->IP[0] = '\0';

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // printf("Current Empty Socket: %d\n", m_sockid);

    if(semop(sem1, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Waiting for Socket Creation\n");
    if(semop(sem2, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("Socket Done returned !!!\n");



    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(sockinfo->sockfd == -1){
        errno = sockinfo->errn;
        sockinfo->sockfd = 0;
        sockinfo->port = 0;
        sockinfo->errn = 0;
        sockinfo->IP[0] = '\0';
        return -1;
    }

    int sockfd = sockinfo->sockfd;

    SM[m_sockid].sockfd = sockfd;
    SM[m_sockid].pid = getpid();

    initializebuffers(&SM[m_sockid]);

    sockinfo->sockfd = 0;
    sockinfo->port = 0;
    sockinfo->errn = 0;
    sockinfo->IP[0] = '\0';

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return m_sockid + M_SOCKET_OFFEST;

}

void initializebuffers(mtp_socket *sock){
    sock->sendbuffer.size = 0;
    sock->sendbuffer.next = 0;
    sock->sendbuffer.nextframed = 0;
    for(int i=0; i<5; i++){
        sock->sendbuffer.data[i].isbusy = 0;
        sock->sendbuffer.data[i].frame = 0;
        sock->sendbuffer.data[i].ack = 0;
    }

    sock->receivebuffer.insertloc = 0;
    sock->receivebuffer.fetchloc = 0;
    sock->receivebuffer.insertsize = 5;
    sock->receivebuffer.fetchsize = 0;
    sock->receivebuffer.togetframe = 1;
    sock->receivebuffer.readframe = 1;

    for(int i = 0;i<5;i++){
        bzero(sock->sendbuffer.data[i].data, 1024);
        bzero(sock->receivebuffer.data[i].data, 1024);
    }

    sock->sendwnd.wndsize = 1;
    sock->recvwnd.wndsize = 0;
    sock->sendwnd.currframe = 1;

    for(int i=0; i<MAX_SEQ; i++){
        sock->sendwnd.sequence[i] = -1;
        sock->recvwnd.sequence[i] = -1;
    }


    return;
}

int m_bind(int msock, const char *srcIP, int srcPort, const char *destIP, int destPort) {
    
    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);
        
    int sem1key = ftok(".", 97);
    int sem1 = semget(sem1key, 1, shmget_flags);
    if(sem1 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct sembuf signal_op = {0, 1, 0};
    struct sembuf wait_op = {0, -1, 0};

    int sem2key = ftok(".", 96);
    int sem2 = semget(sem2key, 1, shmget_flags);
    if(sem2 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sem3key = ftok(".", 95);
    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    int smsockinfokey = ftok(".", 99);
    int sockinfoID = shmget(smsockinfokey, sizeof(SOCK_INFO), shmget_flags);
    SOCK_INFO *sockinfo = (SOCK_INFO *)shmat(sockinfoID, 0, 0);

    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sockfd = SM[msock - M_SOCKET_OFFEST].sockfd;
    sockinfo->sockfd = sockfd;
    sockinfo->port = srcPort;
    sockinfo->errn = 0;
    strcpy(sockinfo->IP, srcIP);

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Signalled for Socket Binding\n");
    if(semop(sem1, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Waiting for Socket Binding\n");

    if(semop(sem2, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("temporary Socket Binding Done\n");

    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        printf("Error in Binding \n");
        exit(EXIT_FAILURE);
    }

    if(sockinfo->sockfd == -1){
        errno = sockinfo->errn;
        sockinfo->sockfd = 0;
        sockinfo->port = 0;
        sockinfo->errn = 0;
        sockinfo->IP[0] = '\0';

        if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
        return -1;
    }

    printf("Socket Bounded Hurrah !!!\n");

    sockinfo->sockfd = 0;
    sockinfo->port = 0;
    sockinfo->errn = 0;
    sockinfo->IP[0] = '\0';



    int m_sockid = msock - M_SOCKET_OFFEST;

    bzero(&SM[m_sockid].destIP, sizeof(SM[m_sockid].destIP));

    strcpy(SM[m_sockid].destIP, destIP);


    SM[m_sockid].destport = destPort;
    SM[m_sockid].isocketcreated = 1;

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;

}

int m_sendto(int sockfd, const void *msg, int len, unsigned int flags,const struct sockaddr *to, socklen_t tolen){
    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);
    int m_sockid = sockfd - M_SOCKET_OFFEST;

    int sem3key = ftok(".", 95);
    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sembuf wait_op = {0, -1, 0};
    struct sembuf signal_op = {0, 1, 0};

    // Matching destination IP and port

    int res = -1;

    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(!(strcmp(SM[m_sockid].destIP, inet_ntoa(((struct sockaddr_in *)to)->sin_addr)) == 0 || SM[m_sockid].destport == ntohs(((struct sockaddr_in *)to)->sin_port))){
        errno = ENOTBOUND;
        res = -1;
    }
    else{
        if(SM[m_sockid].sendbuffer.size == 5){
            //GLOBERR = ENOBUFS;
            // return -1;
            res = -1;
        }
        else{
            int next = SM[m_sockid].sendbuffer.next;
            // TO check if free
            if(SM[m_sockid].sendbuffer.data[next].isbusy == 1){
                //GLOBERR = ENOBUFS;
                printf("Some issue with buffer is free\n");
                // return -1;
                res = -1;
            }
            else{
                SM[m_sockid].sendbuffer.data[next].isbusy = 1;
                SM[m_sockid].sendbuffer.data[next].ack = 0;
                SM[m_sockid].sendbuffer.data[next].frame = 0;
                // Will set the frame number later
                bzero(SM[m_sockid].sendbuffer.data[next].data, sizeof(SM[m_sockid].sendbuffer.data[next].data));
                memcpy(SM[m_sockid].sendbuffer.data[next].data, msg, len);
                SM[m_sockid].sendbuffer.size++;
                // printf("Size of send buffer %d\n", SM[m_sockid].sendbuffer.size);
                printf("Sent messageat index%d\n",next);
                SM[m_sockid].sendbuffer.next = (next + 1)%5;
                

                // return len;
                res = len;
            }
        }
    }


    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    return res;

}


// Seamphore yet to be added

int m_recvfrom(int sockfd, void *buf, int len, unsigned int flags,struct sockaddr *from, socklen_t fromlen){
    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);


    int sem3key = ftok(".", 95);

    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct sembuf wait_op = {0, -1, 0};
    struct sembuf signal_op = {0, 1, 0};


    int m_sockid = sockfd - M_SOCKET_OFFEST;

    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int res = -1;

    if(SM[m_sockid].receivebuffer.fetchsize == 0){
        //GLOBERR = ENOMSG;
        // printf("No message to receive\n");

        if(semop(sem3, &signal_op, 1)==-1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        return -1;
    }
    else{
        
        int framewewant = SM[m_sockid].receivebuffer.readframe;
        printf("Frame we want %d\n", framewewant);
        // int cuuridx = 0;

        for(int i = 0 ;i<5;i++){
            if(SM[m_sockid].receivebuffer.data[i].frame == framewewant && SM[m_sockid].receivebuffer.data[i].isbusy == 1){
                memcpy(buf, SM[m_sockid].receivebuffer.data[i].data, sizeof(SM[m_sockid].receivebuffer.data[i].data));
                SM[m_sockid].receivebuffer.fetchsize--;
                SM[m_sockid].receivebuffer.readframe = (SM[m_sockid].receivebuffer.readframe + 1)%MAX_SEQ;
                if(SM[m_sockid].receivebuffer.readframe == 0){
                    SM[m_sockid].receivebuffer.readframe = 1;
                }
                SM[m_sockid].receivebuffer.insertsize++;

                if(SM[m_sockid].receivebuffer.insertsize == 1){
                    SM[m_sockid].receivebuffer.insertloc = i;
                }
                
                SM[m_sockid].receivebuffer.data[i].isbusy = 0;
                SM[m_sockid].receivebuffer.data[i].frame = 0;
                SM[m_sockid].receivebuffer.data[i].ack = 0;
                
                res = sizeof(SM[m_sockid].receivebuffer.data[i].data);
                break;
            }
        }

    }

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return res;
}

void m_close(int sockfd){
    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);

    int sem3key = ftok(".", 95);
    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sembuf wait_op = {0, -1, 0};
    struct sembuf signal_op = {0, 1, 0};
    int flag = 1;
    int m_sockid = sockfd - M_SOCKET_OFFEST;
    while(1 & flag){
        usleep(50);
    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Checking if all the messages are sent and received
    if(SM[sockfd - M_SOCKET_OFFEST].sendbuffer.size == 0 && SM[sockfd - M_SOCKET_OFFEST].receivebuffer.fetchsize == 0){
        flag = 0;
    }

    
    SM[m_sockid].isfree = 1;

    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    }


    if(semop(sem3, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(SM[m_sockid].sockfd);
    printf("Socket Closed\n");


    if(semop(sem3, &signal_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return;
}

