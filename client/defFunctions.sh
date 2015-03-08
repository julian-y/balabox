#!/bin/bash

function fileExists {

	if [ ! -f $1 ]; then
    	echo "File $1 not found!"
    	exit 1
	fi
}

function exitUpload {
	#if [ ! -d  ./responses ]; then
    #		`sudo rm -r "./responses"`
	#		echo -e "Deleted responses directory\n"
	#fi
	exit 1
}

function checkStatus { 
	echo "HTTP Status Code: $1"
	if [[ "$1" = "200" ]]
	then
		echo -e "OK\n"
	else
		if [[ "$1" = "400" ]]
		then
			echo "Check Input."	
		elif [[ "$1" = "404" ]]
		then
			echo "Not Found."
	
		elif [[ "$1" = "500" ]]
		then
			echo "Internal Server Error."
		fi
		exit 1
	fi
}