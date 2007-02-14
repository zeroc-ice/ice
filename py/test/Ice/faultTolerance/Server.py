#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice
Ice.loadSlice('Test.ice')
import Test

def usage(n):
    sys.stderr.write("Usage: " + n + " port\n")

class TestI(Test.TestIntf):
    def __init__(self, adapter):
        self._adapter = adapter

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

    def abort(self, current=None):
        print "aborting..."
        os._exit(0)

    def idempotentAbort(self, current=None):
        os._exit(0)

    def pid(self, current=None):
        return os.getpid()

def run(args, communicator):
    port = 0
    for arg in args[1:]:
        if arg[0] == '-':
            print >> sys.stderr, args[0] + ": unknown option `" + arg + "'"
            usage(args[0])
            return False
        if port > 0:
            print >> sys.stderr, args[0] + ": only one port can be specified"
            usage(args[0])
            return False

        port = int(arg)

    if port <= 0:
        print >> sys.stderr, args[0] + ": no port specified"
        usage(args[0])
        return False

    endpts = "default -p " + str(port) + ":udp"
    communicator.getProperties().setProperty("TestAdapter.Endpoints", endpts)
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = TestI(adapter)
    adapter.add(object, communicator.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    #
    # In this test, we need a longer server idle time, otherwise
    # our test servers may time out before they are used in the
    # test.
    #
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.ServerIdleTime", "120") # Two minutes.

    communicator = Ice.initialize(sys.argv, initData)
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

sys.exit(not status)
