#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>
#include "leveldb/db.h"

using namespace std;

/* KNOWN PROBLEM: LevelDB and VirtualBox shared folders that prevents
LevelDB from creating/accessing the directories associated with LevelDB databases.
    -Google: IO error Invalid argument LevelDB

    FIX: Move LevelDB-related code outside of shared folder
*/

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

    printf("Writing <hello, world>\n");
    db->Put(woptions, s, t);

    delete db;
}
