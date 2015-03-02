# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
CXXFLAGS=-std=c++11
OPTS = -lfcgi++ -lfcgi -L/usr/lib/x86_64-linux-gnu $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 

all: read write file_fetch file_store
	
read: read.cc
	$(CC) -o read read.cc $(OPTS)

write: write.cc
	$(CC) -o write write.cc $(OPTS)

file_fetch: file_fetch.cc
	$(CC) $(CXXFLAGS) file_fetch.cc -o file_fetch.fcgid  $(OPTS)

file_store: file_store.cc
	$(CC) $(CXXFLAGS) file_store.cc -o file_store.fcgid  $(OPTS)

install: 
	sudo rm /var/www/html/*.fcgid
	sudo cp ./*.fcgid -t /var/www/html

clean:
	sudo rm -rf read write *.fcgid temp /var/www/html/mydb

test: all
	./write
	./read
