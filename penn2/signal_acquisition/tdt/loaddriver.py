#!/usr/bin/env python
from subprocess import Popen
import os

os.chdir("/home/user/")

lproc = Popen([r'sudo', './loaddriver.sh'])

stdout, stderr = lproc.communicate()
