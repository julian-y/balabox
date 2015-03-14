#include "leveldb_helper.hpp"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include <string>
#include <iostream>
#include <unistd.h>		// sleep()
#include <zmq.hpp>

using namespace std;

zmq::context_t defaultContext(1);

LevelDBHelper::LevelDBHelper()
:context(defaultContext), socket(context, ZMQ_REQ) {
	socket.connect("ipc://test.ipc");
}

LevelDBHelper::~LevelDBHelper() {
}

int LevelDBHelper::get(const string& block_hash, string& data) {
	string command("get," + block_hash);
	zmq::message_t request(command.size());
	memcpy((void*) request.data(), command.data(), command.size());
	socket.send(request);

	zmq::message_t response;
	socket.recv(&response);

	string response_data((char*)response.data(), response.size());
	if (response_data == "Operation failed")
		return 1;
	else 
		data = response_data;
	return 0;
}

int LevelDBHelper::put(const string& block_hash, const string& data) {
	string command("put," + block_hash + "," + data);
	zmq::message_t request(command.size());
	memcpy((void*) request.data(), command.data(), command.size());
	socket.send(request);

	zmq::message_t response;
	socket.recv(&response);
	string response_data((char*)response.data(), response.size());
	if (response_data == "Operation failed")
		return 1;
	else
		return 0;
}

bool LevelDBHelper::alreadyExists(const string& block_hash) {
	string dummy;
	return (get(block_hash, dummy) == 0) ? true : false;
}

