#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

import sys, traceback, time, Ice

Ice.loadSlice('Hello.ice')
import Demo
from Person_pb2 import Person

class HelloI(Demo.Hello):
    def sayHello(self, p, current=None):
        print "Hello World from %s" % str(p)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print self.appName() + ": too many arguments"
            return 1

        adapter = self.communicator().createObjectAdapter("Hello")
        adapter.add(HelloI(), self.communicator().stringToIdentity("hello"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv, "config.server"))
