/* Creates the mysql database tables that will store the distributed
 * filesystem metadata
 *
 * We don't actually do any joins, so maybe mysql isn't the best choice 
 */

/* Map each file to its block hashes. 
 * I wish we could store lists in sql
 */
CREATE TABLE IF NOT EXISTS FileBlock (
    user_id VARCHAR(100) NOT NULL,
    file_name VARCHAR(100) NOT NULL,
    block_hash CHAR(64) NOT NULL, /* SHA-256 hex string of block hash */
    block_number INT UNSIGNED NOT NULL, /* block order is important for files */
    time_last_accessed TIMESTAMP NOT NULL
);

/* This info would be more suited to be stored in something like Redis/Memcached 
 * IP SHOULD be stored in network order
 */
CREATE TABLE IF NOT EXISTS BlockCacheIP (
    block_hash BINARY(32) NOT NULL,
    cache_server_ip INT UNSIGNED NOT NULL    
);
 
