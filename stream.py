#!/usr/bin/env python3

from __future__ import print_function

import sys
import time
import struct
import alsaaudio
import smbus
import numpy as np

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

    #x = x * 0.004 
    #y = y * 0.004
    #z = z * 0.004

    #x = x * 9.80665
    #y = y * 9.80665
    #z = z * 9.80665

    #x = round(x, 4)
    #y = round(y, 4)
    #z = round(z, 4)

    return y


if __name__ == '__main__':

    accelerometer_prompt = input("Use accelerometer? [y/N]")
    
    if accelerometer_prompt == 'y':
        bus = smbus.SMBus(1)
        #Turn on i2c interface at 200bps
        bus.write_byte_data(0x53, 0x2C, 0x0E)
        #Configure the sensor to use 10bit 16g resolution 
        bus.write_byte_data(0x53, 0x31, 0x0B)
        #Configure power control (measure and don't sleep)
        bus.write_byte_data(0x53, 0x2D, 0x08)
        #x-axis should remove gravity
        bus.write_byte_data(0x53, 0x1E, 0x41)
    else:    
        # Open the device in nonblocking capture mode in mono, with a sampling rate of 44100 Hz 
        # and 16 bit little endian samples
        # The period size controls the internal number of frames per period.
        # The significance of this parameter is documented in the ALSA api.
        # For our purposes, it is suficcient to know that reads from the device
        # will return this many frames. Each frame being 2 bytes long.
        # This means that the reads below will return either 320 bytes of data
        # or 0 bytes of data. The latter is possible because we are in nonblocking
        # mode.
        inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, channels=1, 
	    rate=1000, format=alsaaudio.PCM_FORMAT_S16_BE, 
            periodsize=15, device='sysdefault:CARD=Microphone')

    out = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK, channels=1, 
        rate=1000, format=alsaaudio.PCM_FORMAT_S16_BE, 
        periodsize=15, device='sysdefault:CARD=Headphones')

    mixer = alsaaudio.Mixer('Headphone')
    mixer.setvolume(100)
   
    try:
        while True:
            # Read data from device
            if accelerometer_prompt == 'y':
                acceleration = getAxes()
                data = bytes(np.full((1,30), acceleration)[0])
                print(data)
                print(sys.getsizeof(data))
            else:
                l, data = inp.read()
                # Convert data to integers
                #data_int = np.frombuffer(data, dtype=np.int32)
                print(data)
                print(sys.getsizeof(data))
                print(l)
            
            #TODO: Do the filtering here

            # Convert antisound to binary
            data_mirror = memoryview(data)

            out.write(data_mirror)
    except (KeyboardInterrupt, SystemExit):
        # Outputting feedback regarding the end of the file
        print('Exiting')

