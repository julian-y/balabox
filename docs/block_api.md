# Blockserver API specification

## /block_store
* URL Structure: blockserver_ip/block_store
* Description: Stores the block hash and binary block pair for a given file.
* Method: POST
* Request URL: ?hash=\<requested_block_hash>
* Request Body: binary block inside request body
* Response HTTP Status Code: OK (200) upon success or Bad Request (400) upon failure
* Response Body: HTML document containing the block hash, block data, and IP of the block server upon 200, otherwise
                 HTML document containing the error message
* Request Sample: curl --data-binary "@test.txt" "blockserver_ip/block_store?hash=hello"

## /block_fetch
* URL Structure: blockserver_ip/block_fetch
* Description: Requests the block data associated with the block hash.
* Method: GET
* Request URL: ?hash=\<requested_block_hash>
* Request Body: None
* Response HTTP Status Code: OK (200) upon success or Bad Request (400) upon failure
* Response Headers: Content-Type: application/binary, Content-Length: \<# of bytes of block data>
* Response Body: Block data
* Request Sample: curl "blockserver_ip/block_fetch?hash=hello"

## Note that cache server will have the identical API, but have some different
logic that communicates with block server and/or other caches and metaserver
