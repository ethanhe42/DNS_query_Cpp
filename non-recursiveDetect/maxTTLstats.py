import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
f = open("maxTTL.txt")
domains = []

for line in f.readlines():
    urlandmaxTTL = str(line).strip().split()
    domains.append(urlandmaxTTL[1])

table=pd.Series(domains)
valueCnt=table.value_counts()
valueCnt= valueCnt.sort_index()
print valueCnt
plt.figure(1)
plt.plot(valueCnt.index,valueCnt)
plt.show()
