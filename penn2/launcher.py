#!/usr/bin/python
from Tkinter import *
import subprocess

master = Tk()
master.title("Launcher")

sprocess = None
gprocess = None
kprocess = None


def RunSupervisor():
    global sprocess
    print "run supervisor"
    sprocess = subprocess.Popen('./supervisor.py', cwd='./supervisor/elam2/', shell=False)


def RunGraphics():
    global gprocess
    print "run graphics"
    gprocess = subprocess.Popen('./elam2', cwd='./graphics/elam2/build/', shell=False)


def RunKinect():
    global kprocess
    print "run graphics"
    kprocess = subprocess.Popen('./kinectv2.py', cwd='./signal_acquisition/kinect/', shell=False)


def Killall():
    global gprocess, sprocess
    print "kill all"
    try:
        gprocess.kill()
    except:
        pass
    try:
        sprocess.kill()
    except:
        pass
    try:
        kprocess.kill()
    except:
        pass

f = Frame(master, height=600, width=320)
#f.pack_propagate(0)
f.pack()

b0 = Button(f, text="Kinect", command=RunKinect)
b0.pack(fill=X, expand=1)
b1 = Button(f, text="Supervisor/Elam2", command=RunSupervisor)
b1.pack(fill=X, expand=1)
b2 = Button(f, text="Graphics/Elam2", command=RunGraphics)
b2.pack(fill=X, expand=1)
bk = Button(f, text="Killall", command=Killall)
bk.pack(fill=X, expand=1)


mainloop()
