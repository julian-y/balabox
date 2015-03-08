#!/bin/bash

dir="$(dirname "$0")"
source $dir/defFunctions.sh #include functions

#uncomment line below for debugging
#set -vx

hostname="http://104.236.169.138" 
port="" #Format ":<port number>"
blache_hostname="http://192.168.1.130" #SET THIS block/cache to VM IP
blache_port=""


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

FILEBLOCKS=blocks/${filename}_blocks
HTTPRESP=responses/${filename}_responses
`sudo mkdir -p $FILEBLOCKS`
`sudo mkdir -p $HTTPRESP`
`sudo chmod 777 $FILEBLOCKS`
`sudo chmod 777 $HTTPRESP`


`sudo split -b 4m $filename $FILEBLOCKS/${filename}_`
echo -e "Split $filename into 4MB files in $FILEBLOCKS ...\n"


#Creates one file with all its blocks' hashes
for file in "$FILEBLOCKS/*"
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

RESPONSE="${HTTPRESP}/block_query_response.txt"
status=`curl -s -w %{http_code} --header "Content-Type: application/json" --header "Accept: application/json" --data '{ "user_id":"'"$user"'", "file_name":"'"$filename"'", "block_list":'"$list"' }'  "${hostname}${port}/block_query.fcgid" -o $RESPONSE`


checkStatus "$status"
echo "block_query Response:"
cat $RESPONSE

nb=`parseJson "$RESPONSE" "nb"`


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
needed_blocks="$(cat $RESPONSE | jq '.needed_blocks[]')"

block_list=`cat $RESPONSE | jq '.needed_blocks[]'| sed 's/\"//g'`
echo $block_list

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

			echo "*******curl below will send error 500 for larger blocks (4 MB blocks not working)*******"

			RESPONSE=${HTTPRESP}/file_store_response
			status=`curl -s -w %{http_code} --data-binary "@$file" "$blache_hostname$blache_port/file_store.fcgid?hash=$ha" -o $RESPONSE`
			
			checkStatus "$status"
			echo "file_store for block $file Response:"
			cat $RESPONSE
						
		fi

	done < "./hashes/${filename}_hash"
done <<< "$block_list"


echo -e "\nSuccessfully retrieved blocks for hashes"
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

RESPONSE=${HTTPRESP}/file_commit_response

status=`curl -s -w %{http_code} --header "Content-Type: application/json" --header "Accept: application/json" --data '{ "user_id":"'"$user"'", "file_name":"'"$filename"'", "block_list":'"$hash_send"', "version":"'"$version"'" }' "${hostname}${port}/file_commit.fcgid" -o $RESPONSE`

checkStatus "$status"
echo "file_store for block $file Response:"
cat $RESPONSE

metadata_updated=`parseJson $RESPONSE "metadata_updated"`


if [[ "$metadata_updated" = True ]]
then
	echo -e "File $file_name successfully committed to metadata"
	echo -e "Currently version $version"
	`sudo echo "$version" > versions/${filename}_version`
else
	message=`parseJson "$RESPONSE" "message"`
	echo -e "Error committing file $file_name to metadata: ${message}"
	exitUpload
fi


exitUpload 
