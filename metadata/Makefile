# 
# 

CC=g++
CPPFLAGS=-Wall
LDFLAGS=

COMMIT_SOURCES=./src/file_commit.cc
QUERY_SOURCES=./src/file_query.cc
QUERY_OBJECTS=./obj/file_query.o
COMMIT_OBJECTS=./obj/file_commit.o

OBJ_DIR=./obj
BINARY_DIR=./bin
SRC_DIR=./src
INC=-I/usr/lib/x86_64-linux-gnu/

all: query commit

query: $(QUERY_OBJECTS)
    $(CC) $(CPPFLAGS) $(QUERY_OBJECTS) -o $(BINARY_DIR)/file_query

commit: $(COMMIT_OBJECTS)
   $(CC) $(CPPFLAGS) $(COMMIT_OBJECTS) -o $(BINARY_DIR)/file_commit

install: 
    cp ./bin/file_commit  ./bin/file_query -t /var/www/html/

clean: 
    rm ./bin/* && rm ./obj/*

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cc
        $(CC) $(CPPFLAGS) $(INC) -c $< -o $@