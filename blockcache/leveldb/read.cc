#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>      // std::ofstream()
#include "leveldb/db.h"

using namespace std;

/*
    Converts a string to vector<unsigned char>.
*/
vector<unsigned char> convertString(const string& value) {
    vector<unsigned char> result;

    for (size_t i = 0; i < value.size(); i++) {
        result.push_back(value[i]);
    }

    if (result.size() != value.size()) {
        cerr << "ERROR: Not same size!" << endl;
    }

    return result;
}

/*
    Writes contents of vector to file.
*/
void writeFile(const vector<unsigned char>& data) {
    ofstream output;
    string output_filename = "test.out";

    output.open(output_filename.c_str(), ios::out | ios::binary);

    if (!output.is_open()) {
        cerr << "ERROR: Cannot open output file!" << endl;
    }

    for (size_t i = 0; i < data.size(); i++) {
        output << data[i];
    }

    output.close();
}


int main() {
    leveldb::DB *db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, "temp", &db);
    if (!status.ok())
        cerr << status.ToString() << endl;
    else {
        cerr << "Database successfully opened/created!" << endl;
    }

    leveldb::ReadOptions roptions;

    leveldb::Iterator* it = db->NewIterator(roptions);
    printf("Reading database\n");
    for(it->SeekToFirst(); it->Valid(); it->Next()) {
        leveldb::Slice s = it->key();
        leveldb::Slice t = it->value();
        cout  << s.ToString() 
             << ":" 
             << t.ToString() 
             << endl;
    }

    string value;
    db->Get(roptions, "test", &value);
    vector<unsigned char> result = convertString(value);
    writeFile(result);

    delete db;
}
