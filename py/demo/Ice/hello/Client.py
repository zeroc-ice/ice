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

Ice.loadSlice('Hello.ice')
import Demo


def menu():
    print """
usage:
t: send greeting as twoway
o: send greeting as oneway
O: send greeting as batch oneway
d: send greeting as datagram
D: send greeting as batch datagram
f: flush all batch requests
T: set a timeout
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
"""

class Client(Ice.Application):
    def run(self, args):
	properties = self.communicator().getProperties()
	refProperty = 'Hello.Proxy'
	proxy = properties.getProperty(refProperty)
	if len(proxy) == 0:
	    print args[0] + ": property `" + refProperty + "' not set"
	    return False

	twoway = Demo.HelloPrx.checkedCast(\
	    self.communicator().stringToProxy(proxy).ice_twoway().ice_timeout(-1).ice_secure(False))
	if not twoway:
	    print args[0] + ": invalid proxy"
	    return False

	oneway = Demo.HelloPrx.uncheckedCast(twoway.ice_oneway())
	batchOneway = Demo.HelloPrx.uncheckedCast(twoway.ice_batchOneway())
	datagram = Demo.HelloPrx.uncheckedCast(twoway.ice_datagram())
	batchDatagram = Demo.HelloPrx.uncheckedCast(twoway.ice_batchDatagram())

	secure = False
	timeout = -1

	menu()

	c = None
	while c != 'x':
	    try:
		c = raw_input("==> ")
		if c == 't':
		    twoway.sayHello()
		elif c == 'o':
		    oneway.sayHello()
		elif c == 'O':
		    batchOneway.sayHello()
		elif c == 'd':
		    if secure:
			print "secure datagrams are not supported"
		    else:
			datagram.sayHello()
		elif c == 'D':
		    if secure:
			print "secure datagrams are not supported"
		    else:
			batchDatagram.sayHello()
		elif c == 'f':
		    self.communicator().flushBatchRequests()
		elif c == 'T':
		    if timeout == -1:
			timeout = 2000
		    else:
			timeout = -1

		    twoway = Demo.HelloPrx.uncheckedCast(twoway.ice_timeout(timeout))
		    oneway = Demo.HelloPrx.uncheckedCast(oneway.ice_timeout(timeout))
		    batchOneway = Demo.HelloPrx.uncheckedCast(batchOneway.ice_timeout(timeout))

		    if timeout == -1:
			print "timeout is now switched off"
		    else:
			print "timeout is now set to 2000ms"
		elif c == 'S':
		    secure = not secure

		    twoway = Demo.HelloPrx.uncheckedCast(twoway.ice_secure(secure))
		    oneway = Demo.HelloPrx.uncheckedCast(oneway.ice_secure(secure))
		    batchOneway = Demo.HelloPrx.uncheckedCast(batchOneway.ice_secure(secure))
		    datagram = Demo.HelloPrx.uncheckedCast(datagram.ice_secure(secure))
		    batchDatagram = Demo.HelloPrx.uncheckedCast(batchDatagram.ice_secure(secure))

		    if secure:
			print "secure mode is now on"
		    else:
			print "secure mode is now off"
		elif c == 's':
		    twoway.shutdown()
		elif c == 'x':
		    pass # Nothing to do
		elif c == '?':
		    menu()
		else:
		    print "unknown command `" + c + "'"
		    menu()
	    except EOFError:
		break

	return True

app = Client()
sys.exit(app.main(sys.argv, "config"))
