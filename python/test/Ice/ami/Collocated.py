#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests
import TestI
import Ice


class Collocated(TestHelper):

    def run(self, args):

        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.AMICallback", "0")
        #
        # This test kills connections, so we don't want warnings.
        #
        properties.setProperty("Ice.Warn.Connections", "0")

        with self.initialize(properties=properties) as communicator:

            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.getTestEndpoint(num=1))
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter2 = communicator.createObjectAdapter("ControllerAdapter")

            testController = TestI.TestIntfControllerI(adapter)

            adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
            adapter.add(TestI.TestIntfII(), Ice.stringToIdentity("test2"))
            # adapter.activate() # Collocated test doesn't need to active the OA

            adapter2.add(testController, Ice.stringToIdentity("testController"))
            # adapter2.activate() # Collocated test doesn't need to active the OA

            AllTests.allTests(self, communicator, True)
