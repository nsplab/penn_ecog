import zmq

context = zmq.Context()

subscriber = context.socket(zmq.SUB)
subscriber.bind("ipc:///tmp/signal.pipe")
subscriber.setsockopt(zmq.SUBSCRIBE, '')

while True:
	print subscriber.recv()
