#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

def run(argv, communicator):
    adapter = communicator.createObjectAdapter("Hello")
    object = HelloI()
    adapter.add(object, Ice.stringToIdentity("hello"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
