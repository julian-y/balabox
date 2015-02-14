#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>      // std::ifstream()
#include "leveldb/db.h"

using namespace std;

/* KNOWN PROBLEM: LevelDB and VirtualBox shared folders that prevents
LevelDB from creating/accessing the directories associated with LevelDB databases.
    -Google: IO error Invalid argument LevelDB

    FIX: Move LevelDB-related code outside of shared folder
*/

/* 
    Reads data from input file into a vector<unsigned char>.
*/
vector<unsigned char> readFile(const string& fileName) {
    ifstream request(fileName.c_str(), ios::in | ios::binary);
    vector<unsigned char> data;
    size_t data_length = 0;

    if (request) {
        while (true) {
            unsigned char c = request.get();
            if (request.eof())
                break;

            data.push_back(c);
            data_length++;
        }
    } else {
        cerr << "ERROR: File not found!" << endl;
    }

    if (data_length != data.size())
        cerr << "Not same sizes!" << endl;

    return data;
}

/* 
    Given a file's hash and the data that needs to be stored, 
    return a key-value pair suitable for leveldb storage.
*/
pair<string, string> constructKV(const string& hash, const vector<unsigned char>& fileData) {
    string value(fileData.begin(), fileData.end());

    if (value.size() != fileData.size())
        cerr << "ERROR: Not same size!" << endl;

    return make_pair(hash, value);
}

int main() {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "temp", &db);      
    if (!status.ok())
        cerr << status.ToString() << endl;
    else {
        cerr << "Database successfully opened/created!" << endl;
    }
    

    leveldb::WriteOptions woptions;
    leveldb::Slice s = "hello";
    leveldb::Slice t = "World";

    printf("Writing {hello: world}\n");
    db->Put(woptions, s, t);

    vector<unsigned char> fileData = readFile("small.gif");
    pair<string, string> test = constructKV("test", fileData);

    db->Put(woptions, test.first, test.second);

    delete db;
}
