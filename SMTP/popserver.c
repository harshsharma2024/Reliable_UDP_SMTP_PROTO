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
#include <stdio.h>

#define MAX_MAILS 50            // Maximum number of mails that can be stored in the mailbox

char username[120];
char password[120];

struct Mail {               // Structure to store mail details
    char from[120];
    char to[120];
    char date[120];
    char subject[120];
    char content[4000];
    int markedForDeletion;
};


struct Mail *mailbox;
int numMails = 0;


// Initialize the mailbox
void init(){
    // Allocating memory for the mailbox
    mailbox = (struct Mail *)malloc(MAX_MAILS * sizeof(struct Mail));           // We are assuming that the mails won;t exceed MAXMAILS, Else we can change the size of mailbox

    for(int i = 0; i < MAX_MAILS; i++){
        mailbox[i].markedForDeletion = 0;
        mailbox[i].from[0] = '\0';
        mailbox[i].to[0] = '\0';
        mailbox[i].date[0] = '\0';
        mailbox[i].subject[0] = '\0';
        mailbox[i].content[0] = '\0';
    }
    char filepath[150];
    sprintf(filepath, "%s/mymailbox", username);
    // Reading the mails from the mailbox
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }

    char line[120];
    int i = 0;

    char single_mail[4500];
    bzero(single_mail, 4500);
    while(1){
        bzero(line, 120);
        if(fgets(line, 120, fp) == NULL){
            break;
        }
        // Store the lines in single_mail until . comes
        strcat(single_mail, line);
        if(strcmp(line, ".\n") == 0){
            // Store the single_mail in mailbox from till /n comes
            int j = 0;
            while(single_mail[j] != '\n'){
                mailbox[i].from[j] = single_mail[j];
                j++;
            }
            mailbox[i].from[j] = '\0';
            j++;
            int k = 0;
            while(single_mail[j] != '\n'){
                mailbox[i].to[k] = single_mail[j];
                j++;
                k++;
            }
            mailbox[i].to[k] = '\0';
            j++;
            k = 0;
            while(single_mail[j] != '\n'){
                mailbox[i].subject[k] = single_mail[j];
                j++;
                k++;
            }
            mailbox[i].subject[k] = '\0';
            j++;
            k = 0;
            while(single_mail[j] != '\n'){
                mailbox[i].date[k] = single_mail[j];
                j++;
                k++;
            }
            mailbox[i].date[k] = '\0';
            j++;
            // Now store the remaining without \n check

            k = 0;
            while(single_mail[j] != '\0'){
                mailbox[i].content[k] = single_mail[j];
                j++;
                k++;
            }


            i++;
            bzero(single_mail, 4500);
        }
        
    }
    numMails = i;
    fclose(fp);

    // Print the mailbox
    // for(int i = 0; i < numMails; i++){
    //     printf("Mail %d\n", i + 1);
    //     printf("From: %s\n", mailbox[i].from);
    //     printf("To: %s\n", mailbox[i].to);
    //     printf("Date: %s\n", mailbox[i].date);
    //     printf("Subject: %s\n", mailbox[i].subject);
    //     printf("Content: %s\n", mailbox[i].content);
    // }
    

}

// Function to handle USER command
int user_cmd(char *cmd_buffer, int newsockfd)
{
    // Extracting username from the command
    int i = 5;
    int j = 0;
    while (cmd_buffer[i] != '\r')
    {
        username[j] = cmd_buffer[i];
        i++;
        j++;
    }

    username[j] = '\0';

    // Checking if username is valid from user.txt file
    FILE *fp = fopen("user.txt", "r");
    if (fp == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }
    char line[120];
    while(1){
        bzero(line, 120);
        if(fgets(line, 120, fp) == NULL){
            break;
        }
        char *token = strtok(line, " ");
        if(strcmp(token, username) == 0){
            send(newsockfd, "+OK User accepted\r\n", 20, 0);
            fclose(fp);
            return 1;
        }
    }

    send(newsockfd, "-ERR User not found\r\n", 22, 0);

    return 0;
}

