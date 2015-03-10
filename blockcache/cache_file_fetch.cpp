#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <iterator>

#include <neon/ne_request.h>
#include <neon/ne_session.h>

#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>

//socket sending stuff:
#include <strings.h>
#include <stdio.h>
// definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/types.h>   
#include <sys/socket.h>  
// definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>
#include <netdb.h>
// constants and structures needed for 
// internet domain addresses, e.g. sockaddr_in
#include "http_helper.h"
#include "leveldb_helper.hpp"

//using namespace cgicc;
using namespace std;
#define BLOCK_SIZE 512

int const MSG_SIZE = 1000;
string const cache_ip = "1.2.3.4";



//TODO: test this
int addUserCache(string user_id) {
    string path = "/user_cache_add?user_id=" + user_id;
    path += "&cache_ip=" + cache_ip;
    
    string responseHeader;
    string response;
    HttpHelper::sendHttpRequest(HttpHelper::metadata_ip, path, "POST", 
            "", responseHeader, response);
    return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int sendMsg(string msg) {
    
    //Code taken from CS118 project
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(serv_addr);
    //contains tons of information, including the server's IP address
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create a new socket
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname("127.0.0.1"); 
        if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //initialize server's address
    bcopy((char *)server->h_addr, 
            (char *)&serv_addr.sin_addr.s_addr, 
            server->h_length);
    serv_addr.sin_port = htons(HttpHelper::prefetch_portno);

    char buffer[MSG_SIZE];
    bzero(buffer, MSG_SIZE);

    msg.copy(buffer, msg.length());

    if(sendto(sockfd, buffer, MSG_SIZE, 0, 
                    (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0 ) {
            //perror("sendto failed");
            printf("sendto failed");
            return 1;
    }

    return 0;
}

int main(void)
{
    int count = 0;
    // Open the database
    LevelDBHelper db("cachedb");

    while(FCGI_Accept() >= 0) {
        streambuf * cin_streambuf  = cin.rdbuf();
        streambuf * cout_streambuf = cout.rdbuf();
        streambuf * cerr_streambuf = cerr.rdbuf();
        
        #if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
        cin  = &cin_fcgi_streambuf;
        cout = &cout_fcgi_streambuf;
        cerr = &cerr_fcgi_streambuf;
        #else
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        #endif
       
        string query_string(getenv("QUERY_STRING"));
        string hash;
        string userId;
        HttpHelper::getQueryParam(query_string, "hash", hash);
        HttpHelper::getQueryParam(query_string, "user", userId);
        
        //we want to send the prefetch message before requesting the original
        //block so the prefetcher can get a "head start"; espcially since the 
        //request to block-server is blocking.

        //send message to another process running on cache server to prefetch
        string msg = "{\"userID\": \"" + userId + "\", \"hash\": \"" + hash + "\" }"; 
        sendMsg(msg);
        
        //add (to metadata) the association of this cache to the user we're serving 

        //if (db.alreadyExists(hash)) {
        //    string data;
        //    db.get(hash, data);
        //    printf("Content-Type: application/binary\r\n\r\n");
        //    printf("%s", data.c_str());
        //    return 0;  
        //}  
        //else {
            string response;
            string responseContentType;
            HttpHelper::requestFromBlockServer(hash, responseContentType, response);
            printf("Content-Type:  %s\r\n\r\n", responseContentType.c_str());
            //printf("%s", response.c_str());
            cout.write(response.c_str(), response.length());
            
            //char* response_c_str = (char*) malloc(response.length());
            //memcpy(response_c_str, response.c_str(), response.length());
            ////fwrite(response_c_str, sizeof(char), response.length(), stdin);
            //free(response_c_str);
        //}
        
    }

	return 0;
}


