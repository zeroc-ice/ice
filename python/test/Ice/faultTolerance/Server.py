#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
Ice.loadSlice('Test.ice')
import Test

def usage(n):
    sys.stderr.write("Usage: " + n + " port\n")

class TestI(Test.TestIntf):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def abort(self, current=None):
        sys.stdout.write("aborting...")
        os._exit(0)

    def idempotentAbort(self, current=None):
        os._exit(0)

    def pid(self, current=None):
        return os.getpid()

def run(args, communicator):
    port = 0
    for arg in args[1:]:
        if arg[0] == '-':
            sys.stderr.write(args[0] + ": unknown option `" + arg + "'\n")
            usage(args[0])
            return False
        if port > 0:
            sys.stderr.write(args[0] + ": only one port can be specified\n")
            usage(args[0])
            return False

        port = 12010 + int(arg)

    if port <= 0:
        sys.stderr.write(args[0] + ": no port specified\n")
        usage(args[0])
        return False

    endpts = "default -p " + str(port) + ":udp"
    communicator.getProperties().setProperty("TestAdapter.Endpoints", endpts)
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = TestI()
    adapter.add(object, Ice.stringToIdentity("test"))
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
    communicator.destroy()

sys.exit(not status)
