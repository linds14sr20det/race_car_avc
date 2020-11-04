# cython: language_level=3, boundscheck=False

import os
import sys
import time
import ADC0832
import math

def init():
    print("Recording sensor data:")
    ADC0832.setup()

init()
try:
    f = open("raw_vibdata.csv", "a+")
    f.truncate(0)
    data = []
    time_start = time.time()
    while True:
        analogVal = ADC0832.getResult()
        data.append(analogVal)
        #f.write(str(analogVal))
        #f.write("\n") 
except KeyboardInterrupt:
    f.close()
    print(time.time()-time_start)
    print(len(data))
    ADC0832.destroy()
print('The end!')
