# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Hello, Ice, _Top

class HelloI(_Top.Hello):
    def sayHello(self, current=None):
        print "Hello World!"
        print "adapter =", current.adapter.getName()
        print "id =", current.id
        print "operation =", current.operation
        print "facet =", current.facet
        print "ctx =", current.ctx

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
except Ice.Exception, ex:
    print ex
    status = False

if communicator:
    try:
        communicator.destroy()
    except Ice.Exception, ex:
        print ex
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
