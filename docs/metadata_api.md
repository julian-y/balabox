# Metaserver API specification

## /block_query
* url structure: metaserver_ip/block_query
* description: Compares a user's block hashes with the server's block hashes 
* method: POST
* request body: JSON document with the block hashes of the file
  * block_list: list of SHA-256 hash values
* parameters: 
  * file_name: file to compare
  * user_id: id of user 
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
  * block_list: list of SHA-256 hash values
* parameters: 
  * user_id: id of user
  * file_name: file to commit updates to 
* returns: JSON document containing updated fields
  * metadata_updated: true or false depending on whether the hashes are inserted into the database
  * last_modified: time when the update occurred  
* error codes:
  * 400: invalid input
  
## /list
* url structure: metaserver_ip/list
* description: Retrieves a list of a user file's associated block hashes.   
* method: GET
* parameters: 
  * user_id: id of user
  * file_name : name of the file that is requested
* returns: JSON document containing a users file block hashes 
  * block_list: array of block hashes for a given file and user
* error codes:
  * 400: invalid input
 
