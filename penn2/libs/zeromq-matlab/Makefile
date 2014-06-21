MATLAB=matlab -nosplash -nodesktop -nojvm -nodisplay
MEX=mex -O -I/usr/local/include -L/usr/local/lib
MEXEXT=$(shell mexext)
PWD=$(shell pwd)

TARGETS=zmq.mexa64
.PHONY: all clean
	
all: $(TARGETS)

zmq.mexa64:	zmq.cc
	$(MEX) $(basename $@).cc -lzmq

test:
	@echo "Testing matlab scripts!"
	$(MATLAB) -r "test_zmq"
	@echo "Done!"

clean:
	rm -f *.$(MEXEXT) *.o
