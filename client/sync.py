#!/usr/bin/env python
import sys
import os
import argparse
import subprocess
import hashlib
import json
import requests
import fileinput
import time
import datetime
from itertools import count

BUF_SIZE = 65536

failed_files=[]

#Check if directory-to-sync parameter is a valid directory
#NOTE: Not using this currently. Now just doing directory where script is located
class readable_dir(argparse.Action): 
    def __call__(self,parser, namespace, values, option_string=None):
        prospective_dir=values
        if not os.path.isdir(prospective_dir):
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a valid path".format(prospective_dir))
        if os.access(prospective_dir, os.R_OK):
            setattr(namespace,self.dest,prospective_dir)
        else:
            raise argparse.ArgumentTypeError("readable_dir:{0} is not a readable dir".format(prospective_dir))

parser=argparse.ArgumentParser(description='Sync files in folder.')
parser.add_argument('-c', help="cache hostname such as cache-lndn.raycoll.me or cache-sfo.raycoll.me", required=True)
parser.add_argument('-u', help="client username", required=True)
#parser.add_argument('-d', action=readable_dir, required=True)

args=parser.parse_args()
user= args.u
cache_host= args.c
#sync_dir= args.d


#Does not throw exception
#Returns: True if status code is 200
#         False otherwise
def requestOK(r):
	if r.status_code != 200:	
		#r.raise_for_status() #Throw Exception

		#print "Status code: ", str(r.status_code())
		print r.content
		return False
	else:
		print ("Status code: 200 OK")
		return True

def clearDir(dir_clear):
	for del_block in os.listdir(dir_clear):
		del_path=os.path.join(dir_clear, del_block)
		os.unlink(del_path)

#Send metadata server a block_list request
#Returns: JSON from response containing block_list and version or None
def getBlockList(fi, user):
	params={'file_name': fi, 'user_id': user}

	print ("***Sending HTTP get to metadata server block_list for file %s***" %fi)
	r=requests.get('http://metaserver.raycoll.me/block_list', params=params)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return None
	return r.json()

class BlockQueryValues:
	def __init__(self, file_name, block_list, user_id):
		self.user_id=user_id
		self.file_name=file_name
		self.block_list=block_list

#Send metadata server block_query request
#Returns: JSON from response containing nb (True or False) and needed_blocks or None
def getBlockQuery(fi, block_list, user_id):
	values=BlockQueryValues(fi, block_list, user_id)
	payload=json.dumps(vars(values))
	headers = {'content-type': 'application/json'}
	

	print ("***Sending HTTP request to metadata server block_query for file %s***" %fi)
	print ("Payload: %s" % payload)
	r=requests.get('http://metaserver.raycoll.me/block_query', data=payload, headers=headers)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return None
	return r.json()

#Send cache server block_store request
#Returns: True or False if get good response back
def postBlockStore(fi,block_hash, user):
	print ("***Sending block %s to block_store for hash %s***" % (fi, block_hash))
	#params={'hash': block_hash, 'user': user}
	#files = {'file': open(fi, 'rb')}
	#headers={'Content-Disposition': 'form-data', 'name':'file', 'filename':fi}

	#r = requests.post('http://'+cache_host+'/cache_block_store', files=files, params=params, headers=headers)
	#print ("URL: %s" % r.url)

	#if( requestOK(r) == False):
	#	return False
	#print r.content, "\n"
	process = subprocess.Popen("curl -s -w %{http_code} --data-binary @"+fi+" 'http://"+cache_host+"/cache_block_store?hash="+block_hash+"&user="+user+"'", shell=True)
	output = process.communicate()[0]
	return True

class FileCommitValues:
	def __init__(self, file_name, block_list, user_id, version):
		self.user_id=user_id
		self.file_name=file_name
		self.block_list=block_list
		self.version=str(version) 

#Send metaserver file_commit request
#Returns: If file_commit is True or False 
def postFileCommit(fi, block_list, user_id, version):
	values=FileCommitValues(fi, block_list, user_id, version)
	payload=json.dumps(vars(values))
	print payload
	headers = {'content-type': 'application/json'}

	print ("***Committing file %s to metadata server file_commit***" % fi)
	r = requests.post('http://metaserver.raycoll.me/file_commit', data=payload, headers=headers)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return False
	print r.content, "\n"
	file_commit_json= r.json()
	if(file_commit_json["metadata_updated"]==True):
		print("md updated")
		return True
	print("md not updated")
	return False


