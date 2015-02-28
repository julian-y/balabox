#include <string>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <sstream>

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
    MYSQL_RES *res;
    // perform a query for each hash in user hashes, add hash to missingHashes if it wasnt in db
    for (unsigned int i = 0; i < userHashes.size(); ++i) {
        string curHashQuery = "SELECT COUNT(*) FROM FileBlock WHERE block_hash='" + userHashes[i] + "'";
        if (mysql_query(m_conn, curHashQuery.c_str()) != 0) {
            return mysql_errno(m_conn);
        }
        res = mysql_store_result(m_conn);
        if (res == NULL) {
            return mysql_errno(m_conn);
        }

        // should only be one row containing the count(0 or 1)
        MYSQL_ROW row = mysql_fetch_row(res);
        // count will be in first column
        if (row && atoi(row[0]) == 0) {
            missingHashes.push_back(userHashes[i]);
        }
    }

    mysql_free_result(res);
    return 0;
}

/** 
 * TODO: add verification of userid and filename here before deleting
 */
int
MySQLHelper::updateFileData(const string& userId, const string& filename, 
        const vector<string>& hashes, unsigned int version) 
{
    // verify version 
    string versionChkStmt = "SELECT version FROM FileBlock WHERE user_id='" + userId + 
        "' AND file_name='" + filename + "'";
    if (mysql_query(m_conn, versionChkStmt.c_str())) {
      return mysql_errno(m_conn);
    }
    MYSQL_RES *res = mysql_store_result(m_conn);
    if(res == NULL) {
      return mysql_errno(m_conn);
    }
    // if file doesn't exist at all in the table, make sure user's version is 0
    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        if (version != 0) {
          mysql_free_result(res);
          return EVERS;
        } 
    }
    // make sure user's version is greater than the current version if file is in mysql
    else {
        if (atoi(row[0]) >= version) {
          mysql_free_result(res);
          return EVERS;
        }
    } 

    // delete file's old rows 
    string deleteStmt = "DELETE FROM FileBlock WHERE user_id='" + userId + 
          "' AND file_name='" + filename + "'";
    if (mysql_query(m_conn,deleteStmt.c_str())) {
      cout << "delete stmt: " << deleteStmt << endl;  
      return mysql_errno(m_conn);
    }
    // insert new rows for file 
    for (unsigned int i = 0; i < hashes.size(); ++i) {
        string insertStmt = "INSERT INTO FileBlock(user_id, file_name, block_hash, block_number, version) VALUES('" 
            + userId + "','" + filename + "','" + hashes[i] + "',";
        insertStmt += intToStr(i) + "," + intToStr(version) + ")";
        if(mysql_query(m_conn, insertStmt.c_str())) {
            return mysql_errno(m_conn);
        }
    }       
    mysql_free_result(res);
    return 0;
}

int
MySQLHelper::getFileBlockList(const std::string& userId, const std::string& filename, 
    std::vector<std::string>& hashes, unsigned int &version)
{
    // perform a query for each hash in user hashes, add hash to missingHashes if it wasnt in db
    string curHashQuery = "SELECT block_hash,version FROM FileBlock WHERE user_id='" 
        + userId + "' AND file_name='" + filename + "' ORDER BY block_number ASC";
    
    if (mysql_query(m_conn, curHashQuery.c_str()) != 0) {
        return mysql_errno(m_conn);
    }

    MYSQL_RES *res = mysql_store_result(m_conn);
    if (res == NULL) {
        return mysql_errno(m_conn);
    }

    // add all blocks to hashes vector
    MYSQL_ROW row;
    while ( (row = mysql_fetch_row(res)) ) {
        string hash = row[0];
        version = atoi(row[1]);
        hashes.push_back(hash);
    }

    mysql_free_result(res);
    return 0;
}

int 
MySQLHelper::getUserFileNames(const std::string& userId, std::vector<std::string>& fileNames)
{
    // perform a query for each hash in user hashes, add hash to missingHashes if it wasnt in db
    string userFilesQuery = "SELECT DISTINCT file_name FROM FileBlock WHERE user_id='" 
        + userId + "'";
    
    if (mysql_query(m_conn, userFilesQuery.c_str()) != 0) {
        return mysql_errno(m_conn);
    }

    MYSQL_RES *res = mysql_store_result(m_conn);
    if (res == NULL) {
        return mysql_errno(m_conn);
    }

    // add all file names to vector
    MYSQL_ROW row;
    while ( (row = mysql_fetch_row(res)) ) {
        string file_name = row[0];
        fileNames.push_back(file_name);
    }

    mysql_free_result(res);
    return 0;
}

int
MySQLHelper::getRecentFirstHashes(const std::string& userId, unsigned int maxHashes,
        std::vector<std::string>& firstHashes)
{
    // perform a query for each hash in user hashes, add hash to missingHashes if it wasnt in db
    string recentHashQuery = "SELECT block_hash FROM FileBlock WHERE user_id='" 
        + userId + "' ORDER BY block_number ASC,time_last_accessed DESC LIMIT ";
    recentHashQuery += intToStr(maxHashes);

    if (mysql_query(m_conn, recentHashQuery.c_str()) != 0) {
        return mysql_errno(m_conn);
    }

    MYSQL_RES *res = mysql_store_result(m_conn);
    if (res == NULL) {
        return mysql_errno(m_conn);
    }

    // add all blocks to hashes vector
    MYSQL_ROW row;
    while ( (row = mysql_fetch_row(res)) ) {
        // should only be one column containing the hash
        string hash = row[0];
        firstHashes.push_back(hash);
    }

    mysql_free_result(res);
    return 0;
}

string MySQLHelper::intToStr(int i)
{
  string temp;
  stringstream out;
  out << i; // dirty trick to convert int to std::string
  return out.str();
}
