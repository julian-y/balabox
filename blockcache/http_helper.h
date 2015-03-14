#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <string>

class HttpHelper {
public:
    static int getQueryParam(const std::string& query_string, 
            const std::string& param, std::string& value);

    static int sendHttpRequest(std::string host_ip, std::string path, 
            std::string reqType, std::string reqBody, std::string &responseContentType, 
            std::string &response, std::string &responseCode);

    static int requestFromBlockServer(std::string hash, std::string &responseContentType, std::string &response, std::string &responseCode);

    static int sendLocalMsg(std::string msg, std::string &resp, int portno);
    static int recvLocalMsg(std::string &msg, int portno);

    const static std::string metadata_ip; 
    const static std::string block_ip;
    const static int prefetch_portno;
    const static int leveldb_portno;
private:
    static int httpResponseReader(void *data, const char *buf, size_t len);
    static void error(const char *msg);
};

#endif
