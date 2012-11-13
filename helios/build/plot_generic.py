import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation

buffer_size = 500

x = collections.deque(maxlen=buffer_size)
x2 = collections.deque(maxlen=buffer_size)
x3 = collections.deque(maxlen=buffer_size)
x4 = collections.deque(maxlen=buffer_size)
x5 = collections.deque(maxlen=buffer_size)
x6 = collections.deque(maxlen=buffer_size)
x7 = collections.deque(maxlen=buffer_size)
x8 = collections.deque(maxlen=buffer_size)
x9 = collections.deque(maxlen=buffer_size)

f = open('/home/nsplab3/code/helios/build/channelParameters.txt', 'r')
t = f.read()
f.seek(0)
pos = t.strip('()\n\ ').split(',')

for i in range(9):
	print i,": ",pos[i]

x.extend([float(pos[0])] * buffer_size)
x2.extend([float(pos[1])] * buffer_size)
x3.extend([float(pos[2])] * buffer_size)
x4.extend([float(pos[3])] * buffer_size)
x5.extend([float(pos[4])] * buffer_size)
x6.extend([float(pos[5])] * buffer_size)
x7.extend([float(pos[6])] * buffer_size)
x8.extend([float(pos[7])] * buffer_size)
x9.extend([float(pos[8])] * buffer_size)

fig = plt.figure()

ax = fig.add_subplot(911)
linex, = ax.plot(x)
#ax.ylabel('a_{0}')

ax2 = fig.add_subplot(912)
linex2, = ax2.plot(x2)
#ax2.ylabel('a_{-1}')

ax3 = fig.add_subplot(913)
linex3, = ax3.plot(x3)
#ax3.ylabel('a_{-2}')

ax4 = fig.add_subplot(914)
linex4, = ax4.plot(x4)
#plt4.ylabel('b_{0}')

ax5 = fig.add_subplot(915)
linex5, = ax5.plot(x5)
#plt5.ylabel('b_{-1}')

ax6 = fig.add_subplot(916)
linex6, = ax6.plot(x6)
#plt6.ylabel('b_{-2}')

ax7 = fig.add_subplot(917)
linex7, = ax7.plot(x7)
#plt7.ylabel('c_{0}')

ax8 = fig.add_subplot(918)
linex8, = ax8.plot(x8)
#plt8.ylabel('c_{-1}')

ax9 = fig.add_subplot(919)
linex9, = ax9.plot(x9)
#plt9.ylabel('c_{-2}')

def update(data):
        linex.set_ydata(x)
	linex2.set_ydata(x2)
	linex3.set_ydata(x3)
	linex4.set_ydata(x4)
	linex5.set_ydata(x5)
	linex6.set_ydata(x6)
	linex7.set_ydata(x7)
	linex8.set_ydata(x8)
	linex9.set_ydata(x9)
	ax.set_ylim(min(x)-0.01, max(x)+0.01)
	ax2.set_ylim(min(x2)-0.01, max(x2)+0.01)
	ax3.set_ylim(min(x3)-0.01, max(x3)+0.01)
	ax4.set_ylim(min(x4)-0.01, max(x4)+0.01)
	ax5.set_ylim(min(x5)-0.01, max(x5)+0.01)
	ax6.set_ylim(min(x6)-0.01, max(x6)+0.01)
	ax7.set_ylim(min(x7)-0.01, max(x7)+0.01)
	ax8.set_ylim(min(x8)-0.01, max(x8)+0.01)
	ax9.set_ylim(min(x9)-0.01, max(x9)+0.01)

def data_gen():
        while True:
                t = f.read()
                f.seek(0)
                pos = t.strip('()\n\ ').split(',')
		print t
		for i in range(9):
			print i,": ",pos[i]
                x.append(float(pos[0]))
          	x2.append(float(pos[1]))
          	x3.append(float(pos[2]))
          	x4.append(float(pos[3]))
          	x5.append(float(pos[4]))
          	x6.append(float(pos[5]))
          	x7.append(float(pos[6]))
          	x8.append(float(pos[7]))
          	x9.append(float(pos[8]))
          	yield x

ani = animation.FuncAnimation(fig, update, data_gen, interval=50)
plt.show()
