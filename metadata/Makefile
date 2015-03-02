# 
# 

CC=g++
CPPFLAGS=-Wall

LDFLAGS=-lfcgi++ -lfcgi -ljsoncpp -L/usr/lib/x86_64-linux-gnu
MYSQL_LINK=`mysql_config --cflags --libs`


OBJ_DIR=./obj
BINARY_DIR=./bin
SRC_DIR=./src
TEST_DIR=./test
INCLUDE_DIR=./include

COMMIT_SOURCES=$(SRC_DIR)/file_commit.cc
DELETE_SOURCES=$(SRC_DIR)/file_delete.cc
QUERY_SOURCES=$(SRC_DIR)/block_query.cc
BLOCK_LIST_SOURCES=$(SRC_DIR)/block_list.cc
RECENT_HASHES_SOURCES=$(SRC_DIR)/recent_hashes.cc
FILE_LIST_SOURCES=$(SRC_DIR)/file_list.cc
USER_CACHE_ADD_SOURCES=$(SRC_DIR)/user_cache_add.cc
USER_CACHES_SOURCES=$(SRC_DIR)/user_caches.cc
USER_CACHE_REMOVE_SOURCES=$(SRC_DIR)/user_cache_remove.cc
MYSQL_HELPER_OBJECTS=$(OBJ_DIR)/mysql_helper.o
FCGI_UTIL_OBJECTS=$(OBJ_DIR)/fcgi_util.o
QUERY_OBJECTS=$(OBJ_DIR)/block_query.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
COMMIT_OBJECTS=$(OBJ_DIR)/file_commit.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
DELETE_OBJECTS=$(OBJ_DIR)/file_delete.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
BLOCK_LIST_OBJECTS=$(OBJ_DIR)/block_list.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
FILE_LIST_OBJECTS=$(OBJ_DIR)/file_list.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
RECENT_HASHES_OBJECTS=$(OBJ_DIR)/recent_hashes.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
QUERY_OBJECTS=$(OBJ_DIR)/block_query.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
USER_CACHE_ADD_OBJECTS=$(OBJ_DIR)/user_cache_add.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
USER_CACHE_REMOVE_OBJECTS=$(OBJ_DIR)/user_cache_remove.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
USER_CACHES_OBJECTS=$(OBJ_DIR)/user_caches.o $(MYSQL_HELPER_OBJECTS) $(FCGI_UTIL_OBJECTS)
TEST_OBJECTS= $(MYSQL_HELPER_OBJECTS) $(TEST_DIR)/mysql_helper_test.o
INC=-I/usr/lib/x86_64-linux-gnu/ -I$(SRC_DIR) -I$(INCLUDE_DIR)/mysql

all: query commit delete block_list recent_hashes file_list user_cache_add user_cache_remove

query: $(QUERY_OBJECTS)
	$(CC) $(QUERY_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/block_query.fcgid $(MYSQL_LINK)

commit: $(COMMIT_OBJECTS)
	$(CC) $(COMMIT_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/file_commit.fcgid $(MYSQL_LINK)

delete: $(DELETE_OBJECTS)
	$(CC) $(DELETE_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/file_delete.fcgid $(MYSQL_LINK)

block_list: $(BLOCK_LIST_OBJECTS)
	$(CC) $(BLOCK_LIST_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/block_list.fcgid $(MYSQL_LINK)	

file_list: $(FILE_LIST_OBJECTS)
	$(CC) $(FILE_LIST_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/file_list.fcgid $(MYSQL_LINK)	

recent_hashes: $(RECENT_HASHES_OBJECTS)
	$(CC) $(RECENT_HASHES_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/recent_hashes.fcgid $(MYSQL_LINK)

user_cache_add: $(USER_CACHE_ADD_OBJECTS)
	$(CC) $(USER_CACHE_ADD_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/user_cache_add.fcgid $(MYSQL_LINK)

user_cache_remove: $(USER_CACHE_REMOVE_OBJECTS)
	$(CC) $(USER_CACHE_REMOVE_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/user_cache_remove.fcgid $(MYSQL_LINK)

user_caches: $(USER_CACHES_OBJECTS)
	$(CC) $(USER_CACHES_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/user_caches.fcgid $(MYSQL_LINK)

test: $(TEST_OBJECTS)
	$(CC) $(TEST_OBJECTS) $(LDFLAGS) -o $(TEST_DIR)/mysql_helper_test $(MYSQL_LINK)

install: 
	cp ./bin/* -t /var/www/html/

clean: 
	rm ./bin/*; \
	rm ./obj/*; \
	rm ./test/*.o;

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CC) $(CPPFLAGS) $(INC) -c $< -o $@

$(TEST_DIR)/%.o : $(TEST_DIR)/%.cc
	$(CC) $(CPPFLAGS) $(INC) -c $< -o $@
