#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time, threading

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Test.ice")
import Test

class ActivateAdapterThread(threading.Thread):
    def __init__(self, adapter, timeout):
        threading.Thread.__init__(self)
        self._adapter = adapter
        self._timeout = timeout

    def run(self):
        time.sleep(self._timeout / 1000.0)
        self._adapter.activate()

class TimeoutI(Test.Timeout):
    def op(self, current=None):
        pass

    def sendData(self, data, current=None):
        pass

    def sleep(self, timeout, current=None):
        if timeout != 0:
            time.sleep(timeout / 1000.0)

    def holdAdapter(self, to, current=None):
        current.adapter.hold()
        t = ActivateAdapterThread(current.adapter, to)
        t.start()

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(TimeoutI(), Ice.stringToIdentity("timeout"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Connections", "0");

    #
    # The client sends large messages to cause the transport
    # buffers to fill up.
    #
    initData.properties.setProperty("Ice.MessageSizeMax", "10000");

    #
    # Limit the recv buffer size, this test relies on the socket
    # send() blocking after sending a given amount of data.
    #
    initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
