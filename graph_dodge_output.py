import ADC0832
import time
import math
import numpy as np
import matplotlib.pyplot as plt

def init():
    ADC0832.setup()

def loop():
    blerp=[]
    time_start = time.time()
    while time.time() < time_start+2:
        analogVal = ADC0832.getResult()
        blerp.append(analogVal)
    print(blerp)
    plt.plot(blerp)
    plt.show()

if __name__ == '__main__':
    init()
    try:
        loop()
    except KeyboardInterrupt: 
        ADC0832.destroy()
        print('The end!')
