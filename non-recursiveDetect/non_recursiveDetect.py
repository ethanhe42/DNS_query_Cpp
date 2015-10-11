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


def query(domain, name_server):
    domain = dns.name.from_text(domain)
    if not domain.is_absolute():
        domain = domain.concatenate(dns.name.root)

    
    request = dns.message.make_query(qname = domain,
                                     rdtype = dns.rdatatype.A)
    request.flags &= ~dns.flags.RD

    response = dns.query.udp(q=request,
                             where=name_server,
                             timeout=2)
    return response

f = open("maxTTL.txt")
domains = []
currenttimeinseconds=time.time()

for line in f.readlines():
    urlandmaxTTL=str(line).strip() .split()
    url=urlandmaxTTL[0]
    maxTTL=urlandmaxTTL[1]
    domains.append([url,
                    maxTTL+currenttimeinseconds])

f = open("remainTTL.txt", mode='w')

q = Queue.Queue()


def getAnswer(domain, q):
    global wrongCnt
    
    try:
        response = query(domain, name_server)
        if len(response.answer) != 0:
            
            q.put(domain + ' ' + response.answer[0].ttl)

    except dns.exception.Timeout:
        wrongCnt += 1



numofthreads = 0
threadspool = []

    




while True:
    stuff=domains.pop()
    currenttimeinseconds=time.time()    
    if stuff[1] <currenttimeinseconds:
        pass
    t = threading.Thread(target=getAnswer,
                         args=(domains.pop,
                               q))
    t.daemon = True
    threadspool.append(t)
    t.start()

    numofthreads += 1
    if numofthreads > maxthreads:
        numofthreads = 0

        while len(threadspool) != 0:
            threadspool.pop().join()

        while not q.empty():
            localT = time.localtime()
            
            line = q.get() 
            +' ' + str(localT.tm_yday)
            +' ' + str(localT.tm_hour)
            +':' + str(localT.tm_min)
            

            print(line)

            f.write(line + '\n')

            # f.write(domain + ' ' + str(TTL) + '\n')
time.sleep(5)
f.close()
