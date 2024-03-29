#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

static time_t start;

#define QUEUE  20                //QUEUE UP CLIENTS
#define MAX_CLIENTS QUEUE              //maximum clients that can be accommodated at once
#define STR_SIZE 32                 //max length of string
#define HOST "127.0.0.1"            //defining host IP address
#define PORT 1024                   //defining port number
#define zero_out(structure) memset(&structure, 0, sizeof(structure))    // MACRO FOR ZEROING

long long factorial(long long n);
void read_write_to_client(int fd, FILE* fptr, struct sockaddr_in* client);
void* serv_functions(void* args);

// sockfd is the socket file descriptor, file_ptr is the pointer to the open file
struct thread_data{
    int sockfd;
    FILE* file_ptr;
    int client_no;
};


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
    printf("Received messages from client %s:%d, printed to OUTPUT_PAR_THREAD.csv.\nExiting...\n", 
            inet_ntoa(client->sin_addr),
            client->sin_port);
}

void* serv_functions(void* args){

    struct thread_data* data = (struct thread_data*) args;
    int sockfd = data->sockfd;
    FILE* fptr = data->file_ptr;
    int client_no = data->client_no;
    if(client_no == 0){
        start = clock();
    }

    struct sockaddr_in client;
    int n_bytes_client = sizeof(client);
    int connect = accept(sockfd, (struct sockaddr*) &client, &n_bytes_client);
    if(connect < 0){
        printf("Couldn't connect");
        exit(EXIT_FAILURE);
    }
    read_write_to_client(connect, fptr, &client);
    close(connect);
}

int main(){
    int sockfd = 0;
    //creating a TCP socket with IP protocol
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket could not be created\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    //printf("Socket FD is: %d\n", sockfd);
    printf("PID IS: %d\n", getpid());

    struct sockaddr_in sock_addr;

    zero_out(sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port   = htons(PORT);
    sock_addr.sin_addr.s_addr = inet_addr(HOST);  //htons(INADDR_ANY);      //server can use any IP address which the local machine uses

    // Opening file for printing
    //printf("CONNECTED!!!\n");
    FILE* fptr = fopen("../../OUTPUT_PAR_THREAD.csv", "w+");
    fprintf(fptr, "Client,i,Factorial\n");
    sync();

    //binding socket to IP
    if((bind(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr))) != 0){
        printf("Couldn't bind socked FD to IP address.\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, QUEUE) != 0){
        printf("Couldn't listen");
        exit(EXIT_FAILURE);
    }


    pthread_t threads[MAX_CLIENTS];
    for(int i = 0; i < MAX_CLIENTS;i++){
        struct thread_data d = {sockfd, fptr, i};
        pthread_create(threads+i, NULL, serv_functions, &d);
    }

    for(int i = 0; i < MAX_CLIENTS;i++){
        pthread_join(threads[i], NULL);
    }

    time_t end = clock();

    printf("\n\nEXECUTION TIME : %.9f\n\n", ((double)end - start)/CLOCKS_PER_SEC);

    return 0;

    

    


}
