import itertools
import json
import multiprocessing
from random import shuffle
import orsoqs
import zmq

def check_symbols():
	'''
	Check the SWIG library has exported all of our expected symbols.
	'''
	orsoqsSymbols = dir(orsoqs)
	expectedSymbols = (
			'create_client',
			'delete_client',
			'connect',
			'disconnect',
			'create_msg',
			'delete_msg',
			'send_msg')
	for name in expectedSymbols:
		assert name in orsoqsSymbols, name + "not found in symbol list"

dataToRequest = json.dumps({"field":"value"})
dataToReply = json.dumps({"field":"value"})

def proc_cb(data):
	'''
	Basic processing function for callback.
	'''
	assert data == dataToRequest
	return dataToReply

def zsocket_listen(endpoint, expectedData, replyData):
	'''
	Function meant to be run in a seperate thread that creates a ZMQ pair server.
	'''
	c = zmq.Context()
	s = c.socket(zmq.PAIR)
	s.bind(endpoint)
	assert s.recv() == expectedData
	s.send(replyData)
	s.unbind(endpoint)
	s.close()

def check_con_push(endpoint):
	'''
	Check our push type connection.
	'''
	listenThread = multiprocessing.Process(target=zsocket_listen,
			args=(endpoint, dataToRequest, dataToReply))
	listenThread.daemon = True
	listenThread.start()
	
	orclient = orsoqs.create_client()
	orsocket = orsoqs.connect(orclient, endpoint, orsoqs.CON_PUSH)
	msg = orsoqs.create_msg(dataToRequest, len(dataToRequest))
	
	rmsg = orsoqs.send_msg(orsocket, msg)
	
	assert rmsg.str() == dataToReply
	
	orsoqs.delete_msg(msg)
	orsoqs.delete_msg(rmsg)
	orsoqs.disconnect(orsocket)
	orsoqs.delete_client(orclient)
	
	listenThread.join()

def check_con_wait(endpoint):
	'''
	Check out wait type connection.
	'''
	orclient = orsoqs.create_client()
	orsocket = orsoqs.connect(orclient, endpoint, orsoqs.CON_WAIT)
	orsocket.set_proc_callback(proc_cb)
	
	c = zmq.Context()
	s = c.socket(zmq.PAIR)
	s.bind(endpoint)
	s.send(dataToRequest)
	
	assert s.recv() == dataToReply
	
	s.unbind(endpoint)
	s.close()
	orsoqs.disconnect(orsocket)
	orsoqs.delete_client(orclient)

def check_con_request(endpoint):
	'''
	Check our request type connection.
	'''
	listenThread = multiprocessing.Process(target=zsocket_listen,
			args=(endpoint, dataToRequest, dataToReply))
	listenThread.daemon = True
	listenThread.start()
	
	orclient = orsoqs.create_client()
	orsocket = orsoqs.connect(orclient, endpoint, orsoqs.CON_REQUEST)
	msg = orsoqs.create_msg(dataToRequest, len(dataToRequest))
	
	rmsg = orsoqs.send_msg(orsocket, msg)
	
	assert rmsg.str() == dataToReply
	
	orsoqs.delete_msg(msg)
	orsoqs.delete_msg(rmsg)
	orsoqs.disconnect(orsocket)
	orsoqs.delete_client(orclient)
	
	listenThread.join()

def check_con_reply(endpoint):
	'''
	Check our reply type connection.
	'''
	orclient = orsoqs.create_client()
	orsocket = orsoqs.connect(orclient, endpoint, orsoqs.CON_REPLY)
	orsocket.set_proc_callback(proc_cb)
	
	c = zmq.Context()
	s = c.socket(zmq.PAIR)
	s.bind(endpoint)
	s.send(dataToRequest)
	
	assert s.recv() == dataToReply
	
	s.unbind(endpoint)
	s.close()
	orsoqs.disconnect(orsocket)
	orsoqs.delete_client(orclient)


if __name__ == "__main__":
	check_symbols()
	
	reqEndpoint = "tcp://127.0.0.1:12346"
	
	checks = [
			check_con_push,
			check_con_wait,
			check_con_request,
			check_con_reply
			]
	
	shuffle(checks)
	
	for test in checks:
		test(reqEndpoint)
