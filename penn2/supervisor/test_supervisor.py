import zmq

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.connect("ipc:///tmp/supervisor.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

while True:
	print subscriber.recv()
