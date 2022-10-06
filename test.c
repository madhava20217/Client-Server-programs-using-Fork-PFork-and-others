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

int main(){
    struct in_addr test;
    inet_pton(AF_INET, HOST, &test);

    printf("%s", inet_ntoa(test));
    
}