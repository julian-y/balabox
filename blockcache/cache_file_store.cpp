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

#include "http_helper.h"

//using namespace cgicc;
using namespace std;
#define BLOCK_SIZE 512

int httpResponseReader(void *data, const char *buf, size_t len)
{
    string *str = (string *)data;
    str->append(buf, len);
    return 0;
}

//forwards the HTTP request to the actual Block Server
//TODO: change host & query to the correct values when finalized
int storeToBlockServer(string hash, string block) {
    //make request here.
    //string host = "127.0.0.1";
    string query = "/file_store?hash=" + hash;
    ne_session *sess;
    ne_request *req;
    string response;
    
    sess = ne_session_create("http", HttpHelper::block_ip.c_str(), 80);

    req = ne_request_create(sess, "GET", query.c_str());
    ne_set_request_body_buffer(req, block.c_str(), block.length());

    ne_add_response_body_reader(req, ne_accept_always, httpResponseReader, &response);
    int result = ne_request_dispatch(req);
    int status = ne_get_status(req)->code;
    string responseHeader(ne_get_response_header(req, "Content-Type"));

    ne_request_destroy(req);
        
    printf("Content-Type:  %s\r\n\r\n", responseHeader.c_str());
    printf("%s", response.c_str());

    return 0;
}

//TODO: refactor Roger's file_fetch hashBlock #'s so that this code uses it too

int main(void)
{
    int count = 0;
    while(FCGI_Accept() >= 0) {
        // NOTE: The actual request body is piped into stdin:
        
        //code from http://stackoverflow.com/questions/10129085/read-from-stdin-write-to-stdout-in-c
        string response_body;
        char buffer[BLOCK_SIZE];
        while(!feof(stdin)) {
            size_t bytes = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
            response_body += buffer;
            //fwrite(buffer, bytes, sizeof(char), stdout);
        }
        
        string query_string = (getenv("QUERY_STRING"));
        string hash;
        HttpHelper::getQueryParam(query_string, "hash", hash);
        printf("hash: %s\n", hash.c_str());
        storeToBlockServer(hash, response_body);
        
    }

	return 0;
}


