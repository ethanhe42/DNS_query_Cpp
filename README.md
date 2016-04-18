# DNS query
This project is about network measurement,  
initally intended to detect DNS caches and TTL of malicious URLs on all DNS server on Internet,  
then analysis their behavior.
However, I didn't get some insights into these data.  
This project is open for use now, so you can detect any URLs caches on any DNS server, and predict their TTL.  
Work is done by sending DNS **non-recursive** query through multi threads(pretty fast, and CPU consuming).  
By non-resursive, DNS server won't query its parent server. So I take advantage of this to check URL cache on any DNS server.  
There are both Python and Cpp version, Python version is easier to read, and Cpp version is written using winsock.  

Note that dnspython should be preinstalled.  

### first
u should input a list of domains,each domain per line in malurl.txt  

### second
run findMaxTTL.py to get maxTTL text file which record all registered domains

### third
run non_recursiveDetect.py with cellular network shared via USB or WiFi  
add DNS to name_server_list  
u can set minTTL using variable minTTL  
remainTTL.csv is desired result  

### attention
findMaxTTL.py will clear maxTTL.txt when it starts  
