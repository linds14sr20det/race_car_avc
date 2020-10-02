#!/usr/bin/env python3

from __future__ import print_function

import sys
import time
import sounddevice as sd
import struct
import smbus
import numpy as np

def getAcceleration():
    b = bus.read_i2c_block_data(0x53, 0x34, 2)
    y = int.from_bytes([b[0], b[1]], byteorder='little', signed=True)
    return y

bus = smbus.SMBus(1)
#Turn on i2c interface at 200Hz
bus.write_byte_data(0x53, 0x2C, 0x0D)
#Configure the sensor to use 10bit 16g resolution 
bus.write_byte_data(0x53, 0x31, 0x0B)
#Configure power control (measure and don't sleep)
bus.write_byte_data(0x53, 0x2D, 0x08)
#y-axis should remove gravity
bus.write_byte_data(0x53, 0x1F, 0x41)
#Set FIFO mode
bus.write_byte_data(0x53, 0x38, 0x00)
start_idx = 0
sd.default.dtype='<i2'
#sd.default.latency='low'
sd.default.blocksize=111
sd.default.prime_output_buffers_using_stream_callback=True

try:
    def callback(outdata, frames, time, status):
        global start_idx
        if status:
            print(status, file=sys.stderr)
            print(start_idx/frames)
        t = (start_idx + np.arange(frames)) / 44100
        t = t.reshape(-1, 1)
        acceleration = getAcceleration()/4
        print(acceleration)
        outdata[:] = acceleration * t
        start_idx += frames

    with sd.OutputStream(channels=1, callback=callback, samplerate=44100):
        input()

except (KeyboardInterrupt, SystemExit):
    # Outputting feedback regarding the end of the file
    print('Exiting')

