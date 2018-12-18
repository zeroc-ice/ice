#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI
import Dispatcher


class Server(TestHelper):

    def run(self, args):

        initData = Ice.InitializationData()
        initData.properties = self.createTestProperties(args)

        #
        # This test kills connections, so we don't want warnings.
        #
        initData.properties.setProperty("Ice.Warn.Connections", "0")

        #
        # Limit the recv buffer size, this test relies on the socket
        # send() blocking after sending a given amount of data.
        #
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000")

        d = Dispatcher.Dispatcher()
        initData.dispatcher = d.dispatch

        with self.initialize(initData=initData) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", self.getTestEndpoint(num=1))
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter2 = communicator.createObjectAdapter("ControllerAdapter")

            testController = TestI.TestIntfControllerI(adapter)

            adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
            adapter.activate()

            adapter2.add(testController, Ice.stringToIdentity("testController"))
            adapter2.activate()

            communicator.waitForShutdown()

        Dispatcher.Dispatcher.terminate()
