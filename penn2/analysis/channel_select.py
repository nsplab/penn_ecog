#! /usr/bin/env python

from sklearn.cross_decomposition import PLSCanonical, PLSRegression
from numpy import array, fft, absolute, vstack, mean, floor
import struct
import os

filename = "../signal_acquisition/synth/data_Tue Sep 24 20:04:49 2013"
sampling_rate = 1024
fft_samples = 64 # ???
fft_offset = 10 # ???
channels = 16
n_components = 4 # ???
scale = True # ???
save_output = True


f = open(filename, 'r')

columns = 20
file_size = os.fstat(f.fileno()).st_size
elements = file_size / 4
assert(elements % columns == 0)
samples = elements / columns
data = struct.unpack('f' * elements, f.read())


freq_samples = (samples - fft_samples + fft_offset) / fft_offset
frequencies = fft_samples / 2 + 1

signal = data[0::columns]
ecog = [[0.0 for i in xrange(samples)] for j in xrange(channels)] # TODO: Does this need to be defined as a 2d array?
for i in xrange(channels):
  ecog[i] = data[(i+4)::20]
ecog_freq = [[[0.0 for i in xrange(freq_samples)] for j in xrange(channels)] for k in xrange(frequencies)]
predictor = [[1.0 for i in xrange(freq_samples)] for j in xrange(channels * frequencies + 1)] # Last row becomes a constant term

signal = array(signal)
signal = signal[fft_samples::10] # Downsample signal
ecog = array(ecog)
ecog_freq = array(ecog_freq)
predictor = array(predictor)

for i in xrange(freq_samples):
  for j in xrange(channels):
    ecog_freq[:,j,i] = fft.rfft(ecog[j, (i * fft_offset):(i * fft_offset) + fft_samples])
  print(i)

ecog_freq = absolute(ecog_freq)

for j in xrange(channels):
  for k in xrange(frequencies):
    predictor[j * frequencies + k, :] = ecog_freq[k, j, :]

if save_output:
  f = open('predictor', 'w')
  for i in xrange(channels * frequencies + 1):
    for j in xrange(freq_samples):
      f.write(str(predictor[i, j]))
      f.write('\t')
    f.write('\n')
    print(i)

pls = PLSRegression(n_components, scale)

pls.fit(predictor.T, signal)
score = pls.score(predictor.T, signal)

print(score)

predicted = pls.predict(predictor.T)

if save_output:
  f = open('signal', 'w')
  for i in xrange(freq_samples):
    f.write(str(signal[i]))
    f.write('\t')
    f.write(str(predicted[i][0]))
    f.write('\n')

for i in range(pls.coefs.size):
  print('Electode #' + str(i / frequencies) + ' (' + str((i % frequencies) * sampling_rate / fft_samples) + ' Hz): ' + str(pls.coefs[i]))
