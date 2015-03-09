#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <string>

class HttpHelper {
public:
    static int getQueryParam(const std::string& query_string, 
            const std::string& param, std::string& value);

    static int sendHttpRequest(std::string host_ip, std::string path, 
            std::string reqType, std::string reqBody, std::string &responseHeader, 
            std::string &response);

    static int requestFromBlockServer(std::string hash, std::string block);

    const static std::string metadata_ip; 
    const static std::string block_ip;
private:
    static int httpResponseReader(void *data, const char *buf, size_t len);
};

#endif
