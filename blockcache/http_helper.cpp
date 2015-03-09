#include "http_helper.h"
#include <string>

#include <neon/ne_request.h>
#include <neon/ne_session.h>

using namespace std;

const string  HttpHelper::metadata_ip = "104.236.169.138";
const string  HttpHelper::block_ip = "104.236.169.138";
const int     HttpHelper::prefetch_portno = 8888;

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
int HttpHelper::requestFromBlockServer(string hash, string &responseContentType, string &response) {
    //make request here.
    string path = "/file_fetch?hash=" + hash;

    HttpHelper::sendHttpRequest(HttpHelper::block_ip, path, "GET", "", responseContentType, 
            response);

    return 0;
}


int HttpHelper::httpResponseReader(void *data, const char *buf, size_t len)
{
    string *str = (string *)data;
    str->append(buf, len);
    return 0;
}

int HttpHelper::sendHttpRequest(string host_ip, string path, string reqType, 
        string reqBody, string &responseContentType, string &response) {

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
    
    responseContentType = ne_get_response_header(req, "Content-Type");

    ne_request_destroy(req);
    ne_session_destroy(sess);
    return 0;
}


