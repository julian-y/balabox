#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <string>

class HttpHelper {
public:
    //query_string: the entire query string passed into the fcgid process
    //param: the param we want to look for
    //value: the value of the associated parameter
    static int getQueryParam(const std::string& query_string, 
            const std::string& param, std::string& value);


    static int sendHttpRequest(std::string host_ip, std::string path, 
            std::string reqType, std::string reqBody, std::string &responseContentType, 
            std::string &response, std::string &responseCode);

    //requests a block from the blockserver
    static int requestFromBlockServer(std::string hash, std::string &responseContentType, std::string &response, std::string &responseCode);

    //sends a local message using UDP sockets (ip 127.0.0.1) 
    //will automatically set up the sockets and choose to receive a single
    //response message if getResp = true.  This response message is placed
    //inside of std::string &resp
    static int sendLocalMsg(std::string msg, std::string &resp, int portno, bool getResp);

    //Handles buffer sending for variable data.  This is because UDP sockets
    //requires us to specify an exact number of data to send / receive.  We
    //choose to set this size with MSG_SIZE.  

    //creates a "buffer" formatted from char* data and int dataSize, this buffer
    //can then be sent through UDP socket
    static void createBuffer(const char* data, int dataSize, char* buffer);

    //extracts char* data and int dataSize from the buffer received from UDP
    //socket
    static void extractBuffer(char* buffer, char* data, int &dataSize);

    static void error(const char *msg);

    const static std::string metadata_ip; 
    const static std::string block_ip;
    const static int prefetch_portno;
    const static int leveldb_portno;
    static const int MSG_SIZE; 
    static const int PACKET_SIZE;


private:
    static int httpResponseReader(void *data, const char *buf, size_t len);
    
    //size is measured in bytes
};

#endif
