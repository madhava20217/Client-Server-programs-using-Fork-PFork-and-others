#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_CLIENTS 10              //maximum clients that can be accommodated at once
#define STR_SIZE 32                 //max length of string
#define HOST "127.0.0.1"            //defining host IP address
#define PORT 1024                   //defining port number
#define zero_out(structure) memset(&structure, 0, sizeof(structure))    // MACRO FOR ZEROING

long long factorial(long long n){
    /// @brief Function for getting factorial
    /// @param n : integer for which factorial is to be calculated
    /// @return factorial of the number n
    long long ret = 1;
    for(int i= 1; i <= n; i++){
        ret*=i;
    }
    return ret;
}

void read_write_to_client(int fd, FILE* fptr, struct sockaddr_in* client){
    char str[STR_SIZE];
    while(1){
        //check for msg from client
        char str[STR_SIZE];
        memset(str, 0, STR_SIZE);
        read(fd, str, STR_SIZE);            //blocking call!

        //printf("MESSAGE FROM CLIENT: %s\n", str);

        int num = atoi(str);
        if(num == -1) {
            //exit condition
            break;
        }
        fprintf(fptr, "%s:%d,%d,%lld\n", 
            inet_ntoa(client->sin_addr),
            client->sin_port,
            num,
            factorial(num)
            );
        sync();
    }
    printf("Received messages from client %s:%d, printed to OUTPUT_PAR_FORK.csv.\nExiting...\n", 
            inet_ntoa(client->sin_addr),
            client->sin_port);
}

void print_header(FILE* fptr){
    fprintf(fptr, "Client,i,Factorial\n");
    sync();
}

int main(){
    int sockfd = 0;
    //creating a TCP socket with IP protocol
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket could not be created\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    // Opening file for printing
    FILE* fptr = fopen("../OUTPUT_PAR_FORK.csv", "w+");
    print_header(fptr);
    fflush(NULL);
    //printf("Socket FD is: %d\n", sockfd);
    

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

    struct sockaddr_in client;
    int n_bytes_client = sizeof(client);

    int connected = 0;
    while(1){
        
        int connect = accept(sockfd, (struct sockaddr*) &client, &n_bytes_client);
        if(connect < 0){
            printf("Couldn't connect");
            exit(EXIT_FAILURE);
        }
        connected++;
        pid_t forking = fork();

        if(forking == 0){
            //child process
            close(sockfd);
            //READ AND WRITE STUFF
            read_write_to_client(connect, fptr, &client);
            close(connect);
            break;
        }
        printf("CONNECTED : %d\n", connected);
        if(connected == MAX_CLIENTS) exit(0);
    }

    return 0;

    

    


}