// Function to handle PASS command
int pass_cmd(char *cmd_buffer, int newsockfd)
{
    // Extracting password from the command
    int i = 5;
    int j = 0;
    while (cmd_buffer[i] != '\r')
    {
        password[j] = cmd_buffer[i];
        i++;
        j++;
    }

    password[j] = '\0';

    // Checking if password is valid from user.txt file
    FILE *fp = fopen("user.txt", "r");
    if (fp == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }
    char line[120];
    while(1){
        bzero(line, 120);
        if(fgets(line, 120, fp) == NULL){
            break;
        }
        // get the second token
        char *token = strtok(line, " \n");
        // token = strtok(NULL, " \r\n");

        if(strcmp(token, username) == 0){
            token = strtok(NULL, " \n");
            if(strcmp(token, password) == 0){
                send(newsockfd, "+OK Password accepted\r\n",24, 0);
                fclose(fp);
                init(); // Initialize the mailbox after successful login
                return 1;
            }
        }
    }
    send(newsockfd, "-ERR Invalid password\r\n", 24, 0);

    return 0;
}

// Function to handle DELE command
int dele_cmd(char *cmd_buffer, int newsockfd)
{
    // Extracting mail number from the command
    int i = 5;
    int mailNumber = 0;
    while (cmd_buffer[i] != '\r')
    {
        mailNumber = mailNumber * 10 + (cmd_buffer[i] - '0');
        i++;
    }


    // Marking the mail for deletion
    if (mailNumber > 0 && mailNumber <= numMails) {
        mailNumber--;
        mailbox[mailNumber].markedForDeletion = 1;
        send(newsockfd, "+OK Mail marked for deletion\r\n", 31, 0);
        return 1;
    } else {
        send(newsockfd, "-ERR Invalid mail number\r\n", 27, 0);
        return 0;
    }
}

// Function to handle STAT command
void stat_cmd(int newsockfd)
{
    int numValidMails = 0;
    int totalValidOctets = 0;

    // Counting the number of valid mails and octets
    for (int i = 0; i < numMails; i++) {
        if (!mailbox[i].markedForDeletion) {
            numValidMails++;
            totalValidOctets += strlen(mailbox[i].content) + strlen(mailbox[i].from) + strlen(mailbox[i].to) + strlen(mailbox[i].date) + strlen(mailbox[i].subject);
        }
    }

    char response[100];
    bzero(response, 100);
    sprintf(response, "+OK %d %d\r\n", numValidMails, totalValidOctets);
    send(newsockfd, response, strlen(response), 0);
}

// Function to handle LIST command
void list_cmd(char *cmd_buffer, int newsockfd)
{
    // Extracting mail number from the command
    int i = 5;
    int mailNumber = 0;
    // handle LIST without argument
    if(cmd_buffer[4] == '\r'){
        char response[100];
        bzero(response, 100);
        sprintf(response, "+OK %d messages (%d octets)\r\n", numMails, (int)(strlen(mailbox[0].content) + strlen(mailbox[0].from) + strlen(mailbox[0].to) + strlen(mailbox[0].date) + strlen(mailbox[0].subject)));
        send(newsockfd, response, strlen(response), 0);

        // send the list of mails
        for (int i = 0; i < numMails; i++) {
            if (!mailbox[i].markedForDeletion) {
                bzero(response, 100);
                sprintf(response, "%d %d\r\n", i + 1, (int)(strlen(mailbox[i].content) + strlen(mailbox[i].from) + strlen(mailbox[i].to) + strlen(mailbox[i].date) + strlen(mailbox[i].subject)));
                send(newsockfd, response, strlen(response), 0);
            }
        }
        return;
    }

    // handle LIST with argument
    while (cmd_buffer[i] != '\r')
    {
        mailNumber = mailNumber * 10 + (cmd_buffer[i] - '0');
        i++;
    }

    if (mailNumber > 0 && mailNumber <= numMails) {
        mailNumber--;
        if (!mailbox[mailNumber].markedForDeletion) {
            char response[100];
            bzero(response, 100);
            sprintf(response, "+OK %d %d\r\n", mailNumber + 1, (int)(strlen(mailbox[mailNumber].content) + strlen(mailbox[mailNumber].from) + strlen(mailbox[mailNumber].to) + strlen(mailbox[mailNumber].date) + strlen(mailbox[mailNumber].subject)));
            send(newsockfd, response, strlen(response), 0);
            return;
        } else {
            send(newsockfd, "-ERR Mail already deleted\r\n", 28, 0);
            return;
        }
    } else {
        send(newsockfd, "-ERR Invalid mail number\r\n", 27, 0);
        return;
    }

    return;
}

