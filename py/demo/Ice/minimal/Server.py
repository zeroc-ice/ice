#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        print("Hello World!")

try:
    communicator = Ice.initialize(sys.argv)
    adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default -h localhost -p 10000")
    adapter.add(HelloI(), communicator.stringToIdentity("hello"))
    adapter.activate()
    communicator.waitForShutdown()
    communicator.destroy()
except:
    traceback.print_exc()
    sys.exit(1)
