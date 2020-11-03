#!/usr/bin/env python3

from __future__ import print_function

import os
import sys
import time
import sounddevice as sd
import soundfile as sf

try:
    data, fs = sf.read("raw_vibdata.csv", dtype='u1')
    sd.play(data, fs, device=0)
    status = sd.wait()
except (KeyboardInterrupt, SystemExit):
    ADC0832.destroy()
    print(f"{time.time()}: Exiting")

