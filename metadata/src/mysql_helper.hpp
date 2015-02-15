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

    /**
     * Creates a connection to the local mysql database
     */
    bool
    connect();

    /**
     * Closes connection to the local mysql database
     * @note connect() must have been previously called
     */
    bool 
    close();

    /**
     * Finds supplied block hashes that are NOT in the database
     * @param user_hashes vector of user supplied hashes  
     * @param missing_hashes vector of user hashes that are NOT in the database
     * @return 0 on success, non-zero on failure
     */
    int 
    getMissingBlockHashes(const std::vector<std::string>& userHashes, 
        std::vector<std::string>& missingHashes);

    /**
     * Commits an update for a file to the database
     * @param userId user id string
     * @param filename name of file to commit changes to
     * @param hashes vector of block hashes for the file
     * @return 0 on success, non-zero on failure 
     * @note a user should have checked to make sure their blocks have been uploaded
     */
    bool
    updateFileData(const std::string& userId, const std::string& filename, 
        const std::vector<std::string>& hashes); 

    /**
     * Retrieves block hashes for a given user's file
     * @param userId user id string
     * @param filename file to retrieve block hashes for
     * @param hashes reference to vector to fill with the requested file's hashes 
     * @return 0 on success, non-zero on failure
     */
    int
    getFileBlockList(const std::string& userId, const std::string& filename, 
        std::vector<std::string>& hashes);

private:
    sql::Driver *m_driver;
    sql::Connection *m_conn;
};

#endif /* MYSQL_HELPER_HPP */