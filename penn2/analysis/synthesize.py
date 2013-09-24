#! /usr/bin/env python

from math import sin, cos, pi
from random import gauss

f1 = open('emg_synthesized', 'w');
f2 = open('eeg_synthesized', 'w');

for i in range(60 * 1024):
  squeeze = (i % 2048) < 1024
  f1.write(str(squeeze * cos(10 * 2 * pi * i / 1024)))
  f1.write('\n')

  f2.write(str(gauss(0, 0.01) + squeeze * cos(10 * pi * i / 1024)))
  f2.write('\n')

