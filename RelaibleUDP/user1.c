#include "msocket.h"

int main() {

    printf("User1\n");

    int sockfd = m_socket(AF_INET, SOCK_MTP, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    printf("Sockefd : %d\n", sockfd);
    
    // Set up the server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4577); 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Connect to the server
    char *server_ip = "127.0.0.1";
    int server_port = 4577;
    char * srcip = "127.0.0.1";
    int srcport = 4578;

    if(m_bind(sockfd, srcip,srcport,server_ip,server_port) < 0){
        perror("Error binding socket");
        return 1;
    }
    

    FILE *fp = fopen("test1.txt", "r");
    FILE *fp1 = fopen("testcheck.txt", "w");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char buffer[1028];
    bzero(buffer, 1028);

    while(fgets(buffer, 1024, fp) != NULL) {
        // printf("\n\nbuffer %s\n\n", buffer);
        while(1){
            if(m_sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                printf("Warning FULL sending message full\n");
            }
            else{
                // printf("sending message %s \n", buffer);
                fprintf(fp1, "%s", buffer);
                bzero(buffer, 1028);
                break;
            }
        }
    }


    printf("Message sent\n");
    
    // Close the socket

    // m_close(sockfd);
    
    return 0;
}