#!/bin/bash

#SET THESE
hostname="http://104.236.169.138"
port=""
blache_hostname="http://192.168.1.130"
blache_port=""
user="cs188"


if [ "$#" -ne 1 ]; then
    echo "Please enter 1 parameter: filename (foo.txt) to download"
    exit 1
fi

filename=$1


function checkStatus {
	status_code=$(echo "$1" |  grep "HTTP/" | awk '{print $2}')
	echo -e "HTTP status: $status_code"
	if [[ "$status_code" = "400" ]]
	then
		echo "HTTP Status Code: 400. Check Input"
		exit 1
	fi
	if [[ "$status_code" = "404" ]]
	then
		echo "HTTP Status Code: 404. Not Found"
		exit 1
	fi
}


#Send GET to metaserver to get hashes

echo -e "\nSending HTTP post to metadata..." 
echo -e "curl '${hostname}${port}/block_list.fcgid?user_id=${user}&file_name=${filename}' --header 'Content-Type: application/json' --header 'Accept: application/json'"

response=$(curl '${hostname}${port}/block_list.fcgid?user_id=${user}&file_name=${filename}' --header 'Content-Type: application/json' --header 'Accept: application/json')

sudo echo "${response}" | sudo python -mjson.tool &>./responses/${filename}_dl_response
block_list="$(cat ./responses/${filename}_dl_response | jq '.block_list[]' | sed "s/\"//g")"
echo -e "Hashes need to request from cache:\n${block_list}"


#temp="["

function fetchBlock {
	echo -e "\nSending HTTP request to get block..."
	response_cache=$(curl -i --data-binary "@${file}"  ${hostname}${port}/file_fetch.fcgid?hash=$1)
	echo -e "${response_cache}\n"

	checkStatus "${response_cache}"

}

#Create new file name
#if filename already exists will create filename-1, filename-2, etc. until it creates a file that does not already exit
n=
set -C
until
  	file=$filename${n:+-$n}
  	{ command exec 3> "$newfile"; } 2> /dev/null
do
  	((n++))
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



