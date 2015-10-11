__author__ = 'rex686568'

name_server_list = ['202.117.0.20']

name_server = name_server_list[0]

maxthreads = 600

wrongCnt = 0

import dns.resolver
import dns.name
import dns.message
import dns.query
import dns.rdatatype
from dns import *
import Queue
import threading
import time

Timeout=2

def query(domain, name_server):
    domain = dns.name.from_text(domain)
    if not domain.is_absolute():
        domain = domain.concatenate(dns.name.root)

    
    request = dns.message.make_query(qname = domain,
                                     rdtype = dns.rdatatype.A)
    request.flags &= ~dns.flags.RD

    response = dns.query.udp(q=request,
                             where=name_server,
                             timeout=Timeout)
    return response

numofthreads = 0

def getAnswer(domain, q):
    global wrongCnt,numofthreads
    
    try:
        response = query(domain, name_server)
        if len(response.answer) != 0:
            
            q.put(domain + ' ' + 
                  str(response.answer[0].ttl)+' '+
                  str(currenttimeinseconds))
        else:
            q.put(domain + ' ' + 
                  str(0)+' '+
                  str(currenttimeinseconds))
    except dns.exception.Timeout:
        wrongCnt += 1
    numofthreads -= 1

f = open("maxTTL.txt")
domains = Queue.PriorityQueue()
currenttimeinseconds = time.time()

for line in f.readlines():
    urlandmaxTTL = str(line).strip().split()
    url = urlandmaxTTL[0]
    maxTTL = int(urlandmaxTTL[1])
    if maxTTL != 0:
        domains.put((currenttimeinseconds,
                     [url,maxTTL]))
    

q = Queue.Queue()






while True:
    
    currenttimeinseconds = time.time()

    stuff = domains.get()
    deltaSeconds = stuff[0] - currenttimeinseconds
    if  deltaSeconds > 0:
        domains.put(stuff)
        time.sleep(deltaSeconds)
    else:
        domain=stuff[1][0]
        maxTTL=stuff[1][1]
        refreshstuff=(maxTTL+currenttimeinseconds,
                      stuff[1])
        t = threading.Thread(target=getAnswer,
                             args=(domain,
                                   q))
        t.daemon = True
        t.start()
        domains.put(refreshstuff)



    numofthreads += 1

    if numofthreads > maxthreads:
        numofthreads = 0
        time.sleep(Timeout+1)            

            

    
    f = open("remainTTL.txt", mode='a')
    while not q.empty():
        line = q.get()
        print(line)
        f.write(line + '\n')
    f.close()

            # f.write(domain + ' ' + str(TTL) + '\n')
time.sleep(Timeout+1)
