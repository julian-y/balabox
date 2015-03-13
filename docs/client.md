# Client scripts specification

#Run bootstrap script to install jq
#Both scripts use funtions from defFunctions.sh

## client_upload
# FIRST change the blache_hostname to your VM IP becuase block/cache is not on the droplet
* 2 parameters: file to upload and userid
* Example: ./client_upload.sh foo.txt synthia
# eventually hostname and port should be same for both metaserver and block/cache
* metaserver hostname: currently droplet IP "http://104.236.169.138"
* metaserver port: currently empty string. If port needed, set it to the colon followed by the port number like this ":8080"
* block/cache hostname: CHANGE THIS to VM IP (use infconfig) i.e. "http://192.168.1.130"
* block/cache hostname: currently empty string. If port needed, set it to the colon followed by the port like this ":8080"
* Exits if any HTTP responses returns 400


## client_upload
# FIRST change the blache_hostname to your VM IP becuase block/cache is not on the droplet
* 2 parameters: file to download and userid
* Example: ./client_download.sh foo.txt synthia
# eventually hostname and port should be same for both metaserver and block/cache
* metaserver hostname: currently droplet IP "http://104.236.169.138"
* metaserver port: currently empty string. If port needed, set it to the colon followed by the port number like this ":8080"
* block/cache hostname: CHANGE THIS to VM IP (use infconfig) i.e. "http://192.168.1.130"
* block/cache hostname: currently empty string. If port needed, set it to the colon followed by the port like this ":8080"
* Exits if any HTTP responses returns 400
