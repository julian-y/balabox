# Client scripts specification



#sync.py
#Two parameters
#	-u user id
#	-c cache to use (either cache-lndn.raycoll.me or cache-sfo.raycoll.me)

#Run bootstrap script to install jq
#Both scripts use funtions from defFunctions.sh

## file_upload.sh
* 3 parameters: file to upload, userid, cache (either cache-lndn.raycoll.me or cache-sfo.raycoll.me)
* Example: ./client_upload.sh foo.txt synthia
* Exits if any HTTP responses 400 (upload failed)


## file_download.sh
* 3 parameters: file to upload, userid, cache (either cache-lndn.raycoll.me or cache-sfo.raycoll.me)
* Example: ./client_download.sh foo.txt synthia
* Exits if any HTTP responses returns 400
