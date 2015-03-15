#include "http_helper.h"
#include <string>
#include <cstring>
#include <neon/ne_request.h>
#include <neon/ne_session.h>
#include <cstdlib>
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
#include <unistd.h>
#include <iostream>

using namespace std;

int const MSG_SIZE = 5000;

const string  HttpHelper::metadata_ip = "104.236.169.138";
const string  HttpHelper::block_ip = "104.236.169.138";
const int     HttpHelper::prefetch_portno = 8888;
const int     HttpHelper::leveldb_portno = 8889;

int HttpHelper::getQueryParam(const std::string& query_string, 
        const std::string& param, std::string& value) {

	// Verify that the parameter required is found
	int paramPos = query_string.find(param);
    
	if (paramPos == string::npos) {
		return 1;
	}

	int valuePos = query_string.find("=", paramPos) + 1;
    int nextParam = query_string.find("&", valuePos);
    
    //if there is another parameter after the one we're searching for
    if (nextParam == string::npos) {
	    value = query_string.substr(valuePos);    
    } else {
        value = query_string.substr(valuePos, nextParam - valuePos);

    }

	return 0;
}

//forwards the HTTP request to the actual Block Server
int HttpHelper::requestFromBlockServer(string hash, string &responseContentType, string &response, std::string &responseCode) {
    //make request here.
    string path = "/file_fetch?hash=" + hash;

    HttpHelper::sendHttpRequest(HttpHelper::block_ip, path, "GET", "", responseContentType, 
            response, responseCode);

    return 0;
}


int HttpHelper::httpResponseReader(void *data, const char *buf, size_t len)
{
    string *str = (string *)data;
    str->append(buf, len);
    return 0;
}

int HttpHelper::sendHttpRequest(string host_ip, string path, string reqType, 
        string reqBody, string &responseContentType, string &response, string &responseCode) {

    ne_session *sess;
    ne_request *req;

    sess = ne_session_create("http", host_ip.c_str(), 80);
    req = ne_request_create(sess, reqType.c_str(), path.c_str());
    
    if(reqBody.length() > 0) {
        ne_set_request_body_buffer(req, reqBody.c_str(), reqBody.length());
    }

    ne_add_response_body_reader(req, ne_accept_always, HttpHelper::httpResponseReader, &response);
    int result = ne_request_dispatch(req);
    if(result) {
        printf("Request failed: %s\n", ne_get_error(sess));
        return -1;
    }
    int status = ne_get_status(req)->code;
    responseCode = to_string(status);

    responseContentType = ne_get_response_header(req, "Content-Type");

    ne_request_destroy(req);
    ne_session_destroy(sess);
    return 0;
}

void HttpHelper::error(const char *msg)
{
    perror(msg);
    exit(0);
}

int HttpHelper::sendLocalMsg(string msg, string &resp, int portno, bool getResp) {
    //Code taken from CS118 project
    int sockfd, newsockfd;
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
    serv_addr.sin_port = htons(portno);

    char buffer[MSG_SIZE];
    bzero(buffer, MSG_SIZE);
    HttpHelper::createBuffer(msg.c_str(), msg.length(), buffer);
    //msg.copy(buffer, msg.length());

    if(sendto(sockfd, buffer, MSG_SIZE, 0, 
                    (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0 ) {
            //perror("sendto failed");
            printf("sendto failed");
            return 1;
    }
    
    bzero(buffer, MSG_SIZE);

    if(getResp) {
        printf("Waiting for response!\n");
        int bytesRcvd = recvfrom(sockfd, buffer, MSG_SIZE, 0, 
                    (struct sockaddr *)&serv_addr, &addrlen);

        while (bytesRcvd < 0) {
                bytesRcvd = recvfrom(sockfd, buffer, MSG_SIZE, 0, 
                   (struct sockaddr *)&serv_addr, &addrlen);
            }
        
        char data[MSG_SIZE];
        int dataSize = 0;
        HttpHelper::extractBuffer(buffer, data, dataSize);
        resp = string(data, dataSize);
        cout << "response received: " << resp << endl;
    }

    return 0;

}

// create a buffer for sending
// char* data is already initialized with the data
void HttpHelper::createBuffer(const char* data, int dataSize, char* buffer) {
    //buffer = (char*) malloc(MSG_SIZE);
    int* intBuffer = (int*) buffer;
    *(intBuffer) = dataSize;
    memcpy((intBuffer + 1), data, dataSize);
}
 
// "unparse" returns the current packet in raw char* form
// char* buffer is already initialized with the message
void HttpHelper::extractBuffer(char* buffer, char* data, int &dataSize) {
    //data = (char*) malloc(MSG_SIZE);
    int* intBuffer = (int*) buffer;
    dataSize = *(intBuffer);
    memcpy(data, (intBuffer + 1), dataSize);
}

