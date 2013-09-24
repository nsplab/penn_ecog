#! /usr/bin/env python

from sklearn.cross_decomposition import PLSCanonical, PLSRegression
from numpy import array

filename = "data"
sampling_rate = 7000
fft_samples = 3500 # ???
fft_offset = 1 # ???
samples = 5 # ???
channels = 16
n_components = 2 # ???
scale = True # ???

pls = PLSRegression(n_components, scale)
signal = [0 for i in xrange(samples)]
ecog = [[0 for i in xrange(samples)] for j in xrange(channels)]

f = open(filename, 'r')

for (i, line) in enumerate(f):
  value = str.split(line)
  signal[i] = float(value[0])
  for j in xrange(channels):
    ecog[j][i] = float(value[j + 1])





# X = [[0., 0., 1.], [1.,0.,0.], [2.,2.,2.], [2.,5.,4.]]
# Y = [[0.1, -0.2], [0.9, 1.1], [6.2, 5.9], [11.9, 12.3]]
# X = array(X)
# Y = array(Y)
# pls.fit(X, Y)
# Y_pred = pls.predict(X)
# print Y_pred
