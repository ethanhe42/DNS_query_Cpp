# Web

the python project is in non-recursiveDetect  
dnspython should be preinstalled  

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
