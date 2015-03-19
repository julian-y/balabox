#!/bin/bash
curl -OL https://github.com/kennethreitz/requests/tarball/master
tar -xzf master
cd kennethreitz-requests*
sudo python setup.py install
cd ..

#install jq for MACOSX
#`ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
#`brew install jq`

#OR
#`sudo apt-get install jq`
