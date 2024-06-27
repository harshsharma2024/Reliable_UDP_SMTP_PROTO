/*
    NETWORKS ASSIGNMENT 3


    ROLL NO : 21CS30023
    NAME : HARSH SHARMA
    ROLL NO : 21CS10032
    NAME : ISHAN RAJ
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>


int main(int argc , char *argv[]){
    int port_no = 20000;

    if(argc == 2){
        printf("Command line argument provided\n");
        port_no = atoi(argv[1]);                // Port number provided as command line argument
    }
    else{
            printf("Default port number 20000 used\n");
    }

    printf("Port Number : %d\n", port_no);

    int sockfd , newsockfd ;                // Socket file descriptors
    int clilen;                             // Client length
    struct sockaddr_in cli_addr, serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){     // Creating socket
        printf("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_no);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){        // Binding socket
        printf("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5); // Upto 5 concurrent client requests will be queued up while the system is executing the "accept" system call below.

    char server_name[120] = "iitkgp.edu";

    while(1){
        printf("\n\nServer Running...\n");
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);         // Accepting client request

        if(newsockfd < 0){
            printf("Accept error\n");
            exit(0);
        }

        if(fork() == 0){
            close(sockfd);

            // print IP of client
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(cli_addr.sin_addr), ip, INET_ADDRSTRLEN);      // Converting IP address from binary to text form
            printf("Client Connected with IP : %s\n", ip);

            char buffer[120];

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 220 Service ready
            strcpy(buffer, "220 <");
            strcat(buffer, server_name);
            strcat(buffer, "> Service ready\r\n");
            printf("Sending : %s\n",buffer);
            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }


            // Receiving HELO
            while(1){
                char buffer1[120];
                bzero(buffer1, 120);
                int recvsiz = recv(newsockfd, buffer1, 120, 0);
                if(recvsiz < 0){
                printf("Error in receiving HELO\n");
                close(newsockfd);
                exit(0);
                break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r\n") != NULL){                   
                    break;
                }
            }


            printf("C : %s\n",buffer);

            // printf("%s",buffer);
            // printf("%s\n",server_name);
            // Checking if HELO is correct
            if(strncmp(buffer,"HELO",4) != 0){
                printf("Error in HELO\n");
                close(newsockfd);
                exit(0);
                break;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 250 OK HELO
            strcpy(buffer, "250 OK HELO ");
            strcat(buffer, server_name);
            strcat(buffer, "\r\n");
            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Receiving MAIL FROM
            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(newsockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(newsockfd);
                    exit(0);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){
                    break;
                }
            }

            printf("C : %s\n", buffer);

            // Checking if MAIL FROM is correct
            if(strncmp(buffer,"MAIL FROM",9) != 0){
                printf("Error in MAIL FROM\n");
                close(newsockfd);
                exit(0);
                break;
            }

            char sender_email[100];
            
            // Extracting sender email from MAIL FROM
            char *token = strtok(buffer, " \r\n<>");
            token = strtok(NULL, " \r\n<>");
            token = strtok(NULL, " \r\n<>");
            strcpy(sender_email, token);

            // Checking if sender email is valid
            if(strstr(sender_email, "@") == NULL){
                printf("Invalid Sender Email\n");
                int stst = send(newsockfd, "550 No such Email\r\n", 23, 0);
                if(stst < 0){
                    printf("Error in sending\n");
                }
                printf("S : 550 No such Email\n");
                close(newsockfd);
                exit(0);
                break;
            }

            // Extracting sender username from sender email
            char sender_username[100];
            for(int i = 0; i < 100; i++){
                sender_username[i] = '\0';
            }

            for(int i = 0; i < strlen(sender_email); i++){
                if(sender_email[i] == '@'){
                    break;
                }
                sender_username[i] = sender_email[i];
            }

            // Checking if sender username is valid
            int fd = open("user.txt", O_RDONLY);
            if(fd < 0){
                printf("Error in opening user.txt\n");
                close(newsockfd);
                exit(0);
                break;
            }

            
            int cnt = 0;
            int flag = 0;
            int bytes_read;
            while(bytes_read = read(fd,&buffer[cnt] , 1)>0){
                if(buffer[cnt] == ' '){
                    buffer[cnt] = '\0';
                    if(strcmp(buffer, sender_username) == 0){
                        flag = 1;
                        break;
                    }
                    cnt = 0;
                    continue;
                }
                if(buffer[cnt] == '\n'){
                    cnt = 0;
                    continue;
                }
                cnt++;
            }

            // If sender username is not valid
            if(flag == 0){
                printf("Invalid Sender Username\n");
                int stst = send(newsockfd, "550 No such User here\r\n", 23, 0);
                if(stst < 0){
                    printf("Error in sending\n");
                }
                printf("S : 550 No such User here\n");
                close(newsockfd);
                exit(0);
                break;
            }

            close(fd);



            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 250 OK Sender
            strcpy(buffer, "250 <");
            strcat(buffer, sender_email);
            strcat(buffer, ">... Sender ok\r\n");

            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Receiving RCPT TO
            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(newsockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(newsockfd);
                    exit(0);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){
                    break;
                }
            }

            printf("C : %s\n", buffer);
            // Checking if RCPT TO is correct
            if(strncmp(buffer,"RCPT TO",7) != 0){
                printf("Error in RCPT TO\n");
                close(newsockfd);
                exit(0);
                break;
            }



            // Extracting receiver email from RCPT TO
            char receiver_email[100];

            token = strtok(buffer, " \r\n<>");
            token = strtok(NULL, " \r\n<>");
            token = strtok(NULL, " \r\n<>");
            strcpy(receiver_email, token);


            if(strstr(receiver_email, "@") == NULL){
                printf("Invalid Receiver Email\n");
                int stst = send(newsockfd, "550 No such Email\r\n", 23, 0);
                if(stst < 0){
                    printf("Error in sending\n");
                }
                printf("S : 550 No such Email\n");
                close(newsockfd);
                exit(0);
                break;
            }
            

            // Extracting receiver username from receiver email
            char receiver_username[100];
            for(int i = 0; i < 100; i++){
                receiver_username[i] = '\0';
            }

            // Checking if receiver username is valid
            for(int i = 0; i < strlen(receiver_email); i++){
                if(receiver_email[i] == '@'){
                    break;
                }
                receiver_username[i] = receiver_email[i];
            }

            cnt = 0;
            flag = 0;
            bytes_read;
            fd = open("user.txt", O_RDONLY);
            while(bytes_read = read(fd,&buffer[cnt] , 1)>0){
                if(buffer[cnt] == ' '){
                    buffer[cnt] = '\0';
                    if(strcmp(buffer, receiver_username) == 0){
                        flag = 1;
                        break;
                    }
                    cnt = 0;
                    continue;
                }
                if(buffer[cnt] == '\n'){
                    cnt = 0;
                    continue;
                }
                cnt++;
            }
            close(fd);

            if(flag == 0){
                printf("Invalid Receiver username\n");
                send(newsockfd, "550 No such User here\r\n", 23, 0);
                close(newsockfd);
                exit(0);
                break;
            }


            

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 250 OK Receiver
            strcpy(buffer, "250 root... Recipient ok\r\n");
            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Receiving DATA
            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(newsockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(newsockfd);
                    exit(0);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){
                    
                    break;
                }
            }

            printf("C : %s\n",buffer);

            // Checking if DATA is correct
            if(strncmp(buffer,"DATA",4) != 0){
                printf("Error in DATA\n");
                close(newsockfd);
                exit(0);
                break;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 354 Enter mail
            strcpy(buffer, "354 Enter mail, end with \".\" on a line by itself\r\n");

            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            char file_address[100];
            for(int i = 0; i < 100; i++){
                file_address[i] = '\0';
            }

            // Opening mymailbox of receiver
            strcpy(file_address, receiver_username);
            strcat(file_address, "/mymailbox");

            fd = open(file_address, O_WRONLY | O_APPEND | O_CREAT, 0644);

            if(fd < 0){
                printf("Error in opening mymailbox\n");
                close(newsockfd);
                exit(0);
                break;
            }

            // Received Time
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);

            char time[100];

            sprintf(time, "Received: %d-%02d-%02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min);

            // Receiving mail
            char received_mail[4500];

            for(int i = 0; i < 4500; i++){
                received_mail[i] = '\0';
            }

            // Receiving mail
            while(1){
                for(int i = 0; i < 120; i++){
                    buffer[i] = '\0';
                }

                int recvsiz = recv(newsockfd, buffer, 120, 0);
                // printf("recvsiz : %d , %s 000\n", recvsiz,buffer);


                if(recvsiz < 0){
                    printf("Error in receiving\n");
                    close(newsockfd);
                    exit(0);
                    break;
                }
                

                printf("C : %s\n", buffer);


                strcat(received_mail, buffer);

                if(strstr(received_mail, "\n.\r\n") != NULL){
                    // printf("Received Terminator\n");
                    break;
                }

            
            }

            // remove all occurrences of \r from received_mail

            for(int i = 0; i < strlen(received_mail); i++){
                if(received_mail[i] == '\r'){
                    for(int j = i; j < strlen(received_mail); j++){
                        received_mail[j] = received_mail[j+1];
                    }
                    i--;
                }
            }



            // Extracting index till subject from mail
            int subject_end = 0;

            for(int i = 0; i < strlen(received_mail); i++){
                if(received_mail[i] == '\n'){
                    subject_end++;
                }
                if(subject_end == 3){
                    subject_end = i;
                    break;
                }
            }
            subject_end++;

            // Writing mail to mymailbox
            write(fd, received_mail, subject_end);
            // Writing time to mymailbox
            write(fd, time, strlen(time));
            // Writing mail to mymailbox
            write(fd, received_mail + subject_end, strlen(received_mail) - subject_end);


            close(fd);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 250 OK Message accepted for delivery
            strcpy(buffer, "250 OK Message accepted for delivery\r\n");

            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Receiving QUIT
            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(newsockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(newsockfd);
                    exit(0);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){
                    break;
                }
            }

            printf("C : %s\n",buffer);

            // Checking if QUIT is correct
            if(strncmp(buffer,"QUIT",4) != 0){
                printf("Error in QUIT\n");
                close(newsockfd);
                exit(0);
                break;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // Sending 221 Server closing connection
            strcpy(buffer, "221 ");
            strcat(buffer, server_name);
            strcat(buffer, " closing connection\r\n");

            send(newsockfd, buffer, strlen(buffer), 0);

            printf("S : %s\n",buffer);


            // Closing connection
            printf("\n\nMail Received\n");
            printf("\n\nClient Disconnected\n");

            close(newsockfd);

            printf("\n\nServer Running...\n");

            exit(0);
            
        }
        // Closing socket
        close(newsockfd);
    }

    return 0;
}