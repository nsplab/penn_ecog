import collections
import matplotlib.pyplot as plt
import matplotlib.animation as animation

buffer_size = 500

x = collections.deque(maxlen=buffer_size)

f = open('/home/nsplab3/code/helios/build/innovations.txt', 'r')
t = f.read()
f.seek(0)
pos = t.strip('()\n\ ').split(',')
x.extend([float(pos[0])] * buffer_size)

fig = plt.figure()

ax = fig.add_subplot(111)
linex, = ax.plot(x)
#ax.ylabel('a_{0}')


def update(data):
        linex.set_ydata(x)
	ax.set_ylim(min(x)-0.01, max(x)+0.01)

def data_gen():
        while True:
                t = f.read()
                f.seek(0)
                pos = t.strip('()\n\ ').split(',')
                x.append(float(pos[0]))
          	yield x

ani = animation.FuncAnimation(fig, update, data_gen, interval=50)
plt.show()
