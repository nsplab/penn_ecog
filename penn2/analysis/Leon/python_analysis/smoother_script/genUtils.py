import numpy as np
class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration
    
    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: # changed for v1.5, see below
            self.fall = True
            return True
        else:
            return False

class matlab_list(list):
    def __init__(self):
        def zero():
            while 1:
                yield 0
        self._num_gen = zero()

    def __setitem__(self,index,value):
        if isinstance(index, int):
            self.expandfor(index)
            return super(matlab_list,self).__setitem__(index,value)

        elif isinstance(index, slice):
            if index.stop<index.start:
                return super(matlab_list_list,self).__setitem__(index,value)
            else:
                self.expandfor(index.stop if abs(index.stop)>abs(index.start) else index.start)
            return super(matlab_list,self).__setitem__(index,value)

    def expandfor(self,index):
            rng = []
            if abs(index)>len(self)-1:
                if index<0:
                    rng = xrange(abs(index)-len(self))
                    for i in rng:
                        self.insert(0,self_num_gen.next())
                else:
                    rng = xrange(abs(index)-len(self)+1)
                    for i in rng:
                        self.append(self._num_gen.next())
