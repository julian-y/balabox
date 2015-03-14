#ifndef LEVELDB_HELPER_HPP
#define LEVELDB_HELPER_HPP

#include <string>
#include "leveldb/db.h"
#include <zmq.hpp>

class LevelDBHelper {
public:
	LevelDBHelper(zmq::context_t& context, zmq::socket_t& socket);
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
	zmq::socket_t& m_socket;
	zmq::context_t& m_context;
};

#endif // LEVELDB_HELPER_HPP
