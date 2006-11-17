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

class AMI_Queue_getI:
    def ice_response(self, message):
        print message

    def ice_exception(self, ex):
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
		    queue.get_async(AMI_Queue_getI())
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

	return True

app = Consumer()
sys.exit(app.main(sys.argv, "config.client"))
