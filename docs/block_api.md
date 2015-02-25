# Blockserver API specification

## /file_store
* url structure: blockserver_ip/file_store
* description: Stores the block hash and binary block pair for a given file.
* method: POST
* request url: ?hash=\<requested_block_hash>
* request body: binary block inside request body
* parameters: 
    * none //TODO: check that this is true
* returns: Acknowledgement (200 OK?) or Error... (500?)
    * return # of bytes in the block

## /file_fetch
* url structure: blockserver_ip/file_fetch
* description: Requests the list of binary blocks based on block hash
* method: GET
* request url: ?hash=\<requested_block_hash>
* request body: none
* returns: binary block inside response

## Note that cache server will have the identical API, but have some different
logic that communicates with block server and/or other caches and metaserver

## Note that binary data should be stored as vector<char>