// Function to handle RETR command
int retr_cmd(char *cmd_buffer , int newsockfd){
    // Extracting mail number from the command
    int i = 5;
    int mailNumber = 0;
    while (cmd_buffer[i] != '\r')
    {
        mailNumber = mailNumber * 10 + (cmd_buffer[i] - '0');
        i++;
    }

    // printf("Mail number for RETR : %d\n", mailNumber);

    if (mailNumber > 0 && mailNumber <= numMails) {
        mailNumber--;
        if (!mailbox[mailNumber].markedForDeletion) {
            char response[120];

            // send +OK
            bzero(response, 120);
            sprintf(response, "+OK %d octets\r\n", (int)(strlen(mailbox[mailNumber].content) + strlen(mailbox[mailNumber].from) + strlen(mailbox[mailNumber].to) + strlen(mailbox[mailNumber].date) + strlen(mailbox[mailNumber].subject)));
            send(newsockfd, response, strlen(response), 0);

            // Sending from
            bzero(response, 120);
            strcpy(response, mailbox[mailNumber].from);
            sprintf(response, "%s\r\n", response);
            send(newsockfd, response, strlen(response), 0);

            // Sending to
            bzero(response, 120);
            strcpy(response, mailbox[mailNumber].to);
            sprintf(response, "%s\r\n", response);
            send(newsockfd, response, strlen(response), 0);

            // Sending subject
            bzero(response, 120);
            strcpy(response, mailbox[mailNumber].subject);
            sprintf(response, "%s\r\n", response);
            send(newsockfd, response, strlen(response), 0);

            // Sending date
            bzero(response, 120);
            strcpy(response, mailbox[mailNumber].date);
            sprintf(response, "%s\r\n", response);
            send(newsockfd, response, strlen(response), 0);


            // send the content in loop separated by \r\n
            int cnt  = 0;
            
            while(1){
                int iin = 0;
                while(mailbox[mailNumber].content[cnt] != '\n'){
                    response[iin] = mailbox[mailNumber].content[cnt];
                    iin++;
                    cnt++;
                }
                response[iin] = '\r';
                response[iin + 1] = '\n';
                response[iin + 2] = '\0';
                send(newsockfd, response, strlen(response), 0);
                cnt++;
                if(mailbox[mailNumber].content[cnt] == '\0'){
                    break;
                }
            }

            return 1;

        } else {
            send(newsockfd, "-ERR Mail already deleted\r\n", 28, 0);
            return 0;
        }
    } else {
        send(newsockfd, "-ERR Invalid mail number\r\n", 27, 0);
        return 0;
    }

}

// Function to handle RSET command
void reset_cmd(int newsockfd){

    for(int i = 0;i<MAX_MAILS;i++){
        mailbox[i].markedForDeletion = 0;
    }
    return;
}


// Function to update the mailbox
void mailboxupdate(){
    char filepath[150];
    sprintf(filepath, "%s/mymailbox", username);
    FILE *fp = fopen(filepath, "w");
    if (fp == NULL)
    {
        printf("Error in opening file\n");
        exit(0);
    }
    // Writing the mails to the mailbox
    for (int i = 0; i < numMails; i++)
    {
        if (!mailbox[i].markedForDeletion)
        {
            fprintf(fp, "%s\n", mailbox[i].from);
            fprintf(fp, "%s\n", mailbox[i].to);
            fprintf(fp, "%s\n", mailbox[i].subject);
            fprintf(fp, "%s\n", mailbox[i].date);
            fprintf(fp, "%s", mailbox[i].content);
        }
    }
    fclose(fp);

    free(mailbox);

    return;

}

