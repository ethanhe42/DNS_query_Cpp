import Queue
pq=Queue.PriorityQueue()
pq.put((2,3))
pq.put((1,5))
pq.put((3,4))
while not pq.empty():
    print pq.get()[1]