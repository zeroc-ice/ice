#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
import time
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

from generated.test.Ice.blobject import Test

import Ice


class TestI(Test.Hello):
    @override
    def sayHello(self, delay: int, current: Ice.Current):
        if delay != 0:
            time.sleep(delay / 1000.0)

    @override
    def raiseUE(self, current: Ice.Current):
        raise Test.UE()

    @override
    def add(self, s1: int, s2: int, current: Ice.Current):
        return s1 + s2

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we suppress
        # this warning.
        #
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
