#!/usr/bin/env python3

from __future__ import print_function

import os
import sys
import time
import sounddevice as sd
import numpy as np
import ADC0832

ADC0832.setup()
default_samplerate = 44100

print(f"{time.time()}: Starting avc")

def getAcceleration():
    accel = (ADC0832.getResult()-122)
    #if -10 < accel < 10:
    #    accel = 0
    return accel

try:
    print(sd.query_devices())
    print(sd.query_devices(0))

    def callback(outdata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        value = np.full((888,1), getAcceleration(), '<i1')
        outdata[:] = value

    with sd.OutputStream(samplerate=default_samplerate, device=0, blocksize = 888, channels=1, dtype='<i1', latency='low', callback=callback):
        os.system('date > /tmp/avcswitch')
        os.system('tail -f /tmp/avcswitch')

except (KeyboardInterrupt, SystemExit):
    ADC0832.destroy()
    print(f"{time.time()}: Exiting")


