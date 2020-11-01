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
data_format = 'u1'

print(f"{time.time()}: Starting avc")

def getAcceleration():
    return ADC0832.getResult()

try:
    print(sd.query_devices())
    print(sd.query_devices(0))

    def callback(outdata, frames, time, status):
        if status:
            print(status, file=sys.stderr)
        value = np.full((888,1), getAcceleration(), data_format)
        outdata[:] = value

    with sd.OutputStream(samplerate=default_samplerate, device=0, blocksize = 888, channels=1, dtype=data_format, latency='low', callback=callback):
        os.system('date > /tmp/avcswitch')
        os.system('tail -f /tmp/avcswitch')

except (KeyboardInterrupt, SystemExit):
    ADC0832.destroy()
    print(f"{time.time()}: Exiting")


