#!/usr/bin/env python3

from __future__ import print_function

import sys
import time
import struct
import alsaaudio
import smbus
import numpy as np

def getAxes():
    b = bus.read_i2c_block_data(0x53, 0x34, 2)
    y = int.from_bytes([b[0], b[1]], byteorder='little', signed=True)
    return y


if __name__ == '__main__':
    bus = smbus.SMBus(1)
    #Turn on i2c interface at 200bps
    bus.write_byte_data(0x53, 0x2C, 0x0E)
    #Configure the sensor to use 10bit 16g resolution 
    bus.write_byte_data(0x53, 0x31, 0x0B)
    #Configure power control (measure and don't sleep)
    bus.write_byte_data(0x53, 0x2D, 0x08)
    #x-axis should remove gravity
    bus.write_byte_data(0x53, 0x1F, 0x41)

    out = alsaaudio.PCM(alsaaudio.PCM_PLAYBACK, channels=1, 
        rate=1000, 
        periodsize=1, device='sysdefault:CARD=Headphones')

    mixer = alsaaudio.Mixer('Headphone')
    mixer.setvolume(100)
    
    dt = np.dtype('<i2')
    data = np.zeros(10, dt)
    try:
        while True:
            acceleration = getAxes()
            data.fill(acceleration)
            out.write(bytes(data))
    except (KeyboardInterrupt, SystemExit):
        # Outputting feedback regarding the end of the file
        print('Exiting')

