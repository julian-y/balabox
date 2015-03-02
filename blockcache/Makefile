# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
CXXFLAGS=-std=c++11
OPTS = -lfcgi++ -lfcgi -L/usr/lib/x86_64-linux-gnu $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 

all: file_fetch file_store leveldb_helper_test
	
read: read.cc
	$(CC) -o read read.cc $(OPTS)

write: write.cc
	$(CC) -o write write.cc $(OPTS)

file_fetch: file_fetch.cc
	$(CC) $(CXXFLAGS) file_fetch.cc -o file_fetch.fcgid  $(OPTS)

file_store: file_store.cc
	$(CC) $(CXXFLAGS) file_store.cc -o file_store.fcgid  $(OPTS)

leveldb_helper_test: leveldb_helper_test.cc	
	$(CC) $(CXXFLAGS) leveldb_helper_test.cc leveldb_helper.cc -o leveldb_helper_test $(OPTS)

install: 
	sudo rm /var/www/html/file_fetch.fcgid
	sudo rm /var/www/html/file_store.fcgid
	sudo cp ./*.fcgid -t /var/www/html

clean:
	sudo rm -rf read write file_fetch file_store leveldb_helper_test *.fcgid temp /var/www/html/mydb

test: all
	./leveldb_helper_test
