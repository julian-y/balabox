# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
CXXFLAGS=-std=c++11
OPTS = -lfcgi++ -lfcgi -L/usr/lib/x86_64-linux-gnu $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 

JSON_LIB = /usr/lib/x86_64-linux-gnu/libjsoncpp.so
CACHE_OPTS = -l neon $(OPTS)

tester: test.cpp
	$(CC) -g -o test test.cpp http_helper.o $(CACHE_OPTS)

all: util block cache

util: http_helper leveldb_server 

block: util block_fetch block_store 

cache: util cache_block_fetch cache_block_store prefetch 
	
read: read.cc
	$(CC) -o read read.cc $(OPTS)

write: write.cc
	$(CC) -o write write.cc $(OPTS)

block_fetch: block_fetch.cc
	$(CC) $(CXXFLAGS) block_fetch.cc -o fcgid/block_fetch  $(OPTS)

block_store: block_store.cc
	$(CC) $(CXXFLAGS) block_store.cc -o fcgid/block_store  $(OPTS)

leveldb_helper_test: leveldb_helper_test.cc	
	$(CC) $(CXXFLAGS) -g http_helper.o leveldb_helper_test.cc leveldb_helper.cc -o leveldb_helper_test $(CACHE_OPTS)

#http helper
http_helper: http_helper.cpp
	$(CC) $(CXXFLAGS) -g -c http_helper.cpp $(CACHE_OPTS)

#cache_block_fetch
cache_block_fetch: cache_block_fetch.cpp
	$(CC) $(CXXFLAGS) http_helper.o leveldb_helper.cc cache_block_fetch.cpp $(JSON_LIB) -o fcgid/cache_block_fetch $(CACHE_OPTS)

cache_block_store: cache_block_store.cpp
	$(CC) $(CXXFLAGS) http_helper.o leveldb_helper.cc cache_block_store.cpp $(JSON_LIB) -o fcgid/cache_block_store $(CACHE_OPTS)

prefetch: prefetch.cpp
	$(CC) $(CXXFLAGS) -g http_helper.o prefetch.cpp leveldb_helper.cc $(JSON_LIB) -o fcgid/prefetch $(CACHE_OPTS)

leveldb_server: leveldb_server.cpp
	$(CC) $(CXXFLAGS) -g http_helper.o leveldb_server.cpp -o fcgid/server $(CACHE_OPTS)

install: 
	sudo rm -f /var/www/html/block_fetch
	sudo rm -f /var/www/html/block_store
	sudo rm -f /var/www/html/cache_block_fetch
	sudo rm -f /var/www/html/cache_block_store
	sudo rm -f /var/www/html/prefetch
	sudo rm -f /var/www/html/server

	sudo cp fcgid/* -t /var/www/html
	sudo chmod -R 777 /var/www/html

clean:
	sudo rm -rf fcgid/* temp /var/www/html/mydb /var/www/html/cacheDB
	sudo mkdir /var/www/html/cacheDB
	sudo mkdir /var/www/html/mydb
	-pkill server
	nohup /var/www/html/./server cacheDB &
test: all
	./leveldb_helper_test
