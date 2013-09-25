#! /usr/bin/env python

from sklearn.cross_decomposition import PLSCanonical, PLSRegression
from numpy import array
from numpy import fft
from numpy import absolute
from numpy import vstack
from numpy import mean
from math import floor

filename_emg = 'emg_craniectomy'
filename_eeg = 'eeg_craniectomy'
#filename_emg = 'emg_synthesized'
#filename_eeg = 'eeg_synthesized'
sampling_rate = 1024
fft_samples = 64 # ???
fft_offset = 10 # ???
#samples = 410620 # skull
samples = 419836 # craniectomy
samples = 209918 # craniectomy before discontinuity
#samples = 60 * 1024 # synthesized
channels = 1
n_components = 4 # ???
scale = True # ???
save_output = True

freq_samples = (samples - fft_samples + fft_offset) / fft_offset
frequencies = fft_samples / 2 + 1

signal = [0.0 for i in xrange(freq_samples)]
emg = [0.0 for i in xrange(samples)]
emg_freq = [[0.0 for i in xrange(freq_samples)] for j in xrange(frequencies)]
eeg = [[0.0 for i in xrange(samples)] for j in xrange(channels)]
eeg_freq = [[[0.0 for i in xrange(freq_samples)] for j in xrange(channels)] for k in xrange(frequencies)]
predictor = [[1.0 for i in xrange(freq_samples)] for j in xrange(channels * frequencies + 1)] # Last row becomes a constant term

signal = array(signal)
emg = array(emg)
emg_freq = array(emg_freq)
eeg = array(eeg)
eeg_freq = array(eeg_freq)
predictor = array(predictor)

f = open(filename_emg, 'r')

for (i, line) in enumerate(f):
  if i >= samples:
    break
  value = str.split(line)
  emg[i] = float(value[0])

f = open(filename_eeg, 'r')

for (i, line) in enumerate(f):
  if i >= samples:
    break
  value = str.split(line)
  for j in xrange(channels):
    eeg[j, i] = float(value[j])

for i in xrange(freq_samples):
  emg_freq[:,i] = fft.rfft(emg[(i * fft_offset):(i * fft_offset) + fft_samples])
  for j in xrange(channels):
    eeg_freq[:,j,i] = fft.rfft(eeg[j, (i * fft_offset):(i * fft_offset) + fft_samples])
  print(i)

#print('emg_freq')
#for i in xrange(freq_samples):
#  #for j in [100, 101, 102, 103, 104]:
#  for j in xrange(frequencies):
#    print emg_freq[j, i],
#  print

emg_freq = absolute(emg_freq)
eeg_freq = absolute(eeg_freq)

for j in xrange(channels):
  for k in xrange(frequencies):
    predictor[j * frequencies + k, :] = eeg_freq[k, j, :]

#for i in xrange(channels * frequencies):
#  predictor[i, :] = abs(predictor[i, :] - mean(predictor[i, :]))

if save_output:
  f = open('emg_freq', 'w')
  for i in xrange(frequencies):
    for j in xrange(freq_samples):
      f.write(str(emg_freq[i, j]))
      f.write('\t')
    f.write('\n')
    print(i)
  
  f = open('predictor', 'w')
  for i in xrange(channels * frequencies + 1):
    for j in xrange(freq_samples):
      f.write(str(predictor[i, j]))
      f.write('\t')
    f.write('\n')
    print(i)

signal[:] = emg_freq[round(10.0 * fft_samples / sampling_rate), :]

pls = PLSRegression(n_components, scale)

pls.fit(predictor.T, signal)
score = pls.score(predictor.T, signal)

print(score)

#for i in [100]:#xrange(frequencies):
#  signal[:] = emg_freq[i, :]
#  pls.fit(predictor.T, signal)
#  score = pls.score(predictor.T, signal)
#  print(score)

predicted = pls.predict(predictor.T)

if save_output:
  f = open('signal', 'w')
  for i in xrange(freq_samples):
    f.write(str(signal[i]))
    f.write('\t')
    f.write(str(predicted[i][0]))
    f.write('\n')

for i in range(pls.coefs.size):
  print('Electrode #' + str(i / frequencies) + ' (' + str((i % frequencies) * sampling_rate / fft_samples) + ' Hz): ' + str(pls.coefs[i]))
