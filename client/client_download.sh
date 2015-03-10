#!/bin/bash

dir="$(dirname "$0")"
source $dir/defFunctions.sh

#uncomment line below for debugging
#set -vx


if [ "$#" -ne 2 ]; then
    echo "Please enter 2 parameters: filename (foo.txt) to download and user_id"
    exit 1
fi

filename=$1
user=$2
fileChanged=False

#SET THESE
hostname="http://104.236.169.138"
port=""
blache_hostname="http://104.236.169.138"
blache_port=""

FILEBLOCKS=blocks/${filename}_blocks
HTTPRESP=responses/${filename}_responses
`sudo mkdir -p $FILEBLOCKS`
`sudo mkdir -p $HTTPRESP`
`sudo chmod 777 $FILEBLOCKS`
`sudo chmod 777 $HTTPRESP`


function fetchBlock { #2 parameters: hash and file block

	echo -e "\nSending HTTP request to get block for hash $1 and saving to $2..."
	RESPONSE="${HTTPRESP}/file_fetch_response"
	status=`curl -s -w %{http_code} "${blache_hostname}${blache_port}/cache_file_fetch?hash=$1&user=$user" -o $RESPONSE`

	checkStatus "$status"
	cat $RESPONSE
	resp=$(cat $RESPONSE)
	checkResponse "$resp"
	echo "file_fetch Response:"
	cat $RESPONSE >> $2
}

function blockDNE {
			echo "enter blockDNE"
			#block does not exist in client's filesystem. Write to a new file for this block

			#Creates filename for block
			n=
			set -C
			until
  				new_block_name=$FILEBLOCKS/${filename}_a${n:+-$n} #TO-DO: fix new block naming system
  				{ command exec 3> "$new_block_name"; } 2> /dev/null
			do
  				((n++))
			done

			fetchBlock "$1" "$new_block_name"

			#Adds new block's hash to locally saved hashes for this file
			echo "$1  $new_block_name" >> "./hashes/${filename}_hash"

			cat "$new_block_name" >> "$2"
}

function compHashes {
	echo "enter compHashes\n 1: $1 \n 2: $2"
	while read in
	do
		echo "in while"
		#remove block file name and get only hash
		file_hash="$(echo "$in" | awk '{print $1}')"
		echo "file_hash: $file_hash"
		#if the hash is in the file, that means the local file should exist
		if [[ "$1" =  "$file_hash" ]] #if hash does equals current line
		then
			#check if file for block still exists
			block_name="$(echo "$in" | awk '{print $2}')"
			if [ ! -f "$block_name" ]; then

				#Block matched for this hash does not exist in file system anymore.
    			echo "File $block_name not found in local file system. Must fetch file from cache" 
    			fetchBlock "$file_hash" "$block_name"	

    			#No error, block successfully retrieved
    		fi
    			#Put new block in filename that did not exist
    			echo "Adding $block_name to file"
				cat "$block_name" >> "$2"	
		else
				echo "Called blockDNE from while loop"
				echo "1: $1"
				blockDNE "$1" "$2"
			
		fi

	done < "./hashes/${filename}_hash"
}



#Send GET to metaserver to get hashes
echo -e "\nSending HTTP post to metadata..." 

RESPONSE="${HTTPRESP}/block_list_response"
status=`curl -s -w %{http_code} "${hostname}${port}/block_list?user_id=${user}&file_name=${filename}" --header "Content-Type: application/json" --header "Accept: application/json" -o $RESPONSE`

checkStatus "$status"
echo "block_query Response:"
cat $RESPONSE

#echo "$response" | sudo python -mjson.tool &>./responses/${filename}_dl_response


block_list="$(cat $RESPONSE| jq '.block_list[]' | sed "s/\"//g")"

echo -e "Hashes for this file:\n${block_list}"


#Create new file name
n=
set -C
until
  	NEWFILE=${filename}_new${n:+-$n}
  	{ command exec 3> "$NEWFILE"; } 2> /dev/null
do
  	((n++))
done
touch "$NEWFILE"

if [ ! -f ./hashes/${filename}_hash ]; then
	echo "No hashes for this file"
    while read -r block_list_hash
	do
		echo "calling blockDNE"
		blockDNE "$block_list_hash" "$NEWFILE" 
	done <<< "$block_list"   	
else 
	while read -r block_list_hash
	do
	addedBlock=False
	while read line
	do
		echo "in while"
		#remove block file name and get only hash
		file_hash="$(echo "$line" | awk '{print $1}')"
		echo "file_hash: $file_hash"
		#if the hash is in the file, that means the local file should exist
		if [[ "$block_list_hash" =  "$file_hash" ]] #if hash equals current line
		then
			#check if file for block name associated with hash still exists
			block_name="$(echo "$line" | awk '{print $2}')"
			if [ ! -f "$block_name" ]; then

				#Block matched for this hash does not exist in file system anymore.
    			echo "File $block_name not found in local file system. Must fetch file from cache" 
    			fetchBlock "$file_hash" "$block_name"	

    			#No error, block successfully retrieved
    		fi
    			
    		#Put new block in filename that did not exist
    		echo "Adding $block_name to file"
			cat "$block_name" >> "$NEWFILE" 	
			addedBlock=True			
		fi
	done < "./hashes/${filename}_hash"
		if [[ "$addedBlock" = False ]]; then
			echo "Called blockDNE from while loop"
			blockDNE "$block_list_hash" "$NEWFILE" 
		fi
	done <<< "$block_list" 
    
fi

#echo "Updated file $filename content:"
#cat $NEWFILE


#replace original file with new one
#`sudo mv "$NEWFILE" "$filename`




