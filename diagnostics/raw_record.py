import ADC0832
import time
import math

def init():
    print("Recording sensor data:")
    ADC0832.setup()

if __name__ == '__main__':
    init()
    try:
        f = open("raw_vibdata.csv", "a+")
        f.truncate(0)
        data = []
        while True:
            analogVal = ADC0832.getResult()
            f.write(str(analogVal))
            f.write("\n")
            time.sleep(0.0005)
    except KeyboardInterrupt:
        f.close()
        ADC0832.destroy()
    print('The end!')
