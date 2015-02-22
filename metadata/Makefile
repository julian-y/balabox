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
QUERY_SOURCES=$(SRC_DIR)/block_query.cc
LIST_SOURCES=$(SRC_DIR)/list.cc
RECENT_HASHES_SOURCES=$(SRC_DIR)/recent_hashes.cc
MYSQL_HELPER_OBJECTS=$(OBJ_DIR)/mysql_helper.o
QUERY_OBJECTS=$(OBJ_DIR)/block_query.o $(MYSQL_HELPER_OBJECTS)
COMMIT_OBJECTS=$(OBJ_DIR)/file_commit.o $(MYSQL_HELPER_OBJECTS)
LIST_OBJECTS=$(OBJ_DIR)/list.o $(MYSQL_HELPER_OBJECTS)
RECENT_HASHES_OBJECTS=$(OBJ_DIR)/recent_hashes.o $(MYSQL_HELPER_OBJECTS)
TEST_OBJECTS= $(MYSQL_HELPER_OBJECTS) $(TEST_DIR)/mysql_helper_test.o
INC=-I/usr/lib/x86_64-linux-gnu/ -I$(SRC_DIR) -I$(INCLUDE_DIR)/mysql

all: query commit list recent_hashes

query: $(QUERY_OBJECTS)
	$(CC) $(QUERY_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/block_query.fcgid $(MYSQL_LINK)

commit: $(COMMIT_OBJECTS)
	$(CC) $(COMMIT_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/file_commit.fcgid $(MYSQL_LINK)

list: $(LIST_OBJECTS)
	$(CC) $(LIST_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/list.fcgid $(MYSQL_LINK)	

recent_hashes: $(RECENT_HASHES_OBJECTS)
	$(CC) $(RECENT_HASHES_OBJECTS) $(LDFLAGS) -o $(BINARY_DIR)/recent_hashes.fcgid $(MYSQL_LINK)

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
