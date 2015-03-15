#!/bin/bash
# This script installs all the necessary dependencies to run the metaserver

# make sure these are installed 
apt-get -y update
apt-get -y install gcc
apt-get -y install g++
apt-get -y install make
apt-get -y install git

# install apache2
echo "Installing Apache web server....."
apt-get -y install apache2

# copy apache config
cp ../blockcache/apache2.conf /etc/apache2/apache2.conf


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

apt-get -y install libapache2-mod-fcgid

# install mysql 
echo "Installing MySQL......"
apt-get -y install mysql-server
apt-get -y install libmysqlclient-dev

# install other utilities 
echo "Installing Utilities....."
apt-get -y install libjsoncpp-dev

# setup mysql 
mysql -u root -p < scripts/setup_mysql.sql
mysql test -u cs188 < scripts/create.sql

# install metaserver cgi binaries
make
make install

sudo service apache2 restart
