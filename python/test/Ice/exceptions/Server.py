#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Test.ice")

import TestI

import Ice


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        properties.setProperty("Ice.Warn.Connections", "0")
        properties.setProperty("Ice.MessageSizeMax", "10")

        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint(num=0))
            communicator.getProperties().setProperty("Ice.MessageSizeMax", "10")
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", self.getTestEndpoint(num=1))
            communicator.getProperties().setProperty("TestAdapter2.MessageSizeMax", "0")
            communicator.getProperties().setProperty("TestAdapter3.Endpoints", self.getTestEndpoint(num=2))
            communicator.getProperties().setProperty("TestAdapter3.MessageSizeMax", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter2 = communicator.createObjectAdapter("TestAdapter2")
            adapter3 = communicator.createObjectAdapter("TestAdapter3")
            object = TestI.ThrowerI()
            adapter.add(object, Ice.stringToIdentity("thrower"))
            adapter2.add(object, Ice.stringToIdentity("thrower"))
            adapter3.add(object, Ice.stringToIdentity("thrower"))
            adapter.activate()
            adapter2.activate()
            adapter3.activate()
            communicator.waitForShutdown()
