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


def menu():
    print "Enter /quit to exit."

class Publisher(Ice.Application):
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

	print "Type a message and hit return to queue a message."
	menu()

	line = None
	try:
	    while 1:
		line = raw_input("==> ")
		if len(line) != 0:
		    if line[0] == '/':
			if line == "/quit":
			    break
			menu()
		    else:
		        queue.add(line)
	except EOFError:
	    return False
	except Ice.Exception, ex:
	    print ex

	return True

app = Publisher()
sys.exit(app.main(sys.argv, "config.client"))
