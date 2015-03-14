# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
CXXFLAGS=-std=c++11
OPTS = -lfcgi++ -lfcgi -L/usr/lib/x86_64-linux-gnu $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 
ZEROMQ_OPTS = -lzmq -I/usr/local/include 

JSON_LIB = /usr/lib/x86_64-linux-gnu/libjsoncpp.so
CACHE_OPTS = -l neon $(OPTS)

all: file_fetch file_store leveldb_helper_test 

cache: http_helper cache_file_fetch cache_file_store prefetch
	
read: read.cc
	$(CC) -o read read.cc $(OPTS)

write: write.cc
	$(CC) -o write write.cc $(OPTS)

file_fetch: file_fetch.cc
	$(CC) $(CXXFLAGS) file_fetch.cc -o fcgid/file_fetch  $(OPTS)

file_store: file_store.cc
	$(CC) $(CXXFLAGS) file_store.cc -o fcgid/file_store  $(OPTS)

leveldb_helper_test: leveldb_helper_test.cc	
	$(CC) $(CXXFLAGS) leveldb_helper_test.cc leveldb_helper.cc -o leveldb_helper_test $(OPTS)

#http helper
http_helper: http_helper.cpp
	$(CC) $(CXXFLAGS) -c http_helper.cpp $(CACHE_OPTS)

#cache_file_fetch
cache_file_fetch: cache_file_fetch.cpp
	$(CC) $(CXXFLAGS) http_helper.o leveldb_helper.cc cache_file_fetch.cpp $(JSON_LIB) -o fcgid/cache_file_fetch $(ZEROMQ_OPTS) $(CACHE_OPTS)

cache_file_store: cache_file_store.cpp
	$(CC) $(CXXFLAGS) http_helper.o leveldb_helper.cc cache_file_store.cpp $(JSON_LIB) -o fcgid/cache_file_store $(ZEROMQ_OPTS) $(CACHE_OPTS)

prefetch: prefetch.cpp
	$(CC) $(CXXFLAGS) -g http_helper.o prefetch.cpp leveldb_helper.cc $(JSON_LIB) -o fcgid/prefetch $(ZEROMQ_OPTS) $(CACHE_OPTS)

server: leveldb_server.cpp
	$(CC) $(CXXFLAGS) leveldb_server.cpp -o fcgid/server $(ZEROMQ_OPTS) $(OPTS)

install: 
#	sudo rm /var/www/html/file_fetch.fcgid
#	sudo rm /var/www/html/file_store.fcgid
#	sudo rm /var/www/html/*.fcgid
	sudo rm /var/www/html/file_fetch
	sudo rm /var/www/html/file_store
	sudo rm /var/www/html/cache_file_fetch
	sudo rm /var/www/html/cache_file_store
	sudo rm /var/www/html/prefetch
	sudo rm /var/www/html/server

	sudo cp fcgid/* -t /var/www/html
	sudo chmod 777 /var/www/html/*

clean:
	sudo rm -rf read write file_fetch file_store leveldb_helper_test *.fcgid temp /var/www/html/mydb

test: all
	./leveldb_helper_test
