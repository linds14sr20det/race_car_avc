#!/usr/bin/env python3

from __future__ import print_function

import sys
import time
import struct
import alsaaudio
import numpy as np

if __name__ == '__main__':

    # Open the device in nonblocking capture mode in mono, with a sampling rate of 44100 Hz 
    # and 16 bit little endian samples
    # The period size controls the internal number of frames per period.
    # The significance of this parameter is documented in the ALSA api.
    # For our purposes, it is suficcient to know that reads from the device
    # will return this many frames. Each frame being 2 bytes long.
    # This means that the reads below will return either 320 bytes of data
    # or 0 bytes of data. The latter is possible because we are in nonblocking
    # mode.
    inp = alsaaudio.PCM(alsaaudio.PCM_CAPTURE, channels=2, 
	rate=44100, format=alsaaudio.PCM_FORMAT_S16_LE, 
        periodsize=160, device='sysdefault:CARD=Microphone')

    out = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK, channels=2, 
        rate=44100, format=alsaaudio.PCM_FORMAT_S16_LE, 
        periodsize=160, device='sysdefault:CARD=Headphones')

    mixer = alsaaudio.Mixer('Headphone')
    mixer.setvolume(100)
    
    try:
        while True:
	    # Read data from device
            l, data = inp.read()
        
            # Convert data to integers
            data_int = np.frombuffer(data, dtype=np.int16)
            #data_np = data_int[::2] + 128
            
            # Convert data to np array and offset by 128
            #data_np = np.array(data_int, dtype='b')[::2] + 128

            # Create array of anti sound
            #data_np_mirror = 128 - (data_np - 128)

            # Convert antisound to integer list to output
            #data_int_mirror = (data_np_mirror - 128)

            # Convert antisound to binary
            data_mirror = memoryview(data_int)

            if l:
                out.write(data_mirror)
                #time.sleep(.001)
    except (KeyboardInterrupt, SystemExit):
        # Outputting feedback regarding the end of the file
        print('Exiting')
