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

#include "fcgio.h"
#include "fcgi_config.h"
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
#include <unistd.h>

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
    string responseCode;
    HttpHelper::sendHttpRequest(HttpHelper::metadata_ip, path, "POST", 
            "", responseHeader, response, responseCode);
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

void outputErrorMessage(const string& error) 
{
     cout << "Status: 400\r\n"
          <<  "Content-type: text/html\r\n"
          <<  "\r\n"
          << "<html><p>400 " << error << "</p></html>\n";
}

int main(void)
{
    int count = 0;
    
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    
    while (FCGX_Accept_r(&request) == 0) {

        // Note that the default bufsize (0) will cause the use of iostream
        // methods that require positioning (such as peek(), seek(),
        // unget() and putback()) to fail (in favour of more efficient IO).
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);
        
        #if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
        cin  = &cin_fcgi_streambuf;
        cout = &cout_fcgi_streambuf;
        cerr = &cerr_fcgi_streambuf;
        #else
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        #endif
       
        char* query_string = FCGX_GetParam("QUERY_STRING", request.envp);
        string errorMsg = "Invalid Input";

        // Invalid inputs
        if (query_string == nullptr) {
        	outputErrorMessage("No parameters");
        	continue;
        } 

        string param = query_string;
        string hash;
        string userId;
        HttpHelper::getQueryParam(query_string, "hash", hash);
        HttpHelper::getQueryParam(query_string, "user", userId);
        
        //add (to metadata) the association of this cache to the user we're serving 
        LevelDBHelper* db = new LevelDBHelper();
	        
//       if (db->alreadyExists(hash)) {
//            string data;
//            db->get(hash, data);
//            cout << "Status: 200\r\n";
//            cout << "Origin: Cache Server\r\n";
//            cout << "Content-Type: application/binary\r\n\r\n";
//            cout.write(data.data(), data.size());
//        }  
//        else {
            string response;
            string responseContentType;
            string responseCode;
            HttpHelper::requestFromBlockServer(hash, responseContentType, response, responseCode);
            cout << "Status: " << responseCode << "\r\n";
            cout << "Origin: Block Server\r\n";
            cout << "Content-Type: " << responseContentType << "\r\n\r\n";
            cout.write(response.data(), response.size());

            //save the response into cache
           db->put(hash, response);
//        }

        //we want to send the prefetch message after requesting the original
        //block so that cache_file_fetch can return as possible (leveldb_server
        //uses a queue to serve requests)

        //send message to another process running on cache server to prefetch
        string msg = "{\"userID\": \"" + userId + "\", \"hash\": \"" + hash + "\" }"; 
        string resp;
        HttpHelper::sendLocalMsg(msg, resp, HttpHelper::prefetch_portno, false);
        
        delete db;
}

#if HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
    cin  = cin_streambuf;
    cout = cout_streambuf;
    cerr = cerr_streambuf;
#else
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
#endif

	return 0;
}


