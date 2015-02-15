#include <string>
#include <vector>

/* MAKE SURE THESE ARE DEFINED LAST */
#include <my_global.h>
#include <mysql.h>
#include "mysql_helper.hpp"

#define DBHOST "127.0.0.1"
#define PORT 3306
#define USER "cs188"
#define PASSWORD ""
#define DATABASE "test"

using namespace std;

int 
MySQLHelper::connect()
{
    // initialize mysql data structure
    m_conn = mysql_init(NULL);
    if (!m_conn) {
        return mysql_errno(m_conn);
    }

    // connect to database
    if (mysql_real_connect(m_conn, DBHOST, USER, PASSWORD, DATABASE, 
        PORT, NULL, 0) == NULL) {
        return mysql_errno(m_conn);
    }

    return 0;
}

int 
MySQLHelper::connect(const string& host, unsigned int port, const string& db, const string& user, 
    const string& pass)
{
    // initialize mysql data structure
    m_conn = mysql_init(NULL);
    if (!m_conn) {
        return mysql_errno(m_conn);
    }

    // connect to database
    if (mysql_real_connect(m_conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), 
        port, NULL, 0) == NULL) {
        return mysql_errno(m_conn);
    }

    return 0; 
}

int 
MySQLHelper::close() 
{
    mysql_close(m_conn);
    return 0;
}

int 
MySQLHelper::getMissingBlockHashes(const std::vector<std::string>& userHashes, 
    std::vector<std::string>& missingHashes)
{
    return 0;
}

int
MySQLHelper::updateFileData(const string& userId, const string& filename, 
        const vector<string>& hashes) 
{
    return 0;
}

int
MySQLHelper::getFileBlockList(const std::string& userId, const std::string& filename, 
    std::vector<std::string>& hashes)
{
    return 0;
}