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
    getMissingBlocks(unsigned int userId, std:string filename);

    bool
    updateFileData(unsigned int userId, std:string filename, const vector<>& hashes);

    std::vector<std::vector<unsigned char>>
    getFileBlockList(unsigned int userId, std:string filename);

private:
    sql::Driver *driver;
    sql::Connection *con;
};

#endif /* MYSQL_HELPER_HPP */