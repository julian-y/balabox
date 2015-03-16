# Balabox Installation Instructions
* NOTE: Installation has only been tested on Ubuntu 14.04 x64. 

## Recommended Installation Method: Use the servers we're maintaining  
* blockserver.raycoll.me
* metaserver.raycoll.me
* cache-sfo.raycoll.me
* cache-lndn.raycoll.me

To use these servers, configure your client code to interact with metaserver.raycoll.me and cache-sfo/lndn.raycoll.me.

## Rolling out your own servers(Requires some manual configuration, not recommended) 
### Meta Server
To create your own metadata server, we've provided a handy dependency install script. Simply ssh into your desired host and run it and then run make install to deploy your server. No other configuration is necessary since the metaserver doesn't make requests to other servers.  

  <pre><code> 
  cd metadata
  sudo ./install_deps.sh
  make
  sudo make install</code></pre>

### Block Server
To create your own block server, the process is very similar to deploying the metadata server code.

  <pre><code> 
  cd blockcache
  sudo ./install_deps.sh
  make server
  sudo make install</code></pre>


### Cache Server(s)
Installing a cache server is trickier since it needs to know the IP addresses or hostnames of both your previously deployed block server and metadata server. These values must be edited in the source code for the cache. 

  <pre><code> 
  cd blockcache
  sudo ./install_deps.sh
  make cache
  sudo make install</code></pre>

## Using the Balabox Client
To use the client, simply run the sync program. The sync program requires you to provide it the url of the cache server and metadata server, along with the user id you want to use. 
