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

class Request(object):
    def __init__(self, id, cb):
        self.id = id
	self.cb = cb

class QueueI(Demo.Queue):
    def __init__(self):
	self._messageQueue = []
	self._requestQueue = []
	self._lock = threading.Lock()

    def get_async(self, cb, id, current=None):
        #
	# If there is already a message in the message queue, send the
	# response immediately. Otherwise add the callback to the
	# request queue.
	#
        self._lock.acquire()
        if len(self._messageQueue) != 0:
	    try:
	        cb.ice_response(self._messageQueue[0])
		del self._messageQueue[0]
	    except Ice.Exception, ex:
	        print ex
	else:
	    request = Request(id, cb)
	    self._requestQueue.append(request)
        self._lock.release()

    def add(self, message, current=None):
        #
	# If there is an outstanding request in the request queue,
	# send a response. Otherwise add the message to the message
	# queue.
	#
        self._lock.acquire()
    	if len(self._requestQueue) != 0:
	    try:
	        self._requestQueue[0].cb.ice_response(message)
	    except Ice.Exception, ex:
	        print ex
	    del self._requestQueue[0]
	else:
	    self._messageQueue.append(message)
        self._lock.release()

    def cancel_async(self, cb, ids, current=None):
        #
	# We send immediate response so that later call to ice_exception
	# on queued requests will not cause deadlocks.
	#
        cb.ice_response()

        self._lock.acquire()
	for p in ids:
	    for i in range(0, len(self._requestQueue)):
	        if self._requestQueue[i].id == p:
	            try:
	                self._requestQueue[i].cb.ice_exception(Demo.RequestCanceledException())
	            except Ice.Exception, ex:
		        # Ignore
	                pass

		    del self._requestQueue[i]
		    break
        self._lock.release()

class Server(Ice.Application):
    def run(self, args):
	adapter = self.communicator().createObjectAdapter("Queue")
	adapter.add(QueueI(), self.communicator().stringToIdentity("queue"))
	adapter.activate()
	self.communicator().waitForShutdown()
	return True

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
