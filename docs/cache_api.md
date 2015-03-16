# Blockserver API specification

## /cache_block_store
* url structure: cache_server_ip/cache_block_store
* description: Stores the block hash and binary block pair for a given file.
* method: POST
* request url: ?hash=<requested_block_hash>&user=<userid>
* request body: binary block inside request body
* parameters: 
    * none //TODO: check that this is true
* returns: Acknowledgement (200 OK?) or Error... (500?)
    * return # of bytes in the block

## /cache_block_fetch
* url structure: cache_server_ip/cache_block_fetch
* description: Requests the list of binary blocks based on block hash
* method: GET
* request url: ?hash=<requested_block_hash>&user=<userid>
* request body: none
* returns: binary block inside response

## Note that cache server will have almost identical API, but have some different
logic that communicates with block server and/or other caches and metaserver

## Note that binary data should be stored as c++ string
