#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice

Ice.loadSlice('Hello.ice')
import Demo

class HelloI(Demo.Hello):
    def __init__(self, name):
        self.name = name

    def sayHello(self, current=None):
        print(self.name + " says Hello World!")

    def shutdown(self, current=None):
        print(self.name + " shutting down...")
        current.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        properties = self.communicator().getProperties()
        adapter = self.communicator().createObjectAdapter("Hello")
        id = self.communicator().stringToIdentity(properties.getProperty("Identity"))
        adapter.add(HelloI(properties.getProperty("Ice.ProgramName")), id)
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv))
