import ADC0832
import time
import math

def init():
    print("Recording 5 seconds of sensor data:")
    ADC0832.setup()

def loop():
    data=[]
    time_start = time.time()
    while time.time() < time_start+5:
        analogVal = ADC0832.getResult()
        Vr = 5 * float(analogVal) / 255
        data.append(Vr)
    f = open("vibdata.csv", "w")
    converted_list = [str(element) for element in data]
    joined_string = ",".join(converted_list)
    f.write(joined_string)
    f.close()


if __name__ == '__main__':
    init()
    try:
        loop()
    except KeyboardInterrupt: 
        ADC0832.destroy()
    print('The end!')
