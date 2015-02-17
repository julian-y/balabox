#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Please enter 1 parameter: filename (foo.txt) to upload"
    exit 1
fi

filename=$1

`sudo mkdir -p temp`
`sudo mkdir -p hashes`
`sudo mkdir -p reponses`

`sudo split -b 4m $filename ./temp/${filename}_`
echo "Split $filename into 4m files in temp directory"


#Creates one file with a each hash as line of the file
for file in "./temp/*"
do
	`shasum -a 256 $file > "./hashes/${filename}_hash"`
done
echo "Created hashes in ./hashes/${filename}_hash"


#Create list of hashes to send in JSON
list="["
while read in
do
 list+=" \"$in\","
done < "./hashes/${filename}_hash"
list=${list%?} #get rid of trailing comma
list+=" ]"

#Set balabox.com reference at /private/etc/hosts after changing network of vm to "Briding adapter"
#    Instructions: http://www.tekrevue.com/tip/edit-hosts-file-mac-os-x/
url="http://balabox.com/block_query.fcgi"
user="cs188@balabox" #change this to parameter or function

echo "Sending HTTP post to metadata" 
echo "curl -i -X POST -H "Content-Type:application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"jsonHashes\":${list} }'  $url"

response=$(sudo curl -i -X POST -H "Content-Type:application/json" -d '{ \"user_id\":\"${user}\", \"file_name\":\"${filename}\", \"jsonHashes\":${list} }'  $url)
echo "${response}"


status_code=$(echo "$response" | sed -n '$p')
if [[ "$status_code" = "400" ]]
then
	echo "HTTP Status Code: 400. Check Input"
	exit 1
fi
#TO-DO catch other status codes


sudo echo "${response}" | sudo python -mjson.tool #&>./responses/${filename}_response

#Might change to jsawk https://github.com/micha/jsawk instead of using python

#Currently using example JSON object NOT the JSON object from HTTP response!

#nb="$(cat ./responses/${filename}_response| python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' nb)"
nb="$(cat ./responses/exampleresponse | python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' nb)"

#Check if client needs to send blocks
if [[ "$nb" = "false" ]]
then
	echo "No blocks need to be sent"
else
	echo "Need to send blocks for these hashes:"

	#hashes="$(cat ./responses/${filename}_response| python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' hashes)"
	hashes="$(cat ./responses/exampleresponse | python -c 'import sys, json; print json.load(sys.stdin)[sys.argv[1]]' hashes)"
	echo "${hashes}"
fi

#TO-DO parse hashes string into array then send corresponding block to metadata
#echo "${hashes}" | perl -nle"print $& if m{[,\s']+\K.*?(?=')}"


`sudo rm -r "./temp"`
echo "Deleted temp directory"
