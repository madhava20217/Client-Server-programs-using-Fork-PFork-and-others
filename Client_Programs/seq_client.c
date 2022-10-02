#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#incldue <inet

#define HOST "127.0.0.1"            //defining host IP address

int main(){
    int sockfd = 0;
    //creating a TCP socket with IP protocol
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket could not be created\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket FD is: %d\n", sockfd);
    

    struct sockaddr


}
