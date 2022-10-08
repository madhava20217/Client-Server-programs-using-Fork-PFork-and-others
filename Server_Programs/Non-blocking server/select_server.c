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
#include <sys/select.h>             //for select syscall

#define MAX_CLIENTS 10              //maximum clients that can be accommodated at once
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
    struct sockaddr_in client;
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
    struct sockaddr_in client = data->client;

    read_write_to_client(sockfd, fptr, &client);
}

int main(){
    int sockfd = 0;
    //creating a TCP socket with IP protocol
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Socket could not be created\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    //printf("Socket FD is: %d\n", sockfd);
    

    struct sockaddr_in sock_addr;

    zero_out(sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port   = htons(PORT);
    sock_addr.sin_addr.s_addr = htons(INADDR_ANY);      //server can use any IP address which the local machine uses

    // Opening file for printing
    //printf("CONNECTED!!!\n");
    FILE* fptr;
    fptr = fopen("../../OUTPUT_SELECT.csv", "w+");
    fprintf(fptr, "Client,i,Factorial\n");
    sync();
    fclose(fptr);

    //binding socket to IP
    if((bind(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr))) != 0){
        printf("Couldn't bind socked FD to IP address.\n");
        exit(EXIT_FAILURE);
    }

    // 20 connection requests can be queued, the rest will be dropped
    if(listen(sockfd, 20) != 0){
        printf("Couldn't listen");
        exit(EXIT_FAILURE);
    }

    fd_set current_socs, prev_socs;
    FD_SET(sockfd, &prev_socs);

    struct sockaddr_in clients[MAX_CLIENTS];            //array of connections
    int client_fd_map[MAX_CLIENTS];                     //array of client to fd mapping
    memset(client_fd_map, -1, sizeof(client_fd_map));   //init to 0


    int num_clients = 0;
    while(1){
        //current to prev update
        current_socs = prev_socs;

        //select syscall to know what changed
        if(select(2*MAX_CLIENTS, &current_socs, NULL, NULL, NULL) < 0){
            //error!
            perror(strerror(errno));
            exit(EXIT_FAILURE);
        }

        //iterate over set of fds
        for(int i = 0; i < 2*MAX_CLIENTS; i++){
            if(FD_ISSET(i, &current_socs)){
                if(i == sockfd){
                    struct sockaddr_in client;
                    int client_size = sizeof(client);
                    int client_socket = accept(sockfd, (struct sockaddr*) &client, &client_size);

                    //add to array
                    clients[num_clients]       = client;
                    client_fd_map[num_clients] =  client_socket;
                    num_clients++;
                }
                else{
                    //it's some other socket, which means we need to read/write stuff

                    //finding client info
                    struct sockaddr_in client;
                    for(int j = 0; j < MAX_CLIENTS; j++){
                        if(client_fd_map[j] == i){
                            client = clients[j];
                            break;
                        }
                    }

                    //read and write time
                    struct thread_data data = {i, fptr, client};
                    serv_functions(&data);
                }
            }
        }

        // struct sockaddr_in client;
        // int client_size = sizeof(client);
        // int client_socket = accept(sockfd, (struct sockaddr*) &client, &client_size);

 
        // struct thread_data data = {client_socket, fptr, client};
        // serv_functions(&data);
    }

    sync();

    return 0;

    

    


}
