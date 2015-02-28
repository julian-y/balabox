#ifndef MYSQL_HELPER_HPP

/* basic c++ headers */
#include <vector>
#include <string>

/* mysql connector headers: DEFINE THESE LAST OR ELSE */
#include <my_global.h>
#include <mysql.h>

#define EVERS 1

/**
 * This class defines basic functionality to interact with the metadata
 * mysql database
 */
class MySQLHelper {
public:

    /**
     * Opens a connection to the local mysql database using default parameters
     * @return 0 on success, nonzero on failure
     */
    int
    connect();

    /**
     * Opens a connection to a mysql database with user provided parameters
     * @return 0 on success, nonzero on failure
     */
     int
     connect(const std::string& host, unsigned int port, const std::string& db, const std::string& user, 
        const std::string& pass);

    /**
     * Closes connection to the local mysql database
     * @return 0 on success, nonzero on failure
     * @note connect() must have been previously called
     */
    int 
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
     * @param version new version of the file. must be greater than the current 
     *                version stored in the metaserver for the update to succeed
     * @return 0 on success, EVERS on version mismatch, other non-zero constant for mysql failure
     * @note a user should have checked to make sure their blocks have been uploaded
     */
    int
    updateFileData(const std::string& userId, const std::string& filename, 
        const std::vector<std::string>& hashes, unsigned int version); 

    /**
     * Retrieves block hashes for a given user's file
     * @param userId user id string
     * @param filename file to retrieve block hashes for
     * @param hashes reference to vector to fill with the requested file's hashes 
     * @param version reference to integer to fill with the requested file's version
     * @return 0 on success, non-zero on failure
     */
    int
    getFileBlockList(const std::string& userId, const std::string& filename, 
        std::vector<std::string>& hashes, unsigned int &version);
    
    /**
     * Retrieves a list of all file names for a given user
     * @param userId id of the user
     * @param fileNames reference to vector that will be filled with the user's file names
     * @return 0 on success, non-zero on failure
     */
    int
    getUserFileNames(const std::string& userId, std::vector<std::string>& fileNames);
    
    /**
     * Retreives recent hashes for a user's files starting with the beginning blocks.
     * Once the first block hashes for all files have been selected, the second block
     * hashes for all will be sent, etc. until maxHashes have been found.  
     *
     * @param userId user to retrieve hashes for
     * @param maxHashes maximum number of block hashes to return
     * @param firstHashes reference to vector to fill with the hashes
     * @return 0 on success, non-zero on failure
     */
    int
    getRecentFirstHashes(const std::string& userId, unsigned int maxHashes, 
           std::vector<std::string>& firstHashes);

    /**
     * Retreives a list of caches associated with a user
     * @param userId id of user to retrieve caches for
     * @param maxCahces maximum number of caches to return
     * @param ipAddrs vector of strings 
     * @return 0 on success, non-zero on failure
     */
    int
    getCaches(const std::string& userId, unsigned int maxCaches, 
        std::vector<std::string>& ipAddrs);

    /**
     * Removes an association between a given user and a given cache
     * @param user to remove cache from
     * @param ipAddr ip address string of cache to remove
     * @return 0 on success, non-zero on error
     */
    int
    removeCache(const std::string& userId, const std::string& ipAddr);

private:
    MYSQL *m_conn;
    std::string intToStr(int i);
};

#endif /* MYSQL_HELPER_HPP */
