#!/bin/bash

#echo -e "Enter .cpp file name (without the ".cpp"): \c"

if [ "$#" -ne 1 ]; then
    echo "Only 1 parameter: filename of .cpp (without .cpp)"
    exit 1
fi

name=$1
fcgidName="${name}.fcgid"
fcgidPath="fcgid/${fcgidName}"
jsonlib="/usr/lib/x86_64-linux-gnu/libjsoncpp.so"
compile="g++ ${name}.cpp $jsonlib -o $fcgidPath -lfcgi -l neon"
echo -e "Compiling: $compile"
`$compile`
#`g++ "${name}.cpp" -o $fcgidName -lfcgi`
`sudo rm "/var/www/html/${fcgidName}"`
echo -e "...removed old copy at /var/www/html/"
`sudo cp $fcgidPath "/var/www/html/"`
echo -e "...copying new version"
`sudo chmod 633 "/var/www/html/${fcgidName}"`
echo -e "...added execute and read permissions (633)"
