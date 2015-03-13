#ifndef LEVELDB_HELPER_HPP
#define LEVELDB_HELPER_HPP

#include <string>
#include "leveldb/db.h"

class LevelDBHelper {
public:
	LevelDBHelper(const std::string& db_name);
	~LevelDBHelper();

	/**
	* Retrieves data associated with block hash from database.
	* @param block_hash: string representing the block hash
	* @param data: string representing data associated with block hash
	* @return 0 on success, non-zero on failure
	*/
	int get(const std::string& block_hash, std::string& data);

	/**
	* Puts a (blockhash, data) pair into the database.
	* @param block_hash: string representing the block hash
	* @param data: string representing data associated with block hash
	* @return 0 on success, non-zero on failure
	*/
	int put(const std::string& block_hash, const std::string& data);

 	/**
	* Finds if a block hash already exists in the database.
	* @param block_hash: string representing the block hash
	* @return true if it exists, false if it does not
	*/
	bool alreadyExists(const std::string& block_hash);
	
private:
	// database
	leveldb::DB *m_db;

	// database options used
	leveldb::Options options;

	// database read options used
	leveldb::ReadOptions readoptions;

	// database write options used
	leveldb::WriteOptions writeoptions;
};

#endif // LEVELDB_HELPER_HPP
