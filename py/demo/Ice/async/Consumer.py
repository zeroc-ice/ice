#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, traceback, threading, Ice

slice_dir = os.getenv('ICEPY_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, 'slice')):
    slice_dir = os.getenv('ICE_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, 'slice')):
    slice_dir = os.path.join('/', 'usr', 'share')
if not os.path.exists(os.path.join(slice_dir, 'slice')):
    print sys.argv[0] + ': Slice directory not found. Define ICEPY_HOME or ICE_HOME.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + '/slice Queue.ice')
import Demo

class AMI_Queue_getI:
    def __init__(self, id):
        self._id = id

        requestMutex.acquire()
	requests.append(id)
        requestMutex.release()

    def ice_response(self, message):
        requestMutex.acquire()
        for i in range(0, len(requests)):
	   if requests[i] == self._id:
	       del requests[i]
	       break
        requestMutex.release()

        print message

    def ice_exception(self, ex):
        requestMutex.acquire()
        for i in range(0, len(requests)):
	   if requests[i] == self._id:
	       del requests[i]
	       break
        requestMutex.release()

        print ex

def menu():
    print """
usage:
g: get a message
x: exit
?: help
"""

class Consumer(Ice.Application):
    def run(self, args):
	properties = self.communicator().getProperties()
	refProperty = 'Queue.Proxy'
	proxy = properties.getProperty(refProperty)
	if len(proxy) == 0:
	    print args[0] + ": property `" + refProperty + "' not set"
	    return False

	queue = Demo.QueuePrx.checkedCast(self.communicator().stringToProxy(proxy))
	if not queue:
	    print args[0] + ": invalid proxy"
	    return False

	menu()

	c = None
	while c != 'x':
	    try:
		c = raw_input("==> ")
		if c == 'g':
		    id = Ice.generateUUID()
		    queue.get_async(AMI_Queue_getI(id), id)
		elif c == 'x':
		    pass # Nothing to do
		elif c == '?':
		    menu()
		else:
		    print "unknown command `" + c + "'"
		    menu()
	    except EOFError:
	        break
	    except Ice.Exception, ex:
	        print ex

        requestMutex.acquire()
	if len(requests) != 0:
	    try:
	        queue.cancel(requests)
	    except Ice.Exception, ex:
	        pass
        requestMutex.release()

	return True


requests = []
requestMutex = threading.Lock()

app = Consumer()
sys.exit(app.main(sys.argv, "config.client"))
