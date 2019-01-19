#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI


class Server(TestHelper):

    def run(self, args):

        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Connections", "0")
        properties.setProperty("Ice.ACM.Timeout", "1")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("TestAdapter.ACM.Timeout", "0")
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI.RemoteCommunicatorI(), Ice.stringToIdentity("communicator"))
            adapter.activate()

            # Disable ready print for further adapters.
            communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0")

            communicator.waitForShutdown()
