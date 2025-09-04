#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
import threading
import time
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

from generated.test.Ice.timeout import Test

import Ice


class ActivateAdapterThread(threading.Thread):
    def __init__(self, adapter: Ice.ObjectAdapter, timeout: int):
        threading.Thread.__init__(self)
        self._adapter = adapter
        self._timeout = timeout

    def run(self):
        time.sleep(self._timeout / 1000.0)
        self._adapter.activate()


class TimeoutI(Test.Timeout):
    @override
    def op(self, current: Ice.Current):
        pass

    @override
    def sendData(self, seq: bytes, current: Ice.Current):
        pass

    @override
    def sleep(self, to: int, current: Ice.Current):
        if to != 0:
            time.sleep(to / 1000.0)


class ControllerI(Test.Controller):
    def __init__(self, adapter: Ice.ObjectAdapter):
        self.adapter = adapter

    @override
    def holdAdapter(self, to: int, current: Ice.Current):
        self.adapter.hold()
        if to >= 0:
            t = ActivateAdapterThread(self.adapter, to)
            t.start()

    @override
    def resumeAdapter(self, current: Ice.Current):
        self.adapter.activate()

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)

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
