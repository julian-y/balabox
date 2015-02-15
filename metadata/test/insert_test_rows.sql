/** 
 * Inserts some dummy data into metaserver mysql db for testing
 */

INSERT INTO FileBlock(user_id,file_name,block_hash,block_number) 
VALUES('steven','testfile','c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe',0);


INSERT INTO FileBlock(user_id,file_name,block_hash,block_number) 
VALUES('steven','testfile','c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe',1);

INSERT INTO FileBlock(user_id,file_name,block_hash,block_number) 
VALUES('smartguy','dumbfile','9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08',0);
