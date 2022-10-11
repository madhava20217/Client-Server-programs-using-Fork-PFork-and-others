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
#include <sys/epoll.h>
#include <time.h>
#include <fcntl.h>

#define QUEUE 200                   //QUEUE up
#define LIMIT 20                   //limit for factorial
#define MAX_CLIENTS QUEUE            //maximum clients that can be accommodated at once
#define STR_SIZE 32                 //max length of string
#define HOST "127.0.0.1"            //defining host IP address
#define PORT 1024                   //defining port number
#define zero_out(structure) memset(&structure, 0, sizeof(structure))    // MACRO FOR ZEROING

int done = 0;                       //tracks done clients, for time tracking

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
    //check for msg from client
    char str[STR_SIZE];
    memset(str, 0, STR_SIZE);
    read(fd, str, STR_SIZE);            //blocking call!

    // usleep(3000);
    int num = atoi(str);
    if(num <= 0) return;
    if(num == LIMIT) {
        done++;               //increment done by 1;
        // close(fd);
    }
    fprintf(fptr, "%s:%d,%d,%lld\n", 
        inet_ntoa(client->sin_addr),
        client->sin_port,
        num,
        factorial(num)
        );
    sync();
    // printf("Received messages from client %s:%d, printed to OUTPUT_PAR_THREAD.csv.\nExiting...\n", 
    //         inet_ntoa(client->sin_addr),
    //         client->sin_port);
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
    printf("PID IS : %d\n\n", getpid());


    struct sockaddr_in sock_addr;

    zero_out(sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port   = htons(PORT);
    sock_addr.sin_addr.s_addr = htons(INADDR_ANY);      //server can use any IP address which the local machine uses

    // Opening file for printing
    //printf("CONNECTED!!!\n");
    FILE* fptr;
    fptr = fopen("../../OUTPUT_EPOLL.csv", "w+");
    fprintf(fptr, "Client,i,Factorial\n");
    sync();
    //fclose(fptr);

    //binding socket to IP
    if((bind(sockfd, (struct sockaddr*) &sock_addr, sizeof(sock_addr))) != 0){
        printf("Couldn't bind socked FD to IP address.\n");
        exit(EXIT_FAILURE);
    }

    // 20 connection requests can be queued, the rest will be dropped
    if(listen(sockfd, QUEUE) != 0){
        printf("Couldn't listen");
        exit(EXIT_FAILURE);
    }

    //create epoll instance
    int epoll_fd = epoll_create1(0);
    if(epoll_fd < 0){
        //error
        printf("ERROR WITH EPOLL! Exiting...\n");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ep_ev, events[2*MAX_CLIENTS];        //for epoll, structures
    ep_ev.events = EPOLLIN;
    ep_ev.data.fd = sockfd;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ep_ev) < 0){
        //error
        printf("Error adding main socket to epoll file descriptor\n");
        exit(EXIT_FAILURE);
    }



    struct sockaddr_in clients[MAX_CLIENTS];            //array of connections
    int client_fd_map[MAX_CLIENTS];                     //array of client to fd mapping
    memset(client_fd_map, -1, sizeof(client_fd_map));   //init to 0

    int TIMEOUT = 1000*1000;                              //timeout initialised to 60 seconds

    int num_clients = 0;

    time_t start;
    while(1){

        //fptr = fopen("../../OUTPUT_EPOLL.csv", "a");   //open fptr for sync

        int num_fds;

        if((num_fds = epoll_wait(epoll_fd, events, 2*MAX_CLIENTS, TIMEOUT)) == -1){
            //error here
            printf("Error waiting for event or timed out\n");
            exit(EXIT_FAILURE);
        }

        //iterate over set of fds
        for(int i = 0; i < num_fds; i++){
            if(events[i].data.fd == sockfd){
                //printf("CONNECTION\n");
                //main socket, accept client
                struct sockaddr_in client;
                int client_size = sizeof(client);
                int client_socket = accept(sockfd, (struct sockaddr*) &client, &client_size);

                printf("CONNECTED %d, DONE: %d\n", num_clients, done);
                if(client_socket < 0){
                    perror(strerror(errno));
                    exit(EXIT_FAILURE);
                }

                struct epoll_event ev;
                // struct thread_data info;
                // info.client = client;
                // info.sockfd = client_socket;
                // info.file_ptr = NULL;
                
                //setnonblocking:
                //fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0)|O_NONBLOCK);


                ev.events = EPOLLIN;
                ev.data.fd = client_socket;
                    //add it to the polling set
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1){
                    //error
                    printf("Error adding client to poll. Exiting\n");
                    exit(EXIT_FAILURE);
                }
                if(num_clients == 0){
                    start = clock();
                }
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
                    if(client_fd_map[j] == events[i].data.fd){
                        client = clients[j];
                        break;
                    }
                }

                //read and write time
                struct thread_data data = {events[i].data.fd, fptr, client};
                serv_functions(&data);
            }
        }
        
        //fclose(fptr);

        if(done == MAX_CLIENTS) break;
    }

    fclose(fptr);

    sync();

    time_t end = clock();

    printf("\n\nEXECUTION TIME : %.9f\n\n", ((double)end - start)/CLOCKS_PER_SEC);


    return 0;

    

    


}
