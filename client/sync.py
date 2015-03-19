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
import shutil

BUF_SIZE = 65536
local_failed_files=[]
meta_failed_files=[]

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
		print "HTTP error", r.content
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
	process = subprocess.Popen("curl -s 'http://"+cache_host+"/cache_block_store?user="+user+"'", stdout=subprocess.PIPE, stderr=None, shell=True)
	response = process.communicate()[0]
	print (response)
	if not "200 OK" in response:
		return False
	return True

class FileCommitValues:
	def __init__(self, file_name, block_list, user_id, version):
		self.user_id=user_id
		self.file_name=file_name
		self.block_list=block_list
		self.version=str(version) 

#Send metaserver file_commit request
#Returns: If file_commit is successful (True or False)
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
	process = subprocess.Popen("split -b 4m "+fi+" "+blockDir+fi+"_", shell=True)
	output = process.communicate()[0]
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
			print("sha256 for %s: %s \n" % (path, block_hash))
			file_hashes.append(block_hash)
			hash_to_file[block_hash]=path
		
	#Call block_query
	block_query_json=getBlockQuery(fi, file_hashes, user)
	if(block_query_json==None):
		print ("Block query failed for file %s" % fi)
		clearDir(blockDir)
		return False

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
	print("Block source: %s" % r.headers['origin'])

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
				#No version exists for this file in the version file and metadata server has no blocks for this file
				#Upload
				if( upFile(fi, "0")== True):
					#Update version file
					addToVersionFile(fi, 0)
				else:
					#Upload failed
					print("File %s failed to upload\n" % fi)
					local_failed_files.append(fi)
				continue
			else:
				#No version exists for this file in the version file BUT metadata server has blocks for this file
				#Download
				if(downFile(fi, block_list)==True):
					#Update version file
					addToVersionFile(fi, block_list_version)
				else:
					#Download failed
					print("File %s failed to download\n" % fi)
					local_failed_files.append(fi)
				continue

		#There is a version for this file and metadata has blocks for this file
		if( block_list_json["block_list"] != None):

			meta_version=block_list_json["version"]
			client_version=version_saved

			(mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime) = os.stat(fi)
			mod_time_actual=time.ctime(mtime)
			saved_mod_time = datetime.datetime.strptime(mod_time_saved, "%a %b %d %H:%M:%S %Y")
			actual_mod_time = datetime.datetime.strptime(mod_time_actual, "%a %b %d %H:%M:%S %Y") 
				
			#If time last synced is greater than or equal to time of last file modification
			if(saved_mod_time >= actual_mod_time):
				#If metadata version is greater than the version saved in client's version file
				if (meta_version > client_version):	
					#Download file	
					if(downFile(fi, block_list)==True):
						#Upate version file
						addToVersionFile(fi, meta_version)
					else:
						#Download failed
						print("File %s failed to download\n" % fi)
						local_failed_files.append(fi)
				else:
					#Client version is less than or equal to metadata version
					print("File %s already in sync\n" % fi)
					continue
			#File has been modified since last sync
			else:
				#If metadata version is greater than the version saved in client's version file
				if (meta_version > client_version):
					#Overwrite metadata version with client version
					if( upFile(fi, int(meta_version)+1)== True):
						addToVersionFile(fi, meta_version+1)
					else:
						print("File %s failed to upload\n" % fi)
						local_failed_files.append(fi)
				else:
					#Upload client version
					if( upFile(fi, int(client_version)+1)== True):
							addToVersionFile(fi, client_version+1)
					else:
						print("File %s failed to download\n" % fi)
						local_failed_files.append(fi)
		else:
			#Delete the file (it is in the version file, we caught keyerror earlier if it was not in it)
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
	print("No files listed on metadata that need to be synced")
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
					print("Downloaded new file %s from meta data server\n" % f)
				else:
					meta_failed_files.append(f)
					print("Failed to delete file %s from metadata server\n" % f)
				
				continue

			#File listed by metadata server in version file but does not exist in file system
			#Call file_delete to metadata server
			file_delete_json=postFileDelete(f)
			if(file_delete_json["is_delete"]==False):
				meta_failed_files.append(f)
				print("Failed to delete file %s from metadata server\n" % f)
			else:
				print("Successfully deleted file %s from metadata server\n" % f)
				jsonFile = open('versions.json', "r")
 				json_data = json.load(jsonFile)
 				jsonFile.close()

 				del json_data[f]

				with open('versions.json', 'w+') as outfile:
					json.dump(json_data, outfile, indent=4)
					outfile.close()

if(len(local_failed_files) > 0):
	print("These files on client file system failed to sync:")
	for fail in local_failed_files:
		print(fail)
if(len(meta_failed_files)>0):
	print("These files listed by metadata server failed to download")

shutil.rmtree("blocks", ignore_errors=True)






			



