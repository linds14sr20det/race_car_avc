#!/usr/bin/env python3

from __future__ import print_function

import os
import sys
import time
import sounddevice as sd
import numpy as np
import ADC0832

ADC0832.setup()
start_idx = 0
sd.default.dtype='<i2'
#sd.default.latency='low'
sd.default.blocksize=222
sd.default.prime_output_buffers_using_stream_callback=True

print(f"{time.time()}: Starting avc")

def getAcceleration():
    accel = (ADC0832.getResult()-122)
    if -10 < accel < 10:
        accel = 0
    return accel

try:
    def callback(outdata, frames, time, status):
        global start_idx
        if status:
            print(status, file=sys.stderr)
            print(start_idx/frames)
        t = (start_idx + np.arange(frames)) / 44100
        t = t.reshape(-1, 1)
        acceleration = getAcceleration()
        outdata[:] = acceleration * t
        start_idx += frames

    with sd.OutputStream(channels=1, callback=callback, samplerate=44100):
        os.system('date > /tmp/avcswitch')
        os.system('tail -f /tmp/avcswitch')

except (KeyboardInterrupt, SystemExit):
    ADC0832.destroy()
    print(f"{time.time()}: Exiting")


