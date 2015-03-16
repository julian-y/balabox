#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sstream>

#include <leveldb/db.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "http_helper.h"

#include <stdio.h>
// definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/types.h>   
#include <sys/socket.h>  
// definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  
// constants and structures needed for 
// internet domain addresses, e.g. sockaddr_in
#include <errno.h>

leveldb::DB* db;
int sockfd, newsockfd, portno, pid;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;

/**
 * open/create the database
 **/
void create_db(const std::string& db_name) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status=leveldb::DB::Open(options, db_name, &db);
  if(!status.ok()) {
    std::cout << "Unable to create database, " << status.ToString() << std::endl;
  }
  std::cout << "Created database" << std::endl;
}
/**
 * add new key/value to the database
 **/
bool put(const std::string& key, const std::string& value) {
  leveldb::Status s = db->Put(leveldb::WriteOptions(),key,value);
  if(!s.ok()) { 
    std::cerr << s.ToString() << std::endl; 
  }
  return s.ok();
}
/**
 * retreive single key/value from the database
 **/
bool get(const std::string& key, std::string& value) {
   
  leveldb::Status s = db->Get(leveldb::ReadOptions(),key,&value);
  if(!s.ok()) { 
    std::cerr << s.ToString() << std::endl; 
  }
  return s.ok();
}

void sendLevelDBMsg(std::string msg) {
    std::string dummy;
    //HttpHelper::sendLocalMsg(msg, dummy, HttpHelper::leveldb_portno, false);
    char* buffer;
    HttpHelper::createBuffer(msg.c_str(), msg.length(), buffer);

    std::cout << "Sending msg: " << msg << std::endl;
    
    int status = sendto(sockfd, buffer, HttpHelper::MSG_SIZE, 0, 
                      (struct sockaddr *) &cli_addr, clilen);
    if (status  < 0) {
        std::cout << "Sendto failed" << std::endl;
        printf("errno %d\n", errno);
    }

    delete buffer;
}
/**
 * retrieve all the values associated with a key
 **/
bool multi_get(const std::string& key, std::string& value) {
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    it->Seek(key);
    if(!it->Valid()) {
        delete it;
        return false;
    }
    while(it->Valid() && it->key().ToString() == key) {
        value = it->value().ToString() + ":";
        it->Next();
    }
    delete it;
    return true;
}


typedef enum {
  ERROR = 0,
  GET = 1,
  PUT = 2,
  MGET = 3
} OperationType;

typedef struct {
  OperationType _type;
  bool _status;
  std::string _key;
  std::string _value;
} Operation;

bool do_operation(const std::string& command, Operation& operation) {

  std::stringstream ss(command);
  const char delim=',';
  std::string action,key,value;
  if(std::getline(ss,action,delim)) {
    if(action=="put") {
      
      std::getline(ss,key,delim);
      std::getline(ss,value,delim);
//      std::cout << "Put" << std::endl;    
      operation._type = PUT;
      operation._key = key;
      operation._value = value;
      operation._status = put(key,value);
      return operation._status;
    }
    else if(action=="get") {
      std::getline(ss,key,delim);
//      std::cout << "Get" << std::endl;
      operation._type = GET;
      operation._key = key;
      operation._status = get(key,value);
      operation._value = value;

      return operation._status;
    }
    else if(action=="mget") {
      std::getline(ss,key,delim);
      
      operation._type = MGET;
      operation._key = key;
      operation._status = multi_get(key,value);
      operation._value = value;

      return operation._status;
    }
     else {
      std::cerr << "unknown action " << action << std::endl;
    }
  }
  else {
    std::cerr << "Unable to parse command " << command << std::endl;
  }
  return false;
}

void run_mono_thread(const std::string& folder) {

    create_db(folder);
    clilen = sizeof(cli_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
       HttpHelper::error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(HttpHelper::leveldb_portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              HttpHelper::error("ERROR on binding");
    
    char* buffer = (char*) malloc(HttpHelper::MSG_SIZE);;
    bzero(buffer, HttpHelper::MSG_SIZE);

  std::cout << "mono-thread server is ready " << std::endl;
  std::cout << "entering while loop" << std::endl;
  while(true) {
    std::cout << "------Waiting for local msg------" << std::endl;
    //HttpHelper::recvLocalMsg(request, HttpHelper::leveldb_portno);
    int recvlen = recvfrom(sockfd, buffer, HttpHelper::MSG_SIZE, 0, 
                    (struct sockaddr *) &cli_addr, &clilen);
    std::cout << "received a message: " << buffer << std::endl;
    std::cout << "recieved buffer message" << std::endl;      
    char* data;
    int dataSize = 0;
    HttpHelper::extractBuffer(buffer, data, dataSize);
    std::cout << "extracted buffer to data" << std::endl;
    std::string request(data, dataSize);
    delete data;

    std::cout << "Received local msg!" << std::endl;
    std::cout << "Local msg: " << request << std::endl;
    Operation operation;
    std::string command((char*)request.data(),request.size());
    bool success=do_operation(std::string((char*)request.data(),request.size()),operation);

    if(success) {
      if(operation._type==GET) {
        sendLevelDBMsg(operation._value);
      }
      else if(operation._type==PUT) {
        sendLevelDBMsg("OK");
      }
      else {
        sendLevelDBMsg("Wrong action");
      }
    }
    else {
      sendLevelDBMsg("Operation failed");
    }

    bzero(buffer, HttpHelper::MSG_SIZE);
  }
  delete buffer;
  delete db;
}

int main(int argc, char** argv) {

    if(argc==1) {
        std::cout << "server folder_database" << std::endl;
        return -1;
    }

    std::string folder = argv[1];

    run_mono_thread(folder);
}

