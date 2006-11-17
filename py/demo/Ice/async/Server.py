#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Queue.ice')
import Demo

class QueueI(Demo.Queue):
    def __init__(self):
	self._messageQueue = []
	self._requestQueue = []

    def get_async(self, getCB, current=None):
        if len(self._messageQueue) != 0:
	    try:
	        getCB.ice_response(self._messageQueue[0])
		del self._messageQueue[0]
	    except Ice.Exception, ex:
	        print ex
	else:
	    self._requestQueue.append(getCB)

    def add(self, message, current=None):
    	if len(self._requestQueue) != 0:
	    try:
	        self._requestQueue[0].ice_response(message)
	    except Ice.Exception, ex:
	        print ex
	    del self._requestQueue[0]
	else:
	    self._messageQueue.append(message)

class Server(Ice.Application):
    def run(self, args):
	adapter = self.communicator().createObjectAdapter("Queue")
	adapter.add(QueueI(), self.communicator().stringToIdentity("queue"))
	adapter.activate()
	self.communicator().waitForShutdown()
	return True

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
