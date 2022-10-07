#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#define STR_SIZE 32                 //max length of string
#define HOST "127.0.0.1"            //defining host IP address
#define PORT 1024
#define zero_out(structure) memset(&structure, 0, sizeof(structure))        //MACRO FOR ZEROING

void read_write_to_server(int fd){
    for(int i = 1; i <= 20; i++){
        char str[STR_SIZE];
        memset(str, 0, STR_SIZE);
        sprintf(str, "%d", i);
        write(fd, str, sizeof(char)*STR_SIZE);
        printf("WRITTEN TO SERVER, val = %d\n", i);
        
    }
    //exit message
    char str[STR_SIZE];
    memset(str, 0, STR_SIZE);
    sprintf(str, "%d", -1);
    write(fd, str, sizeof(char)*STR_SIZE);
}

int main(){
    int sockfd = 0;
    //creating a TCP socket with IP protocol
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket could not be created\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket FD is: %d\n", sockfd);
    

    struct sockaddr_in sock_addr;

    zero_out(sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port   = htons(PORT);
    sock_addr.sin_addr.s_addr = inet_addr(HOST);

    int ERR;
    if((ERR = connect(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr))) != 0){
        printf("\n\nConnection with server failed\n");
        printf("ERROR CODE: %d,\nDescription: %s\n", ERR, strerror(errno));

        exit(EXIT_FAILURE);
    }
    else{
        printf("Connected to the server\n");
    }

    read_write_to_server(sockfd);
    
    close(sockfd);
}
