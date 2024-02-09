import os

f = open('./log.txt', "a+")
for i in range(500000):
    data = '2024-01-16 14:23:57  - INFO -Model-detect() - detect 5 persons\n'
    f.write(data)
