#!/bin/bash

cd ~/
git clone https://github.com/google/leveldb.git
cd leveldb
make
sudo mkdir /var/www/html/mydb
sudo chmod 777 /var/www/html/mydb
sudo mkdir ~/temp