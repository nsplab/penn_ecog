frequency:
  input:
    ecog.pipe (zmq): reads 21 columns at a time as input
                     column 1: unsigned long int (time stamp)
                     columns 2-21: float (channels)
  output:
    frequencies.pipe (zmq): outputs a full set of frequencies for every N samples
                            frequencies are ordered from smallest to largest
                            frequencies from the same channel are grouped together
                            all values are floats

linear_converter:`
  input:
    frequencies.pipe (zmq): output of frequency
    matrix (file): specifies the matrix that multiplies the frequency input
  output:
    signal.pipe (zmq): N floats (N: rows in matrix)

frequency_features:
  input:
    ecog.pipe (zmq): same as in frequency
    matrix (file): same as linear_converter
  output:
    signal.pipe (zmq): same as linear_converter
