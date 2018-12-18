#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI


class Server(TestHelper):

    def run(self, args):

        properties = self.createTestProperties(args)
        #
        # This test kills connections, so we don't want warnings.
        #
        properties.setProperty("Ice.Warn.Connections", "0")

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
            adapter2 = communicator.createObjectAdapter("ControllerAdapter")

            testController = TestI.TestIntfControllerI(adapter)

            adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
            adapter.add(TestI.TestIntfII(), Ice.stringToIdentity("test2"))
            adapter.activate()

            adapter2.add(testController, Ice.stringToIdentity("testController"))
            adapter2.activate()

            communicator.waitForShutdown()
