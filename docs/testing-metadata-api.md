Testing recent_hashes.fcgid
	Assumption: the client only sends in the 2 specified parameters

	// normal case
	request:
		curl  -X GET "http://localhost:8000/recent_hashes.fcgid?user_id=steven&max_hashes=2" --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		{
		   "block_list" : [
		      "c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe",
		      "c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbfff"
		   ]
		}

	// normal case
	request: 
		curl  -X GET "http://localhost:8000/recent_hashes.fcgid?user_id=xtina&max_hashes=1" --header 'Content-Type: application/json' --header 'Accept: application/json'
	
	response:
		{
		   "block_list" : [ "c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbdfe" ]
		}

	// nonexistent user 
	request:
		curl  -X GET "http://localhost:8000/recent_hashes.fcgid?user_id=heh&max_hashes=1" --header 'Content-Type: application/json' --header 'Accept: application/json'
		
	response:	
		{
		   "block_list" : null
		}

	// invalid parameter
	request:
		curl  -X GET "http://localhost:8000/recent_hashes.fcgid?hmmm=heh&max_hashes=1" --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		<html><p>400 INVALID INPUT</p></html>

Testing list.fcgid
	Assumption: the client only sends in the 2 specified parameters 
	
	// normal case
	request:	
		curl  -X GET "http://localhost:8000/list.fcgid?user_id=steven&file_name=testfile" --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		{
		   "block_list" : [
		      "c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe",
		      "c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbfff"
		   ]
		}

	// nonexistent user
	request: 
		curl  -X GET "http://localhost:8000/list.fcgid?user_id=aergi&file_name=testfile" --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		{
		   "block_list" : null
		}

	// invalid parameters	
	request:
		 curl  -X GET "http://localhost:8000/recent_hashes.fcgid?wallala=steven&max_hashes=2" --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		<html><p>400 INVALID INPUT</p></html>


Testing block_query.fcgid
	// normal case: the hashes are already in the database
	request:
		curl -i  --data '{"user_id":"123444", "file_name":"rawr" , "block_list":["c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe", "c7c084318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbfff"]}' http://localhost:8000/block_query.fcgid --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		HTTP/1.1 200 OK
		Date: Mon, 23 Feb 2015 07:22:02 GMT
		Server: Apache/2.4.7 (Ubuntu)
		Vary: Accept-Encoding
		Transfer-Encoding: chunked
		Content-Type: text/html

		{
		   "nb" : false
		}

	// normal case: the hashes are not yet in the database
	request:
		curl -i  --data '{"user_id":"xtina", "file_name":"meow" , "block_list":["c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe", "c7c084318b6f1bece6f74ffce1ea53596070345272dee804003749557d4cbfff"]}' http://localhost:8000/block_query.fcgid --header 'Content-Type: application/json' --header 'Accept: application/json'

	response: 
		HTTP/1.1 200 OK
		Date: Mon, 23 Feb 2015 07:24:37 GMT
		Server: Apache/2.4.7 (Ubuntu)
		Vary: Accept-Encoding
		Transfer-Encoding: chunked
		Content-Type: text/html

		{
		   "needed_blocks" : [
		      "c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe",
		      "c7c084318b6f1bece6f74ffce1ea53596070345272dee804003749557d4cbfff"
		   ],
		   "nb" : true
		}

	//missing one input
	request: 
		curl -i  --data '{"user_id":"xtina", "block_list":["c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbffe", "c7c084318b6f1bece6f74ffce1ea53596070345272dee804003749557d4cbfff"]}' http://localhost:8000/block_query.fcgid --header 'Content-Type: application/json' --header 'Accept: application/json'

	response:
		HTTP/1.1 400 Bad Request
		Date: Mon, 23 Feb 2015 07:26:55 GMT
		Server: Apache/2.4.7 (Ubuntu)
		Connection: close
		Transfer-Encoding: chunked
		Content-Type: text/html

		<html><p>400 INVALID INPUT</p></html>

Testing file_commit.fcgid
	How to test: Should be tested with block_query. First commit files and query to see if those recenty added hashes are in the database (nb should be false)

	// normal case
	request: 
		curl -i  --data '{"user_id":"xtina", "file_name":"meow" , "block_list":["c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbdfe", "c99084318b6f1bece6f74ffce1ea53596070345272dee804003749557d4cbfff"]}' http://localhost:8000/file_commit.fcgid --header 'Content-Type: application/json' --header 'Accept: application/json'

	response: 
		HTTP/1.1 200 OK
		Date: Mon, 23 Feb 2015 07:34:48 GMT
		Server: Apache/2.4.7 (Ubuntu)
		Vary: Accept-Encoding
		Transfer-Encoding: chunked
		Content-Type: text/html

		{
		   "metadata_updated" : true
		}

	//missing one input
	request:
		 curl -i  --data '{ "file_name":"meow" , "block_list":["c7c094318b6f1bece6f74ffce1ea53596070345272dee8040037497c7d4cbdfe", "c99084318b6f1bece6f74ffce1ea53596070345272dee804003749557d4cbfff"]}' http://localhost:8000/file_commit.fcgid --header 'Content-Type: application/json' --header 'Accept: application/json'
	
	response:
		HTTP/1.1 400 Bad Request
		Date: Mon, 23 Feb 2015 07:37:00 GMT
		Server: Apache/2.4.7 (Ubuntu)
		Connection: close
		Transfer-Encoding: chunked
		Content-Type: text/html

		<html><p>400 INVALID INPUT</p></html>
