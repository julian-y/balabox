#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Please enter 1 parameter: filename (foo.txt) to download"
    exit 1
fi

filename=$1

#SET THESE
hostname="http://localhost"
port="8000"
user="cs188"

#Send GET to metaserver to get hashes


echo -e "\nSending HTTP post to metadata..." 
echo -e "curl  -X GET "${hostname}:${port}/list.fcgid?user_id=${user}&file_name=${filename}" --header 'Content-Type: application/json' --header 'Accept: application/json'"

response=$(curl  -X GET "${hostname}:${port}/list.fcgid?user_id=${user}&file_name=${filename}" --header 'Content-Type: application/json' --header 'Accept: application/json')

sudo echo "${response}" | sudo python -mjson.tool &>./responses/${filename}_dl_response
block_list="$(cat ./responses/${filename}_dl_response | jq '.needed_blocks[]' | sed "s/\"//g")"
echo -e "Hashes need to request from cache:\n${block_list}"


file="["
dne=false

while read -r line
do
	while read in
	do
		#remove block file name and get only hash
		temp="$(echo "$in" | awk '{print $1}')"

		#if the hash is in the file, that means the local file exists
		if [[ "$line" =  "$temp" ]] #
		then
			#check if file still exists
			file="$(echo "$in" | awk '{print $2}')"
			if [ ! -f $file ]; then
    			echo "File not found!"
			fi
		else
			echo -e "\nSending HTTP request to get block..."

			response_cache=$(curl -i --data-binary "@${file}"  ${hostname}:${port}/file_fetch.fcgid)
			echo -e "${response_cache}\n"

			checkStatus "${response_cache}"
			#TO-DO error getting blocks
		fi

	done < "./hashes/${filename}_hash"
done <<< "$block_list"


#TO-DO: reconstruct blocks into file in local file system