#Upload file
def upFile(fi, version):
	
	blockDir="./blocks/"+fi+"/"

	#Create block directory for this file if it does not exist
	if not os.path.exists(blockDir):
		os.makedirs(blockDir)

	clearDir(blockDir)
	#Split into 4MB blocks using bash command

	try:
		process = subprocess.Popen("split -b 4m "+fi+" "+blockDir+fi+"_", shell=True)
		output = process.communicate()[0]
	except Exception, e:
		print e.message
		print "Error splitting file. Check permissions"
		return False

	print ("***Split file %s into 4MB files in %s***\n" % (fi, blockDir) )


	#For every block create a hash
	for (dir, subdirs, files) in os.walk(blockDir):

		file_hashes=[] #Create a list of all the hashes
		hash_to_file=dict() #Dictionary maps hash to path to correspoding block

		print ("***Get SHA256 hashes for file %s blocks***" % fi )
		for f in files:

			path= os.path.join(dir, f)
			sha= hashlib.sha256()

			#Read block by a buffer size and use SHA256 to hash
			with open(path, 'rb') as fbuf:
				buf=fbuf.read(BUF_SIZE)
				while len(buf) > 0:
					sha.update(buf)
					buf= fbuf.read(BUF_SIZE)

			block_hash=format(sha.hexdigest())
			print("sha256 for %s: %s" % (path, block_hash))
			file_hashes.append(block_hash)
			hash_to_file[block_hash]=path
		#print file_hashes, "\n"
	
	print "dict", hash_to_file

	#Call block_query
	block_query_json=getBlockQuery(fi, file_hashes, user)
	if(block_query_json==None):
		print ("Block query failed for file %s" % fi)
		clearDir(blockDir)
		return False
	print block_query_json, "\n"

	#Check if blocks need to be sent
	if( block_query_json["nb"] == False):
		print("No blocks need to be sent")

		file_commit=postFileCommit(fi, file_hashes, user, version)
		if(file_commit == False):
			print("File commit for file %s failed" %fi)
			clearDir(blockDir)
			return False


	elif (block_query_json["nb"] == True):
		print("Need to send blocks")
		needed_blocks=block_query_json["needed_blocks"]
		print needed_blocks, "\n"

		for blck_hash in needed_blocks:
			file_store=postBlockStore(hash_to_file[blck_hash], blck_hash, user)
			if(file_store == False):
				print("File store for block %s failed. File %s not uploaded" %(hash_to_file[blck_hash], fi))
				clearDir(blockDir)
				return False

		file_commit=postFileCommit(fi, file_hashes, user, version)
		if(file_commit == False):
			print("File commit for file %s failed" %fi)
			clearDir(blockDir)
			return False

	clearDir(blockDir)
	print("UPLOADED %s \n" % fi)
	return True


#Send cache server block_fetch request
#Returns: HTML confirming hash and data
def getBlockFetch(fi,block_hash, user, save_here):
	print ("***Getting block from cache block_fetch for %s***" % block_hash)
	params={'hash': block_hash, 'user': user}
	r = requests.post('http://'+cache_host+'/cache_block_fetch', params=params, stream=True)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return False

	with open(save_here, 'wb') as blockwrite:
		for chunk in r.iter_content(1024):
			blockwrite.write(chunk)

	return True


def downFile(fi, block_list):

	blockDir="./blocks/"+fi+"/"

	if not os.path.exists(blockDir):
		os.makedirs(blockDir)
	clearDir(blockDir)

	new_filename = (fi+"_%05i" % i for i in count(1))
	save_block=str(next(new_filename))
	for block in block_list:
		if(getBlockFetch(fi, block, user, blockDir+save_block) == False):
			print("Failed to fetch block %s. Stopping download of file %s" % (block, fi) )
			return False
		save_block=str(next(new_filename))

	process = subprocess.Popen("cat "+blockDir+fi+"_* > "+fi, shell=True)
	output = process.communicate()[0]

	#clearDir(blockDir)

	print("DOWNLOADED %s \n" % fi)
	return True


def addToVersionFile(fi, version):
	sync_time=datetime.datetime.now().strftime("%a %b %d %H:%M:%S %Y")

	jsonFile = open('versions.json', "r")
 	data = json.load(jsonFile)
 	jsonFile.close()

 	data[fi]={'version': version, 'mod_time': sync_time}

	with open('versions.json', 'w+') as outfile:
		json.dump(data, outfile, indent=4)
		outfile.close()

#If versions file does not exist. Create one with empty JSON object
if not os.path.isfile('versions.json'):
	with open('versions.json', 'w') as outfile:
		json.dump({}, outfile)
		outfile.close()


