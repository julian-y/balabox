#include <stdio.h>
#include <string>
#include <iostream>
#include "mysql_helper.hpp"

#include <my_global.h>
#include <mysql.h>

using namespace std;

int test_file_list(MySQLHelper &h)
{
   cout << endl << "--FILE LIST TEST--"<< endl; 
   string uid = "steven";
   string filename = "testfile";
   vector<string> hashes;
   if (h.getFileBlockList(uid,filename,hashes) != 0) {
       return -1;
   }
   for (unsigned int i = 0; i <hashes.size(); ++i) {
       cout << hashes[i] << endl;
   }

    return 0;   
}

int test_file_update(MySQLHelper &h)
{
   cout << endl << "--FILE UPDATE TEST--"<< endl; 
   string uid = "steven";
   string filename = "newfile";
   vector<string> hashes;
   hashes.push_back("aaf02993af40bf0c8ab083519af47b0d3c5af5110b72d4a3eaea2df0c765264d");
   hashes.push_back("4a8d881b5d8f7fed33b1f5a6cd0e289ed6d801bd32dbb74bc3feeef8b2eceb3e");
   if (h.updateFileData(uid,filename,hashes) != 0) {
       return -1;
   }
   vector<string> retrieved_hashes;
   if (h.getFileBlockList(uid,filename,hashes) != 0) {
       return -1;
   }

   for (unsigned int i = 0; i <retrieved_hashes.size(); ++i) {
       cout << hashes[i] << endl;
   } 

    return 0;
}

int test_missing_block_hashes(MySQLHelper &h)
{
    cout << endl << "--GET MISSING BLOCK HASH TEST--" << endl;
    vector<string> userHashes;
    userHashes.push_back("c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe");
    userHashes.push_back("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
    // shouldnt be in test db
    userHashes.push_back("ffa63583dfa6706b87d284b86b0d693a161e4840aad2c5cf6b5d27c3b9621f7d");
    
    vector<string> missingHashes;
    if (h.getMissingBlockHashes(userHashes,missingHashes) != 0) {
        return -1;
    }
    if (missingHashes.size() != 1) {
        return -1;
    }

    cout << userHashes[0] << endl;
    return 0;
}

int main(void)
{
    cout << "---MYSQL HELPER TEST---"<<endl<<endl;
    MySQLHelper h;
    int failed = 0;
    if (h.connect() == 0) {
        cout << "Successfully connected to mysql database!\n";
    }
    else {
        cout << "Failed to connect to mysql db!" << endl;
        failed++;
        goto end;
    }

    if(test_file_list(h) != 0) {
        printf("File list test failed!\n");
        failed++;
    }
    else {
        cout << "File list test succeeded"<<endl;
    }
    
    if(test_file_update(h) != 0) {
        cout << "file update test failed!" << endl;
        failed++;
    }
    else {
        cout << "file update test succeeded!" << endl;
    }

    if (test_missing_block_hashes(h) != 0) {
        cout << "get missing block hashes test failed!" << endl;
        failed++;
    }
    else {
        cout << "get missing block hashes test succeeded!" << endl;
   
    }    
    if(h.close() == 0) {
        printf("Successfully closed mysql connection!\n");
    }
    else {
        failed++;
        cout << "Failed to close mysql connection!" << endl;
    }
    
    end:
    if (failed == 0) {
        cout << endl << "---ALL TESTS PASSED---" << endl;
    }
    else {
        printf("\n---%d TESTS Failed---\n", failed);
    } 
    }
