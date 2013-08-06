CPPFLAGS=-O3 -g #-Wall -Wextra -pedantic -std=c++11
LDLIBS=-larmadillo -litpp -lzmq -lstdc++ -lm

all: run_test

run_test: run_test.o tests.o reachstateequation.o jointrse_filter.o matrix.o filter_class.o

clean:
	rm -f *~ *.o run_test

.PHONY: all clean

