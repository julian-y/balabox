# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
CXXFLAGS=-std=c++11
OPTS = -lfcgi++ -lfcgi -L/usr/lib/x86_64-linux-gnu $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 

JSON_LIB = /usr/lib/x86_64-linux-gnu/libjsoncpp.so
CACHE_OPTS = -lfcgi -l neon

all: file_fetch file_store leveldb_helper_test 

cache: http_helper cache_file_fetch prefetch
	
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
	$(CC) -c http_helper.cpp $(CACHE_OPTS)

#cache_file_fetch
cache_file_fetch: cache_file_fetch.cpp
	$(CC) $(CXXFLAGS) http_helper.o cache_file_fetch.cpp $(JSON_LIB) -o fcgid/cache_file_fetch $(CACHE_OPTS)

cache_file_store: cache_file_store.cpp
	$(CC) $(CXXFLAGS) http_helper.o cache_file_store.cpp $(JSON_LIB) -o fcgid/cache_file_store $(CACHE_OPTS)

prefetch: prefetch.cpp
	$(CC) $(CXXFLAGS) http_helper.o prefetch.cpp $(JSON_LIB) -o prefetch $(CACHE_OPTS)

install: 
#	sudo rm /var/www/html/file_fetch.fcgid
#	sudo rm /var/www/html/file_store.fcgid
#	sudo rm /var/www/html/*.fcgid
	sudo cp fcgid/* -t /var/www/html
	sudo chmod 777 /var/www/html/*

clean:
	sudo rm -rf read write file_fetch file_store leveldb_helper_test *.fcgid temp /var/www/html/mydb

test: all
	./leveldb_helper_test
