# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice

Ice.loadSlice('Latency.ice')
import Demo

def run(argv, communicator):
    adapter = communicator.createObjectAdapter("Latency")
    object = Demo.Ping()
    adapter.add(object, Ice.stringToIdentity("ping"))
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
