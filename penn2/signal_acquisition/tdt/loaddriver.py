#!/usr/bin/env python
from subprocess import Popen
import os

#os.chdir("/home/user/")
#print(os.curdir)

lproc = Popen([r'sudo', '../signal_acquisition/tdt/loaddriver.sh'])

stdout, stderr = lproc.communicate()
