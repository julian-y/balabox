#!/bin/bash
# This script installs all dependencies required to run the blockserver/cacheserver

apt-get -y update

# make sure these are installed first
apt-get -y install gcc
apt-get -y install g++
apt-get -y install make
apt-get -y install git

# install apache2
echo "Installing Apache web server....."
apt-get -y install apache2

# copy apache config
cp ./apache2.conf /etc/apache2/apache2.conf

# install fcgi 
echo "Installing FastCGI....."
wget http://www.fastcgi.com/dist/fcgi.tar.gz;
tar -xzvf fcgi.tar.gz;
cd fcgi-2.4.1-SNAP-0311112127;
./configure

# fix bug in fcgi source
printf "#include <stdio.h>\n" > tmp;
cp libfcgi/fcgio.cpp ./;
cat fcgio.cpp >> tmp;
mv tmp libfcgi/fcgio.cpp;
rm fcgio.cpp;

make
make install
cd ..
rm fcgi.tar.gz;
rm -rf fcgi-2.4.1*;

apt-get -y install libapache2-mod-fcgid

# install other utilities 
echo "Installing Utilities....."
apt-get -y install libjsoncpp-dev
apt-get -y install libneon27-gnutls-dev 

# install leveldb
git clone https://github.com/google/leveldb.git
cd leveldb
make
mkdir /var/www/html/mydb
chmod 777 /var/www/html/mydb
cd ..
rm -rf leveldb

sudo service apache2 restart
