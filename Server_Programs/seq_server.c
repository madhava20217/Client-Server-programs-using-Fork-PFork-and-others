#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#define HOST "127.0.0.1"            //defining host IP address
#define PORT 1024
#define zero_out(structure) memset(&structure, 0, sizeof(structure))    // MACRO FOR ZEROING

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
    sock_addr.sin_addr.s_addr = htons(INADDR_ANY);      //server can use any IP address which the local machine uses

    //binding socket to IP
    if((bind(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr))) != 0){
        printf("Couldn't bind socked FD to IP address.\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, 20) != 0){
        printf("Couldn't listen");
        exit(EXIT_FAILURE);
    }

    int n_bytes_client = 0;
    struct sockaddr_in client;
    int connect = accept(sockfd, (struct sockaddr*) &client, &n_bytes_client);
    if(connect < 0){
        printf("Couldn't connect");
        exit(EXIT_FAILURE);
    }

    printf("CONNECTED!!!\n");

    //TODO: READ AND WRITE STUFF
}
