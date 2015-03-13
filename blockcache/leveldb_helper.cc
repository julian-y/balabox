#include "leveldb_helper.hpp"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include <string>
#include <iostream>

using namespace std;

LevelDBHelper::LevelDBHelper(const string& db_name) {
	options.create_if_missing = true;
	writeoptions.sync = true;
	leveldb::Status status = leveldb::DB::Open(options, db_name, &m_db);
	if (!status.ok()) {
		cout << status.ToString() << endl;
	} 
	// else {
	// 	cerr << db_name << " successfully opened/created!" << endl;
	// }
}

LevelDBHelper::~LevelDBHelper() {
	delete m_db;
	m_db = nullptr;
}

int LevelDBHelper::get(const string& block_hash, string& data) {
	string value;
	leveldb::Status status = m_db->Get(readoptions, block_hash, &value);
	if (status.IsNotFound()) {
		return 1;
	}
	data = value;
	return 0;
}

int LevelDBHelper::put(const string& block_hash, const string& data) {
	leveldb::Status status = m_db->Put(writeoptions, block_hash, data);

	if (!status.ok()) {
		cout << status.ToString() << endl;
		return 1;
	} 
	// cout << "(" << block_hash << "," << data << ") succesfully inserted!" << endl;
	return 0;
}

bool LevelDBHelper::alreadyExists(const string& block_hash) {
	string dummy;
	return (get(block_hash, dummy) == 0) ? true : false;
}

