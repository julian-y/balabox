# Metaserver API specification

## /file_comp
* url structure: metaserver_ip/file_comp
* description: Compares a user's block hashes with the server's block hashes for a given file. 
* method: GET
* request body: JSON document with the block hashes of the file
  * block_list: list of SHA-256 hash values
* parameters: 
  * file_name: file to compare
  * user_id: id of user 
* returns: JSON document 
  * nb: true or false depending on if the server requires the client to upload blocks
  * needed_blocks: a list of SHA-256 block hashes that need to be uploaded to block server  

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
  * block_list: 
  * last_modified: time when the update occurred  

