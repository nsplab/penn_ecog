Some of the main approaches to communicate between C and Matlab are descibed in this link:
http://wiki.ros.org/groovy/Planning/Matlab

the least problematic appoach seems to be communicating between the two languages using TCP or a high-level library.

julia uses zmq to provide a bridge between matlab and julia: https://github.com/timholy/julia-matlab

ZMQ seems to be popular for this task. It's also here to allow Python to communicate with Matlab:
http://arokem.github.io/python-matlab-bridge/


