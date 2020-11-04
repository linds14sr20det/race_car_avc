#!/usr/bin/env python3

from __future__ import print_function

import os
import sys
import time
import numpy as np
import sounddevice as sd

try:
    data = np.loadtxt(fname="raw_vibdata.csv", dtype='u1', delimiter="\n")
    sd.play(data, 250, device=0)
    status = sd.wait()
except (KeyboardInterrupt, SystemExit):
    ADC0832.destroy()
    print(f"{time.time()}: Exiting")

