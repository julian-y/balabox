#ifndef MYSQL_HELPER_HPP

/* mysql connector headers */
#include <driver.h>
#include <connection.h>
#include <statement.h>
#include <prepared_statement.h>
#include <resultset.h>
#include <metadata.h>
#include <resultset_metadata.h>
#include <exception.h>
#include <warning.h>

/* basic c++ headers */
#include <vector>
#include <string>

/**
 * This class defines basic functionality to interact with the metadata
 * mysql database
 */
class MySQLHelper {
public:

    void
    connect();

    void 
    close();

    std::vector<std::vector<unsigned char>>
    getMissingBlockHashes(const std::string& userId, const std:string& filename);

    bool
    updateFileData(const std::string& userId, const std:string& filename, 
        const std::vector<std::vector<unsigned char>>& hashes); // This might not be verbose enough

    std::vector<std::vector<unsigned char>>
    getFileBlockList(const std::string& userId, const std:string& filename);

private:
    sql::Driver *m_driver;
    sql::Connection *m_conn;
};

#endif /* MYSQL_HELPER_HPP */