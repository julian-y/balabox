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
#include "leveldb_helper.hpp"
#include <vector>

using namespace std;

const string maxHashes = "10";
int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int PREFETCH_MSG_SIZE = 1000;

void error(const char* msg)
{
    perror(msg);
    //exit(0);
}

int httpResponseReader(void *data, const char *buf, size_t len)
{
    string *str = (string *)data;
    str->append(buf, len);
    return 0;
}

string lookUpRecentHashes(string userID) {
    //make request here.
    string query = "/recent_hashes?user_id=" + userID;
    query += "&max_hashes=" + maxHashes;

    ne_session *sess;
    ne_request *req;
    string response;
    
    sess = ne_session_create("http", HttpHelper::metadata_ip.c_str(), 80);

    req = ne_request_create(sess, "GET", query.c_str());
    //ne_set_request_body_buffer(req, block.c_str(), block.length());

    ne_add_response_body_reader(req, ne_accept_always, httpResponseReader, &response);
    int result = ne_request_dispatch(req);
    if(result) {
        printf("Request failed: %s\n", ne_get_error(sess));
        error("Error making request during lookUpRecentHashes()!");
    }
    
    int status = ne_get_status(req)->code;

    //string responseHeader(ne_get_response_header(req, "Content-Type"));

    ne_request_destroy(req);
    
    cout << "Requesting recent hashes from metadata server @ " << HttpHelper::metadata_ip << ": " << endl;
    //printf("Content-Type:  %s\r\n\r\n", responseHeader.c_str());
    //cout << response << endl;
    return response;
}

void errorParsing(string json) {
    string errorMsg = "Could not parse json!\n";
    errorMsg += json;
    error("Could not parse received message!"); 
}

//simple pickHashes function (just 
vector<string> pickHashes(Json::Value recent_hashes, LevelDBHelper* db) {
    vector<string> hashes;
    for(int i = 0; i < recent_hashes.size(); i++) {
        string curHash = recent_hashes[i].asString();
        
        // if curHash already exists in leveldb, skip it
        if(!db->alreadyExists(curHash)) {
          hashes.push_back(curHash);
            cout << "push hash " << curHash << " into pull vector" << endl;
        } else {
            cout << "cacheDB already has hash " << curHash << endl;
        }
    }
   
    return hashes;
}

int main(int argc, char *argv[]) {
    cout << "starting prefetcher" << endl;
    
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(HttpHelper::prefetch_portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

    if (listen(sockfd, 5) < 0) {
        HttpHelper::error("Error on listening");
    }

    // Open the database    
    LevelDBHelper* db = new LevelDBHelper();

    char buffer[PREFETCH_MSG_SIZE];
    bzero(buffer, PREFETCH_MSG_SIZE);
    while (1) {
        bzero(buffer, PREFETCH_MSG_SIZE);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            HttpHelper::error("Error on Accept");
            exit(1);
        }
        
        int recvlen = recv(newsockfd, buffer, PREFETCH_MSG_SIZE, 0);

        string msg = string(buffer);


        //parse json
        Json::Value msg_root;
        Json::Reader msg_reader;
        bool parsedSuccess = msg_reader.parse(msg, msg_root, false);
        if(!parsedSuccess) {
            continue;
            //errorParsing(msg);    
        }
        
        string userID = msg_root.get("userID", "-1").asString();
        string hash = msg_root.get("hash", "-1").asString();
        cout << "------Incoming Request------" << endl;
        cout << "userID: " << userID << endl;
        cout << "requested hash: " << hash << endl;
        //cout << "msg: " << msg << endl;
        
        // make request to metadata server
        string recent_hashes_json = lookUpRecentHashes(userID);
        
        //parse json
        Json::Value recent_hashes_root;
        Json::Reader recent_hashes_reader;
        parsedSuccess = recent_hashes_reader.parse(recent_hashes_json, recent_hashes_root, false);
        if(!parsedSuccess) {
            //errorParsing(recent_hashes_json);
            continue;
        }

        Json::Value block_list = recent_hashes_root.get("block_list", "");//.asString();
        cout << ">>>Recent Hashes" << endl;
        cout << "block_list: " << block_list << endl;
        if(block_list.isNull()) {
            cout << "block_list is empty, don't need to fetch stuff" << endl;
        } else {

            vector<string> hashesToRetrieve = pickHashes(block_list, NULL);
            cout << "There are " << hashesToRetrieve.size() << " hashes to retrieve" << endl;    
            for(int i = 0; i < hashesToRetrieve.size(); i++) {
                string curHash = hashesToRetrieve[i];
                string responseContentType;
                string block;
                string responseCode;
                cout << "Fetching hash " << curHash << "..." << endl;
                HttpHelper::requestFromBlockServer(curHash, responseContentType, block, responseCode);
                if(responseCode.compare("200") == 0) {
                    cout << "fetched hash " << curHash << endl;
                    //cout << "block: " << endl << block << endl << "---" << endl;
                    int status = db->put(curHash, block);
                    
                    if (status != 0) {
                        cout << "Error inserting into database" << endl;
                    }
                    cout << "stored hash " << curHash << " into cacheDB" << endl;
                } else {
                    cout << "failed to fetch hash " << curHash << endl;
                }
            }
                
        }
        cout << "------Finished Processing Request------" << endl;

    }
    delete db;
}
