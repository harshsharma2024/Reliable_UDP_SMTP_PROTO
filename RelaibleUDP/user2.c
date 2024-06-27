#include "msocket.h"

int main() {

    printf("User2\n");

    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    printf("Sockefd : %d\n", sockfd);
    
    // Set up the server address
    struct sockaddr_in server_addr;
    char *server_ip = "127.0.0.1";
    int server_port = 4578;
    char * srcip = "127.0.0.1";
    int srcport = 4577;

    struct sockaddr_in cliaddr;
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(4577);
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if(m_bind(sockfd, srcip,srcport,server_ip,server_port) < 0){
        perror("Error binding socket");
        return 1;
    }

    printf("User2 Binding done\n");


    char buffer[1028];

    server_addr.sin_port = htons(4578);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;


    FILE *fp = fopen("test2.txt", "a+");

    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    while(1) {
        bzero(buffer, 1024);
        if(m_recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("BUFFER EMPTY receiving message");
            continue;
        }

        // printf("Received message: %s\n", buffer);

        fprintf(fp, "%s", buffer);
        // printf("\nWrote to file\n");
        fflush(fp);
    }



    printf("Message sent\n");

    // Close the socket

    // m_close(sockfd);

    return 0;





}