#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.


import TestI
from TestHelper import TestHelper

import Ice


class Server(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        properties.setProperty("Ice.Warn.Connections", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")

            adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
            adapter.add(TestI.CI(), Ice.stringToIdentity("c"))
            adapter.activate()
            communicator.waitForShutdown()
