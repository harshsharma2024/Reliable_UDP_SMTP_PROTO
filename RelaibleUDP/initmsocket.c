#include "msocket.h"

void *Rthreadfunc(void *arg);
void *Sthreadfunc(void *arg);
void *garbagecollector(void *arg);
void initializebuffers(mtp_socket *sock);
void init();
char *datasendack(int size, int frame);
int is_data_message(const char *msg);
int is_ack_message(const char *msg);
int timedifference(struct timeval *start, struct timeval *end);
char *header_ack(int frame, const char *data);
int noofackleft(sendmsgbuffer sendbuffer);
int dropMessage();

int nospace = 0;

void init(){

    printf("Init\n");

    int shmget_flags = IPC_CREAT | 0666;
    // int shmget_flags = (IPC_CREAT | IPC_EXCL | 0666);

    int smkey = ftok(".", 98);
    printf("%d\n", smkey);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);
    printf("Init 2\n");
    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        // printf("SOmethign went wrong\n");
        exit(EXIT_FAILURE);
    }
    // printf("Init 3\n");
    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);

    for(int i = 0; i < MAX_SOCKET; i++){
        SM[i].isfree = 1;
        SM[i].isocketcreated = 0;
        initializebuffers(&SM[i]);
    }


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

    if(semctl(sem1, 0, SETVAL, 0)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(semctl(sem2, 0, SETVAL, 0)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(semctl(sem3, 0, SETVAL, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(semop(sem3, &wait_op, 1) == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < MAX_SOCKET; i++){
        SM[i].isfree = 1;
        SM[i].isocketcreated = 0;
    }

    int smsockinfokey = ftok(".", 99);
    int sockinfoID = shmget(smsockinfokey, sizeof(SOCK_INFO), shmget_flags);
    SOCK_INFO *sockinfo = (SOCK_INFO *)shmat(sockinfoID, 0, 0);




    if(sockinfoID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    sockinfo->sockfd = 0;
    sockinfo->port = 0;
    sockinfo->errn = 0;
    sockinfo->IP[0] = '\0';

    // Initialize R and S threads
    pthread_t Rthread, Sthread, Gthread;

    pthread_create(&Rthread, NULL, Rthreadfunc, NULL);
    pthread_create(&Sthread, NULL, Sthreadfunc, NULL);
    pthread_create(&Gthread, NULL, garbagecollector, NULL);

    if(semop(sem3, &signal_op, 1) == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while(1){

    printf("Waiting ...\n");
    if(semop(sem1, &wait_op, 1)==-1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Done Waiting \n");

    printf("Semaphore Acquired for Socket Creation\n");

    if(sockinfo->sockfd ==0 && sockinfo->port == 0 && sockinfo->errn == 0){
        printf("Ready to create Socket\n");

        if(semop(sem3, &wait_op, 1) == -1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(sockfd == -1){
            printf("Error: %s\n", strerror(errno));
            sockinfo->errn = errno;
            sockinfo->sockfd = -1;
        }
        else{
            printf("Socket Created socketidudp : %d\n", sockfd);
            sockinfo->sockfd = sockfd;
        }

        if(semop(sem3, &signal_op, 1)==-1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if(semop(sem2, &signal_op, 1)==-1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

    }
    else{

        if(semop(sem3, &wait_op, 1) == -1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Ready to bind Socket\n");
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(sockinfo->port);
        addr.sin_addr.s_addr = inet_addr(sockinfo->IP);
        printf("%s , %d , %d \n", sockinfo->IP, sockinfo->port, sockinfo->sockfd);
        int ret = bind(sockinfo->sockfd, (struct sockaddr *)&addr, sizeof(addr));
        if(ret == -1){
            sockinfo->sockfd = -1;
            sockinfo->errn = errno;
        }
        else{
            printf("Socket Bounded\n");
        }

        if(semop(sem3, &signal_op, 1)==-1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        

        if(semop(sem2, &signal_op, 1)==-1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }


    }


    return;
}

int is_data_message(const char *msg) {
    if(strlen(msg) <= 4) {
        return 0;
    }

    return 1;
}

int is_ack_message(const char *msg) {
    if(strlen(msg) == 4) {
        if(msg[0] >= '0' && msg[0] <= '9' && msg[1] >= '0' && msg[1] <= '9' && msg[2] >= '0' && msg[2] <= '9' && msg[3] >= '0' && msg[3] <= '9') {
            return 1;
        }
        
    }

    return 0;
}


char *datasendack(int size, int frame){
    int required_size = snprintf(NULL, 0, "%02d%02d", size, frame) + 1;

    // Allocate memory for the buffer
    char *ack = malloc(required_size);
    if (ack == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    // Format the integers into the buffer
    snprintf(ack, required_size, "%02d%02d", size, frame);

    return ack;

}


void *Rthreadfunc(void *arg){
    
    int shmget_flags = IPC_CREAT | 0666;
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);
    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);


    int sem3key = ftok(".", 95);

    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    struct sembuf wait_op = {0, -1, 0};
    struct sembuf signal_op = {0, 1, 0};

    int maxfd, i, activity;
    fd_set readfds;
    struct sockaddr_in cliaddr;
    unsigned int addrlen = sizeof(cliaddr);

    FD_ZERO(&readfds);
    maxfd = 0;

    for (i = 0; i < MAX_SOCKET; i++)
    {
        if (SM[i].isfree == 0 && SM[i].isocketcreated == 1)
        {
            FD_SET(SM[i].sockfd, &readfds);
            if (SM[i].sockfd > maxfd)
                maxfd = SM[i].sockfd;
        }
    }

    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = T;

    while (1)
    {

        // if(semop(sem3, &wait_op, 1) == -1){
        //     printf("Error: %s\n", strerror(errno));
        //     exit(EXIT_FAILURE);
        // }

        char buf[1029];
        bzero(buf, 1029);

        activity = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
        if ((activity < 0) && (errno != EINTR))
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }
        else if (activity == 0)
        {
            // printf("Timeout occurred\n");

            FD_ZERO(&readfds);
            maxfd = 0;

            for (i = 0; i < MAX_SOCKET; i++)
            {
                if (SM[i].isfree == 0 && SM[i].isocketcreated == 1)
                {
                    FD_SET(SM[i].sockfd, &readfds);
                    if (SM[i].sockfd > maxfd)
                        maxfd = SM[i].sockfd;
                }
            }
        }
        else
        {
            for (i = 0; i < MAX_SOCKET; i++)
            {
                if (FD_ISSET(SM[i].sockfd, &readfds))
                {

                    bzero(buf, 1029);
                    int bytes_received = recvfrom(SM[i].sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &addrlen);
                    if (bytes_received < 0)
                    {
                        perror("recvfrom failed");
                        continue;
                    }

                    // printf("&&&&&&&Message received : %s , size : %zu\n", buf, strlen(buf));

                    if(dropMessage()){
                        printf("*****Message Dropped*******\n");
                        continue;
                    }

                    if (is_data_message(buf))
                    {

                        if(SM[i].receivebuffer.insertsize == 0){
                            // printf("Buffer is full\n");
                            nospace = 1;
                            continue;
                        }

                        // printf("Data message received\n");

                        char *data = malloc(strlen(buf) - 4 + 1);
                        if (data == NULL)
                        {
                            fprintf(stderr, "Memory allocation error\n");
                            continue;
                        }

                        strcpy(data, buf + 4);
                        char*ack = malloc(4 + 1);
                        if (ack == NULL)
                        {
                            fprintf(stderr, "Memory allocation error\n");
                            continue;
                        }

                        strncpy(ack, buf , 4);
                        ack[4] = '\0';

                        int frame = atoi(ack);
                        printf("Frame %d received\n", frame);
                        // Checking if the frame is in the receive window

                        int ackalreadysent = 1;
                        int val = SM[i].receivebuffer.togetframe;
                        for(int j = 0; j < SM[i].receivebuffer.insertsize; j++){
                            val = (val + j) % MAX_SEQ;
                            if(val == 0){
                                val = 1;
                            }

                            if(val == frame){
                                ackalreadysent = 0;
                                break;
                            }
                        }

                        if(ackalreadysent == 1){
                            printf("Frame %d already received\n", frame);
                            char* ack = malloc(4 + 1);
                            if (ack == NULL)
                            {
                                fprintf(stderr, "Memory allocation error\n");
                                continue;
                            }
                            
                            ack = datasendack(SM[i].receivebuffer.insertsize , frame);
                            printf("Ack sent for frame %d : %s , size: %zu\n", frame , ack, strlen(ack));
                            int t = sendto(SM[i].sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, addrlen);
                            if(t == -1){
                                perror("sendto failed");
                                continue;
                            }

                            continue;
                        }

                        // int nextpostoinsert = SM[i].receivebuffer.insertloc;
                        // SM[i].sendbuffer.data[nextpostoinsert].isbusy = 1;
                        // SM[i].sendbuffer.data[nextpostoinsert].frame = frame;
                        // SM[i].sendbuffer.data[nextpostoinsert].ack = 1;
                        // strcpy(SM[i].sendbuffer.data[nextpostoinsert].data, data);

                        // SM[i].receivebuffer.insertsize--;

                        // Iterating over buffer to see the first unbusy frame for insertloc
                        int wheretoinsert = 0;
                        for(int j = 0; j < 5; j++){
                            if(SM[i].receivebuffer.data[j].isbusy == 0){
                                SM[i].receivebuffer.insertloc = j;
                                wheretoinsert = j;
                                break;
                            }
                        }

                        // SM[i].receivebuffer.fetchsize++;
                        // SM[i].receivebuffer.insertsize--;
                        if(frame == SM[i].receivebuffer.togetframe){
                            SM[i].receivebuffer.togetframe = (frame + 1) % MAX_SEQ;
                            if(SM[i].receivebuffer.togetframe == 0){
                                SM[i].receivebuffer.togetframe = 1;
                            }

                        SM[i].receivebuffer.fetchsize++;
                        SM[i].receivebuffer.insertsize--;
                        SM[i].receivebuffer.data[wheretoinsert].isbusy = 1;
                        SM[i].receivebuffer.data[wheretoinsert].frame = frame;
                        // SM[i].receivebuffer.data[wheretoinsert].ack = 1;
                        strcpy(SM[i].receivebuffer.data[wheretoinsert].data, data);
                        // printf("Data : %s\n" , data);
                        printf("Message received and inserted at %d and frame no = %d\n", wheretoinsert, frame);

                        // printf("Data message received\n");

                        // Sending ACK for the frame
                        ack = malloc(4 + 1);
                        if (ack == NULL)
                        {
                            fprintf(stderr, "Memory allocation error\n");
                            continue;
                        }

                        ack = datasendack(SM[i].receivebuffer.insertsize , frame);

                        int t = sendto(SM[i].sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, addrlen);

                        if(t == -1){
                            perror("sendto failed");
                            continue;
                        }

                        printf("ACK sent for frame %d : %s, size : %zu\n", frame, ack , strlen(ack));

                        }
                        // SM[i].receivebuffer.togetframe = (frame + 1) % MAX_SEQ;
                        // if(SM[i].receivebuffer.togetframe == 0){
                        //     SM[i].receivebuffer.togetframe = 1;
                        // }

                        // SM[i].receivebuffer.data[wheretoinsert].isbusy = 1;
                        // SM[i].receivebuffer.data[wheretoinsert].frame = frame;
                        // // SM[i].receivebuffer.data[wheretoinsert].ack = 1;
                        // strcpy(SM[i].receivebuffer.data[wheretoinsert].data, data);
                        // printf("Data : %s\n" , data);
                        // printf("Message received and inserted at %d and frame no = %d\n", wheretoinsert, frame);

                        // printf("Data message received\n");

                        // // Sending ACK for the frame
                        // ack = malloc(4 + 1);
                        // if (ack == NULL)
                        // {
                        //     fprintf(stderr, "Memory allocation error\n");
                        //     continue;
                        // }

                        // ack = datasendack(SM[i].receivebuffer.insertsize , frame);

                        // int t = sendto(SM[i].sockfd, ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, addrlen);

                        // if(t == -1){
                        //     perror("sendto failed");
                        //     continue;
                        // }

                        // printf("ACK sent for frame %d : %s, size : %zu\n", frame, ack , strlen(ack));
                        

                    }
                    else if (is_ack_message(buf))
                    {
                        printf("#########ACK message received %s\n", buf);

                        char*ack = malloc(4 + 1);
                        if (ack == NULL)
                        {
                            fprintf(stderr, "Memory allocation error\n");
                            continue;
                        }

                        strncpy(ack, buf , 4);
                        ack[4] = '\0';
                        // printf("Frame in string received : %s \n",ack);
                        int frame = 0;
                        for(int j = 0; j < 4; j++){
                            frame = frame * 10 + (ack[j] - '0');
                        }

                        int siz = frame/100;
                        int frameno = frame%100;

                        for(int j = 0; j < 5; j++){
                            if(SM[i].sendbuffer.data[j].frame == frameno && SM[i].sendbuffer.data[j].isbusy == 1 && SM[i].sendbuffer.data[j].ack == 0){
                                SM[i].sendbuffer.data[j].isbusy = 0;
                                SM[i].sendbuffer.data[j].frame = 0;
                                SM[i].sendbuffer.data[j].ack = 1;
                                SM[i].sendwnd.wndsize++;
                                SM[i].sendbuffer.size--;
                                printf("**** Message %d is Acked \n",j);
                            }
                        }

                        // printf("Message received and .Acked to 0\n");

                    }
                    else
                    {
                        printf("Unknown message type received\n");
                    }
                }
            }
        }

        // if(semop(sem3, &signal_op, 1) == -1){
        //     printf("Error: %s\n", strerror(errno));
        //     exit(EXIT_FAILURE);
        // }

    }
    return NULL;
}



int timedifference(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) * 1000000 + (end->tv_usec - start->tv_usec);
}

char *header_ack(int frame, const char *data) {
    // Assuming the acknowledgment header is a fixed size of 4 bytes
    char *header = malloc(4 + strlen(data) + 1); // Allocate memory for header + data + null terminator

    if (header == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    // Copy acknowledgment header to the beginning of the buffer
    snprintf(header, 5, "%04d", frame);

    // Copy data after the acknowledgment header
    strcpy(header + 4, data);

    return header;
}

void *Sthreadfunc(void *arg){

    printf("Sthreadfunc 1\n");

    int shmget_flags = IPC_CREAT | 0666;
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

    printf("Sthreadfunc\n");

    while(1){

        if(semop(sem3, &wait_op, 1) == -1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        sleep(T/2);
        printf("Waited (To see Semaphore is Alive!!!)\n");

        struct timeval now;
        gettimeofday(&now, NULL);

        for(int i=0;i<MAX_SOCKET;i++){
            if(SM[i].isfree == 0){
                // if(SM[i].sendbuffer.size > 0){
                    if(timedifference(&SM[i].sendbuffer.lastsent , &now )> T){
                        printf("Timeout at Socket Index %d Resending ALL !!!\n", i);
                        // May we will have to use the sequence number to resend the frames in order
                        // for(int seqorder = 1; seqorder < MAX_SEQ; seqorder++){
                        //     if(SM[i].sendwnd.sequence[i] == 1)
                        // }
                        for(int j = 0;j<5;j++){
                            if(SM[i].sendbuffer.data[j].isbusy == 1 && SM[i].sendbuffer.data[j].ack == 0 && SM[i].sendbuffer.data[j].frame != 0){
                                printf("Resending Frame %d\n", SM[i].sendbuffer.data[j].frame);
                                char *data = SM[i].sendbuffer.data[j].data;

                                data = header_ack(SM[i].sendbuffer.data[j].frame, data);

                                struct sockaddr_in destaddr;
                                destaddr.sin_family = AF_INET;
                                destaddr.sin_port = htons(SM[i].destport);
                                destaddr.sin_addr.s_addr = inet_addr(SM[i].destIP);
                                sendto(SM[i].sockfd, data, strlen(data), 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
                                printf("Resent Frame %d\n", SM[i].sendbuffer.data[j].frame);
                            }
                        }
                    }
                // }
            }
        }

        // Sending messages depending on the window size of the socket and usinf swnd for the frame sequence
        for(int i = 0;i<MAX_SOCKET;i++){
            if(SM[i].isfree == 0){
                // printf("Is frame %d \n",i);
                // printf("Send Window Size %d\n", SM[i].sendwnd.wndsize);
                // printf("Send Buffer Size %d\n", SM[i].sendbuffer.size);
                // printf("Data in send buffer %s\n", SM[i].sendbuffer.data[0].data);
                // printf("Data in send buffer %s\n", SM[i].sendbuffer.data[1].data);
                int tempsendbufsize = SM[i].sendbuffer.size;
                while( tempsendbufsize > 0 && SM[i].sendwnd.wndsize > 0){
                    int nexttobeframedmessageidx = SM[i].sendbuffer.nextframed;
                    if(SM[i].sendbuffer.data[nexttobeframedmessageidx].isbusy == 1 && SM[i].sendbuffer.data[nexttobeframedmessageidx].frame == 0){
                        SM[i].sendbuffer.data[nexttobeframedmessageidx].frame = SM[i].sendwnd.currframe;
                        SM[i].sendwnd.currframe = (SM[i].sendwnd.currframe + 1) % MAX_SEQ;
                        if(SM[i].sendwnd.currframe == 0){
                            SM[i].sendwnd.currframe = 1;
                        }

                        char *data = SM[i].sendbuffer.data[nexttobeframedmessageidx].data;
                        SM[i].sendbuffer.nextframed = (SM[i].sendbuffer.nextframed + 1) % 5;
                        // SM[i].sendbuffer.size--;
                        tempsendbufsize--;
                        SM[i].sendwnd.wndsize--;

                        data = header_ack(SM[i].sendbuffer.data[nexttobeframedmessageidx].frame, data);
                        struct sockaddr_in destaddr;
                        destaddr.sin_family = AF_INET;
                        destaddr.sin_port = htons(SM[i].destport);
                        destaddr.sin_addr.s_addr = inet_addr(SM[i].destIP);

                        sendto(SM[i].sockfd, data, strlen(data), 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
                        printf("Sent Frame %d\n", SM[i].sendbuffer.data[nexttobeframedmessageidx].frame);

                       
                        gettimeofday(&SM[i].sendbuffer.lastsent, NULL);

                    }
                    else{
                        printf("Error: Frame already framed\n");
                        break;
                    }
                }

                // printf("Something happended \n");
            }
            
        }

        if(semop(sem3, &signal_op, 1) == -1){
            printf("Error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

    }

    return NULL;

}


int noofackleft(sendmsgbuffer sendbuffer){
    int count = 0;
    for(int i = 0; i < 5; i++){
        if(sendbuffer.data[i].isbusy == 1 && sendbuffer.data[i].ack == 0){
            count++;
        }
    }
    return count;
}


void * garbagecollector(void *arg){

    int shmget_flags = IPC_CREAT | 0666;
    // Will implement using ftok() later
    int smkey = ftok(".", 98);
    int SMID = shmget(smkey, MAX_SOCKET * sizeof(mtp_socket), shmget_flags);

    if(SMID == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int sem3key = ftok(".", 95);
    int sem3 = semget(sem3key, 1, shmget_flags);
    if(sem3 == -1){
        printf("Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sembuf wait_op = {0, -1, 0};
    struct sembuf signal_op = {0, 1, 0};

    mtp_socket *SM = (mtp_socket *)shmat(SMID, 0, 0);

    while(1){
            // sleep(2);
            for(int i = 0; i < MAX_SOCKET; i++){
                if(semop(sem3, &wait_op, 1) == -1){
                    printf("Error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

                // printf("Garbage Collector\n");

                if(SM[i].isfree == 0 && noofackleft(SM[i].sendbuffer)==0 && SM[i].receivebuffer.fetchsize == 0){
                    if(kill(SM[i].pid, 0) == -1){
                        // printf("Killing a process\n");
                        if(errno = ESRCH){
                            printf("Process%d , %d is dead\n", i ,SM[i].pid);
                            SM[i].isfree = 1;
                            SM[i].isocketcreated = 0;
                            
                            initializebuffers(&SM[i]);

                        }
                        else{
                            printf("Error: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                    }
                }

                if(semop(sem3, &signal_op, 1) == -1){
                    printf("Error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
    }

    pthread_exit(NULL);

    return NULL;

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
    // sock->receivebuffer.insertsize = 5;
    // sock->receivebuffer.togetframe = 1;
    for(int i=0; i<5; i++){
        sock->receivebuffer.data[i].isbusy = 0;
        sock->receivebuffer.data[i].frame = 0;
        sock->receivebuffer.data[i].ack = 0;
    }

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


int dropMessage(){
    srand(time(NULL));

    // Generate a random number between 0 and 1
    double random_number = (double)rand() / RAND_MAX;

    printf("Random Number : %f \n",random_number);

    if(random_number < p){
        return 1;
    }

    return 0;
}

int main(){
    
        init();
    
        return 0;
}
