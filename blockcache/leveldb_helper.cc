#include "leveldb_helper.hpp"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include <string>
#include <iostream>
#include <unistd.h>		// sleep()
#include "http_helper.h"

using namespace std;

LevelDBHelper::LevelDBHelper()
{
}

LevelDBHelper::~LevelDBHelper() {
}

int LevelDBHelper::get(const string& block_hash, string& data) {
	string command("get," + block_hash);
//	zmq::message_t request(command.size());
//	memcpy((void*) request.data(), command.data(), command.size());
//	m_socket.send(request);
//
//	zmq::message_t response;
//	m_socket.recv(&response);

//	string response_data((char*)response.data(), response.size());
    string response_data = "";
    HttpHelper::sendLocalMsg(command, response_data, HttpHelper::leveldb_portno, true);
    if (response_data == "Operation failed")
		return 1;
	else 
		data = response_data;
	return 0;
}

int LevelDBHelper::put(const string& block_hash, const string& data) {
	string command("put," + block_hash + "," + data);
//	zmq::message_t request(command.size());
//	memcpy((void*) request.data(), command.data(), command.size());
//	m_socket.send(request);
//
//	zmq::message_t response;
//	m_socket.recv(&response);
//	string response_data((char*)response.data(), response.size());
    string response_data = "";
    HttpHelper::sendLocalMsg(command, response_data, HttpHelper::leveldb_portno, true);
    if (response_data == "Operation failed")
		return 1;
	else
		return 0;
}

bool LevelDBHelper::alreadyExists(const string& block_hash) {
	string dummy;
	return (get(block_hash, dummy) == 0) ? true : false;
}

