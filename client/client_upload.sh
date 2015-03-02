#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Please enter 1 parameter: filename (foo.txt) to upload"
    exit 1
fi

filename=$1

#SET THESE
hostname="http://localhost"
port="8000"

#TO DO create more functions to make script more readable...

function checkStatus {
	status_code=$(echo "$1" |  grep "HTTP/" | awk '{print $2}')
	echo -e "HTTP status: $status_code"
	if [[ "$status_code" = "400" ]]
	then
		echo "HTTP Status Code: 400. Check Input"
		exit 1
	fi
	#TO-DO catch more statuses 
}


`sudo mkdir -p temp`
`sudo mkdir -p hashes`
`sudo mkdir -p responses`

`sudo split -b 4m $filename ./temp/${filename}_`
echo -e "Split $filename into 4MB files in temp directory...\n"


#Creates one file with all its blocks' hashes
for file in "./temp/*"
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
#url="http://192.168.1.130/block_query.fcgid" 
user="cs188@balabox" #change this to parameter or function

echo -e "\nSending HTTP post to metadata..." 
echo -e "curl -i -X POST -H "Content-Type:application/json" -H "Accept: application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"block_list\":${list} }'  ${hostname}:${port}/block_query.fcgid\n"

response=$(curl -i -X POST -H "Content-Type:application/json" -H "Accept: application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"block_list\":${list} }'  ${hostname}:${port}/block_query.fcgid)


echo -e "${response}\n"

checkStatus "${response}"


sudo echo "${response}" | sudo python -mjson.tool &>./responses/${filename}_md_response



nb=$(cat ./responses/${filename}_md_response | python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' nb)


#Check if client needs to send blocks
if [[ "$nb" = False ]]
then
	echo "File is synced. No blocks need to be sent."
	exit 1
fi

#parse hashes string then send corresponding blocks to cache server
echo "Need to send blocks for these hashes:"


#sudo apt-get install jq if you don't have it yet installed

needed_blocks="$(cat ./responses/${filename}_md_response | jq '.needed_blocks[]' | sed "s/\"//g")"
echo -e "$needed_blocks"


#TO-D0: Optimize this...
while read -r line
do
	while read in
	do
		#remove block file name and append only hash
		temp="$(echo "$in" | awk '{print $1}')"
		if [[ "$line" =  "$temp" ]]
		then

			file="$(echo "$in" | awk '{print $2}')"
			if [ ! -f $file ]; then
    			echo "File not found!"
			fi

			echo -e "\nSending HTTP POST to cache server with binary data for $file..."

			response_cache=$(curl -i -X POST --data-binary "@${file}"  ${hostname}:${port}/file_store.fcgid)
			echo -e "${response_cache}\n"

			checkStatus "${response_cache}"
			#TO-DO handle error sending blocks
		fi

	done < "./hashes/${filename}_hash"
done <<< "$needed_blocks"


echo -e "\nSending HTTP POST to metadata to commit blocks..." 
hash_send="[$(cat ./responses/${filename}_md_response | jq '.needed_blocks[]'| sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/, /g')]"
#echo "$hash_send"
echo -e "curl -i -X POST -H "Content-Type:application/json" -H "Accept: application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"block_list\":${hash_send} }'  ${hostname}:${port}/file_commit.fcgid\n"


response_fc=$(curl -i -X POST -H "Content-Type:application/json" -H "Accept: application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"block_list\":${hash_send} }'  ${hostname}:${port}/file_commit.fcgid)

echo -e "${response_fc}\n"

checkStatus "${response_fc}"

sudo echo "${response}" | sudo python -mjson.tool &>./responses/${filename}_md_response2


metadata_updated=$(cat ./responses/${filename}_md_response2 | python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' metadata_updated)

if [[ "$metadata_updated" = True ]]
then
	echo -e "File $file_name successfully committed to metadata\n"
else
	echo -e "Error committing file $file_name to metadata"
	exit 1
	#resend?
fi


`sudo rm -r "./temp"`
echo -e "Deleted temp directory\n"
