#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import Test
import TestI

class Server(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        properties.setProperty("Ice.Warn.Connections", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")

            adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.activate()
            communicator.waitForShutdown()
