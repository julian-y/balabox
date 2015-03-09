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

#include <jsoncpp/json/json.h>
#include <neon/ne_request.h>
#include <neon/ne_session.h>

#include "http_helper.h"
using namespace std;

const string maxHashes = "10";

int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int MSG_SIZE = 1000;

void error(const char* msg)
{
    perror(msg);
    exit(0);
}

int httpResponseReader(void *data, const char *buf, size_t len)
{
    string *str = (string *)data;
    str->append(buf, len);
    return 0;
}

string lookUpRecentHashes(string userID) {
    //make request here.
    string query = "/recent_hashes.fcgid?user_id=" + userID;
    query += "&max_hashes=" + maxHashes;

    ne_session *sess;
    ne_request *req;
    string response;
    
    sess = ne_session_create("http", HttpHelper::metadata_ip.c_str(), 80);

    req = ne_request_create(sess, "GET", query.c_str());
    //ne_set_request_body_buffer(req, block.c_str(), block.length());

    ne_add_response_body_reader(req, ne_accept_always, httpResponseReader, &response);
    int result = ne_request_dispatch(req);
    cout << "result: " << result << endl;
    if(result) {
        printf("Request failed: %s\n", ne_get_error(sess));
        error("Error making request during lookUpRecentHashes()!");
    }
    
    int status = ne_get_status(req)->code;

    //string responseHeader(ne_get_response_header(req, "Content-Type"));

    ne_request_destroy(req);
    
    cout << "Requesting recent hashes from metadata server @ " << HttpHelper::metadata_ip << ": " << endl;
    //printf("Content-Type:  %s\r\n\r\n", responseHeader.c_str());
    cout << response << endl;
    return response;
}

void errorParsing(string json) {
    string errorMsg = "Could not parse json!\n";
    errorMsg += json;
    error("Could not parse received message!"); 
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
        //cout << "received a message: " << buffer << endl;
        
        string msg(buffer);
        //parse json
        Json::Value msg_root;
        Json::Reader msg_reader;
        bool parsedSuccess = msg_reader.parse(msg, msg_root, false);
        if(!parsedSuccess) {
            errorParsing(msg);    
        }
        
        string userID = msg_root.get("userID", "-1").asString();
        string hash = msg_root.get("hash", "-1").asString();
        cout << "------Incoming Request------" << endl;
        cout << "userID: " << userID << endl;
        cout << "hash: " << hash << endl;
        //cout << "msg: " << msg << endl;
        
        cout << "call lookUpRecentHashes() here" << endl;
        // make request to metadata server
        string recent_hashes_json = lookUpRecentHashes(userID);
        
        //parse json
        Json::Value recent_hashes_root;
        Json::Reader recent_hashes_reader;
        parsedSuccess = recent_hashes_reader.parse(recent_hashes_json, recent_hashes_root, false);
        if(!parsedSuccess) {
            errorParsing(recent_hashes_json);
        }

        string block_list = recent_hashes_root.get("block_list", "").asString();
        cout << "------Recent Hashes------" << endl;
        cout << "block_list: " << block_list << endl;
        if(block_list.empty()) {
            cout << "block_list is empty, don't need to fetch stuff" << endl;
        } else {
            //TODO:
            //parse array 
            //add logic to choose which hashes to pick
            //make requests to block server
            //save blocks
        }
    }
}
