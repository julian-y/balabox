# Metaserver API specification

## /block_query
* url structure: metaserver_ip/block_query
* description: Compares a user's block hashes with the server's block hashes 
* method: POST
* request body: JSON document with the block hashes of the file
  * file_name: file to compare
  * user_id: id of user 
  * block_list: list of SHA-256 hash hex strings
* returns: JSON document 
  * nb: true or false depending on if the server requires the client to upload blocks
  * needed_blocks: a list of SHA-256 block hashes that need to be uploaded to block server  
* error codes:
  * 400: invalid input
  
## /file_commit
* url structure: metaserver_ip/file_commit
* description: Commits updated block hashes for a file.  
  * precondition: Client should have previously issued a /file_comp request and uploaded missing blocks to block servers. 
* method: POST
* request body: JSON document with the block list of the file
  * user_id: id of user
  * file_name: file to commit updates to  
  * block_list: list of SHA-256 hash hex strings
  * version: the version of the file that the client would like to commit. Must be 1 greater than the version of the file on the              metaserver. 
* returns: JSON document containing update statuses
  * metadata_updated: true or false depending on whether the hashes are inserted into the database
  * message: message indicates why an operation fails if metadata_updated is false. Not present if medata_updated is true.
* error codes:
  * 400: invalid input
  
## /block_list
* url structure: metaserver_ip/block_list
* description: Retrieves a list of a user file's associated block hashes.   
* method: GET
* parameters: 
  * user_id: id of user
  * file_name : name of the file that is requested
* returns: JSON document containing a user's file block hashes and their version
  * block_list: array of block hashes for a given file and user
  * version: the most recent version of the file recorded on the metaserver 
* error codes:
  * 400: invalid input

## /file_list
* url structure: metaserver_ip/file_list
* description: Retrieves a list of all the file names for a given user   
* method: GET
* parameters: 
  * user_id: id of user
* returns: JSON document containing a user's file names
  * files: array of file names
* error codes:
  * 400: invalid input

## /recent_hashes
* url structure: metaserver_ip/recent_hashes
* description: Retrieves a list of most recently updated first block hashes for a user's files 
* method: GET
* parameters: 
  * user_id: id of user
  * max_hashes: maximum number of first block hashes to retrieve
* returns: JSON document containing a user's file block hashes 
  * block_list: array of block hashes for a given user 
* error codes:
  * 400: invalid input 

## /user_caches
* url structure: metaserver_ip/user_caches
* description: Gets IP adresses of recent caches for a user
* method: GET
* parameters: 
  * user_id: id of user
  * max: max number of caches to return
* returns: JSON document containing a list of cache server ip addresses that recently served the user
  * caches: array of cache server ip addresses
* error codes:
  * 400: No entry for user 

## /user_cache_add
* url structure: metaserver_ip/user_cache_add
* description: Adds an association between a cache and a user
* method: PUT
* parameters: 
  * user_id: id of user
  * cache_ip: ip address of cache to add
* error codes:
  * 400: Invalid input

## /user_cache_remove
* url structure: metaserver_ip/user_cache_remove
* description: Removes a cache from the list of caches associated with a user
* method: DELETE
* parameters: 
  * user_id: id of user
  * cache_ip: ip address of cache to remove
* error codes:
  * 400: Invalid input
