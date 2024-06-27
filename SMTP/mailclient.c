/*
    NETWORKS ASSIGNMENT 3

    
    ROLL NO : 21CS30023
    NAME : HARSH SHARMA
    ROLL NO : 21CS10032
    NAME : ISHAN RAJ
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char username[100];
char password[100];

void takingusername(){              // Function to take username
    printf("Enter username: ");
    bzero(username, 100);
    scanf("%s", username);
}

void takingpassword(){              // Function to take password
    printf("Enter password: ");
    bzero(password, 100);
    scanf("%s", password);
}

void Client_QUIT(int sockfd){           // Function for QUIT when Client Exits
    char buffer[120];
    for(int i = 0; i < 120; i++){
        buffer[i] = '\0';
    }
    strcpy(buffer, "QUIT\r\n");                 // sending QUIT
    send(sockfd, buffer, strlen(buffer), 0);

    printf("C : %s\n",buffer);

    for(int i = 0; i < 120; i++){
        buffer[i] = '\0';
    }

    while(1){
        char buffer1[120];
        for(int i = 0; i < 120; i++){
            buffer1[i] = '\0';
        }
        int recvsiz = recv(sockfd, buffer1, 120, 0);
        if(recvsiz <= 0){
            printf("Error in receiving\n");
            close(sockfd);
            break;
        }
        strcat(buffer, buffer1);
        if(strstr(buffer, "\r") != NULL){                   
            break;
        }
    }

    if(strncmp(buffer, "+OK", 3) != 0){
        printf("Error in QUIT\n");
        close(sockfd);
        return;
    }

    printf("Client Disconnected from Server successful\n");
    close(sockfd);

    return;
}


int main(int argc , char *argv[]){
    int port_smtp = 20000;
    int port_pop3 = 20001;
    char * server_ip = "127.0.0.1";
    if(argc == 4){                                      // if custom port and server ip is given
        printf("Using custom port and server ip\n");
        server_ip = argv[1];
        port_smtp = atoi(argv[2]);
        port_pop3 = atoi(argv[3]);
    }
    else{
        printf("Using default port and server ip would be used\n");
    }

    printf("SMTP port: %d , POP3 port: %d, SERVER_IP: %s\n", port_smtp,port_pop3,server_ip);

    printf("*****Welcome to Mail Client*****\n");

    takingusername();
    takingpassword();

    while(1){
        main_menu:
        printf("------------Main Menu-----------\n");
        printf("1. Manage Mail\n");
        printf("2. Send Mail\n");
        printf("3. Quit\n");
        printf("--------------------------------\n");
        printf("Enter your choice: ");
        int input;

        if (scanf("%d", &input) != 1) {
            // Handle invalid input (non-integer)
            printf("Invalid input. Please enter a number.\n");

            // Clear the input buffer
            while (getchar() != '\n');
            
            // Go back to the main menu
            goto main_menu;
        }

        // Clear the input buffer
        // while (getchar() != '\n');

        if(input == 1){
            // See mail
            int sockfd;
            struct sockaddr_in serv_addr;           // server address

            if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){     // creating socket
                printf("Error creating socket\n");
                continue;
            }

            serv_addr.sin_family = AF_INET;             // setting server address
            serv_addr.sin_port = htons(port_pop3);          // setting port
            inet_aton(server_ip, &serv_addr.sin_addr);          // setting ip address

            if((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0){        // connecting to server
                printf("Error connecting to server\n");
                close(sockfd);
                continue;
            }

            char buffer[120];
            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }
            while(1){               // receiving Server ready message
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            if(strncmp(buffer, "+OK", 3) != 0){         // checking if server is ready
                printf("Server not ready\n");
                close(sockfd);
                continue;
            }

            


            username:           // Label to take username (repeatedly)
            

            // Send username and password to server
            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }
            sprintf(buffer, "USER %s\r\n", username);       // sending username
            send(sockfd, buffer, strlen(buffer), 0);        // sending username

            
            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            while(1){               // receiving Server ready message
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            if(strncmp(buffer, "-ERR", 3) == 0){         // Again taking username if invalid
                printf("Invalid Username. Not found\n"); 
                takingusername();
                goto username;
            }

            if(strncmp(buffer, "+OK", 3) != 0){         // Validated username
                printf("Server not ready\n");
                Client_QUIT(sockfd);
                continue;
            }



            // Label to take password
            password:

            bzero(buffer, 120);

            sprintf(buffer, "PASS %s\r\n", password);

            send(sockfd, buffer, strlen(buffer), 0);

            bzero(buffer, 120);

            
            while(1){               // receiving Server ready message
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            if(strncmp(buffer, "-ERR", 3) == 0){         // Again taking password if invalid
                printf("Invalid Password\n");
                takingpassword();
                goto password;

            }

            if(strncmp(buffer, "+OK", 3) != 0){         // validated password
                printf("Server not ready\n");
                Client_QUIT(sockfd);
                continue;
            }

            printf("Logged in successfully\n");

            bzero(buffer, 120);
                sprintf(buffer, "STAT\r\n");                // sending STAT
                send(sockfd, buffer, strlen(buffer), 0);
            bzero(buffer, 120);

            while(1){                                   // receiving STAT
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            // printf("S : %s\n",buffer);
            if(strncmp(buffer, "+OK", 3) != 0){         // checking if STAT is successful
                printf("Error in STAT\n");
                Client_QUIT(sockfd);                    // QUIT if STAT is not successful
                break;
            }
            int mail_count;
            sscanf(buffer, "+OK %d", &mail_count);
            printf("Mail Count: %d\n", mail_count);         // Total mail count in this login
            
            bzero(buffer, 120);

            while(1){
                
                if(mail_count == 0){
                    printf("No mail to show\n");
                    break;
                }

                printf("----------------------Listing mails--------------------------------\n");
                for(int i = 1; i <= mail_count; i++){                   // Sending RETR for each mail
                    bzero(buffer, 120);
                    sprintf(buffer, "RETR %d\r\n", i);
                    send(sockfd, buffer, strlen(buffer), 0);
                    // printf("C : %s\n",buffer);

                    char onemail[4500];             // Storing the mail
                    bzero(onemail, 4500);
                    while(1){
                        char buffer1[120];
                        for(int i = 0; i < 120; i++){
                            buffer1[i] = '\0';
                        }
                        int recvsiz = recv(sockfd, buffer1, 120, 0);
                        if(recvsiz <= 0){
                            printf("Error in receiving\n");
                            close(sockfd);
                            break;
                        }
                        strcat(onemail, buffer1);
                        if(strstr(onemail, "\n.\r\n") != NULL){                   
                            break;
                        }

                        if(strstr(onemail, "-ERR") != NULL && strstr(onemail, "\r\n") != NULL){
                            break;
                        }
                    }

                    // printf("S : %s\n",onemail);

                    // printf("CONTENT OF MAIL ::: %s\n", onemail);

                    if(strncmp(onemail, "-ERR", 4) == 0){         // checking if RETR is successful
                        // printf("Mail %d in RETR\n");
                        continue;
                    }

                    // printf("S : %s\n",buffer);
                    char frompart[100];                 // Segregating the mail
                    char subjectpart[100];
                    char timepart[100];

                    bzero(frompart, 100);
                    bzero(subjectpart, 100);
                    bzero(timepart, 100);

                    
                    char *token = strtok(onemail, "\r\n"); // Extracts the +OK line
                    token = strtok(NULL, "\r\n"); // Extracts the from line
                    strcpy(frompart, token);

                    token = strtok(NULL, "\r\n"); // Extracts the to line

                    
                    token = strtok(NULL, "\r\n"); // Extracts the subject line
                    strcpy(subjectpart, token);

                    token = strtok(NULL, "\r\n"); // Extracts the date line
                    strcpy(timepart, token);

                    char *email = strtok(frompart, " ");
                    email = strtok(NULL, " \r\n");

                    char *subject = strtok(subjectpart, " ");
                    subject = strtok(NULL, "\r\n");

                    char *time = strtok(timepart, " ");
                    time = strtok(NULL, "\r\n");


                    // Listing the mails
                    printf("%d. %s %s %s\n", i, email, subject, time);
                }

                printf("Enter the mail number to read or -1 to return to Main Menu: ");
                
                int mail_number;
                scanf("%d", &mail_number);
                printf("--------------------------------------------\n");
                
                if(mail_number == -1){
                    break;
                }

                if(mail_number < 1 || mail_number > mail_count){            // Invalid mail number
                    printf("Invalid mail number\n");
                    continue;
                }

                bzero(buffer, 120);
                sprintf(buffer, "RETR %d\r\n", mail_number);            // sending RETR
                send(sockfd, buffer, strlen(buffer), 0);

                char RETOUTPUT[4500];
                bzero(RETOUTPUT, 4500);             // Storing the mail
                while(1){
                    char buffer1[120];
                    for(int i = 0; i < 120; i++){
                        buffer1[i] = '\0';
                    }
                    int recvsiz = recv(sockfd, buffer1, 120, 0);
                    if(recvsiz <= 0){
                        printf("Error in receiving\n");
                        close(sockfd);
                        break;
                    }
                    strcat(RETOUTPUT, buffer1);
                    if(strstr(RETOUTPUT, "\n.\r\n") != NULL){                       // checking if RETR is successful  
                        break;
                    }

                    if(strstr(RETOUTPUT, "-ERR") != NULL && strstr(RETOUTPUT, "\r\n") != NULL){ // checking if RETR got -ERR
                        break;
                    }
                }

                // printf("S RTOUT: %s\n",RETOUTPUT);

                if(strncmp(RETOUTPUT, "-ERR", 4) == 0){        
                    printf("ERR in Executing\n");
                    continue;
                }
                // printf("%s", RETOUTPUT);
                char *onemail = strtok(RETOUTPUT, "\n");
                onemail = strtok(NULL, "");

                printf("Mail %d: \n%s\n",mail_number,onemail);


                printf("Enter d to delete mail or any other key to return to mail list: ");
                char ch = getchar();
                while(ch == '\n'){
                    ch = getchar();
                }

                if(ch == 'd' || ch == 'D')
                {
                    // // Delete mail
                    // delete_mail:

                    bzero(buffer, 120);
                    sprintf(buffer, "DELE %d\r\n", mail_number);            // sending DELE
                    send(sockfd, buffer, strlen(buffer), 0);

                    bzero(buffer, 120);
                    while(1){
                        char buffer1[120];
                        for(int i = 0; i < 120; i++){
                            buffer1[i] = '\0';
                        }
                        int recvsiz = recv(sockfd, buffer1, 120, 0);
                        if(recvsiz <= 0){
                            printf("Error in receiving\n");
                            close(sockfd);
                            break;
                        }
                        strcat(buffer, buffer1);
                        if(strstr(buffer, "\r") != NULL){                   
                            break;
                        }
                    }

                    printf("S : %s\n",buffer);
                    if(strncmp(buffer, "+OK", 3) != 0){        
                        printf("Error in DELE\n");
                        Client_QUIT(sockfd);
                        break;
                    }

                    printf("Mail %d Marked for deletion successfully\n", mail_number);
                }

            }

            Client_QUIT(sockfd);            // QUIT after all mails are read or invalid mail
            continue;





        }
        else if(input == 2){
            int sockfd;
            struct sockaddr_in serv_addr;           // server address

            if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){     // creating socket
                printf("Error creating socket\n");
                exit(1);
            }

            serv_addr.sin_family = AF_INET;             // setting server address
            serv_addr.sin_port = htons(port_smtp);          // setting port
            inet_aton(server_ip, &serv_addr.sin_addr);          // setting ip address

            if((connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0){        // connecting to server
                printf("Error connecting to server\n");
                close(sockfd);
                exit(1);
            }

            char buffer[120];


            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }
            while(1){               // receiving Server ready message
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }


            }

            printf("S : %s \n",buffer);

            if(strncmp(buffer, "220", 3) != 0){         // checking if server is ready
                printf("Server not ready\n");   
                close(sockfd);
                continue;
            }


            // Retrieving server name
            char server_name[100];              
            // space separated
            char *token = strtok(buffer, " <>");
            token = strtok(NULL, " <>");
            strcpy(server_name, token);         // copying server name
        
            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }
            // printf("Server Name: %s\n", server_name);
            strcpy(buffer, "HELO");             // sending HELO
            sprintf(buffer, "%s %s", buffer, server_name);
            sprintf(buffer, "%s%s", buffer, "\r\n");

            send(sockfd, buffer, strlen(buffer), 0); 

            printf("C : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }


            while(1){               // receiving welcome message
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            printf("S : %s\n",buffer);

            if(strncmp(buffer, "250 OK", 6) != 0){          // checking if HELO is successful
                printf("Error in HELO\n");
                close(sockfd);
                continue;
            }

            

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }



            char *from = NULL;
            char *to = NULL;
            char *subject = NULL;
            char *body = NULL;
            char *body_part = NULL;


            from = (char *)malloc(100 * sizeof(char));
            to = (char *)malloc(100 * sizeof(char));
            subject = (char *)malloc(100 * sizeof(char));
            body = (char *)malloc(4500 * sizeof(char));
            body_part = (char *)malloc(80 * sizeof(char));

            // Emtying the strings
            for(int i = 0; i < 100; i++){
                from[i] = '\0';
                to[i] = '\0';
                subject[i] = '\0';
            }

            for(int i = 0; i < 4500; i++){
                body[i] = '\0';
            }

            for(int i = 0; i < 80; i++){
                body_part[i] = '\0';
            }

            printf("Taking Mail Input from User\n");        // taking mail input from user
            
            printf("From: ");                               // taking from
            getchar();
            size_t siz = 0;
            int read;
            read = getline(&from,&siz,stdin);
            from[read-1] = '\0';
            if(read == -1){
                printf("Error in From\n");
                close(sockfd);
                continue;
            }
            printf("To: ");                     // taking to
            read = getline(&to,&siz,stdin);
            to[read-1] = '\0';
            printf("Subject: ");
            read = getline(&subject,&siz,stdin);
            subject[read-1] = '\0';
            printf("Body: ");

            strcat(body, "From: ");
            strcat(body, from);
            strcat(body, "\r\n");
            strcat(body, "To: ");
            strcat(body, to);
            strcat(body, "\r\n");
            strcat(body, "Subject: ");
            strcat(body, subject);
            strcat(body, "\r\n");


            for(int i = 0; i < 80; i++){
                body_part[i] = '\0';
            }


            while(strcmp(body_part, ".") != 0){             // taking body
                read = getline(&body_part,&siz,stdin);
                body_part[read-1] = '\0';
                strcat(body, body_part);
                strcat(body, "\n");
                
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            sprintf(buffer, "MAIL FROM:<%s>", from);
            sprintf(buffer, "%s%s", buffer, "\r\n");
            send(sockfd, buffer, strlen(buffer), 0);

            printf("C : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            while(1){                   // receiving message from server
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }
            
            printf("S : %s\n",buffer);

            if(strncmp(buffer, "550",3) == 0){
                printf("NOT A VALID SENDER\n");
                close(sockfd);
                continue;
            }

            if(strncmp(buffer, "250", 3) != 0){         // checking if MAIL FROM is successful
                printf("Error in MAIL FROM\n");
                close(sockfd);
                continue;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            sprintf(buffer, "RCPT TO:<%s>", to);         // sending RCPT TO
            sprintf(buffer, "%s %s", buffer, "\r\n");
            send(sockfd, buffer, strlen(buffer), 0);

            printf("C : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            printf("S : %s\n",buffer);

            if(strncmp(buffer, "550",3) == 0){
                printf("NOT A VALID RECIPIENT\n");
                close(sockfd);
                continue;
            }

            if(strncmp(buffer, "250 root", 7) != 0){            // checking if RCPT TO is successful
                printf("Error in RCPT TO\n");
                close(sockfd);
                continue;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            strcpy(buffer, "DATA\r\n");             // sending DATA
            send(sockfd, buffer, strlen(buffer), 0);

            printf("C : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }


            while(1){                                   // receiving message from server
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            printf("S : %s\n",buffer);

            if(strncmp(buffer, "354 Enter mail", 8) != 0){      // checking if DATA is successful
                printf("Error in DATA\n");
                close(sockfd);
                continue;
            }

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }


            
            int cnt = 0;
            while(!(buffer[0]=='.' && buffer[1]=='\r' && buffer[2]=='\n')){     // sending body
                for(int i = 0; i < 120; i++){
                    buffer[i] = '\0';
                }
                int j = 0;
                while(body[cnt] != '\n'){
                    buffer[j] = body[cnt];
                    cnt++;
                    j++;
                }
                cnt++;
                if(strcmp(buffer , ".")==0){
                    strcat(buffer , "\r\n");
                }
                else{
                    strcat(buffer , "\n");
                }
                send(sockfd, buffer, strlen(buffer), 0);
                printf("C : %s\n",buffer);
                

            }


            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }


            while(1){                                   // receiving message from server
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }

            printf("S : %s\n",buffer);

            

            if(strncmp(buffer, "250", 3) != 0){                  // checking if body is sent successfully
                printf("Error in Message accepted for delivery\n");
                close(sockfd);
                continue;
            }


            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            strcpy(buffer, "QUIT\r\n");                 // sending QUIT
            send(sockfd, buffer, strlen(buffer), 0);

            printf("C : %s\n",buffer);

            for(int i = 0; i < 120; i++){
                buffer[i] = '\0';
            }

            // recv(sockfd, buffer, 120, 0);


            while(1){
                char buffer1[120];
                for(int i = 0; i < 120; i++){
                    buffer1[i] = '\0';
                }
                int recvsiz = recv(sockfd, buffer1, 120, 0);
                if(recvsiz <= 0){
                    printf("Error in receiving\n");
                    close(sockfd);
                    break;
                }
                strcat(buffer, buffer1);
                if(strstr(buffer, "\r") != NULL){                   
                    break;
                }
            }


            printf("S : %s\n",buffer);

            if(strncmp(buffer, "221", 3) != 0){
                printf("Error in QUIT\n");
                close(sockfd);
                continue;
            }

            

            printf("Mail sent successfully\n");

            free(from);
            free(to);
            free(subject);
            free(body);
            free(body_part);

            close(sockfd);

        }
        else if(input == 3){
            break;
        }
        else{
            printf("Invalid Input\n");       
        }


    }

}