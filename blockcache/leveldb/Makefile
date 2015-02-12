# 1. Clone LevelDB git repo into home directory
# 2. Navigate to inside leveldb/ and run `make`

export LEVELDB_DIR = ~/leveldb

CC = g++ 
OPTS = $(LEVELDB_DIR)/libleveldb.a -lpthread -I $(LEVELDB_DIR)/include 

all: read write
	
read: read.cc
	$(CC) -o read read.cc $(OPTS)

write: write.cc
	$(CC) -o write write.cc $(OPTS)

clean:
	rm -rf read write temp

test: all
	./write
	./read
