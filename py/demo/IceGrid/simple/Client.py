#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, IceGrid

Ice.loadSlice('Hello.ice')
import Demo


def menu():
    print """
usage:
t: send greeting as twoway
s: shutdown server
x: exit
?: help
"""

class Client(Ice.Application):
    def run(self, args):
        hello = None
	try:
	    hello = Demo.HelloPrx.checkedCast(self.communicator().stringToProxy("hello"))
	except Ice.NotRegisteredException:
	    proxy = "DemoIceGrid/Query"
	    query = IceGrid.QueryPrx.checkedCast(self.communicator().stringToProxy(proxy))
	    hello = Demo.HelloPrx.checkedCast(query.findObjectByType("::Demo::Hello"))

	if not hello:
	    print self.appName() + ": couldn't find a `::Demo::Hello' object."
	    return False

	menu()

	c = None
	while c != 'x':
	    try:
		c = raw_input("==> ")
		if c == 't':
		    hello.sayHello()
		elif c == 's':
		    hello.shutdown()
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
sys.exit(app.main(sys.argv, "config.client"))
