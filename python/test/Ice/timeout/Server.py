#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import threading
import time
import Ice
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

class ControllerI(Test.Controller):

    def __init__(self, adapter):
        self.adapter = adapter

    def holdAdapter(self, to, current=None):
        self.adapter.hold()
        if to >= 0:
            t = ActivateAdapterThread(self.adapter, to)
            t.start()

    def resumeAdapter(self, current=None):
        self.adapter.activate()

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Connections", "0")

        #
        # The client sends large messages to cause the transport
        # buffers to fill up.
        #
        properties.setProperty("Ice.MessageSizeMax", "10000")

        #
        # Limit the recv buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        properties.setProperty("Ice.TCP.RcvSize", "50000")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.getTestEndpoint(num=1))
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TimeoutI(), Ice.stringToIdentity("timeout"))
            adapter.activate()

            controllerAdapter = communicator.createObjectAdapter("ControllerAdapter")
            controllerAdapter.add(ControllerI(adapter), Ice.stringToIdentity("controller"))
            controllerAdapter.activate()

            communicator.waitForShutdown()
