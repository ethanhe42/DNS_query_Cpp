name_server_list = [
    '114.114.114.114',
    '202.117.0.20'
]

name_server = name_server_list[1]
f = open("malurl.txt")
maxthreads = 300


import dns.resolver
import dns.name
import dns.message
import dns.query
import dns.rdatatype
from dns import *
import Queue
import threading
import time

additZero=0
wrongCnt = 0

def query(domain, name_server):
    domain = dns.name.from_text(domain)
    if not domain.is_absolute():
        domain = domain.concatenate(dns.name.root)

    request = dns.message.make_query(domain, dns.rdatatype.A)
    response = dns.query.udp(request, name_server, timeout=2)

    return response


domains = Queue.Queue()
for line in f.readlines():
    
    domains.append(str(line).split()[0])

f = open("maxTTL.txt", mode='w')

q = Queue.Queue()


def getTTL(domain, q):
    global wrongCnt,additZero
    TTL = 0
    try:
        response = query(domain, name_server)
        additCnt = len(response.additional)
    except dns.exception.Timeout:
        additCnt = 0
        wrongCnt += 1

    if additCnt == 0:
        additZero+=1

    else:
        addit = response.additional[0]
        Serveraddr = addit.items[0].address

        try:
            response = query(domain, Serveraddr)
            ansCnt = len(response.additional)
        except dns.exception.Timeout:
            ansCnt = 0
            wrongCnt += 1

        ansCnt = len(response.answer)
        if ansCnt != 0:
            answer = response.answer[0]
            TTL = answer.ttl

    # print domain, TTL
    # f.write(domain + ' ' + str(TTL) + '\n')
    q.put(domain + ' ' + str(TTL))


numofthreads = 0
threadspool = []

for domain in domains:
    t = threading.Thread(target=getTTL, args=(domain, q))
    t.daemon = True
    threadspool.append(t)
    t.start()

    numofthreads += 1
    if numofthreads > maxthreads:
        numofthreads = 0

        while len(threadspool) != 0:
            threadspool.pop().join()

        while not q.empty():
            line = q.get()
            print(line)

            f.write(line + '\n')

            # f.write(domain + ' ' + str(TTL) + '\n')
        print additZero
time.sleep(5)
f.close()
