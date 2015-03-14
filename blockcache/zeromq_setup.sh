#!/bin/bash

# Install dependencies
wget download.zeromq.org/zeromq-4.1.0-rc1.tar.gz
sudo apt-get install build-essential
sudo apt-get install uuid-dev
sudo apt-get install m4
sudo apt-get install libtool
sudo apt-get install pkg-config
sudo apt-get install autoconf
sudo apt-get install automake
# wget http://ftp.gnu.org/gnu/m4/m4-latest.tar.gz
# wget http://ftp.gnu.org/gnu/autoconf/autoconf-latest.tar.gz
# wget http://ftp.gnu.org/gnu/automake/automake-1.15.tar.gz
# wget ftpmirror.gnu.org/libtool/libtool-2.4.6.tar.gz
# wget pkgconfig.freedesktop.org/releases/pkg-config-0.27.1.tar.gz

# Untar
tar -xvzf zeromq-4.1.0-rc1.tar.gz
#tar -xvzf libtool-2.4.6.tar.gz
#tar -xvzf pkg-config-0.27.1.tar.gz
#tar -xvzf autoconf-latest.tar.gz
#tar -xvzf automake-1.15.tar.gz
#tar -xvzf m4-latest.tar.gz

# ZeroMQ
cd zeromq-4.1.0
./configure --prefix=/usr/local && make
sudo make install

# Solve symbolic link problem
cd /usr/local/lib
sudo rm libzmq.so.4
sudo ln -s libzmq.so.4.0.0 libzmq.so.4

cd ~/balabox/blockcache/zeromq-4.1.0
sudo ldconfig

# Move C++ binding
sudo mv zmq.hpp /usr/local/include/