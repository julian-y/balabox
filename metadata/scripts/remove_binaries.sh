#!/bin/bash
BINARIES=(block_query file_commit file_delete block_list recent_hashes file_list user_cache_add user_cache_remove user_caches)

for file in "${BINARIES[@]}"
do
  echo "removing $file...";
  rm /var/www/html/$file;
done 