#For every file in current directory (just this level)
#Does not sync files in subdirectories
files = [f for f in os.listdir('.') if os.path.isfile(f)]
for fi in files:
	if not (fi =="sync.py" or fi=="versions.json"):
		print("*************************CHECKING FILE %s" % fi)
		#path= os.path.join(dire, fi)

		block_list_json= getBlockList(fi, user)

		#HTTP Error continue to next file
		if (block_list_json== None): 
			print("Block list failed for file %s" % fi)
			continue

		block_list=block_list_json["block_list"]
		block_list_version=block_list_json["version"]

		ver_file= open('versions.json', 'r+')
		version_json=json.load(ver_file)
		ver_file.close()

		try:
			version_saved=version_json[fi]["version"]
			mod_time_saved=version_json[fi]["mod_time"]
		except KeyError:
			if( block_list == None):
				#No version and no blocks exist

				if( upFile(fi, "0")== True):

					addToVersionFile(fi, 0)

					continue
				else:
					failed_files.append(fi)
			else:
				if(downFile(fi, block_list)==True):

					addToVersionFile(fi, block_list_version)

					continue
				else:
					failed_files.append(fi)

		if( block_list_json["block_list"] != None):
			meta_version=block_list_json["version"]
			client_version=version_saved


			(mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime) = os.stat(fi)
			mod_time_actual=time.ctime(mtime)
			saved_mod_time = datetime.datetime.strptime(mod_time_saved, "%a %b %d %H:%M:%S %Y")
			actual_mod_time = datetime.datetime.strptime(mod_time_actual, "%a %b %d %H:%M:%S %Y") 
				
			if(saved_mod_time >= actual_mod_time):
				if (meta_version > client_version):	
					#Download file
					#Upate version file
					if(downFile(fi, block_list)==True):
						addToVersionFile(fi, meta_version)
					else:
						failed_files.append(fi)
				else:
					print("File %s already in sync\n" % fi)
					continue
			else:
				if (meta_version > client_version):
					if( upFile(fi, int(meta_version)+1)== True):
						addToVersionFile(fi, meta_version+1)
					else:
						failed_files.append(fi)
				else:
					if( upFile(fi, int(client_version)+1)== True):
							addToVersionFile(fi, client_version+1)
					else:
						failed_files.append(fi)
		else:
			#Delete the file (it is in the version file, we caught keyerror if it was not in it)
			os.remove(fi)

			jsonFile = open('versions.json', "r")
 			json_data = json.load(jsonFile)
 			jsonFile.close()

 			del json_data[fi]

			with open('versions.json', 'w+') as outfile:
				json.dump(json_data, outfile, indent=4)
				outfile.close()


#Send metadata server a block_list request
#Returns: JSON from response containing block_list and version
def getFileList():
	params={'user_id': user}

	print ("***Sending HTTP get to metadata server file_list for user %s***" % user)
	r=requests.get('http://metaserver.raycoll.me/file_list', params=params)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return None
	return r.json()

def postFileDelete(fi):
	params={'file_name': fi, 'user_id': user}

	print ("***Sending HTTP get to metadata server file_delete for file %s***" % user)
	r=requests.post('http://metaserver.raycoll.me/file_delete', params=params)
	print ("URL: %s" % r.url)

	if( requestOK(r) == False):
		return None
	return r.json()

file_list_json = getFileList()
if (file_list_json == None):
	print("Error with file_list request")
elif (file_list_json["files"] == None):
	print("No files listed on metadata to sync")
else:
	file_list = tuple(x[0] for x in file_list_json["files"])

	for f in file_list:
		if not os.path.isfile(f):

			ver_file= open('versions.json', 'r+')
			version_json=json.load(ver_file)
			ver_file.close()

			try:
				version_saved=version_json[f]["version"]
			except KeyError:
				#Not in version json
				block_list_json= getBlockList(f, user)

				#HTTP Error continue to next file
				if (block_list_json== None): 
					print("Block list failed for file %s" % f)
					continue

				if(downFile(f, block_list_json["block_list"])==True):
					addToVersionFile(f, block_list_json["version"])
				
				continue

			#is in version json
			file_delete_json=postFileDelete(f)
			if(file_delete_json["is_delete"]!=True):
				print("Failed to delete file %s from metadata server" % f)

			jsonFile = open('versions.json', "r")
 			json_data = json.load(jsonFile)
 			jsonFile.close()

 			del json_data[f]

			with open('versions.json', 'w+') as outfile:
				json.dump(json_data, outfile, indent=4)
				outfile.close()

if(len(failed_files) > 0):
	print("These files failed to sync:")
	for fail in failed_files:
		print(fail)






			