void SERVER_QUIT(int newsockfd){
    
    mailboxupdate();        // Update the mailbox before quitting

    // Receive QUIT from client
    char buffer[120];
    bzero(buffer, 120);

    
    
    sprintf(buffer, "+OK POP3 server signing off\r\n");
    send(newsockfd, buffer, strlen(buffer), 0);
    printf("Client Disconnected\n");

    close(newsockfd);
    exit(0);

}



int main(int argc, char *argv[])
{   
    int port_no = 20001;                       // Default Port number for POP3 server
    if(argc > 1){
        printf("Port number provided as argument\n");
        port_no = atoi(argv[1]);
    }
    else{
        printf("Port number not provided as argument. Using default port number 20001\n");
    }

    printf("POP3 server started at port %d\n", port_no);

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

    while(1)
    {
        printf("POP3 Server waiting for new client connection:\n");
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // Accepting new client connection
        if(newsockfd < 0){
            printf("Error in accepting connection\n");
            continue;
        }

        printf("Connection accepted from client\n");

        int pid = fork(); // Creating a new process for each client connection
        if(pid < 0){
            printf("Error in creating new process\n");
            exit(0);
        }

        if(pid == 0){ // Child process
            close(sockfd);
            int n;
            char buffer[120];
            bzero(buffer, 120);
            bzero(username, 120);
            bzero(password, 120);

            sprintf(buffer, "+OK POP3 server ready\r\n");       // Sending +OK POP3 server ready
            send(newsockfd, buffer, strlen(buffer), 0);

            
            while(1){


                bzero(buffer, 120);
                // Read until CRLF is encountered
                while (1)
                {   
                    // receiving command from client
                    char buffer1[120];
                    for (int i = 0; i < 120; i++)
                    {
                        buffer1[i] = '\0';
                    }
                    int recvsiz = recv(newsockfd, buffer1, 120, 0);
                    if (recvsiz <= 0)
                    {   
                        
                        perror("Error in receiving\n");
                            mailboxupdate();
                            close(newsockfd);
                            exit(0);
                    }
                    strcat(buffer, buffer1);
                    if (strstr(buffer, "\r") != NULL)
                    {
                        break;
                    }
                }

                printf("Command received from client: %s\n", buffer);
                int Rstatus = 1;
                if(strncmp(buffer, "USER", 4) == 0){ // USER command
                    user_cmd(buffer, newsockfd);
                }
                else if(strncmp(buffer, "PASS", 4) == 0){ // PASS command
                    pass_cmd(buffer, newsockfd);
                }
                else if(strncmp(buffer, "STAT", 4) == 0){ // STAT command
                    stat_cmd(newsockfd);
                }
                else if(strncmp(buffer, "LIST", 4) == 0){ // LIST command
                    list_cmd(buffer, newsockfd);
                }
                else if(strncmp(buffer, "RETR", 4) == 0){ // RETR command
                    retr_cmd(buffer, newsockfd);
                }
                else if(strncmp(buffer, "DELE", 4) == 0){ // DELE command
                    dele_cmd(buffer, newsockfd);
                }
                else if(strncmp(buffer, "RSET", 4) == 0){ // RSET command
                    bzero(buffer, 120);
                    reset_cmd(newsockfd);
                    sprintf(buffer, "+OK RESET\r\n");
                    int nu = send(newsockfd, buffer, strlen(buffer), 0);
                    if(nu < 0){
                        printf("Error in writing to socket\n");
                        mailboxupdate();
                        exit(0);
                    }
                }
                else if(strncmp(buffer, "QUIT", 4) == 0){ // QUIT command
                    SERVER_QUIT(newsockfd);   
                }
                else{  
                    send(newsockfd, "-ERR Invalid command\r\n", 24, 0);
                    if(n < 0){
                        printf("Error in writing to socket\n");
                        mailboxupdate();
                        exit(0);
                    }
                }

                if(Rstatus == 0){
                    SERVER_QUIT(newsockfd);
                }
            }

            mailboxupdate();
            
        }

        else{
            close(newsockfd);
        }


    }


    return 0;
}