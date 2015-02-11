# Blockserver API specification

## /file_store
* url structure: blockserver_ip/file_store
* description: Stores the list of block hash and binary block pairs for a given
*              file.
* method: POST
* request body: JSON document with the block pairs of the file 
* parameters: 
    * none //TODO: check that this is true
* returns: Acknowledgement (200 OK?) or Error... (500?)

## /file_fetch
* url structure: blockserver_ip/file_fetch
* description: Requests the list of binary blocks based on block hash
* method: GET
* request body: JSON document with the list of block hashes requested
* returns: JSON document of the blocks // possibly use protobuffers?

## Note that cache server will have the identical API, but have some different
logic that communicates with block server and/or other caches and metaserver
