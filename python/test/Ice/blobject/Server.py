#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import time

import Test

import Ice
from TestHelper import TestHelper


class TestI(Test.Hello):
    def sayHello(self, delay, current):
        if delay != 0:
            time.sleep(delay / 1000.0)

    def raiseUE(self, current):
        raise Test.UE()

    def add(self, s1, s2, current):
        return s1 + s2

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class Server(TestHelper):
    def run(self, args):
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
