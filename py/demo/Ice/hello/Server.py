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

class HelloI(Demo.Hello):
    def sayHello(self, current=None):
        print "Hello World!"

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
	adapter = self.communicator().createObjectAdapter("Hello")
	adapter.add(HelloI(), self.communicator().stringToIdentity("hello"))
	adapter.activate()
	self.communicator().waitForShutdown()
	return True

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
