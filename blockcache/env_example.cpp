#include <iostream>
#include <vector>
#include <string>

#include "fcgi_stdio.h"
#include <stdlib.h>

//using namespace cgicc;
//using namespace std;
#define BLOCK_SIZE 512
int main(void)
{
    int count = 0;
    while(FCGI_Accept() >= 0) {
        printf("Content-type: text/html\r\n"
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
        char buffer[BLOCK_SIZE];
        while(!feof(stdin)) {
            size_t bytes = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
            fwrite(buffer, bytes, sizeof(char), stdout);
        }
    }

	return 0;
}
