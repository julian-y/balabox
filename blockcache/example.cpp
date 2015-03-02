#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <iterator>

#include "fcgi_stdio.h"
#include <jsoncpp/json/json.h>

//using namespace cgicc;
using namespace std;
#define BLOCK_SIZE 512
int main(void)
{
    int count = 0;
    while(FCGI_Accept() >= 0) {
        printf("Content-Type: text/html\r\n"
               "\r\n"
               "<title>FastCGI Hello!</title>"
               "<h1>FastCGI Hello!</h1>"
               "<p> this is a test! </p>"
               "<p>Request number %d running on host <i>%s</i>\n</p>",
                ++count, getenv("SERVER_NAME"));
        
        // This example gets the "REQUEST_METHOD" (GET vs POST) and Query_String
        // from the server:
        printf("<p>This is a %s request</p>", getenv("REQUEST_METHOD"));
        //print out query_string
        printf("<p>Query string: \"%s\"</p>", getenv("QUERY_STRING"));
        printf("<p>This is the request body: </p>");

        // NOTE: The actual request body is piped into stdin:
        
        //code from http://stackoverflow.com/questions/10129085/read-from-stdin-write-to-stdout-in-c
        string response_body;
        char buffer[BLOCK_SIZE];
        while(!feof(stdin)) {
            size_t bytes = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
            response_body += buffer;
            //fwrite(buffer, bytes, sizeof(char), stdout);
        }

        printf("%s\n", response_body.c_str());

        Json::Value root;
        Json::Reader reader;

        bool parsedSuccess = reader.parse(response_body, root, false);
        printf("Correctly parsed: %d\n", parsedSuccess);
        printf("Json pretty print:\n%s\n ", root.toStyledString().c_str());
        
        //for some reason.. cout doesn't work??
        
        
        
    }

	return 0;
}
