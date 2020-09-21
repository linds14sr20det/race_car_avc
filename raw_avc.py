import smbus
import time
from time import sleep
import sys
import wave
import math
import struct
import random
import argparse
import pyaudio
import numpy as np
from itertools import *
import scipy as sy
import scipy.fftpack as syfp
import matplotlib.pyplot as pyl

bus = smbus.SMBus(1)

#Turn on i2c interface at 200bps
bus.write_byte_data(0x53, 0x2C, 0x0E)
#Configure the sensor to use 10bit 16g resolution 
bus.write_byte_data(0x53, 0x31, 0x0B)
#Configure power control (measure and don't sleep)
bus.write_byte_data(0x53, 0x2D, 0x08)
#x-axis should remove gravity
bus.write_byte_data(0x53, 0x1E, 0x41)
def getAxes():
    bytes = bus.read_i2c_block_data(0x53, 0x32, 6)
    x = bytes[0] | (bytes[1] << 8)    
    if(x & (1 << 16 - 1)):
        x = x - (1<<16)

    y = bytes[2] | (bytes[3] << 8)
    if(y & (1 << 16 - 1)):
        y = y - (1<<16)

    z = bytes[4] | (bytes[5] << 8)
    if(z & (1 << 16 - 1)):
        z = z - (1<<16)

    x = x * 0.004 
    y = y * 0.004
    z = z * 0.004

    x = x * 9.80665
    y = y * 9.80665
    z = z * 9.80665

    x = round(x, 4)
    y = round(y, 4)
    z = round(z, 4)

    return y



#def sine_wave(frequency=440.0, framerate=44100, amplitude=0.5):
#    period = int(framerate / frequency)
#    if amplitude > 1.0: amplitude = 1.0
#    if amplitude < 0.0: amplitude = 0.0
#    lookup_table = [float(amplitude) * math.sin(2.0*math.pi*float(frequency)*(float(i%period)/float(framerate))) for i in xrange(period)]
#    return (lookup_table[i%period] for i in count(0))

p = pyaudio.PyAudio()

volume = 1     # range [0.0, 1.0]
fs = 44100       # sampling rate, Hz, must be integer
duration = 1.0   # in seconds, may be float
f = 440.0        # sine frequency, Hz, may be float

# generate samples, note conversion to float32 array
samples = (np.sin(2*np.pi*np.arange(fs*duration)*f/fs)).astype(np.float32)
print(samples)
# for paFloat32 sample values must be in range [-1.0, 1.0]
stream = p.open(format=pyaudio.paFloat32,
                channels=1,
                rate=fs,
                output=True)

# play. May repeat with different volume values (if done interactively) 
stream.write(volume*samples)

stream.stop_stream()
stream.close()

p.terminate()


time_end = time.time() + 10

try:
    time_arr = []
    velocity = []
    while time.time() < time_end:
        time_local_end = time.time() + 5
        while time.time() < time_local_end: 
            velocity.append(getAxes())
            time_arr.append(time.time())

        #TODO Interpolate the time array
        #new_times = np.linspace(min(time_arr), max(time_arr), len(time_arr))
        #new_data = np.interp(new_times, time_arr, velocity)
        #print("new_times:")
        #print(new_times)
        #print("new data")
        #print(new_data)
        #t, u = new_times, new_data

        time_avg = (time_arr[len(time_arr)-1] - time_arr[0])/len(time_arr) 
        FFT = sy.fft(velocity)
        freqs = syfp.fftfreq(len(velocity), time_avg)
       
        #TODO Find dominant frequency from freqs
        #TODO shift by 180 deg
        # (1/hz)/2 = phase shift the x value (ie. time)
        
        pyl.subplot(211)
        pyl.plot(time_arr, velocity)
        pyl.xlabel('Time')
        pyl.ylabel('Amplitude')
        pyl.subplot(212)
        pyl.plot(freqs, sy.log10(abs(FFT)), '.')  ## it's important to have the abs here
        pyl.show()
except KeyboardInterrupt:
    sys.exit()
