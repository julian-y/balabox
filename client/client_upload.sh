#!/bin/bash

dir="$(dirname "$0")"
source $dir/defFunctions.sh #include functions

#uncomment line below for debugging
#set -vx

hostname="http://104.236.169.138" 
port="" #Format ":<port number>"
blache_hostname="http://192.168.1.130" #SET THIS block/cache to VM IP
blache_port=""

function getJsonResp {

	echo -e "$1\n"
	checkStatus "$1"
	sudo echo "$1" | sudo python -mjson.tool &>$2	
}

function parseJson {
	local jsonVal=`cat $1 | python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' $2`
	echo "$jsonVal"
}

if [ "$#" -ne 2 ]; then
    echo "Please enter 2 parameters: filename (foo.txt) to upload and user_id"
    exit 1
fi

filename=$1
user=$2
fileExists "$filename"

`sudo mkdir -p blocks`
`sudo mkdir -p hashes`
`sudo mkdir -p responses`
`sudo mkdir -p versions`
`sudo chmod 777 responses`
`sudo chmod 777 hashes`
`sudo chmod 777 versions`

`sudo split -b 4m $filename ./blocks/${filename}_`
echo -e "Split $filename into 4MB files in blocks directory...\n"


#Creates one file with all its blocks' hashes
for file in "./blocks/*"
do
	`shasum -a 256 $file  > "./hashes/${filename}_hash"`
done
echo -e "Created hashes in ./hashes/${filename}_hash\n"


#Create list of hashes to send in JSON
list="["
while read in
do
	#remove block file name and append only hash
	list+=" \"$(echo "$in" | awk '{print $1}')\","
done < "./hashes/${filename}_hash"

list=${list%?} #get rid of trailing comma
list+=" ]"


#HTTP POST to metaserver block_query to see what blocks need to be sent to the block server
echo -e "\nSending HTTP post to metadata..." 


response=`curl --header "Content-Type: application/json" --header "Accept: application/json" --data '{ "user_id":"'"$user"'", "file_name":"'"$filename"'", "block_list":'"$list"' }'  "${hostname}${port}/block_query.fcgid"`

getJsonResp "$response" "./responses/${filename}_md_response"
nb=`parseJson "./responses/${filename}_md_response" "nb"`


#Check if client needs to send blocks
if [[ "$nb" = False ]]
then
	echo "File $filename is synced. No blocks need to be sent."
	exitUpload
elif [[ "$nb" != True ]]
then
	echo "Error invalid value for nb"
	exit 1
fi

#parse hashes string then send corresponding blocks to cache server
echo "Need to send blocks for these hashes:"


#sudo apt-get install jq if you don't have it yet installed
needed_blocks="$(cat ./responses/${filename}_md_response | jq '.needed_blocks[]')"

block_list=`cat ./responses/${filename}_md_response | jq '.needed_blocks[]'| sed 's/\"//g'`

#TO-D0: Optimize
#Might change so filename is hash and content is file block name
while read -r line
do
	while read in
	do
		#remove block file name and append only hash
		ha="$(echo "$in" | awk '{print $1}')"

		if [[ "$line" =  "$ha" ]]
		then
			file="$(echo "$in" | awk '{print $2}')"
			
    		fileExists "$file"

			echo -e "\nSending HTTP POST to cache server with binary data for $file..."

			echo "curl below needs to be changed. Not actaully sending data at file. Just sending the filename.."
			response_cache=`curl --data-binary '"@$file"' "$blache_hostname$blache_port/file_store.fcgid?hash=$ha"`
			
			echo -e "$response_cache\n"
			checkStatus "$response_cache"
						
		fi

	done < "./hashes/${filename}_hash"
done <<< "$block_list"

if echo $needed_blocks | sed -e 's/[[:space:]]/, /g'; then 
	hash_send="[ $(echo $needed_blocks | sed -e 's/[[:space:]]/, /g') ]"
else
	hash_send="[ $needed_blocks ]"	
fi

echo -e "\nSending HTTP POST to metadata to commit blocks..." 
#echo $hash_send

	if [ ! -f  "versions/${filename}_version" ]; then
    	echo "First version of file. Version 0"
    	version="0"
    else
    	oldversion=$(head -n 1 versions/${filename}_version)
    	version=$((oldversion+1))
	fi 
response_fc=`curl --header "Content-Type: application/json" --header "Accept: application/json" --data '{ "user_id":"'"$user"'", "file_name":"'"$filename"'", "block_list":'"$hash_send"', "version":"'"$version"'" }' "${hostname}${port}/file_commit.fcgid"`


getJsonResp "$response_fc" "./responses/${filename}_md_response2"
metadata_updated=`parseJson "./responses/${filename}_md_response2" "metadata_updated"`


if [[ "$metadata_updated" = True ]]
then
	echo -e "File $file_name successfully committed to metadata"
	echo -e "Currently version $version"
	`sudo echo "$version" > versions/${filename}_version`
else
	message=`parseJson "./responses/${filename}_md_response2" "message"`
	echo -e "Error committing file $file_name to metadata: ${message}"
	exitUpload
fi


exitUpload 
