CPPFLAGS=-O3 -g -Wall -pedantic -Wextra -std=c++11
LDLIBS=-lstdc++ -lGL -lGLU -lglut -lglfw -lGLEW `sdl-config --libs` -lzmq -lfftw3
CC=g++

all: linear_converter zmq_publisher zmq_subscriber frequency frequency_features

linear_converter: linear_converter.o
zmq_publisher: zmq_publisher.o
zmq_subscriber: zmq_subscriber.o
frequency: frequency.o fft.h
frequency_features: frequency_features.o fft.h

clean:
	rm -f *~ *.o linear_converter zmq_publisher zmq_subscriber frequency frequency_features

.PHONY: all clean

