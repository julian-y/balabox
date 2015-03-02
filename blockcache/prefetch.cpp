#include <stdio.h>
// definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/types.h>   
#include <sys/socket.h>  
// definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  
// constants and structures needed for 
// internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>

#include <string>
#include <iostream>
using namespace std;

int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int MSG_SIZE = 1000;

void error(const char* msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    cout << "starting prefetcher" << endl;
    char buffer[MSG_SIZE];
    bzero(buffer, MSG_SIZE);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 8080;//atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

    printf("listening on port 8080\n");
    while (1) {
        int recvlen = recvfrom(sockfd, buffer, MSG_SIZE, 0, 
                    (struct sockaddr *) &cli_addr, &clilen);
        cout << "received a message: " << buffer << endl;
    }
}
