#!/bin/bash

dir="$(dirname "$0")"
source $dir/defFunctions.sh

#uncomment line below for debugging
#set -vx

#SET THESE
hostname="http://104.236.169.138"
port=""
blache_hostname="http://192.168.1.130"
blache_port=""

function fetchBlock {

	echo -e "\nSending HTTP request to get block..."
	response_cache=`curl --data-binary '"@${file}"' "${blache_hostname}${blache_port}/file_fetch.fcgid?hash=$1" `
	echo -e "${response_cache}\n"

	checkStatus "${response_cache}"

}

if [ "$#" -ne 2 ]; then
    echo "Please enter 2 parameters: filename (foo.txt) to download and user_id"
    exit 1
fi

filename=$1
user=$2


#Send GET to metaserver to get hashes

echo -e "\nSending HTTP post to metadata..." 

response=`curl "${hostname}${port}/block_list.fcgid?user_id=${user}&file_name=${filename}" --header "Content-Type: application/json" --header "Accept: application/json" `

checkStatus "$response"

echo "$response" | sudo python -mjson.tool &>./responses/${filename}_dl_response


block_list="$(cat ./responses/${filename}_dl_response | jq '.block_list[]' | sed "s/\"//g")"

echo -e "Hashes need to request from cache:\n${block_list}"


#Create new file name
#if filename already exists will create filename-1, filename-2, etc. until it creates a file that does not already exit
n=
set -C
until
  	newfile=$filename${n:+-$n}
  	{ command exec 3> "$newfile"; } 2> /dev/null
do
  	((n++))
  	echo "hi"
done

touch "$newfile"

while read -r line
do
	while read in
	do
		#remove block file name and get only hash
		ha="$(echo "$in" | awk '{print $1}')"

		#if the hash is in the file, that means the local file should exist
		if [[ "$line" =  "$ha" ]] #
		then
			#check if file still exists
			file="$(echo "$in" | awk '{print $2}')"
			if [ ! -f $file ]; then

				#Block matched for this hash does not exist in file sysetm anymore.
    			echo "File ${file} not found!" 
    			fetchBlock "${ha}"

    			#No error, block successfully retrieved

    			#Put new block in filename that did not exist
    			echo "$block" >> "$file"	
			fi
			cat "$file" >> "$newfile"
		else
			fetchBlock "${ha}"

			#No error, block successfully retrieved
    		echo "$block" >> "$newfile"

			#Creates filename for block
			n=
			set -C
			until
  				file="${filename}_aa${n:+-$n}" #TO-DO: fix new block naming system
  				{ command exec 3> "$file"; } 2> /dev/null
			do
  				((n++))
			done
			
			#Adds new block's hash to locally saved hashes for this file
			echo "${ha}  ${file}" >> "./hashes/${filename}_hash"

			cat "$file" >> "$newfile"
		fi

	done < "./hashes/${filename}_hash"
done <<< "$block_list"



