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
#include <arpa/inet.h>
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
  std::cout << "Putting hash: " << key << std::endl;

    leveldb::Status s = db->Put(leveldb::WriteOptions(),key,value);
  if(!s.ok()) { 
    std::cerr << s.ToString() << std::endl; 
  }
    //std::string dummy;
  //db->Get(leveldb::ReadOptions(),key,&dummy);
  //std::cout << "value::" << value << std::endl;
  //std::cout << "dummy::" << dummy << std::endl;
  //std::cout << "string compare: " << value.compare(dummy) << std::endl;
  return s.ok();
}
/**
 * retreive single key/value from the database
 **/
bool get(const std::string& key, std::string& value) {


    std::cout << "Getting hash: " << key << std::endl; 
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

    std::cout << "Sending msg..." << std::endl;
    
    int bytesLeft = HttpHelper::MSG_SIZE;
    char * bufferPtr = buffer;
    while(bytesLeft > 0) {
        int sendSize = HttpHelper::PACKET_SIZE;
        if(bytesLeft < HttpHelper::PACKET_SIZE) {
            sendSize = bytesLeft;
        }
        if(send(newsockfd, bufferPtr, sendSize, 0)  < 0) {
            std::cout << "Sendto failed" << std::endl;
            printf("errno %d\n", errno);
        }

        bytesLeft -= HttpHelper::PACKET_SIZE;
        bufferPtr += HttpHelper::PACKET_SIZE;
    }
    
    std::cout << "Message sent!" << std::endl;
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
      //std::getline(ss,value,delim);
      int index = action.length() + key.length() + 2;
      value = command.substr(index);
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
  
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       HttpHelper::error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(HttpHelper::leveldb_portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(struct sockaddr)) < 0) 
              HttpHelper::error("ERROR on binding");
    
    if (listen(sockfd, 5) < 0) {
	HttpHelper::error("Error on listening");
    }

    char* buffer = (char*) malloc(HttpHelper::MSG_SIZE);;
    if (buffer == 0) {
    	HttpHelper::error("ERROR: Out of memory");
    }
    bzero(buffer, HttpHelper::MSG_SIZE);

   std::cout << "mono-thread server is ready " << std::endl;
   std::cout << "entering while loop" << std::endl;
   while(true) {
    std::cout << "------Waiting for local msg------" << std::endl;
    //HttpHelper::recvLocalMsg(request, HttpHelper::leveldb_portno);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (newsockfd < 0) {
      HttpHelper::error("Error on Accept");
      exit(1);
    }
    std::cout << "Server got connction from client " << inet_ntoa(cli_addr.sin_addr) << std::endl;
//    while (true) {
      char * bufferPtr = buffer;
      int bytesRcvd = 0;
      int n = 1;
      while (bytesRcvd < HttpHelper::MSG_SIZE && n > 0) {
        n = recv(newsockfd, bufferPtr, HttpHelper::PACKET_SIZE, 0);
        if (n == 0) {
		std::cout << "Client closed" << std::endl;
		continue;
	}
	else if (n < 0) {
		printf("errno %d\n", errno);
		continue;
	} else {
		bytesRcvd += n;
		bufferPtr += n;
	}
        
//        std::cout << "bytesRcvd: " << bytesRcvd << std::endl;
        usleep(20000);
        fflush(stdout);
      }

      std::cout << "recieved buffer message" << std::endl;      
      char* data;
      int dataSize = 0;
      HttpHelper::extractBuffer(buffer, data, dataSize);
      std::cout << "extracted buffer to data" << std::endl;
      std::string request(data, dataSize);
      delete data;

      std::cout << "Received local msg!" << std::endl;
      //std::cout << "Local msg: " << request << std::endl;
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
//    }

   
    // int bytesRcvd = 0;
    // while(bytesRcvd < HttpHelper::MSG_SIZE) {
    //     bytesRcvd += recvfrom(sockfd, bufferPtr, HttpHelper::PACKET_SIZE, 0, 
    //                     (struct sockaddr *) &cli_addr, &clilen);
    //     bufferPtr += HttpHelper::PACKET_SIZE;
	   //  std::cout << "bytesRcvd: " << bytesRcvd << std::endl;
    // }

    //std::cout << "received a message: " << buffer << std::endl;
    close(newsockfd);
    bzero(buffer, HttpHelper::MSG_SIZE);
  }
  close(sockfd);
  
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

