# 
# 

CC=g++
CPPFLAGS=-Wall
LDFLAGS=`mysql_config --cflags --libs`

OBJ_DIR=./obj
BINARY_DIR=./bin
SRC_DIR=./src
TEST_DIR=./test
INCLUDE_DIR=./include

COMMIT_SOURCES=$(SRC_DIR)/file_commit.cc
QUERY_SOURCES=$(SRC_DIR)/file_query.cc
MYSQL_HELPER_OBJECTS=$(OBJ_DIR)/mysql_helper.o
QUERY_OBJECTS=$(OBJ_DIR)/file_query.o $(MYSQL_HELPER_OBJECTS)
COMMIT_OBJECTS=$(OBJ_DIR)/file_commit.o $(MYSQL_HELPER_OBJECTS)
TEST_OBJECTS= $(MYSQL_HELPER_OBJECTS) $(TEST_DIR)/mysql_helper_test.o
INC=-I/usr/lib/x86_64-linux-gnu/ -I$(SRC_DIR) -I$(INCLUDE_DIR)/mysql

all: query commit

query: $(QUERY_OBJECTS)
	$(CC) $(QUERY_OBJECTS) -o $(BINARY_DIR)/file_query $(LDFLAGS)

commit: $(COMMIT_OBJECTS)
	$(CC) $(COMMIT_OBJECTS) -o $(BINARY_DIR)/file_commit $(LDFLAGS)

test: $(TEST_OBJECTS)
	$(CC) $(TEST_OBJECTS) -o $(BINARY_DIR)/mysql_helper_test $(LDFLAGS)

install: 
	cp ./bin/file_commit  ./bin/file_query -t /var/www/html/

clean: 
	rm ./bin/*;
	rm ./obj/*;
	rm ./test/*.o;

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
	$(CC) $(CPPFLAGS) $(INC) -c $< -o $@

$(TEST_DIR)/%.o : $(TEST_DIR)/%.cc
	$(CC) $(CPPFLAGS) $(INC) -c $< -o $@
